#include <stdio.h>
#include <rtdef.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <dfs_posix.h>

#include "dev_ini.h"
#include "xgd_config.h"
#include "xgd_config_deal.h"



/* ����˵��

****************************************
******CHAPTER  1:  ׼�������豸
****************************************

1. ����Ĭ��ֵ��
P666667 ��������
P6666FB ��ȡ���а汾��

2. �ӿ�����
P66A667 USB Keyboard
P66A668 USB COM
P66A666 RS232

****************************************
******CHAPTER  2:  �������********
***************************************

�س�/��������
==============
E99311 ��ӻس�
E99310 ȡ���س�
E99211 ��ӻ���
E99210 ȡ������
P66A66E ��ӻس�/����
P66A66F ȡ���س�/����

�ӿ�ʼ/��βȥ������
===================
AFC010 �ӿ�ʼȥ������
AFC011 �ӽ�βȥ������
AFD08X ȥ�������λ��������1����ȥ��һλ�����Ϊ2ȥ����λ�����Ϊ0��������ȥ�����û����Լ����ã�

����ģʽ�������
===============
BFE030 Ĭ��
BFE031 ������word qq����������excel�����±�
BFE032 �����ڼ��±���excel����������word

�������Թ�������
================
E9D040 ����ʱ
E9D041 Ӣ��
E9D042 ����
E9D043 �¹�
E9D044 �����
E9D045 ������
E9D046 ����
E9D048 �¼���
E9D049 �����߶�
E9D0410 �ձ�
E9D0411 ��������
E9D0412 ������
E9D0413 ����˹
E9D0414 ������
E9D0415 ����(����˹)
BFC0816 ̩��


ģ�����
========
BFB011 ����ģ�����
BFB010 ����(�ر�)ģ�����
BFB111 ����ģ�����ǰ��Ϊ��
BFB110 ����(�ر�)ģ�����ǰ��Ϊ��


��Сд�л�
==========
BFD021 ȫСд
BFD022 ȫ��д


****************************************
*******CHAPTER  3:  ����ģʽ����
****************************************

����ģʽ
=========
E7C442 �ֶ�ģʽ
B8F011 IR��Ӧģʽ��
B8F010 IRģʽ��

ͼ��
AFE411 ͼ�񷴰�
AFE410 ����ͼ��

Aimer setting
AFF111 Aimerʹ��
AFF110 Aimer��ֹ

��������
AFF211 Lightʹ��
AFF210 Light��ֹ

ָʾ������
AFF321 LEDָʾ������
AFF320 LEDָʾ�Ʒ���
AFF323 LEDָʾ��һֱ�ر�
AFF322 LEDָʾ��һֱ��


��������
AFF710 ���ȴ�
AFF711 ���ȹر�

Beeper����ʱ��
E7B410 ����
E7B411 �̴�

Beep��������
E7A537 2.7KHz
E7A536 1.6KHz
E7A535 2.0KHz
E7A534 2.4KHz
E7A533 3.1KHz
E7A532 3.5KHz
E7A530 ������

����ģʽ
=========
P666669 �豸����Ϊblink����ģʽ
P666668 ȡ��blink����ģʽ

��������
B8A0810 10s����
B8A08100 100s����
(������������ʱǰ��Ҫ�ӡ�^3���ַ����磺^3B8A08X(X��ʾ����ʱ��)��ѡ��code 128�롣)

���볬ʱ����
AFB0820 30s
AFB0840 60s
AFB0880 120s
AFB08120 180s
AFB08160 240s
AFB08200 300s


�ӿ�����
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

����λ
E9F310 7λ
E9F311 8λ

У��λ����
E9F530 Odd ����
E9F531 S
E9F532 E
E9F533 M
E9F534 N

****************************************
*******CHAPTER  4:  ������������
****************************************

UPC/EAN
FFE611 ʹ��
FFE610 ��ֹ

Codaba
FFE411 ʹ��
FFE410 ��ֹ
C76220 ��У��
C76221 ��У��
C76222 ��У�鲢����У���
F88711 �����ʼ������
F88710 �������ʼ������

Code39
FFE111 ʹ��
FFE110 ��ֹ
C6F022 ��У��
C6F020 ��У��
C6F021 ��У�鲢����У���
C6F210 �����ʼ������
C6F211 �������ʼ������

Full ASCII Code39
FFD711 ʹ��
FFD710 ��ֹ

Interleaved 2 of 5
FFE511 ʹ��
FFE510 ��ֹ
C76021 ��У��
C76020 ��У��
C76022 ��У�鲢����У���

Code92
FFE211 ʹ��
FFE210 ��ֹ

Straight 2 of 5 Industrial
FFE011 ʹ��
FFE010 ��ֹ

Code11
FFF311 ʹ��
FFE310 ��ֹ
F7F510 ����У��λ
F7F511 һ��У��λ

Code128
FFE311 ʹ��
FFE310 ��ֹ

UPC-A
FFD611 ʹ��
FFD610 ��ֹ
F8A610 UPC-E���У��λ
F8A611 UPC-E�����У��λ
F8A310 UPC-E���ͷ�ַ�
F8A311 UPC-E�����ͷ�ַ�
F8A411 UPC-EתEAN13��

EAN-13
FFD111 ʹ��
FFD110 ��ֹ

EAN-8
FFD411 ʹ��
FFD410 ��ֹ

MSI
FFD211 ʹ��
FFD210 ��ֹ


14.GS1 DataBar Omnidirectional

15. GS1 DataBar Limited

16. GS1 DataBar Expanded

17. Trioptic Code

18. Codablock A

19. Codablock F

20. PDF417

21. MicroPDF417

22. QR Code
FF9221 ʹ��
FF9220 ��ֹ
FF9310 ��ɫQRʹ��
FF9311 ��ɫQR��ֹ

23. Mico QR Code

24. Data Matrix Code

25. Aztec Code

26. Hong Kong 2 of 5(China post)

27. Airline 2 of 5

28. Matrix 2 of 5

29. Code 32


****************************************
*******CHAPTER 5: ���⹦�����ã�������
****************************************


ɨ��ֵ˰��

�������볤��


���ģʽ

*/

static ini_ctrl_t *ini_handle = NULL;

const DEFAULT_CFG_XGD config_default[30] = 
{
    {"SET_FACTOORY_SETTINGS","0"},
    {"SET_ENTER_NEWLINE","0"},
    {"SET_COM_MODE","0"},
    {"SET_START_UPGRADE","0"},
    {"SET_CODE_TYPE","7"},//Ĭ����������ȫ��
    {"SET_SCAN_CODE","0"},
    {"SET_READ_CODE","0"},
    {"SET_COLUME","3"},
    {"SET_BAUD","7"},
    {"SET_SCAN_DELAY","3"},
    {"SET_VERSION_CHECK","0"},
    {"SET_HARD_CONFIG","0"},//������ã�Ĭ��
    {"SET_VOICE","0"},//��������������Ĭ�ϵε���
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

/* ���ػ�ȡ����value�ַ����ĳ��� */
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
int xgd_config_write(int id, char value)//���°�װwrite��read��������
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

