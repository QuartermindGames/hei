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

#include "filesystem_private.h"
#include "model_private.h"

#define SMD_VERSION     1
#define SMD_MAX_NODES   4096

typedef struct SMDNode {
	int id;
	char boneName[ 64 ];
	int parentId;
} SMDNode;

/*
typedef struct SMDSkeleton {

} SMDSkeleton;

typedef struct SMDTriangle {

} SMDTriangle;
*/

static SMDNode *SMD_ReadNodes( PLFile *file, unsigned int *numNodes ) {
	*numNodes = 0;

	char buffer[ 256 ];
	if ( plReadString( file, buffer, sizeof( buffer ) ) == NULL ) {
		return NULL;
	}

	/* ensure that it starts with 'nodes' */
	if ( strcmp( buffer, "nodes\n" ) != 0 ) {
		return NULL;
	}

	/* now read through and figure out how many nodes there are */
	size_t pos = plGetFileOffset( file );
	while( plReadString( file, buffer, sizeof( buffer ) ) != NULL ) {
		if ( strcmp( buffer, "end\n" ) == 0 ) {
			break;
		}

		numNodes++;
	}

	if ( *numNodes == 0 ) {
		return NULL;
	}

	/* now set us back to the start of the list */
	plFileSeek( file, pos, PL_SEEK_SET );

	/* allocate our nodes list */
	SMDNode *nodes = pl_malloc( *numNodes * sizeof( SMDNode ) );
	SMDNode *curNode = nodes;
	while( plReadString( file, buffer, sizeof( buffer ) ) != NULL ) {
		if ( strcmp( buffer, "end\n" ) == 0 ) {
			break;
		}

		if ( sscanf( buffer, "%d %s %d\n", &curNode->id, curNode->boneName, &curNode->parentId ) != 3 ) {

		}

		curNode++;
	}
}

static PLModel *SMD_ReadFile( PLFile *file ) {
	char buffer[ 256 ];
	if ( plReadString( file, buffer, sizeof( buffer ) ) == NULL ) {
		return NULL;
	}

	/* expect the version string first */
	unsigned int version = 0;
	sscanf( buffer, "version %d\n", &version );
	if ( version != SMD_VERSION ) {
		ReportBasicError( PL_RESULT_FILEVERSION );
		return NULL;
	}

	/* read in the bones (aka, nodes) */
	unsigned int numNodes;
	SMDNode *nodes = SMD_ReadNodes( file, &numNodes );
	if ( nodes == NULL ) {
		ReportError( PL_RESULT_FILEREAD, "failed to read in nodes" );
		return NULL;
	}
}

/**
 * Loads an SMD model. These are stored as plain ASCII.
 */
PLModel *plLoadSmdModel( const char *path ) {
	PLFile *file = plOpenFile( path, false );
	if ( file == NULL ) {
		return NULL;
	}

#if 0
	const char *buf = ( char * ) plGetFileData( file );
	size_t length = plGetFileSize( file );

	PLModel *model = SMD_ParseBuffer( buf, length );
#else
	PLModel *model = SMD_ReadFile( file );
#endif

	plCloseFile( file );

	return model;
}

static void SMD_WriteVertex( FILE *fp, const PLVertex *vertex ) {
	/*               P X  Y  Z  NX NY NZ U  V */
	fprintf( fp, "0 %f %f %f %f %f %f %f %f\n",

	         vertex->position.x,
	         vertex->position.y,
	         vertex->position.z,

	         vertex->normal.x,
	         vertex->normal.y,
	         vertex->normal.z,

	         vertex->st[ 0 ].x,
	         vertex->st[ 0 ].y );
}

/* writes given model out to Valve's SMD model format */
bool plWriteSmdModel( PLModel *model, const char *path ) {
	FILE *fp_out = NULL;
	for ( unsigned int i = 0; i < model->num_levels; ++i ) {
		char full_path[ PL_SYSTEM_MAX_PATH ];
		if ( i > 0 ) {
			snprintf( full_path, sizeof( full_path ), "%s_%d.smd", path, i );
		} else {
			snprintf( full_path, sizeof( full_path ), "%s.smd", path );
		}
		fp_out = fopen( full_path, "w" );
		if ( fp_out == NULL ) {
			ReportError( PL_RESULT_FILEWRITE, plGetResultString( PL_RESULT_FILEWRITE ) );
			return false;
		}

		/* header */
		fprintf( fp_out, "version 1\n\n" );

		/* write out the nodes block */
		fprintf( fp_out, "nodes\n" );
		if ( model->type != PL_MODELTYPE_SKELETAL ) {
			/* write out a dummy bone! */
			fprintf( fp_out, "0 \"root\" -1\n" );
		} else {
			/* todo, revisit this so we're correctly connecting child/parent */
			for ( unsigned int j = 0; j < model->internal.skeletal_data.num_bones; ++j ) {
				fprintf( fp_out, "%u %s %d\n", j, model->internal.skeletal_data.bones[ j ].name, ( int ) j - 1 );
			}
		}
		fprintf( fp_out, "end\n\n" );

		/* skeleton block */
		fprintf( fp_out, "skeleton\ntime 0\n" );
		if ( model->type != PL_MODELTYPE_SKELETAL ) {
			/* write out dummy bone coords! */
			fprintf( fp_out, "0 0 0 0 0 0 0\n" );
		} else {
			/* todo, print out default coords for each bone */
		}
		fprintf( fp_out, "end\n\n" );

		/* triangles block */
		fprintf( fp_out, "triangles\n" );
		PLModelLod *lod = plGetModelLodLevel( model, i );
		for ( unsigned int j = 0; j < lod->num_meshes; ++j ) {
			for ( unsigned int k = 0; k < lod->meshes[ j ]->num_indices; ) {
				if ( lod->meshes[ j ]->texture == NULL ) {
					fprintf( fp_out, "null\n" );
				} else {
					fprintf( fp_out, "%s\n", lod->meshes[ j ]->texture->name );
				}
				SMD_WriteVertex( fp_out, &lod->meshes[ j ]->vertices[ lod->meshes[ j ]->indices[ k++ ] ] );
				SMD_WriteVertex( fp_out, &lod->meshes[ j ]->vertices[ lod->meshes[ j ]->indices[ k++ ] ] );
				SMD_WriteVertex( fp_out, &lod->meshes[ j ]->vertices[ lod->meshes[ j ]->indices[ k++ ] ] );
			}
		}
		/* and leave a blank line at the end, to keep studiomdl happy */
		fprintf( fp_out, "end\n\n\n" );
		_pl_fclose( fp_out );
	}
	return true;
}
