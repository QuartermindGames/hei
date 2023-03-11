// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2023 Mark E Sowden <hogsy@oldtimes-software.com>

#include <plcore/pl_parse.h>

#include "plm_private.h"

/**
 * PLY Loader based on information from the following sources...
 * 	http://paulbourke.net/dataformats/ply/
 * 	https://en.wikipedia.org/wiki/PLY_(file_format)
 *
 * Slight disclaimer that this doesn't currently support everything
 * the format provides, mostly for the obvious reasoning that we're
 * loading the format to convert into *our* own format and don't
 * expose any of this to the end user. As a matter of fact, this
 * probably does more than it needs to...
 */

#define PLY_MAX_PROPERTIES 16
#define PLY_MAX_ELEMENTS   16

typedef char PLYName[ 32 ];

// the way these structures are set up could be improved,
// most of the complexity just comes in because of 'list'

typedef enum PLYPropertyType {
	PLY_PROPERTY_TYPE_INVALID,

	PLY_PROPERTY_TYPE_CHAR,
	PLY_PROPERTY_TYPE_UCHAR,
	PLY_PROPERTY_TYPE_SHORT,
	PLY_PROPERTY_TYPE_USHORT,
	PLY_PROPERTY_TYPE_INT,
	PLY_PROPERTY_TYPE_UINT,
	PLY_PROPERTY_TYPE_FLOAT,
	PLY_PROPERTY_TYPE_DOUBLE,
	PLY_PROPERTY_TYPE_LIST,

	PLY_MAX_PROPERTY_TYPES
} PLYPropertyType;

typedef struct PLYData {
	union {
		int8_t tchar;
		uint8_t tuchar;
		int16_t tshort;
		uint16_t tushort;
		int32_t tint;
		uint32_t tuint;
		float tfloat;
		double tdouble;
		struct PLYData *data;
	};
	PLYPropertyType type;
} PLYData;

typedef struct PLYProperty {
	PLYPropertyType type, subType;// subtype only matters for lists
	PLYName name;                 // name of the property
} PLYProperty;

typedef struct PLYElement {
	PLYName name;                                // name of the element
	unsigned int num;                            // number of this element in the file
	PLYProperty properties[ PLY_MAX_PROPERTIES ];// properties
	unsigned int numProperties;                  // number of properties for this element

	PLYData *data;
	unsigned int dataLength;
} PLYElement;

unsigned int PropertyTypeForToken( const char *token ) {
	if ( strcmp( token, "list" ) == 0 ) {
		return PLY_PROPERTY_TYPE_LIST;
	} else if ( strcmp( token, "char" ) == 0 ) {
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

static const PLYData *ParseData( const char **p, PLYData *data ) {
	switch ( data->type ) {
		case PLY_PROPERTY_TYPE_CHAR:
			data->tchar = ( int8_t ) PlParseInteger( p, NULL );
			return data;
		case PLY_PROPERTY_TYPE_UCHAR:
			data->tuchar = ( uint8_t ) PlParseInteger( p, NULL );
			return data;
		case PLY_PROPERTY_TYPE_SHORT:
			data->tshort = ( int16_t ) PlParseInteger( p, NULL );
			return data;
		case PLY_PROPERTY_TYPE_USHORT:
			data->tushort = ( uint16_t ) PlParseInteger( p, NULL );
			return data;
		case PLY_PROPERTY_TYPE_INT:
			data->tint = PlParseInteger( p, NULL );
			return data;
		case PLY_PROPERTY_TYPE_UINT:
			data->tuint = ( unsigned ) PlParseInteger( p, NULL );
			return data;
		case PLY_PROPERTY_TYPE_FLOAT:
			data->tfloat = PlParseFloat( p, NULL );
			return data;
		case PLY_PROPERTY_TYPE_DOUBLE:
			data->tdouble = PlParseDouble( p, NULL );
			break;
		default:
			break;
	}

	return NULL;
}

static const PLYElement *LookupElementByName( const char *name, const PLYElement *elements, unsigned int numElements ) {
	for ( unsigned int i = 0; i < numElements; ++i ) {
		if ( strcmp( elements[ i ].name, name ) == 0 ) {
			return &elements[ i ];
		}
	}
	return NULL;
}

static const PLYProperty *LookupPropertyByName( const char *name, const PLYElement *element ) {
	for ( unsigned int i = 0; i < element->numProperties; ++i ) {
		if ( strcmp( element->properties[ i ].name, name ) == 0 ) {
			return &element->properties[ i ];
		}
	}
	return NULL;
}

static PLYElement elements[ PLY_MAX_ELEMENTS ];
static unsigned int numElements;

static bool ParsePlyHeader( const char **p ) {
	// this reads in the header block *after* the format line,
	// and provides some additional validation on the way
	bool headerEnd = false;
	PLYElement *element = NULL;
	while ( **p != '\0' && !headerEnd ) {
		char token[ 32 ];
		if ( PlParseToken( p, token, sizeof( token ) ) == NULL ) {
			break;
		}

		if ( strcmp( token, "element" ) == 0 ) {
			if ( numElements + 1 >= PLY_MAX_ELEMENTS ) {
				PlReportErrorF( PL_RESULT_MEMORY_EOA, "exceeded max element limit" );
				return false;
			}

			element = &elements[ numElements ];

			if ( PlParseToken( p, element->name, sizeof( PLYName ) ) == NULL ) {
				PlReportErrorF( PL_RESULT_FILEERR, "element with no name" );
				return false;
			}
			element->num = PlParseInteger( p, NULL );

			numElements++;
		} else if ( strcmp( token, "property" ) == 0 ) {
			if ( element == NULL ) {
				PlReportErrorF( PL_RESULT_FILEERR, "property with no element" );
				return false;
			}

			if ( element->numProperties + 1 >= PLY_MAX_PROPERTIES ) {
				PlReportErrorF( PL_RESULT_MEMORY_EOA, "exceeded max property limit" );
				return false;
			}

			PLYProperty *property = &element->properties[ element->numProperties ];

			// type
			if ( PlParseToken( p, token, sizeof( token ) ) == NULL ) {
				PlReportErrorF( PL_RESULT_FILEERR, "property with no type" );
				return false;
			}
			property->type = PropertyTypeForToken( token );
			if ( property->type == PLY_PROPERTY_TYPE_INVALID ) {
				PlReportErrorF( PL_RESULT_FILEERR, "property with unsupported/invalid type" );
				return false;
			} else if ( property->type == PLY_PROPERTY_TYPE_LIST ) {
				// for lists, there's an additional type we need to read in
				if ( PlParseToken( p, token, sizeof( token ) ) == NULL ) {
					PlReportErrorF( PL_RESULT_FILEERR, "property list with no subtype" );
					return false;
				}
				property->subType = PropertyTypeForToken( token );
				if ( property->subType == PLY_PROPERTY_TYPE_INVALID || property->subType == PLY_PROPERTY_TYPE_LIST ) {
					PlReportErrorF( PL_RESULT_FILEERR, "property list with unsupported/invalid subtype" );
					return false;
				}
			}

			// name
			if ( PlParseToken( p, token, sizeof( token ) ) == NULL ) {
				PlReportErrorF( PL_RESULT_FILEERR, "property with no name" );
				return false;
			}
			snprintf( property->name, sizeof( PLYName ), "%s", token );

			element->numProperties++;
		} else if ( strcmp( token, "end_header" ) == 0 ) {
			headerEnd = true;
		}

		// this is probably going to bite later,
		// but we'll just ignore anything we don't handle for now
		PlSkipLine( p );
	}

	if ( **p == '\0' ) {
		PlReportErrorF( PL_RESULT_FILEERR, "unexpected end of ply" );
		return false;
	}

	return true;
}

static PLMModel *ParsePly( const char *buf ) {
	const char *p = buf;

	PL_ZERO( elements, sizeof( PLYElement ) * PLY_MAX_ELEMENTS );
	numElements = 0;

	if ( !ParsePlyHeader( &p ) ) {
		return NULL;
	}

	// okay, now we can load in the actual data - this will be in the order the elements were provided

	PLYElement *vertexElement = LookupElementByName( "vertex", elements, numElements );
	if ( vertexElement == NULL ) {
		PlReportErrorF( PL_RESULT_FILEERR, "no vertex element in ply" );
		return NULL;
	}

	PLYElement *faceElement = LookupElementByName( "face", elements, numElements );
	if ( faceElement == NULL ) {
		PlReportErrorF( PL_RESULT_FILEERR, "no face element in ply" );
		return NULL;
	}

	// read in the remaining data now ...

	vertexElement->data = PL_NEW_( PLYData, sizeof( PLYData ) * ( vertexElement->numProperties * vertexElement->num ) );
	for ( unsigned int i = 0; i < vertexElement->num; ++i ) {
		for ( unsigned int j = 0; j < vertexElement->numProperties; ++j ) {
			PLYData *data = &vertexElement->data[ i + j ];
			data->type = vertexElement->properties[ j ].type;
			if ( ParseData( &p, data ) == NULL ) {
			}
		}
	}

	// cleanup
	for ( unsigned int i = 0; i < numElements; ++i ) {
		PL_DELETE( elements[ i ].data );
	}

	return NULL;
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
