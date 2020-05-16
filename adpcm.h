// ADPCM Data type
#ifndef ADPCM
#define ADPCM

struct SoundSlot {
	int len;
	int samplerate;			// 11025/22050/44100
	int channel;			// 1/2
	int compress;			// Adpcm/Pcm/PSG/SCC
	unsigned char* buf;		// Data
	int id;					// For reverse rotation of song numbers

	int play;
	int volume[2];  		// 0-16
	int pos;
	int playedPos;
	int playedPosL;
	int guess[2];			// Adpcm Resource
	int delta[2];			// Adpcm Resource

	int posLoop;			// For special loops
	int guessLoop[2];		// For special loops
	int deltaLoop[2];		// For special loops
};

struct AdpcmList {
	int len;
	int samplerate;			// 11025/22050/44100
	int channel;			// 1/2
	int compress;			// Adpcm/Pcm/PSG/SCC
	unsigned char* buf;		// Data

	int posLoop;			// For special loops (if it is -1 or 0x7FFFFFFF, it is disabled)
	int guessLoop[2];		// For special loops
	int deltaLoop[2];		// For special loops
};

struct SoundNameList {
	int id;
	char name[0x40];
};

const int MAX_ID_NUM = 0x80;

const int SOUND_GAME_VOLUME = 128;
const int SOUND_USER_VOLUME = 16;

class Adpcm {
	public:
		Adpcm();
		virtual ~Adpcm();

		void initialize( void );

		void fill( int* stream, int length );
		void setFillFormat( int samplerate, int channel );

		void decode( struct SoundSlot*, int*, int, int* );

		int entry( char* name, unsigned char* buf, int len, int samplerate, int channel, int loopPos );
		int setSpecialLoop( const char* name, int pos );
		int erase( char* name );

		int play( const char* );
		void stop( void );

		void setUserVolume( int );
		int getUserVolume( void );

		void setGameVolume( int );
		int getGameVolume( void );

		int getId( const char* );

		int getPlayedBgmTimes( void );
		int getPlayedBgmSamples( void );

		void setEndBgm5sec( void );

	protected:
		AdpcmList _list[ MAX_ID_NUM ];
		SoundNameList _cvt[ MAX_ID_NUM ];
		SoundSlot _slot;
		int _userVolume;			// 0-16  User-supplied volume
		int _samplerate;			// output 11025 | 22050 | 44100
		int _channel;				// output 1 | 2
		int _mutex;
		int* resampleBuffer;
};

#endif

