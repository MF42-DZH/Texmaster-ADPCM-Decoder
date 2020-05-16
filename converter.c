#ifdef __cplusplus
// #include "converter.h"

extern "C" {
#endif // __cplusplus

int lib_strcmp( char* c1, char* c2 ){
	while( 1 ){

		if( *c1 > *c2 ){
			return -1;
		}
		if( *c1 < *c2 ){
			return 1;
		}
		if( *c1 == '\0' && *c2 == '\0' ){
			return 0;
		}
		if( *c1 == '\0' && *c2 != '\0' ){
			return -1;
		}
		if( *c1 != '\0' && *c2 == '\0' ){
			return -1;
		}
		++c1;
		++c2;
	}
}

char* lib_strcpy( char* dst, const char* src ){
	while( *src != '\0' ){
		*dst = *src;
		++dst;
		++src;
	}
	*dst = '\0';
	return dst;
}

void* lib_memcpy( void* dst, void* src, int count ){
	char* c1 = (char*)dst;
	char* c2 = (char*)src;
	while( count > 0 ){
		*c1 = *c2;
		--count;
		++c1;
		++c2;
	}
	return dst;
}

void* lib_memset( void* dst, int c, int count ){
	union {
		int i32;
		char i8;
	} _cvt;
	char* c1 = (char*)dst;
	_cvt.i32 = c;
	while( count > 0 ){
		*c1 = _cvt.i8;
		--count;
		++c1;
	}
	return dst;
}

#ifdef __cplusplus
}
#endif // __cplusplus

