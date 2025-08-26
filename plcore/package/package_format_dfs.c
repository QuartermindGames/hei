// SPDX-License-Identifier: MIT
// Copyright © 2017-2023 Mark E Sowden <hogsy@oldtimes-software.com>

#include "package_private.h"
#include "qmos/public/qm_os_memory.h"

#define DFS_MAGIC   PL_MAGIC_TO_NUM( 'S', 'F', 'D', 'X' )
#define DFS_VERSION 1// Area 51 is 3 - will see about adding support later

/**
 * We actually need to load the data from a different file
 */
static void *LoadPackageFile( PLFile *file, PLPackageIndex *index ) {
	const char *dfsPath = PlGetFilePath( file );
	size_t dfsPathSize = strlen( dfsPath );

	PLPath dataPath;
	PL_ZERO_( dataPath );
	strncpy( dataPath, dfsPath, dfsPathSize - 3 );
	strcat( dataPath, "000" );

	void *data = NULL;
	PLFile *dataFile = PlOpenFile( dataPath, false );
	if ( PlFileSeek( dataFile, ( signed ) index->offset, PL_SEEK_SET ) ) {
		data = QM_OS_MEMORY_NEW_( uint8_t, index->fileSize );
		if ( PlReadFile( dataFile, data, sizeof( char ), index->fileSize ) != index->fileSize ) {
			qm_os_memory_free( data );
			data = NULL;
		}
	}

	PlCloseFile( dataFile );

	return data;
}

static int ValidateDFSFile( PLFile *file ) {
	int magic = PlReadInt32( file, false, NULL );
	if ( magic != DFS_MAGIC ) {
		PlReportError( PL_RESULT_FILETYPE, PL_FUNCTION, "invalid magic" );
		return -1;
	}

	int version = PlReadInt32( file, false, NULL );
	if ( version <= 0 || version > DFS_VERSION ) {
		PlReportError( PL_RESULT_FILEVERSION, PL_FUNCTION, "invalid version" );
		return -1;
	}

	return version;
}

static char *ReadString( PLFile *file, char **dst, uint32_t offset ) {
	PLFileOffset store = PlGetFileOffset( file );
	PlFileSeek( file, offset, PL_SEEK_SET );
	for ( ;; ) {
		bool status;
		char c = ( char ) PlReadInt8( file, &status );
		if ( !status || c == '\0' ) {
			break;
		}
		if ( c == '\\' ) {
			c = '/';
		}
		*( *dst )++ = c;
	}
	PlFileSeek( file, store, PL_SEEK_SET );
	return *dst;
}

PLPackage *PlParseDfsPackage_( PLFile *file ) {
	size_t fileSize = PlGetFileSize( file );
	if ( fileSize == 0 ) {
		PlReportError( PL_RESULT_FILESIZE, PL_FUNCTION, "empty file" );
		return NULL;
	}

	int version = ValidateDFSFile( file );
	if ( version == -1 ) {
		return NULL;
	}

	PlReadInt32( file, false, NULL );// unknown
	PlReadInt32( file, false, NULL );// unknown

	uint32_t numFiles = PL_READUINT32( file, false, NULL );
	if ( numFiles <= 0 ) {
		PlReportError( PL_RESULT_FILEERR, PL_FUNCTION, "invalid number of files" );
		return NULL;
	}

	PlReadInt32( file, false, NULL );// unknown

	uint32_t stringTableSize = PL_READUINT32( file, false, NULL );
	if ( stringTableSize <= 0 || stringTableSize >= fileSize ) {
		PlReportError( PL_RESULT_FILEERR, PL_FUNCTION, "invalid string table size" );
		return NULL;
	}

	PlReadInt32( file, false, NULL );// unknown
	PlReadInt32( file, false, NULL );// unknown

	uint32_t stringTableOffset = PL_READUINT32( file, false, NULL );
	if ( stringTableOffset <= 0 || stringTableOffset >= fileSize ) {
		PlReportError( PL_RESULT_FILEERR, PL_FUNCTION, "invalid string table offset" );
		return NULL;
	}

	PlReadInt32( file, false, NULL );// unknown

	PLPackage *package = PlCreatePackageHandle( PlGetFilePath( file ), numFiles, LoadPackageFile );

	for ( uint32_t i = 0; i < numFiles; ++i ) {
		// urgh...
		uint32_t nameOffset = PL_READUINT32( file, false, NULL );
		uint32_t nameExtendedOffset = PL_READUINT32( file, false, NULL );
		uint32_t dirOffset = PL_READUINT32( file, false, NULL );
		uint32_t extensionOffset = PL_READUINT32( file, false, NULL );

		char *c = package->table[ i ].fileName;
		ReadString( file, &c, stringTableOffset + dirOffset );
		ReadString( file, &c, stringTableOffset + nameOffset );
		ReadString( file, &c, stringTableOffset + nameExtendedOffset );
		ReadString( file, &c, stringTableOffset + extensionOffset );
		pl_strntolower( package->table[ i ].fileName, sizeof( package->table[ i ].fileName ) );

		package->table[ i ].offset = PL_READUINT32( file, false, NULL );
		package->table[ i ].fileSize = PL_READUINT32( file, false, NULL );
	}

	return package;
}
