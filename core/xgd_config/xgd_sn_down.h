#ifndef __XGD_SN_H__
#define __XGD_SN_H__
#include "common.h"
#include <rtthread.h>
#include <rtdevice.h>
#include <dfs_posix.h>


/*--------------------------*/
struct _strFrame01
{
    u8 Valid;
    
    u8 frame;
    u8 cmd;
    u8 *buf;
    u16 len;
};
struct sParaseFrameCtrl//控制解析帧过程
{
    u8 SohCnt;  //报文头计数器
    u8 Step;    //解析步骤
    u8 Bcc;    
    u16 Cnt;
};
#define SOH             0x01        /*帧头标识*/
#define FRAME_O1_DATA_MAX 100 //zg 银联改造需要扩大空间

#define KEY_TEXT        0x12        /*键盘正文帧标识*/
#define KEY_ORDER       0x13        /*键盘命令帧标识*/
#define KEY_LTEXT       0x15        /*键盘扩展正文帧标识*/
#define TEXTEX      0x14            //扩展协议
#define PTE_LTEXT       0x45        /*PTE中使用的扩展文本帧*/

/*    PTE 01 命令 */
#define PTE_GET_M_NUM                   0x06        //获取机型号与硬件版本
#define PTE_GET_FUSE                    0xA0        //获取BOOT熔丝版本。
#define PTE_GET_CPUID                   0xA1        //获取CPU ID(8 byte)
#define PTE_GET_FLASHID                 0xA2        //获取 FLASH ID(16 byte)
#define PTE_INJECT_MACID                0xA3        //注入MAC ID 12 byte
#define PTE_INJECT_CPUID_FLASHID        0xA4        //注入经加密的CPUID跟FLASHID(256 byte)
#define PTE_GET_ID                      0xA5        //获取 CPUID(8 byte)+FLASHID(16 byte)+MACID(12 byte)
#define PTE_INJECT_BTNAME               0xA6        //注入蓝牙名称(3 byte < = BTNAME  <=16 byte)
#define PTE_DOWNLOAD_DEVICE_NUM         0xA7        //下载设备号
#define PTE_DOWNLOAD_PSAMID             0xA8        //下载PSAMID
#define PTE_DOWNLOAD_IDENTIFICATION     0xA9        //下载客户唯一标识码
#define PTE_DOWNLOAD_POSSN              0xAA        //下载机身号
#define PTE_DOWNLOAD_TIME               0xAB        //下载时间
#define PTE_CHECK_VERSION				0xAC		//核对版本
#define PTE_CHECK_CUSTOM				0xAD		//核对客制信息
#define PTE_DOWNLOAD_PUB				0xAE		//下载pub.csr证书文件
#define PTE_DOWNLOAD_SK				    0xAF		//下载sk.der文件
#define PTE_CONTROL_RESET				0xB0		//MES 控制重启
#define PTE_GET_BT_RESULT               0x50        //获取蓝牙测试结果
#define PTE_GET_QR_INFO                 0x51        //获取扫码结果，便于比对，扫码产品特加


#define POSSN_MAX_SIZE      29 //底层最大存储29字节

 
#define SECURITY_ADDR         		(0x280000 - 1024)//sn和一些加密数据存放在这里，core的最后一k的空间
#define SECURITY_SIZE         		(1*1024UL)//1k

#define SN_ADDR (SECURITY_ADDR+0)//SN机身号的组成XGD(3)+len(1)+data(29)+SUM(1) = 34字节
#define SN_SIZE 34
#define CIPHERTEXT_ADDR (SN_ADDR + SN_SIZE)//密文的组成XGD(3)+len(1)+data(251)+SUM(1) = 256字节
#define CIPHERTEXT_SIZE (255)
#define SN 0
#define CIPHERTEXT 1
#define p_func rt_kprintf("IN %s \r\n",__FUNCTION__)
#define sp rt_kprintf



s32 xgd_default_sn(void);
s32 xgd_info_read(u8 type,u8 *data,u32 lenth);
s32 xgd_info_write(u8 type,u8 *data,u32 len);
s32 xgd_sn_exist(void);
void xgd_sn_init(void);
#endif
