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

#include "zbar.h"
#include "time.h"
#include "xgdzbar_statics.h"
#include "decoderAPI.h"
#include "xgd_config_deal.h"

//
void image_queue_init();
void audio_player_go(void);
void xgd_send_str(char *pstr);


/* 邮箱控制块 */
static struct rt_mailbox mb_img;
/* 用于放邮件的内存池 */
static char mb_pool_img[32];
static char save_file_path[16]={0};
xgdzbar_statics_t xgdzbar_stat={0};
//扫码延时、码制设置、硬件配置变量
static unsigned char code_config = 7;//默认三种码制全开
static char scan_delay_config = SET_DELAY_3S;
static unsigned char hard_config = 0;

static unsigned char scan_status = 0;//解码打开或关闭标志，用于cdc和uart模式的

void scan_delay_set(void)
{
    scan_delay_config = xgd_config_read(SET_SCAN_DELAY);
}

void xgd_scanner_start(void)
{
    rt_enter_critical();
    scan_status = 1;
    rt_exit_critical();
    backlight_led_ctrl(1);//打开扫描，打开背光
}

void xgd_scanner_stop(void)
{
    rt_enter_critical();
    scan_status = 0;
    rt_exit_critical();
    backlight_led_ctrl(0);//关闭扫描，关闭背光
}
int xgd_scanner_status(void)
{
    return scan_status;
}

void check_save_y8_file(unsigned char *data,int size)
{
    int loop,cnt;
    char *path;
    char buff[128],ch;
    struct dfs_fd fd;
    
    if(save_file_path[0] == 0)
    {
        return;
    }

    path = save_file_path;

    if(dfs_file_open(&fd, path, DFS_O_WRONLY | DFS_O_CREAT) < 0)
    {
        printf("Create File:%s error\n",path);
        return;
    }
    memset(save_file_path,0,sizeof(save_file_path));

    dfs_file_write(&fd,data,size);
    dfs_file_close(&fd);  
}


/* trig save y8 file */
int trig(int argc,char argv[])
{
    int len;
    char *path;

    if(argc > 1)
    {
        path = argv[1];
    }
    else
    {
        path = "cap.y8";
    }
    
    len = strlen(path);
    if(len < sizeof(save_file_path))
    {
        printf("trig %s\n",path);
        memcpy(save_file_path,path,len);
    }
    else
    {
        printf("file name too long\n");
    }
}


static int _zbar_decode_data(int width,int height,char *pbuf)
{
    int ret,str_len,loop;
    zbar_image_t *image;
    zbar_image_scanner_t *scanner;
    const zbar_symbol_t *symbol;


    unsigned int dec_start_time;
    unsigned int dec_end_time;
    char log_buff[30];

    dec_start_time = rt_tick_get();
    scanner = zbar_image_scanner_create();
    zbar_image_scanner_set_config(scanner, 0, ZBAR_CFG_ENABLE, 0);
    zbar_image_scanner_set_config(scanner, ZBAR_QRCODE, ZBAR_CFG_ENABLE, 1);
    zbar_image_scanner_set_config(scanner, ZBAR_CODE128, ZBAR_CFG_ENABLE, 1);
    
    image = zbar_image_create();
    zbar_image_set_format(image, *(int*)"Y800");
    zbar_image_set_size(image, width, height);
    zbar_image_set_data(image, pbuf, width * height, 0);

    ret = zbar_scan_image(scanner, image);
    
    
    if (ret > 0) {
        
        //rt_thread_delay(0);
        
        symbol = zbar_image_first_symbol(image);
        dec_end_time = rt_tick_get();
#if 1
        if(symbol)
        {
            memset(log_buff,0,30);
            //sprintf(log_buff,"time = %d\r\n",(dec_end_time-dec_start_time)*10);
            xgd_send_str(log_buff);
            zbar_symbol_type_t typ = zbar_symbol_get_type(symbol);
            unsigned int       len = zbar_symbol_get_data_length(symbol);
            const char       *data = zbar_symbol_get_data(symbol);

            str_len = strlen(data);
            
            if( (typ == ZBAR_CODE128) && (str_len == 0) )
            {
                rt_kprintf("decoded %s symbol,len %d\n", zbar_get_symbol_name(typ),len);
                rt_kprintf("\n\n\nNOTICE DUMMY CODE128 DETECT\n\n\n\n");
                rt_kprintf("len=%d\n",len);
                for(loop=0;loop<len;loop++)
                {
                    rt_kprintf("%02x ",data[loop]);
                    if((loop&0xF) && (loop > 0))
                    {
                        rt_kprintf("\n");
                    }
                }
                rt_kprintf("\n\n\nNOTICE END\n\n\n");
            }
            // qr binary
            else if( (typ == ZBAR_QRCODE) && (str_len != len))
            {
                audio_player_go();
                scan_ok_led_ctrl(1);
                xgd_send_binary(data,len);
                //rt_kprintf("decoded %s symbol,len %d, binary data\n", zbar_get_symbol_name(typ),len);
            }
            else
            {
                audio_player_go();
                scan_ok_led_ctrl(1);
                xgd_send_str(data);
                //rt_kprintf("decoded %s symbol,len %d: \"%s\"\n", zbar_get_symbol_name(typ),len,data);
            }
            
        }
#else
        for(; symbol; symbol = zbar_symbol_next(symbol)) {
            zbar_symbol_type_t typ = zbar_symbol_get_type(symbol);
            unsigned int       len = zbar_symbol_get_data_length(symbol);
            const char *data = zbar_symbol_get_data(symbol);
            rt_kprintf("decoded %s symbol,len %d: \"%s\"\n", zbar_get_symbol_name(typ),len,data);
            xgd_send_str(data);
            break;
        }
#endif
    }
    else
    {
    }
    
    zbar_image_destroy(image);
    zbar_image_scanner_destroy(scanner);

    return ret;
}

void array_Flip(char *p ,int width,int height)
{
    char temp[640];
    int i,j;
    char *start = p;
    char *tp = p;
    for(j=0;j<height;j++)
    {
        for(i=0;i<width;i++)//倒叙储存
        {
            temp[i] = *(p+width-i-1);
        }
        memcpy(p,temp,width);
        p += width;
    }
    
    return ;
}
#if 1


static int new_decode_data(int width,int height,char *pbuf)
{
    int retval, maxlen;
    char str[256];
    char str_bak[256];
    unsigned int dec_start_time;
    unsigned int dec_end_time;
    
    static int scan_delay_time = 0;
    static int cancle_delay_time = 0;//清零延时时间
    static char detect_flag = 0;//检测到相同码标志

    static char scan_flag = 0;
 
    initDecoder(width, height);
    enableSymbology(QR, 1);
    enableSymbology(EAN13, 1);
    if((code_config & SET_OPEN_CODE_39) != 0)
    {
        enableSymbology(CODE39, 1);
    }
    else
    {
        enableSymbology(CODE39, 0);
    }
    if((code_config & SET_OPEN_CODE_128) != 0)
    {
        enableSymbology(CODE128, 1);
    }
    else
    {
        enableSymbology(CODE128, 0);
    }
    
    setDecodeScore(20000);
    setDecoderSearchTimeMax(250);
    setDecoderAttemptTimeMax(250);
    if(scan_flag)
    {
        scan_flag = 0;
        if(hard_config)
            API_ISP_SET(0x65/*0x38*/);//浅锅
        else
            API_ISP_SET(0x28/*0x38*/);//深锅
    }
    else
    {
        scan_flag += 1;
        if(hard_config)
            API_ISP_SET(120/*0x38*/);//浅锅
        else
            API_ISP_SET(70/*0x38*/);//深锅
       
    }
    //dec_start_time = rt_tick_get();
    retval = DecodeImage((unsigned char *)pbuf, width, height);
    //dec_end_time = rt_tick_get();

    if (retval > 0) 
    {
        //dbg_print("ncam decode:%s,last %d ticks\n",retval>0?"success":"error",(dec_end_time-dec_start_time));
        maxlen = getResultLength();
        if (maxlen > 0) 
        {
            char same_str_flag = 0;
            if (maxlen > sizeof(str)-1) 
            {
                maxlen = sizeof(str) - 1;
            }
            memcpy(str, getResultString(), maxlen);
            str[maxlen] = 0;

            if(memcmp(str,str_bak,maxlen) != 0)//检测到扫描数据和前一次的不同则上报数据
            {
                 same_str_flag = 1;
            }
            if((rt_tick_get()-scan_delay_time >scan_delay_config*100) || same_str_flag)//相同码延时处理
            {
                if(mes_get_info_status())
                {
                    info_to_mes_voice();
                    info_to_mes_sender(str,strlen(str)+1);//加一是为了将结束符传过去
                }
                else
                {
                    xgd_send_str(str);
                }
                memset(str_bak,0,256);
                memcpy(str_bak,str,maxlen+1);//将最后一个字节都复制过去
                scan_delay_time = rt_tick_get();//开始计时
            }
        }
    }
    unInitDecoder();
    return retval;
}
#endif


int xgd_decode(int width,int height,char *pbuf)
{
    int ret;
    static int flag=0;
    static unsigned int prev_tick=0xFFFFFFFF; // 上一次扫码成功时间
    unsigned int cur_tick;
    int skip_scan = 0;

#if 0
    if(scan_wait_internal() < 0)
    {
        return -1;
    }

    ret = zbar_decode_data(width,height,pbuf);
    return ret;
#else

    cur_tick = rt_tick_get();

    // first time
    if(prev_tick == 0xFFFFFFFF)
    {
        skip_scan = 0;
        prev_tick = cur_tick;
    }
    else if(cur_tick >= prev_tick)
    {
        if( (cur_tick-prev_tick) > (RT_TICK_PER_SECOND*1/2) )  // 200ms
        {
            skip_scan = 0;
        }
        else
        {
            skip_scan = 1;
        }
    }
    else    // cur_tick < prev_tick
    {
        if( (0xFFFFFFFF-prev_tick + cur_tick) > (RT_TICK_PER_SECOND*1/2)) // 200ms
        {
            skip_scan = 0;
        }
        else
        {
            skip_scan = 1;
        }
    }

    if(skip_scan == 0)
    {
#if 1

        ret = new_decode_data(width,height,pbuf);
#else
        ret = _zbar_decode_data(width,height,pbuf);
#endif
        if(ret > 0)
        {
            prev_tick = cur_tick;
        }
    }

    return 0;
#endif
}

void image_frame_send(void *ptr, int len)
{
    unsigned char *pbuf;

    xgdzbar_stat.frame_send_cnt++;
    pbuf = image_queue_malloc();
    if(NULL == pbuf)
    {
        //printf("imge queue full\n");
        return;
    }

    memcpy(pbuf,ptr,len);
    //printf("send1: %d\n",rt_tick_get());
    rt_mb_send(&mb_img, (rt_uint32_t)pbuf);
    //printf("send2: %d\n",rt_tick_get());
}

void xgd_decoder_thread(void *arg)
{
    char *ptr;
    int width = CH1_WIDTH;
    int height = CH1_HEIGHT;

    unsigned int prev_tick,cur_tick,last_time;
    unsigned long start_time, end_time, total_time, average_time;
    struct timeval tv;
    int frame;

    xgdzbar_stat.frame_dec_max_time = 0;
    xgdzbar_stat.frame_dec_min_time = 10000;
    
    /* 初始化一个mailbox */
    rt_mb_init(&mb_img,
               "mb_img",
               &mb_pool_img[0],
               sizeof(mb_pool_img)/4,
               RT_IPC_FLAG_FIFO);
    
    image_queue_init();

    code_config = xgd_config_read(SET_CODE_TYPE);
    if(code_config > 7)
    {
        code_config = 7;
    }
    scan_delay_config = xgd_config_read(SET_SCAN_DELAY);
    if(scan_delay_config > SET_DELAY_10S)
    {
        scan_delay_config = SET_DELAY_3S;
    }
    hard_config =  xgd_config_read(SET_HARD_CONFIG);
    if(hard_config != 1)
    {
        hard_config = 0;
    }
    //rt_kprintf("SET_SCAN_DELAY=%d  =%d code =%d,hard_config =%d\r\n",SET_SCAN_DELAY,scan_delay_config,code_config,hard_config);
    while(1)
    {
        if (rt_mb_recv(&mb_img, (rt_uint32_t*)&ptr, RT_WAITING_FOREVER) == RT_EOK)
        {
            xgdzbar_stat.frame_scan_cnt++;
            prev_tick = rt_tick_get();
            //gettimeofday(&tv, NULL);
            //start_time = tv.tv_sec*1000 + tv.tv_usec/1000;
            xgd_decode(width,height,ptr);

            //gettimeofday(&tv, NULL);
            //end_time = tv.tv_sec*1000 + tv.tv_usec/1000;
            cur_tick = rt_tick_get();

            // TODO if cur_tick less than last_tick
            if(cur_tick >= prev_tick)
            {
                last_time = cur_tick - prev_tick;
            }
            else
            {
                last_time = 0xFFFFFFFF-prev_tick+cur_tick;
            }
            last_time = 1000/RT_TICK_PER_SECOND*last_time;
            //printf("decode %dx%d : %dms\n",width,height,last_time);

            if(last_time > xgdzbar_stat.frame_dec_max_time)
            {
                xgdzbar_stat.frame_dec_max_time = last_time;
            }
            if(last_time < xgdzbar_stat.frame_dec_min_time)
            {
                xgdzbar_stat.frame_dec_min_time = last_time;
            }
            xgdzbar_stat.frame_dec_tot_time += last_time;
        

            if(xgdzbar_stat.prev_tick == 0)
            {
                xgdzbar_stat.prev_tick = cur_tick;
                xgdzbar_stat.frame_scan_cnt_p = xgdzbar_stat.frame_scan_cnt;
                xgdzbar_stat.frame_send_cnt_p = xgdzbar_stat.frame_send_cnt;
            }
            else
            {
                prev_tick = xgdzbar_stat.prev_tick;
                
                if(cur_tick >= prev_tick)
                {
                    last_time = cur_tick-prev_tick;
                }
                else
                {
                    last_time = 0xFFFFFFFF-prev_tick+cur_tick;
                }

                // 2秒钟统计一次fps
                if(last_time > (2*RT_TICK_PER_SECOND))
                {
                    xgdzbar_stat.prev_tick = cur_tick;
                    
                    last_time = 1000/RT_TICK_PER_SECOND*last_time;
                    frame = xgdzbar_stat.frame_send_cnt-xgdzbar_stat.frame_send_cnt_p;
                    xgdzbar_stat.frame_send_fps = frame*10000/last_time;
                    //rt_kprintf("1 last time:%dms, frame:%d\n",last_time,frame);

                    frame = xgdzbar_stat.frame_scan_cnt-xgdzbar_stat.frame_scan_cnt_p;
                    xgdzbar_stat.frame_scan_fps = frame*10000/last_time;
                    //rt_kprintf("2 last time:%dms, frame:%d\n",last_time,frame);

                    xgdzbar_stat.frame_scan_cnt_p = xgdzbar_stat.frame_scan_cnt;
                    xgdzbar_stat.frame_send_cnt_p = xgdzbar_stat.frame_send_cnt;

                    xgdzbar_stat.frame_dec_ave_time = xgdzbar_stat.frame_dec_tot_time/xgdzbar_stat.frame_scan_cnt;
                }
            }
            
            //total_time = end_time - start_time;
            //printf("-------time=%ld---------\n", total_time);
            check_save_y8_file(ptr,width*height);
            image_queue_free(ptr);
            //printf("got frame finish\n");

        }
    }
}


int xgdzbar(int argc,char *argv[])
{
    unsigned long start_time;
    struct timeval tv;

    //gettimeofday(&tv, NULL);
    //start_time = tv.tv_sec*1000 + tv.tv_usec/1000;
    
    printf("xgdzbar statics: %d\n",rt_tick_get());
    printf("\tframe_send_cnt:%d fps:%d.%d\n",xgdzbar_stat.frame_send_cnt,\
        xgdzbar_stat.frame_send_fps/10,xgdzbar_stat.frame_send_fps%10);
    printf("\tframe processed:%d fps:%d.%d\n",xgdzbar_stat.frame_scan_cnt,\
        xgdzbar_stat.frame_scan_fps/10,xgdzbar_stat.frame_scan_fps%10);
    printf("\tframe decode time min:%d,max:%d,ava:%d\n",xgdzbar_stat.frame_dec_min_time,\
        xgdzbar_stat.frame_dec_max_time,xgdzbar_stat.frame_dec_ave_time);
    rt_kprintf("\taudio req:%d play:%d\n",xgdzbar_stat.audio_req_cnt,xgdzbar_stat.audio_play_cnt);
    rt_kprintf("\tstring send:%d\n",xgdzbar_stat.string_send_cnt);
}

#ifdef RT_USING_FINSH
#include <finsh.h>
MSH_CMD_EXPORT(xgdzbar, xgdzbar info);
MSH_CMD_EXPORT(trig, trig save y8 file);
#endif

