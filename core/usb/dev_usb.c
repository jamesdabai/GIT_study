/*
	功能描述：usb cdc/hid功能
	接口规范：按照DDI文档USB章节描述实现
*/
#include <rtthread.h>
#include <rtdevice.h>
#include "common.h"
#include "dev_usb.h"

#define USB_DEBUG
#undef USB_DEBUG

#ifdef USB_DEBUG
#define usb_debug rt_kprintf
#else
#define usb_debug(fmt,...)
#endif

static u8 const usb_version[18]={"V9A00000190121003"};
static int g_usb_fd = -1;
 rt_device_t usb_device;


static unsigned char key2usageid_table_single[] = 
{
	'\r',0x28,
	' ',0x2C
};
static unsigned char key2usageid_table_shift[] = 
{
	'!',0x1E,
	'@',0x1F,
	'#',0x20,
	'$',0x21,
	'%',0x22,
	'^',0x23,
	'&',0x24,
	'*',0x25,
	'(',0x26,
	')',0x27
};
static unsigned char  key2usageid_table_mix[] = 
{
	'-','_',0x2D,
	'=','+',0x2E,
	'[','{',0x2F,
	']','}',0x30,
	'\\','|',0x31,
	';',':',0x33,
	'\'','\"',0x34,
	'`','~',0x35,
	',','<',0x36,
	'.','>',0x37,
	'/','?',0x38
};

//return value:
//0:key nod found
//1:key found,without shift
//2:key found,with shift
static int hid_key2usageid(unsigned char keyname,unsigned char *usageid)
{
	int ret=0;
	unsigned int n;
	
	if(keyname == '0')
	{
		*usageid = 0x27;
		ret = 1;
	}
	else if(keyname >= '1' && keyname <= '9')
	{
		*usageid = 0x1e + keyname -'1';
		ret = 1;
	}
	else if(keyname >= 'a' && keyname <= 'z')
	{
		*usageid = 0x04 + keyname -'a';
		ret = 1;
	}
	else if(keyname >= 'A' && keyname <= 'Z')
	{
		*usageid = 0x04 + keyname -'A';
		ret = 2;
	}
	else
	{
		for(n=0;n<sizeof(key2usageid_table_single);n+=2)
		{
			if(key2usageid_table_single[n] == keyname)
			{
				*usageid = key2usageid_table_single[n+1];
				ret = 1;
				break;
			}
		}
		if(ret == 0)
		{
			for(n=0;n<sizeof(key2usageid_table_shift);n+=2)
			{
				if(key2usageid_table_shift[n] == keyname)
				{
					*usageid = key2usageid_table_shift[n+1];
					ret = 2;
					break;
				}
			}
			if(ret == 0)
			{
				for(n=0;n<sizeof(key2usageid_table_mix);n+=3)
				{
					if(key2usageid_table_mix[n] == keyname)
					{
						*usageid = key2usageid_table_mix[n+2];
						ret = 1;
						break;
					}
					else if(key2usageid_table_mix[n+1] == keyname)
					{
						*usageid = key2usageid_table_mix[n+2];
						ret = 2;
						break;
					}
				}
			}
		}
	}
	return ret;
}

#ifdef RT_HID_USING_TX_WAIT
static struct rt_completion sTxHidComp;
static rt_err_t _hid_send_complete(rt_device_t dev, void *buffer)
{
    rt_completion_done(&sTxHidComp);
    return 0;
}
#endif
static int _hid_send_report(unsigned char *buf,unsigned char len)
{
	int i;
	
	if(len %8 != 0) return -2;
	
	for(i=0; i < len ;i += 8)
	{	

#ifdef RT_HID_USING_TX_WAIT
		int ret;
		rt_completion_init(&sTxHidComp);
		ret = rt_device_write(usb_device, buf[i], &buf[i+1], 8-1);
		rt_completion_wait(&sTxHidComp, RT_WAITING_FOREVER);
#else
		rt_device_write(usb_device, buf[i], &buf[i+1], 8*1);
#endif
	}

	return i;
}

static int _hid_device_write(unsigned char *buff)
{
	int ret= 0,shift;
	unsigned int len,n;
	unsigned char report[8],usageid;
	unsigned char array_idx=0;
	unsigned char i;
	unsigned char array_same_letter = 0;
	
	if(buff==NULL)
		return -6;
		
	len = strlen(buff);
	memset(report,0,sizeof(report));

	for(n=0; n < len; n++)
	{
		shift = hid_key2usageid(buff[n],&usageid);
		if(shift == 0 )
		{
			if(buff[n] == '\n')
			{			
				continue;
			}
			else
			{
				ret = -7;
				break;
			}
		}
		else
		{
			if(shift == 2)
			{
				report[0] = 0x02;
				if(8 != _hid_send_report(report,8))
				{
					ret = -7;
					break;
				}
			}
			report[2] = usageid;
			if(8 != _hid_send_report(report,8))
			{
				ret = -7;
				break;
			}
			report[0] = 0;
			report[2] = 0;
			if(8 != _hid_send_report(report,8))
			{
				ret = -7;
				break;
			}
		}
	}
	return n==0?ret:n;
}


s32 dev_usb_open(u32 nClass)
{

	s32 cRet = DDI_OK;
	
	if(nClass != CDC_DEVICE && nClass != HID_DEVICE)
		return DDI_EINVAL;
	
	if(g_usb_fd < 0)
	{
		if(nClass == CDC_DEVICE)
		{
			usb_device = rt_device_find("vcom");//查找设备
			RT_ASSERT(usb_device != RT_NULL);
			rt_device_init(usb_device);
			#ifndef RT_VCOM_TX_USE_DMA
			cRet =rt_device_open(usb_device,
				RT_DEVICE_FLAG_RDWR);
			#else
			cRet =rt_device_open(usb_device,
				RT_DEVICE_FLAG_RDWR|RT_DEVICE_FLAG_DMA_TX);
			#endif
		}
		else if( nClass == HID_DEVICE )
		{
		    usb_device = rt_device_find("hidd");
		    usb_debug("hid_device :%x\n", usb_device);
		    RT_ASSERT(usb_device != RT_NULL);
#ifdef RT_HID_USING_TX_WAIT
		    usb_device->tx_complete = _hid_send_complete;
#endif
		    cRet = rt_device_open(usb_device, 0);
		    usb_debug("rt_thread_open re_return = %d\n");			
		}
		
		if(cRet < 0)
			cRet = DDI_ERR;
		else
			g_usb_fd = nClass;
	}
	
	return cRet;

}

s32 dev_usb_close(u32 nClass)
{
	s32 cRet = DDI_OK;

	if(nClass != CDC_DEVICE && nClass != HID_DEVICE)
		return DDI_EINVAL;

	if(g_usb_fd >= 0)
	{
		rt_device_close(usb_device);
		g_usb_fd = -1;
	}
	return DDI_OK;

}

s32 dev_usb_read(u32 nClass ,u8 *lpOut ,u32 nLe)
{
	s32 cRet=DDI_OK;	

	if(g_usb_fd < 0)
		return DDI_EIO;
	
	if(nClass != CDC_DEVICE && nClass != HID_DEVICE)
		return DDI_EINVAL;


	if(lpOut==NULL)
		return DDI_EINVAL;

	if(nLe == 0 || nLe > 512)
	{
		return DDI_EINVAL;
	}	


	if(nClass == CDC_DEVICE)
	{
		cRet = rt_device_read(usb_device,0,lpOut,nLe);
	}
	else if(nClass == HID_DEVICE )  //hid has no read function now
	{
		cRet = DDI_OK;
	}
	
	return cRet;

}

s32 dev_usb_write(u32 nClass ,u8 *lpIn ,u32 nLe)
{

	s32 cRet=DDI_OK;	

	if(g_usb_fd < 0)
		return DDI_EIO;
	
	if(nClass != CDC_DEVICE && nClass != HID_DEVICE)
		return DDI_EINVAL;


	if(lpIn==NULL)
		return DDI_EINVAL;

	if(nLe == 0 || nLe > 512)
	{
		return DDI_EINVAL;
	}	

	if(nClass == CDC_DEVICE)
	{
		cRet = rt_device_write(usb_device,0,lpIn,nLe);
	}
	else if(nClass == HID_DEVICE)
	{
		cRet = _hid_device_write(lpIn);
	}
	
	return cRet;

}


s32 dev_usb_ioctl(u32 nCmd ,u32 lParam ,u32 wParam)
{

	s32 cRet=DDI_OK;

	if(g_usb_fd < 0)
		return DDI_EIO;

	switch(nCmd)
	{
		case DDI_USB_CTL_VER:
			strcpy((char *)wParam,(char const*)usb_version);
			break;
		case DDI_USB_CTL_CLEAR:
			break;
		case DDI_USB_CTL_STATUS:
			if(g_usb_fd == CDC_DEVICE)
			{			
				if(RT_TRUE == usb_vcom_status(usb_device))
					*(u32 *)wParam = 0;
				else
					*(u32 *)wParam = 1;
			}	
			else if(g_usb_fd == HID_DEVICE )
			{	
				if(RT_TRUE == usb_hid_status(usb_device))
					*(u32 *)wParam = 0;
				else
					*(u32 *)wParam = 1;				
			}
			break;	
		default:
			return DDI_EINVAL;
			break;
	}	

	return cRet;

}

s32 dbg_print_usb(char *fmt,...)
{
	va_list ap;
	u8 *bp;
	u32 len;
	
	static u8 usb_open_flag = 0;
	u32 usb_status;
	s32 ret = -1;
	static u8 print_buf_usb[512] ;	
	
	bp= print_buf_usb;
	*bp= 0;	
	va_start (ap, fmt);
    vsprintf(bp, fmt, ap);
	len = strlen(bp);
	
	if(0 == usb_open_flag)
	{
		usb_open_flag = 1;
		dev_usb_open(CDC_DEVICE);		
	}

	dev_usb_ioctl(DDI_USB_CTL_STATUS ,CDC_DEVICE ,(u32)&usb_status);
	if(0 == usb_status)
	{
		ret = dev_usb_write(CDC_DEVICE ,bp ,len);		
	} 		

	va_end (ap);
	return ret;	
}


