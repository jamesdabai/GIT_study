#include <rtdef.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <dfs_posix.h>
#include "pwm_lb2.h"

static struct rt_device *pwmDevice = RT_NULL;
static int pwm_fd, int_all_count = 0;

void pwm0_finish_once(struct fh_pwm_chip_data *data)
{
    int_all_count++;
    if (int_all_count < 20)
        rt_kprintf("pwm%d_finish_once\r\n", data->id);
}

void pwm0_finish_all(struct fh_pwm_chip_data *data)
{
    //rt_kprintf("pwm%d_finish_all\r\n", data->id);
}


int beep_func(int hz, int time_ms)
{
    int num,period;
    int ret = 0;
    int i = 0;

    struct fh_pwm_chip_data pwm[1] = { { 0 } };

    if( (hz < 100) || (hz > 20000) )
    {
        hz = 2000;
    }

    if(time_ms < 10)
    {
        time_ms  = 10;
    }

    period = 1000000000/hz;
    num = time_ms*1000*1000/period;
    //rt_kprintf("hz=%d,time_ms=%d,period=%dns,num=%d\n",hz,time_ms,period,num);

    pwm[0].id = i;
    pwm[0].config.period_ns = period; // 2KHz
    pwm[0].config.percent = 50;
    pwm[0].config.delay_ns = 0;
    pwm[0].config.phase_ns = 0;
    pwm[0].config.pulses = FH_PWM_PULSE_LIMIT;
    pwm[0].config.pulse_num = num;
    pwm[0].config.finish_all = 1;
    pwm[0].config.finish_once = 1;
    pwm[0].finishall_callback = pwm0_finish_all;
    pwm[0].finishonce_callback = pwm0_finish_once;

    rt_device_control(pwmDevice, SET_PWM_DUTY_CYCLE_PERCENT, &pwm[0]);
    rt_device_control(pwmDevice, ENABLE_PWM, &pwm[0]);

    //rt_thread_delay(500);
    //rt_device_control(pwmDevice, GET_PWM_CONFIG, &pwm[0]);
    return ret;
}


int pwm_func(int percent)
{
    int ret = 0;
    int i = 0;

    struct fh_pwm_chip_data pwm[1] = { { 0 } };

    pwm[0].id = i;
    pwm[0].config.period_ns = 1000000000/2000; // 2KHz
    pwm[0].config.percent = percent;
    pwm[0].config.delay_ns = 0;
    pwm[0].config.phase_ns = 0;
    pwm[0].config.pulses = FH_PWM_PULSE_NOLIMIT;
    pwm[0].config.pulse_num = 10;
    pwm[0].config.finish_all = 1;
    pwm[0].config.finish_once = 1;
    pwm[0].finishall_callback = pwm0_finish_all;
    pwm[0].finishonce_callback = pwm0_finish_once;

    rt_device_control(pwmDevice, SET_PWM_DUTY_CYCLE_PERCENT, &pwm[0]);
    rt_device_control(pwmDevice, ENABLE_PWM, &pwm[0]);

    rt_thread_delay(500);
    rt_device_control(pwmDevice, GET_PWM_CONFIG, &pwm[0]);

    rt_kprintf("set duty ratio to %d%%\n", percent);
    return ret;
}

void pwm_demo_main(void *para)
{
    rt_kprintf("pwm demo start:\n");
    pwmDevice = rt_device_find("pwm");

    pwm_fd = rt_device_open(pwmDevice, O_RDWR);
    if (pwm_fd == -1)
    {
        rt_kprintf("rt_device_open pwm failed\n");

        return;
    }
    //pwm_func(50);

    return;
}

int pwm_demo_init(void)
{

    rt_thread_t threadPwm;

    threadPwm = rt_thread_create("pwm", pwm_demo_main, RT_NULL, 10 * 1024, 8,
            20);

    if (threadPwm != RT_NULL)
        rt_thread_startup(threadPwm);
    return 0;
}

int beep(int argc,char *argv[])
{
    int hz=2000,time=200;
    
    if(argc > 1)
    {
        hz = atoi(argv[1]);
    }

    if(argc > 2)
    {
        time = atoi(argv[2]);
    }

    beep_func(hz,time);
    
    return 0;
}

#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT(pwm_func, pwm_func(int percent));
MSH_CMD_EXPORT(beep, beep sound);

#endif
