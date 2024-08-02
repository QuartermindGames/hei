// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2024 Mark E Sowden <hogsy@oldtimes-software.com>

#include "blitz.h"

#define PSI_MAGIC PL_MAGIC_TO_NUM( 'P', 'S', 'I', '\0' )

typedef struct PsiHeader {
	uint32_t magic;

	uint32_t version;
	uint32_t flags;

	char name[ 32 ];

	uint32_t numMeshes;
	uint32_t numVertices;

	uint32_t numPrimitives;
	uint32_t primOffset;

	uint16_t frameStart;
	uint16_t frameEnd;

	uint32_t numSegments;
	uint32_t segmentOffset;

	uint32_t numTextures;
	uint32_t textureOffset;

	uint32_t meshOffset;

	uint32_t radius;

	uint8_t padding[ 104 ];
} PsiHeader;

typedef struct PsiVector {
	int32_t x, y, z, p;
} PsiVector;

typedef struct PsiSVector {
	int16_t x, y, z, p;
} PsiSVector;

typedef char PsiTexture[ 32 ];

typedef struct PsiMeshM {
	PsiVector *vertices;
	PsiVector *normals;
} PsiMeshM;

typedef struct PsiMesh {
	uint32_t v;
	uint32_t numVertices;

	uint32_t n;
	uint32_t numNormals;

	uint32_t scale;

	char name[ 16 ];

	uint32_t childMeshOffs;
	uint32_t nextMeshOffs;

	uint16_t numScaleKeys;
	uint16_t numMoveKeys;
	uint16_t numRotateKeys;
	uint16_t padding0;

	uint32_t scaleKeysOffs;
	uint32_t moveKeysOffs;
	uint32_t rotateKeysOffs;

	uint16_t sortListSize[ 8 ];
	uint32_t sortListOffs[ 8 ];

	PsiSVector center;

	int16_t lastScaleKey;
	int16_t lastMoveKey;
	int16_t lastRotateKey;
	int16_t padding1;

	PsiMeshM memory;
} PsiMesh;
PL_STATIC_ASSERT( sizeof( PsiMesh ) - sizeof( PsiMeshM ) == 128, "Invalid struct size!" );

// mesh 2 is at 11212

typedef struct PsiModel {
	PsiHeader header;
	PsiTexture *textures;
	PsiMesh *meshes;
} PsiModel;

#if !defined( NDEBUG )
#	define DPRINT( ... ) printf( __VA_ARGS__ )
#else
#	define DPRINT( ... )
#endif

static PsiModel *ParseModel( PLFile *file ) {
	PsiHeader header = {};
	gInterface->ReadFile( file, &header, sizeof( PsiHeader ), 1 );

	if ( header.magic != PSI_MAGIC ) {
		return NULL;
	}

	PsiTexture *textures = gInterface->CAlloc( header.numTextures, sizeof( PsiTexture ), true );
	gInterface->FileSeek( file, ( PLFileOffset ) header.textureOffset, PL_SEEK_SET );
	for ( unsigned int i = 0; i < header.numTextures; ++i ) {
		PLFileOffset offs = gInterface->GetFileOffset( file );
		gInterface->ReadFile( file, &textures[ i ], sizeof( PsiTexture ), 1 );
		DPRINT( "TEXTURE: (%d) %s\n", offs, textures[ i ] );
	}

	PsiMesh *meshes = gInterface->CAlloc( header.numMeshes, sizeof( PsiMesh ), true );
	gInterface->FileSeek( file, ( PLFileOffset ) header.meshOffset, PL_SEEK_SET );
	for ( unsigned int i = 0; i < header.numMeshes; ++i ) {
		PLFileOffset offs = gInterface->GetFileOffset( file );
		gInterface->ReadFile( file, &meshes[ i ], sizeof( PsiMesh ) - sizeof( PsiMeshM ), 1 );
		DPRINT( "MESH: (%d) %s %u %u\n", offs,
		        meshes[ i ].name,
		        meshes[ i ].numVertices,
		        meshes[ i ].nextMeshOffs );

		gInterface->FileSeek( file, ( PLFileOffset ) ( offs + meshes[ i ].childMeshOffs ), PL_SEEK_SET );
		//TODO: handle next mesh offs
	}

	PsiModel *model = gInterface->MAlloc( sizeof( PsiModel ), true );
	model->header = header;
	model->meshes = meshes;
	model->textures = textures;

	return model;
}

PsiModel *Blitz_Format_Psi_LoadFile( const char *path ) {
	PLFile *file = gInterface->OpenFile( path, false );
	if ( file == NULL ) {
		return NULL;
	}

	PsiModel *model = ParseModel( file );

	gInterface->CloseFile( file );

	return model;
}

void Blitz_Format_Psi_ConvertModelCommand( unsigned int argc, char **argv ) {
	const char *inputPath = argv[ 1 ];

	PsiModel *model = Blitz_Format_Psi_LoadFile( inputPath );
	if ( model == NULL ) {
		return;
	}
}
