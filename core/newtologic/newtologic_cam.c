#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtthread.h>
#include <dfs.h>
#include <dfs_file.h>
#include <rthw.h>
#include "time.h"
#include "decoderAPI.h"

typedef unsigned int u32;

static int tick_per_one_second=0;

//----------------export for libnew2swift.lib
void *newtologic_malloc(u32 size)
{
    //dbg_print("n malloc %d bytes\n",size);
	return malloc(size);
}

void newtologic_free(void *ptr)
{
	free(ptr);
}


void newtologic_get_tick(u32 *time_ms)
{
    unsigned int cur_tick;

    cur_tick = rt_tick_get();
    
	if(tick_per_one_second == 0)
	{
        tick_per_one_second = rt_tick_from_millisecond(1000);
	}

    if(tick_per_one_second == 100)
    {
        *time_ms = cur_tick*10;
    }
    else if(tick_per_one_second == 1000)
    {
        *time_ms = cur_tick*1;
    }
    else
    {
        *time_ms = 0;
    }
    //dbg_print("time %dms\n",*time_ms);
}
//----------------export for libnew2swift.lib


void dbg_print(char *fmt,...)
{
    va_list args;
    rt_size_t length;
    char log_buf[512];

    va_start(args, fmt);
    /* the return value of vsnprintf is the number of bytes that would be
     * written to buffer had if the size of the buffer been sufficiently
     * large excluding the terminating null byte. If the output string
     * would be larger than the rt_log_buf, we have to adjust the output
     * length. */
    length = rt_vsnprintf(log_buf, sizeof(log_buf) - 1, fmt, args);
    if (length > 512 - 1)
        length = 512 - 1;
    log_buf[length] = 0;
    printf("%s",log_buf);
    va_end(args);
}


int ncam_decode_data(char *buff,int width,int height)
{
    int retval, maxlen;
    char str[128];
    unsigned int dec_start_time;
    unsigned int dec_end_time;

    dec_start_time = rt_tick_get();
    
    initDecoder(width, height);
    enableSymbology(CODE128, 1);
    enableSymbology(QR, 1);
    setDecodeScore(20000);
    setDecoderSearchTimeMax(300);
    setDecoderAttemptTimeMax(300);

    retval = DecodeImage((unsigned char *)buff, width, height);
    dec_end_time = rt_tick_get();
    
    dbg_print("ncam decode:%s,last %d ticks\n",retval>0?"success":"error",(dec_end_time-dec_start_time));
    
    if (retval > 0) {
        maxlen = getResultLength();
    }
    if (maxlen > 0) {
        if (maxlen > sizeof(str)-1) {
            maxlen = sizeof(str) - 1;
        }
        memcpy(str, getResultString(), maxlen);
        str[maxlen] = 0;
        dbg_print("%s\n",str);
    }

    unInitDecoder();

    return 0;
}


#if 0
unsigned char *image_queue_malloc();
void image_queue_free(void *p);


int ncam_decode_file(int width,int height,char *path,int flag)
{
    int fd,size,ret;
    char *buf;

    size = width*height;
    buf = (char *)image_queue_malloc();

    if(NULL == buf)
    {
        dbg_print("malloc buf error\n");
        return -1;
    }
    
    fd = dev_vfs_open(path,"r");
    if(fd == 0)
    {
        dbg_print("open file:%s error\n",path);
        ret = -1;
        goto error_out;
    }
    dev_vfs_read(buf,size,fd);
    dev_vfs_close(fd);

    ncam_decode_data(buf,width,height);

error_out:
    image_queue_free(buf);

    return 0;
}

#endif


