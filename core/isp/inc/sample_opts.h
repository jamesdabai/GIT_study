#ifndef __SAMPLE_OPTS_H__
#define __SAMPLE_OPTS_H__


#if 0
/* isp configuration */
#define ISP_FORMAT FORMAT_800X600P30//FORMAT_VGAP30 //FORMAT_720P30 //FORMAT_1080P25
#define ISP_INIT_WIDTH 800 //640 //1280 //1920
#define ISP_INIT_HEIGHT 600 //480 //720 //1088

/* dsp configuration */
#define VIDEO_INPUT_WIDTH 800 //640 //1280 //1920
#define VIDEO_INPUT_HEIGHT 600 //480 //720 //1080

/* channel 0 configuration */
#define CH0_WIDTH 800 //640 //1280 //1920
#define CH0_HEIGHT 600 //480 //720 //1080
#define CH0_BIT_RATE (4*1024*1024)
#define CH0_FRAME_COUNT 30
#define CH0_FRAME_TIME 1

/* channel 1 configuration */
#define CH1_WIDTH 800 //640 //352
#define CH1_HEIGHT 600 //480 //288
#define CH1_BIT_RATE (4*1024*1024)
#define CH1_FRAME_COUNT 30
#define CH1_FRAME_TIME 1
#else

#if 1
/* isp configuration */
#define ISP_FORMAT FORMAT_VGAP25 //FORMAT_720P30 //FORMAT_1080P25
#define ISP_INIT_WIDTH 640 //1280 //1920
#define ISP_INIT_HEIGHT 480 //720 //1088

/* dsp configuration */
#define VIDEO_INPUT_WIDTH 640 //1280 //1920
#define VIDEO_INPUT_HEIGHT 480 //720 //1080

/* channel 0 configuration */
#define CH0_WIDTH 640 //1280 //1920
#define CH0_HEIGHT 480 //720 //1080
#define CH0_BIT_RATE (4*1024*1024)
#define CH0_FRAME_COUNT 25 // 10
#define CH0_FRAME_TIME 1

/* channel 1 configuration */
#define CH1_WIDTH 640 //352
#define CH1_HEIGHT 480 //288
#define CH1_BIT_RATE (4*1024*1024)
#define CH1_FRAME_COUNT 25 // 10
#define CH1_FRAME_TIME 1

#else
/* isp configuration */
#define ISP_FORMAT FORMAT_VGAP25 //FORMAT_720P30 //FORMAT_1080P25
#define ISP_INIT_WIDTH 480 //1280 //1920
#define ISP_INIT_HEIGHT 320 //720 //1088

/* dsp configuration */
#define VIDEO_INPUT_WIDTH 480 //1280 //1920
#define VIDEO_INPUT_HEIGHT 320 //720 //1080

/* channel 0 configuration */
#define CH0_WIDTH 480 //1280 //1920
#define CH0_HEIGHT 320 //720 //1080
#define CH0_BIT_RATE (4*1024*1024)
#define CH0_FRAME_COUNT 25 // 10
#define CH0_FRAME_TIME 1

/* channel 1 configuration */
#define CH1_WIDTH 480 //352
#define CH1_HEIGHT 320 //288
#define CH1_BIT_RATE (4*1024*1024)
#define CH1_FRAME_COUNT 25 // 10
#define CH1_FRAME_TIME 1
#endif
#endif

#define JPEG_INIT_WIDTH 1920
#define JPEG_INIT_HEIGHT 1088

#endif



