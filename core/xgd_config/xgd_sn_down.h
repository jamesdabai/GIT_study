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
struct sParaseFrameCtrl//���ƽ���֡����
{
    u8 SohCnt;  //����ͷ������
    u8 Step;    //��������
    u8 Bcc;    
    u16 Cnt;
};
#define SOH             0x01        /*֡ͷ��ʶ*/
#define FRAME_O1_DATA_MAX 100 //zg ����������Ҫ����ռ�

#define KEY_TEXT        0x12        /*��������֡��ʶ*/
#define KEY_ORDER       0x13        /*��������֡��ʶ*/
#define KEY_LTEXT       0x15        /*������չ����֡��ʶ*/
#define TEXTEX      0x14            //��չЭ��
#define PTE_LTEXT       0x45        /*PTE��ʹ�õ���չ�ı�֡*/

/*    PTE 01 ���� */
#define PTE_GET_M_NUM                   0x06        //��ȡ���ͺ���Ӳ���汾
#define PTE_GET_FUSE                    0xA0        //��ȡBOOT��˿�汾��
#define PTE_GET_CPUID                   0xA1        //��ȡCPU ID(8 byte)
#define PTE_GET_FLASHID                 0xA2        //��ȡ FLASH ID(16 byte)
#define PTE_INJECT_MACID                0xA3        //ע��MAC ID 12 byte
#define PTE_INJECT_CPUID_FLASHID        0xA4        //ע�뾭���ܵ�CPUID��FLASHID(256 byte)
#define PTE_GET_ID                      0xA5        //��ȡ CPUID(8 byte)+FLASHID(16 byte)+MACID(12 byte)
#define PTE_INJECT_BTNAME               0xA6        //ע����������(3 byte < = BTNAME  <=16 byte)
#define PTE_DOWNLOAD_DEVICE_NUM         0xA7        //�����豸��
#define PTE_DOWNLOAD_PSAMID             0xA8        //����PSAMID
#define PTE_DOWNLOAD_IDENTIFICATION     0xA9        //���ؿͻ�Ψһ��ʶ��
#define PTE_DOWNLOAD_POSSN              0xAA        //���ػ����
#define PTE_DOWNLOAD_TIME               0xAB        //����ʱ��
#define PTE_CHECK_VERSION				0xAC		//�˶԰汾
#define PTE_CHECK_CUSTOM				0xAD		//�˶Կ�����Ϣ
#define PTE_DOWNLOAD_PUB				0xAE		//����pub.csr֤���ļ�
#define PTE_DOWNLOAD_SK				    0xAF		//����sk.der�ļ�
#define PTE_CONTROL_RESET				0xB0		//MES ��������
#define PTE_GET_BT_RESULT               0x50        //��ȡ�������Խ��
#define PTE_GET_QR_INFO                 0x51        //��ȡɨ���������ڱȶԣ�ɨ���Ʒ�ؼ�


#define POSSN_MAX_SIZE      29 //�ײ����洢29�ֽ�

 
#define SECURITY_ADDR         		(0x280000 - 1024)//sn��һЩ�������ݴ�������core�����һk�Ŀռ�
#define SECURITY_SIZE         		(1*1024UL)//1k

#define SN_ADDR (SECURITY_ADDR+0)//SN����ŵ����XGD(3)+len(1)+data(29)+SUM(1) = 34�ֽ�
#define SN_SIZE 34
#define CIPHERTEXT_ADDR (SN_ADDR + SN_SIZE)//���ĵ����XGD(3)+len(1)+data(251)+SUM(1) = 256�ֽ�
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
