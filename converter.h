// Conversion functions
#ifndef CONVERTER
#define CONVERTER

// Main includes
// #include <cstdio>
// #include <cstring>
// #include <cstdlib>

/* This header in the original source aliases the following:
 * <stdio.h> -> strcmp, strcpy, memcpy, memset
 * <stdlib.h> -> ???
 * Function to clamp sint32 -> sshort16
 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// 32-bit signed int -> saturated 16 bit signed int
// Clamps 32-bit signed int to range -32768 until 32767
static __inline short clampInt32ToShort16(int signedInt) {
	// Bitmask first 17 bits, excluding first.
	int a = signedInt & 0x7FFF8000;
	
	// Bitmask last 15 bits.
	short b = signedInt & 0x7FFF;
	
	if (signedInt < 0) {
		// Negative case
		if ( a == 0x7FFF8000 ) {
			// Return number if number > -32768
			return 0x8000 | b;
		} else {
			// Return this otherwise
			return -32768;
		}
	} else {
		// Positive case
		if ( a == 0x00000000 ) {
			// Return number if number < 32767
			return b;
		} else {
			// Return this otherwise
			return 32767;
		}
	}
}

int lib_strcmp( const char*, const char* );
char* lib_strcpy( char*, const char* );
void* lib_memcpy( void*, void*, int );
void* lib_memset( void *, int, int );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

