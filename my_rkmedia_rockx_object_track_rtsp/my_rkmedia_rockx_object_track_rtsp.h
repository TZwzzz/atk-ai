#ifndef _RKMEDIA_ROCKX_FACE_DETECTION_RTSP_H
#define _RKMEDIA_ROCKX_FACE_DETECTION_RTSP_H

#include <assert.h>
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <iostream>
#include <vector>

#include "im2d.h"
#include "rga.h"
#include "rockx.h"
#include "rkmedia_api.h"
#include "rkmedia_venc.h"
#include "sample_common.h"
#include "opencv2/opencv.hpp"
#include "modules/carplate.h"
#include "librtsp/rtsp_demo.h"

using namespace std;
using namespace cv;

static bool quit = false;
rtsp_demo_handle g_rtsplive = NULL;
static rtsp_session_handle g_rtsp_session;

int nv12_border(char *pic, int pic_w, int pic_h, int rect_x, int rect_y,
                int rect_w, int rect_h, int R, int G, int B);
void *rkmedia_vi_rockx_thread(void *args);
void *venc_rtsp_tidp(void *args);

#endif
