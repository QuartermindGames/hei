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

#pragma once

#include <PL/platform.h>
#include <PL/platform_mesh.h>
#include <PL/platform_model.h>
#include <PL/platform_package.h>

typedef struct PLPluginDescription {
	unsigned int interfaceVersion;
	unsigned int pluginVersion[3]; /* Major, minor and patch. */
	const char *description;
} PLPluginDescription;

/**
 * Functions exported from the platform library.
 */
typedef struct PLPluginExportTable {
	unsigned int version;

	void (*ReportError)( PLresult resultType, const char *err, ... );

	void *( *MAlloc )( size_t size );
	void *( *CAlloc )( size_t num, size_t size );
	void *( *ReAlloc )( void *ptr, size_t newSize );
	void ( *Free )( void *ptr );

	/**
	 * FILE API
	 **/

	bool (*LocalFileExists)( const char *path );
	bool (*FileExists)( const char *path );
	bool (*LocalPathExists)( const char *path );
	bool (*PathExists)( const char *path );

	void (*ScanDirectory)( const char *path, const char *extension, void (*Function)( const char *, void * ), bool recursive, void *userData );

	bool (*CreateDirectory)( const char *path );
	bool (*CreatePath)( const char *path );

	PLFile *(*OpenLocalFile)( const char *path, bool cache );
	PLFile *(*OpenFile)( const char *path, bool cache );
	void (*CloseFile)( PLFile *file );

	bool (*IsEndOfFile)( const PLFile *file );

	const char *(*GetFilePath)( const PLFile *file );
	const uint8_t *(*GetFileData)( const PLFile *file );
	size_t (*GetFileSize)( const PLFile *file );
	size_t (*GetFileOffset)( const PLFile *file );

	size_t (*ReadFile)( PLFile *file, void *destination, size_t size, size_t count );

	char (*ReadInt8)( PLFile *file, bool *status );
	int16_t (*ReadInt16)( PLFile *file, bool bigEndian, bool *status );
	int32_t (*ReadInt32)( PLFile *file, bool bigEndian, bool *status );
	int64_t (*ReadInt64)( PLFile *file, bool bigEndian, bool *status );

	char *(*ReadString)( PLFile *file, char *destination, size_t size );

	bool (*FileSeek)( PLFile *file, long int pos, PLFileSeek seek );
	void (*RewindFile)( PLFile *file );

	/**
 	 * PLUGIN API
 	 **/

	PLPackage *(*CreatePackageHandle)( const char *path, unsigned int tableSize, void(*OpenFile)( PLFile *filePtr, PLPackageIndex *index ) );

	void (*RegisterPackageLoader)( const char *extension, PLPackage *(*LoadFunction)( const char *path ) );

	const char *(*GetPackagePath)( const PLPackage *package );
	unsigned int (*GetPackageTableSize)( const PLPackage *package );
	unsigned int (*GetPackageTableIndex)( const PLPackage *package, const char *indexName );

	const char *(*GetPackageFileName)( const PLPackage *package, unsigned int index );

	/**
	 * MESH API
	 **/

	PLMesh *(*CreateMesh)( PLMeshPrimitive primitive, PLMeshDrawMode mode, unsigned int num_tris, unsigned int num_verts );
	void (*DestroyMesh)( PLMesh *mesh );
	void (*ClearMesh)( PLMesh *mesh );
	void (*ClearMeshVertices)( PLMesh *mesh );
	void (*ClearMeshTriangles)( PLMesh *mesh );
	void (*ScaleMesh)( PLMesh *mesh, PLVector3 scale );
	unsigned int (*AddMeshVertex)( PLMesh *mesh, PLVector3 position, PLVector3 normal, PLColour colour, PLVector2 st );
	unsigned int (*AddMeshTriangle)( PLMesh *mesh, unsigned int x, unsigned int y, unsigned int z );
	void (*GenerateMeshNormals)( PLMesh *mesh, bool perFace );

	void (*GenerateTextureCoordinates)( PLVertex *vertices, unsigned int numVertices, PLVector2 textureOffset, PLVector2 textureScale );
	void (*GenerateVertexNormals)( PLVertex *vertices, unsigned int numVertices, unsigned int *indices, unsigned int numTriangles, bool perFace );

	/**
 	 * MODEL API
 	 **/

	PLModel *(*CreateStaticModel)( PLMesh *meshes, unsigned int numMeshes );
	PLModel *(*CreateSkeletalModel)( PLMesh *meshes, unsigned int numMeshes, PLModelBone *bones, unsigned int numBones );
	void (*DestroyModel)( PLModel *model );
	void (*GenerateModelNormals)( PLModel *model, bool perFace );
	void (*GenerateModelBounds)( PLModel *model );

	/**
	 * IMAGE API
	 **/

	PLImage *(*CreateImage)( uint8_t *buf, unsigned int width, unsigned int height, PLColourFormat colourFormat, PLImageFormat dataFormat );
	void (*DestroyImage)( PLImage *image );
	bool (*ConvertPixelFormat)( PLImage *image, PLImageFormat newFormat );
	void (*InvertImageColour)( PLImage *image );
	void (*ReplaceImageColour)( PLImage *image, PLColour target, PLColour destination );
	bool (*FlipImageVertical)( PLImage *image );

	unsigned int (*GetNumberOfColourChannels )( PLColourFormat colourFormat );
	unsigned int (*GetImageSize)( PLImageFormat format, unsigned int width, unsigned int height );

	void (*RegisterModelLoader)( const char *extension, PLModel*(*LoadFunction)( const char *path ) );
} PLPluginExportTable;

/* be absolutely sure to change this whenever the API is updated! */
#define PL_PLUGIN_INTERFACE_VERSION 1

#define PL_PLUGIN_QUERY_FUNCTION    "PLQueryPlugin"
#define PL_PLUGIN_INIT_FUNCTION     "PLInitializePlugin"

typedef const PLPluginDescription *(*PLPluginQueryFunction )( unsigned int interfaceVersion );
typedef void (*PLPluginInitializationFunction)( const PLPluginExportTable *functionTable );
