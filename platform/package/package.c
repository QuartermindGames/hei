/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org>
*/

#include "platform_private.h"
#include "package_private.h"
#include "filesystem_private.h"

PLPackage* plCreatePackage( const char* dest ) {
	if ( dest == NULL || dest[ 0 ] == '\0' ) {
		ReportError( PL_RESULT_FILEPATH, "invalid path" );
		return NULL;
	}

	PLPackage* package = pl_calloc( 1, sizeof( PLPackage ) );
	if ( package == NULL ) {
		return NULL;
	}

	memset( package, 0, sizeof( PLPackage ) );

	strncpy( package->path, dest, sizeof( package->path ) );

	return NULL;
}

/* Unloads package from memory
 */
void plDestroyPackage( PLPackage* package ) {
	if ( package == NULL ) {
		return;
	}

	pl_free( package->table );
	pl_free( package );
}
#if 0 // todo
void plWritePackage(PLPackage *package) {

}
#endif
/////////////////////////////////////////////////////////////////

typedef struct PLPackageLoader {
	const char* ext;
	PLPackage* (* LoadFunction)( const char* path );
} PLPackageLoader;

static PLPackageLoader package_loaders[MAX_OBJECT_INTERFACES];
static unsigned int num_package_loaders = 0;

void _plInitPackageSubSystem( void ) {
	plClearPackageLoaders();
}

#if 0 /* todo */
void plQuerySupportedPackages(char **array, unsigned int *size) {
	static char
}
#endif

void plClearPackageLoaders( void ) {
	memset( package_loaders, 0, sizeof( PLPackageLoader ) * MAX_OBJECT_INTERFACES );
	num_package_loaders = 0;
}

void plRegisterPackageLoader( const char* ext, PLPackage* (* LoadFunction)( const char* path ) ) {
	package_loaders[ num_package_loaders ].ext = ext;
	package_loaders[ num_package_loaders ].LoadFunction = LoadFunction;
	num_package_loaders++;
}

void plRegisterStandardPackageLoaders( void ) {
	plRegisterPackageLoader( "ff", plLoadFFPackage );
	
	plRegisterPackageLoader( "mad", plLoadMADPackage );
	plRegisterPackageLoader( "mtd", plLoadMADPackage );
	
	plRegisterPackageLoader( "lst", plLoadLSTPackage );
	plRegisterPackageLoader( "tab", plLoadTABPackage );
	plRegisterPackageLoader( "vsr", plLoadVSRPackage );
	plRegisterPackageLoader( "wad", plLoadDoomWadPackage );

	/* eradicator */
	plRegisterPackageLoader( "rid", plLoadBdirPackage );
	plRegisterPackageLoader( "rim", plLoadBdirPackage );
}

PLPackage* plLoadPackage( const char* path ) {
	FunctionStart();

	if ( !plFileExists( path ) ) {
		ReportError( PL_RESULT_FILEREAD, "failed to load package, \"%s\"", path );
		return NULL;
	}

	const char* ext = plGetFileExtension( path );
	for ( unsigned int i = 0; i < num_package_loaders; ++i ) {
		if ( package_loaders[ i ].LoadFunction == NULL ) {
			break;
		}

		if ( !plIsEmptyString( ext ) && !plIsEmptyString( package_loaders[ i ].ext ) ) {
			if ( pl_strncasecmp( ext, package_loaders[ i ].ext, sizeof( package_loaders[ i ].ext ) ) == 0 ) {
				PLPackage* package = package_loaders[ i ].LoadFunction( path );
				if ( package != NULL ) {
					strncpy( package->path, path, sizeof( package->path ) );
					return package;
				}
			}
		} else if ( plIsEmptyString( ext ) && plIsEmptyString( package_loaders[ i ].ext ) ) {
			PLPackage* package = package_loaders[ i ].LoadFunction( path );
			if ( package != NULL ) {
				strncpy( package->path, path, sizeof( package->path ) );
				return package;
			}
		}
	}

	return NULL;
}

PLFile* plLoadPackageFile( PLPackage* package, const char* path ) {
	if ( package->internal.LoadFile == NULL ) {
		ReportError( PL_RESULT_FILEREAD, "package has not been initialized, no LoadFile function assigned, aborting" );
		return NULL;
	}

	for ( unsigned int i = 0; i < package->table_size; ++i ) {
		if ( strcmp( path, package->table[ i ].fileName ) != 0 ) {
			continue;
		}

		/* load in the package */
		PLFile* packageFile = plOpenFile( package->path, true );
		if ( packageFile == NULL ) {
			return NULL;
		}

		PLFile* file = NULL;

		uint8_t* dataPtr = package->internal.LoadFile( packageFile, &( package->table[ i ] ) );
		if ( dataPtr != NULL ) {
			file = pl_malloc( sizeof( PLFile ) );
			snprintf( file->path, sizeof( file->path ), "%s", package->table[ i ].fileName );
			file->size = package->table[ i ].fileSize;
			file->data = dataPtr;
			file->pos = file->data;
		}

		plCloseFile( packageFile );

		return file;
	}

	ReportError( PL_RESULT_INVALID_PARM2, "failed to find file in package" );
	return NULL;
}

PLFile *plLoadPackageFileByIndex( PLPackage *package, unsigned int index ) {
	if ( index >= package->table_size ) {
		ReportBasicError( PL_RESULT_INVALID_PARM2 );
		return NULL;
	}

	return plLoadPackageFile( package, package->table[ index ].fileName );
}

const char *plGetPackagePath( const PLPackage *package ) {
	return package->path;
}

const char *plGetPackageFileName( const PLPackage *package, unsigned int index ) {
	if ( index >= package->table_size ) {
		ReportBasicError( PL_RESULT_INVALID_PARM2 );
		return NULL;
	}

	return package->table[ index ].fileName;
}

unsigned int plGetPackageTableSize( const PLPackage *package ) {
	return package->table_size;
}

unsigned int plGetPackageTableIndex( const PLPackage *package, const char *indexName ) {
	FunctionStart();

	for ( unsigned int i = 0; i < package->table_size; ++i ) {
		if ( strcmp( indexName, package->table[ i ].fileName ) != 0 ) {
			continue;
		}

		return i;
	}

	ReportBasicError( PL_RESULT_INVALID_PARM2 );

	/* todo: revisit this... */
	return (unsigned int) -1;
}
