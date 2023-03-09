// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2023 Mark E Sowden <hogsy@oldtimes-software.com>

#include <plcore/pl_parse.h>

#include "plm_private.h"

#define PLY_MAX_PROPERTIES 16
#define PLY_MAX_ELEMENTS   16

typedef char PLYName[ 16 ];

typedef struct PLYProperty {
	enum {
		PLY_PROPERTY_TYPE_INVALID,

		PLY_PROPERTY_TYPE_CHAR,
		PLY_PROPERTY_TYPE_UCHAR,
		PLY_PROPERTY_TYPE_SHORT,
		PLY_PROPERTY_TYPE_USHORT,
		PLY_PROPERTY_TYPE_INT,
		PLY_PROPERTY_TYPE_UINT,
		PLY_PROPERTY_TYPE_FLOAT,
		PLY_PROPERTY_TYPE_DOUBLE,

		PLY_MAX_PROPERTY_TYPES
	} type;
	union {
		int8_t tchar;
		uint8_t tuchar;
		int16_t tshort;
		uint16_t tushort;
		int32_t tint;
		uint32_t tuint;
		float tfloat;
		double tdouble;
	} data;
	PLYName name;// name of the property
} PLYProperty;

typedef struct PLYElement {
	PLYName name;                                // name of the element
	unsigned int num;                            // number of this element in the file
	PLYProperty properties[ PLY_MAX_PROPERTIES ];// properties
	unsigned int numProperties;                  // number of properties for this element
} PLYElement;

unsigned int PropertyTypeForToken( const char *token ) {
	if ( strcmp( token, "char" ) == 0 ) {
		return PLY_PROPERTY_TYPE_CHAR;
	} else if ( strcmp( token, "uchar" ) == 0 ) {
		return PLY_PROPERTY_TYPE_UCHAR;
	} else if ( strcmp( token, "short" ) == 0 ) {
		return PLY_PROPERTY_TYPE_SHORT;
	} else if ( strcmp( token, "ushort" ) == 0 ) {
		return PLY_PROPERTY_TYPE_USHORT;
	} else if ( strcmp( token, "int" ) == 0 ) {
		return PLY_PROPERTY_TYPE_INT;
	} else if ( strcmp( token, "uint" ) == 0 ) {
		return PLY_PROPERTY_TYPE_UINT;
	} else if ( strcmp( token, "float" ) == 0 ) {
		return PLY_PROPERTY_TYPE_FLOAT;
	} else if ( strcmp( token, "double" ) == 0 ) {
		return PLY_PROPERTY_TYPE_DOUBLE;
	} else {
		return PLY_PROPERTY_TYPE_INVALID;
	}
}

static PLMModel *ParsePly( const char *buf ) {
	const char *p = buf;

	PLYElement elements[ PLY_MAX_ELEMENTS ];
	PL_ZERO( elements, sizeof( PLYElement ) * PLY_MAX_ELEMENTS );
	int numElements = -1;

	PLYElement *element = NULL;

	// this reads in the header block *after* the format line
	while ( *p != '\0' ) {
		char token[ 32 ];
		if ( PlParseToken( &p, token, sizeof( token ) ) == NULL ) {
			break;
		}

		if ( strcmp( token, "element" ) == 0 ) {
			numElements++;
			PlParseToken( &p, elements[ numElements ].name, sizeof( PLYName ) );
			elements[ numElements ].num = PlParseInteger( &p, NULL );
			element = &elements[ numElements ];
		} else if ( strcmp( token, "property" ) == 0 ) {
			if ( element == NULL ) {
				PlReportErrorF( PL_RESULT_FILEERR, "property with no element" );
				PlSkipLine( &p );
				continue;
			}

			element->properties[ element->numProperties ].type = PropertyTypeForToken( PlParseToken( &p, token, sizeof( token ) ) );
			snprintf( element->properties[ element->numProperties ].name, sizeof( PLYName ), "%s", PlParseToken( &p, token, sizeof( token ) ) );
		} else if ( strcmp( token, "end_header" ) == 0 ) {
			PlSkipLine( &p );
			break;
		}

		PlSkipLine( &p );
	}

	if ( *p == '\0' ) {
		PlReportErrorF( PL_RESULT_FILEERR, "unexpected end of ply" );
		return NULL;
	}
}

PLMModel *PlmDeserializePly( PLFile *file ) {
	char token[ 32 ];
	// first just validate it's a 'ply' file
	PlReadString( file, token, sizeof( token ) );
	if ( strncmp( token, "ply", 3 ) != 0 ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "unexpected file type" );
		return NULL;
	}
	// and now confirm the format
	PlReadString( file, token, sizeof( token ) );
	if ( strncmp( token, "format ascii 1.0", 16 ) != 0 ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "unexpected ply format" );
		return NULL;
	}

	// now let's load the whole damn thing into memory
	size_t size = PlGetFileSize( file );
	char *buf = PL_NEW_( char, size + 1 );
	PlReadFile( file, buf, sizeof( char ), size );

	// parse through the buffer
	PLMModel *model = ParsePly( buf );

	// and now cleanup
	PL_DELETE( buf );

	return model;
}
