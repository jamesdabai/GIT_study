/*
 * File      : sample_vlcview.c
 * This file is part of SOCs BSP for RT-Thread distribution.
 *
 * Copyright (c) 2017 Shanghai Fullhan Microelectronics Co., Ltd.
 * All rights reserved
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *  Visit http://www.fullhan.com to get contact with Fullhan.
 *
 * Change Logs:
 * Date           Author       Notes
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtthread.h>
#include <dfs.h>
#include <dfs_file.h>

#include "types/type_def.h"
#include "dsp/fh_system_mpi.h"
#include "dsp/fh_vpu_mpi.h"
#include "dsp/fh_venc_mpi.h"
#include "isp/isp_api.h"
#include "bufCtrl.h"
#include "sample_common_isp.h"
#include "sample_opts.h"
#include "isp_enum.h"
#include "multi_sensor.h"
#include "xgd_config_deal.h"


static FH_BOOL g_stop_running = FH_TRUE;
static rt_thread_t g_thread_isp;
static rt_thread_t g_thread_xgdzbar_img;
static rt_thread_t g_thread_vlc;

#ifdef FH_USING_COOLVIEW
#include "dbi/dbi_over_tcp.h"
#include "dbi/dbi_over_udp.h"
static struct dbi_tcp_config g_tcp_conf;
#endif

#ifdef RT_USING_LWIP
#include "pes_pack.h"
#endif
#include "time.h"

#define CHANNEL_COUNT 2

struct channel_info
{
    FH_SINT32 channel;
    FH_UINT32 width;
    FH_UINT32 height;
    FH_UINT8 frame_count;
    FH_UINT8 frame_time;
    FH_UINT32 bps;
};

static struct channel_info g_channel_infos[] = {
    {
        .channel = 0,
        .width = CH0_WIDTH,
        .height = CH0_HEIGHT,
        .frame_count = CH0_FRAME_COUNT,
        .frame_time = CH0_FRAME_TIME,
        .bps = CH0_BIT_RATE
    },
    {
        .channel = 1,
        .width = CH1_WIDTH,
        .height = CH1_HEIGHT,
        .frame_count = CH1_FRAME_COUNT,
        .frame_time = CH1_FRAME_TIME,
        .bps = CH1_BIT_RATE
    },
};


void sample_vlcview_exit(void)
{
    rt_thread_t exit_process;

    exit_process = rt_thread_find("vlc_get_stream");
    if (exit_process)
        rt_thread_delete(exit_process);

    API_ISP_Exit();
    exit_process = rt_thread_find("vlc_isp");
    if (exit_process)
        rt_thread_delete(exit_process);
    
    exit_process = rt_thread_find("vlc_get_vlc_stream");
    if (exit_process)
        rt_thread_delete(exit_process);

#ifdef FH_USING_COOLVIEW
    rt_thread_delay(100);
    exit_process = rt_thread_find("coolview");
    if (exit_process)
        rt_thread_delete(exit_process);
#endif
    FH_SYS_Exit();
}

#ifdef XGD_ENABLE_TCPIP //RT_USING_LWIP 

void sample_vlcview_get_stream_proc(void *arg)
{
    FH_SINT32 ret, i;
    FH_SINT32 chan = 0;
    FH_VENC_STREAM stream;
    FH_SINT32 *cancel = (FH_SINT32 *)arg;
    struct vlcview_enc_stream_element stream_element;
    while (!*cancel)
    {
        do
        {
            ret = FH_VENC_GetStream_Block(FH_STREAM_H264, &stream);
            if (ret == RETURN_OK)
            {
                stream_element.frame_type = stream.h264_stream.frame_type == FH_FRAME_I ? VLCVIEW_ENC_H264_I_FRAME : VLCVIEW_ENC_H264_P_FRAME;
                stream_element.frame_len  = stream.h264_stream.length;
                stream_element.time_stamp = stream.h264_stream.time_stamp;
                stream_element.nalu_count = stream.h264_stream.nalu_cnt;
                chan = stream.h264_stream.chan;
                for (i = 0; i < stream_element.nalu_count; i++)
                {
                    stream_element.nalu[i].start = stream.h264_stream.nalu[i].start;
                    stream_element.nalu[i].len = stream.h264_stream.nalu[i].length;
                }
                vlcview_pes_stream_pack(chan, stream_element);
                FH_VENC_ReleaseStream(chan);
            }
        } while (ret == RETURN_OK);
        rt_thread_delay(1);
    }
}

#endif
int zbar_decode_data(int width,int height,char *pbuf);

void API_ISP_SET(char value)
{
    API_ISP_SetSensorReg(0x04,value/*0x38*/);//深锅
}
void xgdzbar_get_stream_proc(void *arg)
{
    FH_SINT32 ret = 0;
    FH_UINT32 chan = 1;
    FH_VPU_STREAM yuv_data;
    int width = CH1_WIDTH;
    int height = CH1_HEIGHT;

   
    void *yluma = malloc(width * height);
    if (!yluma) {
        printf("Error: malloc(%d) fail\n", width * height);
        return;
    }

    int count = 0;
    unsigned long sum_time = 0;
    while (1) 
    {
        ret = FH_VPSS_GetChnFrame(chan, &yuv_data);
        if (ret != RETURN_OK) 
        {
            printf("Error: FH_VPSS_GetChnFrame failed with %d\n", ret);
            rt_thread_delay(3);
            continue;
        }
        if(!xgd_scanner_status())
        {
            continue;//解码未打开时，继续
        }
#ifdef XGD_MUTI_THREAD_DECODE
        image_frame_send(yuv_data.yluma.vbase,width*height);
        //rt_thread_delay(30);
#else  
        memcpy(yluma, yuv_data.yluma.vbase, width*height);
        xgd_decode(width,height,yluma);
#endif
    }
    free(yluma);
}

int vlcview(char *dsp_ip, rt_uint32_t port_no)
{
    FH_VPU_SIZE vi_pic;
    FH_VPU_CHN_CONFIG chn_attr;
    FH_SINT32 ret;
    FH_SINT32 i;
    FH_VENC_CHN_CAP cfg_vencmem;
    FH_VENC_CHN_CONFIG cfg_param;
    
    if (!g_stop_running)
    {
        printf("vicview is running!\n");
        return 0;
    }

    bufferInit((unsigned char *)FH_SDK_MEM_START, FH_SDK_MEM_SIZE);

    //vpu_write_proc("cap_1_640_480");
    vpu_write_proc("cap_1_800_600");
    
    /******************************************
     step  1: init media platform
    ******************************************/
    ret = FH_SYS_Init();
    if (ret != RETURN_OK)
    {
        printf("Error: FH_SYS_Init failed with %d\n", ret);
        goto err_exit;
    }

    /******************************************
     step  2: set vpss resolution
    ******************************************/
    vi_pic.vi_size.u32Width = VIDEO_INPUT_WIDTH;
    vi_pic.vi_size.u32Height = VIDEO_INPUT_HEIGHT;
    ret = FH_VPSS_SetViAttr(&vi_pic);
    if (ret != RETURN_OK)
    {
        printf("Error: FH_VPSS_SetViAttr failed with %d\n", ret);
        goto err_exit;
    }

    /******************************************
     step  3: start vpss
    ******************************************/
    ret = FH_VPSS_Enable(0);
    if (ret != RETURN_OK)
    {
        printf("Error: FH_VPSS_Enable failed with %d\n", ret);
        goto err_exit;
    }
    
    for (i = 0; i < CHANNEL_COUNT; i++)
    {
        chn_attr.vpu_chn_size.u32Width = g_channel_infos[i].width;
        chn_attr.vpu_chn_size.u32Height = g_channel_infos[i].height;
        ret = FH_VPSS_SetChnAttr(i, &chn_attr);
        if (ret != RETURN_OK)
        {
            printf("Error: FH_VPSS_SetChnAttr failed with %d\n", ret);
            goto err_exit;
        }
        
        /******************************************
         step  5: open vpss channel
        ******************************************/
        ret = FH_VPSS_OpenChn(i);
        if (ret != RETURN_OK)
        {
            printf("Error: FH_VPSS_OpenChn failed with %d\n", ret);
            goto err_exit;
        }

        if(i == 0)
        {
            FH_FRAMERATE _vpufr;
            _vpufr.frame_count = 25;
            _vpufr.frame_time  = 1;
            FH_VPSS_SetFramectrl(i,&_vpufr);
        }

        if(i == 1)
            FH_VPSS_SetVOMode(i, VPU_VOMODE_SCAN);
        
        if(i == 0)
        {
            cfg_vencmem.support_type = FH_NORMAL_H264;
            cfg_vencmem.max_size.u32Width = g_channel_infos[i].width;
            cfg_vencmem.max_size.u32Height = g_channel_infos[i].height;
            
            ret = FH_VENC_CreateChn(i, &cfg_vencmem);
            if (ret != RETURN_OK)
            {
                printf("Error: FH_VENC_CreateChn failed with %d\n", ret);
                goto err_exit;
            }
            
            cfg_param.chn_attr.enc_type = FH_NORMAL_H264;
            cfg_param.chn_attr.h264_attr.profile = H264_PROFILE_MAIN;
            cfg_param.chn_attr.h264_attr.i_frame_intterval = 50;
            cfg_param.chn_attr.h264_attr.size.u32Width = g_channel_infos[i].width;
            cfg_param.chn_attr.h264_attr.size.u32Height = g_channel_infos[i].height;
            
            cfg_param.rc_attr.rc_type = FH_RC_H264_VBR;
            cfg_param.rc_attr.h264_vbr.init_qp = 35;
            cfg_param.rc_attr.h264_vbr.bitrate = CH0_BIT_RATE;
            cfg_param.rc_attr.h264_vbr.ImaxQP = 42;
            cfg_param.rc_attr.h264_vbr.IminQP = 28;
            cfg_param.rc_attr.h264_vbr.PmaxQP = 42;
            cfg_param.rc_attr.h264_vbr.PminQP = 28;
            cfg_param.rc_attr.h264_vbr.maxrate_percent = 200;
            cfg_param.rc_attr.h264_vbr.IFrmMaxBits = 0;
            cfg_param.rc_attr.h264_vbr.IP_QPDelta = 0;
            cfg_param.rc_attr.h264_vbr.I_BitProp = 5;
            cfg_param.rc_attr.h264_vbr.P_BitProp = 1;
            cfg_param.rc_attr.h264_vbr.fluctuate_level = 0;
            cfg_param.rc_attr.h264_vbr.FrameRate.frame_count = g_channel_infos[i].frame_count;
            cfg_param.rc_attr.h264_vbr.FrameRate.frame_time = g_channel_infos[i].frame_time;
            
            ret = FH_VENC_SetChnAttr(i, &cfg_param);
            if (ret != RETURN_OK)
            {
                printf("Error: FH_VENC_SetChnAttr failed with %d\n", ret);
                goto err_exit;
            }

            ret = FH_VENC_StartRecvPic(i);
            if (ret != RETURN_OK)
            {
                printf("Error: FH_VENC_StartRecvPic failed with %d\n", ret);
                goto err_exit;
            }
            
            /******************************************
             step  8: bind vpss channel to venc channel
            ******************************************/
            ret = FH_SYS_BindVpu2Enc(i, i);
            if (ret != RETURN_OK)
            {
                printf("Error: FH_SYS_BindVpu2Enc failed with %d\n", ret);
                goto err_exit;
            }
        }
    }
    /******************************************
     step  9: init ISP, and then start ISP process thread
    ******************************************/
    g_stop_running = FH_FALSE;

    if (sample_isp_init() != 0)
    {
        goto err_exit;
    }
    //API_ISP_SetSensorReg(0x17,0x40);
    {
        u8 set_hard_config = 0;
        set_hard_config = xgd_config_read(SET_HARD_CONFIG);
        API_ISP_SetSensorReg(0x03,0x00);
        if(set_hard_config == SET_HARD_SHALLOW)
            API_ISP_SetSensorReg(0x04,0x65/*0x38*/);    /* 曝光由10改为20,为了拍微信退款码 */
        else
            API_ISP_SetSensorReg(0x04,0x28/*0x38*/);//深锅
        API_ISP_SetSensorReg(0xB1,0x0001);
        API_ISP_SetSensorReg(0xB2,0x00c0);
        API_ISP_SetSensorReg(0xB6,0x0002);  /* 数字增益调到最大 */
    }
    g_thread_isp = rt_thread_create("vlc_isp", sample_isp_proc, &g_stop_running,
                                    3 * 1024, RT_APP_THREAD_PRIORITY, 10);
    rt_thread_startup(g_thread_isp);

#ifdef XGD_ENABLE_TCPIP //RT_USING_LWIP 
    ret = vlcview_pes_init(CHANNEL_COUNT-1);
    if (ret != 0)
    {
        printf("Error: vlcview_pes_init failed with %d\n", ret);
        goto err_exit;
    }
    vlcview_pes_send_to_vlc(0, dsp_ip, port_no);
    
    /******************************************
     step  11: get stream, pack as PES stream and then send to vlc
    ******************************************/
    g_thread_vlc =
        rt_thread_create("vlc_get_vlc_stream", sample_vlcview_get_stream_proc,
                         &g_stop_running, 3 * 1024, RT_APP_THREAD_PRIORITY, 10);
    if (g_thread_vlc != RT_NULL)
    {
        rt_thread_startup(g_thread_vlc);
    }
#endif

    g_thread_xgdzbar_img =
        rt_thread_create("xgdzbar_get_stream", xgdzbar_get_stream_proc,
                         &g_stop_running, 3 * 1024, RT_APP_THREAD_PRIORITY, 10);
    if (g_thread_xgdzbar_img != RT_NULL)
    {
        rt_thread_startup(g_thread_xgdzbar_img);
    }

#ifdef XGD_ENABLE_TCPIP    
#ifdef FH_USING_COOLVIEW
    rt_thread_t thread_dbg;

    g_tcp_conf.cancel = &g_stop_running;
    g_tcp_conf.port = 8888;

    thread_dbg = rt_thread_find("coolview");
    if (thread_dbg == RT_NULL)
    {
        thread_dbg = rt_thread_create("coolview", (void *)tcp_dbi_thread, &g_tcp_conf,
                4 * 1024, RT_APP_THREAD_PRIORITY + 10, 10);
        if (thread_dbg != RT_NULL)
        {
            rt_thread_startup(thread_dbg);
        }
    }
    else
    {
        rt_thread_resume(thread_dbg);
    }
#endif
#endif

    return 0;

err_exit:
    g_stop_running = FH_TRUE;
    sample_vlcview_exit();
#ifdef XGD_ENABLE_TCPIP
    vlcview_pes_uninit();
#endif
    return -1;
}

int vlcview_exit(void)
{
    if (!g_stop_running)
    {
        g_stop_running = FH_TRUE;
        sample_vlcview_exit();
    }
    else
    {
        printf("vicview is not running!\n");
    }

    return 0;
}

int vlc(int argc,char *argv[])
{
    char *ip="10.150.130.155";
    int port=1234;

    if(argc > 1)
    {
        ip = argv[1];
    }

    if(argc > 2)
    {
        port = atoi(argv[2]);
    }

    printf("vlc %s:%d\n",ip,port);
    vlcview(ip,port);

    return 0;
}


#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT(vlcview, vlcview(dst_ip, port));
FINSH_FUNCTION_EXPORT(vlcview_exit, vlcview_exit());
MSH_CMD_EXPORT(vlc, vlc);
#endif

