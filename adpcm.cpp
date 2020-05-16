#include "adpcm.h"
#include "adpcm_decode.h"
extern "C" {
	#include "converter.h"
}

#include <iostream>

Adpcm::Adpcm(){
	// Do not initialise buffer here
}

Adpcm::~Adpcm(){
	if( resampleBuffer ){
		delete [] resampleBuffer;
	}
}

void Adpcm::initialize(){
	_userVolume = SOUND_USER_VOLUME;

	_samplerate = 0;
	_channel = 0;
	
	for( int i = 0; i < MAX_ID_NUM; ++i ){
		_cvt[ i ].id = -1;
	}
	_slot.play = 0;
	_slot.pos = 0;
}

void Adpcm::setFillFormat( int samplerate, int channel ){
	_samplerate = samplerate;
	_channel = channel;
}

void Adpcm::fill( int* stream, int streamLength ){

	struct SoundSlot* list = &_slot;
	
	// std::cout << list->play << std::endl;
	// std::cout << _userVolume << std::endl;
	// std::cout << list->volume[ 0 ] << std::endl;
	// std::cout << list->volume[ 1 ] << std::endl;
	
	if( list->play ){
		int volume[ 2 ];

		volume[ 0 ] = ( list->volume[ 0 ] * _userVolume );
		volume[ 1 ] = ( list->volume[ 1 ] * _userVolume );

		decode( list, stream, streamLength, volume );
	}
}

int Adpcm::entry( char* name, unsigned char* buf, int len, int samplerate, int channel, int loopPos ){

	int entryId = -1;

	for( int i = 0; i < MAX_ID_NUM; ++i ){
		if( _cvt[ i ].id == -1 ){
			entryId = i;
			break;
		}
	}
	
	if( entryId == -1 ){
		// ???????????????????????????
		return 0;
	}

	struct AdpcmList* entryList = &_list[ entryId ];
	struct SoundNameList* entryCvt = &_cvt[ entryId ];

	entryCvt->id = entryId;
	lib_strcpy( entryCvt->name, name );
	
	entryList->buf = buf;
	entryList->channel = channel;
	entryList->len = len;
	entryList->samplerate = samplerate;
	entryList->posLoop = loopPos;

	return 1;
}

void Adpcm::decode( struct SoundSlot* slot, int* stream, int streamLength, int* volume ){
	
	int remain = ( slot->len - slot->pos );

	unsigned char* data_pointer = slot->buf + ( slot->pos / ( slot->channel == 1 ? 2 : 1 ));
	
	if( !resampleBuffer ){
		resampleBuffer = new int [ streamLength * 2 ];  // This is a terrible idea
	}
	
	// std::cout << "MADE IT PAST BUFFER CHECK" << std::endl;
	
	int dstFreq = ( _samplerate / 11025 );
	int srcFreq = ( slot->samplerate / 11025 );
	int mul = ( srcFreq * 4 ) / dstFreq;
	int streamLength2 = streamLength * srcFreq / dstFreq;
	if( remain > streamLength2 ){
		remain = streamLength2;
	}else{
		remain = remain;  // WTF?
	}
	
	// std::cout << "MADE IT PAST MUL CHECK" << std::endl;
	
	for( int i = 0; i < 2048; ++i ){
		resampleBuffer[ i ] = 0;
	}
	
	// std::cout << "MADE IT PAST RESAMPLE BUFFER INITIALISATION" << std::endl;
	// std::cout << slot->channel << std::endl;
	// std::cout << slot->guess[0] << '\t' << slot->guess[1] << std::endl;
	// std::cout << slot->delta[0] << '\t' << slot->delta[1] << std::endl;
	
	// std::cout << "REMAINING TO DECODE: " << remain << '/' << streamLength << std::endl;
	
	if( slot->channel == 1 ){
		ym2608adpcm_decode1_mix2( data_pointer, slot->pos & 1, resampleBuffer, remain, slot->guess, slot->delta, volume[ 0 ] );
	}
	if( slot->channel == 2 ){
		ym2608adpcm_decode2_mix2( data_pointer, resampleBuffer, remain, slot->guess, slot->delta, volume );
	}
	
	// std::cout << "MADE IT PAST MIXING" << std::endl;
	
	if( mul == 8 ){
		// std::cout << "M8" << std::endl;
		for( int i = 0; i < remain; i += 4 ){
		//	std::cout << "HERE: " << (i - 1) << '/' << remain << std::endl;
			stream[ 0 ] += ( resampleBuffer[ i ] + resampleBuffer[ i + 2 ] ) / 2;
			stream[ 1 ] += ( resampleBuffer[ i + 1 ] + resampleBuffer[ i + 3 ] ) / 2;
			stream += 2;
		}
	}
	if( mul == 4 ){
	// 	std::cout << "M4" << std::endl;
		for( int i = 0; i < remain * 2; i += 2 ){
		//	std::cout << "HERE: " << (i - 1) << '/' << remain << std::endl;
			stream[ 0 ] += resampleBuffer[ i ];
			stream[ 1 ] += resampleBuffer[ i + 1 ];
			stream += 2;
		}
	}
	if( mul == 2 ){
	//	std::cout << "M2" << std::endl;
		for( int i = 0; i < remain * 2; i += 2 ){
		//	std::cout << "HERE: " << (i - 1) << '/' << remain << std::endl;
			stream[ 0 ] += resampleBuffer[ i ];
			stream[ 1 ] += resampleBuffer[ i + 1 ];
			stream[ 2 ] += resampleBuffer[ i ];
			stream[ 3 ] += resampleBuffer[ i + 1 ];
			stream += 4;
		}
	}
	if( mul == 1 ){
	//	std::cout << "M1" << std::endl;
		for( int i = 0; i < remain * 2; i += 2 ){
		//	std::cout << "HERE: " << (i - 1) << '/' << remain << std::endl;
			stream[ 0 ] += resampleBuffer[ i ];
			stream[ 1 ] += resampleBuffer[ i + 1 ];
			stream[ 2 ] += resampleBuffer[ i ];
			stream[ 3 ] += resampleBuffer[ i + 1 ];
			stream[ 4 ] += resampleBuffer[ i ];
			stream[ 5 ] += resampleBuffer[ i + 1 ];
			stream[ 6 ] += resampleBuffer[ i ];
			stream[ 7 ] += resampleBuffer[ i + 1 ];
			stream += 8;
		}
	}
	
	//std::cout << "MADE IT PAST STREAM WRITING" << std::endl;

	slot->pos += remain;
	slot->playedPosL += remain;
	while( slot->playedPosL > slot->samplerate ){
		slot->playedPosL -= slot->samplerate;
		++slot->playedPos;
	}
	
	//std::cout << "MADE IT PAST FINALISATION" << std::endl;
	
	if( slot->pos == slot->len ){
		if( slot->len <= slot->posLoop ){
			// one play
			slot->play = 0;
		}else{
			// loop
			slot->pos = slot->posLoop;
			slot->guess[ 0 ] = slot->guessLoop[ 0 ];
			slot->guess[ 1 ] = slot->guessLoop[ 1 ];
			slot->delta[ 0 ] = slot->deltaLoop[ 0 ];
			slot->delta[ 1 ] = slot->deltaLoop[ 1 ];
			decode( slot, stream, streamLength - remain * ( dstFreq / srcFreq ), volume );
		}
	}
	
	//std::cout << "MADE IT PAST THE OTHER DECODE LOOP" << std::endl;
}

int Adpcm::getId( const char* name ){
	for( int i = 0; i < MAX_ID_NUM; ++i ){
		if( lib_strcmp( _cvt[ i ].name, name ) == 0 ){
			return i;
		}
	}
	return -1;
}

int Adpcm::play( const char* name ){
	int id = getId( name );
	if( id == -1 ){
		return 0;
	}
	_slot.play = 1;
	_slot.pos = 0;
	_slot.volume[ 0 ] = SOUND_GAME_VOLUME;
	_slot.volume[ 1 ] = SOUND_GAME_VOLUME;
	_slot.playedPos = 0;
	_slot.playedPosL = 0;

	_slot.id = id;
	_slot.buf = _list[ id ].buf;
	_slot.channel = _list[ id ].channel;
	_slot.guess[ 0 ] = YM2608_DEFAULT_GUESS;
	_slot.delta[ 0 ] = YM2608_DEFAULT_DELTA;
	_slot.guess[ 1 ] = YM2608_DEFAULT_GUESS;
	_slot.delta[ 1 ] = YM2608_DEFAULT_DELTA;
	_slot.len = _list[ id ].len;
	_slot.posLoop = _list[ id ].posLoop;
	_slot.guessLoop[ 0 ] = _list[ id ].guessLoop[ 0 ];
	_slot.guessLoop[ 1 ] = _list[ id ].guessLoop[ 1 ];
	_slot.deltaLoop[ 0 ] = _list[ id ].deltaLoop[ 0 ];
	_slot.deltaLoop[ 1 ] = _list[ id ].deltaLoop[ 1 ];
	_slot.samplerate = _list[ id ].samplerate;
	
	//std::cout << "SAMPLERATE: " << _list[id].samplerate << std::endl
	//<< "LENGTH: " << _list[id].len << std::endl;
	
	return 1;
}

