#include <rtthread.h>
#include <rtdevice.h>
#include "dsp/fh_audio_mpi.h" //����������ͷ�ļ�
#include "xgd_config.h"
#include "xgd_config_deal.h"
#include "xgd_sender.h"


const char core_ver[] = "VABCR10S190510002";

char uboot[20];
char recovery[20];
char sn[32];

const SYS_SET_T sys_set_List[] =
{
    {"SET_FACTORY_SETTINGS",   SET_FACTOORY_SETTINGS,   SET_FACTOORY_SETTINGS ,     "��������"},
    
    {"SET_ADD_ENTER",           SET_ENTER_NEWLINE,      SET_ADD_ENTER,      "���س�"},
    {"SET_CAN_ENTER",           SET_ENTER_NEWLINE,      SET_CAN_ENTER,      "�����س�"},
    {"SET_ADD_NEWLINE",         SET_ENTER_NEWLINE,      SET_ADD_NEWLINE,      "������"},
    {"SET_CAN_NEWLINE",         SET_ENTER_NEWLINE,      SET_CAN_NEWLINE,      " ��������"},
    {"SET_ADD_EN/NEW",          SET_ENTER_NEWLINE,      SET_ADD_EN_NEW,     "���س�/����"},
    {"SET_CAN_EN/NEW",          SET_ENTER_NEWLINE,      SET_CAN_EN_NEW,      " �����س�/����"},
    
    {"SET_USB_HID_MODE",        SET_COM_MODE,           SET_USB_HID_MODE,    "USB(HID)ģʽ  "},
    {"SET_USB_CDC_COM",         SET_COM_MODE,           SET_USB_CDC_COM,     "USBCOM(CDC)"},
    {"SET_USRT_RS232",          SET_COM_MODE,           SET_UART_RS232,      "  RS232"},
    
    {"SET_START_UPGRADE",       SET_START_UPGRADE,      SET_START_UPGRADE,     "��������"},
    
    {"SET_OPEN_CODE_128_QR",    SET_CODE_TYPE,          SET_OPEN_CODE_128_QR,     "�ָ�128+QR"},
    {"SET_OPEN_CODE_39",        SET_CODE_TYPE,          SET_OPEN_CODE_39,     "��39 "},
    {"SET_CLOSE_CODE_39",       SET_CODE_TYPE,          SET_CLOSE_CODE_39 ,    "  �ر�39"},
    {"SET_OPEN_CODE_128",       SET_CODE_TYPE,          SET_OPEN_CODE_128,    "��������"},
    {"SET_CLOSE_CODE_128",      SET_CODE_TYPE,          SET_CLOSE_CODE_128 ,    "��������"},
    
    {"SET_OPEN_SCAN",           SET_SCAN_CODE,          SET_OPEN_SCAN ,    "��������"},
    {"SET_OPEN_COMMAND",        SET_SCAN_CODE,          SET_OPEN_COMMAND,      "��������"},
    
    {"SET_SCAN_GAP_5s",         SET_SCAN_CODE,          SET_SCAN_GAP_5s ,    "��������"},
    {"SET_SCAN_GAP_10s",        SET_SCAN_CODE,          SET_SCAN_GAP_10s,     "��������"},
    {"SET_SCAN_GAP_45s",        SET_SCAN_CODE,          SET_SCAN_GAP_45s,    "��������"},
    {"SET_SCAN_GAP_60s",        SET_SCAN_CODE,          SET_SCAN_GAP_60s ,    "��������"},
    {"SET_STD_LIGHT_MODE",      SET_SCAN_CODE,          SET_STD_LIGHT_MODE,     "��������"},
    {"SET_WHIHE_MODE",          SET_SCAN_CODE,          SET_WHIHE_MODE  ,   "��������"},
    {"SET_GREEN_MODE",          SET_SCAN_CODE,          SET_GREEN_MODE ,    "��������"},
    {"SET_CLOSE_LIGHT",         SET_SCAN_CODE,          SET_CLOSE_LIGHT,    "��������"},
    
    {"SET_SCAN_TIMEOUT_30s",    SET_READ_CODE,          SET_SCAN_TIMEOUT_30s,     "��������"},
    {"SET_SCAN_TIMEOUT_60s",    SET_READ_CODE,          SET_SCAN_TIMEOUT_60s,     "��������"},
    {"SET_SCAN_TIMEOUT_120s",   SET_READ_CODE,          SET_SCAN_TIMEOUT_120s,    "��������"},
    {"SET_SCAN_TIMEOUT_180s",   SET_READ_CODE,          SET_SCAN_TIMEOUT_180s,     "��������"},
    {"SET_SCAN_TIMEOUT_240s",   SET_READ_CODE,          SET_SCAN_TIMEOUT_240s ,     "��������"},
    {"SET_SCAN_TIMEOUT_300s",   SET_READ_CODE,          SET_SCAN_TIMEOUT_300s ,   "��������"},
    
    {"SET_COLUME_0",            SET_COLUME,             SET_COLUME_0 ,   "��������0��"},
    {"SET_COLUME_1",            SET_COLUME,             SET_COLUME_1 ,   "��������1��"},
    {"SET_COLUME_2",            SET_COLUME,             SET_COLUME_2 ,   "��������2��"},
    {"SET_COLUME_3",            SET_COLUME,             SET_COLUME_3 ,  "��������3��"},
    {"SET_COLUME_4",            SET_COLUME,             SET_COLUME_4 ,  "��������4��"},
    {"SET_COLUME_5",            SET_COLUME,             SET_COLUME_5 ,   "��������5��"},
    
    {"SET_BAUD_2400",           SET_BAUD,               SET_BAUD_2400 ,   "���ô��ڲ�����"},
    {"SET_BAUD_4800",           SET_BAUD,               SET_BAUD_4800,    "���ô��ڲ�����"},
    {"SET_BAUD_9600",           SET_BAUD,               SET_BAUD_9600 ,   "���ô��ڲ�����"},
    {"SET_BAUD_14400",          SET_BAUD,               SET_BAUD_14400,    "���ô��ڲ�����"},
    {"SET_BAUD_19200",          SET_BAUD,               SET_BAUD_19200 ,  "���ô��ڲ�����"},
    {"SET_BAUD_38400",          SET_BAUD,               SET_BAUD_38400,   "���ô��ڲ�����"},
    {"SET_BAUD_57600",          SET_BAUD,               SET_BAUD_57600 ,   "���ô��ڲ�����"},
    {"SET_BAUD_115200",         SET_BAUD,               SET_BAUD_115200 ,   "���ô��ڲ�����"},
    
    {"SET_DELAY_500MS",         SET_SCAN_DELAY,   SET_DELAY_500MS ,"��������"},
    {"SET_DELAY_1S",            SET_SCAN_DELAY,   SET_DELAY_1S ,   "��������"},
    {"SET_DELAY_2S",            SET_SCAN_DELAY,   SET_DELAY_2S ,   "��������"},
    {"SET_DELAY_3S",            SET_SCAN_DELAY,   SET_DELAY_3S ,   "��������"},
    {"SET_DELAY_5S",            SET_SCAN_DELAY,   SET_DELAY_5S,    "��������"},
    {"SET_DELAY_10S",           SET_SCAN_DELAY,   SET_DELAY_10S ,  "��������"},
    
    {"SET_VERSION_CHECK",       SET_VERSION_CHECK,   SET_VERSION_CHECK,    "���汾"},
    
    {"SET_HARD_DEEP",           SET_HARD_CONFIG  ,   SET_HARD_DEEP,     "Ӳ������"},
    {"SET_HARD_SHALLOW",        SET_HARD_CONFIG  ,   SET_HARD_SHALLOW,  "Ӳ������"},
    {"SET_VOICE_BEEP",          SET_VOICE  ,         SET_VOICE_BEEP,     "ɨ�����"},
    {"SET_VOICE_OK",            SET_VOICE  ,         SET_VOICE_OK,       "ɨ��ɹ�"},
    
    {"SET_END",                 SET_END,    SET_END,   "����"},
};

void xgd_download(void)
{

    xgd_update_write(0xef);
    rt_thread_delay(200);
    rt_kprintf("��������\n");
    system("system_r");//�ػ�
    return ;
}
void xgd_uart_config(void)
{
    int ret;
    int baud;
    char value[10];
    rt_device_t uart_device;
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;

    uart_device = (rt_device_t)rt_device_find("uart0");
    if(uart_device == NULL)
        return ;
    else
        rt_kprintf("uart device find!\n");
        
    ret = xgd_config_read(SET_BAUD);
    switch (ret)
    {
        case SET_BAUD_2400:
            baud = 2400;
            break;
        case SET_BAUD_4800:
            baud = 4800;
            break;
        case SET_BAUD_9600:
            baud = 9600;
            break;
        case SET_BAUD_14400:
            baud = 14400;
            break;
        case SET_BAUD_19200:
            baud = 19200;
            break;
        case SET_BAUD_38400:
            baud = 38400;
            break;
        case SET_BAUD_57600:
            baud = 57600;
            break;
        case SET_BAUD_115200:
            baud = 115200;
            break;
        default :
            baud = 115200;
            break;
    }
    
    config.baud_rate = baud;
    uart_device->control(uart_device,RT_DEVICE_CTRL_CONFIG,(void *)&config);
    rt_kprintf("xgd_uart_cfg baud=%d,�������\r\n",baud);
    return ;
}
void xgd_com_config(void)
{
    //�ڵײ�������
}


void xgd_get_version(void)
{
    Flash_Base_Init();
    spi_flash_read(RECV_VER_ADDR ,recovery,17);
    spi_flash_read(ADDR,uboot,17);
}
void xgd_show_version(void)
{
    
    char version[40] ;
    if(strlen(uboot)!=17)
    {
        rt_kprintf("uboot VERSION err len=%d\r\n",strlen(uboot));
        memset(uboot,0,20);
        memcpy(uboot,"ver err",sizeof("ver err"));
    }
    if(strlen(recovery)!=17)
    {
        rt_kprintf("recovery VERSION err=%d\r\n",strlen(recovery));
        memset(recovery,0,20);
        memcpy(recovery,"ver err",sizeof("ver err"));
    }

    sprintf(version,"uboot:%s\r\n",uboot);
    xgd_send_message(version);
    
    sprintf(version,"recovery:%s\r\n",recovery);
    xgd_send_message(version);

    sprintf(version,"core:%s\r\n",core_ver);
    xgd_send_message(version);

    sprintf(version,"sn:%s\r\n",sn);
    xgd_send_message(version);
    
}

int xgd_volume_config(void)//0~100
{
    int ret;
    char  volume;
    
    ret = xgd_config_read(SET_COLUME);
    switch (ret)
    {
        case SET_COLUME_0:
            volume = 0;
            break;
        case SET_COLUME_1:
            volume = 20;
            break;
        case SET_COLUME_2:
            volume = 25;
            break;
        case SET_COLUME_3:
            volume = 28;
            break;
        case SET_COLUME_4:
            volume = 35;
            break;
        case SET_COLUME_5:
            volume = 50;
            break;
        default :
            volume = 28;
            break;
    }
    ret = FH_AC_AO_SetVol(volume);
    return volume;
}
int xgd_config_parser(u8* config_cmd)
{
    u8 loop;
    s32 ret;
    ret = memcmp(config_cmd,"SET",3);
    if(ret != 0)
        return -1;

    if(strcmp("SET_FACTORY_SETTINGS",config_cmd) == 0)
    {
        unlink(XGD_CFG_FILE);//ɾ���ļ�
        return 0;
    }
    else if(strcmp("SET_VERSION_CHECK",config_cmd) == 0)
    {
        xgd_show_version();
        return 0;
    }
    else if(strcmp("SET_START_UPGRADE",config_cmd) == 0)
    {
        xgd_download();
        return 0;
    }
    for(loop = 0;;loop++)
    {
        if(strcmp(sys_set_List[loop].cmd_str,config_cmd) == 0)
        {   
            if(sys_set_List[loop].id == SET_CODE_TYPE)
            {
                u8 value;
                value = xgd_config_read(SET_CODE_TYPE);
                if(sys_set_List[loop].param == SET_OPEN_CODE_128_QR || \
                    sys_set_List[loop].param == SET_OPEN_CODE_39 || \
                    sys_set_List[loop].param == SET_OPEN_CODE_128)
                {
                    value |= sys_set_List[loop].param;//λ����
                }
                else
                {
                    value &= sys_set_List[loop].param;//λ����
                }
                xgd_config_write(sys_set_List[loop].id, value);
            }
            else
            {
                xgd_config_write(sys_set_List[loop].id,sys_set_List[loop].param);
            }
            
            if(sys_set_List[loop].id == SET_BAUD)//���������Ч������
            {
                xgd_uart_config();
            }
            if(sys_set_List[loop].id == SET_COLUME)
            {
                xgd_volume_config();
            }
            if(sys_set_List[loop].id == SET_SCAN_DELAY)
            {
                scan_delay_set();
            }
            break;
        }
        else if(strcmp(sys_set_List[loop].cmd_str,"SET_END") == 0)
        {
            rt_kprintf("û�ҵ���������\r\n");
            return -1;
        }
    }
    return 0;
}

void xgd_Configuration_init()
{
    xgd_config_init();
    xgd_get_version();
    xgd_uart_config();
    xgd_com_config();
}


