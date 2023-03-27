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
#include <errno.h> 
#include <malloc.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>  
#include <net/if.h>  
#include <arpa/inet.h> 
#include <sys/ioctl.h>  
#include <sys/types.h>  
#include <sys/time.h> 

#include <thread>
#include <memory>
#include <iostream>
#include <string>

#include "im2d.h"
#include "rga.h"
#include "rknn_api.h"
#include "rkmedia_api.h"
#include "sample_common.h"
#include "opencv2/opencv.hpp"
#include "RtspServer.h"
#include "Timer.h"

#define MODEL_INPUT_SIZE 300
#define IF_NAME "eth0"

RK_U32 video_width = 1280;
RK_U32 video_height = 720;
int disp_width = 720;
int disp_height = 1280;
CODEC_TYPE_E enCodecType = RK_CODEC_TYPE_H264;

static bool quit = false;

void SendFrameThread(xop::RtspServer* rtsp_server, xop::MediaSessionId session_id, int& clients);
int get_local_ip(const char *eth_inf,std::string *ip);

#endif
