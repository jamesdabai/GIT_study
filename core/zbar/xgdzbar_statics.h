#ifndef __XGDZBAR_STATICS_H__
#define __XGDZBAR_STATICS_H__


typedef struct
{
    unsigned int prev_tick; /* ��һ��ͳ�Ƶ�ʱ��tick��,���ڼ���fps */
    int frame_send_cnt_p;   /* ���ڼ���fps */
    int frame_scan_cnt_p;   /* ���ڼ���fps */
    
    int frame_send_cnt;     /* ���͵���֡�� */
    int frame_send_fps;     /* ���͵�֡��(2�����) */
    int frame_scan_cnt;     /* ��ȥ�����֡�� */
    int frame_scan_fps;     /* �����֡��(2�����) */
    
    unsigned int frame_dec_tot_time;/* ������ʱ��ms,    ���ڼ���ƽ��ʱ�� */
    int frame_dec_max_time; /* ����һ��ͼƬ���ʱ��ms */
    int frame_dec_min_time; /* ����һ��ͼƬ��Сʱ��ms */
    int frame_dec_ave_time; /* ����һ��ͼƬƽ��ʱ��ms */

    int audio_req_cnt;
    int audio_play_cnt;

    int scan_success_cnt;   /* ����ɹ�ͼƬ���� */
    int string_send_cnt;    /* ���͵��ַ������� */
}xgdzbar_statics_t;

extern xgdzbar_statics_t xgdzbar_stat;

#endif


