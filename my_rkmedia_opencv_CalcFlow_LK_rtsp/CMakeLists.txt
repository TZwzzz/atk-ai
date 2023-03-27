# @author        Liang Huanyan
# @email         lianghuanyan@alientek.com
# http://www.openedv.com/forum.php

cmake_minimum_required(VERSION 3.8)
message(STATUS "cmake version ${CMAKE_VERSION}")

set(TOOLCHAIN_DIR /opt/atk-dlrv1126-toolchain)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}/usr/bin/arm-linux-gnueabihf-g++)
set(CMAKE_C_COMPILER ${TOOLCHAIN_DIR}/usr/bin/arm-linux-gnueabihf-gcc)
set(SYSROOT ${TOOLCHAIN_DIR}/arm-buildroot-linux-gnueabihf/sysroot/usr/include)
set(CMAKE_SYSROOT ${TOOLCHAIN_DIR}/arm-buildroot-linux-gnueabihf/sysroot)

set(CMAKE_CXX_STANDARD 11)
add_definitions(-g -O0 -ggdb -gdwarf -funwind-tables -rdynamic)
add_definitions(-Wno-write-strings -Wno-return-type -Wno-psabi)

set(OPENCV_LIBS opencv_core opencv_dnn opencv_features2d opencv_flann opencv_freetype opencv_highgui opencv_imgcodecs opencv_imgproc opencv_ml opencv_objdetect opencv_photo opencv_shape opencv_stitching opencv_superres opencv_videoio opencv_video opencv_videostab)
set(RKMEDIA_LIBS easymedia rga rkaiq rockx sample_common_isp)
link_directories(include ./librtsp)

include_directories(${SYSROOT})
include_directories(${SYSROOT}/rga)
include_directories(${SYSROOT}/easymedia)
include_directories(${SYSROOT}/rkaiq/uAPI)
include_directories(${SYSROOT}/rkaiq/xcore)
include_directories(${SYSROOT}/rkaiq/algos)
include_directories(${SYSROOT}/rkaiq/common)
include_directories(${SYSROOT}/rkaiq/iq_parser)
include_directories(${SYSROOT}/rockx)
include_directories(.)
include_directories(./librtsp)

add_definitions(-DRKAIQ)

project(alientek)
add_executable(my_rkmedia_opencv_CalcFlow_LK_rtsp my_rkmedia_opencv_CalcFlow_LK_rtsp.cpp)
target_link_libraries(my_rkmedia_opencv_CalcFlow_LK_rtsp ${OPENCV_LIBS} ${RKMEDIA_LIBS} pthread dl librtsp.a)
