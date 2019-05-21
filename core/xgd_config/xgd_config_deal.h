#ifndef __SYS_SET_H__
#define __SYS_SET_H__
#include "common.h"


typedef struct
{
    u8* cmd_str;
    u8  id;
    u8  param;
    u8* func_description;
}SYS_SET_T;

#define CHCHE_ADDR          0x3E000//cache区
#define ADDR  (CHCHE_ADDR + 4096)
#define RECV_VER_ADDR    (0x280000 + 0x800)

enum CFG_CMD
{
    SET_FACTOORY_SETTINGS = 0,  
    SET_ENTER_NEWLINE,          
    SET_COM_MODE,     
    SET_START_UPGRADE,    
    SET_CODE_TYPE,
    SET_SCAN_CODE,   
    SET_READ_CODE,
    SET_COLUME,         
    SET_BAUD,        
    SET_SCAN_DELAY,      
    SET_VERSION_CHECK,    
    SET_HARD_CONFIG,
    SET_VOICE,
    SET_END_CFG
};
enum 
{
    SET_FACTOORY_SETTING = 0
};

enum 
{
    SET_ADD_ENTER = 0,      
    SET_CAN_ENTER,      
    SET_ADD_NEWLINE,    
    SET_CAN_NEWLINE,    
    SET_ADD_EN_NEW,     
    SET_CAN_EN_NEW
};

enum 
{
    SET_USB_HID_MODE = 0,   
    SET_USB_CDC_COM,    
    SET_UART_RS232
};
enum 
{
    SET_OPEN_CODE_128_QR = 1,
    SET_OPEN_CODE_39     = 0x2,   
    SET_CLOSE_CODE_39    = 0xfd,//~(0x2),  
    SET_OPEN_CODE_128    = 0x4,  
    SET_CLOSE_CODE_128   = 0xfb,//~(0x4)
};

enum 
{
    SET_OPEN_SCAN = 0,//cdc和串口常开
    SET_OPEN_COMMAND, //cdc和串口通过命令打开或关闭
    SET_SCAN_GAP_5s,    
    SET_SCAN_GAP_10s,   
    SET_SCAN_GAP_45s,   
    SET_SCAN_GAP_60s,   
    SET_STD_LIGHT_MODE, 
    SET_WHIHE_MODE,     
    SET_GREEN_MODE,     
    SET_CLOSE_LIGHT
};

enum 
{
    SET_SCAN_TIMEOUT_30s = 0,
    SET_SCAN_TIMEOUT_60s,
    SET_SCAN_TIMEOUT_120s,
    SET_SCAN_TIMEOUT_180s,
    SET_SCAN_TIMEOUT_240s,
    SET_SCAN_TIMEOUT_300s
};

enum 
{
    SET_COLUME_0 = 0,       
    SET_COLUME_1,       
    SET_COLUME_2,      
    SET_COLUME_3,       
    SET_COLUME_4,      
    SET_COLUME_5
};

enum 
{
    SET_BAUD_2400 = 0,      
    SET_BAUD_4800,      
    SET_BAUD_9600,     
    SET_BAUD_14400,     
    SET_BAUD_19200,    
    SET_BAUD_38400,     
    SET_BAUD_57600,     
    SET_BAUD_115200
};
enum 
{
    SET_DELAY_500MS = 0,   
    SET_DELAY_1S = 1,       
    SET_DELAY_2S = 2,       
    SET_DELAY_3S = 3,       
    SET_DELAY_5S = 5,       
    SET_DELAY_10S = 10 
};
enum 
{
    SET_HARD_DEEP = 0,
    SET_HARD_SHALLOW = 1,
};
enum 
{
    SET_VOICE_BEEP = 0,
    SET_VOICE_OK   = 1,
};

enum 
{
   SET_END = 66
};



#endif
