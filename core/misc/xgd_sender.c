#include <rtthread.h>
#include <rtdevice.h>
#include <string.h>

#ifdef RT_USING_USB_DEVICE
#include "usb_common.h"
#include "type_def.h"
#include "time.h"
#include "xgdzbar_statics.h"
#include "sec_manage.h"
#include "xgd_config_deal.h"
#include "xgd_sender.h"
#include "public_usb.h"
#include "xgd_sn_down.h"
#include "dev_audio.h"



static struct rt_mailbox mb_sender;/* 邮箱控制块 */
static char mb_pool_hid[4];/* 用于放邮件的内存池 */


void test_udelay(void)  
{
    int i = 0;
    do{
        unsigned int btick = rt_tick_get();
        for(i = 0; i < 120; i++)
        {
            fh_udelay(500);// 
        }
        rt_kprintf("~~~UseTick(%u)\n",rt_tick_get() - btick);
        rt_thread_delay(1);
    }while(1);
}
void xgd_send_str(char *pstr)
{
    char *pbuf;
    int   len;

    len  = strlen(pstr);
    pbuf = rt_malloc(len+4);
    strncpy(pbuf,pstr,len);
    pbuf[len] = 0;
    
    rt_mb_send(&mb_sender, (rt_uint32_t)&pbuf[0]);
}

void xgd_send_binary(char *buf,int len)
{
    char *pbuf;
    int   loop;
    
    if( (len > 2048)||(len <= 0) )
    {
        return;
    }
    
    pbuf = rt_malloc(len*2+4);
    memset(pbuf,0,len*2+4);
    
    for(loop=0;loop<len;loop++)
    {
        sprintf(&pbuf[loop*2],"%02x",buf[loop]);
    }
    
    rt_mb_send(&mb_sender, (rt_uint32_t)&pbuf[0]);
}
static int com_choice = -1;
static rt_device_t uart0_dev = NULL;

void xgd_send_message(char *pbuf)
{
    if(com_choice == -1)
        return ;
    if(com_choice == SET_UART_RS232)
    {
        if(uart0_dev !=NULL)
             rt_device_write(uart0_dev, 0, pbuf, strlen(pbuf));
    }
    else if(com_choice == SET_USB_CDC_COM)
    {
        dev_usb_write(CDC_DEVICE,pbuf,strlen(pbuf));					
    }
    else if(com_choice == SET_USB_HID_MODE)
    {
        dev_usb_write(HID_DEVICE,pbuf,strlen(pbuf));
    }
    
    return ;
}
#define RT_TICK_PER_SECOND 100
static void xgd_sender_thread(void *param)
{
    int ret;
    int str_len;
    int return_newline;
    int cdc_uart_tick;//开机3秒
    int mb_timeout = RT_TICK_PER_SECOND;
	char return_yes = 0;
	char scaner_ok = 0;//解码成功标志
	char init_flag = 1;
	int voice_xgd;
	int voice_delay;
	unsigned char* str;
    unsigned char* msg_tmp;
	unsigned char *msg_ptr;//重新组织发送信息用的指针;
	unsigned char sn_len = 0;
	rt_device_t uart0_devie;

	msg_ptr = rt_malloc(500);
	if(msg_ptr == NULL)
	{
	    rt_kprintf("%s,malloc fail\r\n",__FUNCTION__);
	    return ;
	}
	
	com_choice = xgd_config_read(SET_COM_MODE);
    if(xgd_sn_exist() != 0)//没有SN则用CDC
    {
        com_choice = SET_USB_CDC_COM;
        mb_timeout = RT_WAITING_FOREVER;
    }
    else
    {
        if(xgd_config_read(SET_SCAN_CODE) == 0)
        {
            mb_timeout = RT_WAITING_FOREVER;//配置指令长打开
        }
    }
    
    if(com_choice == SET_UART_RS232)
    {
        uart0_dev = rt_device_find(UART0_NAME);
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
	else if(com_choice == SET_USB_HID_MODE)
	{
        rt_kprintf("通过HID打印\r\n");	
        dev_usb_open(HID_DEVICE);
        mb_timeout = RT_WAITING_FOREVER;//HID模式则永远等待
	}
	
    return_newline = xgd_config_read(SET_ENTER_NEWLINE);
    if(return_newline == SET_ADD_ENTER ||return_newline == SET_ADD_NEWLINE ||return_newline ==SET_ADD_EN_NEW)
    {
        return_yes = 1;
    }

    memset(msg_ptr,0,500);
    ret = xgd_info_read(SN,msg_ptr,32);
    if(ret == -1)
    {
        sn_len = strlen("sn not exist^");
        memcpy(msg_ptr,"sn not exist^",sn_len);
    }
    else
    {
        //一个客户要求,在扫码信息前添加机身号，中间通过^连接
        sn_len = ret + 1;
        msg_ptr[ret] = '^';
    }
    msg_tmp = &msg_ptr[sn_len];

	ret=dev_audio_open();
	if(ret<=0)
	{
		rt_kprintf("dev_audio_open ret=%d\n",ret);
	}


    voice_xgd  = xgd_config_read(SET_VOICE);
	voice_delay=voice_xgd?90:50;
	
    xgd_scanner_start();//启动扫描
    while (1)
    {
        if(rt_mb_recv(&mb_sender, (rt_uint32_t*)&str, mb_timeout) == RT_EOK)
        {
            xgdzbar_stat.string_send_cnt++;
			scan_ok_led_ctrl(1);
			
			if(voice_xgd == SET_VOICE_OK)
			{
				ret=dev_audio_play("sok.wav");
				if(ret!=0)
					dev_audio_play("beep.wav");
			}
			else
			{
				dev_audio_play("beep.wav");
			}
			
            ret = xgd_config_parser(str);
            if(ret==0)
            {
				rt_thread_delay(voice_delay);
				scan_ok_led_ctrl(0);
                continue;
            }
            str_len = strlen(str);
            if(!get_security_stat())
            {
                //添加安全校验后的*号
                if(str_len>6)
                {
                    memcpy(&str[str_len/2-2],"*****",5);
                }
                else
                {
                    memcpy(&str[str_len/2],'*',1);
                }
            }
            if(return_yes)
            {
                //根据配置选择回车换行符
                memcpy(&str[str_len],"\r\n",2);
                str_len  += 2;
            }
            memcpy(msg_tmp,str,str_len);
            msg_ptr[sn_len+str_len] = 0;
            xgd_send_message(msg_ptr);
            cdc_uart_tick = rt_tick_get();
            scaner_ok = 1;//表示解码成功标志
            rt_free(str);
			
			rt_thread_delay(voice_delay);
			scan_ok_led_ctrl(0);
        }
        else
        {
            if(init_flag>0)//开机3秒都保持打开的
            {
                init_flag++;
                if(init_flag>4)
                {
                    xgd_scanner_stop();
                    init_flag = 0;
                }
                else
                    continue;
            }
            if(scaner_ok)
            {
                if(rt_tick_get() - cdc_uart_tick > 200)
                {
                    xgd_scanner_stop();
                    scaner_ok = 0;
                }
            }
        }
    }
}




void xgd_sender_thread_init(void)
{
    rt_thread_t init_thread;

    /* 初始化一个mailbox */
    rt_mb_init(&mb_sender,
               "mb_sender",             
               &mb_pool_hid[0],      
               sizeof(mb_pool_hid)/4,
               RT_IPC_FLAG_FIFO);
    
    init_thread = rt_thread_create("xgd_sender",
                                   xgd_sender_thread,
                                   (void*)NULL,
                                   2048,
                                   TASK_PRIORITY_HID,
                                   20);

    if (init_thread != RT_NULL) rt_thread_startup(init_thread);
}

int hid_msh(int argc,char *argv[])
{
    if(argc > 1)
    {
        xgd_send_str(argv[1]);
    }
    else
    {
        test_udelay();
    }

    return 0;
}


#ifdef RT_USING_FINSH
#include <finsh.h>
MSH_CMD_EXPORT(hid_msh, hid_msh);
#endif

#endif
