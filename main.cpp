#include <iostream>
#include <string>
#include "adpcm.h"
#include "fileloader.h"
extern "C" {
	// For some <stdio.h> rebindings
	#include "converter.h"
}

#define USAGE_STRING "Usage: TXM-ADPCM-DECODE [ADPCM file; required] [Loop Point Sample Number; optional]"

// - TEXMASTER ADPCM DECODER - 0xFC963F18DC21 -
// The files always start with 4 bytes of empty space.

char* SOUND_ID = "ADPCM";
const unsigned int outputSampleRate = 44100;  // 44100 Hz output
const unsigned int outputChannels = 2;        // Stereo output

// File to output the PCM data into a RIFF WAVE file.
// Returns true if successful, returns false otherwise.
bool outputWAVFile(std::string fileName, int* stream, int length, unsigned int loopSNum) {
	// Data headers
	char riff_header[] = { 'R', 'I', 'F', 'F' };
	char wave_header[] = { 'W', 'A', 'V', 'E' };
	char fmt_header[] = { 'f', 'm', 't', ' ' };
	char data_header[] = { 'd', 'a', 't', 'a' };
	char cue_header[] = { 'c', 'u', 'e', ' ' };
	char smpl_header[] = { 's', 'm', 'p', 'l' };
	char null_term = '\0';  // For appending to sample data if in the unlikely case that it is not even-length.
	
	// File object
	FILE* file = NULL;
	
	unsigned short dataSize = 32;  // 32-bit
	unsigned short dataType = 1;   // PCM
	unsigned int blkSize = ((unsigned int)dataSize * outputChannels * outputSampleRate) / 8;
	unsigned short outputType = (dataSize * (unsigned short)outputChannels) / 8;
	unsigned int zero = 0;
	unsigned int sixteen = 16;
	unsigned int byteLength = length * 4;  // Length of stream in bytes
	
	// Open binary file for reading;
	file = fopen(fileName.c_str(), "wb");
	if (!file) {
		std::cout << "Output failed to open." << std::endl;
		return false;  // File failed to open.
	}
	
	// Write RIFF header
	fwrite(riff_header, sizeof(char), 4, file);
	fwrite(&zero, sizeof(unsigned int), 1, file);  // DUMMY VALUE
	
	if (ferror(file)) {
		std::cout << "An error has occurred in writing the RIFF header." << std::endl;
		fclose(file);
		return false;
	}
	
	// Write WAVE
	fwrite(wave_header, sizeof(char), 4, file);
	
	if (ferror(file)) {
		std::cout << "An error has occurred in writing the WAVE header." << std::endl;
		fclose(file);
		return false;
	}
	
	// Write "fmt " header
	fwrite(fmt_header, sizeof(char), 4, file);
	fwrite(&sixteen, sizeof(unsigned int), 1, file);
	fwrite(&dataType, sizeof(unsigned short), 1, file);
	fwrite(&outputChannels, sizeof(unsigned short), 1, file);
	fwrite(&outputSampleRate, sizeof(unsigned int), 1, file);
	fwrite(&blkSize, sizeof(unsigned int), 1, file);
	fwrite(&outputType, sizeof(unsigned short), 1, file);
	fwrite(&dataSize, sizeof(unsigned short), 1, file);
	
	if (ferror(file)) {
		std::cout << "An error has occurred in writing the \"fmt \" header." << std::endl;
		fclose(file);
		return false;
	}
	
	// Write data header
	fwrite(data_header, sizeof(char), 4, file);
	fwrite(&byteLength, sizeof(unsigned int), 1, file);
	
	if (ferror(file)) {
		std::cout << "An error has occurred in writing the data header." << std::endl;
		fclose(file);
		return false;
	}
	
	// Write data stream
	fwrite(stream, sizeof(int), length, file);
	if ((length & 1) != 0) fwrite(&null_term, sizeof(char), 1, file);
	
	if (ferror(file)) {
		std::cout << "An error has occurred in writing the data stream." << std::endl;
		fclose(file);
		return false;
	}
	
	// Loop point placement
	// Cue chunk write
	unsigned int cueSize = 28;  // 4 + 24
	
	/* POINT DATA */
	unsigned int POINTS = 1;
	unsigned int ID = 1;
	unsigned int POSITION = 0;
	// USE data_header
	unsigned int CHUNKSTART = 0;
	unsigned int BLOCKSTART = 0;
	// USE loopSNum
	
	fwrite(cue_header, sizeof(char), 4, file);
	fwrite(&cueSize, sizeof(unsigned int), 1, file);
	fwrite(&POINTS, sizeof(unsigned int), 1, file);
	fwrite(&ID, sizeof(unsigned int), 1, file);
	fwrite(&POSITION, sizeof(unsigned int), 1, file);
	
	if (ferror(file)) {
		std::cout << "An error has occurred in writing the cue header." << std::endl;
		fclose(file);
		return false;
	}
	
	fwrite(data_header, sizeof(char), 4, file);
	fwrite(&CHUNKSTART, sizeof(unsigned int), 1, file);
	fwrite(&BLOCKSTART, sizeof(unsigned int), 1, file);
	fwrite(&loopSNum, sizeof(int), 1, file);
	
	if (ferror(file)) {
		std::cout << "An error has occurred in writing the \"cue \" points." << std::endl;
		fclose(file);
		return false;
	}
	
	// -------------------------
	
	// SMPL Chunk Write
	unsigned int smplSize = 60;  // 36 + 24
	// MAN = 0 -- SEE: https://sites.google.com/site/musicgapi/technical-documents/wav-file-format#smpl
	// PRO = 0
	// SMP = 0
	// MUN = 0
	// MPF = 0
	// SFM = 0
	// SOS = 0
	unsigned int loops = 1;
	// SPD = 0
	
	/* Loop Point Info */
	unsigned int CPID = 1;
	unsigned int LTYPE = 0;
	// USE loopSNum
	unsigned int end = length / 2 - 1;
	unsigned int FRAC = 0;
	unsigned int AMT = 0;
	
	// Write SMPL header to set cue point as loop point
	fwrite(smpl_header, sizeof(char), 4, file);
	fwrite(&smplSize, sizeof(unsigned int), 1, file);
	fwrite(&zero, sizeof(unsigned int), 1, file);
	fwrite(&zero, sizeof(unsigned int), 1, file);
	fwrite(&zero, sizeof(unsigned int), 1, file);
	fwrite(&zero, sizeof(unsigned int), 1, file);
	fwrite(&zero, sizeof(unsigned int), 1, file);
	fwrite(&zero, sizeof(unsigned int), 1, file);
	fwrite(&zero, sizeof(unsigned int), 1, file);
	fwrite(&loops, sizeof(unsigned int), 1, file);
	fwrite(&zero, sizeof(unsigned int), 1, file);
	
	if (ferror(file)) {
		std::cout << "An error has occurred in writing the SMPL header." << std::endl;
		fclose(file);
		return false;
	}
	
	fwrite(&CPID, sizeof(unsigned int), 1, file);
	fwrite(&LTYPE, sizeof(unsigned int), 1, file);
	fwrite(&loopSNum, sizeof(int), 1, file);
	fwrite(&end, sizeof(int), 1, file);
	fwrite(&FRAC, sizeof(unsigned int), 1, file);
	fwrite(&AMT, sizeof(unsigned int), 1, file);
	
	if (ferror(file)) {
		std::cout << "An error has occurred in writing the loop points." << std::endl;
		fclose(file);
		return false;
	}
	// -------------------------------------------------------
	
	// Write length of file.
	fseek(file, 0, SEEK_END);
	unsigned int filelength = ftell(file) - 8;  // Subtract the length of "RIFF", not sure about the other 4 bytes.
	fseek(file, 4, SEEK_SET);  // Put back to start + 4
	fwrite(&filelength, sizeof(unsigned int), 1, file);  // Write length value
	
	if (ferror(file)) {
		std::cout << "An error has occurred in writing the file length." << std::endl;
		fclose(file);
		return false;
	}
	
	// Close file object
	fclose(file);
	return true;
}

// ADPCM file struct for holding metadata
struct AdpcmFile {
	int header;
	unsigned short samplerate;
	unsigned short channel;
	int length;
	int loopPosition;
};

int main(int argc, char** argv) {
	// Parse arguments
	if (argc < 2 || argc > 3) {
		std::cout << "Invalid number of arguments." << std::endl
		<< USAGE_STRING << std::endl;
		return 0;
	}
	
	char* adpcmName = argv[1];  // Apparently absolute paths are safe
	unsigned int loopSample = 0;
	if (argc == 3) {
		unsigned long l = strtoul(argv[2], NULL, 10);
		if (l > 0) loopSample = l;
		else {
			std::cout << "Error: Loop sample must be a number > 0." << std::endl
			<< USAGE_STRING << std::endl;
			return 0;
		}
	}
	
	// Initialise file loader and ADPCM data objects
	Loader loader;
	Adpcm adpcm;
	loader.load(adpcmName, "adpcm");  // Verifies that the file is an ADPCM file
	
	std::cout << "File loaded." << std::endl;
	
	if (!loader.getPtr("adpcm")) {
		// File would have hidden ADPCM tag if it was a Texmaster ADPCM file
		std::cout << "File is not ADPCM." << std::endl;
		return 0;
	}
	
	// Sets up the object's members.
	adpcm.initialize();
	
	std::cout << "Setting info." << std::endl;
	
	// Stream length multiplier.
	// Varies on # of channels and original file sample rate relative to output rate
	int multiplier = 2;
	
	{
		unsigned char* ptr = reinterpret_cast<unsigned char*>(loader.getPtr("adpcm"));
		struct AdpcmFile _adpcmFile;
		
		// Grab metadata from file and input into metadata object.
		lib_memcpy(&_adpcmFile, ptr, 0x10);
		adpcm.entry(SOUND_ID,
			ptr + 0x10,
			_adpcmFile.length,
			_adpcmFile.samplerate,
			_adpcmFile.channel,
			_adpcmFile.length );
		
		// Stream length multiplier fuckery
		multiplier = 2 * outputSampleRate / _adpcmFile.samplerate;
		multiplier *= (2 / _adpcmFile.channel); 
		
		// Force 44100Hz, 2-channel audio
		adpcm.setFillFormat(outputSampleRate, 2);
	}
	
	// The actual stream is 16-bit data, so the length of the input stream is double the buffer length
	int halfMul = multiplier / 2;
	
	std::cout << "Preparing stream data array." << std::endl;
	
	// NOTE: 32-BIT SIGNED INT PCM AUDIO
	int streamLength = (loader.getLength("adpcm") - 0x10);  // -16 because of the length of the header.
	
	std::cout << "Length fetched." << std::endl;
	
	int* outputStream = new int[streamLength * multiplier];
	
	std::cout << "Filling stream." << std::endl;
	
	// Fills internal data slots with the raw ADPCM data and sets some volumes.
	adpcm.play(SOUND_ID);
	
	// Fill array with (hopefully decoded) PCM data
	adpcm.fill(outputStream, streamLength * halfMul);
	
	std::cout << "Preparing file." << std::endl;
	
	// Get file name
	std::string rawName = std::string(argv[1]);
	
	// Get file name with .wav extension instead of .adpcm extension
	std::string name;
	if (rawName.substr(rawName.length() - 6, 6) == ".adpcm") name = rawName.substr(0, rawName.length() - 6) + ".wav";
	else name = rawName + ".wav";
	
	// Amplify sound 16x, like the in-game maximum setting.
	for (int i = 0; i < streamLength * multiplier; i++) outputStream[i] = outputStream[i] * 0x10;
	
	// Attempt to output wave file.
	if (outputWAVFile(name, outputStream, streamLength * multiplier, loopSample)) {
		std::cout << ("File output success: " + name) << std::endl;
	} else {
		std::cout << "File output failure." << std::endl;
	}
	
	// Clear array
	free(outputStream);
	return 0;
}
