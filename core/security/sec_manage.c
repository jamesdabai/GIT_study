#include <rtthread.h>
#include <dfs_posix.h>

#include "fh_aes.h"
#include "fh_crypto_api.h"
#include <stdlib.h>
#include <xgd_sn_down.h>

/* 0 -- not initial or check error
   1 -- check pass   */
static int  security_status = 0;
static int read_efuse_otp_id(char *pbuf,int len)
{
    int ret,copy_len;
    CRYPTO_HANDLE pCryptoHandle;
    rt_uint8_t result[8];

    ret = FH_CRYPTO_Init();
    if(ret != 0)
    {
        rt_kprintf("FH_CRYPTO_Init failed, ret = %d\n", ret);
    }
    
    rt_memset(&pCryptoHandle,0,sizeof(pCryptoHandle));
    ret = FH_CRYPTO_CreateHandle(&pCryptoHandle); // �ɲ�����
    if(ret != 0)
    {
        rt_kprintf("FH_CRYPTO_CreateHandle failed, ret = %d\n", ret);
        return -3;
    }
    
    rt_memset(result, 0, sizeof(result));
    ret = FH_CRYPTO_ReadOTPDeviceID(result, 8);
    if(ret != 0)
    {
        rt_kprintf("FH_CRYPTO_ReadOTPDeviceID failed, ret = %d\n", ret);
    }

    if(len > 8)
    {
        copy_len = 8;
    }
    else
    {
        copy_len = len;
    }
    memcpy(pbuf,result,copy_len);

    ret = FH_CRYPTO_DestroyHandle(&pCryptoHandle);
    if(ret != 0)
    {
        rt_kprintf("FH_CRYPTO_DestroyHandle failed, ret = %d\n", ret);
    }

    ret = FH_CRYPTO_DeInit();
    if(ret != 0)
    {
        rt_kprintf("FH_CRYPTO_DeInit failed, ret = %d\n", ret);
    }
    
    return copy_len;
}

int get_security_stat(void)
{
    return security_status;
}
void printf_format(u8 *buf, s32 len)
{
	int i;
	rt_kprintf("\r\n");
	for(i=0; i<len; i++)
	{
		if(i>0 && (i%16)==0)
		{
			rt_kprintf("\r\n");
		}
		rt_kprintf("%02x ", buf[i]);
	}
	rt_kprintf("\r\n");
}

/****************************************************************************
**Description:           ͨ������ţ�tick����α���������Ϊ��Կ
**Input parameters:      
**Output parameters:     
**                       
**Returned value:        
**                       
**Created by:            (2019-5-16)
**--------------------------------------------------------------------------
**Modified by:          
**Modified by:          
****************************************************************************/
s32 generate_random(u8 *data,u8 len,s32 send)
{
    s32 tick;
    s32 ret;
    u8  sn_buff[29];
    u8 i;
    s32 sum = 0;

    tick = rt_tick_get();
    ret = xgd_info_read(SN,sn_buff,32);
    if(ret != -1)
    {
        for(i=0;i<ret;i++)
        {
            sum +=sn_buff[i];
        }
    }
    sum += tick;
    if(send == 0xff)
    {
        srand(sum);//������ʱ��ͻ������ص�����
    }
    else
    {
        srand(send);//�����Ͳ���send��ص�����
    }
    for(i=0;i<len;i++)
    {
        data[i] = rand();
    }
    //printf_format(data,len);
    return len;
    
}

/****************************************************************************
**Description:      ���ݻ���Ż�ȡ����     
**Input parameters:      
**Output parameters:     
**                       
**Returned value:        
**                       
**Created by:            xqy(2019-5-16)
**--------------------------------------------------------------------------
**Modified by:          
**Modified by:          
****************************************************************************/
s32 get_text(u8 *data)
{
    s32 ret = -1;
    u8 i,j = 0;
    u8 len = 0;
    u8 sn_buff[29];
    u8 temp[16];
    ret = xgd_info_read(SN,sn_buff,32);
    if(ret == -1)
    {
        return -1;
    }  
    if(ret > 6)
    {
        i = ret - 6;//ֻȡ����λ���������������
    }
    else
    {
        i = 0;//��Ȼ��ȫ����Ϊ���������
    }
    for(;i<ret;i++)
    {
        generate_random(temp,16,sn_buff[ret-i-1]);
        memcpy(&data[(j++)*16],temp,16);
        len += 16;
    }
    return len;
}
static u8 laws[100];
static u8 secret[100];//�������ĵ�������ʱ�Ļ�����

s32 injection_key(void)
{
    s32 ret = -1;
    u8 len = 0;
    u8 sn_buff[29];
    u8 secret_key[48];

    memset(sn_buff,0,29);
    memset(secret_key,0,48);
    ret = xgd_info_read(SN,sn_buff,32);
    if(ret == -1)
    {
        return -1;
    }   
    generate_random(secret_key,48,0xff);
    efuse_write_key(0,secret_key);//д���һ����Կ
    efuse_write_key(1,secret_key);//д��ڶ�����Կ
    efuse_write_key(2,secret_key);//д���������Կ

    memset(laws,0,100);
    memset(secret,0,100);
    len = get_text(laws);
    ret = efuse_aes_enc(laws,secret,len);//��ȡ�����ܵ�����secret�����浽flash��
    if(ret == 0)
    {
        xgd_info_write(CIPHERTEXT,secret,len);
    }

    memset(laws,0,100);
    memset(secret,0,100);
    ret = xgd_info_read(CIPHERTEXT,secret,100);
    if(ret != -1)
    {
        rt_kprintf("��ʼ����\r\n");
        efuse_aes_dec(secret,laws,ret);//��������
    }
    len = get_text(secret);//��ȡ����
    if(memcmp(laws,secret,len) == 0)//�Ա����ĺ�����
    {
        u8 sn_len = strlen(sn_buff);
        sn_len > 8 ? (sn_len = sn_len - 8) : (sn_len = 0);
        efuse_write_id(&sn_buff[sn_len]);//д��device ID ֻ��дһ��
        rt_kprintf("%s���ӽ��ܳɹ����Ƚϳɹ�\r\n",__FUNCTION__);
    }
    else
    {
        rt_kprintf("%s���ӽ���ʧ�ܣ��Ƚ�ʧ��\r\n",__FUNCTION__);
    }
    return 0;
}

int security_init(void)
{
    s32 ret = -1;
    memset(laws,0,100);
    memset(secret,0,100);
    ret = xgd_info_read(CIPHERTEXT,secret,100);
    if(ret != -1)
    {
        efuse_aes_dec(secret,laws,ret);//��������
    }
    else
    {
        return -1;
    }
    get_text(secret);//��ȡ����
    if(memcmp(laws,secret,ret) == 0)//�Ա����ĺ�����
    {
        rt_kprintf("%s����ȫ����\r\n",__FUNCTION__);
        security_status = 1;
    }
    else
    {
        rt_kprintf("%s���ǰ�ȫ����\r\n",__FUNCTION__);
    }
    return 0;
}

void random_test(u8 len)
{
    u8 data[48];
    generate_random(data,len,0);
    rt_kprintf("%s\r\n",__FUNCTION__);
    printf_format(data, len);
}

void p_info(u16 len)
{
    u8 buff[300];
    spi_flash_read(SN_ADDR,buff,len);
    printf_format(buff,len);
    return ;

}


