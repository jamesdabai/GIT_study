
#ifndef __SAMPLE_COMMON_ISP_H__
#define __SAMPLE_COMMON_ISP_H__

#include "types/type_def.h"

int sample_isp_init(void);
FH_UINT32 sample_isp_change_fps(void);
void sample_isp_proc(void *arg);

#endif
