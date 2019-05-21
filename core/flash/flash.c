#include <rtdef.h>
#include <rtthread.h>
#include <rtdevice.h>

#include "common.h"
#include "flash.h"


#if 0
    #define fl_printf rt_kprintf
#else
    #define fl_printf 
#endif


static rt_device_t flash_device = 0;

static ST_Flash_Base_Flash_Layout   cs0_fimage        = {0x00000000, 0x00040000, "/dev/mtdblock0"};// 256KB 头信息区 + ramboot
static ST_Flash_Base_Flash_Layout   cs0_env           = {0x00040000, 0x00010000, "/dev/mtdblock1"};
static ST_Flash_Base_Flash_Layout   cs0_uboot         = {0x00050000, 0x00030000, "/dev/mtdblock2"};// 192KB
static ST_Flash_Base_Flash_Layout   cs0_kernel        = {0x00000000, 0x00400000, "/dev/mtdblock3"};
static ST_Flash_Base_Flash_Layout   cs0_rootfs        = {0x000a0000, 0x00000000, "/dev/mtdblock4"};// size为0 不占mtd资源 TBD_WAIT
static ST_Flash_Base_Flash_Layout   cs0_app           = {0x000a0000, 0x00250000, "/dev/mtdblock5"};

static ST_Flash_Base_Flash_Layout   cs0_log           = {0x00000000, 0x00000000, "/dev/mtdblock6"};
static ST_Flash_Base_Flash_Layout   cs0_logB          = {0x00000000, 0x00000000, "/dev/mtdblock7"};  
static ST_Flash_Base_Flash_Layout   cs0_logC          = {0x00000000, 0x00000000, "/dev/mtdblock8"};  
static ST_Flash_Base_Flash_Layout   cs0_logD          = {0x00000000, 0x00000000, "/dev/mtdblock9"}; 
   
static ST_Flash_Base_Flash_Layout   cs0_conf          = {0x002f0000, 0x00010000, "/dev/mtdblock10"};

static ST_Flash_Base_Flash_Layout   cs0_pp            = {0x00300000, 0x00010000, "/dev/mtdblock11"};
static ST_Flash_Base_Flash_Layout   cs0_specialcfg    = {0x00310000, 0x00010000, "/dev/mtdblock12"};

#if FLASH_CFG_USER_BURN
static ST_Flash_Base_Flash_Layout   cs0_userCfgConf   = {0x00320000, 0x00010000, "/dev/mtdblock13"};
#endif


static ST_Flash_Base_Flash_Layout   cs0_wififirmware  = {0x00330000, 0x00050000, "/dev/mtdblock14"};        
//static ST_Flash_Base_Flash_Layout   cs0_arcfireware  = {0x00380000, 0x00080000, "/dev/mtdblock15"};    

static ST_Flash_Base_Flash_Layout   cs0_env0          = {0x00000000, 0x00000000, "/dev/mtdblock16"};

static ST_Flash_Base_Flash_Layout   cs0_env1          = {0x00000000, 0x00000000, "/dev/mtdblock17"};
static ST_Flash_Base_Flash_Layout   cs0_appbak        = {0x00000000, 0x00000000, "/dev/mtdblock18"};

static ST_Flash_Base_Dev_Info  dev_fimage         = {-1, &cs0_fimage};
static ST_Flash_Base_Dev_Info  dev_uboot          = {-1, &cs0_uboot};

static ST_Flash_Base_Dev_Info  dev_kernel         = {-1, &cs0_kernel};
static ST_Flash_Base_Dev_Info  dev_rootfs         = {-1, &cs0_rootfs};

static ST_Flash_Base_Dev_Info  dev_env            = {-1, &cs0_env};
static ST_Flash_Base_Dev_Info  dev_log            = {-1, &cs0_log};
static ST_Flash_Base_Dev_Info  dev_logB           = {-1, &cs0_logB};   
static ST_Flash_Base_Dev_Info  dev_logC           = {-1, &cs0_logC};  
static ST_Flash_Base_Dev_Info  dev_logD           = {-1, &cs0_logD}; 

static ST_Flash_Base_Dev_Info  dev_conf           = {-1, &cs0_conf};
static ST_Flash_Base_Dev_Info  dev_app            = {-1, &cs0_app};
static ST_Flash_Base_Dev_Info  dev_wififirmware   = {-1, &cs0_wififirmware}; 


static ST_Flash_Base_Dev_Info  dev_userenv0       = {-1, &cs0_env0};
static ST_Flash_Base_Dev_Info  dev_userenv1       = {-1, &cs0_env1}; 
static ST_Flash_Base_Dev_Info  dev_backapp        = {-1, &cs0_appbak};

#if FLASH_CFG_USER_BURN
static ST_Flash_Base_Dev_Info  dev_userDefCfg     = {-1, &cs0_userCfgConf};
#endif

static ST_Flash_Base_Dev_Info  dev_specialcfg     = {-1, &cs0_specialcfg}; 
static ST_Flash_Base_Dev_Info  dev_pp             = {-1, &cs0_pp};



#define FB_FLASH_DEV_NAME "fh_flash"  //flash设备名

static int sFlashBaseInited = 0;
static rt_device_t  sFlashDev = NULL;
static struct rt_device_blk_geometry sFlashInfo;
static int sFlashWriteEnable = 1;


static ST_Flash_Base_Dev_Info *get_dev_info(FlashBase_FileType_e type, unsigned char reopenF)
{
    ST_Flash_Base_Dev_Info *pdev_info = NULL;

    switch(type)
    {
        case EN_FLASH_FILE_TYPE_FIMAGE:
            pdev_info = &dev_fimage;
            break;      
        case EN_FLASH_FILE_TYPE_BOOT:
            pdev_info = &dev_uboot;
            break;
        case EN_FLASH_FILE_TYPE_FIRMWARE:
            //pdev_info = &dev_ecos;
            break;
        case EN_FLASH_FILE_TYPE_KERNEL:
            pdev_info = &dev_kernel;
            break;
        case EN_FLASH_FILE_TYPE_ENV:
            pdev_info = &dev_env;
            break;

        case EN_FLASH_FILE_TYPE_LOG:
            pdev_info = &dev_log;
            break;
        case EN_FLASH_FILE_TYPE_LOG_B:
            pdev_info = &dev_logB;
            break;
        case EN_FLASH_FILE_TYPE_LOG_C:
            pdev_info = &dev_logC;
            break;
        case EN_FLASH_FILE_TYPE_LOG_D:
            pdev_info = &dev_logD;
            break;
        case EN_FLASH_FILE_TYPE_CFG:
            pdev_info = &dev_conf;
            break;
        case EN_FLASH_FILE_TYPE_APP:
            pdev_info = &dev_app;
            break;
        case EN_FLASH_FILE_TYPE_ROOTFS:
            pdev_info = &dev_rootfs;
            break;
        case EN_FLASH_FILE_TYPE_WIFIFIRMWARE:
            pdev_info = &dev_wififirmware;
            break;
        case EN_FLASH_FILE_TYPE_SPECIALCFG:
            pdev_info = &dev_specialcfg;
            break;
        case EN_FLASH_FILE_TYPE_PERMANENT:
            pdev_info = &dev_pp;
            break;
        case EN_FLASH_FILE_TYPE_BAKAPP:
            pdev_info = &dev_backapp;
            break;
        case EN_FLASH_FILE_TYPE_ENV0:
            pdev_info = &dev_userenv0;
            break;
        case EN_FLASH_FILE_TYPE_ENV1:
            pdev_info = &dev_userenv1;
            break;
            
    #if FLASH_CFG_USER_BURN        
        case EN_FLASH_FILE_TYPE_USERBURN_CFG:
            pdev_info = &dev_userDefCfg;
            break;
    #endif
        case EN_FLASH_FILE_TYPE_BAKKERNEL:
        case EN_FLASH_FILE_TYPE_BAKLOG:
        case EN_FLASH_FILE_TYPE_BAKFIRMWARE:
        default:
            fl_printf("FLASH NO BACKDATA...\n");
            break;
     }
     
     return pdev_info;
}

int Flash_Base_Init(void)
{
    if(sFlashBaseInited)
        return 0;

    rt_device_t _middev;
    int ret = 0;

    _middev = (rt_device_t)rt_device_find(FB_FLASH_DEV_NAME);
    if(_middev == NULL)
        return -1;
    else
        fl_printf("flash device find!\n");

    if(_middev->init)
        _middev->init(_middev);

    if(_middev->open)
        ret = _middev->open(_middev, 0);
    if(ret != 0)
    {
        fl_printf("flashBaseOpen failed! %d\n", ret);
        return -2;
    }

    sFlashDev = _middev;

    if(sFlashDev)
    {
        ret = sFlashDev->control(sFlashDev, RT_DEVICE_CTRL_BLK_GETGEOME, &sFlashInfo);
        fl_printf("FlashInfo:::sectorNum[%lu],SectorPerSz[%lu],blockSz[%lu]\n",sFlashInfo.sector_count,sFlashInfo.bytes_per_sector,sFlashInfo.block_size);
    }

//    bootenv_init(dev_env.fd);
    if(sFlashDev)
        sFlashBaseInited = 1;

    return 0;
}

int Flash_Base_Denint(void)
{
    if(sFlashDev)
        sFlashDev->close(sFlashDev);

    sFlashDev = NULL;
    sFlashBaseInited = 0;

    return 0;
}

unsigned int Flash_Base_Read(FlashBase_FileType_e type, int offset, void *buffer, unsigned int size)

{
#if FLASH_MODULE
    ST_Flash_Base_Dev_Info *pdev_info = NULL;
    int allOffset;
    unsigned int temv1;
    unsigned int startPartSecNo,startPartValidLen = 0,startPartOffset; // 起始数据所在sector
    unsigned int midPartSecNo,midPartSecNum = 0,midValidlen = 0; // 中间段跨越的n个完整sector
    unsigned int tailPartSecNo,tailPartValidLen = 0;// 末尾所在的sector
    char *pSectorBuf = NULL,*pfill = NULL;

    fl_printf("flash read type %d, offset %d, size %d\n",type,offset,size);
    if(!sFlashBaseInited || !sFlashDev)
        return 0;

    pdev_info = get_dev_info(type,1);
    if(NULL == pdev_info)
        return 0;
    
    if((offset + size > pdev_info->layout->size) || size <= 0 || sFlashInfo.bytes_per_sector == 0) //
    {
        fl_printf("flash read Ill(offset=%d, size=%d)\n", offset, size);
        return 0;
    }
    
    allOffset = pdev_info->layout->offset + offset;
    
    temv1 = allOffset % sFlashInfo.bytes_per_sector;
    if(temv1) // 有余数 
    {
        startPartOffset = temv1;
        startPartValidLen = sFlashInfo.bytes_per_sector - temv1;
        if(startPartValidLen > size)
            startPartValidLen = size;
        startPartSecNo = allOffset / sFlashInfo.bytes_per_sector;
    }

    temv1 = size - startPartValidLen;
    if(temv1 >= sFlashInfo.bytes_per_sector)
    {
        midPartSecNum = temv1 / sFlashInfo.bytes_per_sector;
        midPartSecNo = (allOffset + startPartValidLen)/sFlashInfo.bytes_per_sector;
        midValidlen = midPartSecNum * sFlashInfo.bytes_per_sector;
    }

    if((startPartValidLen + midValidlen > size) || (size - startPartValidLen - midValidlen >= sFlashInfo.bytes_per_sector))
    {
        fl_printf("FlashErr:RdFail(NoPossible,L%d)\n",__LINE__);
        return 0;
    }

    temv1 = size - startPartValidLen - midValidlen;
    if(temv1 > 0)
    {
        tailPartSecNo = (allOffset + startPartValidLen + midValidlen)/sFlashInfo.bytes_per_sector;
        tailPartValidLen = temv1;
    }

    pSectorBuf = (char *)rt_malloc(sFlashInfo.bytes_per_sector + 4);
    if(!pSectorBuf)
    {
        fl_printf("FlashErr:MallocFail(L%d)\n",__LINE__);
        return 0;
    }
    pfill = (char *)buffer;

    if(startPartValidLen > 0)
    {
        sFlashDev->read(sFlashDev,startPartSecNo,pSectorBuf,1);
        memcpy(pfill,pSectorBuf + startPartOffset,startPartValidLen);
    }
    if(midValidlen > 0)
    {
        sFlashDev->read(sFlashDev,midPartSecNo,(pfill + startPartValidLen),midPartSecNum);
    }
    if(tailPartValidLen > 0)
    {
        sFlashDev->read(sFlashDev,tailPartSecNo,pSectorBuf,1);
        memcpy((pfill + startPartValidLen + midValidlen),pSectorBuf,tailPartValidLen);
    }
    rt_free(pSectorBuf);
    return size;
#else
    return 0;
#endif
}

// 驱动层是按照扇区为单位来读写的  对于首尾残余需要先读再写
unsigned int Flash_Base_Write(FlashBase_FileType_e type, int offset, void *buffer, unsigned int size)
{
#if FLASH_MODULE
    ST_Flash_Base_Dev_Info *pdev_info;
    int allOffset;
    unsigned int temv1;
    unsigned int startPartSecNo,startPartValidLen = 0,startPartOffset; // 起始数据所在sector
    unsigned int midPartSecNo,midPartSecNum = 0,midValidlen = 0; // 中间段跨越的n个完整sector
    unsigned int tailPartSecNo,tailPartValidLen = 0;// 末尾所在的sector
    char *pSectorBuf = NULL, *pSrc = NULL;	
    struct rt_device_blk_sectors sectors;

    if(!sFlashBaseInited || !sFlashDev || !sFlashWriteEnable)
        return 0;

    pdev_info = get_dev_info(type,1);
    if(NULL == pdev_info)
        return 0;
        
    if((offset + size > pdev_info->layout->size) || size <= 0 || sFlashInfo.bytes_per_sector == 0)
    {
        fl_printf("flash write Ill(offset=%d, size=%d)\n", offset, size);
        return 0;
    }
    
    fl_printf("Flash Write: %s\n",pdev_info->layout->device_pathname);     

    allOffset = pdev_info->layout->offset + offset;
    
    temv1 = allOffset % sFlashInfo.bytes_per_sector;
    if(temv1) // 有余数 
    {
        startPartOffset = temv1;
        startPartValidLen = sFlashInfo.bytes_per_sector - temv1;
        if(startPartValidLen > size)
            startPartValidLen = size;
        startPartSecNo = allOffset / sFlashInfo.bytes_per_sector;
    }

    temv1 = size - startPartValidLen;
    if(temv1 >= sFlashInfo.bytes_per_sector)
    {
        midPartSecNum = temv1 / sFlashInfo.bytes_per_sector;
        midPartSecNo = (allOffset + startPartValidLen)/sFlashInfo.bytes_per_sector;
        midValidlen = midPartSecNum * sFlashInfo.bytes_per_sector;
    }

    if((startPartValidLen + midValidlen > size) || (size - startPartValidLen - midValidlen >= sFlashInfo.bytes_per_sector))
    {
        fl_printf("FlashErr:WrFail(NoPossible,L%d)\n",__LINE__);
        return 0;
    }

    temv1 = size - startPartValidLen - midValidlen;
    if(temv1 > 0)
    {
        tailPartSecNo = (allOffset + startPartValidLen + midValidlen)/sFlashInfo.bytes_per_sector;
        tailPartValidLen = temv1;
    }

    pSectorBuf = (char *)rt_malloc(sFlashInfo.bytes_per_sector + 4);
    if(!pSectorBuf)
    {
        fl_printf("FlashErr:MallocFail(L%d)\n",__LINE__);
        return 0;
    }
    pSrc = (char *)buffer;

    if(startPartValidLen > 0)
    {
        sFlashDev->read(sFlashDev,startPartSecNo,pSectorBuf,1);
        memcpy(pSectorBuf + startPartOffset,pSrc,startPartValidLen);

		sectors.sector_begin = startPartSecNo * sFlashInfo.bytes_per_sector;
		sectors.sector_end = sectors.sector_begin + sFlashInfo.bytes_per_sector;
		sFlashDev->control(sFlashDev,RT_DEVICE_CTRL_BLK_ERASE,&sectors); // LB2 2018.01 add

		sFlashDev->write(sFlashDev,startPartSecNo,pSectorBuf,1);
    }
    if(midValidlen > 0)
    {     
	    sectors.sector_begin = midPartSecNo * sFlashInfo.bytes_per_sector;
        sectors.sector_end = (midPartSecNo+midPartSecNum) * sFlashInfo.bytes_per_sector;
		sFlashDev->control(sFlashDev,RT_DEVICE_CTRL_BLK_ERASE,&sectors); // LB2 2018.01 add
	
        sFlashDev->write(sFlashDev,midPartSecNo,(pSrc + startPartValidLen),midPartSecNum);
    }
    if(tailPartValidLen > 0)
    {
        sFlashDev->read(sFlashDev,tailPartSecNo,pSectorBuf,1);
        memcpy(pSectorBuf,(pSrc+ startPartValidLen + midValidlen),tailPartValidLen);

        sectors.sector_begin = tailPartSecNo * sFlashInfo.bytes_per_sector;  // 以 byte计
        sectors.sector_end = (tailPartSecNo+1) * sFlashInfo.bytes_per_sector;
		sFlashDev->control(sFlashDev,RT_DEVICE_CTRL_BLK_ERASE,&sectors); // LB2 2018.01 add
		
        sFlashDev->write(sFlashDev,tailPartSecNo,pSectorBuf,1);
    }

    rt_free(pSectorBuf);
    return size;
#else
    return 0;
#endif
}
s32 spi_flash_read(u32 addr,void *buf,u32 len)
{
    Flash_Base_Init();
    return Flash_Base_Read(EN_FLASH_FILE_TYPE_KERNEL,addr,buf, len);
}


s32 spi_flash_write(u32 addr,const void *buf,u32 len )//不需要擦除，写函数中自己带有擦除
{
    Flash_Base_Init();
    return Flash_Base_Write(EN_FLASH_FILE_TYPE_KERNEL,addr,buf, len);
}
#define CHCHE_ADDR          0x3E000//cache区

void xgd_update_write(u8 type)
{
    u8 i;
    download_flag_t dl_flag;

    Flash_Base_Init();
    if(!(type == 0 || type == 0xef))
        return ;
        
    dl_flag.dl_flag = 0x5a5a5aa5;
    memcpy(dl_flag.name,"XGD",3);
    dl_flag.type_flag = type;
    spi_flash_write(CHCHE_ADDR, (u8 *)&dl_flag, sizeof(download_flag_t));
}
#if 0 /* 2019-5-17*/
u8 data_1[256];
void flash_write(u32 addr,u8 flag)
{
    memset(data_1,flag,256);
    spi_flash_write((0x280000 - 1024)+ addr,data_1,256);
    return ;
}

void flash_read(u32 addr)
{
    memset(data_1,0,256);
    spi_flash_read((0x280000 - 1024)+ addr,data_1,256);
    printf_format(data_1, 256);
    return ;
}

#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT(flash_write, flash_write(addr, flag));
FINSH_FUNCTION_EXPORT(flash_read, flash_read(addr));
#endif
#endif

