#ifndef RTCONFIG_PLATFORM_H__
#define RTCONFIG_PLATFORM_H__

/* RT-Thread Components */

/* C++ features */

/* CONFIG_RT_USING_CPLUSPLUS is not set */

/* Command shell */

#define RT_USING_FINSH
#define FINSH_USING_SYMTAB
#define FINSH_USING_DESCRIPTION
#define FINSH_THREAD_STACK_SIZE 4096
/* CONFIG_FINSH_USING_AUTH is not set */
#define FINSH_DEFAULT_PASSWORD "rtthread"
#define FINSH_USING_MSH
#define FINSH_USING_MSH_DEFAULT
/* CONFIG_FINSH_USING_MSH_ONLY is not set */

/* Device virtual file system */

#define RT_USING_DFS
/* CONFIG_DFS_DEBUG is not set */
#define DFS_USING_WORKDIR
#define DFS_FILESYSTEMS_MAX 4
#define DFS_FD_MAX 16
/* CONFIG_RT_USING_DFS_RAMFS is not set */
/* #define RT_USING_DFS_RAMFS */
/* #define RT_USING_DFS_ELMFAT */
/* #define RT_DFS_ELM_CODE_PAGE_FILE */
/* #define RT_DFS_ELM_USE_EXFAT */
/* #define RT_DFS_ELM_WORD_ACCESS */
/* CONFIG_RT_DFS_ELM_USE_LFN_0 is not set */
/* CONFIG_RT_DFS_ELM_USE_LFN_1 is not set */
/* #define RT_DFS_ELM_USE_LFN_2 */
/* CONFIG_RT_DFS_ELM_USE_LFN_3 is not set */
/* #define RT_DFS_ELM_USE_LFN 2 */
/* #define RT_DFS_ELM_MAX_LFN 128 */
/* #define RT_DFS_ELM_DRIVES 2 */
/* #define RT_DFS_ELM_MAX_SECTOR_SIZE 512 */
/* CONFIG_RT_DFS_ELM_USE_ERASE is not set */
/* #define RT_DFS_ELM_REENTRANT */
#define RT_USING_DFS_DEVFS
#define RT_USING_DFS_NET
/* #define RT_USING_DFS_NFS */
/* CONFIG_RT_USING_DFS_YAFFS2 is not set */
#define RT_USING_DFS_JFFS2

/* Device Drivers */

/* CAN */

/* CONFIG_RT_USING_CAN is not set */

/* HW Timer */

/* CONFIG_RT_USING_HWTIMER is not set */

/* RTC */

#define RT_USING_RTC

/* USB */
#define RT_USING_USB_DEVICE
#define RT_USB_DEVICE_HID
#define RT_USB_DEVICE_HID_KEYBOARD
#define RT_HID_USING_TX_WAIT


#if 1/*xqy 2019-1-25*/
#define RT_USING_USB_DEVICE
#define RT_USB_DEVICE_CDC
#define RT_VCOM_TASK_STK_SIZE 1024
#define RT_VCOM_TX_USE_DMA
#define _VENDOR_ID 
#define _PRODUCT_ID 
#define   USB_VENDOR_ID 0x294A
#define   USB_HID_PRODUCT_ID 0x7305
#define   USB_CDC_PRODUCT_ID 0x3101

#endif



/* libc */

#define RT_USING_LIBC
#define RT_USING_PTHREADS
#define PTHREADS_DEFAULT_STACK_SIZE 8192
#define RT_USING_LIBDL

/* Network stack */

/* light weight TCP/IP stack */

#define RT_USING_LWIP
/* CONFIG_RT_USING_LWIP141 is not set */
/* CONFIG_RT_USING_LWIP200 is not set */
#define RT_USING_LWIP202
/* #define RT_USING_LWIPV6 */
#define LWIP_TIMERS 1
#define RT_LWIP_IP6ADDR0 0xfe800000
#define RT_LWIP_IP6ADDR1 0x0
#define RT_LWIP_IP6ADDR2 0x0
#define RT_LWIP_IP6ADDR3 0x20
#define RT_LWIP_IGMP
#define RT_LWIP_ICMP
/* CONFIG_RT_LWIP_SNMP is not set */
#define RT_LWIP_DNS
#define RT_LWIP_DHCP
#define IP_SOF_BROADCAST 1
#define IP_SOF_BROADCAST_RECV 1
#define LWIP_USING_DHCPD
#define RT_LWIP_UDP
#define RT_LWIP_TCP
#define RT_LWIP_RAW
/* CONFIG_RT_LWIP_PPP is not set */
/* CONFIG_RT_LWIP_PPPOE is not set */
/* CONFIG_RT_LWIP_PPPOS is not set */
#define RT_LWIP_PBUF_NUM 16
#define RT_LWIP_RAW_PCB_NUM 8
#define RT_LWIP_UDP_PCB_NUM 256
#define RT_LWIP_TCP_PCB_NUM 246
#define RT_LWIP_TCP_SEG_NUM 40
#define RT_LWIP_TCP_SND_BUF 10240
#define RT_LWIP_TCP_WND 8192
#define RT_LWIP_TCPTHREAD_PRIORITY 100
#define RT_LWIP_TCPTHREAD_MBOX_SIZE 32
#define RT_LWIP_TCPTHREAD_STACKSIZE 8192
#define RT_LWIP_ETHTHREAD_PRIORITY 126
#define RT_LWIP_ETHTHREAD_STACKSIZE 1024
#define RT_LWIP_ETHTHREAD_MBOX_SIZE 32
/* CONFIG_RT_LWIP_REASSEMBLY_FRAG is not set */
#define RT_LWIP_USING_RT_MEM
#define LWIP_NETIF_STATUS_CALLBACK 1
#define SO_REUSE 1
#define LWIP_SO_RCVTIMEO 1
#define LWIP_SO_SNDTIMEO 1
#define LWIP_SO_RCVBUF 1
#define RECV_BUFSIZE_DEFAULT 8192
#define RT_LWIP_IPADDR0 192
#define RT_LWIP_IPADDR1 168
#define RT_LWIP_IPADDR2 1
#define RT_LWIP_IPADDR3 30
#define RT_LWIP_GWADDR0 192
#define RT_LWIP_GWADDR1 168
#define RT_LWIP_GWADDR2 1
#define RT_LWIP_GWADDR3 1
#define RT_LWIP_MSKADDR0 255
#define RT_LWIP_MSKADDR1 255
#define RT_LWIP_MSKADDR2 255
#define RT_LWIP_MSKADDR3 0

/* BSP Configuration */

#define RT_USING_INTERRUPT_INFO
#define INIT_DRV_THREAD_STACK_SIZE 0x8000

/* Chip and Board */

#define CONFIG_CHIP_FH8632
/* CONFIG_CONFIG_CHIP_FH8833 is not set */
/* CONFIG_CONFIG_CHIP_FH8833T is not set */
/* CONFIG_CONFIG_CHIP_FH8830 is not set */
/* CONFIG_CONFIG_CHIP_FH8630 is not set */
/* CONFIG_CONFIG_CHIP_FH8812 is not set */
/* CONFIG_CONFIG_CHIP_FH8810 is not set */
/* CONFIG_CONFIG_CHIP_FH8620 is not set */
/* CONFIG_CONFIG_CHIP_FH8622 is not set */
/* CONFIG_CONFIG_BOARD_APP is not set */
/* CONFIG_CONFIG_BOARD_FPGA is not set */
#define CONFIG_BOARD_TEST

/* WiFi configuration */

/* CONFIG_RT_USING_WIFI is not set */

/* Platform Components */

/* Network Components */

#define RT_USING_COMPONENTS_NET

/* Tftp Components */

#define RT_USING_TFTP

/* Bonjour Components */

/* CONFIG_RT_USING_COMPONENTS_BONJOUR is not set */

/* VBUS Components */

#define RT_USING_VBUS
#define RT_VBUS_CONFIG_CLIENT
#define RT_USING_SOFTCORE
#define FH_CONFIG_VBUS_STACKSIZE 4096

/* MBEDTLS components */

/* CONFIG_RT_USING_MBEDTLS is not set */

/* Profiling Components */

#define RT_USING_COMPONENTS_PROFILING
/* CONFIG_FH_USING_DDR_STATUS is not set */
#define RT_USING_YMODEM
#define RT_USING_ZMODEM
#endif
