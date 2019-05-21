#ifndef FLASH_H
#define FLASH_H

#define FLASH_MODULE 1

typedef enum
{
    EN_FLASH_FILE_TYPE_CFG = 0,
    EN_FLASH_FILE_TYPE_BAKCFG,
    EN_FLASH_FILE_TYPE_LOG,
    EN_FLASH_FILE_TYPE_BAKLOG,
     
    EN_FLASH_FILE_TYPE_APP,
    EN_FLASH_FILE_TYPE_BAKAPP,
    EN_FLASH_FILE_TYPE_KERNEL,  
    EN_FLASH_FILE_TYPE_BAKKERNEL,
    EN_FLASH_FILE_TYPE_FIRMWARE,
    EN_FLASH_FILE_TYPE_BAKFIRMWARE,

    EN_FLASH_FILE_TYPE_LOGO,
    EN_FLASH_FILE_TYPE_ENV,
    EN_FLASH_FILE_TYPE_FIMAGE,
    
    EN_FLASH_FILE_TYPE_BOOT,
    EN_FLASH_FILE_TYPE_LOG_B,  // 2013.01.22 add
    EN_FLASH_FILE_TYPE_LOG_C,  // 2013.01.22 add
    EN_FLASH_FILE_TYPE_LOG_D,  // 2013.01.22 add

    EN_FLASH_FILE_TYPE_ROOTFS,
    EN_FLASH_FILE_TYPE_WIFIFIRMWARE,// 2015.11.12 add

    EN_FLASH_FILE_TYPE_ENV0,
    EN_FLASH_FILE_TYPE_ENV1,

    EN_FLASH_FILE_TYPE_SPECIALCFG,  // 2015.12.11 add
    EN_FLASH_FILE_TYPE_PERMANENT,// 2015.12.17 add

    EN_FLASH_FILE_TYPE_USERBURN_CFG,
    
    EN_FLASH_FILE_TYPE_UNKONW
}FlashBase_FileType_e;

#define RT_DEVICE_CTRL_BLK_ERASE        0x12            /**< erase block on block device */

typedef struct
{
    int offset;
    int size;
    const char *device_pathname;
}ST_Flash_Base_Flash_Layout;

typedef struct
{
    int fd;
    ST_Flash_Base_Flash_Layout *layout;
}ST_Flash_Base_Dev_Info;


#endif
