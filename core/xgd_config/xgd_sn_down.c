#include <rtthread.h>
#include "xgd_sn_down.h"
#include "public_usb.h"
#include "xgd_config_deal.h"

#define MQ_POOL_SIZE 100
static u8 mes_get_info = 0 ;//mes系统通过命令来获取当前扫码的数据，便于工厂测试扫码数据
struct rt_messagequeue info_to_mes_mq;//消息列队
static char *info_to_mes_pool;/* 用于放消息的内存池 */

static rt_thread_t sn_tid;     /* config & control thread */
struct _strFrame01 Frame01;//帧结构体
struct sParaseFrameCtrl ParaseFrameCtrl;//控制解析帧过程
extern char sn[32];
s32 xgd_sn_exist(void)
{
    s32 ret;
    u8 sn_buff[32];
    ret = xgd_info_read(SN,sn_buff,32);
    if(ret == -1)
    {
        //rt_kprintf("SN号读取失败 %s\r\n",__FUNCTION__);
        return -1;
    }
    //rt_kprintf("SN号读取成功 %s \r\n",__FUNCTION__);
    return 0;
}
s32 xgd_info_write(u8 type,u8 *data,u32 len)
{
    u8 i;
    u8 sum = 0;
    u8 max_len;
    u8 size = 0;
    u32 addr;
    s32 ret;
    u8 info_buff[CIPHERTEXT_SIZE];
    
    if(type == SN)
    {
        addr = SN_ADDR;
        max_len = SN_SIZE - 4;
        size = SN_SIZE;
    }
    else if(type == CIPHERTEXT)
    {
        addr = CIPHERTEXT_ADDR;
        max_len = CIPHERTEXT_SIZE - 4;
        size = CIPHERTEXT_SIZE;
    }
    else
    {
        return -1;
    }
    if(len > max_len)
    {
        return -1;
    }
    memset(info_buff,0,size);
    memcpy(info_buff,"XGD",3);
    info_buff[3] = len;
    memcpy(&info_buff[4],data,len);
    for(i=0;i<size-1;i++)
    {
        sum += info_buff[i];
    }
    info_buff[size-1] = sum;
    ret = spi_flash_write(addr,info_buff,size);
    if(ret != size)
    {
        return -1;
    }
    return 0;
}

s32 xgd_info_read(u8 type,u8 *data,u32 lenth)
{
    u8 i;
    u8 sum = 0;
    u8 len;
    u8 size;
    u32 addr;
    s32 ret;
    u8 info_buff[CIPHERTEXT_SIZE];
    
    if(type == SN)
    {
        addr = SN_ADDR;
        size = SN_SIZE;
    }
    else if(type == CIPHERTEXT)
    {
        addr = CIPHERTEXT_ADDR;
        size = CIPHERTEXT_SIZE;
    }
    else
    {
        return -1;
    }
    
    ret = spi_flash_read(addr,info_buff,size);
    if(ret != size)
    {
        return -1;
    }
        
    for(i=0;i<size-1;i++)
    {
        sum += info_buff[i];
    }
    if(memcmp("XGD",info_buff,3) == 0 && sum == info_buff[size-1])
    {
        len = info_buff[3];
        memcpy(data,&info_buff[4],len);
        return len;
    }
    else
    {
        return -1;
    }
}
s32 xgd_info_erase(u8 type)
{
    s32 ret;
    u8 size;
    u32 addr;
    u8 info_buff[CIPHERTEXT_SIZE];
    
    if(type == SN)
    {
        addr = SN_ADDR;
        size = SN_SIZE;
    }
    else if(type == CIPHERTEXT)
    {
        addr = CIPHERTEXT_ADDR;
        size = CIPHERTEXT_SIZE;
    }
    else
    {
        return -1;
    }
    
    memset(info_buff,0,size);
    ret = spi_flash_write(addr,info_buff,size);
    if(ret != size)
    {
        return -1;
    }
    return 0;
}

void mes_get_info_start(void)
{
    mes_get_info = 1;
}
void mes_get_info_stop(void)
{
    mes_get_info = 0;
}
s32 mes_get_info_status(void)
{
    return mes_get_info;
}
void info_to_mes_sender(u8 *ptr,u32 size)
{   
    rt_mq_send(&info_to_mes_mq,ptr,size);
}
void info_to_mes_voice(void)
{
    int voice_xgd;
    int voice_delay;
    int ret;
    voice_xgd  = xgd_config_read(SET_VOICE);
    voice_delay=voice_xgd?90:50;
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
    rt_thread_delay(voice_delay);
	scan_ok_led_ctrl(0);
}
s32 frame_01_send(u8 Frame, u8 Cmd, u16 Len, u8*Src)
{
    s32 ret;
    u8 bcc;
    u8 buff[5+1+1+2] = {SOH,SOH,SOH,SOH,SOH,0x00,0x00,0x00,0x00};
    u16 index;

    bcc = SOH;
    buff[5] = Frame;
    buff[6] = Cmd;
    buff[7] = Len/256;
    buff[8] = Len%256;

    bcc += buff[5];
    bcc += buff[6];
    bcc += buff[7];
    bcc += buff[8];
    for(index = 0; index < Len; index++)
    {
        bcc += *(Src+index);
    }
    
    ret = dev_usb_write(CDC_DEVICE, buff, 9);
    ret = dev_usb_write(CDC_DEVICE, Src, Len);
    buff[0] = 0x03;
    buff[1] = bcc + 0x03;
    ret = dev_usb_write(CDC_DEVICE, buff, 2);
    return 0;
}
s32 frame_01_open(void)
{   
    ParaseFrameCtrl.Step = 0;
    Frame01.Valid = 0;

    Frame01.buf = (u8 *)rt_malloc(FRAME_O1_DATA_MAX);
    if(Frame01.buf == NULL)
    {
        return 1;
    }
}

s32 frame_01_close(void)
{
    if(Frame01.buf != NULL)
    {
        rt_free(Frame01.buf);
    }
}
s32 pte_deal_01_frame(u8 Cmd, u16 Len, u8* Src)
{
    s32 ret;
    u8 status = 0;
	u8 check_buff[60];
    switch(Cmd)
    {
        case PTE_GET_M_NUM://获取机型号与硬件版本号
            frame_01_send(PTE_LTEXT, PTE_GET_M_NUM, 2, "\xAB\x00");
            break;
        case PTE_DOWNLOAD_POSSN://下载机身号
        {
            u8 w_len;
            u8 w_buff[32];
            status = 0;
            
            memset(w_buff,0x00,sizeof(w_buff));
            w_len = strlen((char *)Src);
            if(w_len > Len)
                w_len = Len;
            memcpy(w_buff,Src,w_len);
            if( w_len > POSSN_MAX_SIZE)
            {
                status = 0x01;
                frame_01_send(PTE_LTEXT, PTE_DOWNLOAD_POSSN, 1, &status);
                break;
            }
            ret = xgd_info_write(SN,w_buff,w_len);
            if(ret != 0)
            {
                status = 0x01;
            }
            else
            {
                xgd_sn_init();
            }
            injection_key();//注入秘钥功能
            frame_01_send(PTE_LTEXT, PTE_DOWNLOAD_POSSN, 1, &status);
        }break;
		case PTE_CHECK_VERSION://check version
		{
		    extern char uboot[20];
            extern char recovery[20];
            extern const char core_ver[];
           
            memset(check_buff,0x00,sizeof(check_buff));
            
            memcpy(check_buff,uboot,17);
            
            memcpy(&check_buff[17],recovery,17);
            
            memcpy(&check_buff[34],core_ver,17);
			
			frame_01_send(PTE_LTEXT, PTE_CHECK_VERSION,51,check_buff);
        }break;
        case PTE_GET_QR_INFO://mes系统通过命令来获取扫码数据，节省工厂组装测试时间
        {
            u8 *buff;
            u32 info_len = 0;
            buff = rt_malloc(MQ_POOL_SIZE);
            if(buff == NULL)
            {
                rt_kprintf("%s,malloc fail\r\n",__FUNCTION__);
                return ;
            }
            memset(buff,0,MQ_POOL_SIZE);
            if(rt_mq_recv(&info_to_mes_mq,buff,MQ_POOL_SIZE,1000) == RT_EOK)//等10s钟
            {
                
                info_len = strlen(buff);
                //rt_kprintf("%s,mes获取命令成功info =%s,len=%d\r\n",__FUNCTION__,buff,info_len);
                frame_01_send(PTE_LTEXT, PTE_GET_QR_INFO,info_len,buff);
            }
            else
            {
                rt_kprintf("%s,消息列队等待超时\r\n",__FUNCTION__);
            }
            rt_free(buff);
         	break;
        }
		case PTE_CHECK_CUSTOM:
            
         	break;
		case PTE_CONTROL_RESET:
			//status = 0;
			//frame_01_send(PTE_LTEXT, PTE_CONTROL_RESET, 1,&status);
            //sdk_delay_ms(200);
			//ddi_sys_poweroff();
			break;
        default:
            status = 0x01;
            frame_01_send(PTE_LTEXT, 0xee, 1, &status);
            //uart_printf("PTE不处理命令\r\n");
            break;
    }
}

s32 frame_01_deal(void)
{
    s32 ret = -1;
    if(Frame01.Valid == 1)
    {
        switch(Frame01.frame)
        {
            case PTE_LTEXT:
                mes_get_info_start();
                ret = pte_deal_01_frame(Frame01.cmd, Frame01.len, Frame01.buf);
                mes_get_info_stop();
                break;

            case KEY_LTEXT:
                //uart_printf("键盘扩展 01帧: %02x, %d\r\n", Frame01.cmd, Frame01.len);
                break;

            case KEY_ORDER:
                //uart_printf("键盘命令 01帧: %02x, %d\r\n", Frame01.cmd, Frame01.len);
                break;

            case KEY_TEXT:
                //uart_printf("键盘文本 01帧: %02x, %d\r\n", Frame01.cmd, Frame01.len);
                break;

            case TEXTEX:
				//ret = pk_deal_01_frame(Frame01.cmd, Frame01.len, Frame01.buf);
                break;

            default:
                //uart_printf("其他类型01帧 %02x, %d\r\n", Frame01.cmd, Frame01.len);
                break;
        }
        Frame01.Valid = 0; 
    }
    return ret;
}

/**
 *@brief:     parserxdata
 *@details:   解析01协议
 *@param[in]  u8 *Src  
              u32 Len  
 *@param[out] 无
 *@retval:    static
 */
 
s32 frame_01_parse(u8 *Src, s32 Len)
{
    u8 data;
    s32 index = 0;
    memset(Frame01.buf,0,FRAME_O1_DATA_MAX);
    while((index < Len) && (Frame01.Valid == 0))    
    {
        data = *(Src + index);
        index++;
        
        switch(ParaseFrameCtrl.Step)
        {
            case 0:
                if(data == SOH)
                {
                     ParaseFrameCtrl.SohCnt++;  
                     if(ParaseFrameCtrl.SohCnt >= 0x03)
                     {
                        ParaseFrameCtrl.Step  = 1;
                        ParaseFrameCtrl.SohCnt = 0;
                     }
                }
                else
                {
                    ParaseFrameCtrl.SohCnt = 0;    
                }
                break;

            case 1:
                if((data == 0x45))
                {
                    ParaseFrameCtrl.Bcc = SOH + data;
                    Frame01.frame = data;
                    ParaseFrameCtrl.Step  = 7;     //LTEXT

                }
                else if(data == SOH)
                {
                    //重复SOH，不处理
                }
                else
                {
                    //uart_printf("未知01帧\r\n");
                    ParaseFrameCtrl.Step  = 0;
                }
                break;
            case 7: //接收到扩展帧正文标示
                    ParaseFrameCtrl.Step  = 8;
                    ParaseFrameCtrl.Bcc += data;
                    Frame01.cmd = data;
                    break;
            case 8:  //接收扩展帧的长度的高位字节
                    ParaseFrameCtrl.Step  = 9;
                    ParaseFrameCtrl.Bcc += data;
                    Frame01.len = (u16)data << 8;
                    break;
            case 9:  //接收扩展域的长度的低位字节
                    ParaseFrameCtrl.Bcc += data; 
                    Frame01.len += data;
					ParaseFrameCtrl.Cnt = 0;
                    //uart_printf("rec ltext len:%d\r\n", serialrx.s_len);
                    if( Frame01.len > FRAME_O1_DATA_MAX )
                    {
                        ParaseFrameCtrl.Step  = 0;
                    }
                    else if(Frame01.len == 0 )
                    {
                        ParaseFrameCtrl.Step  = 11;
                        //uart_printf("空扩展文本帧\r\n");
                    }
                    else
                    {
                        ParaseFrameCtrl.Step  = 10;
                        ParaseFrameCtrl.Cnt = 0;
                    }
                    break;
            case 10: //接收到扩展帧正文
                    ParaseFrameCtrl.Bcc += data;
                    Frame01.buf[ParaseFrameCtrl.Cnt++] = data;
                    if(ParaseFrameCtrl.Cnt == Frame01.len)
                    {
                        ParaseFrameCtrl.Step  = 11;
                    }
                    break;
                    
            case 11:
                    if(data == 0x03)
                    {
                        ParaseFrameCtrl.Step  = 12;
                        ParaseFrameCtrl.Cnt += data;//?????????
                        ParaseFrameCtrl.Bcc += data;
                    }
                    else
                    {
                        //uart_printf("结束符错误\r\n");
						ParaseFrameCtrl.Step  = 0;
                        //Frame01.Valid = 3;//没有结束符
                    }
                    break;
                    
            case 12:
                    ParaseFrameCtrl.Step  = 0;
                    if(ParaseFrameCtrl.Bcc == data)
                    {
                            Frame01.Valid = 1;
                            return index;
                    }
                    else
                    {
                        //uart_printf("校验和错误\r\n");
						ParaseFrameCtrl.Step  = 0;
                        //Frame01.Valid = 2;   //接收校验错误
                    }
                    break;
                    
            default:
                index--;
                break;
                    
            }
    }

    return index;
}

void xgd_sn_thread(void)
{
    s32 ret;
    u8  sn_data[100];
   
    dev_usb_open(CDC_DEVICE);
    frame_01_open();
    rt_kprintf("enter xgd_sn_down\r\n");
    while(1)
    {
        ret = dev_usb_read(CDC_DEVICE,sn_data,FRAME_O1_DATA_MAX);
        if(ret>0)
        {   
            rt_kprintf("get usb cdc data ret =%d\r\n",ret);
            //for(i = 0;i<ret;i++)
                //rt_kprintf("%02x ",sn_data[i]);
            ret = frame_01_parse(sn_data,ret);
            frame_01_deal();
        }
        rt_thread_delay(1);
    }
}
void xgd_sn_init(void)
{
    if(xgd_sn_exist() != 0)
    {
        info_to_mes_pool = rt_malloc(MQ_POOL_SIZE);
        if(info_to_mes_pool == NULL)
        {
            rt_kprintf("%s,malloc fail\r\n",__FUNCTION__);
            return ;
        }
        rt_mq_init(&info_to_mes_mq, "info_to_mes_mq", info_to_mes_pool, MQ_POOL_SIZE/2,MQ_POOL_SIZE,\
                RT_IPC_FLAG_FIFO);
        rt_kprintf("xgd_sn_init\n");
        sn_tid   = rt_thread_create("sn_down", xgd_sn_thread,
                                          NULL, 1024*4, 135, 10);
        if(sn_tid == NULL)
        {
            rt_kprintf("xgd_sn_init ERR\n");
            return ;
        }
        rt_thread_startup(sn_tid);
        return ;
    }
    xgd_info_read(SN,sn,32);
    return;
}
/*========================下载机身号 END============================*/


#ifdef RT_USING_FINSH
#include <finsh.h>

FINSH_FUNCTION_EXPORT(xgd_info_erase, xgd_info_erase(type));
#endif

