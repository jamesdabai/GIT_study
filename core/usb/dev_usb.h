#ifndef __DEV_USB_H
#define __DEV_USB_H 
#include "public_usb.h"

s32 dev_usb_open(u32 nClass);
s32 dev_usb_close(u32 nClass);
s32 dev_usb_read(u32 nClass ,u8 *lpOut ,u32 nLe);
s32 dev_usb_write(u32 nClass ,u8 *lpIn ,u32 nLe);
s32 dev_usb_ioctl(u32 nCmd ,u32 lParam ,u32 wParam);

#endif