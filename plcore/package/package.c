// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2024 Mark E Sowden <hogsy@oldtimes-software.com>

#include <plcore/pl_console.h>

#include "package_private.h"
#include "filesystem_private.h"

#define MINIZ_NO_ARCHIVE_APIS
#include "../3rdparty/miniz/miniz.h"
#include "../3rdparty/blast/blast.h"
#include "qmos/public/qm_os_memory.h"
#include "qmos/public/qm_os_string.h"

/****************************************
 * Generic Loader
 ****************************************/

typedef struct BlstUser
{
	uint8_t     *buffer;
	unsigned int maxLength, length;
} BlstUser;

static unsigned int BlstCbIn( void *how, unsigned char **buf )
{
	BlstUser *user = how;
	if ( user->buffer == nullptr )
	{
		return 0;
	}

	*buf = user->buffer;
	return user->length;
}

static int BlstCbOut( void *how, unsigned char *buf, unsigned int len )
{
	BlstUser *user = how;
	if ( user->length >= user->maxLength )
	{
		return 1;
	}
	memcpy( user->buffer + user->length, buf, len );
	user->length += len;
	return 0;
}

static int Inflate( unsigned char *dst, uint32_t *dstLength, const unsigned char *src, uint32_t srcLength, bool raw )
{
	mz_stream stream = {};
	stream.next_in   = src;
	stream.avail_in  = srcLength;
	stream.next_out  = dst;
	stream.avail_out = *dstLength;

	int status = mz_inflateInit2( &stream, raw ? -MZ_DEFAULT_WINDOW_BITS : MZ_DEFAULT_WINDOW_BITS );
	if ( status != MZ_OK )
		return status;

	status = mz_inflate( &stream, MZ_FINISH );
	if ( status != MZ_STREAM_END )
	{
		mz_inflateEnd( &stream );
		return ( ( status == MZ_BUF_ERROR ) && ( !stream.avail_in ) ) ? MZ_DATA_ERROR : status;
	}
	*dstLength = stream.total_out;

	return mz_inflateEnd( &stream );
}

/**
 * Generic loader for package files, since this is unlikely to change
 * in most cases.
 */
static void *LoadGenericPackageFile( QmFsFile *fh, QmFsPackageFile *pi )
{
	FunctionStart();

	if ( !qm_fs_file_seek( fh, pi->offset, QM_FS_SEEK_SET ) )
	{
		return nullptr;
	}

	size_t   size    = ( pi->compressionType != PL_COMPRESSION_NONE ) ? pi->compressedSize : pi->size;
	uint8_t *dataPtr = QM_OS_MEMORY_NEW_( uint8_t, size );
	if ( PlReadFile( fh, dataPtr, sizeof( uint8_t ), size ) != size )
	{
		qm_os_memory_free( dataPtr );
		return nullptr;
	}

	if ( pi->compressionType != PL_COMPRESSION_NONE )
	{
		switch ( pi->compressionType )
		{
			default:
			{
				PlReportErrorF( PL_RESULT_UNSUPPORTED, "unsupported compression type for packages" );
				qm_os_memory_free( dataPtr );
				return nullptr;
			}
			case PL_COMPRESSION_DEFLATE:
			case PL_COMPRESSION_GZIP:
			{
				uint8_t *decompressedPtr    = QM_OS_MEMORY_NEW_( uint8_t, pi->size );
				uint32_t uncompressedLength = ( uint32_t ) pi->size;
				int      status             = Inflate( decompressedPtr, &uncompressedLength, dataPtr, ( unsigned long ) pi->compressedSize, ( pi->compressionType == PL_COMPRESSION_GZIP ) );
				qm_os_memory_free( dataPtr );
				dataPtr = decompressedPtr;
				if ( status != Z_OK )
				{
					qm_os_memory_free( dataPtr );
					PlReportErrorF( PL_RESULT_FILEERR, "failed to decompress buffer (%s)", zError( status ) );
					return nullptr;
				}
				break;
			}
			case PL_COMPRESSION_IMPLODE:
			{
				uint8_t *decompressedPtr    = QM_OS_MEMORY_NEW_( uint8_t, pi->size );
				uint32_t uncompressedLength = ( uint32_t ) pi->size;
				BlstUser in                 = {
				                                 .buffer = dataPtr,
				                                 .length = ( unsigned int ) size,
                         },
				         out = {
				                 .buffer    = decompressedPtr,
				                 .length    = 0,
				                 .maxLength = uncompressedLength,
				         };
				int status = blast( BlstCbIn, &in, BlstCbOut, &out, nullptr, nullptr );
				qm_os_memory_free( dataPtr );
				dataPtr = decompressedPtr;
				if ( status != 0 )
				{
					const char *errmsg;
					switch ( status )
					{
						case 2:
							errmsg = "ran out of input before completing decompression";
							break;
						case 1:
							errmsg = "output error before completing decompression";
							break;
						case -1:
							errmsg = "literal flag not zero or one";
							break;
						case -2:
							errmsg = "dictionary size not in 4..6";
							break;
						case -3:
							errmsg = "distance is too far back";
							break;
						default:
							errmsg = "unknown error when decompressing buffer";
							break;
					}

					qm_os_memory_free( dataPtr );
					PlReportErrorF( PL_RESULT_FILEREAD, "%s (%d)", errmsg, status );
					return nullptr;
				}
				break;
			}
			case PL_COMPRESSION_LZRW1:
			{
				size_t uncompressedLength = pi->size;
				void  *decompressedPtr    = PlDecompress_LZRW1( dataPtr, pi->compressedSize, &uncompressedLength );
				qm_os_memory_free( dataPtr );
				dataPtr = decompressedPtr;
				break;
			}
		}
	}

	return dataPtr;
}

/****************************************
 ****************************************/

/**
 * Allocate a new package handle.
 */
QmFsPackage *PlCreatePackageHandle( const char *path, unsigned int tableSize, void *( *OpenFile )( QmFsFile *filePtr, QmFsPackageFile *index ) )
{
	QmFsPackage *package = QM_OS_MEMORY_MALLOC_( sizeof( QmFsPackage ) );

	if ( OpenFile == nullptr )
	{
		package->internal.LoadFile = LoadGenericPackageFile;
	}
	else
	{
		package->internal.LoadFile = OpenFile;
	}

	package->numFiles = package->maxFiles = tableSize;
	package->files                              = QM_OS_MEMORY_NEW_( QmFsPackageFile, tableSize );

	package->path = qm_os_string_alloc( "%s", path );

	return package;
}

/* Unloads package from memory
 */
void PlDestroyPackage( QmFsPackage *package )
{
	if ( package == nullptr )
	{
		return;
	}

	qm_os_memory_free( package->files );
	qm_os_memory_free( package->path );
	qm_os_memory_free( package );
}
#if 0// todo
void plWritePackage(QmFsPackage *package) {

}
#endif

void PlExtractPackage( QmFsPackage *package, const char *path )
{
	for ( unsigned int i = 0; i < package->numFiles; ++i )
	{
		/* create the file dir first */
		PLPath subPath;
		snprintf( subPath, sizeof( subPath ), "%s", package->files[ i ].name );
		unsigned int l = ( unsigned int ) strlen( subPath );
		for ( unsigned int j = l; j > 0; --j )
		{
			if ( subPath[ j ] != '\\' && subPath[ j ] != '/' )
				continue;

			subPath[ j ] = '\0';
			break;
		}
		PLPath writePath;
		snprintf( writePath, sizeof( writePath ), PlPathEndsInSlash( path ) ? "%s%s" : "%s/%s", path, subPath );
		if ( !PlCreatePath( writePath ) )
		{
			FSLog( "Failed to create path: %s\n", PlGetError() );
			continue;
		}

		QmFsFile *file = PlLoadPackageFileByIndex( package, i );
		if ( file == nullptr )
		{
			FSLog( "Failed to load package file: %s\n", PlGetError() );
			continue;
		}
		const void *p = qm_fs_file_get_data( file );

		/* now write it out */
		snprintf( writePath, sizeof( writePath ), PlPathEndsInSlash( path ) ? "%s%s" : "%s/%s", path, package->files[ i ].name );
		if ( !PlWriteFile( writePath, p, package->files[ i ].size ) )
		{
			FSLog( "Failed to write package file: %s\n", PlGetError() );
		}

		PlCloseFile( file );
	}
}

/////////////////////////////////////////////////////////////

typedef struct PLPackageLoader
{
	const char *ext;
	QmFsPackage *( *LoadFunction )( const char *path );
	QmFsPackage *( *ParseFunction )( QmFsFile *file );
} PLPackageLoader;

static PLPackageLoader package_loaders[ MAX_OBJECT_INTERFACES ] = {};
static unsigned int    num_package_loaders                      = 0;

void PlInitPackageSubSystem( void )
{
	PlClearPackageLoaders();
}

/**
 * Returns a list of file extensions representing all
 * the formats supported by the package loader.
 */
const char **PlGetSupportedPackageFormats( unsigned int *numElements )
{
	static const char *formats[ MAX_OBJECT_INTERFACES ];
	for ( unsigned int i = 0; i < num_package_loaders; ++i )
	{
		formats[ i ] = package_loaders[ i ].ext;
	}

	*numElements = num_package_loaders;

	return formats;
}

void PlClearPackageLoaders( void )
{
	num_package_loaders = 0;
}

void PlRegisterPackageLoader( const char *ext, QmFsPackage *( *LoadFunction )( const char * ), QmFsPackage *( *ParseFunction )( QmFsFile * ) )
{
	package_loaders[ num_package_loaders ].ext           = ext;
	package_loaders[ num_package_loaders ].LoadFunction  = LoadFunction;
	package_loaders[ num_package_loaders ].ParseFunction = ParseFunction;
	num_package_loaders++;
}

void PlRegisterStandardPackageLoaders( unsigned int flags )
{
	typedef struct PackageLoader
	{
		unsigned int flag;
		const char  *extension;
		QmFsPackage *( *parseFunction )( QmFsFile *file );
	} PackageLoader;

	static const PackageLoader loaders[] = {
	        {PL_PACKAGE_LOAD_FORMAT_ZIP,         "zip",   PlParseZipPackage      },
	        {PL_PACKAGE_LOAD_FORMAT_ZIP,         "pak",   PlParseZipPackage      },
	        {PL_PACKAGE_LOAD_FORMAT_ZIP,         "pk3",   PlParseZipPackage      },
	        {PL_PACKAGE_LOAD_FORMAT_ZIP,         "pk4",   PlParseZipPackage      },
	        {PL_PACKAGE_LOAD_FORMAT_ZIP,         "cache", PlParseZipPackage      },
	        {PL_PACKAGE_LOAD_FORMAT_WAD_DOOM,    "wad",   PlParseWadPackage_     },
	        {PL_PACKAGE_LOAD_FORMAT_WAD_QUAKE,   "wad",   PlParseQWadPackage_    },
	        {PL_PACKAGE_LOAD_FORMAT_MAD_GREMLIN, "mad",   PlParseMadPackage_     },
	        {PL_PACKAGE_LOAD_FORMAT_MAD_GREMLIN, "mtd",   PlParseMadPackage_     },
	        {PL_PACKAGE_LOAD_FORMAT_PAK_QUAKE,   "pak",   PlParsePakPackage_     },
	        {PL_PACKAGE_LOAD_FORMAT_BIN_FRESH,   "bin",   PlParseFreshBinPackage_},
	        {PL_PACKAGE_LOAD_FORMAT_DFS,         "dfs",   PlParseDfsPackage_     },
	        {PL_PACKAGE_LOAD_FORMAT_VPK_VTMB,    "vpk",   PlParseVpkPackage_     },
	        {PL_PACKAGE_LOAD_FORMAT_GRP,         "grp",   PlParseGrpPackage_     },
	        {PL_PACKAGE_LOAD_FORMAT_VPP,         "vpp",   PlParseVppPackage      },
	        {PL_PACKAGE_LOAD_FORMAT_OPK,         "opk",   PlParseOpkPackage_     },
	        {PL_PACKAGE_LOAD_FORMAT_INU,         "inu",   PlParseInuPackage_     },
	        {PL_PACKAGE_LOAD_FORMAT_ALL_ACCLAIM, "all",   PlParseAllPackage_     },
	        {PL_PACKAGE_LOAD_FORMAT_AFS,         "afs",   PlParseAfsPackage_     },
	        {PL_PACKAGE_LOAD_FORMAT_AHF,         "ahf",   PlParseAhfPackage_     },
	        {PL_PACKAGE_LOAD_FORMAT_DAT_ANGEL,   "dat",   PlParseAngelDatPackage_},
	        {PL_PACKAGE_LOAD_FORMAT_HAL,         "hal",   PlParseHalPackage_     },
	        {PL_PACKAGE_LOAD_FORMAT_DAT_ICE3D,   "dat",   PlParseIce3DDatPackage_},
#if ( RAR_SUPPORTED == 1 )
	        {PL_PACKAGE_LOAD_FORMAT_RAR,         "rar",   PlParseRarPackage_     },
#endif
	        {PL_PACKAGE_LOAD_FORMAT_FRD_PAK,     "pak",   PlParseFrdPakPackage_  },

	        {PL_PACKAGE_LOAD_FORMAT_PAK_VENOM,   "bpak0", PlParseVenomPakPackage },
	        {PL_PACKAGE_LOAD_FORMAT_PAK_VENOM,   "bpak1", PlParseVenomPakPackage },
	        {PL_PACKAGE_LOAD_FORMAT_PAK_VENOM,   "bpak2", PlParseVenomPakPackage },
	        {PL_PACKAGE_LOAD_FORMAT_PAK_VENOM,   "bpak3", PlParseVenomPakPackage },
	};

	for ( unsigned int i = 0; i < QM_OS_ARRAY_ELEMENTS( loaders ); ++i )
	{
		if ( flags != PL_PACKAGE_LOAD_FORMAT_ALL && !( flags & loaders[ i ].flag ) )
		{
			continue;
		}

		PlRegisterPackageLoader( loaders[ i ].extension, nullptr, loaders[ i ].parseFunction );
	}
}

QmFsPackage *PlLoadPackage( const char *path )
{
	FunctionStart();

	const char *ext = PlGetFileExtension( path );
	for ( unsigned int i = 0; i < num_package_loaders; ++i )
	{
		if ( package_loaders[ i ].LoadFunction == nullptr )
		{
			continue;
		}

		if ( !PL_INVALID_STRING( ext ) && !PL_INVALID_STRING( package_loaders[ i ].ext ) )
		{
			if ( pl_strncasecmp( ext, package_loaders[ i ].ext, sizeof( package_loaders[ i ].ext ) ) == 0 )
			{
				QmFsPackage *package = package_loaders[ i ].LoadFunction( path );
				if ( package != nullptr )
				{
					package->path = qm_os_string_alloc( "%s", path );
					return package;
				}
			}
		}
		else if ( PL_INVALID_STRING( ext ) && PL_INVALID_STRING( package_loaders[ i ].ext ) )
		{
			QmFsPackage *package = package_loaders[ i ].LoadFunction( path );
			if ( package != nullptr )
			{
				package->path = qm_os_string_alloc( "%s", path );
				return package;
			}
		}
	}

	//TODO: this should replace the above, eventually...

	QmFsFile *file = qm_fs_file_open( path, false );
	if ( file == nullptr )
	{
		return nullptr;
	}

	QmFsPackage *package = nullptr;
	for ( unsigned int i = 0; i < num_package_loaders; ++i )
	{
		if ( package_loaders[ i ].ParseFunction == nullptr )
		{
			continue;
		}

		if ( !PL_INVALID_STRING( ext ) && !PL_INVALID_STRING( package_loaders[ i ].ext ) )
		{
			if ( pl_strncasecmp( ext, package_loaders[ i ].ext, sizeof( package_loaders[ i ].ext ) ) == 0 )
			{
				package = package_loaders[ i ].ParseFunction( file );
				if ( package != nullptr )
				{
					break;
				}
			}
		}
		else if ( PL_INVALID_STRING( ext ) && PL_INVALID_STRING( package_loaders[ i ].ext ) )
		{
			package = package_loaders[ i ].ParseFunction( file );
			if ( package != nullptr )
			{
				break;
			}
		}

		qm_fs_file_rewind( file );
	}

	PlCloseFile( file );

	if ( package != nullptr && *package->path == '\0' )
	{
		package->path = qm_os_string_alloc( "%s", path );
	}
	else if ( PlGetFunctionResult() == PL_RESULT_SUCCESS )
	{
		/* this was clearly not the case... */
		PlReportBasicError( PL_RESULT_UNSUPPORTED );
	}

	return package;
}

QmFsFile *PlLoadPackageFileByIndex( QmFsPackage *package, unsigned int index )
{
	if ( index >= package->numFiles )
	{
		PlReportBasicError( PL_RESULT_INVALID_PARM2 );
		return nullptr;
	}

	/* load in the package */
	QmFsFile *packageFile = qm_fs_file_open( package->path, false );
	if ( packageFile == nullptr )
	{
		return nullptr;
	}

	QmFsFile *file = nullptr;

	uint8_t *dataPtr = package->internal.LoadFile( packageFile, &package->files[ index ] );
	if ( dataPtr != nullptr )
	{
		file = qm_fs_file_from_memory(
		        package->files[ index ].name,
		        dataPtr,
		        package->files[ index ].size,
		        QM_FS_FILE_OWNERSHIP_TYPE_OWNER );
	}

	PlCloseFile( packageFile );

	return file;
}

QmFsFile *PlLoadPackageFile( QmFsPackage *package, const char *path )
{
	if ( package->internal.LoadFile == nullptr )
	{
		PlReportErrorF( PL_RESULT_FILEREAD, "package has not been initialized, no LoadFile function assigned, aborting" );
		return nullptr;
	}

	for ( unsigned int i = 0; i < package->numFiles; ++i )
	{
		if ( strcmp( path, package->files[ i ].name ) != 0 )
		{
			continue;
		}

		return PlLoadPackageFileByIndex( package, i );
	}

	PlReportErrorF( PL_RESULT_INVALID_PARM2, "failed to find file in package" );
	return nullptr;
}

const char *PlGetPackagePath( const QmFsPackage *package )
{
	return package->path;
}

const char *PlGetPackageFileName( const QmFsPackage *package, unsigned int index )
{
	if ( index >= package->numFiles )
	{
		PlReportBasicError( PL_RESULT_INVALID_PARM2 );
		return nullptr;
	}

	return package->files[ index ].name;
}

unsigned int PlGetPackageTableSize( const QmFsPackage *package )
{
	return package->numFiles;
}

int PlGetPackageTableIndex( const QmFsPackage *package, const char *indexName )
{
	FunctionStart();

	for ( unsigned int i = 0; i < package->numFiles; ++i )
	{
		if ( strcmp( indexName, package->files[ i ].name ) != 0 )
		{
			continue;
		}

		return ( int ) i;
	}

	PlReportBasicError( PL_RESULT_INVALID_PARM2 );

	return -1;
}
