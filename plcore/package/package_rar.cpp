// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2024 Mark E Sowden <hogsy@oldtimes-software.com>

#include "package_private.h"

#if ( RAR_SUPPORTED == 1 )

#	include <unrar/rar.hpp>
#	include <unrar/dll.hpp>

static void *OpenRarFile( PLFile *file, PLPackageIndex *index ) {
	PLPath path;
	PlSetupPath( path, true, "%s", PlGetFilePath( file ) );

	RAROpenArchiveDataEx archiveData = {};
	archiveData.ArcName = path;
	archiveData.OpenMode = RAR_OM_EXTRACT;

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

		PLPath tmp;
		if ( PlGetTempName( tmp ) == nullptr ) {
			break;
		}

		if ( unsigned int r; ( r = RARProcessFile( handle, RAR_EXTRACT, tmp, nullptr ) ) != 0 ) {
			PlReportErrorF( PL_RESULT_FILETYPE, "failed to extract file from rar archive (%u)", r );
			break;
		}

		PLFile *tmpFile = PlOpenFile( tmp, false );
		buf = PL_NEW_( uint8_t, headerData.CmtSize );
		PlReadFile( tmpFile, buf, sizeof( uint8_t ), headerData.CmtSize );
		PlCloseFile( tmpFile );
		PlFree( tmp );
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
				package->table = static_cast< PLPackageIndex * >( PL_REALLOCA( package->table, sizeof( PLPackageIndex ) * ( package->maxTableSize + 16 ) ) );
				package->maxTableSize += 16;
			}

			PLPackageIndex *index = &package->table[ package->table_size++ ];
			index->compressedSize = headerData.PackSize;
			index->fileSize = headerData.UnpSize;
			index->compressionType = PL_COMPRESSION_RAR;
			snprintf( index->fileName, sizeof( index->fileName ), "%s", headerData.FileName );

			RARProcessFile( handle, RAR_SKIP, nullptr, nullptr );
		}
	}

	RARCloseArchive( handle );

	return package;
}

#endif
