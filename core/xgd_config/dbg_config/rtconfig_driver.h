#ifndef RTCONFIG_DRIVER_H__
#define RTCONFIG_DRIVER_H__

/* IPC */

#define RT_USING_DEVICE_IPC

/* Serial */

#define RT_USING_SERIAL

/* I2C */

#define RT_USING_I2C
#define RT_USING_I2C_BITOPS

/* GPIO */

#define RT_USING_PIN

/* MTD */

#define RT_USING_MTD_NOR
/* CONFIG_RT_USING_MTD_NAND is not set */
#define RT_USING_FH_FLASH_ADAPT

/* SDIO */

/* #define RT_USING_SDIO */

/* SPI */

#define RT_USING_SPI
/* CONFIG_RT_USING_SFUD is not set */
#define RT_USING_W25QXX
#define RT_USING_GD
/* CONFIG_RT_USING_ENC28J60 is not set */
/* CONFIG_RT_USING_SPI_WIFI is not set */

/* Watch Dog */

#define RT_USING_WDT

/* Platform Driver configuration */

#define RT_USING_MTD
#define RT_USING_UART0
/* CONFIG_RT_USING_UART1 is not set */
#define RT_USING_GPIO
#define RT_USING_FH_ACW
#define RT_USING_PWM
#define RT_USING_FH_DMA
#define RT_USING_SADC
#define RT_USING_FH_CLOCK
#define RT_USING_GMAC
#define RT_USING_AES
#define RT_USING_EFUSE_MAP
#define RT_USING_ALGORITHM
#endif
