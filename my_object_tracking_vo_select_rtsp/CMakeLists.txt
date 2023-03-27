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
add_definitions(-Wno-write-strings -Wno-return-type)

set(OPENCV_LIBS opencv_dnn opencv_stitching opencv_superres opencv_videostab opencv_aruco opencv_bgsegm opencv_bioinspired opencv_ccalib opencv_dpm opencv_highgui opencv_videoio opencv_face opencv_fuzzy opencv_hfs opencv_img_hash opencv_line_descriptor opencv_optflow opencv_reg opencv_rgbd opencv_saliency opencv_stereo opencv_structured_light opencv_phase_unwrapping opencv_surface_matching opencv_tracking opencv_datasets opencv_plot opencv_xfeatures2d opencv_shape opencv_video opencv_ml opencv_ximgproc opencv_xobjdetect opencv_objdetect opencv_calib3d opencv_imgcodecs opencv_features2d opencv_flann opencv_freetype opencv_xphoto opencv_photo opencv_imgproc opencv_core)
set(RKMEDIA_LIBS easymedia od_share rga rkaiq rknn_api sample_common_isp)
link_directories(include ./librtsp)
	
include_directories(${SYSROOT})
include_directories(${SYSROOT}/rga)
include_directories(${SYSROOT}/easymedia)
include_directories(${SYSROOT}/rkaiq/uAPI)
include_directories(${SYSROOT}/rkaiq/xcore)
include_directories(${SYSROOT}/rkaiq/algos)
include_directories(${SYSROOT}/rkaiq/common)
include_directories(${SYSROOT}/rkaiq/iq_parser)
include_directories(${SYSROOT}/rknn)
include_directories(.)
include_directories(./include)
include_directories(./include/3rdparty)
include_directories(./include/3rdparty/drm/include/libdrm)
include_directories(./include/3rdparty/rga/include)
include_directories(./librtsp)

add_definitions(-DRKAIQ)


add_executable(my_object_tracking_vo_select_rtsp my_object_tracking_vo_select_rtsp.cpp)
target_link_libraries(my_object_tracking_vo_select_rtsp ${OPENCV_LIBS} ${RKMEDIA_LIBS} pthread dl librtsp.a ts) 
