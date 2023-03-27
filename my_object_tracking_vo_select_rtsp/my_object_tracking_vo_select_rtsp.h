#ifndef _ATK_SSD_OBJECT_RECOGNIZE_H
#define _ATK_SSD_OBJECT_RECOGNIZE_H

#include <getopt.h>
#include <math.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>

#include "im2d.h"
#include "rga.h"
#include "rknn_api.h"
#include "rkmedia_api.h"
#include "sample_common.h"
#include "opencv2/opencv.hpp"
#include "librtsp/rtsp_demo.h"
#include "tslib.h"
#include "opencv2/tracking.hpp"

#define MODEL_INPUT_SIZE 300

/***************************************/
//追踪算法选择
//1._MEANSHIFT_TRACKING_    //均值迁移法
//2._CAMSHIFT_TRACKING_     //自适应均值迁移法
//3._KCF_TRACKING_          //KCF算法
//4._MOSSE_TRACKING_        //MOSSE算法
#define _MEANSHIFT_TRACKING_ //<==修改这里
/***************************************/

#if defined _MEANSHIFT_TRACKING_ || defined _CAMSHIFT_TRACKING_
typedef struct  //识别到图像的坐标（左上）和宽高
{
    cv::Rect rect;
    cv::Point2f pts[4];//旋转矩阵的四个顶点
    bool getRect = false;
}TRACK_RECT;
TRACK_RECT t_rect;
#endif

#if defined _KCF_TRACKING_ || defined _MOSSE_TRACKING_
typedef struct  //识别到图像的坐标（左上）和宽高
{
    cv::Rect2d rect;
    bool getRect = false;
}TRACK_RECT;
TRACK_RECT t_rect;
#endif


RK_U32 video_width = 1280;//视频宽高
RK_U32 video_height = 720;
int disp_width = 720;//屏幕宽高
int disp_height = 1280;
cv::Mat frame_share;//全局frame
MEDIA_BUFFER src_mb_share = NULL;//全局视频缓冲区

rtsp_demo_handle g_rtsplive = NULL;//rtsp句柄
static rtsp_session_handle g_rtsp_session;

CODEC_TYPE_E enCodecType = RK_CODEC_TYPE_H264;//VENC编码方式

static bool one_off = false;//是否开始画下一次框
static bool quit = false;



void *rkmedia_rga01_venc_thread(void *args);

void *rkmedia_rga02_vo_thread(void *args);

void *venc_rtsp_thread(void *args);

void *tracking_thread(void *args);

#endif