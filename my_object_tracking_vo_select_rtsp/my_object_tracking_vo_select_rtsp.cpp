// Copyright 2020 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "my_object_tracking_vo_select_rtsp.h"


int main(int argc, char *argv[])
{
  RK_CHAR *pDeviceName_01 = "rkispp_scale0";
  RK_CHAR *pDeviceName_02 = "rkispp_scale1";
  RK_CHAR *pcDevNode = "/dev/dri/card0";
  char *iq_file_dir = "/etc/iqfiles";
  RK_S32 s32CamId = 0;
  RK_U32 u32BufCnt = 3;
  RK_U32 fps = 30;
  int ret;

  printf("\n###############################################\n");
  printf("VI CameraIdx: %d\npDeviceName: %s\nResolution: %dx%d\n\n",
          s32CamId,pDeviceName_01,video_width,video_height);

  printf("VO pcDevNode: %s\nResolution: %dx%d\n",
          pcDevNode,disp_width,disp_height);
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

  RGA_ATTR_S stRgaAttr_02;
  memset(&stRgaAttr_02, 0, sizeof(stRgaAttr_02));
  stRgaAttr_02.bEnBufPool = RK_TRUE;
  stRgaAttr_02.u16BufPoolCnt = 3;
  stRgaAttr_02.u16Rotaion = 270;
  stRgaAttr_02.stImgIn.u32X = 0;
  stRgaAttr_02.stImgIn.u32Y = 0;
  stRgaAttr_02.stImgIn.imgType = IMAGE_TYPE_NV12;
  stRgaAttr_02.stImgIn.u32Width = video_width;
  stRgaAttr_02.stImgIn.u32Height = video_height;
  stRgaAttr_02.stImgIn.u32HorStride = video_width;
  stRgaAttr_02.stImgIn.u32VirStride = video_height;
  stRgaAttr_02.stImgOut.u32X = 0;
  stRgaAttr_02.stImgOut.u32Y = 0;
  stRgaAttr_02.stImgOut.imgType = IMAGE_TYPE_RGB888;
  stRgaAttr_02.stImgOut.u32Width = disp_width;
  stRgaAttr_02.stImgOut.u32Height = disp_height;
  stRgaAttr_02.stImgOut.u32HorStride = disp_width;
  stRgaAttr_02.stImgOut.u32VirStride = disp_height;
  ret = RK_MPI_RGA_CreateChn(1, &stRgaAttr_02);
  if (ret) 
  {
    printf("ERROR: create RGA[1:1] falied! ret=%d\n", ret);
    return -1;
  }

  VO_CHN_ATTR_S stVoAttr = {0};
  stVoAttr.pcDevNode = "/dev/dri/card0";
  stVoAttr.emPlaneType = VO_PLANE_OVERLAY;
  stVoAttr.enImgType = IMAGE_TYPE_RGB888;
  stVoAttr.u16Zpos = 0;
  stVoAttr.stImgRect.s32X = 0;
  stVoAttr.stImgRect.s32Y = 0;
  stVoAttr.stImgRect.u32Width = disp_width;
  stVoAttr.stImgRect.u32Height = disp_height;
  stVoAttr.stDispRect.s32X = 0;
  stVoAttr.stDispRect.s32Y = 0;
  stVoAttr.stDispRect.u32Width = disp_width;
  stVoAttr.stDispRect.u32Height = disp_height;
  ret = RK_MPI_VO_CreateChn(0, &stVoAttr);
  if (ret)
  {
    printf("ERROR: create VO[0:0] failed! ret=%d\n", ret);
    return -1;
  }

  VENC_CHN_ATTR_S venc_chn_attr;
  memset(&venc_chn_attr, 0, sizeof(venc_chn_attr));
  switch (enCodecType)
  {
  case RK_CODEC_TYPE_H265:
    venc_chn_attr.stVencAttr.enType = RK_CODEC_TYPE_H265;
    venc_chn_attr.stRcAttr.enRcMode = VENC_RC_MODE_H265CBR;
    venc_chn_attr.stRcAttr.stH265Cbr.u32Gop = 100;
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
    venc_chn_attr.stRcAttr.stH264Cbr.u32Gop = 100;
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

  printf("Bind VI[0:0] to RGA[1:1]....\n");
  stSrcChn.enModId = RK_ID_VI;
  stSrcChn.s32DevId = s32CamId;
  stSrcChn.s32ChnId = 0;
  stDestChn.enModId = RK_ID_RGA;
  stSrcChn.s32DevId = s32CamId;
  stDestChn.s32ChnId = 1;
  ret = RK_MPI_SYS_Bind(&stSrcChn, &stDestChn);
  if (ret) 
  {
    printf("ERROR: bind VI[0:0] to RGA[1:1] failed! ret=%d\n", ret);
    return -1;
  }

  pthread_t rkmedia_tidp[2];
  pthread_t rtsp_tidp;
  pthread_create(&rkmedia_tidp[0], NULL, rkmedia_rga01_venc_thread, NULL);
  pthread_create(&rkmedia_tidp[1], NULL, rkmedia_rga02_vo_thread, NULL);
  pthread_create(&rtsp_tidp, NULL, venc_rtsp_thread, NULL);
  
  printf("%s initial finish\n", __func__);

  while (!quit)
  {
    usleep(500000);
  }
  pthread_join(rkmedia_tidp[0],NULL);
  pthread_join(rkmedia_tidp[1],NULL);
  pthread_join(rtsp_tidp,NULL);

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


void *rkmedia_rga01_venc_thread(void *args)
{
  pthread_detach(pthread_self());
  int ret;

  while (!quit)
  {
    src_mb_share = RK_MPI_SYS_GetMediaBuffer(RK_ID_RGA, 0, -1);
    if (!src_mb_share)
    {
      printf("ERROR: RK_MPI_SYS_GetMediaBuffer get null buffer!\n");
      break;
    }
    //printf("get src_mb\r\n");

    if(false == t_rect.getRect && t_rect.rect.x != 0 && t_rect.rect.y != 0)
    {
      using namespace cv;
      frame_share = Mat(video_height, video_width, CV_8UC3, RK_MPI_MB_GetPtr(src_mb_share));
#if defined _CAMSHIFT_TRACKING_
      if(false == t_rect.getRect && true == one_off)
      {
        rectangle(frame_share,t_rect.rect,Scalar(0,255,0),3,8,0);
      }
      if(0 != t_rect.pts[0].x)
      {
        //画框
        for(int i=0;i<4;i++)
        {
          line(frame_share,t_rect.pts[i],t_rect.pts[(i+1)%4],Scalar(0,255,255),5);
        }
      }
#else
      rectangle(frame_share,t_rect.rect,Scalar(0,255,0),3,8,0);
#endif
    }
    if(true == t_rect.getRect)
    {
      static pthread_t camshift_tidp;//均值迁移法跟踪线程tid
      static bool already_create_pthread = false;//是否已经创建线程
      t_rect.getRect = false;
      if(already_create_pthread)
      {
        ret = pthread_cancel(camshift_tidp);
        if(ret)
        {
          perror("pthread_cancel failed\r\n");
        }
        printf("pthread_cancel successfully\r\n");
      }
      ret = pthread_create(&camshift_tidp,NULL,tracking_thread,NULL);
      if(ret)
      {
        perror("pthread_create failed\r\n");
        exit(EXIT_FAILURE);
      }
      printf("camshift_pthread created successfully\r\n");
      already_create_pthread = true;
      // pthread_exit(NULL);
    }
    RK_MPI_SYS_SendMediaBuffer(RK_ID_VENC, 0, src_mb_share);
    RK_MPI_MB_ReleaseBuffer(src_mb_share);
    src_mb_share = NULL;
  }

  return NULL;
}

void *rkmedia_rga02_vo_thread(void *args)
{
  pthread_detach(pthread_self());
  using namespace cv;

  struct tsdev *ts = NULL;
  struct ts_sample samp;
  int pressure = 0;
  int ret;
  Point point;
  int temp;//用于交换宽高

  ts = ts_setup(NULL,-1);//非零为非阻塞
  if(NULL == ts)
  {
    fprintf(stderr,"ts_setup failed\n");
    exit(EXIT_FAILURE);
  }

  while (!quit)
  {
    MEDIA_BUFFER vo_mb = NULL;
    vo_mb = RK_MPI_SYS_GetMediaBuffer(RK_ID_RGA, 1, -1);
    if (!vo_mb)
    {
      printf("ERROR: RK_MPI_SYS_GetMediaBuffer get null buffer!\n");
      break;
    }
    //printf("get vo_mb\r\n");
    Mat frame = Mat(video_height, video_width, CV_8UC3, RK_MPI_MB_GetPtr(vo_mb));
    
    if (0 > ts_read(ts, &samp, 1)) 
    {
      fprintf(stderr, "ts_read error");
      ts_close(ts);
      exit(EXIT_FAILURE);
    }
    if (samp.pressure) //按压力>0
    {
      //转置坐标
      temp = samp.y;
      samp.y = samp.x;
      samp.x = video_width - temp;
      if (pressure) //若上一次的按压力>0
      {
        printf("坐标(%d, %d)\n", samp.x, samp.y);
        t_rect.rect.width = samp.x - point.x;
        t_rect.rect.height = samp.y - point.y;
        if(t_rect.rect.width > 0 && t_rect.rect.x != point.x)
        {
          t_rect.rect.x = point.x;
        }
        else if(t_rect.rect.width < 0)
        {
          t_rect.rect.width = abs(t_rect.rect.width);
          t_rect.rect.x = samp.x;
        }
        if(t_rect.rect.height > 0 && t_rect.rect.y != point.y)
        {
          t_rect.rect.y = point.y;
        }
        else if(t_rect.rect.height < 0)
        {
          t_rect.rect.height = abs(t_rect.rect.height);
          t_rect.rect.y = samp.y;
        }
      }
      else
      {
        printf("按下(%d, %d)\n", samp.x, samp.y);
        point.x = samp.x;//得到假定左上角坐标
        point.y = samp.y;
        one_off = true;
      }
    }
    else if (false == t_rect.getRect && t_rect.rect.x != 0 && t_rect.rect.y != 0 && true == one_off)
    {
      printf("松开\n");
      t_rect.getRect = true;
      one_off = false;
    }
    pressure = samp.pressure;
    RK_MPI_SYS_SendMediaBuffer(RK_ID_VO, 0, vo_mb);
    RK_MPI_MB_ReleaseBuffer(vo_mb);
    vo_mb = NULL;
  }
}

void *venc_rtsp_thread(void *args)
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

//目标跟踪线程
void *tracking_thread(void *args)
{
  using namespace cv;
  using namespace std;
#if defined _MEANSHIFT_TRACKING_ || defined _CAMSHIFT_TRACKING_
  //计算直方图和反向直方图的相关参数
  int hsize = 16;
  float hranges[] = {0,180};
  const float *phranges = hranges;
  Mat hsv,hue,hist,backproj;
  bool trackObject = false;//是否已经计算目标区域直方图标志
#endif
#ifdef _KCF_TRACKING_
  Ptr<Tracker> tracker = TrackerKCF::create();
  bool trackObject = false;//是否已经初始化第一帧
#endif
#ifdef _MOSSE_TRACKING_
  Ptr<Tracker> tracker = TrackerMOSSE::create();
  bool trackObject = false;//是否已经初始化第一帧
#endif
  while(true)
  {
#if defined _MEANSHIFT_TRACKING_ || defined _CAMSHIFT_TRACKING_
    //将图像转化成HSV颜色空间
    cvtColor(frame_share,hsv,COLOR_BGR2HSV);
    //定义计算直方图和反向直方图的相关数据和图像
    int ch[] = {0,0};
    hue.create(hsv.size(),hsv.depth());
    cv::mixChannels(&hsv,1,&hue,1,ch,1);
    //是否已经完成跟踪目标直方图的计算
    if(false == trackObject)
    {
      //目标区域的HSV颜色空间
      Mat roi(hue,t_rect.rect);
      //计算直方图和直方图归一化
      calcHist(&roi,1,0,roi,hist,1,&hsize,&phranges);
      normalize(hist,hist,0,255,NORM_MINMAX);
      //将标志状态改变，不再计算目标区域的直方图
      trackObject = true;
      cout << "calcHist finished" << endl;
    }
    //计算目标区域的反向直方图
    calcBackProject(&hue,1,0,hist,backproj,&phranges);
#ifdef _MEANSHIFT_TRACKING_
    //均值迁移法跟踪目标
    meanShift(backproj,t_rect.rect,TermCriteria(TermCriteria::EPS|TermCriteria::COUNT,10,1));
#endif
#ifdef _CAMSHIFT_TRACKING_
    //自适应均值迁移法跟踪目标
    RotatedRect trackBox = CamShift(backproj,t_rect.rect,
        TermCriteria(TermCriteria::EPS|TermCriteria::COUNT,10,1));
    if(t_rect.rect.area() <= 1)
    {
      //确保追踪方框有一个最小值
      int cols = backproj.cols,rows = backproj.rows;
      int offset = MIN(rows,cols)+1;
      t_rect.rect = Rect(t_rect.rect.x - offset,t_rect.rect.y - offset,
        t_rect.rect.x + offset,t_rect.rect.y + offset) & Rect(0,0,cols,rows);
    }
    trackBox.points(t_rect.pts);//得到旋转矩阵顶点坐标
#endif
#endif
#ifdef _KCF_TRACKING_
    if(false == trackObject)
    {
      //初始化跟踪器
      tracker->init(frame_share,t_rect.rect);
      trackObject = true;
    }
    //刷新跟踪器结果
    tracker->update(frame_share,t_rect.rect);
#endif
#ifdef _MOSSE_TRACKING_
    if(false == trackObject)
    {
      //初始化跟踪器
      tracker->init(frame_share,t_rect.rect);
      trackObject = true;
    }
    //刷新跟踪器结果
    tracker->update(frame_share,t_rect.rect);
#endif
  }
}

