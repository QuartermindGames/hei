// SPDX-License-Identifier: MIT
// Copyright © 2021-2023 Mark E Sowden <hogsy@oldtimes-software.com>

#include "plm_private.h"

/************************************************************/
/* MTL Format */

/* todo */

/************************************************************/
/* Obj Static Model Format */

typedef struct ObjVectorLst {
	PLVector3 v;
	struct ObjVectorLst *next;
} ObjVectorLst;

typedef struct ObjFaceLst {
	PLGVertex *vertices;
	unsigned int num_vertices;
	char mtl_name[ 64 ];
	struct ObjFaceLst *next;
} ObjFaceLst;

typedef struct ObjHandle {
	ObjVectorLst *vertex_normals;
	ObjVectorLst *vertex_positions;
	ObjVectorLst *vertex_tex_coords;
	ObjFaceLst *faces;
	char mtllib_path[ PL_SYSTEM_MAX_PATH ];
} ObjHandle;

static void FreeObjHandle( ObjHandle *obj ) {
	PlFree( obj );
}

static ObjVectorLst *GetVectorIndex( ObjVectorLst *start, unsigned int idx ) {
	ObjVectorLst *cur = start;
	for ( unsigned int i = 0; i < idx - 1; ++i ) {
		if ( cur->next == NULL ) {
			ModelLog( "Invalid vector index (%d)!\n", i );
			return NULL;
		}

		cur = cur->next;
	}

	return cur;
}

PLMModel *PlmLoadObjModel( const char *path ) {
	PLFile *fp = PlOpenFile( path, false );
	if ( fp == NULL ) {
		return NULL;
	}

	ObjHandle *obj = PlMAllocA( sizeof( ObjHandle ) );

	ObjVectorLst **cur_v = &( obj->vertex_positions );
	ObjVectorLst **cur_vn = &( obj->vertex_normals );
	ObjVectorLst **cur_vt = &( obj->vertex_tex_coords );
	*cur_v = *cur_vn = *cur_vt = NULL;

	ObjFaceLst **cur_face = &( obj->faces );
	*cur_face = NULL;

	char tk[ 256 ];
	while ( PlReadString( fp, tk, sizeof( tk ) ) != NULL ) {
		if ( tk[ 0 ] == '\0' || tk[ 0 ] == '#' || tk[ 0 ] == 'o' || tk[ 0 ] == 'g' || tk[ 0 ] == 's' ) {
			continue;
		} else if ( tk[ 0 ] == 'v' && tk[ 1 ] == ' ' ) { /* vertex position */
			ObjVectorLst *this_v = PlMAllocA( sizeof( ObjVectorLst ) );
			unsigned int n = sscanf( &tk[ 2 ], "%f %f %f", &this_v->v.x, &this_v->v.y, &this_v->v.z );
			if ( n < 3 ) {
				ModelLog( "Invalid vertex position, less than 3 coords!\n\"%s\"\n", tk );
			} else if ( n > 3 ) {
				ModelLog( "Ignoring fourth vertex position parameter, unsupported!\n\"%s\"\n", tk );
			}

			*cur_v = this_v;
			this_v->next = NULL;
			cur_v = &( this_v->next );
			continue;
		} else if ( tk[ 0 ] == 'v' && tk[ 1 ] == 't' ) { /* vertex texture */
			ObjVectorLst *this_vt = PlMAllocA( sizeof( ObjVectorLst ) );
			unsigned int n = sscanf( &tk[ 2 ], "%f %f", &this_vt->v.x, &this_vt->v.y );
			if ( n < 2 ) {
				ModelLog( "Invalid vertex uv, less than 2 coords!\n\"%s\"\n", tk );
			} else if ( n > 2 ) {
				ModelLog( "Ignoring third uv parameter, unsupported!\n\"%s\"\n", tk );
			}

			*cur_vt = this_vt;
			this_vt->next = NULL;
			cur_vt = &( this_vt->next );
			continue;
		} else if ( tk[ 0 ] == 'v' && tk[ 1 ] == 'n' ) { /* vertex normal */
			ObjVectorLst *this_vn = PlMAllocA( sizeof( ObjVectorLst ) );
			if ( sscanf( &tk[ 2 ], "%f %f %f", &this_vn->v.x, &this_vn->v.y, &this_vn->v.z ) < 3 ) {
				ModelLog( "Invalid vertex normal, less than 3 coords!\n\"%s\"\n", tk );
			}

			*cur_vn = this_vn;
			this_vn->next = NULL;
			cur_vn = &( this_vn->next );
			continue;
		} else if ( tk[ 0 ] == 'f' && tk[ 1 ] == ' ' ) { /* face */
			int idx[ 64 ];
			unsigned int num_elements = 1;

			/* faces are kind of a special case, so we'll need
             * to parse them manually here */
			char *pos = tk;
			while ( *pos != '\0' && *pos != '\n' && *pos != '\r' ) {
				if ( *pos == ' ' ) {
					continue;
				}

				pos++;
			}

			ObjFaceLst *this_face = PlMAllocA( sizeof( ObjFaceLst ) );

			*cur_face = this_face;
			this_face->next = NULL;
			cur_face = &( this_face->next );
			continue;
		}

		ModelLog( "Unknown/unsupported parameter '%s', ignoring!\n", tk[ 0 ] );
	}

	PlCloseFile( fp );

	/* right we're finally done, time to see what we hauled... */

	return NULL;
}

static void WriteVertices( FILE *file, const PLGMesh *mesh ) {
	fprintf( file, "# %u vertices\n", mesh->num_verts );
	for ( unsigned int j = 0; j < mesh->num_verts; ++j ) {
		fprintf( file, "v %s\n", PlPrintVector3( &mesh->vertices[ j ].position, PL_VAR_F32 ) );
	}
	for ( unsigned int j = 0; j < mesh->num_verts; ++j ) {
		fprintf( file, "vt %s\n", PlPrintVector2( &mesh->vertices[ j ].st[ 0 ], PL_VAR_F32 ) );
	}
	for ( unsigned int j = 0; j < mesh->num_verts; ++j ) {
		fprintf( file, "vn %s\n", PlPrintVector3( &mesh->vertices[ j ].normal, PL_VAR_F32 ) );
	}
}

static void WriteFaces( FILE *file, const PLGMesh *mesh ) {
	if ( mesh->primitive == PLG_MESH_TRIANGLES ) {
		fprintf( file, "# %u faces\n", mesh->num_triangles );
		const unsigned int *index = mesh->indices;
		for ( unsigned int j = 0; j < mesh->num_triangles; j++, index += 3 ) {
			/* this was broken for a long time; these start at 1 for obj, not 0!
			 * the moral of the story is, read!! */
			uint32_t x = index[ 0 ] + 1;
			uint32_t y = index[ 1 ] + 1;
			uint32_t z = index[ 2 ] + 1;
			fprintf( file, "f %u/%u/%u %u/%u/%u %u/%u/%u\n", x, x, x, y, y, y, z, z, z );
		}
	}
}

static void WriteObjectHeader( FILE *file, const char *name, const PLMModel *model, const PLGMesh *mesh ) {
	fprintf( file, "o %s\n", name );
	if ( mesh->texture != NULL && mesh->texture->name[ 0 ] != '\0' ) {
		fprintf( file, "usemtl %s\n", mesh->texture->name );
	} else if ( model->numMaterials > 0 ) {
		fprintf( file, "usemtl %s\n", model->materials[ mesh->materialIndex ] );
	}
}

bool PlmWriteObjModel( PLMModel *model, const char *path ) {
	FILE *fp = fopen( path, "w" );
	if ( fp == NULL ) {
		PlReportBasicError( PL_RESULT_FILEWRITE );
		return false;
	}

	fprintf( fp, "# generated by hei platform lib (https://github.com/TalonBraveInfo/platform)\n" );
	if ( model->type == PLM_MODELTYPE_SKELETAL ) {
		ModelLog( "Model is of type skeletal; Obj only supports static models so skeleton will be discarded...\n" );
	}

	if ( model->numMaterials > 0 ) {
		for ( unsigned int i = 0; i < model->numMaterials; ++i ) {
			fprintf( fp, "mtllib %s.mtl\n", model->materials[ i ] );
		}
	} else {
		PLPath mtl_name;
		snprintf( mtl_name, strlen( PlGetFileName( path ) ) - 4, "%s", PlGetFileName( path ) );
		fprintf( fp, "mtllib %s.mtl\n", mtl_name );
	}

	if ( model->type == PLM_MODELTYPE_VERTEX ) {
		const PLGMesh *mesh = model->meshes[ 0 ];
		PLVector3 *vertices = PL_NEW_( PLVector3, mesh->num_verts );

		unsigned int numFrames;
		const PLMVertexAnimationFrame *frames = PlmGetVertexAnimationFrames( model, &numFrames );
		for ( unsigned int i = 0; i < numFrames; ++i ) {
			for ( unsigned int j = 0; j < mesh->num_verts; ++j ) {
				vertices[ j ] = mesh->vertices[ j ].position;
			}
			for ( unsigned int j = 0; j < frames[ i ].numTransforms; ++j ) {
				vertices[ frames[ i ].transforms[ j ].index ] = frames[ i ].transforms[ j ].position;
			}

			char tmp[ 64 ];
			snprintf( tmp, sizeof( tmp ), "%s.frame%0u\n", ( *model->name != '\0' ) ? model->name : "mesh", i );
			WriteObjectHeader( fp, tmp, model, mesh );

			fprintf( fp, "# %u vertices\n", mesh->num_verts );
			for ( unsigned int j = 0; j < mesh->num_verts; ++j ) {
				fprintf( fp, "v %s\n", PlPrintVector3( &vertices[ j ], PL_VAR_F32 ) );
			}
			for ( unsigned int j = 0; j < mesh->num_verts; ++j ) {
				fprintf( fp, "vt %s\n", PlPrintVector2( &mesh->vertices[ j ].st[ 0 ], PL_VAR_F32 ) );
			}
			for ( unsigned int j = 0; j < mesh->num_verts; ++j ) {
				fprintf( fp, "vn %s\n", PlPrintVector3( &mesh->vertices[ j ].normal, PL_VAR_F32 ) );
			}
			WriteFaces( fp, mesh );
		}
		PL_DELETE( vertices );
	} else {
		for ( unsigned int i = 0; i < model->numMeshes; ++i ) {
			PLGMesh *mesh = model->meshes[ i ];

			char tmp[ 64 ];
			snprintf( tmp, sizeof( tmp ), "%s.%0u\n", ( *model->name != '\0' ) ? model->name : "mesh", i );
			WriteObjectHeader( fp, tmp, model, mesh );
			WriteVertices( fp, mesh );
			WriteFaces( fp, mesh );
		}
	}

	fclose( fp );

	return true;
}
