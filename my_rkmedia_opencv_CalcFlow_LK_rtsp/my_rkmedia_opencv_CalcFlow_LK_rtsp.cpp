/****************************************************************************
 *
 *    Copyright (c) 2017 - 2019 by Rockchip Corp.  All rights reserved.
 *
 *    The material in this file is confidential and contains trade secrets
 *    of Rockchip Corporation. This is proprietary information owned by
 *    Rockchip Corporation. No part of this work may be disclosed,
 *    reproduced, copied, transmitted, or used in any way for any purpose,
 *    without the express written permission of Rockchip Corporation.
 *
 *****************************************************************************/

#include "my_rkmedia_opencv_CalcFlow_LK_rtsp.h"

int main(int argc, char *argv[])
{
  RK_U32 video_width = 640;  // 输入图像区域的宽度
  RK_U32 video_height = 360; // 输入图像区域的高度

  RK_CHAR *pDeviceName = "rkispp_scale0"; // video节点路径
  RK_CHAR *pcDevNode = "/dev/dri/card0";  // 视频VO输出设备节点
  char *iq_file_dir = "/etc/iqfiles";
  RK_S32 s32CamId = 0;  // 管道号
  RK_U32 u32BufCnt = 3; // VI捕获视频缓冲区计数
  RK_U32 fps = 10;      // 帧率
  int ret;
  pthread_t rkmedia_vi_opencv_tidp; // opencv线程tid
  pthread_t rtsp_tidp;              // rtsp线程tid
  RK_BOOL bMultictx = RK_FALSE;
  CODEC_TYPE_E enCodecType = RK_CODEC_TYPE_H264;

  printf("\n###############################################\n");
  printf("VI CameraIdx: %d\npDeviceName: %s\nResolution: %dx%d\n\n",
         s32CamId, pDeviceName, video_width, video_height);
  printf("###############################################\n\n");

  if (iq_file_dir)
  {
#ifdef RKAIQ
    printf("#Rkaiq XML DirPath: %s\n", iq_file_dir);
    printf("#bMultictx: %d\n\n", bMultictx);
    rk_aiq_working_mode_t hdr_mode = RK_AIQ_WORKING_MODE_NORMAL;
    SAMPLE_COMM_ISP_Init(s32CamId, hdr_mode, bMultictx, iq_file_dir);
    SAMPLE_COMM_ISP_Run(s32CamId);
    SAMPLE_COMM_ISP_SetFrameRate(s32CamId, fps);
#endif
  }

  // init rtsp
  g_rtsplive = create_rtsp_demo(554);
  g_rtsp_session = rtsp_new_session(g_rtsplive, "/live/main_stream");
  if (enCodecType == RK_CODEC_TYPE_H264)
  {
    rtsp_set_video(g_rtsp_session, RTSP_CODEC_ID_VIDEO_H264, NULL, 0);
  }
  else if (enCodecType == RK_CODEC_TYPE_H265)
  {
    rtsp_set_video(g_rtsp_session, RTSP_CODEC_ID_VIDEO_H265, NULL, 0);
  }
  else
  {
    printf("not support other type\n");
    return -1;
  }
  rtsp_sync_video_ts(g_rtsp_session, rtsp_get_reltime(), rtsp_get_ntptime());

  RK_MPI_SYS_Init();                      // 初始化MPI
  VI_CHN_ATTR_S vi_chn_attr;              // 定义VI通道属性结构体指针
  vi_chn_attr.pcVideoNode = pDeviceName;  // video 节点路径
  vi_chn_attr.u32BufCnt = u32BufCnt;      // VI捕获视频缓冲区计数
  vi_chn_attr.u32Width = video_width;     // video宽度
  vi_chn_attr.u32Height = video_height;   // video高度
  vi_chn_attr.enPixFmt = IMAGE_TYPE_NV12; // video格式
  vi_chn_attr.enBufType = VI_CHN_BUF_TYPE_MMAP;
  vi_chn_attr.enWorkMode = VI_WORK_MODE_NORMAL; // VI通道工作模式
  // s32CamId为管道号,0为VI通道号,vi_chn_attr为VI通道属性结构体指针
  ret = RK_MPI_VI_SetChnAttr(s32CamId, 0, &vi_chn_attr);
  // s32CamId 为 VI 管道号； 0 为 VI 通道号
  ret |= RK_MPI_VI_EnableChn(s32CamId, 0);
  if (ret)
  {
    printf("ERROR: create VI[0:0] error! ret=%d\n", ret);
    return -1;
  }

  RGA_ATTR_S stRgaAttr; // 定义 RGA 属性结构体
  memset(&stRgaAttr, 0, sizeof(stRgaAttr));
  stRgaAttr.bEnBufPool = RK_TRUE;                 // 使能缓冲池
  stRgaAttr.u16BufPoolCnt = u32BufCnt;            // 缓冲池计数
  stRgaAttr.u16Rotaion = 0;                       // stRgaAttr.u16Rotaion = 270; //旋转 取值范围0,90,180,270
  stRgaAttr.stImgIn.u32X = 0;                     // RGA通道输入图片的X轴坐标
  stRgaAttr.stImgIn.u32Y = 0;                     // RGA通道输入图片的y轴坐标
  stRgaAttr.stImgIn.imgType = IMAGE_TYPE_NV12;    // RGA通道输入图片格式
  stRgaAttr.stImgIn.u32Width = video_width;       // 输入图片宽度
  stRgaAttr.stImgIn.u32Height = video_height;     // 输入图片高度
  stRgaAttr.stImgIn.u32HorStride = video_width;   // 输入图片虚宽
  stRgaAttr.stImgIn.u32VirStride = video_height;  // 输入图片虚高
  stRgaAttr.stImgOut.u32X = 0;                    // RGA通道输出图片的X轴坐标
  stRgaAttr.stImgOut.u32Y = 0;                    // RGA通道输出图片的Y轴坐标
  stRgaAttr.stImgOut.imgType = IMAGE_TYPE_BGR888; // RGA通道输出图片格式
  stRgaAttr.stImgOut.u32Width = video_width;      // 输出图片宽度
  stRgaAttr.stImgOut.u32Height = video_height;    // 输出图片高度
  stRgaAttr.stImgOut.u32HorStride = video_width;  // 输出图片虚宽
  stRgaAttr.stImgOut.u32VirStride = video_height; // 输出图片虚高
  // 创建RGA通道
  // 0为RGA通道号,stRgaAttr为RGA通道属性指针
  ret = RK_MPI_RGA_CreateChn(0, &stRgaAttr);
  if (ret)
  {
    printf("ERROR: create RGA[0:0] falied! ret=%d\n", ret);
    return -1;
  }

  VENC_CHN_ATTR_S venc_chn_attr;
  memset(&venc_chn_attr, 0, sizeof(venc_chn_attr));
  switch (enCodecType)
  {
  case RK_CODEC_TYPE_H265:
    venc_chn_attr.stVencAttr.enType = RK_CODEC_TYPE_H265;
    venc_chn_attr.stRcAttr.enRcMode = VENC_RC_MODE_H265CBR;
    venc_chn_attr.stRcAttr.stH265Cbr.u32Gop = 30;
    venc_chn_attr.stRcAttr.stH265Cbr.u32BitRate = video_width * video_height;
    // frame rate: in 30/1, out 30/1.
    venc_chn_attr.stRcAttr.stH265Cbr.fr32DstFrameRateDen = 1;
    venc_chn_attr.stRcAttr.stH265Cbr.fr32DstFrameRateNum = 30;
    venc_chn_attr.stRcAttr.stH265Cbr.u32SrcFrameRateDen = 1;
    venc_chn_attr.stRcAttr.stH265Cbr.u32SrcFrameRateNum = 30;
    break;
  case RK_CODEC_TYPE_H264:
  default:
    venc_chn_attr.stVencAttr.enType = RK_CODEC_TYPE_H264;
    venc_chn_attr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
    venc_chn_attr.stRcAttr.stH264Cbr.u32Gop = 30;
    venc_chn_attr.stRcAttr.stH264Cbr.u32BitRate = video_width * video_height * 3;
    // frame rate: in 30/1, out 30/1.
    venc_chn_attr.stRcAttr.stH264Cbr.fr32DstFrameRateDen = 1;
    venc_chn_attr.stRcAttr.stH264Cbr.fr32DstFrameRateNum = 30;
    venc_chn_attr.stRcAttr.stH264Cbr.u32SrcFrameRateDen = 1;
    venc_chn_attr.stRcAttr.stH264Cbr.u32SrcFrameRateNum = 30;
    break;
  }

  venc_chn_attr.stVencAttr.imageType = IMAGE_TYPE_RGB888;
  venc_chn_attr.stVencAttr.u32PicWidth = video_width;
  venc_chn_attr.stVencAttr.u32PicHeight = video_height;
  venc_chn_attr.stVencAttr.u32VirWidth = video_width;
  venc_chn_attr.stVencAttr.u32VirHeight = video_height;
  venc_chn_attr.stVencAttr.u32Profile = 66;
  ret = RK_MPI_VENC_CreateChn(0, &venc_chn_attr);
  if (ret)
  {
    printf("ERROR: create VENC[0:0] error! ret=%d\n", ret);
    return -1;
  }

  MPP_CHN_S stSrcChn; // 定义模块设备通道结构体
  MPP_CHN_S stDestChn;
  printf("Bind VI[0:0] to RGA[0:0]....\n");
  stSrcChn.enModId = RK_ID_VI;   // 模块号为RK_ID_VI
  stSrcChn.s32DevId = s32CamId;  // 模块通道号
  stSrcChn.s32ChnId = 0;         // 设置VI通道号
  stDestChn.enModId = RK_ID_RGA; // 模块号
  stDestChn.s32DevId = s32CamId; // 模块设备号
  stDestChn.s32ChnId = 0;        // 模块通道号
  // 通道绑定
  // stSrcChn为源通道指针,stDestChn目的通道指针
  ret = RK_MPI_SYS_Bind(&stSrcChn, &stDestChn);
  if (ret)
  {
    printf("ERROR: Bind VI[0:0] to RGA[0:0] failed! ret=%d\n", ret);
    return -1;
  }

  pthread_create(&rkmedia_vi_opencv_tidp, NULL, rkmedia_vi_opencv_thread, NULL);
  pthread_create(&rtsp_tidp, NULL, venc_rtsp_tidp, NULL);

  printf("%s initial finish\n", __func__);

  while (!quit)
  {
    usleep(500000);
  }

  printf("%s exit!\n", __func__);
  printf("Unbind VI[0:0] to RGA[0:0]....\n");
  stSrcChn.enModId = RK_ID_VI;
  stSrcChn.s32DevId = s32CamId;
  stSrcChn.s32ChnId = 0;
  stDestChn.enModId = RK_ID_RGA;
  stSrcChn.s32DevId = s32CamId;
  stDestChn.s32ChnId = 0;
  ret = RK_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
  if (ret)
  {
    printf("ERROR: unbind VI[0:0] to RGA[0:0] failed! ret=%d\n", ret);
    return -1;
  }

  printf("Destroy VENC[0:0] channel\n");
  ret = RK_MPI_VENC_DestroyChn(0);
  if (ret)
  {
    printf("ERROR: Destroy VENC[0:0] error! ret=%d\n", ret);
    return 0;
  }

  printf("Destroy RGA[0:0] channel\n");
  ret = RK_MPI_RGA_DestroyChn(0);
  if (ret)
  {
    printf("ERROR: Destroy RGA[0:0] error! ret=%d\n", ret);
    return 0;
  }

  printf("Destroy VI[0:0] channel\n");
  ret = RK_MPI_VI_DisableChn(s32CamId, 0);
  if (ret)
  {
    printf("ERROR: destroy VI[0:0] error! ret=%d\n", ret);
    return -1;
  }

  if (iq_file_dir)
  {
#if RKAIQ
    SAMPLE_COMM_ISP_Stop(s32CamId);
#endif
  }
  return 0;
}

int nv12_border(char *pic, int pic_w, int pic_h, int rect_x, int rect_y,
                int rect_w, int rect_h, int R, int G, int B)
{
  /* Set up the rectangle border size */
  const int border = 5;
  /* RGB convert YUV */
  int Y, U, V;
  Y = 0.299 * R + 0.587 * G + 0.114 * B;
  U = -0.1687 * R + 0.3313 * G + 0.5 * B + 128;
  V = 0.5 * R - 0.4187 * G - 0.0813 * B + 128;
  /* Locking the scope of rectangle border range */
  int j, k;
  for (j = rect_y; j < rect_y + rect_h; j++)
  {
    for (k = rect_x; k < rect_x + rect_w; k++)
    {
      if (k < (rect_x + border) || k > (rect_x + rect_w - border) ||
          j < (rect_y + border) || j > (rect_y + rect_h - border))
      {
        /* Components of YUV's storage address index */
        int y_index = j * pic_w + k;
        int u_index =
            (y_index / 2 - pic_w / 2 * ((j + 1) / 2)) * 2 + pic_w * pic_h;
        int v_index = u_index + 1;
        /* set up YUV's conponents value of rectangle border */
        pic[y_index] = Y;
        pic[u_index] = U;
        pic[v_index] = V;
      }
    }
  }
  return 0;
}

void *rkmedia_vi_opencv_thread(void *args)
{
  pthread_detach(pthread_self()); // 将线程状态改为unjoinable状态，确保资源的释放

  Mat prevframe, prevImg;
  MEDIA_BUFFER src_mb = RK_MPI_SYS_GetMediaBuffer(RK_ID_RGA, 0, -1);
  rkMB_IMAGE_INFO ImageInfo = {0};
  RK_MPI_MB_GetImageInfo(src_mb, &ImageInfo);
  // 得到第一张图像
  prevframe = Mat(ImageInfo.u32Height, ImageInfo.u32Width, CV_8UC3, RK_MPI_MB_GetPtr(src_mb));
  cvtColor(prevframe, prevImg, COLOR_BGR2GRAY);
  // 角点检测相关参数设置
  vector<Point2f> Points;
  double qualityLevel = 0.01;
  int minDistance = 10;
  int blockSize = 3;
  bool useHarrisDetector = false;
  double k = 0.04;
  int Corners = 5000;
  // 角点检测
  goodFeaturesToTrack(prevImg, Points, Corners, qualityLevel, minDistance,
                      Mat(), blockSize, useHarrisDetector, k);
  // 稀疏光流法检测相关参数设置
  vector<Point2f> prevPts; // 前一帧图像角点坐标
  vector<Point2f> nextPts; // 当前帧图像角点坐标
  vector<uchar> status;    // 角点是否跟踪成功的状态向量
  vector<float> err;
  TermCriteria criteria = TermCriteria(TermCriteria::COUNT + TermCriteria::EPS, 30, 0.01);
  double derivlambda = 0.5;
  int flags = 0;

  // 初始状态的角点
  vector<Point2f> initPoints;
  initPoints.insert(initPoints.end(), Points.begin(), Points.end());
  // 前一帧图像中的角点坐标
  prevPts.insert(prevPts.end(), Points.begin(), Points.end());

  while (!quit)
  {
    src_mb = NULL;
    src_mb = RK_MPI_SYS_GetMediaBuffer(RK_ID_RGA, 0, -1);
    clock_t t = clock();
    if (!src_mb)
    {
      printf("ERROR: RK_MPI_SYS_GetMediaBuffer get null buffer!\n");
      break;
    }
    Mat nextframe, nextImg;
    nextframe = Mat(ImageInfo.u32Height, ImageInfo.u32Width, CV_8UC3, RK_MPI_MB_GetPtr(src_mb));
    // 光流跟踪
    cvtColor(nextframe, nextImg, COLOR_BGR2GRAY);
    calcOpticalFlowPyrLK(prevImg, nextImg, prevPts, nextPts, status, err,
                         Size(31, 31), 3, criteria, derivlambda, flags);
    // 判断角点是否移动
    size_t i, k;
    for (i = k = 0; i < nextPts.size(); i++)
    {
      // 距离和状态测量
      double dist = abs(prevPts[i].x - nextPts[i].x) + abs(prevPts[i].y - nextPts[i].y);
      if (status[i] && dist > 2)
      {
        prevPts[k] = prevPts[i];
        initPoints[k] = initPoints[i];
        nextPts[k++] = nextPts[i];
        circle(nextframe, Point(nextPts[i].x, nextPts[i].y), 3, Scalar(0, 255, 0), -1, 8);
      }
    }
    // 更新移动角点数目
    nextPts.resize(k);
    prevPts.resize(k);
    initPoints.resize(k);
    // 绘制跟踪轨迹
    draw_lines(nextframe, initPoints, nextPts);
    // 更新角点坐标和前一帧图像
    std::swap(nextPts, prevPts);
    nextImg.copyTo(prevImg);
    // 如果角点数目少于30个，那么重新检测角点
    if (initPoints.size() < 30)
    {
      goodFeaturesToTrack(prevImg, Points, Corners, qualityLevel, minDistance,
                          Mat(), blockSize, useHarrisDetector, k);
      initPoints.insert(initPoints.end(), Points.begin(), Points.end());
      prevPts.insert(prevPts.end(), Points.begin(), Points.end());
      printf("total feature points : %d\n", prevPts.size());
    }
    t = clock() - t;
    printf("t = %ld\r\n",t);
    RK_MPI_SYS_SendMediaBuffer(RK_ID_VENC, 0, src_mb);
    RK_MPI_MB_ReleaseBuffer(src_mb);
  }

  return NULL;
}

void *venc_rtsp_tidp(void *args)
{
  pthread_detach(pthread_self()); // 将线程状态改为unjoinable状态，确保资源的释放

  MEDIA_BUFFER mb = NULL;

  while (!quit)
  {
    mb = RK_MPI_SYS_GetMediaBuffer(RK_ID_VENC, 0, -1);
    if (!mb)
    {
      printf("ERROR: RK_MPI_SYS_GetMediaBuffer get null buffer!\n");
      break;
    }

    rtsp_tx_video(g_rtsp_session, (unsigned char *)RK_MPI_MB_GetPtr(mb), RK_MPI_MB_GetSize(mb), RK_MPI_MB_GetTimestamp(mb));
    RK_MPI_MB_ReleaseBuffer(mb);
    rtsp_do_event(g_rtsplive);
  }
  return NULL;
}

void draw_lines(Mat &image, vector<Point2f> pt1, vector<Point2f> pt2)
{
  static vector<Scalar> color_lut; // 颜色查找表
  RNG rng(500);
  if (color_lut.size() < pt1.size())
  {
    for (size_t t = 0; t < pt1.size(); t++)
    {
      color_lut.push_back(Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255)));
    }
  }
  for (size_t t = 0; t < pt1.size(); t++)
  {
    line(image, pt1[t], pt2[t], color_lut[t], 2, 8, 0);
  }
}