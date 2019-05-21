#ifndef __RTTHREAD_APP_H__
#define __RTTHREAD_APP_H__

/*
 * SECTION: Application
 *
 * NOTE: choice one sensor only if not multi_sensor
 */

#define XGD_ENABLE_TCPIP
#define XGD_MUTI_THREAD_DECODE

#define FH_USING_COOLVIEW
/* #define FH_USING_MULTI_SENSOR */
#ifdef CONFIG_CHIP_FHZIYANG
#define RT_USING_IMX291
#else
#ifdef CONFIG_CHIP_FH8833T
#define RT_USING_JXF22
#else
//#define RT_USING_JXK02_MIPI
//#define RT_USING_JXF22_MIPI
//#define RT_USING_GC2905_MIPI
#define RT_USING_GC0406_MIPI

#endif
#endif

#define RT_USING_LIBVLC
#define RT_USING_SAMPLE_OVERLAY
#define RT_USING_SAMPLE_VENC
#define RT_USING_SAMPLE_VLCVIEW
#define RT_USING_SAMPLE_MJPEG
#define RT_APP_THREAD_PRIORITY 172
#define FH_USING_ADVAPI_ISP
#define FH_USING_ADVAPI_OSD

#define TASK_PRIORITY_XGDZBAR   168   
#define TASK_PRIORITY_AUDIO     169
#define TASK_PRIORITY_CONFIG    170
#define TASK_PRIORITY_SENDER    171
#define TASK_PRIORITY_HID       192

#define RT_USING_DSP
#define RT_USING_ISP

/* #define FH_USING_RTSP */
#ifdef FH_USING_RTSP
# define FH_USING_RTSP_RTP_TCP 1
# define FH_USING_RTSP_RTP_UDP 0
#else
# define FH_USING_PES_PACK
#endif

#ifdef RT_USING_SOFTCORE
#ifndef RT_USING_VBUS
#error "softcore depends on VBUS, you must define RT_USING_VBUS."
#endif
#endif
#define FH_DGB_DSP_PROC
#define FH_DGB_ISP_PROC

#define RT_USING_COMPONENTS_LINUX_ADAPTER

#include "platform_def.h"
#endif /* end of rtconfig.h */
