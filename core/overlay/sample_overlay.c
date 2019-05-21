#include <rtthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dfs_def.h>
#include "types/type_def.h"
#include "dsp/fh_system_mpi.h"
#include "dsp/fh_vpu_mpi.h"
#include "dsp/fh_venc_mpi.h"
#include "isp/isp_api.h"
#include "sample_common_isp.h"
#include "sample_opts.h"
#include "logo_64x64.h"
#include "FHAdv_OSD_mpi.h"
#include "osd_fontlib.h"

#ifdef FH_USING_RTSP
#include "rtsp.h"
struct rtsp_server_context *g_rtsp_server;
#else
#include "pes_pack.h"
#endif

#ifdef FH_USING_COOLVIEW
#include "dbi/dbi_over_tcp.h"
#include "dbi/dbi_over_udp.h"
static struct dbi_tcp_config tcp_conf;
#endif

enum masaic_size
{
    MASAIC_SIZE_16 = 0,
    MASAIC_SIZE_32
};

static FH_BOOL g_stop_running = FH_TRUE;
static rt_thread_t g_thread_isp;
static rt_thread_t g_thread_stream;
static FHT_OSD_CONFIG_t g_osd_config = {0};
static FHT_OSD_Logo_t g_logo_config = {0};

void sample_overlay_exit(void)
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
        rt_thread_suspend(exit_process);
#endif

    FHAdv_Osd_UnLoadFontLib(FHEN_FONT_TYPE_ASC);
    FHAdv_Osd_UnLoadFontLib(FHEN_FONT_TYPE_CHINESE);
#if defined(CONFIG_CHIP_FH8632) || defined(CONFIG_CHIP_FH8833) || defined(CONFIG_CHIP_FH8833T)|| defined(CONFIG_CHIP_FHZIYANG)
    FHAdv_Osd_Uninit();
#endif
    FH_SYS_Exit();
}

void sample_overlay_get_stream_proc(void *arg)
{
    FH_SINT32 ret, i;
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
        do
        {
            ret = FH_VENC_GetStream(0, &stream);
            if (ret == RETURN_OK)
            {
#ifdef FH_USING_RTSP
                for (i = 0; i < stream.nalu_cnt; i++)
                    rtsp_push_data(g_rtsp_server, stream.nalu[i].start, stream.nalu[i].length, stream.time_stamp);
#else
                stream_element.frame_type = stream.frame_type == I_SLICE ? VLCVIEW_ENC_H264_I_FRAME : VLCVIEW_ENC_H264_P_FRAME;
                stream_element.frame_len  = stream.length;
                stream_element.time_stamp = stream.time_stamp;
                stream_element.nalu_count = stream.nalu_cnt;
                for (i = 0; i < stream_element.nalu_count; i++)
                {
                    stream_element.nalu[i].start = stream.nalu[i].start;
                    stream_element.nalu[i].len = stream.nalu[i].length;
                }

                vlcview_pes_stream_pack(stream.chan, stream_element);
#endif
                FH_VENC_ReleaseStream(0);
            }
        } while (ret == RETURN_OK);
        rt_thread_delay(1);
#else
        ret = FH_VENC_GetStream_Block(FH_STREAM_H264, &stream);
        if (ret == RETURN_OK)
        {
#ifdef FH_USING_RTSP
            for (i = 0; i < stream.h264_stream.nalu_cnt; i++)
            {
                rtsp_push_data(g_rtsp_server, stream.h264_stream.nalu[i].start, stream.h264_stream.nalu[i].length,
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
}

FH_UINT32 argb2ayuv(FH_UINT32 A, FH_UINT32 R, FH_UINT32 G, FH_UINT32 B)
{
    FH_UINT32 Y  = (FH_UINT32)(0.299 * R + 0.587 * G + 0.114 * B);
    FH_UINT32 Cr = (FH_UINT32)((0.500 * R - 0.4187 * G - 0.0813 * B) + 128);
    FH_UINT32 Cb = (FH_UINT32)((-0.1687 * R - 0.3313 * G + 0.500 * B) + 128);

    return (((A & 0xff) << 24) | ((Y & 0xff) << 16) | ((Cb & 0xff) << 8) |
            ((Cr & 0xff)));
}

void sample_overlay_set_mask(FH_BOOL masaic_enable)
{
    FH_VPU_MASK mask_config;
    FH_SINT32 index;
    FH_SINT32 ret;

    for (index = 0; index < MAX_MASK_AREA; index++)
    {
        mask_config.mask_enable[index] = 1;
        mask_config.area_value[index].u32X = (index % 4) * 320;
        mask_config.area_value[index].u32Y = index > 3 ? 620 : 0;
        mask_config.area_value[index].u32Width = 100;
        mask_config.area_value[index].u32Height = 100;
    }

    mask_config.color = argb2ayuv(255, 0, 255, 0);  /* Green */

    if (masaic_enable)
    {
        mask_config.masaic.masaic_enable = 1;
        mask_config.masaic.masaic_size = MASAIC_SIZE_16;
    }
    else
    {
        mask_config.masaic.masaic_enable = 0;
    }

    ret = FH_VPSS_SetMask(&mask_config);
    if (ret != RETURN_OK)
    {
        printf("Error: FH_VPSS_SetMask failed with %d\n", ret);
    }
}

static FH_SINT32 load_font_lib(FHT_OSD_FontType_e type, unsigned char *font_array, int font_array_size)
{
    FH_SINT32 ret;
    FHT_OSD_FontLib_t font_lib;

    font_lib.pLibData = font_array;
    font_lib.fontSize = font_array_size;

    ret = FHAdv_Osd_LoadFontLib(type, &font_lib);
    if (ret != 0)
    {
        printf("Error: FHAdv_Osd_LoadFontLib failed, ret=%d\n", ret);
        return ret;
    }

    return 0;
}

void sample_overlay_set_osd(void)
{
    FH_SINT32 ret;

    ret = FHAdv_Osd_Init(0, 0);
    if (ret != RETURN_OK)
    {
        printf("Error: FHAdv_Osd_Init failed with %d\n", ret);
        return;
    }

    if (load_font_lib(FHEN_FONT_TYPE_ASC, asc16, sizeof(asc16)) != 0)
        return;

    if (load_font_lib(FHEN_FONT_TYPE_CHINESE, gb2312, sizeof(gb2312)) != 0)
        return;

    g_osd_config.osdColor.norcolor.fRed = 255;
    g_osd_config.osdColor.norcolor.fGreen = 255;
    g_osd_config.osdColor.norcolor.fBlue = 255;
    g_osd_config.osdColor.norcolor.fAlpha = 255;
    g_osd_config.osdColor.invcolor.fRed = 0;
    g_osd_config.osdColor.invcolor.fGreen = 0;
    g_osd_config.osdColor.invcolor.fBlue = 0;
    g_osd_config.osdColor.invcolor.fAlpha = 255;

    g_osd_config.timeOsdEnable = 1;
    g_osd_config.timeOsdPosition.pos_x = 950;
    g_osd_config.timeOsdPosition.pos_y = 950;
    g_osd_config.timeOsdFormat = 6;
    g_osd_config.timeOsdNorm = 0;
    g_osd_config.weekFlag = 1;

    g_osd_config.text01Enable = 1;
    g_osd_config.sttext01Position.pos_x = 10;
    g_osd_config.sttext01Position.pos_y = 900;
    strcpy((char *)g_osd_config.text01Info, "text01");

    g_osd_config.text02Enable = 1;
    g_osd_config.sttext02Position.pos_x = 640;
    g_osd_config.sttext02Position.pos_y = 128;

    g_osd_config.osdSize.width = 32;
    g_osd_config.osdSize.height = 32;

    FHAdv_Osd_SetText(&g_osd_config);
}

void sample_overlay_set_logo(void)
{
    g_logo_config.enable = 1;
    g_logo_config.alpha = 255;
    g_logo_config.area.fTopLeftX = 640;
    g_logo_config.area.fTopLeftY = 360;
    g_logo_config.area.fWidth = AUTO_GEN_PIC_WIDTH;
    g_logo_config.area.fHeigh = AUTO_GEN_PIC_HEIGHT;
    g_logo_config.pData = logo_data;

    FHAdv_Osd_SetLogo(&g_logo_config);
}

#ifdef FH_USING_RTSP
void force_iframe_overlay(void *param)
{
    FH_VENC_RequestIDR(0);
}
#endif

int overlay(char *target_ip, rt_uint32_t port_no)
{
    FH_VPU_SIZE vi_pic;
    FH_VPU_CHN_CONFIG chn_attr;
#if defined(CONFIG_CHIP_FH8830) || defined(CONFIG_CHIP_FH8630) || defined(CONFIG_CHIP_FH8630M)
    FH_CHR_CONFIG cfg_param;
#else
    FH_VENC_CHN_CAP cfg_vencmem;
    FH_VENC_CHN_CONFIG cfg_param;
#endif
    FH_SINT32 ret;

    if (!g_stop_running)
    {
        printf("overlay is running!\n");
        return 0;
    }

    extern void bufferInit(unsigned char *, unsigned int);
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
     step  4: configure vpss channel 0
    ******************************************/
    chn_attr.vpu_chn_size.u32Width = CH0_WIDTH;
    chn_attr.vpu_chn_size.u32Height = CH0_HEIGHT;

    ret = FH_VPSS_SetChnAttr(0, &chn_attr);
    if (ret != RETURN_OK)
    {
        printf("Error: FH_VPSS_SetChnAttr failed with %d\n", ret);
        goto err_exit;
    }

    /******************************************
     step  5: open vpss channel 0
    ******************************************/
    ret = FH_VPSS_OpenChn(0);
    if (ret != RETURN_OK)
    {
        printf("Error: FH_VPSS_OpenChn failed with %d\n", ret);
        goto err_exit;
    }

    /******************************************
     step  6: create venc channel 0
    ******************************************/
#if defined(CONFIG_CHIP_FH8830) || defined(CONFIG_CHIP_FH8630) || defined(CONFIG_CHIP_FH8630M)
    cfg_param.chn_attr.size.u32Width = CH0_WIDTH;
    cfg_param.chn_attr.size.u32Height = CH0_HEIGHT;
    cfg_param.rc_config.bitrate = CH0_BIT_RATE;
    cfg_param.rc_config.FrameRate.frame_count = CH0_FRAME_COUNT;
    cfg_param.rc_config.FrameRate.frame_time  = CH0_FRAME_TIME;
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

    ret = FH_VENC_CreateChn(0, &cfg_param);
    if (ret != RETURN_OK)
    {
        printf("Error: FH_VENC_CreateChn failed with %d\n", ret);
        goto err_exit;
    }
#else
    cfg_vencmem.support_type = FH_NORMAL_H264;
    cfg_vencmem.max_size.u32Width = CH0_WIDTH;
    cfg_vencmem.max_size.u32Height = CH0_HEIGHT;

    ret = FH_VENC_CreateChn(0, &cfg_vencmem);
    if (ret != RETURN_OK)
    {
        printf("Error: FH_VENC_CreateChn failed with %d\n", ret);
        goto err_exit;
    }

    cfg_param.chn_attr.enc_type = FH_NORMAL_H264;
    cfg_param.chn_attr.h264_attr.profile = H264_PROFILE_MAIN;
    cfg_param.chn_attr.h264_attr.i_frame_intterval = 50;
    cfg_param.chn_attr.h264_attr.size.u32Width = CH0_WIDTH;
    cfg_param.chn_attr.h264_attr.size.u32Height = CH0_HEIGHT;

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
    cfg_param.rc_attr.h264_vbr.FrameRate.frame_count = CH0_FRAME_COUNT;
    cfg_param.rc_attr.h264_vbr.FrameRate.frame_time = CH0_FRAME_TIME;

    ret = FH_VENC_SetChnAttr(0, &cfg_param);
    if (ret != RETURN_OK)
    {
        printf("Error: FH_VENC_SetChnAttr failed with %d\n", ret);
        goto err_exit;
    }
#endif
    /******************************************
     step  7: start venc channel 0
    ******************************************/
    ret = FH_VENC_StartRecvPic(0);
    if (ret != RETURN_OK)
    {
        printf("Error: FH_VENC_StartRecvPic failed with %d\n", ret);
        goto err_exit;
    }

    /******************************************
     step  8: bind vpss channel 0 with venc channel 0
    ******************************************/
    ret = FH_SYS_BindVpu2Enc(0, 0);
    if (ret != RETURN_OK)
    {
        printf("Error: FH_SYS_BindVpu2Enc failed with %d\n", ret);
        goto err_exit;
    }

    /******************************************
     step  9: init ISP, and then start ISP process thread
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
     step  10: set mask
    ******************************************/
    sample_overlay_set_mask(0);

    /******************************************
     step  11: set osd,logo by using FHAdv API
    ******************************************/
    sample_overlay_set_osd();
    sample_overlay_set_logo();

    /******************************************
     step  12: get stream, then save it to file
    ******************************************/
    g_thread_stream = rt_thread_create(
        "vlc_get_stream", (void *)sample_overlay_get_stream_proc, RT_NULL,
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

#ifdef FH_USING_RTSP
#if FH_USING_RTSP_RTP_TCP
    g_rtsp_server = rtsp_start_server(RTP_TRANSPORT_TCP, port_no);
#else
    g_rtsp_server = rtsp_start_server(RTP_TRANSPORT_UDP, port_no);
#endif
    rtsp_play_sethook(g_rtsp_server, force_iframe_overlay, NULL);
#else
    ret = vlcview_pes_init(1);
    if (ret != 0)
    {
        printf("Error: vlcview_pes_init failed with %d\n", ret);
        goto err_exit;
    }
    vlcview_pes_send_to_vlc(0, target_ip, port_no);
#endif

#ifdef FH_USING_COOLVIEW
    rt_thread_t thread_dbg;
    tcp_conf.cancel = &g_stop_running;
    tcp_conf.port = 8888;

    thread_dbg = rt_thread_find("coolview");
    if (thread_dbg == RT_NULL)
    {
        thread_dbg = rt_thread_create("coolview", (void *)tcp_dbi_thread, &tcp_conf,
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
    sample_overlay_exit();
#ifdef FH_USING_RTSP
    rtsp_stop_server(g_rtsp_server);
#else
    vlcview_pes_uninit();
    deinit_stream_pack();
#endif
    return -1;
}

int overlay_exit(void)
{
    if (!g_stop_running)
    {
        g_stop_running = FH_TRUE;
        sample_overlay_exit();
#ifdef FH_USING_RTSP
        rtsp_stop_server(g_rtsp_server);
#else
        vlcview_pes_uninit();
        deinit_stream_pack();
#endif
    }
    else
    {
        printf("overlay is not running!\n");
    }

    return 0;
}

#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT(overlay, overlay(target_ip, port));
FINSH_FUNCTION_EXPORT(overlay_exit, overlay_exit());
#endif
