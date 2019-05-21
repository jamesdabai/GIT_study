#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <rtthread.h>

#include "types/type_def.h"
#include "dsp/fh_system_mpi.h"
#include "dsp/fh_vpu_mpi.h"
#include "dsp/fh_venc_mpi.h"
#if defined(CONFIG_CHIP_FH8830) || defined(CONFIG_CHIP_FH8630) || defined(CONFIG_CHIP_FH8630M)
#include "dsp/fh_jpeg_mpi.h"
#endif
#include "isp/isp_api.h"
#include "sample_common_isp.h"
#include "bufCtrl.h"
#include "sample_opts.h"

static FH_BOOL g_stop_running = FH_TRUE;
static rt_thread_t g_thread_isp;
static rt_thread_t g_thread_stream;

extern void mjpeg_stop_server(void);

void sample_mjpeg_exit(void)
{
    rt_thread_t exit_process;

    exit_process = rt_thread_find("vlc_get_stream");
    if (exit_process)
        rt_thread_delete(exit_process);

    API_ISP_Exit();
    exit_process = rt_thread_find("vlc_isp");
    if (exit_process)
        rt_thread_delete(exit_process);

    mjpeg_stop_server();

    FH_SYS_Exit();
}

extern void mjpeg_send_stream(void *data, int size);

void sample_mjpeg_get_stream_proc(void *arg)
{
    FH_SINT32 ret = 0;
#if defined(CONFIG_CHIP_FH8830) || defined(CONFIG_CHIP_FH8630) || defined(CONFIG_CHIP_FH8630M)
    FH_JPEG_STREAM_INFO jpeg_info;
    FH_JPEG_CONFIG jpeg_config;

    jpeg_config.QP = 50;
    jpeg_config.rate = 0x70;
    jpeg_config.rotate = 0;
    FH_JPEG_Setconfig(&jpeg_config);

    while (!g_stop_running)
    {
        ret = FH_SYS_BindVpu2Jpeg(0);
        if (ret == 0)
        {
            do
            {
                ret = FH_JPEG_Getstream_Block(&jpeg_info);
            } while (ret != 0);

            if (jpeg_info.stream.size != 0)
            {
                mjpeg_send_stream(jpeg_info.stream.addr, jpeg_info.stream.size);
            }
            else
            {
                printf("jpeg stream size is zero\n");
            }
        }
        else
        {
            printf("jpeg bind failed %d\n", ret);
        }
    }
#else
    FH_VENC_STREAM stream;
    unsigned int chan,len;
    unsigned char *start;

    while (!g_stop_running)
    {
        ret = FH_VENC_GetStream_Block(FH_STREAM_MJPEG, &stream);
        if (ret == RETURN_OK)
        {
            chan = stream.mjpeg_stream.chan;
            start = stream.mjpeg_stream.start;
            len = stream.mjpeg_stream.length;
            mjpeg_send_stream(start, len);
            FH_VENC_ReleaseStream(chan);
        }
    }
#endif
}

extern int mjpeg_start_server(int port);

int mjpeg(int port)
{
    FH_VPU_SIZE vi_pic;
    FH_VPU_CHN_CONFIG chn_attr;
    FH_SINT32 ret;
#if defined(CONFIG_CHIP_FH8632) || defined(CONFIG_CHIP_FH8833) || defined(CONFIG_CHIP_FH8833T) || defined(CONFIG_CHIP_FHZIYANG)
    FH_VENC_CHN_CAP cfg_vencmem;
    FH_VENC_CHN_CONFIG cfg_param;
#endif

    if (!g_stop_running)
    {
        printf("vicview is running!\n");
        return 0;
    }

    extern int bufferInit(unsigned char *, unsigned int);
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
    vi_pic.vi_size.u32Width = CH0_WIDTH;
    vi_pic.vi_size.u32Height = CH0_HEIGHT;
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
     step  6: init ISP, and then start ISP process thread
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
     step  7: init jpeg system
    ******************************************/
#if defined(CONFIG_CHIP_FH8830) || defined(CONFIG_CHIP_FH8630) || defined(CONFIG_CHIP_FH8630M)
    ret = FH_JPEG_InitMem(JPEG_INIT_WIDTH, JPEG_INIT_HEIGHT);
    if (ret != RETURN_OK)
    {
        printf("Error: FH_JPEG_InitMem failed with %d\n", ret);
        goto err_exit;
    }
#else
    cfg_vencmem.support_type = FH_MJPEG;
    cfg_vencmem.max_size.u32Width = CH0_WIDTH;
    cfg_vencmem.max_size.u32Height = CH0_HEIGHT;
    ret = FH_VENC_CreateChn(0, &cfg_vencmem);
    if (ret != RETURN_OK)
    {
        printf("Error: FH_VENC_CreateChn failed with %d\n", ret);
        goto err_exit;
    }

    cfg_param.chn_attr.enc_type = FH_MJPEG;
    cfg_param.chn_attr.mjpeg_attr.pic_size.u32Width = CH0_WIDTH;
    cfg_param.chn_attr.mjpeg_attr.pic_size.u32Height = CH0_HEIGHT;
    cfg_param.chn_attr.mjpeg_attr.rotate = 0;
    cfg_param.chn_attr.mjpeg_attr.encode_speed = 4;/* 0-9 */

    cfg_param.rc_attr.rc_type = FH_RC_MJPEG_FIXQP;
    cfg_param.rc_attr.mjpeg_fixqp.qp = 50;
    cfg_param.rc_attr.mjpeg_fixqp.FrameRate.frame_count = CH0_FRAME_COUNT;
    cfg_param.rc_attr.mjpeg_fixqp.FrameRate.frame_time = CH0_FRAME_TIME;

    ret = FH_VENC_SetChnAttr(0, &cfg_param);
    if (ret != RETURN_OK)
    {
        printf("Error: FH_VENC_SetChnAttr failed with %d\n", ret);
        goto err_exit;
    }

    ret = FH_VENC_StartRecvPic(0);
    if (ret != RETURN_OK)
    {
        printf("Error: FH_VENC_StartRecvPic failed with %d\n", ret);
        goto err_exit;
    }

    ret = FH_SYS_BindVpu2Enc(0, 0);
    if (ret != RETURN_OK)
    {
        printf("Error: FH_SYS_BindVpu2Enc failed with %d\n", ret);
        goto err_exit;
    }
#endif

    /******************************************
     step  8: start mjpeg server thread and mjpeg stream thread,
              the server waits the clients to connect
    ******************************************/
    mjpeg_start_server(port);
    g_thread_stream =
        rt_thread_create("vlc_get_stream", sample_mjpeg_get_stream_proc,
                         &g_stop_running, 3 * 1024, RT_APP_THREAD_PRIORITY, 10);
    rt_thread_startup(g_thread_stream);

    return 0;

err_exit:
    sample_mjpeg_exit();

    return -1;
}

void mjpeg_exit(void)
{
    if (!g_stop_running)
    {
        g_stop_running = FH_TRUE;
        sample_mjpeg_exit();
    }
    else
    {
        printf("vicview is not running!\n");
    }
}

#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT(mjpeg, mjpeg(server_port));
FINSH_FUNCTION_EXPORT(mjpeg_exit, mjpeg_exit());
#endif
