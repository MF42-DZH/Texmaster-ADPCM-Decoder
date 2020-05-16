#include "fileloader.h"

extern "C" {
	#include "converter.h"
}

// Default constructor
Loader::Loader() {
	// Initialise all chunks with empty data.
	for (int i = 0; i < MAX_FILE_NUMBER; i++) {
		_data[i].ptr = NULL;
		_data[i].type = TYPE_NUM;
		_data[i].tag[0] = '\0';
	}

	// As of now, the file is still valid.
	_valid = true;
}

// Default destructor
Loader::~Loader() {
	for (int i = 0; i < MAX_FILE_NUMBER; i++) {
		// Free all collected pointers.
		if (_data[i].type == TYPE_NORMAL) free(_data[i].ptr);
		
		// Hopefully the SDL surfaces won't bite me in the ass.
	}
}

// Load file
bool Loader::load(const char* filename, const char* tag, int type) {
	int id = 0;
	for (int i = 0; i < MAX_FILE_NUMBER; i++) {
		id = i;
		if (_data[i].ptr == 0) {
			if (type == TYPE_NORMAL) {
				FILE* fp = NULL;
				int length;
				char* ptr = NULL;
				
				// Open file in binary-read mode.
				fp = fopen(filename, "rb");
				if (!fp) return false;  // False if file fails to be read.
				
				// Find length of file.
				fseek(fp, 0, SEEK_END);
				length = ftell(fp);
				fseek(fp, 0, SEEK_SET);  // Put back to start.

				ptr = new char[length];  // Set up buffer.

				// Read all bytes to ptr and close.
				fread(ptr, 1, length, fp);
				fclose(fp);

				_data[id].ptr = ptr;  // Allocate bytes to file.
				lib_strcpy(_data[id].tag, tag);  // Allocate type of data.
				_data[id].length = length;   // Allocate data length.
				_data[id].type = TYPE_NORMAL;
			}

			return true;  // File read successful.
		}
	}

	_valid = false;  // File read unsuccessful.
	return false;
}

// Release all memory held by the loader.
void Loader::release() {
	for (int i = 0; i < MAX_FILE_NUMBER; i++) {
		if (_data[i].ptr) {
			free(_data[i].ptr);      // Release all memory
			_data[i].ptr = NULL;     // Set pointers to null pointers
			_data[i].tag[0] = '\0';  // Terminate all tag names
		}
	}
}

// Fetch file data
char* Loader::getPtr(char* tag) {
	for (int i = 0; i < MAX_FILE_NUMBER; i++) {
		if (lib_strcmp(_data[i].tag, tag) == 0) return _data[i].ptr;
	}

	return NULL;
}

// Fetch file length
int Loader::getLength(char* tag) {
	for (int i = 0; i < MAX_FILE_NUMBER; i++) {
		if (lib_strcmp(_data[i].tag, tag) == 0) return _data[i].length;
	}

	return 0;
}

