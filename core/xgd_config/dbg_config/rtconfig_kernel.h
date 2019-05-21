#ifndef RTCONFIG_KERNEL_H__
#define RTCONFIG_KERNEL_H__

#define RT_NAME_MAX 16
#define RT_ALIGN_SIZE 4
#define RT_THREAD_PRIORITY_MAX 256
#define RT_TICK_PER_SECOND 100
/* CONFIG_RT_DEBUG is not set */
#define RT_USING_HOOK
#define IDLE_THREAD_STACK_SIZE 1024
#define RT_USING_TIMER_SOFT
#define RT_TIMER_THREAD_PRIO 8
#define RT_TIMER_THREAD_STACK_SIZE 1536

/* Inter-Thread communication */

#define RT_USING_SEMAPHORE
#define RT_USING_MUTEX
#define RT_USING_EVENT
#define RT_USING_MAILBOX
#define RT_USING_MESSAGEQUEUE

/* Memory Management */

#define RT_USING_MEMPOOL
#define RT_USING_MEMHEAP
#define RT_USING_HEAP
/* CONFIG_RT_USING_SMALL_MEM is not set */
#define RT_USING_SLAB
#define RT_USING_MM_TRACE

/* Kernel Device Object */

#define RT_USING_DEVICE
#define RT_USING_CONSOLE
#define RT_CONSOLEBUF_SIZE 128
#define RT_CONSOLE_DEVICE_NAME "uart0"
#define RT_USING_MODULE
#define XGD_ENABLE_PRINTF 


/* Timekeeping Components */

#define RT_USING_TIMEKEEPING

/* Boot stages */

#define FH_BOOT_IN_2STAGE
#define FH_COMPRESS_STAGE2CODE
#endif
