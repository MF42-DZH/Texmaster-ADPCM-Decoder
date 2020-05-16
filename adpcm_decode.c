#ifdef __cplusplus
#include <iostream>

extern "C" {
#endif /* __cplusplus */

static int f[] = { 57, 57, 57, 57, 77, 102, 128, 153 };

#include "converter.h"

void ym2608adpcm_decode1( unsigned char* adpcmData, short* pcmData, int length, int* guess, int* delta ){
	int i, adpcm;
	int _guess = *guess;
	int _delta = *delta;
	for( i = 0; i < length; i++ ){
		/* Reading input ADPCM data */
		if( ( i % 2 ) == 0 ){
			adpcm = ( adpcmData[ i / 2 ] >> 4 ) & 0xF;
		}else{
			adpcm = ( adpcmData[ i / 2 ] >> 0 ) & 0xF;
		}
		/* Update predictions */
		if( adpcm < 8 ){
			_guess = _guess + (( adpcm & 7 ) * 2 + 1) * _delta / 8;
		}else{
			_guess = _guess - (( adpcm & 7 ) * 2 + 1) * _delta / 8;
		}
		_guess = ( _guess < -32768 ) ? -32768 : _guess;
		_guess = ( _guess > 32767 ) ? 32767 : _guess;
		
		/* Update the reference value of the difference */
		_delta = _delta * f[ adpcm & 7 ] / 64;
		
		_delta = ( _delta < 127 ) ? 127 : _delta;
		_delta = ( _delta > 24576 ) ? 24576 : _delta;
		
		/* Write output PCM data (= predicted value) */
		pcmData[ i ] = _guess;
	}
	*guess = _guess;
	*delta = _delta;
}


void ym2608adpcm_decode2( unsigned char* adpcmData, short* pcmBuffer, int length, int* guess, int* delta ){
	int i;
	int _guess[ 2 ];
	int _delta[ 2 ];
	int _guessTemp[ 2 ];
	int adpcm[ 2 ];
	
	_guess[ 0 ] = *( guess + 0 );
	_guess[ 1 ] = *( guess + 0 );
	_delta[ 0 ] = *( delta + 0 );
	_delta[ 1 ] = *( delta + 1 );

	for( i = 0; i < length; i++ ){
		/* Reading input ADPCM data */
		adpcm[ 0 ] = ( adpcmData[ i ] >> 4 ) & 0xF;
		adpcm[ 1 ] = ( adpcmData[ i ] >> 0 ) & 0xF;
		/* Update predictions */
		_guessTemp[ 0 ] = (( adpcm[ 0 ] & 7 ) * 2 + 1 ) * _delta[ 0 ] / 8;
		_guessTemp[ 1 ] = (( adpcm[ 1 ] & 7 ) * 2 + 1 ) * _delta[ 1 ] / 8;
		
		if( adpcm[ 0 ] < 8 ){
			_guess[ 0 ] += _guessTemp[ 0 ];
		}else{
			_guess[ 0 ] -= _guessTemp[ 0 ];
		}
		if( adpcm[ 1 ] < 8 ){
			_guess[ 1 ] += _guessTemp[ 1 ];
		}else{
			_guess[ 1 ] -= _guessTemp[ 1 ];
		}
		
		_guess[ 0 ] = ( _guess[ 0 ] < -32768 ) ? -32768 : _guess[ 0 ];
		_guess[ 1 ] = ( _guess[ 1 ] < -32768 ) ? -32768 : _guess[ 1 ];
		_guess[ 0 ] = ( _guess[ 0 ] > 32767 ) ? 32767 : _guess[ 0 ];
		_guess[ 1 ] = ( _guess[ 1 ] > 32767 ) ? 32767 : _guess[ 1 ];
		
		/* Update the reference value of the difference */
		_delta[ 0 ] = _delta[ 0 ] * f[ adpcm[ 0 ] & 7 ] / 64;
		_delta[ 1 ] = _delta[ 1 ] * f[ adpcm[ 1 ] & 7 ] / 64;
		
		_delta[ 0 ] = ( _delta[ 0 ] < 127 ) ? 127 : _delta[ 0 ];
		_delta[ 1 ] = ( _delta[ 1 ] < 127 ) ? 127 : _delta[ 1 ];
		_delta[ 0 ] = ( _delta[ 0 ] > 24576 ) ? 24576 : _delta[ 0 ];
		_delta[ 1 ] = ( _delta[ 1 ] > 24576 ) ? 24576 : _delta[ 1 ];
		
		/* Write output PCM data (= predicted value) */
		pcmBuffer[ i * 2 + 0 ] = _guess[ 0 ];
		pcmBuffer[ i * 2 + 1 ] = _guess[ 1 ];
	}
	*( guess + 0 ) = _guess[ 0 ];
	*( guess + 1 ) = _guess[ 1 ];
	*( delta + 0 ) = _delta[ 0 ];
	*( delta + 1 ) = _delta[ 1 ];
}


//
// Decode Simultaneous Mix
//

void ym2608adpcm_decode1_mix1( unsigned char* adpcmData, int* pcmBuffer, int length, int* guess, int* delta, int volume ){
	int i;
	int _guess = *guess;
	int _delta = *delta;
	unsigned char adpcm;
	
	for( i = 0; i < length; ++i ){
		/* Reading input ADPCM data */
		if( ( i % 2 ) == 0 ){
			adpcm = *( adpcmData ) >> 4;
		}else{
			adpcm = *( adpcmData ) & 0xF;
			++adpcmData;
		}
		/* Update predictions */
		if( adpcm < 8 ){
			_guess = _guess + (( adpcm & 7 ) * 2 + 1) * _delta / 8;
		}else{
			_guess = _guess - (( adpcm & 7 ) * 2 + 1) * _delta / 8;
		}
		
		_guess = ( _guess < -32768 ) ? -32768 : _guess;
		_guess = ( _guess > 32767 ) ? 32767 : _guess;
		
		/* Update the reference value of the difference */
		_delta = _delta * f[ adpcm & 7 ] / 64;
		
		_delta = ( _delta < 127 ) ? 127 : _delta;
		_delta = ( _delta > 24576 ) ? 24576 : _delta;

		pcmBuffer[ i ] += _guess * volume;

	}
	*guess = _guess;
	*delta = _delta;
}

//
// Stereo ADPCM input -> Stereo ADPCM output
//
void ym2608adpcm_decode2_mix2( unsigned char* adpcmData, int* pcmBuffer, int length, int* guess, int* delta, int* volume ){
	int i;
	int _guess[ 2 ];
	int _delta[ 2 ];
	int _volume[ 2 ];
	int _guessTemp[ 2 ];
	int adpcm[ 2 ];
	
	_guess[ 0 ] = *( guess + 0 );
	_guess[ 1 ] = *( guess + 1 );
	_delta[ 0 ] = *( delta + 0 );
	_delta[ 1 ] = *( delta + 1 );
	_volume[ 0 ] = *( volume + 0 );
	_volume[ 1 ] = *( volume + 1 );
	
	#ifdef __cplusplus
	//	std::cout << "DECODING START! Length: " << length << std::endl;
	#endif

	for( i = 0; i < length; i++ ){
		// std::cout << "HERE IN DECODE: " << (i - 1) << '/' << length << std::endl;
		#ifdef __cplusplus
			if (i % 5000 == 0) std::cout << "Decoding ADPCM: " << (i + 1) << '/' << length << '\r';
		#endif
		
		/* Reading input ADPCM data */
		adpcm[ 0 ] = ( adpcmData[ i ] >> 4 ) & 0xF;
		adpcm[ 1 ] = ( adpcmData[ i ] >> 0 ) & 0xF;
		/* Update predictions */
		_guessTemp[ 0 ] = (( adpcm[ 0 ] & 7 ) * 2 + 1 ) * _delta[ 0 ] / 8;
		_guessTemp[ 1 ] = (( adpcm[ 1 ] & 7 ) * 2 + 1 ) * _delta[ 1 ] / 8;
		
		if( adpcm[ 0 ] < 8 ){
			_guess[ 0 ] += _guessTemp[ 0 ];
		}else{
			_guess[ 0 ] -= _guessTemp[ 0 ];
		}
		if( adpcm[ 1 ] < 8 ){
			_guess[ 1 ] += _guessTemp[ 1 ];
		}else{
			_guess[ 1 ] -= _guessTemp[ 1 ];
		}
		
		_guess[ 0 ] = ( _guess[ 0 ] < -32768 ) ? -32768 : _guess[ 0 ];
		_guess[ 1 ] = ( _guess[ 1 ] < -32768 ) ? -32768 : _guess[ 1 ];
		_guess[ 0 ] = ( _guess[ 0 ] > 32767 ) ? 32767 : _guess[ 0 ];
		_guess[ 1 ] = ( _guess[ 1 ] > 32767 ) ? 32767 : _guess[ 1 ];
		
		#ifdef __cplusplus
			// std::cout << "REACHED DELTA AND GUESS UPDATE at " << i << " / " << length << std::endl;
		#endif
		
		/* Update the reference value of the difference */
		_delta[ 0 ] = _delta[ 0 ] * f[ adpcm[ 0 ] & 7 ] / 64;
		_delta[ 1 ] = _delta[ 1 ] * f[ adpcm[ 1 ] & 7 ] / 64;
		
		_delta[ 0 ] = ( _delta[ 0 ] < 127 ) ? 127 : _delta[ 0 ];
		_delta[ 1 ] = ( _delta[ 1 ] < 127 ) ? 127 : _delta[ 1 ];
		_delta[ 0 ] = ( _delta[ 0 ] > 24576 ) ? 24576 : _delta[ 0 ];
		_delta[ 1 ] = ( _delta[ 1 ] > 24576 ) ? 24576 : _delta[ 1 ];
		
		#ifdef __cplusplus
			// std::cout << "REACHED PCM WRITE at " << (i * 2) << " & " << (i * 2 + 1) << std::endl;
			// std::cout << "GUESSES: " << _guess[0] << " & " << _guess[1] << std::endl;
		#endif
		
		/* Write output PCM data (= predicted value) */
		pcmBuffer[ i * 2 + 0 ] += _guess[ 0 ] * _volume[ 0 ];
		pcmBuffer[ i * 2 + 1 ] += _guess[ 1 ] * _volume[ 1 ];
	}
	*( guess + 0 ) = _guess[ 0 ];
	*( guess + 1 ) = _guess[ 1 ];
	*( delta + 0 ) = _delta[ 0 ];
	*( delta + 1 ) = _delta[ 1 ];
	
	#ifdef __cplusplus
		std::cout << "Decoding ADPCM: " << length << '/' << length << std::endl;
	#endif
}


//
// Monaural ADPCM input -> Stereo PCM output
//
void ym2608adpcm_decode1_mix2( unsigned char* adpcmData, int d, int* pcmBuffer, int length, int* guess, int* delta, int volume ){
	int i;
	int _guess = *guess;
	int _delta = *delta;
	unsigned char adpcm;
	
	for( i = 0; i < length; ++i ){
		#ifdef __cplusplus
			if (i % 5000 == 0) std::cout << "Decoding ADPCM: " << (i + 1) << '/' << length << '\r';
		#endif
		
		/* Reading input ADPCM data */
		if( ( i % 2 ) == d ){
			adpcm = *( adpcmData ) >> 4;
		}else{
			adpcm = *( adpcmData ) & 0xF;
			++adpcmData;
		}
		/* Update predictions */
		if( adpcm < 8 ){
			_guess = _guess + (( adpcm & 7 ) * 2 + 1) * _delta / 8;
		}else{
			_guess = _guess - (( adpcm & 7 ) * 2 + 1) * _delta / 8;
		}
		
		_guess = ( _guess < -32768 ) ? -32768 : _guess;
		_guess = ( _guess > 32767 ) ? 32767 : _guess;
		
		/* Update the reference value of the difference */
		_delta = _delta * f[ adpcm & 7 ] / 64;
		
		_delta = ( _delta < 127 ) ? 127 : _delta;
		_delta = ( _delta > 24576 ) ? 24576 : _delta;

		/* Write output PCM data (= predicted value) */
		pcmBuffer[ i * 2 + 0 ] += _guess * volume;
		pcmBuffer[ i * 2 + 1 ] += _guess * volume;
	}
	*guess = _guess;
	*delta = _delta;
	
	#ifdef __cplusplus
		std::cout << "Decoding ADPCM: " << length << '/' << length << std::endl;
	#endif
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
