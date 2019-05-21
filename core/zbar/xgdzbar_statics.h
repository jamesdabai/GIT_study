#ifndef __XGDZBAR_STATICS_H__
#define __XGDZBAR_STATICS_H__


typedef struct
{
    unsigned int prev_tick; /* 上一次统计的时间tick数,用于计算fps */
    int frame_send_cnt_p;   /* 用于计算fps */
    int frame_scan_cnt_p;   /* 用于计算fps */
    
    int frame_send_cnt;     /* 发送的总帧数 */
    int frame_send_fps;     /* 发送的帧率(2秒更新) */
    int frame_scan_cnt;     /* 送去解码的帧数 */
    int frame_scan_fps;     /* 解码的帧率(2秒更新) */
    
    unsigned int frame_dec_tot_time;/* 解码总时间ms,    用于计算平均时间 */
    int frame_dec_max_time; /* 解码一张图片最大时间ms */
    int frame_dec_min_time; /* 解码一张图片最小时间ms */
    int frame_dec_ave_time; /* 解码一张图片平均时间ms */

    int audio_req_cnt;
    int audio_play_cnt;

    int scan_success_cnt;   /* 解码成功图片计数 */
    int string_send_cnt;    /* 发送的字符串次数 */
}xgdzbar_statics_t;

extern xgdzbar_statics_t xgdzbar_stat;

#endif


