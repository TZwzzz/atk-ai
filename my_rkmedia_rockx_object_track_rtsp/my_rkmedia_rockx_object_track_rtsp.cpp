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

#include "my_rkmedia_rockx_object_track_rtsp.h"

int main(int argc, char *argv[]) 
{
  RK_U32 video_width = 1280;
  RK_U32 video_height = 720;

  RK_CHAR *pDeviceName = "rkispp_scale0";
  RK_CHAR *pcDevNode = "/dev/dri/card0";
  char *iq_file_dir = "/etc/iqfiles";
  RK_S32 s32CamId = 0;
  RK_U32 u32BufCnt = 3;
  RK_U32 fps = 20;
  int ret;
  pthread_t rkmedia_vi_rockx_tidp;
  pthread_t rtsp_tidp;
  RK_BOOL bMultictx = RK_FALSE;
  CODEC_TYPE_E enCodecType = RK_CODEC_TYPE_H264;

  
  printf("\n###############################################\n");
  printf("VI CameraIdx: %d\npDeviceName: %s\nResolution: %dx%d\n\n",
          s32CamId,pDeviceName,video_width,video_height);
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

  RK_MPI_SYS_Init();
  VI_CHN_ATTR_S vi_chn_attr;
  vi_chn_attr.pcVideoNode = pDeviceName;
  vi_chn_attr.u32BufCnt = u32BufCnt;
  vi_chn_attr.u32Width = video_width;
  vi_chn_attr.u32Height = video_height;
  vi_chn_attr.enPixFmt = IMAGE_TYPE_NV12;
  vi_chn_attr.enBufType = VI_CHN_BUF_TYPE_MMAP;
  vi_chn_attr.enWorkMode = VI_WORK_MODE_NORMAL;
  ret = RK_MPI_VI_SetChnAttr(s32CamId, 0, &vi_chn_attr);
  ret |= RK_MPI_VI_EnableChn(s32CamId, 0);
  if (ret) 
  {
    printf("ERROR: create VI[0:0] error! ret=%d\n", ret);
    return -1;
  }

  RGA_ATTR_S stRgaAttr;
  memset(&stRgaAttr, 0, sizeof(stRgaAttr));
  stRgaAttr.bEnBufPool = RK_TRUE;
  stRgaAttr.u16BufPoolCnt = u32BufCnt;
  stRgaAttr.u16Rotaion = 0;
  stRgaAttr.stImgIn.u32X = 0;
  stRgaAttr.stImgIn.u32Y = 0;
  stRgaAttr.stImgIn.imgType = IMAGE_TYPE_NV12;
  stRgaAttr.stImgIn.u32Width = video_width;
  stRgaAttr.stImgIn.u32Height = video_height;
  stRgaAttr.stImgIn.u32HorStride = video_width;
  stRgaAttr.stImgIn.u32VirStride = video_height;
  stRgaAttr.stImgOut.u32X = 0;
  stRgaAttr.stImgOut.u32Y = 0;
   stRgaAttr.stImgOut.imgType = IMAGE_TYPE_BGR888;
  stRgaAttr.stImgOut.u32Width = video_width;
  stRgaAttr.stImgOut.u32Height = video_height;
  stRgaAttr.stImgOut.u32HorStride = video_width;
  stRgaAttr.stImgOut.u32VirStride = video_height;
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

  MPP_CHN_S stSrcChn;
  MPP_CHN_S stDestChn;
  printf("Bind VI[0:0] to RGA[0:0]....\n");
  stSrcChn.enModId = RK_ID_VI;
  stSrcChn.s32DevId = s32CamId;
  stSrcChn.s32ChnId = 0;
  stDestChn.enModId = RK_ID_RGA;
  stDestChn.s32DevId = s32CamId;
  stDestChn.s32ChnId = 0;
  ret = RK_MPI_SYS_Bind(&stSrcChn, &stDestChn);
  if (ret) 
  {
    printf("ERROR: Bind VI[0:0] to RGA[0:0] failed! ret=%d\n", ret);
    return -1;
  }

  pthread_create(&rkmedia_vi_rockx_tidp, NULL, rkmedia_vi_rockx_thread, NULL);
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

void *rkmedia_vi_rockx_thread(void *args) 
{
  pthread_detach(pthread_self());//将线程状态改为unjoinable状态，确保资源的释放

  rockx_ret_t ret;
  rockx_handle_t object_det_handle;
  rockx_handle_t object_track_handle;

  char *rockx_data = "/demo/src/rockx_data";
  rockx_config_t *config = rockx_create_config();
  rockx_add_config(config, ROCKX_CONFIG_DATA_PATH, rockx_data);

  ret = rockx_create(&object_det_handle, ROCKX_MODULE_OBJECT_DETECTION, config,sizeof(rockx_config_t));
  if (ret != ROCKX_RET_SUCCESS) 
  {
    printf("ERROR: init rockx module ROCKX_MODULE_OBJECT_DETECTION error %d\n", ret);
  }

  ret = rockx_create(&object_track_handle, ROCKX_MODULE_OBJECT_TRACK, config,sizeof(rockx_config_t));
  if (ret != ROCKX_RET_SUCCESS)
  {
    printf("ERROR: init rockx module ROCKX_MODULE_OBJECT_DETECTION error %d\n", ret);
  }

  while (!quit) 
  {
    MEDIA_BUFFER src_mb = NULL;

    src_mb = RK_MPI_SYS_GetMediaBuffer(RK_ID_RGA, 0, -1);
    clock_t t = clock();
    if (!src_mb) 
    {
      printf("ERROR: RK_MPI_SYS_GetMediaBuffer get null buffer!\n");
      break;
    }
     
    rockx_image_t input_image;
    rkMB_IMAGE_INFO ImageInfo={0};
    RK_MPI_MB_GetImageInfo(src_mb,&ImageInfo);
    input_image.width=ImageInfo.u32Width;   
    input_image.height=ImageInfo.u32Height;
    input_image.pixel_format = ROCKX_PIXEL_FORMAT_RGB888;
    input_image.size = RK_MPI_MB_GetSize(src_mb);
    input_image.data = (uint8_t *)RK_MPI_MB_GetPtr(src_mb);

    rockx_object_array_t object_array;
    memset(&object_array, 0, sizeof(rockx_object_array_t));
    ret = rockx_object_detect(object_det_handle, &input_image, &object_array, nullptr);
    if (ret != ROCKX_RET_SUCCESS) 
    {
      printf("ERROR: rockx_object_detect error %d\n", ret);
    }

    int max_track_time = 4;
    rockx_object_array_t out_track_objects;
    ret = rockx_object_track(object_track_handle, input_image.width,  input_image.height, 
                             max_track_time,&object_array, &out_track_objects);
    if (ret != ROCKX_RET_SUCCESS) 
    {
      printf("ERROR: rockx_object_track error %d\n", ret);
    }

    for (int i = 0; i < out_track_objects.count; i++) 
    {
      int left = out_track_objects.object[i].box.left;
      int top = out_track_objects.object[i].box.top;
      int right = out_track_objects.object[i].box.right;
      int bottom = out_track_objects.object[i].box.bottom;
      int cls_idx = out_track_objects.object[i].cls_idx;
      const char *cls_name = ROCKX_OBJECT_DETECTION_LABELS_91[cls_idx];
      float score = out_track_objects.object[i].score;
      int track_id = out_track_objects.object[i].id;
      printf("\nbox=(%d %d %d %d) cls_name=%s, score=%f track_id=%d\n", 
              left, top, right, bottom,cls_name, score, track_id);

      char show_str[32];
      memset(show_str, 0, 32);
      snprintf(show_str, 32, "%d-%s", track_id, cls_name);

      // 使用opencv来画矩形框和绘制文字
      using namespace cv;
      Mat show_img = Mat(input_image.height, input_image.width, CV_8UC3,RK_MPI_MB_GetPtr(src_mb));
      // 采用opencv来绘制矩形框,颜色格式是B、G、R
      cv::rectangle(show_img,cv::Point(left, top),cv::Point(right, bottom),cv::Scalar(0,0,255),3,8,0);
      std::string text=show_str;
      // 采用opencv在框的旁边绘制文字,颜色格式是B、G、R
      cv::putText(show_img, text, cv::Point(left, top-16), cv::FONT_HERSHEY_TRIPLEX, 2, cv::Scalar(0,0,255),2,8,0);
      show_img.release();
    }
    t = clock() - t;
    printf("t = %ld\r\n",t);
    RK_MPI_SYS_SendMediaBuffer(RK_ID_VENC, 0, src_mb);
    RK_MPI_MB_ReleaseBuffer(src_mb);
  }
  rockx_destroy(object_track_handle);
  rockx_destroy(object_det_handle);
  return NULL;
}


void *venc_rtsp_tidp(void *args)
{
  pthread_detach(pthread_self());//将线程状态改为unjoinable状态，确保资源的释放
  
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