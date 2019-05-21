#ifndef _ZBAR_QUEUE_H
#define _ZBAR_QUEUE_H

/*
#define WIDTH_IMAGE 640
#define HEIGHT_IMAGE 480
//#define TOTAL_SIZE WIDTH_IMAGE*HEIGHT_IMAGE
#define BUFFER_NUMBER 1
*/

void image_queue_init();

unsigned char *image_queue_malloc();

void image_queue_free(void *p);

int image_queue_size(void *p);



#endif //end _ZBAR_QUEUE_H






