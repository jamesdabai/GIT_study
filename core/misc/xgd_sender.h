#ifndef __XGD_SENDER_H__
#define __XGD_SENDER_H__

#define UART0_NAME "uart0"
#define CDC_NAME   "vcom"
#define FHSZ_ABS(d_a,d_b) ((d_a > d_b) ? (d_a - d_b) : (d_b - d_a))

#ifndef RT_HID_USING_TX_WAIT
#define FH_HID_DELAY() \
do{\
    int _delay = 0;\
    for(_delay=0; _delay<11; _delay++)\
        rt_udelay(100);\
    }while(0)
#else
#define FH_HID_DELAY()
#endif

typedef enum
{
    no_Control      = 0x00,
    left_Control    = 0x01,
    left_Shift      = 0x02,
    left_Alt        = 0x04,
    left_Gui        = 0x08,
    right_Control   = 0x10,
    right_Shift     = 0x20,
    right_Alt       = 0x40,
    right_Gui       = 0x80,
}FunctionKey_t;

/* usb device type for enum */
typedef enum _USB_CLASS
{
    NONE = 0x00,
    HID  = 0x01,
    CDC  = 0x02,
    CCID = 0x03,
    MSD  = 0x04,
}USB_CLASS;

typedef struct
{
    char a;
    char b;
}UnitKey_t;


static rt_err_t hid_tx_complete(rt_device_t dev, void *buffer);
#endif
