#include <stdio.h>
#include <rtdef.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <dfs_posix.h>
#include "dsp/fh_audio_mpi.h"
#include "wavplay.h"
#include "xgdzbar_statics.h"
#include "sok.h"
#include "xgd_config_deal.h"


#define RECORD_TIME 10            // in seconds
#define RECORD_TYPE FH_AC_MIC_IN  // FH_AC_MIC_IN or FH_AC_LINE_IN
#define ONE_FRAME_SIZE 1024       // how many audio samples in one frame...

void speaker_sound_enable(int ctrl);
int audio_player_restart(void);

static int audio_play_force_stop;

static int samples_to_bytes(int n, int sample_bits)
{
	if (sample_bits == 24)
	{
		return (n*4);
	}

	return (n * (sample_bits>>3));
}

static void print_range(void)
{
	rt_kprintf("enc_type    [0:PCM, 1:G711A, 2:G711U, 3:G726_16K, 4:G726_32K, 5:AAC]\n");
	rt_kprintf("sample_rate [8000, 16000, 32000, 48000, 11025, 22050, 44100]\n");
	rt_kprintf("bit_width   [8, 16, 32]");
	rt_kprintf("volume      [0~31]");
}
	
static int check_range(int enc_type, int sample_rate, int bit_width, int volume)
{
	if ((unsigned int)enc_type > 5)
	{
		return -1;
	}

	if (sample_rate != 8000 && sample_rate != 16000 && sample_rate != 32000 && sample_rate != 48000 &&
	    sample_rate != 11025 && sample_rate != 22050 && sample_rate != 44100 )
	{
		return -1;
	}

	if (bit_width != 8 && bit_width != 16 && bit_width != 32)
	{
		return -1;
	}

	if (enc_type != FH_PT_LPCM && bit_width != 16)
	{
		return -1;
	}

	if (volume < 0 || volume > 31)
	{
		return -1;
	}

	return 0;
}

static void audio_cap(char* filename, int enc_type, int sample_rate, int bit_width, int volume)
{
	int fd = -1;
	int ret;
	int period_bytes;
	int tick1;
	int tickdiff;
	FH_UINT32 diff;
	FH_UINT64 pts;
	FH_UINT64 pts2;

    printf("\n\nXXXXXXXXXXXXXXXX NO3 XXXXXXXXXXXXX\n\n\n\n");
    while(1);

	FH_AC_CONFIG cfg;
	FH_AC_FRAME_S frame_info;

	print_range();
	if (check_range(enc_type, sample_rate, bit_width, volume) != 0)
	{
		rt_kprintf("audio_cap: invalid parameter!\n");
		return;
	}

	cfg.io_type     = RECORD_TYPE;
	cfg.sample_rate = sample_rate;
	cfg.bit_width   = bit_width;
	cfg.enc_type    = enc_type;
	cfg.channels    = 1;
	cfg.period_size = ONE_FRAME_SIZE;
	cfg.volume      = volume;

	period_bytes = samples_to_bytes(cfg.period_size, cfg.bit_width);
	frame_info.data = rt_malloc(period_bytes);
	if (!frame_info.data)
	{
		rt_kprintf("rt_malloc: failed!\n");
		return;
	}

	ret = FH_AC_Init();
	if (ret != 0)
	{
		rt_kprintf("FH_AC_Init: failed ret=%d.\n", ret);
		goto Exit;
	}

	ret = FH_AC_Set_Config(&cfg);
	if (ret != 0)
	{
		rt_kprintf("FH_AC_Set_Config: failed ret=%d.\n", ret);
		goto Exit;
	}

	FH_AC_AI_MICIN_SetVol(2);
	ret = FH_AC_AI_Enable();
	if (ret != 0)
	{
		rt_kprintf("FH_AC_AI_Enable: failed ret=%d.\n", ret);
		goto Exit;
	}

	fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0);
	if (fd < 0)
	{
		rt_kprintf("open file for failed\n");
		goto Exit2;
	}

	tick1 = rt_tick_get();
	while (1)
	{
		frame_info.len = period_bytes;
		ret = FH_AC_AI_GetFrameWithPts(&frame_info, &pts2);
		if (ret != 0)
		{
			rt_kprintf("FH_AC_AI_GetFrame: failed, ret=%d.\n", ret);
			break;
		}

		diff = pts2 - pts;
		pts = pts2;
		rt_kprintf("%d.\n", diff);

		if (fd >= 0)
		{
			if (frame_info.len != write(fd, frame_info.data, frame_info.len))
			{
				rt_kprintf("audio write failed!\n");
				goto Exit;
			}
		}
		
		tickdiff = rt_tick_get() - tick1;
		if (tickdiff >= RECORD_TIME * RT_TICK_PER_SECOND)
			break;
	}

	rt_kprintf("audio capture finished.\n");

Exit:
	if (fd >= 0)
		close(fd);
Exit2:
	FH_AC_AI_Disable();
	FH_AC_DeInit();

	if (frame_info.data)
		rt_free(frame_info.data);
}

static void audio_play(char* filename, int enc_type, int sample_rate, int bit_width, int volume)
{
	int ret;
	int fd = -1;
	int period_bytes;

	FH_AC_CONFIG  cfg;
	FH_AC_FRAME_S frame_info;

    printf("\n\nXXXXXXXXXXXXXXXX NO2 XXXXXXXXXXXXX\n\n\n\n");
    while(1);

	print_range();
	if (check_range(enc_type, sample_rate, bit_width, volume) != 0)
	{
		rt_kprintf("audio_play: invalid parameter!\n");
		return;
	}

	if (enc_type == FH_PT_AAC)
	{
		rt_kprintf("AAC play is not supported now!\n");
		return;
	}

	fd = open(filename, O_RDWR, 0);
	if (fd < 0)
	{
		rt_kprintf("open file failed!\n");
		return;
	}

	cfg.io_type     = FH_AC_LINE_OUT;
	cfg.sample_rate = sample_rate;
	cfg.bit_width   = bit_width;
	cfg.enc_type    = enc_type;
	cfg.channels    = 1;
	cfg.period_size = ONE_FRAME_SIZE;
	cfg.volume      = volume;

	period_bytes = samples_to_bytes(cfg.period_size, cfg.bit_width);
	frame_info.data = rt_malloc(period_bytes);
	if (!frame_info.data)
	{
		rt_kprintf("rt_malloc: failed!\n");
		goto Exit;
	}

	if ((ret=FH_AC_Init()) != 0)
	{
		rt_kprintf("FH_AC_Init: failed, ret=%d!\n", ret);
		goto Exit;
	}

	if ((ret=FH_AC_Set_Config(&cfg)) != 0)
	{
		rt_kprintf("FH_AC_Set_Config: failed, ret=%d!\n", ret);
		goto Exit;
	}

	if ((ret=FH_AC_AO_Enable()) != 0)
	{
		rt_kprintf("FH_AC_AO_Enable: failed, ret=%d!\n", ret);
		goto Exit;
	}

	while (1)
	{
		if (period_bytes != read(fd, frame_info.data, period_bytes))
		{
			rt_kprintf("Read file finished!\n");
			break;
		}

		frame_info.len = period_bytes;
		ret = FH_AC_AO_SendFrame(&frame_info);
		if (ret != 0)
		{
			rt_kprintf("FH_AC_AO_SendFrame: failed, ret=%d!\n", ret);
			break;
		}
	}

	printf("play over!\n");
	rt_thread_delay(5*RT_TICK_PER_SECOND); //wailt playing the remained buffered audio data...


Exit:
	if (fd >= 0)
		close(fd);

	FH_AC_AO_Disable();
	FH_AC_DeInit();

	if (frame_info.data)
		rt_free(frame_info.data);
}

static void audio_loopback(int sample_rate, int volume)
{
	int ret;
	int period_bytes;
	int tick1;
	int tickdiff;

	FH_AC_CONFIG cfg;
	FH_AC_FRAME_S frame_info;

	cfg.io_type     = RECORD_TYPE;
	cfg.sample_rate = sample_rate;
	cfg.bit_width   = AC_BW_16;
	cfg.enc_type    = FH_PT_LPCM;
	cfg.channels    = 1;
	cfg.period_size = ONE_FRAME_SIZE;
	cfg.volume      = volume;

	period_bytes = samples_to_bytes(cfg.period_size, cfg.bit_width);
	frame_info.data = rt_malloc(period_bytes);
	if (!frame_info.data)
	{
		return;
	}

	if (FH_AC_Init() != 0)
		goto Exit;

	//capture config
	if (FH_AC_Set_Config(&cfg) != 0)
		goto Exit;
	
	//play config
	cfg.io_type     = FH_AC_LINE_OUT;
	cfg.volume      = 31;
	if (FH_AC_Set_Config(&cfg) != 0)
		goto Exit;

	FH_AC_AI_MICIN_SetVol(3);

	if (FH_AC_AI_Enable() != 0)
		goto Exit;

	if (FH_AC_AO_Enable() != 0)
		goto Exit;

	tick1 = rt_tick_get();
	while (1)
	{
		frame_info.len = period_bytes;
		ret = FH_AC_AI_GetFrame(&frame_info);
		if (ret != 0)
			break;
		
		FH_AC_AO_SendFrame(&frame_info);

		tickdiff = rt_tick_get() - tick1;
		if (tickdiff >= RECORD_TIME * RT_TICK_PER_SECOND)
			break;
	}

	rt_kprintf("audio loopback test finished.\n");

Exit:
	FH_AC_AI_Disable();
	FH_AC_AO_Disable();
	FH_AC_DeInit();

	if (frame_info.data)
		rt_free(frame_info.data);
}


static unsigned char wav_buf[256*1024];
static int wav_len=0;
static unsigned char sZeroAoBuf[ONE_FRAME_SIZE * 2 + 8];
int audio_decoder_init(void)
{
    int ret;
    int fd = -1;
    int period_bytes;
    int enc_type;
    int sample_rate;
    int bit_width;
    int nread,loop;
    unsigned short *pdata;

    riffchunk_t ck;
    datachunk_t dat;
    wavheader_t wavh;
    FH_AC_CONFIG  cfg;
    FH_AC_FRAME_S frame_info;

    int volume_xgd = xgd_volume_config();//获取配置音量大小
    int voice_xgd  = xgd_config_read(SET_VOICE);

    rt_memset(sZeroAoBuf,0,sizeof(sZeroAoBuf));
    audio_play_force_stop = 0;
    if(voice_xgd == SET_VOICE_OK)
    {
        fd = open("sok.wav", O_RDWR, 0);//扫码成功
    }
    else
    {
        fd = open("beep.wav", O_RDWR, 0);//滴滴声
    }
    if (fd < 0)
    {
        fd = open("beep.wav", O_WRONLY | O_CREAT | O_TRUNC, 0);
        write(fd,beepok,sizeof(beepok));
        close(fd);

        fd = open("beep.wav", O_RDWR, 0);
        if(fd < 0)
        {
            rt_kprintf("open audio file  failed!\n");
            return -1;
        }
    }
    /* read RIFF Header */
    if(sizeof(ck) != read(fd,&ck,sizeof(ck)))
    {
        goto err1_out;
    }
    if(strncmp(ck.id,"RIFF",4))
    {
        goto err1_out;
    }

    /* read WAVE Header */ 
    if(sizeof(ck.id) != read(fd,&ck.id,sizeof(ck.id)))
    {
        goto err1_out;
    }
    if(strncmp(ck.id,"WAVE",4))
    {
        goto err1_out;
    }

    /* read fmt Header */
    if(sizeof(ck) != read(fd,&ck,sizeof(ck)))
    {
        goto err1_out;
    }
    if(strncmp(ck.id,"fmt ",4))
    {
        goto err1_out;
    }

    /* read wave header */
    if(sizeof(wavh) != read(fd,&wavh,sizeof(wavh)))
    {
        goto err1_out;
    }

    /* read data Header */
    if(sizeof(dat) != read(fd,&dat,sizeof(dat)))
    {
        goto err1_out;
    }
    if(strncmp(dat.id,"data",4))
    {
        goto err1_out;
    }

    #if 0/*xqy 2019-3-21*/
    rt_kprintf("Wave file Info:\n");
    rt_kprintf("format:%d\n",wavh.format);
    rt_kprintf("channel:%d\n",wavh.nchannels);
    rt_kprintf("framerate:%d\n",wavh.framerate);
    //rt_kprintf("byterate:%d\n",warrrvh.byterate);
    rt_kprintf("blocksize:%d\n",wavh.blocksize);
    rt_kprintf("bitdepth:%d\n",wavh.bitdepth);
    rt_kprintf("data len:%d\n",dat.size);
    #endif
    
    if(wavh.format != 0x1)
    {
        rt_kprintf("wave format must be 0x1,not %d\n",wavh.format);
        goto err1_out;
    }
    if(wavh.bitdepth != 16)
    {
        rt_kprintf("bitdepth must be 16\n");
        goto err1_out;
    }


    // Initial audio decoder hardware
    cfg.io_type     = FH_AC_LINE_OUT;
    cfg.sample_rate = wavh.framerate;
    cfg.bit_width   = wavh.bitdepth;
    cfg.enc_type    = FH_PT_LPCM;
    cfg.channels    = 1;
    cfg.period_size = ONE_FRAME_SIZE;
    cfg.volume      = 25; // 31;

    period_bytes = cfg.period_size*(cfg.bit_width>>3)*wavh.nchannels;
    frame_info.data = rt_malloc(period_bytes);
    if (NULL == frame_info.data)
    {
        rt_kprintf("rt_malloc: failed!\n");
        goto Exit;
    }

    if ((ret=FH_AC_Init()) != 0)
    {
        rt_kprintf("FH_AC_Init: failed, ret=%d!\n", ret);
        goto Exit;
    }

    if ((ret=FH_AC_Set_Config(&cfg)) != 0)
    {
        rt_kprintf("FH_AC_Set_Config: failed, ret=%d!\n", ret);
        goto Exit;
    }
#if 0
    if ((ret=FH_AC_AO_Enable()) != 0)
    {
        rt_kprintf("FH_AC_AO_Enable: failed, ret=%d!\n", ret);
        goto Exit;
    }
#endif
    //speaker_sound_enable(1);

    /* play a silence sound to skip pupu sound */
    memset(frame_info.data,0,sizeof(frame_info.data));
    frame_info.len = 128;

    /* play loop */
    while (1)
    {
        nread = read(fd, frame_info.data, period_bytes);
        if (nread <= 0)
        {
            rt_kprintf("Read file finished!\n");
            break;
        }

        /* convert to 1 channel data */
        if(wavh.nchannels == 2)
        {
            pdata = (unsigned short*)frame_info.data;
            for(loop=0;loop<nread/4;loop++)
            {
                pdata[loop] = pdata[loop*2];
            }
        }

        frame_info.len = nread/wavh.nchannels;

        if( (wav_len+frame_info.len) < sizeof(wav_buf) )
        {
            memcpy(&wav_buf[wav_len],frame_info.data,frame_info.len);
            wav_len += frame_info.len;
        }

        /*
        ret = FH_AC_AO_SendFrame(&frame_info);
        if (ret != 0)
        {
            rt_kprintf("FH_AC_AO_SendFrame: failed, ret=%d!\n", ret);
            break;
        }
        */
    }
    FH_AC_AO_SetVol(volume_xgd);//设置默认的音量
    printf("audio initial done\n");
    //rt_thread_delay(1*RT_TICK_PER_SECOND); //wailt playing the remained buffered audio data...
    
Exit:
    if (fd >= 0)
        close(fd);
    if (frame_info.data)
        rt_free(frame_info.data);
    
    //FH_AC_AO_Disable();
    //FH_AC_DeInit();
    //speaker_sound_enable(0);
    return 0;

err1_out:
    close(fd);
    return -1;
}
//static int sAoInitF = 0;
int wavplay(int argc,char *argv[])
{
    int ret;
    int fd = -1;
    int period_bytes;
    int enc_type;
    int sample_rate;
    int bit_width;
    int nread,loop;
    unsigned short *pdata;

    riffchunk_t ck;
    wavheader_t wavh;
    FH_AC_CONFIG  cfg;
    FH_AC_FRAME_S frame_info;

    printf("\n\nXXXXXXXXXXXXXXXX NO XXXXXXXXXXXXX\n\n\n\n");
    while(1);

    print_range();
    fd = open(argv[1], O_RDWR, 0);
    if (fd < 0)
    {
        rt_kprintf("open file:%s failed!\n",argv[1]);
        return;
    }

    /* read RIFF Header */
    if(sizeof(ck) != read(fd,&ck,sizeof(ck)))
    {
        goto err1_out;
    }
    if(strncmp(ck.id,"RIFF",4))
    {
        goto err1_out;
    }

    /* read WAVE Header */ 
    if(sizeof(ck.id) != read(fd,&ck.id,sizeof(ck.id)))
    {
        goto err1_out;
    }
    if(strncmp(ck.id,"WAVE",4))
    {
        goto err1_out;
    }

    /* read fmt Header */
    if(sizeof(ck) != read(fd,&ck,sizeof(ck)))
    {
        goto err1_out;
    }
    if(strncmp(ck.id,"fmt ",4))
    {
        goto err1_out;
    }

    /* read wave header */
    if(sizeof(wavh) != read(fd,&wavh,sizeof(wavh)))
    {
        goto err1_out;
    }

    rt_kprintf("Wave file Info:\n");
    rt_kprintf("format:%d\n",wavh.format);
    rt_kprintf("channel:%d\n",wavh.nchannels);
    rt_kprintf("framerate:%d\n",wavh.framerate);
    rt_kprintf("byterate:%d\n",wavh.byterate);
    rt_kprintf("blocksize:%d\n",wavh.blocksize);
    rt_kprintf("bitdepth:%d\n",wavh.bitdepth);
    
    if(wavh.format != 0x1)
    {
        rt_kprintf("wave format must be 0x1,not %d\n",wavh.format);
        goto err1_out;
    }
    if(wavh.bitdepth != 16)
    {
        rt_kprintf("bitdepth must be 16\n");
        goto err1_out;
    }
    
    cfg.io_type     = FH_AC_LINE_OUT;
    cfg.sample_rate = wavh.framerate;
    cfg.bit_width   = wavh.bitdepth;
    cfg.enc_type    = FH_PT_LPCM;
    cfg.channels    = 1;
    cfg.period_size = ONE_FRAME_SIZE;
    cfg.volume      = 31;

    period_bytes = cfg.period_size*(cfg.bit_width>>3)*wavh.nchannels;
    frame_info.data = rt_malloc(period_bytes);
    if (NULL == frame_info.data)
    {
        rt_kprintf("rt_malloc: failed!\n");
        goto Exit;
    }

    if ((ret=FH_AC_Init()) != 0)
    {
        rt_kprintf("FH_AC_Init: failed, ret=%d!\n", ret);
        goto Exit;
    }

    if ((ret=FH_AC_Set_Config(&cfg)) != 0)
    {
        rt_kprintf("FH_AC_Set_Config: failed, ret=%d!\n", ret);
        goto Exit;
    }

    if ((ret=FH_AC_AO_Enable()) != 0)
    {
        rt_kprintf("FH_AC_AO_Enable: failed, ret=%d!\n", ret);
        goto Exit;
    }
    speaker_sound_enable(1);

    /* play a silence sound to skip pupu sound */
    memset(frame_info.data,0,sizeof(frame_info.data));
    frame_info.len = 128;
    
    /* play loop */
    while (1)
    {
        nread = read(fd, frame_info.data, period_bytes);
        if (nread <= 0)
        {
            rt_kprintf("Read file finished!\n");
            break;
        }

        /* convert to 1 channel data */
        if(wavh.nchannels == 2)
        {
            pdata = (unsigned short*)frame_info.data;
            for(loop=0;loop<nread/4;loop++)
            {
                pdata[loop] = pdata[loop*2];
            }
        }

        frame_info.len = nread/wavh.nchannels;
        ret = FH_AC_AO_SendFrame(&frame_info);
        if (ret != 0)
        {
            rt_kprintf("FH_AC_AO_SendFrame: failed, ret=%d!\n", ret);
            break;
        }
    }

    printf("play over!\n");
    rt_thread_delay(1*RT_TICK_PER_SECOND); //wailt playing the remained buffered audio data...


Exit:
    if (fd >= 0)
        close(fd);

    FH_AC_AO_Disable();
    FH_AC_DeInit();
    speaker_sound_enable(0);
    if (frame_info.data)
        rt_free(frame_info.data);
    return ;

err1_out:
    close(fd);
    
}


#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT(audio_cap, audio_cap(char* filename, int enc_type, int sample_rate, int bit_width, int volume));
FINSH_FUNCTION_EXPORT(audio_play, audio_play(char* filename, int enc_type, int sample_rate, int bit_width, int volume));
FINSH_FUNCTION_EXPORT(audio_loopback, audio_loopback(8000, 20));
MSH_CMD_EXPORT(wavplay, wave file player);
#endif
