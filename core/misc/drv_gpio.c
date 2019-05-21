#include <rtdef.h>
#include <rtthread.h>
#include "gpio.h"
#include "iomux.h"

#define GPIO_VOL_EN     0
#define GPIO_BK_LIGHT   1
#define GPIO_SCAN_OK    2
/* 
    SD0_DATA1(GPIO61) - VOL-EN(Speeker open) 1-SPK Enable 0-SPK Disable
    SD0_DATA0(GPIO62) - LIGHT_EN(backlight on) 1-on 0-off
    SD0_CLK(GPIO63)   - LED_EN(SCAN OK) 1-on 0-off
*/
static const char gpio_num[3] = {2,0,1};//{61,62,63};
static const char gpio_init_value[3] = {0,1,0};

void gpio_ctrl_init(void)
{
    int i,status;

    for(i=0;i<sizeof(gpio_num)/sizeof(gpio_num[0]);i++)
    {
        fh_select_gpio(gpio_num[i]);
        status = gpio_request(gpio_num[i]);
        if (status < 0)
        {
            rt_kprintf("ERROR can not open GPIO %d\n", gpio_num[i]);
        }

        gpio_direction_output(gpio_num[i], gpio_init_value[i]);
    }
}

void backlight_led_ctrl(int ctrl)
{
    gpio_set_value(gpio_num[GPIO_BK_LIGHT],ctrl);
}

void scan_ok_led_ctrl(int ctrl)
{
    if(xgd_scanner_status())
    {
        gpio_set_value(gpio_num[GPIO_BK_LIGHT],!ctrl);
    }
    gpio_set_value(gpio_num[GPIO_SCAN_OK],ctrl);
    
}

void speaker_sound_enable(int ctrl)
{
    gpio_set_value(gpio_num[GPIO_VOL_EN],ctrl);
}


int gpio_ctrl(int argc,char *argv[])
{
    int gpio,value;
    if(argc < 3)
    {
        rt_kprintf("err");
    }

    gpio = atoi(argv[1]);
    value = atoi(argv[2]);

    gpio_set_value(gpio,value);

    return 0;
}

#ifdef RT_USING_FINSH
#include <finsh.h>
MSH_CMD_EXPORT(gpio_ctrl, gpio_ctrl);
#endif
