
#include <rtdef.h>
#include <rtthread.h>


extern int isp_read_proc();
extern int enc_read_proc();


int isp_proc(int argc,char *argv[])
{
    isp_read_proc();

    return 0;
}

int enc_proc(int argc,char *argv[])
{
    enc_read_proc();

    return 0;
}


#ifdef RT_USING_FINSH
#include <finsh.h>
MSH_CMD_EXPORT(isp_proc, isp_proc);
MSH_CMD_EXPORT(enc_proc, enc_proc);
#endif


