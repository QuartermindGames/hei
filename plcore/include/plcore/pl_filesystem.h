/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#pragma once

#include <plcore/pl.h>

#define PlKilobytesToBytes( a ) ( ( double ) ( ( a ) ) * 1000 )
#define PlMegabytesToBytes( a ) ( PlKilobytesToBytes( a ) * 1000 )
#define PlGigabytesToBytes( a ) ( PlMegabytesToBytes( a ) * 1000 )

#define PlBytesToKilobytes( a ) ( ( double ) ( ( a ) ) / 1000 )
#define PlBytesToMegabytes( a ) ( PlBytesToKilobytes( a ) / 1000 )
#define PlBytesToGigabytes( a ) ( PlBytesToMegabytes( a ) / 1000 )

#define PlBytesToKibibytes( a ) ( ( double ) ( ( a ) ) / 1024 )
#define PlBytesToMebibytes( a ) ( PlBytesToKibibytes( a ) / 1024 )
#define PlBytesToGibibytes( a ) ( PlBytesToMebibytes( a ) / 1024 )

typedef struct PLFile PLFile;

typedef enum PLFileSeek {
#if defined( SEEK_SET ) && defined( SEEK_CUR ) && defined( SEEK_END )
	PL_SEEK_SET = SEEK_SET, /* set the explicit location */
	PL_SEEK_CUR = SEEK_CUR, /* seek from the current location */
	PL_SEEK_END = SEEK_END
#else
	PL_SEEK_SET,
	PL_SEEK_CUR,
	PL_SEEK_END
#endif
} PLFileSeek;

typedef enum PLFileMemoryBufferType {
	PL_FILE_MEMORYBUFFERTYPE_COPY,      /* creates a copy of the given input and takes ownership */
	PL_FILE_MEMORYBUFFERTYPE_OWNER,     /* takes ownership of the input and frees on close */
	PL_FILE_MEMORYBUFFERTYPE_UNMANAGED, /* doesn't free on close */
} PLFileMemoryBufferType;

typedef enum PLFileSystemMountType {
	PL_FS_MOUNT_DIR,
	PL_FS_MOUNT_PACKAGE,
} PLFileSystemMountType;

typedef struct PLFileSystemMount PLFileSystemMount;

#ifdef PL_FILESYSTEM_64
typedef int64_t PLFileOffset;
#else
typedef int32_t PLFileOffset;
#endif

PL_EXTERN_C

#if !defined( PL_COMPILE_PLUGIN )

/**
 * Constructs a formatted path string and stores it in the provided destination buffer.
 *
 * The function uses variable arguments to build a formatted string and stores the result in 'dst'.
 * If 'truncate' is set to `true`, the function will truncate the resulting string if it exceeds
 * the size of `PLPath`. Otherwise, the function will report an error if the size is exceeded.
 *
 * If the length of the constructed path is zero or the path exceeds the maximum allowed length and
 * 'truncate' is false, the function returns NULL.
 *
 * The resulting path is normalized by converting backslashes ('\') to forward slashes ('/'), and
 * ensuring that trailing slashes are removed.
 *
 * @param dst 		The destination buffer of type `PLPath` where the constructed path will be stored.
 * @param truncate 	A boolean indicating whether to truncate the resulting path if it exceeds the
 *        			size of `PLPath`.
 * @param msg 		A format string that specifies how to construct the path.
 * @param ... 		Additional arguments that will be used in the construction of the formatted string.
 *
 * @return 			A pointer to the resulting path stored in 'dst', or NULL if an error occurs.
 */
const char *PlSetupPath( PLPath dst, bool truncate, const char *msg, ... );

/**
 * Append a source path to a destination path, with an option to truncate if necessary.
 *
 * This function appends the given source path to the destination path. If the combined
 * length of both paths exceeds the maximum length and truncation is allowed, it will truncate
 * the destination path. If truncation is not allowed and the combined length exceeds
 * the maximum length, it will report an error and return NULL.
 *
 * @param dst 		The destination path (of type PLPath).
 * @param src 		The source path to append to the destination path.
 * @param truncate 	Flag indicating whether to truncate the destination path if the combined length exceeds the limit.
 *
 * @return The modified destination path, or NULL if an error occurred.
 */
const char *PlAppendPath( PLPath dst, const char *src, bool truncate );

/**
 * @brief Appends a formatted string to a destination path with optional truncation.
 *
 * This function appends a formatted string, specified by a format string and
 * optional additional arguments, to an existing destination path.
 * The resulting path is normalized, converting backslashes ('\\') to forward slashes ('/').
 *
 * @param dst 		The destination path buffer where the formatted string should be appended.
 * @param truncate 	A boolean flag to allow truncation of the resulting path if it exceeds the maximum path length.
 * @param msg 		The format string that specifies how subsequent arguments are converted for output.
 * @param ... 		Additional arguments for the format string.
 *
 * @return 	A pointer to the destination buffer containing the combined and normalized path,
 * 			or NULL if the operation fails due to memory constraints or improper formatting.
 */
const char *PlAppendPathEx( PLPath dst, bool truncate, const char *msg, ... );

/**
 * Prefixes a given source path to a destination path, optionally truncating the result if it exceeds the maximum path length.
 *
 * @param dst 		The destination path buffer where the prefixed path will be stored.
 * @param src 		The source path to prefix to the destination path.
 * @param truncate 	A boolean flag indicating whether to truncate the resulting path if it exceeds the maximum allowed length.
 *
 * @return A pointer to the destination path buffer containing the prefixed path, or NULL if an error occurred.
 */
const char *PlPrefixPath( PLPath dst, const char *src, bool truncate );

void PlNormalizePath( char *path, size_t length );

char *PlGetFolderForPath( PLPath dst, PLPath src );

char *PlGetUserName( char *out, size_t n );

const char *PlGetWorkingDirectory( void );
void PlSetWorkingDirectory( const char *path );

const char *PlGetExecutablePath( char *out, size_t outSize );
const char *PlGetExecutableDirectory( char *out, size_t outSize );

char *PlGetApplicationDataDirectory( const char *app_name, char *out, size_t n );

bool PlPathEndsInSlash( const char *p );

void PlStripExtension( char *dest, size_t length, const char *in );

const char *PlGetFileExtension( const char *in );
const char *PlGetFileName( const char *path );

const char *PlResolveVirtualPath( const char *path, char *dest, size_t size );

bool PlLocalFileExists( const char *path );
bool PlFileExists( const char *path );
bool PlLocalPathExists( const char *path );
bool PlPathExists( const char *path );

void PlScanDirectory( const char *path, const char *extension, void ( *Function )( const char *, void * ), bool recursive, void *userData );

bool PlCreateDirectory( const char *path );
bool PlCreatePath( const char *path );

// File I/O ...

PLFile *PlCreateFileFromMemory( const char *path, void *buf, size_t bufSize, PLFileMemoryBufferType bufType );
PLFile *PlOpenLocalFile( const char *path, bool cache );
PLFile *PlOpenFile( const char *path, bool cache );
void PlCloseFile( PLFile *ptr );

bool PlCopyFile( const char *path, const char *dest );
bool PlWriteFile( const char *path, const void *buf, size_t length );
bool PlDeleteFile( const char *path );

bool PlIsFileModified( time_t oldtime, const char *path );
bool PlIsEndOfFile( const PLFile *ptr );

time_t PlGetLocalFileTimeStamp( const char *path );
size_t PlGetLocalFileSize( const char *path );

const char *PlGetFilePath( const PLFile *ptr );
const void *PlGetFileData( const PLFile *ptr );
time_t PlGetFileTimeStamp( PLFile *ptr );
size_t PlGetFileSize( const PLFile *ptr );
PLFileOffset PlGetFileOffset( const PLFile *ptr );

size_t PlReadFile( PLFile *ptr, void *dest, size_t size, size_t count );

int8_t PlReadInt8( PLFile *ptr, bool *status );
int16_t PlReadInt16( PLFile *ptr, bool big_endian, bool *status );
int32_t PlReadInt32( PLFile *ptr, bool big_endian, bool *status );
int64_t PlReadInt64( PLFile *ptr, bool big_endian, bool *status );

#	define PL_READUINT8( FILE, STATUS )          ( uint8_t ) PlReadInt8( ( FILE ), ( STATUS ) )
#	define PL_READUINT16( FILE, ENDIAN, STATUS ) ( uint16_t ) PlReadInt16( ( FILE ), ( ENDIAN ), ( STATUS ) )
#	define PL_READUINT32( FILE, ENDIAN, STATUS ) ( uint32_t ) PlReadInt32( ( FILE ), ( ENDIAN ), ( STATUS ) )
#	define PL_READUINT64( FILE, ENDIAN, STATUS ) ( uint64_t ) PlReadInt64( ( FILE ), ( ENDIAN ), ( STATUS ) )

float PlReadFloat32( PLFile *ptr, bool big_endian, bool *status );
double PlReadFloat64( PLFile *ptr, bool big_endian, bool *status );

char *PlReadString( PLFile *ptr, char *str, size_t size );

bool PlFileSeek( PLFile *ptr, PLFileOffset pos, PLFileSeek seek );
void PlRewindFile( PLFile *ptr );

const void *PlCacheFile( PLFile *file );

/** FS Mounting **/

PLFileSystemMount *PlMountLocalLocation( const char *path );
PLFileSystemMount *PlMountLocation( const char *path );

void PlClearMountedLocation( PLFileSystemMount *location );
void PlClearMountedLocations( void );

PLFileSystemMountType PlGetMountLocationType( const PLFileSystemMount *fileSystemMount );
const char *PlGetMountLocationPath( const PLFileSystemMount *fileSystemMount );

/****/

void PlClearFileAliases( void );
void PlAddFileAlias( const char *alias, const char *target );
const char *PlGetPathForAlias( const char *alias );

#endif

PL_EXTERN_C_END
