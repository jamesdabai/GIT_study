#include <stdio.h>
#include <rtdef.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <dfs_posix.h>

#include "dev_ini.h"
#include "xgd_config.h"
#include "xgd_config_deal.h"



/* 配置说明

****************************************
******CHAPTER  1:  准备您的设备
****************************************

1. 重置默认值：
P666667 出厂设置
P6666FB 获取现有版本号

2. 接口设置
P66A667 USB Keyboard
P66A668 USB COM
P66A666 RS232

****************************************
******CHAPTER  2:  输出配置********
***************************************

回车/换行设置
==============
E99311 添加回车
E99310 取消回车
E99211 添加换行
E99210 取消换行
P66A66E 添加回车/换行
P66A66F 取消回车/换行

从开始/结尾去掉条码
===================
AFC010 从开始去掉条码
AFC011 从结尾去掉条码
AFD08X 去掉条码的位数（最后的1代表去掉一位，如果为2去掉两位，如果为0则正常不去掉，用户可自己配置）

键盘模式输出中文
===============
BFE030 默认
BFE031 可用于word qq，不可用于excel，记事本
BFE032 可用于记事本、excel，不能用于word

键盘语言国家类型
================
E9D040 比利时
E9D041 英国
E9D042 法国
E9D043 德国
E9D044 意大利
E9D045 西班牙
E9D046 美国
E9D048 新加坡
E9D049 萨尔瓦多
E9D0410 日本
E9D0411 塞拉利昂
E9D0412 土耳其
E9D0413 俄罗斯
E9D0414 匈牙利
E9D0415 俄语(俄罗斯)
BFC0816 泰语


模拟键盘
========
BFB011 启用模拟键盘
BFB010 禁用(关闭)模拟键盘
BFB111 开启模拟键盘前面为零
BFB110 禁用(关闭)模拟键盘前面为零


大小写切换
==========
BFD021 全小写
BFD022 全大写


****************************************
*******CHAPTER  3:  功能模式设置
****************************************

工作模式
=========
E7C442 手动模式
B8F011 IR感应模式开
B8F010 IR模式关

图像
AFE411 图像反白
AFE410 正常图像

Aimer setting
AFF111 Aimer使能
AFF110 Aimer禁止

照明配置
AFF211 Light使能
AFF210 Light禁止

指示灯设置
AFF321 LED指示灯正常
AFF320 LED指示灯反向
AFF323 LED指示灯一直关闭
AFF322 LED指示灯一直打开


喇叭设置
AFF710 喇叭打开
AFF711 喇叭关闭

Beeper持续时间
E7B410 正常
E7B411 短促

Beep音调设置
E7A537 2.7KHz
E7A536 1.6KHz
E7A535 2.0KHz
E7A534 2.4KHz
E7A533 3.1KHz
E7A532 3.5KHz
E7A530 不发声

测试模式
=========
P666669 设备配置为blink测试模式
P666668 取消blink测试模式

休眠设置
B8A0810 10s休眠
B8A08100 100s休眠
(制作配置条码时前面要加”^3”字符，如：^3B8A08X(X表示休眠时间)，选择code 128码。)

读码超时设置
AFB0820 30s
AFB0840 60s
AFB0880 120s
AFB08120 180s
AFB08160 240s
AFB08200 300s


接口设置
EA7040 300
EA7041 600
EA7043 2400 
EA7044 4800
EA7045 9600
EA7046 14400
EA7047 19200
EA7048 38400
EA7049 57600
EA70410 115200

数据位
E9F310 7位
E9F311 8位

校验位设置
E9F530 Odd 奇数
E9F531 S
E9F532 E
E9F533 M
E9F534 N

****************************************
*******CHAPTER  4:  条码码制配置
****************************************

UPC/EAN
FFE611 使能
FFE610 禁止

Codaba
FFE411 使能
FFE410 禁止
C76220 无校验
C76221 打开校验
C76222 打开校验并传输校验符
F88711 输出起始结束符
F88710 不输出起始结束符

Code39
FFE111 使能
FFE110 禁止
C6F022 无校验
C6F020 打开校验
C6F021 打开校验并传输校验符
C6F210 输出起始结束符
C6F211 不输出起始结束符

Full ASCII Code39
FFD711 使能
FFD710 禁止

Interleaved 2 of 5
FFE511 使能
FFE510 禁止
C76021 无校验
C76020 打开校验
C76022 打开校验并传输校验符

Code92
FFE211 使能
FFE210 禁止

Straight 2 of 5 Industrial
FFE011 使能
FFE010 禁止

Code11
FFF311 使能
FFE310 禁止
F7F510 两个校验位
F7F511 一个校验位

Code128
FFE311 使能
FFE310 禁止

UPC-A
FFD611 使能
FFD610 禁止
F8A610 UPC-E输出校验位
F8A611 UPC-E不输出校验位
F8A310 UPC-E输出头字符
F8A311 UPC-E不输出头字符
F8A411 UPC-E转EAN13码

EAN-13
FFD111 使能
FFD110 禁止

EAN-8
FFD411 使能
FFD410 禁止

MSI
FFD211 使能
FFD210 禁止


14.GS1 DataBar Omnidirectional

15. GS1 DataBar Limited

16. GS1 DataBar Expanded

17. Trioptic Code

18. Codablock A

19. Codablock F

20. PDF417

21. MicroPDF417

22. QR Code
FF9221 使能
FF9220 禁止
FF9310 反色QR使能
FF9311 反色QR禁止

23. Mico QR Code

24. Data Matrix Code

25. Aztec Code

26. Hong Kong 2 of 5(China post)

27. Airline 2 of 5

28. Matrix 2 of 5

29. Code 32


****************************************
*******CHAPTER 5: 特殊功能配置（范例）
****************************************


扫增值税码

设置条码长度


编程模式

*/

static ini_ctrl_t *ini_handle = NULL;

const DEFAULT_CFG_XGD config_default[30] = 
{
    {"SET_FACTOORY_SETTINGS","0"},
    {"SET_ENTER_NEWLINE","0"},
    {"SET_COM_MODE","0"},
    {"SET_START_UPGRADE","0"},
    {"SET_CODE_TYPE","7"},//默认三种码制全开
    {"SET_SCAN_CODE","0"},
    {"SET_READ_CODE","0"},
    {"SET_COLUME","3"},
    {"SET_BAUD","7"},
    {"SET_SCAN_DELAY","3"},
    {"SET_VERSION_CHECK","0"},
    {"SET_HARD_CONFIG","0"},//深锅配置，默认
    {"SET_VOICE","0"},//配置语音播报，默认滴滴声
    {"SET_END","0"}
};

static int xgd_config_default(void)
{
    int fd;
    int i;
    
    fd = open(XGD_CFG_FILE,O_WRONLY | O_CREAT | O_TRUNC, 0);
	if (fd < 0)
	{
		rt_kprintf("open file for write failed\n");
		return -1;
	}
	ini_handle = dev_ini_load(XGD_CFG_FILE);
    if(ini_handle == NULL)
    {
        printf("Load ini file:%s error\n",XGD_CFG_FILE);
        return -1;
    }
    for(i=0;;i++)
    {
        if(strcmp(config_default[i].key,"SET_END")==0)
            break;
        xgd_config_set(SECTION_NAEM,config_default[i].key,config_default[i].value);
    }
    xgd_config_save();
    //xgd_config_free();
    close(fd);

    return 0;    
}

int xgd_config_init(void)
{
    ini_handle = dev_ini_load(XGD_CFG_FILE);
    if(NULL == ini_handle)
    {
        xgd_config_default();
        ini_handle = dev_ini_load(XGD_CFG_FILE);
    }
    return 0;
}

/* 返回获取到的value字符串的长度 */
int xgd_config_get(const char* section, const char* key, char* value,int len)
{
    return dev_ini_get_key(ini_handle,section,key,value);
}

int xgd_config_set(const char* section, const char* key, char* value)
{
    return dev_ini_set_key(ini_handle,section,key,value,NULL);
}

int xgd_config_del_section(const char* section)
{
    return dev_ini_del_section(ini_handle,section);
}

int xgd_config_del_key(const char* section, const char* key)
{
    return dev_ini_del_key(ini_handle,section,key);
}

int xgd_config_save(void)
{
    return dev_ini_save(ini_handle);
}

int xgd_config_free(void)
{
    int ret;
    ret = dev_ini_free(ini_handle);
    return ret;
}
int xgd_config_write(int id, char value)//重新包装write和read两个函数
{
    char *p;
    int ret;
    char  str[2]; 
    if(ini_handle == NULL)
    {
        xgd_config_init();
    }
    p = config_default[id].key;
    sprintf(str,"%d",value);
    ret = xgd_config_set(SECTION_NAEM,p,str);
    xgd_config_save();
    return ret;
    
}

int xgd_config_read(int id)
{
    int num;
    int ret = -1;
    char value[2];
    if(ini_handle == NULL)
    {
       xgd_config_init();
    }
    if(id>SET_END_CFG)
        return -1;
    ret = xgd_config_get(SECTION_NAEM,config_default[id].key,value,0);
    if(ret<=0)
        return -1;
    num = atoi(value);
    return num ;
}
void xgd_config_test(void)
{
    int i;
    char value[2];
    for(i=0;;i++)
    {
        if(strcmp(config_default[i].key,"SET_END") == 0)
            break;
        xgd_config_get(SECTION_NAEM,config_default[i].key,value,0);
        rt_kprintf("id=%d -%s-----%s =%d\n",i,config_default[i].key,value,atoi(value));
    }
    rt_kprintf("######################\r\n");
}

