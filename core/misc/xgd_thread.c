#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <rtthread.h>
#include <rtdevice.h>
#include "xgdzbar_statics.h"
#include "public_usb.h"
#include "xgd_sn_down.h"
#include "xgd_config_deal.h"
#include "dev_audio.h"

typedef void (*console_rcv_filter_callback)(char);

static struct rt_event play_event,play_over;

static rt_thread_t decoder_tid;    /* decoder thread */
static rt_thread_t audio_player_tid;    /* audio player thread */
static rt_thread_t cdc_uart_cmd_tid;    /* audio player thread */


int beep_func(int hz, int time_ms);
void finsh_RegisterRcvCharFilter(console_rcv_filter_callback cb);
void xgd_scanner_start(void);
void xgd_scanner_stop(void);
void scan_ok_led_ctrl(int ctrl);
void xgd_send_str(char *pstr);
int audio_decoder_play_ok(char type);
void audio_decoder_play_ok_stop(void);

void xgd_decoder_thread(void *arg);

void audio_player_go(void)
{
    rt_event_send(&play_event, 1);
    xgdzbar_stat.audio_req_cnt++;
}

void audio_player_over(void)
{
    rt_event_send(&play_over, 1);

}


int audio_player_restart(void)
{
    rt_uint32_t e;
    
    if (rt_event_recv(&play_event, 1,
                                 RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
                                 RT_WAITING_NO, &e) == RT_EOK)
    {
        //rt_kprintf("audio_player_restart\r\n");
        return 1;
    }

    return 0;
}

int audio_thread_over(void)
{
    rt_uint32_t e;
    
   
        if(RT_EOK == rt_event_recv(&play_over, 1,
                      RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
                      0, &e) )
        {
            return 1;
        }
        
    return 0;
}


void audio_player_thread(void *arg)
{
    int ret;
    rt_uint32_t e;
    
    while(1)
    {
        if (rt_event_recv(&play_event, 1,
                              RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
                              RT_TICK_PER_SECOND*1000, &e) == RT_EOK)
        {
        
			audio_play_thread();
#if 0
            do
            {
                ret = audio_decoder_play_ok();
            }while(ret>0);
#endif
            xgdzbar_stat.audio_play_cnt++;
        }
    }
}

void cdc_uart_cmd_thread(void *arg)
{
    int ret;
    int sn_ret = -1;
    int com_choice;
    unsigned char buff[64];
    rt_device_t uart0_dev;
    
    sn_ret = xgd_info_read(SN,buff,32);//没有机身号之前，不打开该功能
    if(sn_ret == -1)
    {
        return ;
    }
        
    com_choice = xgd_config_read(SET_COM_MODE);
    if(com_choice == SET_UART_RS232)
    {
        uart0_dev = rt_device_find("uart0");
        if(uart0_dev !=NULL)
        {
            rt_kprintf("通过串口打印\r\n");
            rt_device_open(uart0_dev, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_STREAM);
        }
    }
    else if(com_choice == SET_USB_CDC_COM)
    {
        rt_kprintf("通过CDC打印\r\n");
        dev_usb_open(CDC_DEVICE);
    }
	else 
	{
        return ;
	}
    while(1)
    {
        memset(buff,0,64);
        if(com_choice == SET_UART_RS232)
        {
            if(uart0_dev !=NULL)
                ret = rt_device_read(uart0_dev, 0, buff, 64);
        }
        else 
        {
            
            ret = dev_usb_read(CDC_DEVICE,buff,64);		
        }
        //rt_kprintf("ret =%d,%s\r\n",ret,buff);
        if(ret>0)
        {
            if(buff[ret-1] == '\r' || buff[ret-1] == '\n' || memcmp(&buff[ret-2],"\r\n",2) == 0)
            {
                xgd_scanner_start();
            }
        }
        rt_thread_delay(10);
    }
}

int xgd_thread_init(void)
{
    xgd_sender_thread_init();
    rt_event_init(&play_event, "play_event", RT_IPC_FLAG_FIFO);
    rt_event_init(&play_over, "play_over", RT_IPC_FLAG_FIFO);
    decoder_tid       = rt_thread_create("xgd_decoder", xgd_decoder_thread,
                                          NULL, 1024*128, TASK_PRIORITY_XGDZBAR, 100);
    audio_player_tid  = rt_thread_create("audio_player", audio_player_thread,
                                          NULL, 1024, TASK_PRIORITY_AUDIO, 10);
    cdc_uart_cmd_tid  = rt_thread_create("cdc_uart_cmd", cdc_uart_cmd_thread,
                                          NULL, 1024, TASK_PRIORITY_CMD, 10);
    if((decoder_tid == NULL) || (audio_player_tid == NULL) || (cdc_uart_cmd_tid == NULL))
    {
        rt_kprintf("FETAL ERR\n");
        return -1;
    }
    rt_thread_startup(decoder_tid);
    rt_thread_startup(audio_player_tid);
    rt_thread_startup(cdc_uart_cmd_tid);

    return 0;
}


