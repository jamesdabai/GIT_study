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


#define DDI_OK 0 //�ɹ�
#define DDI_ERR -1 //����
#define DDI_ETIMEOUT -2 //��ʱ
#define DDI_EBUSY -3 //�豸��æ
#define DDI_ENODEV -4	//�豸������
#define DDI_EACCES -5	//��Ȩ��
#define DDI_EINVAL -6  //������Ч
#define DDI_EIO -7 //�豸δ�򿪻��豸��������
#define DDI_EDATA -8 //���ݴ���
#define DDI_EPROTOCOL -9 //Э�����
#define DDI_ETRANSPORT -10 //�������

#define NULL (void *)0

typedef enum {FALSE = 0, TRUE = !FALSE} bool;
#define uart_printf rt_kprintf

typedef struct //cache�е���ʱ�����ر�־������cache��ַ��ʼ�ĵط����
{
    u32 dl_flag;//0x5a5a5aa5,���ر�ʶ
    u8  name[4];//XGD
    u32  type_flag;//0x0:��ͨ�ĸ��£��������ݸ��½�ʧ�ܣ�0xEF:u_boot,recovery,arc_rtt����ȫ�����Ը���
}download_flag_t;


#endif


