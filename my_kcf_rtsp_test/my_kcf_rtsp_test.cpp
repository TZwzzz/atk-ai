// Copyright 2020 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "my_kcf_rtsp_test.h"


int main(int argc, char *argv[])
{
  RK_CHAR *pDeviceName_01 = "rkispp_scale0";
  RK_CHAR *pDeviceName_02 = "rkispp_scale1";
  RK_CHAR *pcDevNode = "/dev/dri/card0";
  char *iq_file_dir = "/etc/iqfiles";
  RK_S32 s32CamId = 0;
  RK_U32 u32BufCnt = 3;
  RK_U32 fps = 20;
  int ret;

  printf("\n###############################################\n");
  printf("VI CameraIdx: %d\npDeviceName: %s\nResolution: %dx%d\n\n",
          s32CamId,pDeviceName_01,video_width,video_height);
  printf("###############################################\n\n");

  if (iq_file_dir) 
  {
#ifdef RKAIQ
  printf("#Rkaiq XML DirPath: %s\n", iq_file_dir);
  rk_aiq_working_mode_t hdr_mode = RK_AIQ_WORKING_MODE_NORMAL;
  SAMPLE_COMM_ISP_Init(s32CamId,hdr_mode, RK_FALSE,iq_file_dir);
  SAMPLE_COMM_ISP_Run(s32CamId);
  SAMPLE_COMM_ISP_SetFrameRate(s32CamId,25);
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


  RK_MPI_SYS_Init();
  VI_CHN_ATTR_S vi_chn_attr_01;
  memset(&vi_chn_attr_01, 0, sizeof(vi_chn_attr_01));
  vi_chn_attr_01.pcVideoNode = pDeviceName_01;
  vi_chn_attr_01.u32BufCnt = u32BufCnt;
  vi_chn_attr_01.u32Width = video_width;
  vi_chn_attr_01.u32Height = video_height;
  vi_chn_attr_01.enPixFmt = IMAGE_TYPE_NV12;
  vi_chn_attr_01.enBufType = VI_CHN_BUF_TYPE_MMAP;
  vi_chn_attr_01.enWorkMode = VI_WORK_MODE_NORMAL;
  ret = RK_MPI_VI_SetChnAttr(s32CamId, 0, &vi_chn_attr_01);
  ret |= RK_MPI_VI_EnableChn(s32CamId, 0);
  if (ret)
  {
    printf("ERROR: create VI[0:0] error! ret=%d\n", ret);
    return 0;
  }

  RGA_ATTR_S stRgaAttr_01;
  memset(&stRgaAttr_01, 0, sizeof(stRgaAttr_01));
  stRgaAttr_01.bEnBufPool = RK_TRUE;
  stRgaAttr_01.u16BufPoolCnt = 3;
  stRgaAttr_01.u16Rotaion = 0;
  stRgaAttr_01.stImgIn.u32X = 0;
  stRgaAttr_01.stImgIn.u32Y = 0;
  stRgaAttr_01.stImgIn.imgType = IMAGE_TYPE_NV12;
  stRgaAttr_01.stImgIn.u32Width = video_width;
  stRgaAttr_01.stImgIn.u32Height = video_height;
  stRgaAttr_01.stImgIn.u32HorStride = video_width;
  stRgaAttr_01.stImgIn.u32VirStride = video_height;
  stRgaAttr_01.stImgOut.u32X = 0;
  stRgaAttr_01.stImgOut.u32Y = 0;
  stRgaAttr_01.stImgOut.imgType = IMAGE_TYPE_RGB888;
  stRgaAttr_01.stImgOut.u32Width = video_width;
  stRgaAttr_01.stImgOut.u32Height = video_height;
  stRgaAttr_01.stImgOut.u32HorStride = video_width;
  stRgaAttr_01.stImgOut.u32VirStride = video_height;
  ret = RK_MPI_RGA_CreateChn(0, &stRgaAttr_01);
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

  venc_chn_attr.stVencAttr.imageType = IMAGE_TYPE_BGR888;
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

  MPP_CHN_S stSrcChn;
  MPP_CHN_S stDestChn;
  printf("Bind VI[0:0] to RGA[0:0]....\n");
  stSrcChn.enModId = RK_ID_VI;
  stSrcChn.s32DevId = s32CamId;
  stSrcChn.s32ChnId = 0;
  stDestChn.enModId = RK_ID_RGA;
  stSrcChn.s32DevId = s32CamId;
  stDestChn.s32ChnId = 0;
  ret = RK_MPI_SYS_Bind(&stSrcChn, &stDestChn);
  if (ret) 
  {
    printf("ERROR: bind VI[0:0] to RGA[0:0] failed! ret=%d\n", ret);
    return -1;
  }
  
  /* 打开并配置触摸屏设备 */
  ts = ts_setup(NULL, -1);//非阻塞
  if (NULL == ts) 
  {
    fprintf(stderr, "ts_setup error");
    exit(EXIT_FAILURE);
  }

  pthread_t rkmedia_tidp;
  pthread_t rtsp_tidp;              // rtsp线程tid
  pthread_create(&rkmedia_tidp, NULL, rkmedia_thread, NULL);
  pthread_create(&rtsp_tidp, NULL, venc_rtsp_tidp, NULL);
  printf("%s initial finish\n", __func__);

  while (!quit)
  {
    usleep(500000);
  }

  printf("UnBind VI[0:0] to RGA[0:0]....\n");
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

  RK_MPI_RGA_DestroyChn(0);
  RK_MPI_VI_DisableChn(s32CamId, 0);
  
  if (iq_file_dir) 
  {
#if RKAIQ
    SAMPLE_COMM_ISP_Stop(s32CamId);
#endif
  }
  return 0;
}

void *rkmedia_thread(void *args)
{
  pthread_detach(pthread_self());
  using namespace cv;

  int ret;
  bool pressKey = false;
  Mat frame,frame_copy;//用于存储视频帧
  Rect2d roi;//用于存储roi框
  Ptr<Tracker> tracker = TrackerKCF::create();//创建一个KCF跟踪器

  while (!quit)
  {
    MEDIA_BUFFER src_mb = NULL;
    src_mb = RK_MPI_SYS_GetMediaBuffer(RK_ID_RGA, 0, -1);
    if (!src_mb)
    {
      printf("ERROR: RK_MPI_SYS_GetMediaBuffer get null buffer!\n");
      break;
    }
    printf("get src_mb\r\n");
    frame = Mat(video_height, video_width, CV_8UC3, RK_MPI_MB_GetPtr(src_mb));
    
    if (0 > ts_read(ts, &samp, 1)) 
    {
      fprintf(stderr, "ts_read error");
      ts_close(ts);
      exit(EXIT_FAILURE);
    }
    if (false == pressKey && samp.pressure) //按压力>0
    {
      pressKey = true;
      roi = Rect(200,200,500,500);//选定roi范围
      tracker->init(frame,roi);//初始化跟踪器
      printf("Press Key\r\n");
    }

    if(false == pressKey)
    {
      // 采用opencv来绘制矩形框,颜色格式是B、G、R
      rectangle(frame,cv::Point(200, 200),cv::Point(500, 500),cv::Scalar(0,255,255),5,8,0);
    }
    else
    {
      //开始跟踪
      printf("select image\r\n"); 
      double start,cost;
      start = clock();
      tracker->update(frame,roi);//刷新跟踪结果
      cost = (clock()-start)/CLOCKS_PER_SEC;
      printf("time cost:%f\r\n",cost);

      //框选出跟踪目标
      rectangle(frame,roi,Scalar(255,0,0),2,1);
    }
    RK_MPI_SYS_SendMediaBuffer(RK_ID_VENC, 0, src_mb);
    RK_MPI_MB_ReleaseBuffer(src_mb);
  
    src_mb = NULL;
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
