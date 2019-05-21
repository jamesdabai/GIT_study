#ifndef __DEV_AUDIO_H
#define __DEV_AUDIO_H 

typedef struct _riffchunk {
	char		id[4];
	uint32_t	size;
} riffchunk_t;

typedef struct _datachunk {
	char		id[4];
	uint32_t	size;
} datachunk_t;


typedef struct _wavheader {
	int16_t	format;
	int16_t	nchannels;
	int32_t	framerate;
	int32_t	byterate;
	int16_t	blocksize;
	int16_t	bitdepth;
} wavheader_t;


int dev_audio_open(void);

int dev_audio_close(void);

int dev_audio_play(const unsigned char *lpFile);         

int audio_play_thread(void); 			

int dev_audio_stop(void);

int dev_audio_pause(void);

int dev_audio_replay(void);

int dev_audio_ioctl(unsigned int nCmd, unsigned int lParam, unsigned int wParam);

#endif

































