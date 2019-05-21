#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rtthread.h>
#include <rtdef.h>
#include "types/type_def.h"
#include "dsp/fh_system_mpi.h"
#include "dsp/fh_vpu_mpi.h"
#include "dsp/fh_venc_mpi.h"
#if defined(CONFIG_CHIP_FH8830) || defined(CONFIG_CHIP_FH8630) || defined(CONFIG_CHIP_FH8630M)
#include "dsp/fh_jpeg_mpi.h"
#endif
#include "isp/isp_api.h"
#include "bufCtrl.h"
#include "sample_common_isp.h"
#include "sample_opts.h"
#include "yuv_trans.h"

#define CHANNEL_COUNT 2
#if defined(CONFIG_CHIP_FH8632) || defined(CONFIG_CHIP_FH8833) || defined(CONFIG_CHIP_FH8833T) || defined(CONFIG_CHIP_FHZIYANG)
#define VENC_JPEG_CHN 5
#endif

#ifdef FH_USING_COOLVIEW
#include "dbi/dbi_over_tcp.h"
#include "dbi/dbi_over_udp.h"
static struct dbi_tcp_config g_tcp_conf;
#endif

#ifdef FH_USING_RTSP
#include "rtsp.h"
struct rtsp_server_context *g_rtsp_server[CHANNEL_COUNT];
#else
#include "pes_pack.h"
#endif

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

static FH_BOOL g_stop_running = FH_TRUE;
static rt_thread_t g_thread_isp;
static rt_thread_t g_thread_stream;

void change_resolution(void)
{
    FH_VPU_CHN_CONFIG chn_attr;
#if defined(CONFIG_CHIP_FH8830) || defined(CONFIG_CHIP_FH8630) || defined(CONFIG_CHIP_FH8630M)
    FH_CHR_CONFIG cfg_param;
#else
    FH_VENC_CHN_CONFIG cfg_param;
#endif
    static FH_SINT32 channel_index = 0;
    FH_SINT32 ret;

    channel_index = (channel_index + 1) % 2;
    printf("Change resolution to %d x %d\n\n", g_channel_infos[channel_index].width, g_channel_infos[channel_index].height);

    ret = FH_VPSS_CloseChn(0);
    if (ret != RETURN_OK)
    {
        printf("Error: FH_VPSS_CloseChn failed with %d\n", ret);
        return;
    }

    ret = FH_VENC_StopRecvPic(0);
    if (ret != RETURN_OK)
    {
        printf("Error: FH_VENC_StopRecvPic failed with %d\n", ret);
        return;
    }

    chn_attr.vpu_chn_size.u32Width = g_channel_infos[channel_index].width;
    chn_attr.vpu_chn_size.u32Height = g_channel_infos[channel_index].height;

    ret = FH_VPSS_SetChnAttr(0, &chn_attr);
    if (ret != RETURN_OK)
    {
        printf("Error: FH_VPSS_SetChnAttr failed with %d\n", ret);
        return;
    }

    ret = FH_VPSS_OpenChn(0);
    if (ret != RETURN_OK)
    {
        printf("Error: FH_VPSS_OpenChn failed with %d\n", ret);
        return;
    }
#if defined(CONFIG_CHIP_FH8830) || defined(CONFIG_CHIP_FH8630) || defined(CONFIG_CHIP_FH8630M)
    cfg_param.chn_attr.profile = FH_PRO_MAIN;
    cfg_param.chn_attr.i_frame_intterval = 50;
    cfg_param.chn_attr.size.u32Width = g_channel_infos[channel_index].width;
    cfg_param.chn_attr.size.u32Height = g_channel_infos[channel_index].height;
    cfg_param.rc_config.bitrate = g_channel_infos[channel_index].bps;
    cfg_param.init_qp = 35;
    cfg_param.rc_config.ImaxQP = 42;
    cfg_param.rc_config.IminQP = 28;
    cfg_param.rc_config.PmaxQP = 42;
    cfg_param.rc_config.PminQP = 28;
    cfg_param.rc_config.RClevel = FH_RC_LOW;
    cfg_param.rc_config.RCmode = FH_RC_VBR;
    cfg_param.rc_config.max_delay = 8;
    cfg_param.rc_config.FrameRate.frame_count = g_channel_infos[channel_index].frame_count;
    cfg_param.rc_config.FrameRate.frame_time  = g_channel_infos[channel_index].frame_time;

    ret = FH_VENC_SetChnAttr(0, &cfg_param);
    if (ret != RETURN_OK)
    {
        printf("Error: FH_VENC_SetChnAttr failed with %d\n", ret);
        return;
    }
#else
    cfg_param.chn_attr.enc_type = FH_NORMAL_H264;
    cfg_param.chn_attr.h264_attr.profile = H264_PROFILE_MAIN;
    cfg_param.chn_attr.h264_attr.i_frame_intterval = 50;
    cfg_param.chn_attr.h264_attr.size.u32Width = g_channel_infos[channel_index].width;
    cfg_param.chn_attr.h264_attr.size.u32Height = g_channel_infos[channel_index].height;

    cfg_param.rc_attr.rc_type = FH_RC_H264_VBR;
    cfg_param.rc_attr.h264_vbr.init_qp = 35;
    cfg_param.rc_attr.h264_vbr.bitrate = g_channel_infos[channel_index].bps;
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
    cfg_param.rc_attr.h264_vbr.FrameRate.frame_count = g_channel_infos[channel_index].frame_count;
    cfg_param.rc_attr.h264_vbr.FrameRate.frame_time  = g_channel_infos[channel_index].frame_time;

    ret = FH_VENC_SetChnAttr(0, &cfg_param);
    if (ret != RETURN_OK)
    {
        printf("Error: FH_VENC_SetChnAttr failed with %d\n", ret);
        return;
    }
#endif
    ret = FH_VENC_StartRecvPic(0);
    if (ret != RETURN_OK)
    {
        printf("Error: FH_VENC_StartRecvPic failed with %d\n", ret);
        return;
    }

    ret = FH_SYS_BindVpu2Enc(0, 0);
    if (ret != RETURN_OK)
    {
        printf("Error: FH_SYS_BindVpu2Enc failed with %d\n", ret);
        return;
    }
}

void change_fps(void)
{
    FH_SINT32 fps;
    FH_SINT32 ret;

    API_ISP_Pause();

    ret = FH_VPSS_Disable();
    if (ret != RETURN_OK)
    {
        printf("Error: FH_VPSS_Disable failed with %d\n", ret);
        return;
    }

    fps = sample_isp_change_fps();

    ret = FH_VPSS_Enable(0);
    if (ret != RETURN_OK)
    {
        printf("Error: FH_VPSS_Enable failed with %d\n", ret);
        return;
    }

    API_ISP_Resume();

    printf("Change current fps to %d\n\n", fps);
}

void rotate(void)
{
#if defined(CONFIG_CHIP_FH8830) || defined(CONFIG_CHIP_FH8630) || defined(CONFIG_CHIP_FH8630M)
    FH_ROTATE rotate_info;
#else
    FH_ROTATE_OPS rotate_info;
#endif
    FH_SINT32 channel = 0;
    static FH_UINT32 rotate_value;
    FH_SINT32 ret;

    rotate_value = (rotate_value + 1) % 4;
#if defined(CONFIG_CHIP_FH8830) || defined(CONFIG_CHIP_FH8630) || defined(CONFIG_CHIP_FH8630M)
    rotate_info.enable = 1;
    rotate_info.rotate = rotate_value;
    ret = FH_VENC_SetRotate(channel, &rotate_info);
#else
    rotate_info = rotate_value;
    ret = FH_VENC_SetRotate(channel, rotate_info);
#endif
    if (ret != RETURN_OK)
    {
        printf("Error: FH_VENC_SetRotate failed with %d\n", ret);
    }

    printf("Rotate to %d degree\n\n", rotate_value * 90);
}

void capture_jpeg(void)
{
#if defined(CONFIG_CHIP_FH8830) || defined(CONFIG_CHIP_FH8630) || defined(CONFIG_CHIP_FH8630M)
    FH_JPEG_CONFIG jpeg_c;
    FH_JPEG_STREAM_INFO jpeg_stream = {0};
    FH_SINT32 channel = 0;
    FH_SINT32 ret;
    static FH_SINT32 jpeg_inited;

    if (!jpeg_inited)
    {
        ret = FH_JPEG_InitMem(JPEG_INIT_WIDTH, JPEG_INIT_HEIGHT);
        if (ret != RETURN_OK)
        {
            printf("Error: FH_JPEG_InitMem failed with %d\n", ret);
            return;
        }
        jpeg_inited = 1;
    }

    jpeg_c.QP = 50;
    jpeg_c.rate = 0x70;
    jpeg_c.rotate = 0;
    ret = FH_JPEG_Setconfig(&jpeg_c);
    if (ret != RETURN_OK)
    {
        printf("Error: FH_JPEG_Setconfig failed with %d\n", ret);
        return;
    }

    ret = FH_SYS_BindVpu2Jpeg(channel);
    if (ret == RETURN_OK)
    {
        ret = FH_JPEG_Getstream_Block(&jpeg_stream);
        if (ret == RETURN_OK)
        {
            if (jpeg_stream.stream.size != 0)
                printf("Capture JPEG OK, size=%d\n\n", jpeg_stream.stream.size);
        }
        else
        {
            printf("Error: FH_JPEG_Getstream_Block failed with %d\n", ret);
            return;
        }
    }
    else
    {
        printf("Error: FH_SYS_BindVpu2Jpeg failed with %d\n", ret);
        return;
    }
#else
    int ret;
    FH_VENC_STREAM stream;
    unsigned int chan, len, frametype;
    unsigned char *start;
    FH_VENC_CHN_CONFIG enc_cfg_param;

    enc_cfg_param.chn_attr.enc_type = FH_JPEG;
    enc_cfg_param.chn_attr.jpeg_attr.qp = 50;
    enc_cfg_param.chn_attr.jpeg_attr.rotate = 0;
    enc_cfg_param.chn_attr.jpeg_attr.encode_speed = 4;

    ret = FH_VENC_SetChnAttr(VENC_JPEG_CHN, &enc_cfg_param);
    if (ret != RETURN_OK)
    {
        printf("Error: FH_VENC_SetChnAttr failed with %d\n", ret);
        return;
    }

    ret = FH_SYS_BindVpu2Enc(0, VENC_JPEG_CHN);
    if (ret != RETURN_OK)
    {
         printf("Error: FH_SYS_BindVpu2Enc failed with %d\n", ret);
         return;
    }

    ret = FH_VENC_GetStream_Block(FH_STREAM_JPEG, &stream);
    if (ret == RETURN_OK)
    {
        chan = stream.jpeg_stream.chan;
        start = stream.jpeg_stream.start;
        len = stream.jpeg_stream.length;
        FH_VENC_ReleaseStream(chan);
        printf("Capture JPEG OK, size=%d\n\n", len);
    }
#endif
}

void toggle_freeze(void)
{
    static FH_SINT32 on;
    FH_SINT32 ret;

    on = !on;
    if (on)
    {
        printf("Freeze Video\n\n");
        ret = FH_VPSS_FreezeVideo();
        if (ret != RETURN_OK)
        {
            printf("Error: FH_VPSS_FreezeVideo failed with %d\n", ret);
        }
    }
    else
    {
        printf("Unfreeze Video\n\n");
        ret = FH_VPSS_UnfreezeVideo();
        if (ret != RETURN_OK)
        {
            printf("Error: FH_VPSS_UnfreezeVideo failed with %d\n", ret);
        }
    }
}

void capture_yuv(void)
{
    FH_VPU_STREAM yuv_data;
    FH_UINT8 *dst;
    FH_SINT32 ret;
    FH_SINT32 chan = 1;

    dst = rt_malloc(g_channel_infos[chan].width * g_channel_infos[chan].height * 3 / 2);
    if (dst == NULL)
    {
        printf("Error: failed to allocate yuv transform buffer\n");
        return;
    }

    ret = FH_VPSS_GetChnFrame(chan, &yuv_data);
    if (ret == RETURN_OK)
    {
        yuv_trans(yuv_data.yluma.vbase, yuv_data.chroma.vbase, dst,
                      g_channel_infos[chan].width, g_channel_infos[chan].height);
        printf("GET CHN %d YUV DATA w:%d h:%d OK\n\n", chan,
               g_channel_infos[chan].width, g_channel_infos[chan].height);
    }
    else
    {
        printf("Error: FH_VPSS_GetChnFrame failed with %d\n\n", ret);
    }
    rt_free(dst);
}

void sample_venc_exit(void)
{
    rt_thread_t exit_process;

    exit_process = rt_thread_find("vlc_get_stream");
    if (exit_process)
        rt_thread_delete(exit_process);

    API_ISP_Exit();
    exit_process = rt_thread_find("vlc_isp");
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

#ifdef FH_USING_RTSP
void force_iframe_enc(void *param)
{
    int *p_chan = (int *)param;
    FH_VENC_RequestIDR(*p_chan);
}
#endif

void sample_venc_get_stream_proc(void *arg)
{
    int i;
    FH_SINT32 ret;
#if defined(CONFIG_CHIP_FH8830) || defined(CONFIG_CHIP_FH8630) || defined(CONFIG_CHIP_FH8630M)
    FH_ENC_STREAM_ELEMENT stream;
#else
    FH_VENC_STREAM stream;
    unsigned int chan;
#endif

#ifndef FH_USING_RTSP
    struct vlcview_enc_stream_element stream_element;
#endif

    while (!g_stop_running)
    {
#if defined(CONFIG_CHIP_FH8830) || defined(CONFIG_CHIP_FH8630) || defined(CONFIG_CHIP_FH8630M)
        for (i = 0; i < CHANNEL_COUNT; i++)
        {
            do
            {
                ret = FH_VENC_GetStream(i, &stream);
                if (ret == RETURN_OK)
                {
                    int j;
#ifdef FH_USING_RTSP
                    for (j = 0; j < stream.nalu_cnt; j++)
                        rtsp_push_data(g_rtsp_server[stream.chan], stream.nalu[j].start, stream.nalu[j].length, stream.time_stamp);
#else
                    stream_element.frame_type = stream.frame_type == I_SLICE ? VLCVIEW_ENC_H264_I_FRAME : VLCVIEW_ENC_H264_P_FRAME;
                    stream_element.frame_len  = stream.length;
                    stream_element.time_stamp = stream.time_stamp;
                    stream_element.nalu_count = stream.nalu_cnt;
                    for (j = 0; j < stream_element.nalu_count; j++)
                    {
                        stream_element.nalu[j].start = stream.nalu[j].start;
                        stream_element.nalu[j].len = stream.nalu[j].length;
                    }

                    vlcview_pes_stream_pack(stream.chan, stream_element);
#endif
                    FH_VENC_ReleaseStream(i);
                }
            } while (ret == RETURN_OK);
        }
        rt_thread_delay(1);
#else
        ret = FH_VENC_GetStream_Block(FH_STREAM_H264, &stream);
        if (ret == RETURN_OK)
        {
#ifdef FH_USING_RTSP
            chan = stream.h264_stream.chan;
            for (i = 0; i < stream.h264_stream.nalu_cnt; i++)
            {
                rtsp_push_data(g_rtsp_server[chan], stream.h264_stream.nalu[i].start, stream.h264_stream.nalu[i].length,
                    stream.h264_stream.time_stamp);
            }
#else
            stream_element.frame_type = stream.h264_stream.frame_type == FH_FRAME_I ? VLCVIEW_ENC_H264_I_FRAME : VLCVIEW_ENC_H264_P_FRAME;
            stream_element.frame_len = stream.h264_stream.length;
            stream_element.time_stamp = stream.h264_stream.time_stamp;
            stream_element.nalu_count = stream.h264_stream.nalu_cnt;
            chan = stream.h264_stream.chan;
            for (i = 0; i < stream_element.nalu_count; i++)
            {
                stream_element.nalu[i].start = stream.h264_stream.nalu[i].start;
                stream_element.nalu[i].len = stream.h264_stream.nalu[i].length;
            }
            vlcview_pes_stream_pack(chan, stream_element);
#endif
            FH_VENC_ReleaseStream(chan);
        }
#endif
    }
    return;
}

void print_func_help(void)
{
    printf("Available function:\n");
    printf("   venc_func(\"h\")    print help info\n");
    printf("   venc_func(\"w\")    change resolution\n");
    printf("   venc_func(\"e\")    change frame rate\n");
    printf("   venc_func(\"r\")    rotate\n");
    printf("   venc_func(\"j\")    capture jpeg\n");
    printf("   venc_func(\"f\")    toggle freeze/unfreeze\n");
    printf("   venc_func(\"y\")    capture yuv data\n");
}

int venc(char *target_ip, rt_uint32_t port_no)
{
    FH_VPU_SIZE vi_pic;
    FH_VPU_CHN_CONFIG chn_attr;
#if defined(CONFIG_CHIP_FH8830) || defined(CONFIG_CHIP_FH8630) || defined(CONFIG_CHIP_FH8630M)
    FH_CHR_CONFIG cfg_param;
#else
    FH_VENC_CHN_CAP cfg_vencmem;
    FH_VENC_CHN_CONFIG cfg_param;
#endif
    FH_SINT32 i;
    FH_SINT32 ret;

    if (!g_stop_running)
    {
        printf("vicview is running!\n");
        return 0;
    }

    bufferInit((unsigned char *)FH_SDK_MEM_START, FH_SDK_MEM_SIZE);

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

    /******************************************
     step  4: create venc channels
    ******************************************/

    for (i = 0; i < CHANNEL_COUNT; i++)
    {
        /******************************************
         step 4.1 configure vpss channel
        ******************************************/
        chn_attr.vpu_chn_size.u32Width = g_channel_infos[i].width;
        chn_attr.vpu_chn_size.u32Height = g_channel_infos[i].height;
        ret = FH_VPSS_SetChnAttr(i, &chn_attr);
        if (ret != RETURN_OK)
        {
            printf("Error: FH_VPSS_SetChnAttr failed with %d\n", ret);
            goto err_exit;
        }

        /******************************************
         step 4.2 open vpss channel
        ******************************************/
        ret = FH_VPSS_OpenChn(i);
        if (ret != RETURN_OK)
        {
            printf("Error: FH_VPSS_OpenChn failed with %d\n", ret);
            goto err_exit;
        }

        /******************************************
         step 4.3 create venc channel
        ******************************************/
#if defined(CONFIG_CHIP_FH8830) || defined(CONFIG_CHIP_FH8630) || defined(CONFIG_CHIP_FH8630M)
        cfg_param.chn_attr.size.u32Width = g_channel_infos[i].width;
        cfg_param.chn_attr.size.u32Height = g_channel_infos[i].height;
        cfg_param.rc_config.bitrate = g_channel_infos[i].bps;
        cfg_param.rc_config.FrameRate.frame_count = g_channel_infos[i].frame_count;
        cfg_param.rc_config.FrameRate.frame_time  = g_channel_infos[i].frame_time;
        cfg_param.chn_attr.profile = FH_PRO_MAIN;
        cfg_param.chn_attr.i_frame_intterval = 50;
        cfg_param.init_qp = 35;
        cfg_param.rc_config.ImaxQP = 42;
        cfg_param.rc_config.IminQP = 28;
        cfg_param.rc_config.PmaxQP = 42;
        cfg_param.rc_config.PminQP = 28;
        cfg_param.rc_config.RClevel = FH_RC_LOW;
        cfg_param.rc_config.RCmode = FH_RC_VBR;
        cfg_param.rc_config.max_delay = 8;
        ret = FH_VENC_CreateChn(i, &cfg_param);
        if (ret != RETURN_OK)
        {
            printf("Error: FH_VENC_CreateChn failed with %d\n", ret);
            goto err_exit;
        }
#else
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
        cfg_param.rc_attr.h264_vbr.bitrate = g_channel_infos[i].bps;
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
        cfg_param.rc_attr.h264_vbr.FrameRate.frame_time  = g_channel_infos[i].frame_time;

        ret = FH_VENC_SetChnAttr(i, &cfg_param);
        if (ret != RETURN_OK)
        {
            printf("Error: FH_VENC_SetChnAttr failed with %d\n", ret);
            goto err_exit;
        }
#endif
        /******************************************
         step 4.4 start venc channel
        ******************************************/
        ret = FH_VENC_StartRecvPic(i);
        if (ret != RETURN_OK)
        {
            printf("Error: FH_VENC_StartRecvPic failed with %d\n", ret);
            goto err_exit;
        }

        /******************************************
         step 4.5 bind vpss channel with venc channel
        ******************************************/
        ret = FH_SYS_BindVpu2Enc(i, i);
        if (ret != RETURN_OK)
        {
            printf("Error: FH_SYS_BindVpu2Enc failed with %d\n", ret);
            goto err_exit;
        }
    }

#if defined(CONFIG_CHIP_FH8632) || defined(CONFIG_CHIP_FH8833) || defined(CONFIG_CHIP_FH8833T) || defined(CONFIG_CHIP_FHZIYANG)
    /******************************************
     initialize jpeg channel
    ******************************************/
    FH_VENC_CHN_CAP enc_chn_cap;

    enc_chn_cap.support_type = FH_JPEG;
    enc_chn_cap.max_size.u32Width = JPEG_INIT_WIDTH;
    enc_chn_cap.max_size.u32Height = JPEG_INIT_HEIGHT;
    ret = FH_VENC_CreateChn(VENC_JPEG_CHN, &enc_chn_cap);
    if (ret != RETURN_OK)
    {
        printf("Error: FH_VENC_CreateChn failed with %d\n", ret);
        goto err_exit;
    }
#endif

    /******************************************
     step  5: init ISP, and then start ISP process thread
    ******************************************/
    g_stop_running = FH_FALSE;

    if (sample_isp_init() != 0)
    {
        goto err_exit;
    }
    g_thread_isp = rt_thread_create("vlc_isp", sample_isp_proc, &g_stop_running,
                                    3 * 1024, RT_APP_THREAD_PRIORITY, 10);
    rt_thread_startup(g_thread_isp);

    /******************************************
     step  6: get stream, then save it to file
    ******************************************/
    g_thread_stream =
        rt_thread_create("vlc_get_stream", sample_venc_get_stream_proc, RT_NULL,
                         3 * 1024, RT_APP_THREAD_PRIORITY, 20);
    if (g_thread_stream != RT_NULL)
    {
        rt_thread_startup(g_thread_stream);
    }
    else
    {
        printf("error: vlc_get_stream creat failed...\n");
        goto err_exit;
    }

    /******************************************
     step  7: initialize stream packaging component
    ******************************************/
#ifdef FH_USING_RTSP
    for (i = 0; i < CHANNEL_COUNT; i++)
    {
#if FH_USING_RTSP_RTP_TCP
        g_rtsp_server[i] = rtsp_start_server(RTP_TRANSPORT_TCP, port_no + i * 2);
#else
        g_rtsp_server[i] = rtsp_start_server(RTP_TRANSPORT_UDP, port_no + i * 2);
#endif
        rtsp_play_sethook(g_rtsp_server[i], force_iframe, &g_channel_infos[i].channel);
    }
#else
    ret = vlcview_pes_init(CHANNEL_COUNT);
    if (ret != 0)
    {
        printf("Error: vlcview_pes_init failed with %d\n", ret);
        goto err_exit;
    }
    for (i = 0; i < CHANNEL_COUNT; i++)
        vlcview_pes_send_to_vlc(i, target_ip, port_no + i);
#endif

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

    return 0;

err_exit:
    g_stop_running = FH_TRUE;
    sample_venc_exit();
#ifdef FH_USING_RTSP
    for (i = 0; i < CHANNEL_COUNT; i++)
        rtsp_stop_server(g_rtsp_server[i]);
#else
    vlcview_pes_uninit();
    deinit_stream_pack();
#endif
    return -1;
}

int venc_exit(void)
{
    if (!g_stop_running)
    {
        g_stop_running = FH_TRUE;
        sample_venc_exit();
#ifdef FH_USING_RTSP
        int i;
        for (i = 0; i < CHANNEL_COUNT; i++)
            rtsp_stop_server(g_rtsp_server[i]);
#else
        vlcview_pes_uninit();
        deinit_stream_pack();
#endif
    }
    else
    {
        printf("vicview is not running!\n");
    }

    return 0;
}

void venc_func(char *cmd)
{
    if (strcmp(cmd, "h") == 0)
    {
        print_func_help();
    }
    else if (strcmp(cmd, "w") == 0)
    {
        change_resolution();
    }
    else if (strcmp(cmd, "e") == 0)
    {
        change_fps();
    }
    else if (strcmp(cmd, "r") == 0)
    {
        rotate();
    }
    else if (strcmp(cmd, "j") == 0)
    {
        capture_jpeg();
    }
    else if (strcmp(cmd, "f") == 0)
    {
        toggle_freeze();
    }
    else if (strcmp(cmd, "y") == 0)
    {
        capture_yuv();
    }
    else
    {
        printf("Invalid function cmd !\n");
        print_func_help();
    }
}

#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT(venc, venc(target_ip, port));
FINSH_FUNCTION_EXPORT(venc_exit, venc_exit());
FINSH_FUNCTION_EXPORT(venc_func, venc_func(cmd); use venc_func("h") for more info);
#endif
