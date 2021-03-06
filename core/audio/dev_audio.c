#include <stdio.h>
#include <rtdef.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <dfs_posix.h>
#include "dsp/fh_audio_mpi.h"
#include "dev_audio.h"
#include "common.h"
#include "xgd_config_deal.h"

#define ONE_FRAME_SIZE 1024      

#define DDI_AUDIO_CTL_VER 			0 	//获取音频设备版本
#define DDI_AUDIO_CTL_VOLUME 		1 	//控制音量
#define DDI_AUDIO_CTL_BUZZER 		2 	//控制蜂鸣器发3KHz的按键音
#define DDI_AUDIO_CTL_COMB_PLAY 	3 	//组合播放
#define DDI_AUDIO_CTL_GET_STATUS 	4 	//获取播放状态
#define DDI_AUDIO_CTL_BEEP_TUNING  5	//控制蜂鸣器声音大小和频率

unsigned char acok[] = {
0x52, 0x49, 0x46, 0x46, 0xE2, 0x07, 0x00, 0x00, 0x57, 0x41, 0x56, 0x45, 0x66, 0x6D, 0x74, 0x20,
0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x80, 0x3E, 0x00, 0x00, 0x00, 0x7D, 0x00, 0x00,
0x02, 0x00, 0x10, 0x00, 0x64, 0x61, 0x74, 0x61, 0xBE, 0x07, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x01, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF,
0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0xFE, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00,
0x01, 0x00, 0xFE, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 0xFF, 0x00, 0x00, 0xFF, 0xFF,
0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF,
0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFE, 0xFF, 0x01, 0x00, 0xFF, 0xFF,
0x00, 0x00, 0xFE, 0xFF, 0x00, 0x00, 0xFE, 0xFF, 0x01, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0xFE, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x01, 0x00, 0xFE, 0xFF, 0x00, 0x00, 0xFF, 0xFF,
0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0xFF, 0xFF,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x01, 0x00, 0xFE, 0xFF, 0x00, 0x00,
0xFE, 0xFF, 0x02, 0x00, 0xFD, 0xFF, 0x03, 0x00, 0xFB, 0xFF, 0x05, 0x00, 0xF8, 0xFF, 0x0A, 0x00,
0xF3, 0xFF, 0x12, 0x00, 0xE4, 0xFF, 0x28, 0x00, 0xB9, 0xFF, 0x8C, 0x00, 0x82, 0xFE, 0x5A, 0x0A,
0x39, 0x52, 0x9F, 0x40, 0x0B, 0xDD, 0x8C, 0xA5, 0x37, 0xDD, 0x24, 0x40, 0xAA, 0x53, 0x09, 0x00,
0x41, 0xAC, 0xF3, 0xBF, 0xA8, 0x22, 0x9F, 0x5A, 0xAA, 0x22, 0xEE, 0xBF, 0x46, 0xAC, 0x01, 0x00,
0xB5, 0x53, 0x14, 0x40, 0x52, 0xDD, 0x62, 0xA5, 0x52, 0xDD, 0x12, 0x40, 0xB6, 0x53, 0xFF, 0xFF,
0x48, 0xAC, 0xED, 0xBF, 0xAD, 0x22, 0x9D, 0x5A, 0xAC, 0x22, 0xEC, 0xBF, 0x48, 0xAC, 0xFF, 0xFF,
0xB8, 0x53, 0x12, 0x40, 0x52, 0xDD, 0x62, 0xA5, 0x54, 0xDD, 0x12, 0x40, 0xB8, 0x53, 0xFE, 0xFF,
0x48, 0xAC, 0xEC, 0xBF, 0xAE, 0x22, 0x9D, 0x5A, 0xAE, 0x22, 0xEC, 0xBF, 0x4A, 0xAC, 0xFE, 0xFF,
0xB8, 0x53, 0x11, 0x40, 0x53, 0xDD, 0x62, 0xA5, 0x53, 0xDD, 0x11, 0x40, 0xB6, 0x53, 0xFF, 0xFF,
0x49, 0xAC, 0xEC, 0xBF, 0xAD, 0x22, 0x9D, 0x5A, 0xAD, 0x22, 0xED, 0xBF, 0x48, 0xAC, 0xFD, 0xFF,
0xB8, 0x53, 0x12, 0x40, 0x53, 0xDD, 0x62, 0xA5, 0x53, 0xDD, 0x12, 0x40, 0xB7, 0x53, 0x00, 0x00,
0x48, 0xAC, 0xEC, 0xBF, 0xAC, 0x22, 0x9C, 0x5A, 0xAC, 0x22, 0xEC, 0xBF, 0x48, 0xAC, 0xFF, 0xFF,
0xB7, 0x53, 0x12, 0x40, 0x53, 0xDD, 0x63, 0xA5, 0x52, 0xDD, 0x12, 0x40, 0xB7, 0x53, 0xFE, 0xFF,
0x48, 0xAC, 0xEC, 0xBF, 0xAE, 0x22, 0x9D, 0x5A, 0xAE, 0x22, 0xED, 0xBF, 0x48, 0xAC, 0xFF, 0xFF,
0xB6, 0x53, 0x12, 0x40, 0x52, 0xDD, 0x62, 0xA5, 0x54, 0xDD, 0x12, 0x40, 0xB8, 0x53, 0x00, 0x00,
0x48, 0xAC, 0xED, 0xBF, 0xAD, 0x22, 0x9C, 0x5A, 0xAC, 0x22, 0xED, 0xBF, 0x48, 0xAC, 0x00, 0x00,
0xB8, 0x53, 0x13, 0x40, 0x53, 0xDD, 0x62, 0xA5, 0x52, 0xDD, 0x13, 0x40, 0xB7, 0x53, 0x00, 0x00,
0x47, 0xAC, 0xEC, 0xBF, 0xAC, 0x22, 0x9D, 0x5A, 0xAD, 0x22, 0xED, 0xBF, 0x49, 0xAC, 0x00, 0x00,
0xB8, 0x53, 0x13, 0x40, 0x53, 0xDD, 0x62, 0xA5, 0x53, 0xDD, 0x13, 0x40, 0xB7, 0x53, 0x00, 0x00,
0x48, 0xAC, 0xED, 0xBF, 0xAC, 0x22, 0x9D, 0x5A, 0xAC, 0x22, 0xED, 0xBF, 0x48, 0xAC, 0x00, 0x00,
0xB7, 0x53, 0x13, 0x40, 0x52, 0xDD, 0x63, 0xA5, 0x52, 0xDD, 0x14, 0x40, 0xB7, 0x53, 0xFE, 0xFF,
0x48, 0xAC, 0xED, 0xBF, 0xAD, 0x22, 0x9D, 0x5A, 0xAD, 0x22, 0xEC, 0xBF, 0x49, 0xAC, 0xFF, 0xFF,
0xB6, 0x53, 0x13, 0x40, 0x52, 0xDD, 0x62, 0xA5, 0x53, 0xDD, 0x12, 0x40, 0xB7, 0x53, 0x00, 0x00,
0x47, 0xAC, 0xEC, 0xBF, 0xAD, 0x22, 0x9D, 0x5A, 0xAC, 0x22, 0xEC, 0xBF, 0x48, 0xAC, 0x01, 0x00,
0xB8, 0x53, 0x13, 0x40, 0x52, 0xDD, 0x62, 0xA5, 0x53, 0xDD, 0x12, 0x40, 0xB7, 0x53, 0xFF, 0xFF,
0x47, 0xAC, 0xEC, 0xBF, 0xAD, 0x22, 0x9E, 0x5A, 0xAC, 0x22, 0xED, 0xBF, 0x48, 0xAC, 0x00, 0x00,
0xB8, 0x53, 0x12, 0x40, 0x52, 0xDD, 0x62, 0xA5, 0x52, 0xDD, 0x13, 0x40, 0xB6, 0x53, 0xFF, 0xFF,
0x48, 0xAC, 0xEC, 0xBF, 0xAC, 0x22, 0x9D, 0x5A, 0xAC, 0x22, 0xEC, 0xBF, 0x47, 0xAC, 0x01, 0x00,
0xB6, 0x53, 0x13, 0x40, 0x52, 0xDD, 0x62, 0xA5, 0x52, 0xDD, 0x12, 0x40, 0xB6, 0x53, 0x00, 0x00,
0x48, 0xAC, 0xEC, 0xBF, 0xAE, 0x22, 0x9C, 0x5A, 0xAD, 0x22, 0xEC, 0xBF, 0x49, 0xAC, 0x00, 0x00,
0xB8, 0x53, 0x13, 0x40, 0x53, 0xDD, 0x62, 0xA5, 0x53, 0xDD, 0x13, 0x40, 0xB7, 0x53, 0xFF, 0xFF,
0x48, 0xAC, 0xEC, 0xBF, 0xAE, 0x22, 0x9E, 0x5A, 0xAD, 0x22, 0xED, 0xBF, 0x48, 0xAC, 0xFF, 0xFF,
0xB6, 0x53, 0x12, 0x40, 0x52, 0xDD, 0x62, 0xA5, 0x53, 0xDD, 0x12, 0x40, 0xB7, 0x53, 0x00, 0x00,
0x48, 0xAC, 0xEC, 0xBF, 0xAE, 0x22, 0x9D, 0x5A, 0xAD, 0x22, 0xEC, 0xBF, 0x48, 0xAC, 0xFF, 0xFF,
0xB6, 0x53, 0x13, 0x40, 0x52, 0xDD, 0x62, 0xA5, 0x52, 0xDD, 0x13, 0x40, 0xB6, 0x53, 0xFF, 0xFF,
0x47, 0xAC, 0xEC, 0xBF, 0xAE, 0x22, 0x9E, 0x5A, 0xAB, 0x22, 0xEB, 0xBF, 0x47, 0xAC, 0x00, 0x00,
0xB7, 0x53, 0x13, 0x40, 0x51, 0xDD, 0x62, 0xA5, 0x52, 0xDD, 0x14, 0x40, 0xB6, 0x53, 0x00, 0x00,
0x49, 0xAC, 0xED, 0xBF, 0xAD, 0x22, 0x9C, 0x5A, 0xAD, 0x22, 0xEC, 0xBF, 0x48, 0xAC, 0x00, 0x00,
0xB7, 0x53, 0x13, 0x40, 0x53, 0xDD, 0x62, 0xA5, 0x52, 0xDD, 0x12, 0x40, 0xB6, 0x53, 0xFF, 0xFF,
0x49, 0xAC, 0xEC, 0xBF, 0xAC, 0x22, 0x9B, 0x5A, 0xAD, 0x22, 0xED, 0xBF, 0x48, 0xAC, 0xFF, 0xFF,
0xB7, 0x53, 0x12, 0x40, 0x52, 0xDD, 0x62, 0xA5, 0x52, 0xDD, 0x12, 0x40, 0xB8, 0x53, 0x00, 0x00,
0x48, 0xAC, 0xEC, 0xBF, 0xAC, 0x22, 0x9E, 0x5A, 0xAD, 0x22, 0xEC, 0xBF, 0x48, 0xAC, 0xFF, 0xFF,
0xB6, 0x53, 0x12, 0x40, 0x52, 0xDD, 0x62, 0xA5, 0x52, 0xDD, 0x11, 0x40, 0xB5, 0x53, 0x01, 0x00,
0x47, 0xAC, 0xED, 0xBF, 0xAC, 0x22, 0x9D, 0x5A, 0xAC, 0x22, 0xEE, 0xBF, 0x48, 0xAC, 0x00, 0x00,
0xB6, 0x53, 0x12, 0x40, 0x53, 0xDD, 0x63, 0xA5, 0x53, 0xDD, 0x13, 0x40, 0xB8, 0x53, 0xFF, 0xFF,
0x49, 0xAC, 0xEC, 0xBF, 0xAD, 0x22, 0x9E, 0x5A, 0xAC, 0x22, 0xED, 0xBF, 0x48, 0xAC, 0x01, 0x00,
0xB8, 0x53, 0x11, 0x40, 0x52, 0xDD, 0x61, 0xA5, 0x52, 0xDD, 0x13, 0x40, 0xB6, 0x53, 0xFF, 0xFF,
0x47, 0xAC, 0xEC, 0xBF, 0xAC, 0x22, 0x9D, 0x5A, 0xAC, 0x22, 0xED, 0xBF, 0x48, 0xAC, 0x00, 0x00,
0xB7, 0x53, 0x12, 0x40, 0x53, 0xDD, 0x63, 0xA5, 0x52, 0xDD, 0x12, 0x40, 0xB7, 0x53, 0x00, 0x00,
0x48, 0xAC, 0xEE, 0xBF, 0xAC, 0x22, 0x9D, 0x5A, 0xAC, 0x22, 0xEC, 0xBF, 0x48, 0xAC, 0x00, 0x00,
0xB7, 0x53, 0x13, 0x40, 0x52, 0xDD, 0x61, 0xA5, 0x52, 0xDD, 0x12, 0x40, 0xB6, 0x53, 0x00, 0x00,
0x48, 0xAC, 0xED, 0xBF, 0xAD, 0x22, 0x9C, 0x5A, 0xAC, 0x22, 0xEC, 0xBF, 0x48, 0xAC, 0x00, 0x00,
0xB7, 0x53, 0x12, 0x40, 0x51, 0xDD, 0x62, 0xA5, 0x53, 0xDD, 0x13, 0x40, 0xB8, 0x53, 0xFF, 0xFF,
0x47, 0xAC, 0xED, 0xBF, 0xAC, 0x22, 0x9D, 0x5A, 0xAD, 0x22, 0xEE, 0xBF, 0x49, 0xAC, 0xFF, 0xFF,
0xB7, 0x53, 0x12, 0x40, 0x53, 0xDD, 0x63, 0xA5, 0x53, 0xDD, 0x12, 0x40, 0xB7, 0x53, 0x00, 0x00,
0x48, 0xAC, 0xEC, 0xBF, 0xAE, 0x22, 0x9E, 0x5A, 0xAD, 0x22, 0xED, 0xBF, 0x48, 0xAC, 0xFF, 0xFF,
0xB8, 0x53, 0x12, 0x40, 0x53, 0xDD, 0x63, 0xA5, 0x53, 0xDD, 0x13, 0x40, 0xB7, 0x53, 0xFF, 0xFF,
0x48, 0xAC, 0xED, 0xBF, 0xAC, 0x22, 0x9D, 0x5A, 0xAD, 0x22, 0xED, 0xBF, 0x49, 0xAC, 0xFF, 0xFF,
0xB6, 0x53, 0x11, 0x40, 0x52, 0xDD, 0x62, 0xA5, 0x53, 0xDD, 0x13, 0x40, 0xB6, 0x53, 0xFF, 0xFF,
0x48, 0xAC, 0xED, 0xBF, 0xAE, 0x22, 0x9D, 0x5A, 0xAC, 0x22, 0xED, 0xBF, 0x47, 0xAC, 0x00, 0x00,
0xB7, 0x53, 0x12, 0x40, 0x53, 0xDD, 0x62, 0xA5, 0x52, 0xDD, 0x13, 0x40, 0xB6, 0x53, 0x00, 0x00,
0x47, 0xAC, 0xEC, 0xBF, 0xAC, 0x22, 0x9E, 0x5A, 0xAC, 0x22, 0xEC, 0xBF, 0x48, 0xAC, 0x00, 0x00,
0xB7, 0x53, 0x14, 0x40, 0x52, 0xDD, 0x63, 0xA5, 0x52, 0xDD, 0x13, 0x40, 0xB7, 0x53, 0xFF, 0xFF,
0x48, 0xAC, 0xEC, 0xBF, 0xAC, 0x22, 0x9E, 0x5A, 0xAE, 0x22, 0xEC, 0xBF, 0x48, 0xAC, 0x00, 0x00,
0xB6, 0x53, 0x12, 0x40, 0x51, 0xDD, 0x62, 0xA5, 0x52, 0xDD, 0x13, 0x40, 0xB8, 0x53, 0x00, 0x00,
0x48, 0xAC, 0xED, 0xBF, 0xAC, 0x22, 0x9E, 0x5A, 0xAE, 0x22, 0xEC, 0xBF, 0x48, 0xAC, 0xFF, 0xFF,
0xB7, 0x53, 0x13, 0x40, 0x52, 0xDD, 0x62, 0xA5, 0x51, 0xDD, 0x13, 0x40, 0xB6, 0x53, 0x00, 0x00,
0x47, 0xAC, 0xED, 0xBF, 0xAC, 0x22, 0x9D, 0x5A, 0xAD, 0x22, 0xED, 0xBF, 0x48, 0xAC, 0x00, 0x00,
0xB7, 0x53, 0x11, 0x40, 0x54, 0xDD, 0x61, 0xA5, 0x52, 0xDD, 0x12, 0x40, 0xB8, 0x53, 0x00, 0x00,
0x48, 0xAC, 0xED, 0xBF, 0xAD, 0x22, 0x9D, 0x5A, 0xAD, 0x22, 0xED, 0xBF, 0x47, 0xAC, 0x00, 0x00,
0xB6, 0x53, 0x12, 0x40, 0x52, 0xDD, 0x62, 0xA5, 0x52, 0xDD, 0x13, 0x40, 0xB8, 0x53, 0xFF, 0xFF,
0x49, 0xAC, 0xEE, 0xBF, 0xAC, 0x22, 0x9D, 0x5A, 0xAD, 0x22, 0xEC, 0xBF, 0x49, 0xAC, 0xFF, 0xFF,
0xB7, 0x53, 0x13, 0x40, 0x52, 0xDD, 0x61, 0xA5, 0x52, 0xDD, 0x12, 0x40, 0xB8, 0x53, 0x00, 0x00,
0x48, 0xAC, 0xEE, 0xBF, 0xAD, 0x22, 0x9D, 0x5A, 0xAC, 0x22, 0xEC, 0xBF, 0x48, 0xAC, 0x00, 0x00,
0xB7, 0x53, 0x12, 0x40, 0x52, 0xDD, 0x62, 0xA5, 0x52, 0xDD, 0x13, 0x40, 0xB6, 0x53, 0xFE, 0xFF,
0x48, 0xAC, 0xED, 0xBF, 0xAC, 0x22, 0x9D, 0x5A, 0xAD, 0x22, 0xED, 0xBF, 0x48, 0xAC, 0x00, 0x00,
0xB8, 0x53, 0x12, 0x40, 0x52, 0xDD, 0x62, 0xA5, 0x52, 0xDD, 0x12, 0x40, 0xB7, 0x53, 0x00, 0x00,
0x48, 0xAC, 0xED, 0xBF, 0xAC, 0x22, 0x9C, 0x5A, 0xAD, 0x22, 0xEE, 0xBF, 0x48, 0xAC, 0xFF, 0xFF,
0xB5, 0x53, 0x12, 0x40, 0x51, 0xDD, 0x62, 0xA5, 0x53, 0xDD, 0x12, 0x40, 0xB6, 0x53, 0x01, 0x00,
0x48, 0xAC, 0xEC, 0xBF, 0xAD, 0x22, 0x9C, 0x5A, 0xAC, 0x22, 0xEE, 0xBF, 0x48, 0xAC, 0xFF, 0xFF,
0xB8, 0x53, 0x14, 0x40, 0x53, 0xDD, 0x63, 0xA5, 0x53, 0xDD, 0x11, 0x40, 0xB7, 0x53, 0xFF, 0xFF,
0x48, 0xAC, 0xEC, 0xBF, 0xAD, 0x22, 0x9D, 0x5A, 0xAC, 0x22, 0xEC, 0xBF, 0x49, 0xAC, 0x00, 0x00,
0xB7, 0x53, 0x13, 0x40, 0x53, 0xDD, 0x62, 0xA5, 0x52, 0xDD, 0x12, 0x40, 0xB7, 0x53, 0xFF, 0xFF,
0x48, 0xAC, 0xEC, 0xBF, 0xAC, 0x22, 0x9B, 0x5A, 0xAD, 0x22, 0xEB, 0xBF, 0x47, 0xAC, 0x00, 0x00,
0xB8, 0x53, 0x13, 0x40, 0x52, 0xDD, 0x62, 0xA5, 0x51, 0xDD, 0x12, 0x40, 0xB7, 0x53, 0x00, 0x00,
0x48, 0xAC, 0xEC, 0xBF, 0xAD, 0x22, 0x9C, 0x5A, 0xAB, 0x22, 0xED, 0xBF, 0x46, 0xAC, 0xFF, 0xFF,
0xB6, 0x53, 0x13, 0x40, 0x53, 0xDD, 0x63, 0xA5, 0x53, 0xDD, 0x13, 0x40, 0xB6, 0x53, 0x00, 0x00,
0x47, 0xAC, 0xEC, 0xBF, 0xAD, 0x22, 0x9D, 0x5A, 0xAC, 0x22, 0xED, 0xBF, 0x47, 0xAC, 0x00, 0x00,
0xB8, 0x53, 0x12, 0x40, 0x52, 0xDD, 0x63, 0xA5, 0x53, 0xDD, 0x13, 0x40, 0xB7, 0x53, 0x01, 0x00,
0x48, 0xAC, 0xEE, 0xBF, 0xAC, 0x22, 0x9E, 0x5A, 0xAD, 0x22, 0xEC, 0xBF, 0x47, 0xAC, 0x00, 0x00,
0xB7, 0x53, 0x13, 0x40, 0x51, 0xDD, 0x63, 0xA5, 0x50, 0xDD, 0x14, 0x40, 0xB6, 0x53, 0x01, 0x00,
0x46, 0xAC, 0xEE, 0xBF, 0xAA, 0x22, 0x9E, 0x5A, 0xAB, 0x22, 0xEE, 0xBF, 0x46, 0xAC, 0x02, 0x00,
0xB5, 0x53, 0x16, 0x40, 0x4F, 0xDD, 0x66, 0xA5, 0x4E, 0xDD, 0x16, 0x40, 0xB3, 0x53, 0x04, 0x00,
0x43, 0xAC, 0xF0, 0xBF, 0xA8, 0x22, 0xA3, 0x5A, 0xA8, 0x22, 0xF3, 0xBF, 0x42, 0xAC, 0x05, 0x00,
0xB1, 0x53, 0x1A, 0x40, 0x4A, 0xDD, 0x6A, 0xA5, 0x48, 0xDD, 0x1C, 0x40, 0xAD, 0x53, 0x09, 0x00,
0x3E, 0xAC, 0xF5, 0xBF, 0xA1, 0x22, 0xA9, 0x5A, 0xA1, 0x22, 0xFA, 0xBF, 0x39, 0xAC, 0x0D, 0x00,
0xA9, 0x53, 0x22, 0x40, 0x43, 0xDD, 0x73, 0xA5, 0x40, 0xDD, 0x25, 0x40, 0xA3, 0x53, 0x15, 0x00,
0x33, 0xAC, 0x02, 0xC0, 0x96, 0x22, 0xB5, 0x5A, 0x93, 0x22, 0x06, 0xC0, 0x2D, 0xAC, 0x1C, 0x00,
0x9A, 0x53, 0x32, 0x40, 0x31, 0xDD, 0x84, 0xA5, 0x30, 0xDD, 0x37, 0x40, 0x92, 0x53, 0x26, 0x00,
0x1E, 0xAC, 0x18, 0xC0, 0x7F, 0x22, 0xCB, 0x5A, 0x7A, 0x22, 0x20, 0xC0, 0x12, 0xAC, 0x38, 0x00,
0x7A, 0x53, 0x50, 0x40, 0x12, 0xDD, 0xA6, 0xA5, 0x0C, 0xDD, 0x5D, 0x40, 0x68, 0x53, 0x51, 0x00,
0xF2, 0xAB, 0x48, 0xC0, 0x4C, 0x22, 0x03, 0x5B, 0x41, 0x22, 0x60, 0xC0, 0xCD, 0xAB, 0x82, 0x00,
0x2C, 0x53, 0xA9, 0x40, 0xB2, 0xDC, 0x11, 0xA6, 0x94, 0xDC, 0xE3, 0x40, 0xD0, 0x52, 0x01, 0x01,
0x26, 0xAB, 0x39, 0xC1, 0x27, 0x21, 0x70, 0x5C, 0x66, 0x20, 0xF1, 0xC2, 0xDD, 0xA7, 0x07, 0x08,
0xEF, 0x17, 0x1F, 0xF9, 0xFD, 0x03, 0x34, 0xFD, 0x23, 0x02, 0x44, 0xFE, 0x72, 0x01, 0xC0, 0xFE,
0x17, 0x01, 0x09, 0xFF, 0xDF, 0x00, 0x36, 0xFF, 0xB7, 0x00, 0x58, 0xFF, 0x9D, 0x00, 0x6F, 0xFF,
0x86, 0x00, 0x82, 0xFF, 0x75, 0x00, 0x92, 0xFF, 0x69, 0x00, 0x9D, 0xFF, 0x5C, 0x00, 0xA7, 0xFF,
0x53, 0x00, 0xB0, 0xFF, 0x4B, 0x00, 0xB9, 0xFF, 0x43, 0x00, 0xBE, 0xFF, 0x3C, 0x00, 0xC4, 0xFF,
0x38, 0x00, 0xC9, 0xFF, 0x33, 0x00, 0xCE, 0xFF, 0x2E, 0x00, 0xD3, 0xFF, 0x2C, 0x00, 0xD6, 0xFF,
0x28, 0x00, 0xDA, 0xFF, 0x24, 0x00, 0xDC, 0xFF, 0x20, 0x00, 0xE0, 0xFF, 0x1E, 0x00, 0xE2, 0xFF,
0x1C, 0x00, 0xE5, 0xFF, 0x1A, 0x00, 0xE7, 0xFF, 0x18, 0x00, 0xE9, 0xFF, 0x15, 0x00, 0xEA, 0xFF,
0x14, 0x00, 0xED, 0xFF, 0x13, 0x00, 0xEE, 0xFF, 0x10, 0x00};		

#define RECORD_TIME 10            // in seconds
#define RECORD_TYPE FH_AC_MIC_IN  // FH_AC_MIC_IN or FH_AC_LINE_IN
#define ONE_FRAME_SIZE 1024       // how many audio samples in one frame...
#define WAV_BUF_SIZE 256*1024
static int xgd_volume;
static int xgd_audio_open=0;
static int audio_play_force_stop;
static unsigned char *wav_buf;
static int wav_buf_len;
static char last_filename[64]={0};
static char audio_play_status=0;
extern void speaker_sound_enable(int ctrl);
extern int audio_thread_over(void);
extern void audio_player_go(void);
extern void audio_player_over(void);

int dev_audio_open(void)
{
    int ret;
    int fd = -1;
    int enc_type;
    int sample_rate;
    int bit_width;
    int nread,loop;
    unsigned short *pdata;

    riffchunk_t ck;
    datachunk_t dat;
    wavheader_t wavh;
    FH_AC_CONFIG  cfg;
    FH_AC_FRAME_S frame_info;

    xgd_volume = xgd_volume_config();//获取配置音量大小
    if(xgd_volume<0||xgd_volume>=100)
	{
		xgd_volume=25;
    }
    audio_play_force_stop = 0;
	
	rt_kprintf("xgd_volume=%d!\n",xgd_volume);
    fd = open("beep.wav", O_RDWR, 0);
    if (fd < 0)
    {
#if 1
        fd = open("beep.wav", O_WRONLY | O_CREAT | O_TRUNC, 0);
        write(fd,acok,sizeof(acok));
        rt_kprintf("beep.wav size =%d\r\n",sizeof(acok));
        close(fd);

        fd = open("beep.wav", O_RDWR, 0);
#endif
        if(fd < 0)
        {
            rt_kprintf("open audio file  failed!\n");
            return -1;
        }
    }

    /* read RIFF Header */
    if(sizeof(ck) != read(fd,&ck,sizeof(ck)))
    {
        goto err1_out;
    }
    if(strncmp(ck.id,"RIFF",4))
    {
        goto err1_out;
    }

    /* read WAVE Header */ 
    if(sizeof(ck.id) != read(fd,&ck.id,sizeof(ck.id)))
    {
        goto err1_out;
    }
    if(strncmp(ck.id,"WAVE",4))
    {
        goto err1_out;
    }

    /* read fmt Header */
    if(sizeof(ck) != read(fd,&ck,sizeof(ck)))
    {
        goto err1_out;
    }
    if(strncmp(ck.id,"fmt ",4))
    {
        goto err1_out;
    }

    /* read wave header */
    if(sizeof(wavh) != read(fd,&wavh,sizeof(wavh)))
    {
        goto err1_out;
    }

    /* read data Header */
    if(sizeof(dat) != read(fd,&dat,sizeof(dat)))
    {
        goto err1_out;
    }
    if(strncmp(dat.id,"data",4))
    {
        goto err1_out;
    }

    if(wavh.format != 0x1)
    {
        rt_kprintf("wave format must be 0x1,not %d\n",wavh.format);
        goto err1_out;
    }
    if(wavh.bitdepth != 16)
    {
        rt_kprintf("bitdepth must be 16\n");
        goto err1_out;
    }

    // Initial audio decoder hardware
    cfg.io_type     = FH_AC_LINE_OUT;
    cfg.sample_rate = wavh.framerate;
    cfg.bit_width   = wavh.bitdepth;
    cfg.enc_type    = FH_PT_LPCM;
    cfg.channels    = 1;
    cfg.period_size = ONE_FRAME_SIZE;
    cfg.volume      = xgd_volume;

    if ((ret=FH_AC_Init()) != 0)
    {
        rt_kprintf("FH_AC_Init: failed, ret=%d!\n", ret);
        goto err1_out;
    }

    if ((ret=FH_AC_Set_Config(&cfg)) != 0)
    {
        rt_kprintf("FH_AC_Set_Config: failed, ret=%d!\n", ret);
        goto err1_out;
    }
    
    FH_AC_AO_SetVol(xgd_volume);//设置默认的音量
    printf("audio initial done\n");
	
    close(fd);
    xgd_audio_open=1;
	if(wav_buf==NULL)
    {
    	wav_buf = rt_malloc(WAV_BUF_SIZE);
	}
	if(wav_buf==NULL)
 	{
 		return DDI_ERR;
	}
    return DDI_OK;

err1_out:
    close(fd);
    return DDI_ERR;
}

int dev_audio_close(void)
{
	return 0;
}

int dev_audio_play(const unsigned char *lpFile)             
{	
	unsigned char tail[4]  = ".wav";
	unsigned char font[64] = {0};
    int fd = -1;
	int nread;
    int period_bytes;
	int ret=-1;
    riffchunk_t ck;
    wavheader_t wavh;	
    datachunk_t dat;
    FH_AC_FRAME_S fr2_info;
	int data_size=0;
	if(xgd_audio_open==0||wav_buf==NULL)//判断是否已经初始化
		return DDI_EIO;
	if(audio_play_status)
    	audio_player_over();//关闭之前正在播放线程
	if(0==strcmp(lpFile, last_filename))
	{
    	audio_player_go();
		return DDI_OK;
	}
	else
	{
		memset(last_filename,0,sizeof(last_filename));
		memcpy(last_filename,lpFile,strlen(lpFile));
	}
	
	strcpy(font,lpFile);
	if(strstr(lpFile,".wav") == NULL)//判断是否有后缀，若无后缀，自动添加
	{
		strcat(font,tail);
	}

    fd = open(font, O_RDWR, 0);
    if (fd < 0)
    {
        rt_kprintf("open file:%s failed!\n",font);
       goto err1_out;
    }

    /* read RIFF Header */
    if(sizeof(ck) != read(fd,&ck,sizeof(ck)))
    {
        goto err1_out;
    }
    if(strncmp(ck.id,"RIFF",4))
    {
        goto err1_out;
    }

    /* read WAVE Header */ 
    if(sizeof(ck.id) != read(fd,&ck.id,sizeof(ck.id)))
    {
        goto err1_out;
    }
    if(strncmp(ck.id,"WAVE",4))
    {
        goto err1_out;
    }

    /* read fmt Header */
    if(sizeof(ck) != read(fd,&ck,sizeof(ck)))
    {
        goto err1_out;
    }
    if(strncmp(ck.id,"fmt ",4))
    {
        goto err1_out;
    }

    /* read wave header */
    if(sizeof(wavh) != read(fd,&wavh,sizeof(wavh)))
    {
        goto err1_out;
    }
	 /* read data Header */
    if(sizeof(dat) != read(fd,&dat,sizeof(dat)))
    {
        goto err1_out;
    }
    if(strncmp(dat.id,"data",4))
    {
        goto err1_out;
    }
#if 0
    rt_kprintf("Wave file Info:\n");
    rt_kprintf("format:%d\n",wavh.format);
    rt_kprintf("channel:%d\n",wavh.nchannels);
    rt_kprintf("framerate:%d\n",wavh.framerate);
    rt_kprintf("byterate:%d\n",wavh.byterate);
    rt_kprintf("blocksize:%d\n",wavh.blocksize);
    rt_kprintf("bitdepth:%d\n",wavh.bitdepth);
#endif    
    if(wavh.format != 0x1)
    {
        rt_kprintf("wave format must be 0x1,not %d\n",wavh.format);
        goto err1_out;
    }
    if(wavh.bitdepth != 16)
    {
        rt_kprintf("bitdepth must be 16\n");
        goto err1_out;
    }
	if(wavh.nchannels==2)//暂时限定wav为单声道
	{
        rt_kprintf("nchannels must be 1\n");
        goto err1_out;
	}
	data_size=(dat.size/2048+3)*2048;
	//rt_kprintf("data_size= %d\n",data_size);

	memset(wav_buf,0,WAV_BUF_SIZE);
    fr2_info.data = rt_malloc(ONE_FRAME_SIZE*2);
	
	if (NULL == fr2_info.data)
    {
        rt_kprintf("rt_malloc: failed!\n");
        goto err1_out;
    }
	
/*********      read data to  wav_buf        *******/	
	wav_buf_len=2048;
	while (1)
	{
	   nread = read(fd, fr2_info.data, ONE_FRAME_SIZE*2);
	   if (nread <= 0)
	   {
		   rt_kprintf("Read file finished!\n");
		   break;
	   }
	   fr2_info.len = nread;

	   memcpy(wav_buf+wav_buf_len,fr2_info.data,fr2_info.len);
	   wav_buf_len += fr2_info.len;
	}
	//rt_kprintf("wav_buf_len= %d\n",wav_buf_len);
	wav_buf_len=data_size;
	audio_player_go();
    close(fd);
	rt_free(fr2_info.data);
	return DDI_OK;
	
err1_out:
	if(fr2_info.data!=NULL)		
		rt_free(fr2_info.data);
	if(fd!=-1)
    	close(fd);
    return DDI_ERR;

}

int audio_play_thread(void) 			
{
	int ret=-1;
	static int index=0,copy_len=0,end_flag=0;
	unsigned char sound_enable=0;
	int tick_delay=0;
	int ii=0;
    FH_AC_FRAME_S fr2_info;

	audio_play_status=1;

    if ((ret=FH_AC_AO_Enable()) != 0)
    {
        rt_kprintf("FH_AC_AO_Enable: failed, ret=%d!\n", ret);
        goto err1_out;
    }

/*********		ready to play		*******/	
    fr2_info.data = rt_malloc(ONE_FRAME_SIZE*2);

	index = 0;
	while(index < wav_buf_len)
	{
		copy_len = wav_buf_len - index;
		if(copy_len >= ONE_FRAME_SIZE*2)
		{
		  copy_len	= ONE_FRAME_SIZE*2;
		}
		else
		{
		  break;
		}
		memcpy(fr2_info.data,wav_buf+index,copy_len);
		index += copy_len;
		fr2_info.len =copy_len;
		ret=FH_AC_AO_SendFrame(&fr2_info);
		//rt_kprintf("FH_AC_AO_SendFrame  ret=%d!\n", ret);
		if(copy_len < ONE_FRAME_SIZE*2)
		{
		  break;
		}
		if(audio_thread_over())
		{
		
        rt_kprintf("audio_thread_over!\n");
		  end_flag = 1;
		  break;
		}
		if(sound_enable==0)
		{
			speaker_sound_enable(1);
			sound_enable=1;
		}
		
		//rt_kprintf("index= %d\n",index);
	}
	
	tick_delay=wav_buf_len/320+100;
	if(end_flag==0)//等待音频播放完毕
	{
	    for(ii=0;ii<tick_delay;ii++)
	    {   
	        rt_thread_delay(1);
			if(audio_thread_over())
			{
			
				rt_kprintf("audio_thread_over\n");
			  break;
			}
	    }
	}
    speaker_sound_enable(0);
	rt_free(fr2_info.data);
    FH_AC_AO_Disable();
	audio_play_status=0;
	return DDI_OK;
	
err1_out:
	audio_play_status=0;
    return DDI_ERR;
}

int audio_comb_play(unsigned int argc,char *argv[])
{
	unsigned char tail[4]  = ".wav";
	unsigned char font[64] = {0};
	int fd = -1;
	int nread;
	int period_bytes;
	int ret=-1;
	int file_num=0;
	riffchunk_t ck;
	wavheader_t wavh;	
	datachunk_t dat;
	FH_AC_FRAME_S fr2_info;
	
	if(xgd_audio_open==0||wav_buf==NULL)//判断是否已经初始化
		return DDI_EIO;
	if(audio_play_status)
		audio_player_over();//关闭之前正在播放线程

	memset(wav_buf,0,WAV_BUF_SIZE);
	fr2_info.data = rt_malloc(ONE_FRAME_SIZE*2);
	
	if (NULL == fr2_info.data)
	{
		rt_kprintf("rt_malloc: failed!\n");
		goto err1_out;
	}
	wav_buf_len=2048;

	for(file_num=0;file_num<argc;file_num++)
	{
		fd=-1;
		memset(font,0,sizeof(font));
		strcpy(font,argv[file_num]);
		if(strstr(font,".wav") == NULL)//判断是否有后缀，若无后缀，自动添加
		{
			strcat(font,tail);
		}

		fd = open(font, O_RDWR, 0);
		if (fd < 0)
		{
			rt_kprintf("open file:%s failed!\n",font);
		   goto err1_out;
		}

		/* read RIFF Header */
		if(sizeof(ck) != read(fd,&ck,sizeof(ck)))
		{
			goto err1_out;
		}
		if(strncmp(ck.id,"RIFF",4))
		{
			goto err1_out;
		}

		/* read WAVE Header */ 
		if(sizeof(ck.id) != read(fd,&ck.id,sizeof(ck.id)))
		{
			goto err1_out;
		}
		if(strncmp(ck.id,"WAVE",4))
		{
			goto err1_out;
		}

		/* read fmt Header */
		if(sizeof(ck) != read(fd,&ck,sizeof(ck)))
		{
			goto err1_out;
		}
		if(strncmp(ck.id,"fmt ",4))
		{
			goto err1_out;
		}

		/* read wave header */
		if(sizeof(wavh) != read(fd,&wavh,sizeof(wavh)))
		{
			goto err1_out;
		}
		 /* read data Header */
		if(sizeof(dat) != read(fd,&dat,sizeof(dat)))
		{
			goto err1_out;
		}
		if(strncmp(dat.id,"data",4))
		{
			goto err1_out;
		}
#if 0
		rt_kprintf("Wave file Info:\n");
		rt_kprintf("format:%d\n",wavh.format);
		rt_kprintf("channel:%d\n",wavh.nchannels);
		rt_kprintf("framerate:%d\n",wavh.framerate);
		rt_kprintf("byterate:%d\n",wavh.byterate);
		rt_kprintf("blocksize:%d\n",wavh.blocksize);
		rt_kprintf("bitdepth:%d\n",wavh.bitdepth);
#endif    
		if(wavh.format != 0x1)
		{
			rt_kprintf("wave format must be 0x1,not %d\n",wavh.format);
			goto err1_out;
		}
		if(wavh.bitdepth != 16)
		{
			rt_kprintf("bitdepth must be 16\n");
			goto err1_out;
		}
		if(wavh.nchannels==2)//暂时限定wav为单声道
		{
			rt_kprintf("nchannels must be 1\n");
			goto err1_out;
		}
		rt_kprintf("dat.size= %d\n",dat.size);

	/*********		read data to  wav_buf		 *******/	
		while (1)
		{
		   nread = read(fd, fr2_info.data, ONE_FRAME_SIZE*2);
		   if (nread <= 0)
		   {
			   rt_kprintf("Read file finished!\n");
			   break;
		   }
		   fr2_info.len = nread;

		   memcpy(wav_buf+wav_buf_len,fr2_info.data,fr2_info.len);
		   wav_buf_len += fr2_info.len;
		}
		rt_kprintf("wav_buf_len= %d\n",wav_buf_len);
		close(fd);
	}
	
	wav_buf_len+=4096;
	audio_player_go();
	rt_free(fr2_info.data);
	return DDI_OK;
	
err1_out:
	if(fr2_info.data!=NULL) 	
		rt_free(fr2_info.data);
	if(fd!=-1)
		close(fd);
	return DDI_ERR;
}
int dev_audio_stop(void)
{
	if(audio_play_status)
    	audio_player_over();//关闭之前正在播放线程
	return 0;
}

int dev_audio_pause(void)
{
	return 0;  
}

int dev_audio_replay(void)
{
	return 0;
}

int dev_audio_ioctl(unsigned int nCmd, unsigned int lParam, unsigned int wParam)
{
	int ret=-1;

	switch (nCmd)
	{
		case DDI_AUDIO_CTL_VER:
			//暂不支持
			break;
		case DDI_AUDIO_CTL_VOLUME:
			if(lParam>=0&&lParam<=5)
			{
				xgd_config_write(SET_COLUME,lParam);
				xgd_volume_config();
				ret=DDI_OK;
			}
			else
				ret=DDI_EINVAL;
			break;
		case DDI_AUDIO_CTL_BUZZER:
			//暂不支持
			break;
		case DDI_AUDIO_CTL_COMB_PLAY:
			if(lParam<=10&&lParam>=1)
				ret=audio_comb_play(lParam,(char **)wParam);
			break;
		case DDI_AUDIO_CTL_GET_STATUS:
			//由于底层播放时间无法精确计算，故暂不支持
			break;
		case DDI_AUDIO_CTL_BEEP_TUNING:
			//暂不支持
			break;

		default:
			break;

	}

  	return ret;  
}






















