#ifndef _RKMEDIA_ROCKX_OBJECT_TRACK_H
#define _RKMEDIA_ROCKX_OBJECT_TRACK_H

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

#include "im2d.h"
#include "rga.h"
#include "rockx.h"
#include "rkmedia_api.h"
#include "sample_common.h"
#include "opencv2/opencv.hpp"
#include "modules/carplate.h"

static bool quit = false;

void *rkmedia_vi_rockx_thread(void *args);

#endif
