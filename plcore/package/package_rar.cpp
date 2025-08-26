// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2025 Mark E Sowden <hogsy@oldtimes-software.com>
// Purpose: Load and extract RAR archives.

#include "package_private.h"
#include "qmos/public/qm_os_memory.h"

#if ( RAR_SUPPORTED == 1 )

#	ifndef _WIN32
#		define _UNIX /* Needed to make dll.hpp work on non-Windows. */
#	endif

#	if defined( RAR_UNRAR )
#		include <unrar/dll.hpp>
#	elif defined( RAR_LIBUNRAR )
#		include <libunrar/dll.hpp>
#	endif

static void *OpenRarFile( PLFile *file, PLPackageIndex *index ) {
	PLPath path;
	PlSetupPath( path, true, "%s", PlGetFilePath( file ) );

	RAROpenArchiveDataEx archiveData = {};
	archiveData.ArcName = path;
	archiveData.OpenMode = RAR_OM_EXTRACT;

	//TODO:
	//	is there a better way we can handle this?
	//	right now we open, close and open this again and again!

	HANDLE handle = RAROpenArchiveEx( &archiveData );
	if ( archiveData.OpenResult != 0 ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "failed to open rar archive (%u)", archiveData.OpenResult );
		return nullptr;
	}

	void *buf = nullptr;

	RARHeaderDataEx headerData = {};
	while ( RARReadHeaderEx( handle, &headerData ) == 0 ) {
		if ( strcmp( headerData.FileName, index->fileName ) != 0 ) {
			RARProcessFile( handle, RAR_SKIP, nullptr, nullptr );
			continue;
		}

		PLPath dir;
		if ( hei_fs_get_temp_path( dir, sizeof( dir ) ) == nullptr ) {
			break;
		}

		PLPath tmpFilename;
		PlSetupPath( tmpFilename, true, "%s/blob.dat", dir );

		if ( unsigned int r; ( r = RARProcessFile( handle, RAR_EXTRACT, nullptr, tmpFilename ) ) != 0 ) {
			PlReportErrorF( PL_RESULT_FILETYPE, "failed to extract file from rar archive (%u)", r );
			break;
		}

		PLFile *tmpFile = PlOpenLocalFile( tmpFilename, false );
		if ( tmpFile == nullptr ) {
			break;
		}

		buf = QM_OS_MEMORY_NEW_( uint8_t, headerData.UnpSize + 1 );
		if ( PlReadFile( tmpFile, buf, sizeof( uint8_t ), headerData.UnpSize ) != headerData.UnpSize ) {
			PlCloseFile( tmpFile );
			break;
		}

		PlCloseFile( tmpFile );

		break;
	}

	RARCloseArchive( handle );

	return buf;
}

extern "C" PLPackage *PlParseRarPackage_( PLFile *file ) {
	PLPath path;
	PlSetupPath( path, true, "%s", PlGetFilePath( file ) );

	// I couldn't find any really good documentation on this,
	// but my impression is that I can't just feed the library
	// the file, and it has to load it itself...

	RAROpenArchiveDataEx archiveData = {};
	archiveData.ArcName = path;
	archiveData.OpenMode = RAR_OM_LIST;

	HANDLE handle = RAROpenArchiveEx( &archiveData );
	if ( archiveData.OpenResult != 0 ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "failed to open rar archive (%u)", archiveData.OpenResult );
		return nullptr;
	}

	PLPackage *package = PlCreatePackageHandle( path, 0, OpenRarFile );
	if ( package != nullptr ) {
		RARHeaderDataEx headerData = {};
		while ( RARReadHeaderEx( handle, &headerData ) == 0 ) {
			if ( package->table_size >= package->maxTableSize ) {
				package->table = static_cast< PLPackageIndex * >( qm_os_memory_realloc( package->table, sizeof( PLPackageIndex ) * ( package->maxTableSize + 16 ) ) );
				package->maxTableSize += 16;
			}

			PLPackageIndex *index = &package->table[ package->table_size++ ];
			index->compressedSize = headerData.PackSize;
			index->fileSize = headerData.UnpSize;
			index->compressionType = PL_COMPRESSION_RAR;
			PlSetupPath( index->fileName, true, "%s", headerData.FileName );

			RARProcessFile( handle, RAR_SKIP, nullptr, nullptr );
		}
	}

	RARCloseArchive( handle );

	return package;
}

#endif
