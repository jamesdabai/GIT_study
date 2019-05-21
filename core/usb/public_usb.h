#ifndef __PUBLIC_USB_H
#define __PUBLIC_USB_H 

/*USB设备类型*/
#define CDC_HOST 0
#define CDC_DEVICE 1
#define HID_HOST   2
#define HID_DEVICE 3
#define HID_DEVICE_OLD 0xEF /*兼容老的SDK*/

/*USB IOCTL 命令*/
#define DDI_USB_CTL_VER  0
#define DDI_USB_CTL_CLEAR  1
#define DDI_USB_CTL_STATUS  2
#endif
