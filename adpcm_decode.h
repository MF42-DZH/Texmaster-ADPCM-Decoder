#ifndef __adpcm_decode__
#define __adpcm_decode__

#define YM2608_DEFAULT_GUESS (0)
#define YM2608_DEFAULT_DELTA (127)

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	// 1 channel
	void ym2608adpcm_decode1( unsigned char*, short*, int, int*, int* );
	// 2 channel
	void ym2608adpcm_decode2( unsigned char*, short*, int, int*, int* );

	
	// 1 channel to 1 channel MIX
	void ym2608adpcm_decode1_mix1( unsigned char*, int*, int, int*, int*, int );
	// 1 channel toÅ2 channel MIX
	void ym2608adpcm_decode1_mix2( unsigned char*, int, int*, int, int*, int*, int );
	// 2 channel toÅ2 channel MIX
	void ym2608adpcm_decode2_mix2( unsigned char*, int*, int, int*, int*, int* );

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __adpcm_decode__ */
