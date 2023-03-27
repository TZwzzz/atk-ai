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
#include <time.h>

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

RK_U32 video_width = 1280;
RK_U32 video_height = 720;
rtsp_demo_handle g_rtsplive = NULL;
static rtsp_session_handle g_rtsp_session;
CODEC_TYPE_E enCodecType = RK_CODEC_TYPE_H264;

//用于触摸屏配置
struct tsdev *ts = NULL;
struct ts_sample samp;

static bool quit = false;

void *rkmedia_thread(void *args);

void *venc_rtsp_tidp(void *args);

#endif