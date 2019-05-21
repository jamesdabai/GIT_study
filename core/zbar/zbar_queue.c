#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtthread.h>

#include "zbar_queue.h"


#define IMG_WIDTH   640
#define IMG_HEIGHT  480
#define IMG_BUF_NUM 1
#define IMG_SIZE    (IMG_WIDTH*IMG_HEIGHT)

typedef struct
{
    int flag;
    int width;
    int height;
    int totalsize;
    unsigned char *bufferptr;
} image_buffer_t;

typedef struct
{
    image_buffer_t imagebuffer[IMG_BUF_NUM];
} image_queue_t;


static unsigned char memory_array[IMG_BUF_NUM][IMG_SIZE];
static image_queue_t image_queue;

void image_queue_init()
{
    int i;

    for(i = 0; i < IMG_BUF_NUM; i++)
    {
        ((image_queue.imagebuffer)[i]).flag      = 0;
        ((image_queue.imagebuffer)[i]).width     = IMG_WIDTH;
        ((image_queue.imagebuffer)[i]).height    = IMG_HEIGHT;
        ((image_queue.imagebuffer)[i]).totalsize = IMG_SIZE;
        ((image_queue.imagebuffer)[i]).bufferptr = (unsigned char *)(memory_array[i]);
    }
}

unsigned char *image_queue_malloc()
{
    int i;

    for(i = 0; i < IMG_BUF_NUM; i++)
    {
        if(((image_queue.imagebuffer)[i]).flag==0)
        {
            ((image_queue.imagebuffer)[i]).flag = 1;
            return ((image_queue.imagebuffer)[i]).bufferptr;
        }
    }

    return NULL;
}

void image_queue_free(void *p)
{
    int i;

    for(i = 0; i < IMG_BUF_NUM; i++)
    {
        if(((image_queue.imagebuffer)[i]).bufferptr == (unsigned char *)p)
        {
            ((image_queue.imagebuffer)[i]).flag = 0;
            break;
        }
    }
}

unsigned char image_queue_ishavespace()
{
    int i;

    for(i = 0; i < IMG_BUF_NUM; i++)
    {
        if(((image_queue.imagebuffer)[i]).flag == 0)
        {
            return 1;
        }
    }

    return 0;
}

int image_queue_size(void *p)
{
    return  IMG_SIZE;
}


