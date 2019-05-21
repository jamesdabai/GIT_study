#include <rtthread.h>
#include "clock.h"

#if defined(CONFIG_CHIP_FH8830) || defined(CONFIG_CHIP_FH8630) || defined(CONFIG_CHIP_FH8630M)
#define FH_SENSOR_CLK (27*1000*1000)
#else
#define FH_SENSOR_CLK (24*1000*1000)
#endif

extern void fh_media_process_module_init(void);
extern void fh_pae_module_init(void);
extern void fh_vpu_module_init(void);
extern void fh_isp_module_init(void);
extern rt_err_t fh_bgm_module_init(void);
extern void fh_jpeg_module_init(void);
#if defined(CONFIG_CHIP_FH8830) || defined(CONFIG_CHIP_FH8630) || defined(CONFIG_CHIP_FH8630M)
extern rt_err_t fh_fd_module_init(void);
extern void fh_vou_module_init(void);
#endif
extern int pwm_demo_init(void);
extern int xgd_thread_init(void);
void gpio_ctrl_init(void);
void xgd_sender_thread_init(void);
int audio_decoder_init(void);
int security_init(void);

void xgd_dsp_init(void)
{
#ifdef RT_USING_DSP
    fh_media_process_module_init();
    fh_pae_module_init();
    fh_vpu_module_init();
    fh_bgm_module_init();
    fh_jpeg_module_init();    
#endif
}

void xgd_isp_init(void)
{
    struct fh_clk *clk;    
    clk = (struct fh_clk *)clk_get("cis_clk_out");
    if (!clk)
    {
        rt_kprintf("isp set sensor clk failed\n");
    }
    else
    {
        clk_set_rate(clk, FH_SENSOR_CLK);
    }
    fh_isp_module_init();
}
void xgd_boot_prompt(void)
{
    gpio_ctrl_init();
    backlight_led_ctrl(1);
}
void user_main(void)
{
    xgd_dsp_init();
    xgd_isp_init();
    xgd_Configuration_init();
    xgd_thread_init();
    audio_decoder_init();
    security_init();
    xgd_sn_init();
#ifdef XGD_ENABLE_TCPIP   
    //system("ifconfig e0 10.150.130.201 10.150.130.1 255.255.255.0");
    //system("vlc");
#else
    vlcview("10.150.130.155",1234);
#endif
}

