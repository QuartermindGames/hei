/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

unsigned int pl_strcnt( const char *s, char c ) {
	unsigned int num = 0;
	for ( unsigned int i = 0; s[ i ] != '\0'; ++i ) {
		if ( s[ i ] != c ) {
			continue;
		}

		num++;
	}
	return num;
}

unsigned int pl_strncnt( const char *s, char c, unsigned int n ) {
	unsigned int num = 0;
	for ( unsigned int i = 0; i < n; ++i ) {
		if ( s[ i ] == '\0' ) {
			break;
		}

		if ( s[ i ] != c ) {
			continue;
		}

		num++;
	}
	return num;
}
