/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#if !defined( _MSC_VER )
#include <unistd.h>
#include <dirent.h>
#endif

#include <plcore/pl_console.h>
#include <plcore/pl_package.h>

#include "filesystem_private.h"
#include "pl_private.h"

#if defined( _WIN32 )
#include "3rdparty/portable_endian.h"

/*  this is required by secext.h */
#define SECURITY_WIN32
#include <security.h>
#include <shlobj.h>
#include <direct.h>
#include <io.h>

#if defined( _MSC_VER )
#if !defined( S_ISREG ) && defined( S_IFMT ) && defined( S_IFREG )
#define S_ISREG( m ) ( ( ( m ) &S_IFMT ) == S_IFREG )
#endif
#endif
#else
#if defined( __APPLE__ )
#include "3rdparty/portable_endian.h"
#endif
#include <pwd.h>
#endif

/*	File System	*/

/** FS Mounting **/
/** Future
 * 		- Descriptor for locations; then can use <location>:// to mount from a specific location
 */

typedef enum FSMountType {
	FS_MOUNT_DIR,
	FS_MOUNT_PACKAGE,
} FSMountType;

typedef struct PLFileSystemMount {
	FSMountType type;
	union {
		PLPackage *pkg;                  /* FS_MOUNT_PACKAGE */
		char path[ PL_SYSTEM_MAX_PATH ]; /* FS_MOUNT_DIR */
	};
	struct PLFileSystemMount *next, *prev;
} PLFileSystemMount;
static PLFileSystemMount *fs_mount_root = NULL;
static PLFileSystemMount *fs_mount_ceiling = NULL;

#define FS_LOCAL_HINT "local://"

IMPLEMENT_COMMAND( fsExtractPkg, "Extract the contents of a package." ) {
	if ( argc == 1 ) {
		Print( "%s", fsExtractPkg_var.description );
		return;
	}

	const char *path = argv[ 1 ];
	if ( path == NULL ) {
		PrintWarning( "Invalid path specified!\n" );
		return;
	}

	PLPackage *pkg = PlLoadPackage( path );
	if ( pkg == NULL ) {
		PrintWarning( "Failed to load package \"%s\"!\nPL: %s\n", path, PlGetError() );
		return;
	}

	for ( unsigned int i = 0; i < pkg->table_size; ++i ) {
		PLFile *file = PlLoadPackageFileByIndex( pkg, i );
		if ( file == NULL ) {
			PrintWarning( "Failed to load file at index %d, \"%s\"!\nPL: %s\n", i, pkg->table[ i ].fileName, PlGetError() );
			continue;
		}

		char pkgPath[ PL_SYSTEM_MAX_PATH ];
		memset( pkgPath, 0, sizeof( pkgPath ) );
		strncpy( pkgPath, file->path, strlen( file->path ) - strlen( PlGetFileName( file->path ) ) );

		char outPath[ PL_SYSTEM_MAX_PATH ];
		snprintf( outPath, sizeof( outPath ), "extracted/%s", pkgPath );
		if ( !PlCreatePath( outPath ) ) {
			PrintWarning( "Failed to create path, \"%s\"!\nPL: %s\n", outPath, PlGetError() );
			break;
		}

		snprintf( outPath, sizeof( outPath ), "extracted/%s", file->path );

		FILE *fout = fopen( outPath, "wb" );
		if ( fout == NULL ) {
			PrintWarning( "Failed to write file to destination, \"%s\"!\n", outPath );
			break;
		}
		fwrite( PlGetFileData( file ), sizeof( uint8_t ), PlGetFileSize( file ), fout );
		fclose( fout );

		Print( "Wrote \"%s\"\n", outPath );
	}
	Print( "End\n" );

	PlDestroyPackage( pkg );
}

IMPLEMENT_COMMAND( fsLstPkg, "List all the files in a particular package." ) {
	if ( argc == 1 ) {
		Print( "%s", fsLstPkg_var.description );
		return;
	}

	const char *path = argv[ 1 ];
	if ( path == NULL ) {
		Print( "Invalid path specified!\n" );
		return;
	}

	PLPackage *pkg = PlLoadPackage( path );
	if ( pkg == NULL ) {
		Print( "Failed to load package \"%s\"!\nPL: %s\n", path, PlGetError() );
		return;
	}

	Print( "Listing contents of %s (%d)...\n", path, pkg->table_size );
	for ( unsigned int i = 0; i < pkg->table_size; ++i ) {
		Print( "\n"
		       " num:    %d\n"
		       " name:   %s\n"
		       " size:   %u\n"
		       " csize:  %u\n"
		       " ctype:  %d\n"
		       " offset: %u\n",
		       i,
		       pkg->table[ i ].fileName,
		       pkg->table[ i ].fileSize,
		       pkg->table[ i ].compressedSize,
		       pkg->table[ i ].compressionType,
		       pkg->table[ i ].offset );
	}
	Print( "End\n" );

	PlDestroyPackage( pkg );
}

IMPLEMENT_COMMAND( fsListMounted, "Lists all of the mounted directories." ) {
	PlUnused( argv );
	PlUnused( argc );

	if ( fs_mount_root == NULL ) {
		Print( "No locations mounted\n" );
		return;
	}

	unsigned int numLocations = 0;
	PLFileSystemMount *location = fs_mount_root;
	while ( location != NULL ) {
		numLocations++;

		/* this sucks... */
		const char *path;
		if ( location->type == FS_MOUNT_PACKAGE )
			path = location->pkg->path;
		else
			path = location->path;

		Print( " (%d) %-20s : %-20s\n", numLocations, path, location->type == FS_MOUNT_DIR ? "DIRECTORY" : "PACKAGE" );
		location = location->next;
	}
	Print( "%d locations mounted\n", numLocations );
}

IMPLEMENT_COMMAND( fsUnmount, "Unmount the specified directory." ) {
	if ( argc == 1 ) {
		Print( "%s", fsUnmount_var.description );
		return;
	}

	if ( fs_mount_root == NULL ) {
		Print( "No locations mounted\n" );
		return;
	}

	PLFileSystemMount *location = fs_mount_root;
	while ( location != NULL ) {
		if ( strcasecmp( argv[ 1 ], location->path ) == 0 ) {
			PlClearMountedLocation( location );
			Print( "Done!\n" );
			return;
		}
	}

	Print( "Failed to find location: \"%s\"!\n", argv[ 1 ] );
}

IMPLEMENT_COMMAND( fsMount, "Mount the specified directory." ) {
	if ( argc == 1 ) {
		Print( "%s", fsMount_var.description );
		return;
	}

	const char *path = argv[ 1 ];
	if ( path == NULL ) {
		Print( "Invalid path specified!\n" );
		return;
	}

	PlMountLocation( path );
}

static void _plRegisterFSCommands( void ) {
	PLConsoleCommand fsCommands[] = {
	        fsExtractPkg_var,
	        fsLstPkg_var,
	        fsListMounted_var,
	        fsUnmount_var,
	        fsMount_var,
	};
	for ( unsigned int i = 0; i < PL_ARRAY_ELEMENTS( fsCommands ); ++i ) {
		PlRegisterConsoleCommand( fsCommands[ i ].cmd, fsCommands[ i ].Callback, fsCommands[ i ].description );
	}
}

void PlClearMountedLocation( PLFileSystemMount *location ) {
	if ( location->type == FS_MOUNT_PACKAGE ) {
		PlDestroyPackage( location->pkg );
		location->pkg = NULL;
	}

	if ( location->prev != NULL ) {
		location->prev->next = location->next;
	}
	if ( location->next != NULL ) {
		location->next->prev = location->prev;
	}

	/* ensure root and ceiling are always pointing to a valid location */
	if ( location == fs_mount_root ) {
		fs_mount_root = location->next;
	}
	if ( location == fs_mount_ceiling ) {
		fs_mount_ceiling = location->prev;
	}

	pl_free( location );
}

/**
 * Clear all of the mounted locations
 */
void PlClearMountedLocations( void ) {
	while ( fs_mount_root != NULL ) { PlClearMountedLocation( fs_mount_root ); }
}

static void _plInsertMountLocation( PLFileSystemMount *location ) {
	if ( fs_mount_root == NULL ) {
		fs_mount_root = location;
	}

	location->prev = fs_mount_ceiling;
	if ( fs_mount_ceiling != NULL ) {
		fs_mount_ceiling->next = location;
	}
	fs_mount_ceiling = location;
	location->next = NULL;
}

PLFileSystemMount *PlMountLocalLocation( const char *path ) {
	PLFileSystemMount *location = pl_malloc( sizeof( PLFileSystemMount ) );
	if ( PlLocalPathExists( path ) ) { /* attempt to mount it as a path */
		_plInsertMountLocation( location );
		location->type = FS_MOUNT_DIR;
		snprintf( location->path, sizeof( location->path ), "%s", path );

		Print( "Mounted directory %s successfully!\n", path );

		return location;
	} else { /* attempt to mount it as a package */
		// LoadPackage operates via the VFS, but we want to enforce a local
		// path here, so the only reasonable solution right now is to prefix
		// it with the local dir hint
		char localPath[ PL_SYSTEM_MAX_PATH ];
		if ( strncmp( FS_LOCAL_HINT, path, sizeof( FS_LOCAL_HINT ) ) != 0 ) {
			snprintf( localPath, sizeof( localPath ), FS_LOCAL_HINT "%s", path );
		} else {
			snprintf( localPath, sizeof( localPath ), "%s", path );
		}

		PLPackage *pkg = PlLoadPackage( localPath );
		if ( pkg != NULL ) {
			_plInsertMountLocation( location );
			location->type = FS_MOUNT_PACKAGE;
			location->pkg = pkg;

			Print( "Mounted package %s successfully!\n", path );

			return location;
		}
	}

	pl_free( location );

	PlReportErrorF( 0, "failed to mount location, %s", path );
	return NULL;
}

/**
 * Mount the given location. On failure returns -1.
 */
PLFileSystemMount *PlMountLocation( const char *path ) {
	if ( strncmp( FS_LOCAL_HINT, path, sizeof( FS_LOCAL_HINT ) ) == 0 ) {
		path += sizeof( FS_LOCAL_HINT );
		return PlMountLocalLocation( path );
	}

	PLFileSystemMount *location = pl_malloc( sizeof( PLFileSystemMount ) );
	if ( PlPathExists( path ) ) { /* attempt to mount it as a path */
		_plInsertMountLocation( location );
		location->type = FS_MOUNT_DIR;
		snprintf( location->path, sizeof( location->path ), "%s", path );

		Print( "Mounted directory %s successfully!\n", path );

		return location;
	} else { /* attempt to mount it as a package */
		PLPackage *pkg = PlLoadPackage( path );
		if ( pkg != NULL ) {
			_plInsertMountLocation( location );
			location->type = FS_MOUNT_PACKAGE;
			location->pkg = pkg;

			Print( "Mounted package %s successfully!\n", path );

			return location;
		}
	}

	pl_free( location );

	PlReportErrorF( 0, "failed to mount location, %s", path );
	return NULL;
}

/****/

PLFunctionResult PlInitFileSystem( void ) {
	_plRegisterFSCommands();

	PlClearMountedLocations();
	return PL_RESULT_SUCCESS;
}

void PlShutdownFileSystem( void ) {
	PlClearMountedLocations();
}

// Checks whether a file has been modified or not.
bool PlIsFileModified( time_t oldtime, const char *path ) {
	if ( !oldtime ) {
		PlReportErrorF( PL_RESULT_FILEERR, "invalid time, skipping check" );
		return false;
	}

	struct stat attributes;
	if ( stat( path, &attributes ) == -1 ) {
		PlReportErrorF( PL_RESULT_FILEERR, "failed to stat %s: %s", path, strerror( errno ) );
		return false;
	}

	if ( attributes.st_mtime > oldtime ) {
		return true;
	}

	return false;
}

bool PlIsEndOfFile( const PLFile *ptr ) {
	return ( PlGetFileOffset( ptr ) == PlGetFileSize( ptr ) );
}

/**
 * Returns the modified time of the given file.
 * @param path
 * @return Modification time in seconds. returns 0 upon fail.
 */
time_t PlGetLocalFileTimeStamp( const char *path ) {
	struct stat attributes;
	if ( stat( path, &attributes ) == -1 ) {
		PlReportErrorF( PL_RESULT_FILEERR, "failed to stat %s: %s", path, strerror( errno ) );
		return 0;
	}

	return attributes.st_mtime;
}

time_t PlGetFileTimeStamp( PLFile *ptr ) {
	/* timestamp defaults to -1 for files loaded locally */
	if ( ptr->timeStamp < 0 ) {
		PlGetLocalFileTimeStamp( ptr->path );
	}

	return ptr->timeStamp;
}

// Creates a folder at the given path.
bool PlCreateDirectory( const char *path ) {
	if ( PlLocalPathExists( path ) ) {
		return true;
	}

	if ( _pl_mkdir( path ) == 0 ) {
		return true;
	}

	PlReportErrorF( PL_RESULT_FILEERR, "%s", strerror( errno ) );

	return false;
}

bool PlCreatePath( const char *path ) {
	size_t length = strlen( path );
	if ( length >= PL_SYSTEM_MAX_PATH ) {
		PlReportErrorF( PL_RESULT_INVALID_PARM1, "path is greater that maximum supported path length, %d vs %d",
		                length, PL_SYSTEM_MAX_PATH );
		return false;
	}

	char dir_path[ PL_SYSTEM_MAX_PATH ];
	memset( dir_path, 0, sizeof( dir_path ) );
	for ( size_t i = 0; i < length; ++i ) {
		dir_path[ i ] = path[ i ];
		if ( i != 0 &&
		     ( path[ i ] == '\\' || path[ i ] == '/' ) &&
		     ( path[ i - 1 ] != '\\' && path[ i - 1 ] != '/' ) ) {
			if ( !PlCreateDirectory( dir_path ) ) {
				return false;
			}
		}
	}

	return PlCreateDirectory( dir_path );
}

// Returns the extension for the file.
const char *PlGetFileExtension( const char *in ) {
	const char *s = strrchr( in, '.' );
	if ( !s || s == in ) {
		return "";
	}

	return s + 1;
}

// Strips the extension from the filename.
void PlStripExtension( char *dest, size_t length, const char *in ) {
	if ( PL_INVALID_STRING( in ) ) {
		*dest = 0;
		return;
	}

	const char *s = strrchr( in, '.' );
	while ( in < s ) {
		if ( --length <= 1 ) {
			break;
		}
		*dest++ = *in++;
	}
	*dest = 0;
}

/**
 * Returns pointer to the last component in the given filename.
 *
 * @param path
 * @return
 */
const char *PlGetFileName( const char *path ) {
	const char *lslash;
	if ( ( lslash = strrchr( path, '/' ) ) == NULL ) {
		lslash = strrchr( path, '\\' );
	}

	if ( lslash != NULL ) {
		path = lslash + 1;
	}

	return path;
}

/** Returns the name of the systems current user.
 *
 * @param out
 */
char *PlGetUserName( char *out, size_t n ) {
#ifdef _WIN32
	char user_string[ PL_SYSTEM_MAX_USERNAME ];
	ULONG size = PL_SYSTEM_MAX_USERNAME;
	if ( GetUserNameEx( NameDisplay, user_string, &size ) == 0 ) {
		snprintf( user_string, sizeof( user_string ), "user" );
	}
#else// Linux
	char *user_string = getenv( "LOGNAME" );
	if ( user_string == NULL ) {
		user_string = "user";
	}
#endif

	strncpy( out, user_string, n );
	return out;
}

/** Returns directory for saving application data.
 *
 * @param app_name Name of your application.
 * @param out Buffer we'll be storing the path to.
 * @param n Length of the buffer.
 * @return Pointer to the output, will return NULL on error.
 */
char *PlGetApplicationDataDirectory( const char *app_name, char *out, size_t n ) {
	if ( PL_INVALID_STRING( app_name ) ) {
		PlReportErrorF( PL_RESULT_FILEPATH, "invalid app name" );
		return NULL;
	}

#ifndef _WIN32
	const char *home;
	if ( ( home = getenv( "HOME" ) ) == NULL ) {
		struct passwd *pw = getpwuid( getuid() );
		home = pw->pw_dir;
	}
	snprintf( out, n, "%s/.%s/", home, app_name );
#else
	char home[ MAX_PATH ];
	if ( SUCCEEDED( SHGetFolderPath( NULL, CSIDL_APPDATA, NULL, 0, home ) ) ) {
		snprintf( out, n, "%s/.%s", home, app_name );
		return out;
	}
	snprintf( home, sizeof( home ), "." );
#endif

	return out;
}

/**
 * Returns true if the path ends in a forward/backward slash.
 */
static bool PathEndsInSlash( const char *p ) {
	size_t l = strlen( p );
	return ( l > 0 && ( p[ l - 1 ] == '/' || p[ l - 1 ] == '\\' ) );
}

typedef struct FSScanInstance {
	char path[ PL_SYSTEM_MAX_PATH ];
	struct FSScanInstance *next;
} FSScanInstance;

static void ScanLocalDirectory( const PLFileSystemMount *mount, FSScanInstance **fileList, const char *path,
                                const char *extension, void ( *Function )( const char *, void * ), bool recursive, void *userData ) {
#if !defined( _MSC_VER )
	DIR *directory = opendir( path );
	if ( directory ) {
		struct dirent *entry;
		while ( ( entry = readdir( directory ) ) ) {
			if ( strcmp( entry->d_name, "." ) == 0 || strcmp( entry->d_name, ".." ) == 0 ) {
				continue;
			}

			char filestring[ PL_SYSTEM_MAX_PATH + 1 ];
			if ( PathEndsInSlash( path ) ) {
				snprintf( filestring, sizeof( filestring ), "%s%s", path, entry->d_name );
			} else {
				snprintf( filestring, sizeof( filestring ), "%s/%s", path, entry->d_name );
			}

			struct stat st;
			if ( stat( filestring, &st ) == 0 ) {
				if ( S_ISREG( st.st_mode ) ) {
					if ( extension == NULL || pl_strcasecmp( PlGetFileExtension( entry->d_name ), extension ) == 0 ) {
						if ( mount == NULL ) {
							Function( filestring, userData );
							continue;
						}

						size_t pos = strlen( mount->path );
						if ( pos >= sizeof( filestring ) ) {
							PrintWarning( "pos >= %d!\n", pos );
							continue;
						}
						const char *filePath = &filestring[ pos ];

						// Ensure it's not already in the list
						FSScanInstance *cur = *fileList;
						while ( cur != NULL ) {
							if ( strcmp( filePath, cur->path ) == 0 ) {
								// File was already passed back
								break;
							}

							cur = cur->next;
						}

						// Jumped the list early, so abort here
						if ( cur != NULL ) {
							continue;
						}

						Function( filePath, userData );

						// Tack it onto the list
						cur = pl_calloc( 1, sizeof( FSScanInstance ) );
						strncpy( cur->path, filePath, sizeof( cur->path ) );
						cur->next = *fileList;
						*fileList = cur;
					}
				} else if ( S_ISDIR( st.st_mode ) && recursive ) {
					ScanLocalDirectory( mount, fileList, filestring, extension, Function, recursive, userData );
				}
			}
		}

		closedir( directory );
	} else {
		PlReportErrorF( PL_RESULT_FILEPATH, "opendir failed!" );
	}
#else /* assumed win32 impl */
	if ( extension == NULL ) {
		extension = "*";
	}

	char selectorPath[ PL_SYSTEM_MAX_PATH ];
	snprintf( selectorPath, sizeof( selectorPath ), PathEndsInSlash( path ) ? "%s*.%s" : "%s/*.%s", path, extension );

	WIN32_FIND_DATA ffd;
	HANDLE find = FindFirstFile( selectorPath, &ffd );
	if ( find == INVALID_HANDLE_VALUE ) {
		return;
	}

	do {
		snprintf( selectorPath, sizeof( selectorPath ), PathEndsInSlash( path ) ? "%s%s" : "%s/%s", path, ffd.cFileName );

		if ( ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
			if ( recursive && !( strcmp( ffd.cFileName, "." ) == 0 || strcmp( ffd.cFileName, ".." ) == 0 ) ) {
				ScanLocalDirectory( mount, fileList, selectorPath, extension, Function, recursive, userData );
			}
			continue;
		}

		const char *rp = selectorPath;
		if (mount != NULL) {
			size_t pos = strlen( mount->path );
			if ( pos >= sizeof( selectorPath ) ) {
				PrintWarning( "pos >= %d!\n", pos );
				continue;
			}
			rp = &selectorPath[ pos ];
		}

		Function( rp, userData );
	} while ( FindNextFile( find, &ffd ) != FALSE );

	FindClose( find );
#endif
}

/**
 * Scans the given directory.
 *
 * @param path path to directory.
 * @param extension the extension to scan for (exclude '.').
 * @param Function callback function to deal with the file.
 * @param recursive if true, also scans the contents of each sub-directory.
 */
void PlScanDirectory( const char *path, const char *extension, void ( *Function )( const char *, void * ), bool recursive, void *userData ) {
	if ( strncmp( FS_LOCAL_HINT, path, sizeof( FS_LOCAL_HINT ) ) == 0 ) {
		ScanLocalDirectory( NULL, NULL, path + sizeof( FS_LOCAL_HINT ), extension, Function, recursive, userData );
		return;
	}

	// If no mounted locations, assume local scan
	if ( fs_mount_root == NULL ) {
		ScanLocalDirectory( NULL, NULL, path, extension, Function, recursive, userData );
		return;
	}

	FSScanInstance *fileList = NULL;
	PLFileSystemMount *location = fs_mount_root;
	while ( location != NULL ) {
		if ( location->type == FS_MOUNT_PACKAGE ) {
			// Only works for directories for now
		} else if ( location->type == FS_MOUNT_DIR ) {
			char mounted_path[ PL_SYSTEM_MAX_PATH + 1 ];
			if ( PathEndsInSlash( location->path ) ) {
				snprintf( mounted_path, sizeof( mounted_path ), "%s%s", location->path, path );
			} else {
				snprintf( mounted_path, sizeof( mounted_path ), "%s/%s", location->path, path );
			}

			ScanLocalDirectory( location, &fileList, mounted_path, extension, Function, recursive, userData );
		}

		location = location->next;
	}

	// Clean up the list
	FSScanInstance *current = fileList;
	while ( current != NULL ) {
		FSScanInstance *prev = current;
		current = current->next;
		pl_free( prev );
	}
}

const char *PlGetWorkingDirectory( void ) {
	static char out[ PL_SYSTEM_MAX_PATH ] = { '\0' };
	if ( getcwd( out, PL_SYSTEM_MAX_PATH ) == NULL ) {
		/* The MSDN documentation for getcwd() is gone, but it proooobably uses
		 * errno and friends.
		 */
		PlReportErrorF( PL_RESULT_SYSERR, "%s", strerror( errno ) );
		return NULL;
	}
	return out;
}

void PlSetWorkingDirectory( const char *path ) {
	if ( chdir( path ) != 0 ) {
		PlReportErrorF( PL_RESULT_SYSERR, "%s", strerror( errno ) );
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// FILE I/O

bool PlLocalFileExists( const char *path ) {
	struct stat buffer;
	return ( bool ) ( stat( path, &buffer ) == 0 );
}

/**
 * Checks whether or not the given file is accessible or exists.
 * @param path
 * @return False if the file wasn't accessible.
 */
bool PlFileExists( const char *path ) {
	if ( fs_mount_root == NULL ) {
		return PlLocalFileExists( path );
	} else if ( strncmp( FS_LOCAL_HINT, path, sizeof( FS_LOCAL_HINT ) ) == 0 ) {
		path += sizeof( FS_LOCAL_HINT );
		return PlLocalFileExists( path );
	}

	PLFileSystemMount *location = fs_mount_root;
	while ( location != NULL ) {
		if ( location->type == FS_MOUNT_DIR ) {
			/* todo: don't allow path to search outside of mounted path */
			char buf[ PL_SYSTEM_MAX_PATH + 1 ];
			snprintf( buf, sizeof( buf ), "%s/%s", location->path, path );
			if ( PlLocalFileExists( buf ) ) {
				return true;
			}
		} else {
			PLFile *fp = PlLoadPackageFile( location->pkg, path );
			if ( fp != NULL ) {
				PlCloseFile( fp );
				return true;
			}
		}

		location = location->next;
	}

	return false;
}

bool PlLocalPathExists( const char *path ) {
#if defined( _MSC_VER )
	errno_t err = _access_s( path, 0 );
	if ( err != 0 )
		return false;
	
	struct _stat s;
	_stat( path, &s );
	return ( s.st_mode & S_IFDIR );
#else
	DIR *dir = opendir( path );
	if ( dir ) {
		closedir( dir );
		return true;
	}

	return false;
#endif
}

bool PlPathExists( const char *path ) {
	if ( fs_mount_root == NULL ) {
		return PlLocalPathExists( path );
	} else if ( strncmp( FS_LOCAL_HINT, path, sizeof( FS_LOCAL_HINT ) ) == 0 ) {
		path += sizeof( FS_LOCAL_HINT );
		return PlLocalPathExists( path );
	}

	PLFileSystemMount *location = fs_mount_root;
	while ( location != NULL ) {
		if ( location->type == FS_MOUNT_DIR ) {
			/* todo: don't allow path to search outside of mounted path */
			char buf[ PL_SYSTEM_MAX_PATH + 1 ];
			snprintf( buf, sizeof( buf ), "%s/%s", location->path, path );
			if ( PlLocalPathExists( buf ) ) {
				return true;
			}
		} else {
			for ( unsigned int i = 0; i < location->pkg->table_size; ++i ) {
				char *p = strstr( location->pkg->path, path );
				if ( p != NULL && p == location->pkg->path ) {
					return true;
				}
			}
		}

		location = location->next;
	}

	return false;
}

/**
 * Deletes the specified file. Not VFS compatible.
 * @param path Path to the file you want to delete.
 * @return True on success and false on fail.
 */
bool PlDeleteFile( const char *path ) {
	if ( !PlLocalFileExists( path ) ) {
		return true;
	}

	int result = remove( path );
	if ( result == 0 ) {
		return true;
	}

	PlReportErrorF( PL_RESULT_FILEREAD, strerror( errno ) );
	return false;
}

/**
 * Write the data to the specified location. Not VFS compatible.
 * @param path Path to the destination.
 * @param buf Data buffer for whatever you're writing.
 * @param length Length of the data buffer.
 * @return True on success and false on fail.
 */
bool PlWriteFile( const char *path, const uint8_t *buf, size_t length ) {
	FILE *fp = fopen( path, "wb" );
	if ( fp == NULL ) {
		PlReportErrorF( PL_RESULT_FILEREAD, "failed to open %s", path );
		return false;
	}

	bool result = true;
	if ( fwrite( buf, sizeof( char ), length, fp ) != length ) {
		PlReportErrorF( PL_RESULT_FILEWRITE, "failed to write entirety of file" );
		result = false;
	}

	_pl_fclose( fp );

	return result;
}

bool PlCopyFile( const char *path, const char *dest ) {
	// read in the original
	PLFile *original = PlOpenFile( path, true );
	if ( original == NULL ) {
		PlReportErrorF( PL_RESULT_FILEREAD, "failed to open %s", path );
		return false;
	}

	// write out the copy
	FILE *copy = fopen( dest, "wb" );
	if ( copy == NULL ) {
		PlReportErrorF( PL_RESULT_FILEWRITE, "failed to open %s for write", dest );
		goto BAIL;
	}

	if ( fwrite( original->data, 1, original->size, copy ) != original->size ) {
		PlReportErrorF( PL_RESULT_FILEWRITE, "failed to write out %d bytes for %s", original->size, path );
		goto BAIL;
	}

	_pl_fclose( copy );

	PlCloseFile( original );
	return true;

BAIL:

	if ( copy != NULL ) {
		_pl_fclose( copy );
	}

	PlCloseFile( original );
	return false;
}

size_t PlGetLocalFileSize( const char *path ) {
	struct stat buf;
	if ( stat( path, &buf ) != 0 ) {
		PlReportErrorF( PL_RESULT_FILEERR, "failed to stat %s: %s", path, strerror( errno ) );
		return 0;
	}

	return ( size_t ) buf.st_size;
}

///////////////////////////////////////////

PLFile *PlOpenLocalFile( const char *path, bool cache ) {
	FILE *fp = fopen( path, "rb" );
	if ( fp == NULL ) {
		PlReportErrorF( PL_RESULT_FILEREAD, strerror( errno ) );
		return NULL;
	}

	PLFile *ptr = pl_calloc( 1, sizeof( PLFile ) );
	snprintf( ptr->path, sizeof( ptr->path ), "%s", path );
	ptr->size = PlGetLocalFileSize( path );

	if ( cache ) {
		ptr->data = pl_malloc( ptr->size * sizeof( uint8_t ) );
		ptr->pos = ptr->data;
		if ( fread( ptr->data, sizeof( uint8_t ), ptr->size, fp ) != ptr->size ) {
			FSLog( "Failed to read complete file (%s)!\n", path );
		}
		_pl_fclose( fp );
	} else {
		ptr->fptr = fp;
	}

	/* timestamp for local files is a special case */
	ptr->timeStamp = -1;

	return ptr;
}

/**
 * Opens the specified file via the VFS.
 * @param path Path to the file you want to open.
 * @param cache Whether or not to cache the entire file into memory.
 * @return Returns handle to the file instance.
 */
PLFile *PlOpenFile( const char *path, bool cache ) {
	if ( PL_INVALID_STRING( path ) ) {
		PlReportBasicError( PL_RESULT_FILEPATH );
		return NULL;
	}

	if ( fs_mount_root == NULL ) {
		return PlOpenLocalFile( path, cache );
	} else if ( strncmp( FS_LOCAL_HINT, path, sizeof( FS_LOCAL_HINT ) ) == 0 ) {
		path += sizeof( FS_LOCAL_HINT );
		return PlOpenLocalFile( path, cache );
	}

	char buf[ PL_SYSTEM_MAX_PATH + 1 ];
	PLFileSystemMount *location = fs_mount_root;
	while ( location != NULL ) {
		PLFile *fp;
		if ( location->type == FS_MOUNT_DIR ) {
			/* todo: don't allow path to search outside of mounted path */
			snprintf( buf, sizeof( buf ), "%s/%s", location->path, path );
			fp = PlOpenLocalFile( buf, cache );
		} else {
			fp = PlLoadPackageFile( location->pkg, path );
		}

		if ( fp == NULL ) {
			location = location->next;
			continue;
		}

		return fp;
	}

	/* the above will have reported an error */

	return NULL;
}

void PlCloseFile( PLFile *ptr ) {
	if ( ptr == NULL ) {
		return;
	}

	if ( ptr->fptr != NULL ) {
		_pl_fclose( ptr->fptr );
	}

	pl_free( ptr->data );
	pl_free( ptr );
}

/**
 * Returns the path of the current open file.
 * @param ptr Pointer to file handle.
 * @return Full path to the current file.
 */
const char *PlGetFilePath( const PLFile *ptr ) {
	return ptr->path;
}

const uint8_t *PlGetFileData( const PLFile *ptr ) {
	return ptr->data;
}

/**
 * Returns file size in bytes.
 * @param ptr Pointer to file handle.
 * @return Number of bytes within file.
 */
size_t PlGetFileSize( const PLFile *ptr ) {
	if ( ptr->fptr != NULL ) {
		return PlGetLocalFileSize( ptr->path );
	}

	return ptr->size;
}

/**
 * Returns the current position within the file handle (ftell).
 * @param ptr Pointer to the file handle.
 * @return Number of bytes into the file.
 */
size_t PlGetFileOffset( const PLFile *ptr ) {
	if ( ptr->fptr != NULL ) {
		return ftell( ptr->fptr );
	}

	return ptr->pos - ptr->data;
}

size_t PlReadFile( PLFile *ptr, void *dest, size_t size, size_t count ) {
	/* bail early if size is 0 to avoid division by 0 */
	if ( size == 0 ) {
		PlReportBasicError( PL_RESULT_FILESIZE );
		return 0;
	}

	if ( ptr->fptr != NULL ) {
		return fread( dest, size, count, ptr->fptr );
	}

	/* ensure that the read is valid */
	size_t length = size * count;
	size_t posn = PlGetFileOffset( ptr );
	if ( posn + length >= ptr->size ) {
		/* out of bounds, truncate it */
		length = ptr->size - posn;
	}

	memcpy( dest, ptr->pos, length );
	ptr->pos += length;
	return length / size;
}

char PlReadInt8( PLFile *ptr, bool *status ) {
	if ( PlGetFileOffset( ptr ) >= ptr->size ) {
		if ( status != NULL ) {
			*status = false;
		}
		return 0;
	}

	if ( status != NULL ) {
		*status = true;
	}

	if ( ptr->fptr != NULL ) {
		return ( char ) ( fgetc( ptr->fptr ) );
	}

	return ( char ) *( ptr->pos++ );
}

static int64_t ReadSizedInteger( PLFile *ptr, size_t size, bool big_endian, bool *status ) {
	int64_t n;
	if ( PlReadFile( ptr, &n, size, 1 ) != 1 ) {
		if ( status != NULL ) {
			*status = false;
		}
		return 0;
	}

	if ( status != NULL ) {
		*status = true;
	}

	if ( big_endian ) {
		if ( size == sizeof( int16_t ) ) {
			return be16toh( ( int16_t ) n );
		} else if ( size == sizeof( int32_t ) ) {
			return be32toh( ( int32_t ) n );
		} else if ( size == sizeof( int64_t ) ) {
			return be64toh( n );
		} else {
			if ( status != NULL ) {
				*status = false;
			}
			return 0;
		}
	}

	return n;
}

int16_t PlReadInt16( PLFile *ptr, bool big_endian, bool *status ) {
	return ( int16_t ) ReadSizedInteger( ptr, sizeof( int16_t ), big_endian, status );
}

int32_t PlReadInt32( PLFile *ptr, bool big_endian, bool *status ) {
	return ( int32_t ) ReadSizedInteger( ptr, sizeof( int32_t ), big_endian, status );
}

int64_t PlReadInt64( PLFile *ptr, bool big_endian, bool *status ) {
	return ReadSizedInteger( ptr, sizeof( int64_t ), big_endian, status );
}

char *PlReadString( PLFile *ptr, char *str, size_t size ) {
	if ( size == 0 ) {
		PlReportBasicError( PL_RESULT_INVALID_PARM3 );
		return NULL;
	}

	if ( ptr->fptr != NULL ) {
		return fgets( str, ( int ) size, ptr->fptr );
	}

	if ( ptr->pos >= ptr->data + ptr->size ) {
		PlReportBasicError( PL_RESULT_FILEREAD );
		return NULL;
	}

	char *nl = memchr( ptr->pos, '\n', ptr->size - ( ptr->pos - ptr->data ) );
	if ( nl == NULL ) {
		nl = ( char * ) ( ptr->data + ptr->size - 1 );
	}

	if ( ( nl - ( char * ) ptr->pos ) + 1 >= ( signed long ) size ) {
		nl = ( char * ) ( ptr->pos + size );
	}

	memcpy( str, ptr->pos, ( nl - ( char * ) ptr->pos ) + 1 );
	str[ ( nl - ( char * ) ptr->pos ) + 1 ] = '\0';

	ptr->pos = ( uint8_t * ) ( nl + 1 );

	return str;
}

bool PlFileSeek( PLFile *ptr, long int pos, PLFileSeek seek ) {
	if ( ptr->fptr != NULL ) {
		int err = fseek( ptr->fptr, pos, seek );
		if ( err != 0 ) {
			PlReportErrorF( PL_RESULT_FILEREAD, "failed to seek file (%s)", GetLastError_strerror( GetLastError() ) );
			return false;
		}

		return true;
	}

	size_t posn = PlGetFileOffset( ptr );
	switch ( seek ) {
		case PL_SEEK_CUR:
			if ( posn + pos > ptr->size || pos < -( ( signed long ) posn ) ) {
				PlReportBasicError( PL_RESULT_INVALID_PARM2 );
				return false;
			}
			ptr->pos += pos;
			break;

		case PL_SEEK_SET:
			if ( pos > ( signed long ) ptr->size || pos < 0 ) {
				PlReportBasicError( PL_RESULT_INVALID_PARM2 );
				return false;
			}
			ptr->pos = &ptr->data[ pos ];
			break;

		case PL_SEEK_END:
			if ( pos <= -( ( signed long ) ptr->size ) ) {
				PlReportBasicError( PL_RESULT_INVALID_PARM2 );
				return false;
			}
			ptr->pos = &ptr->data[ ptr->size - pos ];
			break;

		default:
			PlReportBasicError( PL_RESULT_INVALID_PARM3 );
			return false;
	}

	return true;
}

void PlRewindFile( PLFile *ptr ) {
	if ( ptr->fptr != NULL ) {
		rewind( ptr->fptr );
		return;
	}

	ptr->pos = ptr->data;
}
