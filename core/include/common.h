#ifndef COMMON_H__
#define COMMON_H__



#undef u8
#undef u16
#undef u32
#undef s8
#undef s16
#undef s32
typedef unsigned char   u8;
typedef unsigned short  u16;
typedef unsigned int    u32;
typedef char            s8;
typedef short           s16;
typedef int             s32;


#define DDI_OK 0 //成功
#define DDI_ERR -1 //错误
#define DDI_ETIMEOUT -2 //超时
#define DDI_EBUSY -3 //设备繁忙
#define DDI_ENODEV -4	//设备不存在
#define DDI_EACCES -5	//无权限
#define DDI_EINVAL -6  //参数无效
#define DDI_EIO -7 //设备未打开或设备操作出错
#define DDI_EDATA -8 //数据错误
#define DDI_EPROTOCOL -9 //协议错误
#define DDI_ETRANSPORT -10 //传输错误

#define NULL (void *)0

typedef enum {FALSE = 0, TRUE = !FALSE} bool;
#define uart_printf rt_kprintf

typedef struct //cache中的暂时的下载标志，放在cache地址起始的地方存放
{
    u32 dl_flag;//0x5a5a5aa5,下载标识
    u8  name[4];//XGD
    u32  type_flag;//0x0:普通的更新，敏感数据更新将失败，0xEF:u_boot,recovery,arc_rtt数据全部可以更新
}download_flag_t;


#endif


