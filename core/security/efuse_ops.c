#include <rtthread.h>
#include "fh_aes.h"
#include "fh_crypto_api.h"

// 需 define RT_USING_AES  RT_USING_EFUSE_MAP  RT_USING_ALGORITHM

#define AES_ONCE_ENC_DEC_MAX 2048  // aes api 单次加解密最长长度

// 烧写key到efuse 共有三个区(每个区16个字节)，每个区均可写一次  不可读
// index 取值[0,2]
int efuse_write_key(int index,rt_uint8_t *data)
{
    int ret;
    CRYPTO_HANDLE pCryptoHandle;
    rt_uint8_t *pKey = NULL;

    if(index < 0 || index > 2)
    {
        rt_kprintf("efuse write index ill(%d)...\n",index);
        return -1;
    }
    ret = FH_CRYPTO_Init();
    if(ret != 0)
    {
        rt_kprintf("FH_CRYPTO_Init failed, ret = %d\n", ret);
        return -2;
    }
    
    rt_memset(&pCryptoHandle,0,sizeof(pCryptoHandle));
    ret = FH_CRYPTO_CreateHandle(&pCryptoHandle); // 可不调用 
    if(ret != 0)
    {
        rt_kprintf("FH_CRYPTO_CreateHandle failed, ret = %d\n", ret);
        return -3;
    }
    pKey = &data[index*16];//
    ret = FH_CRYPTO_WriteOTPKey(index, pKey, 16);
    if(ret != 0)
    {
        rt_kprintf("FH_CRYPTO_WriteOTPKey failed, ret = %d\n", ret);
        return -4;
    }

    ret = FH_CRYPTO_DestroyHandle(&pCryptoHandle);
    if(ret != 0)
    {
        rt_kprintf("FH_CRYPTO_DestroyHandle failed, ret = %d\n", ret);
        return -8;
    }

    ret = FH_CRYPTO_DeInit();
    if(ret != 0)
    {
        rt_kprintf("FH_CRYPTO_DeInit failed, ret = %d\n", ret);
        return -9;
    }

    rt_kprintf("efuse write key(index %d) done!\n",index);
    return 0;
}
int efuse_aes_enc(char *IN ,char *OUT, int len)
{
    int ret, i;
    CRYPTO_HANDLE pCryptoHandle;
    CRYPTO_CTRL_S cryptoCtrl;
    ret = FH_CRYPTO_Init();
    if(ret != 0)
    {
        rt_kprintf("FH_CRYPTO_Init failed, ret = %d\n", ret);
        return -2;
    }
    
    rt_memset(&pCryptoHandle,0,sizeof(pCryptoHandle));
    ret = FH_CRYPTO_CreateHandle(&pCryptoHandle); // 可不调用 
    if(ret != 0)
    {
        rt_kprintf("FH_CRYPTO_CreateHandle failed, ret = %d\n", ret);
        return -3;
    }    
    rt_memset(&pCryptoHandle.request,0,sizeof(struct rt_crypto_request));
    rt_memset(&cryptoCtrl,0,sizeof(cryptoCtrl));
    cryptoCtrl.enKeySrc = CRYPTO_KEY_SRC_EFUSE; // aes key 来自efuse  efuse与aes模块有内部通道传递key 任何外界手段均无法探知
    cryptoCtrl.enAlg = CRYPTO_ALG_AES;
    cryptoCtrl.enWorkMode = CRYPTO_WORK_MODE_ECB;
    cryptoCtrl.enKeyLen = CRYPTO_KEY_AES_128BIT; // 16 bytes key
    cryptoCtrl.enIVLen = CRYPTO_IV_KEY_AES_0BIT;
    cryptoCtrl.enBitWidth = CRYPTO_BIT_WIDTH_128BIT;

    // efuse所烧key 被aes硬件模块使用时，是否作顺序map 具体见函数FH_CRYPTO_Mapswitch说明
    // 下述map16个字节正序作为aes key【0~3】【8~11】【16~19】【24~27】
    unsigned int u32MapBuf[4]; 
    u32MapBuf[0] = 0; 
    u32MapBuf[1] = 8; 
    u32MapBuf[2] = 16; 
    u32MapBuf[3] = 24; 
    ret = FH_CRYPTO_Mapswitch(&pCryptoHandle, u32MapBuf, 4);
    if(ret != 0)
    {
        rt_kprintf("FH_CRYPTO_Mapswitch failed, ret = %d\n", ret);
        return -4;
    }

    ret = FH_CRYPTO_ConfigHandle(&pCryptoHandle, &cryptoCtrl);
    if(ret != 0)
    {
        rt_kprintf("FH_CRYPTO_ConfigHandle failed, ret = %d\n", ret);
        return -5;
    }

    // aes 加密为对称加密 明文要求为16对齐  
    // 单次最大加密长度 AES_ONCE_ENC_DEC_MAX， 超过此长度需拆分多次
    int _encTimes = len / AES_ONCE_ENC_DEC_MAX;
    unsigned int _encRestLen = len % AES_ONCE_ENC_DEC_MAX;

    for(i = 0; i < _encTimes; i++)
    {
        ret = FH_CRYPTO_Encrypt(&pCryptoHandle, (unsigned int)&IN[i * AES_ONCE_ENC_DEC_MAX], \
            (unsigned int)&OUT[i * AES_ONCE_ENC_DEC_MAX], AES_ONCE_ENC_DEC_MAX);
        if(ret != 0)
            rt_kprintf("FH_CRYPTO_Encrypt failed, ret = %d\n", ret);
    }
    ret = FH_CRYPTO_Encrypt(&pCryptoHandle, (unsigned int)&IN[_encTimes * AES_ONCE_ENC_DEC_MAX], \
        (unsigned int)&OUT[_encTimes * AES_ONCE_ENC_DEC_MAX], _encRestLen);
    if(ret != 0)
        rt_kprintf("FH_CRYPTO_Encrypt B failed, ret = %d\n", ret);

    ret = FH_CRYPTO_DestroyHandle(&pCryptoHandle);
    if(ret != 0)
    {
        rt_kprintf("FH_CRYPTO_DestroyHandle failed, ret = %d\n", ret);
        return -8;
    }
    
    ret = FH_CRYPTO_DeInit();
    if(ret != 0)
    {
        rt_kprintf("FH_CRYPTO_DeInit failed, ret = %d\n", ret);
        return -9;
    }

    #if 0 /*xqy 2019-5-16*/
    rt_kprintf("AES Encode:\n");
    rt_kprintf("KEY:");
    for(i=0;i<16;i++)
    {
        //rt_kprintf("%02x ",otp0_key[i]);
    }
    rt_kprintf("\nIN :");
    for(i=0;i<16;i++)
    {
        rt_kprintf("%02x ",sIndata[i]);
    }
    rt_kprintf("\nOUT:");
    for(i=0;i<16;i++)
    {
        rt_kprintf("%02x ",sDataByAes[i]);
    }    
    #endif
    rt_kprintf("\nefuse aes enc done!\n\n");
    return 0;
}

int efuse_aes_dec(char *IN ,char *OUT, int len)
{
    int ret, i;
    CRYPTO_HANDLE pCryptoHandle;
    CRYPTO_CTRL_S cryptoCtrl;
    
    ret = FH_CRYPTO_Init();
    if(ret != 0)
    {
        rt_kprintf("FH_CRYPTO_Init failed, ret = %d\n", ret);
        return -2;
    }
    
    rt_memset(&pCryptoHandle,0,sizeof(pCryptoHandle));
    ret = FH_CRYPTO_CreateHandle(&pCryptoHandle); // 可不调用 
    if(ret != 0)
    {
        rt_kprintf("FH_CRYPTO_CreateHandle failed, ret = %d\n", ret);
        return -3;
    }
    rt_memset(&pCryptoHandle.request,0,sizeof(struct rt_crypto_request));
    
    rt_memset(&cryptoCtrl,0,sizeof(cryptoCtrl));
    cryptoCtrl.enKeySrc = CRYPTO_KEY_SRC_EFUSE; // aes key 来自efuse  efuse与aes模块有内部通道传递key 任何外界手段均无法探知
    cryptoCtrl.enAlg = CRYPTO_ALG_AES;
    cryptoCtrl.enWorkMode = CRYPTO_WORK_MODE_ECB;
    cryptoCtrl.enKeyLen = CRYPTO_KEY_AES_128BIT; // 16 bytes key
    cryptoCtrl.enIVLen = CRYPTO_IV_KEY_AES_0BIT;
    cryptoCtrl.enBitWidth = CRYPTO_BIT_WIDTH_128BIT;

    // 设置efuse所烧key 被aes硬件模块使用是是否作顺序map 具体见函数FH_CRYPTO_Mapswitch说明
    // 下述map 即直接将efuse的[0-15] 16个字节正序作为aes key
    unsigned int u32MapBuf[4]; 
    u32MapBuf[0] = 0; 
    u32MapBuf[1] = 8; 
    u32MapBuf[2] = 16; 
    u32MapBuf[3] = 24; 
    ret = FH_CRYPTO_Mapswitch(&pCryptoHandle, u32MapBuf, 4);  
    if(ret != 0)
    {
        rt_kprintf("FH_CRYPTO_Mapswitch failed, ret = %d\n", ret);
        return -4;
    }

    ret = FH_CRYPTO_ConfigHandle(&pCryptoHandle, &cryptoCtrl);
    if(ret != 0)
    {
        rt_kprintf("FH_CRYPTO_ConfigHandle failed, ret = %d\n", ret);
        return -5;
    }

    // aes 加密为对称加密 明文要求为16对齐  
    // 单次最大加密长度 AES_ONCE_ENC_DEC_MAX， 超过此长度需拆分多次
    int _decTimes = len / AES_ONCE_ENC_DEC_MAX;
    unsigned int _decRestLen = len % AES_ONCE_ENC_DEC_MAX;

    for(i = 0; i < _decTimes; i++)
    {
        ret = FH_CRYPTO_Decrypt(&pCryptoHandle, (unsigned int)&IN[i * AES_ONCE_ENC_DEC_MAX], \
            (unsigned int)&OUT[i * AES_ONCE_ENC_DEC_MAX], AES_ONCE_ENC_DEC_MAX);
        if(ret != 0)
            rt_kprintf("FH_CRYPTO_Decrypt failed, ret = %d\n", ret);
    }
    ret = FH_CRYPTO_Decrypt(&pCryptoHandle, (unsigned int)&IN[_decTimes * AES_ONCE_ENC_DEC_MAX], \
        (unsigned int)&OUT[_decTimes * AES_ONCE_ENC_DEC_MAX], _decRestLen);
    if(ret != 0)
        rt_kprintf("FH_CRYPTO_Decrypt B failed, ret = %d\n", ret);

    ret = FH_CRYPTO_DestroyHandle(&pCryptoHandle);
    if(ret != 0)
    {
        rt_kprintf("FH_CRYPTO_DestroyHandle failed, ret = %d\n", ret);
        return -8;
    }

    ret = FH_CRYPTO_DeInit();
    if(ret != 0)
    {
        rt_kprintf("FH_CRYPTO_DeInit failed, ret = %d\n", ret);
        return -9;
    }

    #if 0 /*xqy 2019-5-16*/
    rt_kprintf("AES Decode:\n");
    rt_kprintf("KEY:");
    for(i=0;i<16;i++)
    {
        //rt_kprintf("%02x ",otp0_key[i]);
    }
    rt_kprintf("\nIN :");
    for(i=0;i<16;i++)
    {
        rt_kprintf("%02x ",sMidBuf[i]);
    }
    rt_kprintf("\nOUT:");
    for(i=0;i<16;i++)
    {
        rt_kprintf("%02x ",sDataByDecode[i]);
    }    
    rt_kprintf("\nefuse aes dec done!\n\n");
    #endif

    return 0;
}

// 熔断(锁止读) 每颗芯片仅可熔断一次 不可逆
// 建议当产线烧入key后，并且完成明文加密烧flash 然后读flash密文解密，通过后 再熔断，然后出厂
int efuse_lock_key(void)
{
    int ret = 0;
    // 直接将efuse 0-2三个key区全部熔断  熔断后不再可读 即便尝试读 读的也是全0 
    ret = FH_CRYPTO_SetLock(0, 3, 0, 4); 
    rt_kprintf("\nefuse Lock Key(ret %d)\n\n",ret);
    return 0;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// efuse ID 区可存8个字节 每颗芯片可写一次 可随意读
int efuse_write_id(char *data)
{
    int ret;
    CRYPTO_HANDLE pCryptoHandle;

    ret = FH_CRYPTO_Init();
    if(ret != 0)
    {
        rt_kprintf("FH_CRYPTO_Init failed, ret = %d\n", ret);
    }
    
    rt_memset(&pCryptoHandle,0,sizeof(pCryptoHandle));
    ret = FH_CRYPTO_CreateHandle(&pCryptoHandle); // 可不调用
    if(ret != 0)
    {
        rt_kprintf("FH_CRYPTO_CreateHandle failed, ret = %d\n", ret);
        return -3;
    }
    ret = FH_CRYPTO_WriteOTPDeviceID(data, 8);
    if(ret != 0)
    {
        rt_kprintf("FH_CRYPTO_WriteOTPKey failed, ret = %d\n", ret);
    }

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

    rt_kprintf("efuse write ID done\n");

    return 0;
}
// efuse ID 区可存8个字节 每颗芯片可写一次 可随意读
int efuse_read_id(void)
{
    int ret;
    int i = 0;
    CRYPTO_HANDLE pCryptoHandle;
    rt_uint8_t result[8];

    ret = FH_CRYPTO_Init();
    if(ret != 0)
    {
        rt_kprintf("FH_CRYPTO_Init failed, ret = %d\n", ret);
    }
    
    rt_memset(&pCryptoHandle,0,sizeof(pCryptoHandle));
    ret = FH_CRYPTO_CreateHandle(&pCryptoHandle); // 可不调用
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

    rt_kprintf("device id: ");
    for(i = 0; i < 8; i++)
        rt_kprintf("0x%x, ", result[i]);
    rt_kprintf("\n");

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
    
    return 0;
}

#ifdef RT_USING_FINSH
#include <finsh.h>
//FINSH_FUNCTION_EXPORT(efuse_aes_enc, efuse_aes_enc);
//FINSH_FUNCTION_EXPORT(efuse_aes_dec, efuse_aes_dec);
FINSH_FUNCTION_EXPORT(efuse_read_id, efuse_read_id);
#endif


