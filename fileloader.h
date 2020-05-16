// File loader class header.
#ifndef FILELOADER
#define FILELOADER

#include <cstdio>
#include <cstdlib>
#include <cstring>

// Includes
extern "C" {
	#include "converter.h"
}

// Macros
#define MAX_FILE_NUMBER 0x100

enum {
	TYPE_NORMAL,
	TYPE_SDL_SURFACE,  // Kept this here just in case the enum breaks.
	TYPE_NUM
};

struct FileUnit {
	char tag[0x100];  // 256-long c-string that holds file type.
	char* ptr;        // Holds the data bytes as an array.
	int length;       // Length of file in bytes.
	int type;         // Type of data. (ADPCM or SDL_SURFACE data).
};

class Loader {
	protected:
		FileUnit _data[MAX_FILE_NUMBER];  // File data objects.
		bool _valid;  // File validity check boolean.
	public:
		// Constructor and Destructor
		Loader();   // Default constructor
		~Loader();  // Default destructor

		// Member functions
		bool load(const char* fileName, const char* tag, int type = 0);  // Load file?
		void release();            // Release memory held by class
		char* getPtr(char* tag);   // Get data
		int getLength(char* tag);  // Get length of file
};

#endif

