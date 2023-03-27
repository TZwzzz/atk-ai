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

#include "rkmedia_rockx_object_track.h"

int main(int argc, char *argv[]) 
{
  int disp_width = 720;
  int disp_height = 1280;
  RK_U32 video_width = 720;
  RK_U32 video_height = 1280;

  RK_CHAR *pDeviceName = "rkispp_scale0";
  RK_CHAR *pcDevNode = "/dev/dri/card0";
  char *iq_file_dir = "/etc/iqfiles";
  RK_S32 s32CamId = 0;
  RK_U32 u32BufCnt = 3;
  RK_U32 fps = 20;
  int ret;
  pthread_t rkmedia_vi_rockx_tidp;
  RK_BOOL bMultictx = RK_FALSE;
  
  printf("\n###############################################\n");
  printf("VI CameraIdx: %d\npDeviceName: %s\nResolution: %dx%d\n\n",
          s32CamId,pDeviceName,video_width,video_height);
  printf("VO pcDevNode: %s\nResolution: %dx%d\n",
          pcDevNode,disp_height,disp_width);
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
  
  RK_MPI_SYS_Init();
  VI_CHN_ATTR_S vi_chn_attr;
  vi_chn_attr.pcVideoNode = pDeviceName;
  vi_chn_attr.u32BufCnt = u32BufCnt;
  vi_chn_attr.u32Width = video_width;
  vi_chn_attr.u32Height = video_height;
  vi_chn_attr.enPixFmt = IMAGE_TYPE_NV12;
  vi_chn_attr.enBufType = VI_CHN_BUF_TYPE_MMAP;
  vi_chn_attr.enWorkMode = VI_WORK_MODE_NORMAL;
  ret = RK_MPI_VI_SetChnAttr(s32CamId, 1, &vi_chn_attr);
  ret |= RK_MPI_VI_EnableChn(s32CamId, 1);
  if (ret) 
  {
    printf("ERROR: create VI[0:1] error! ret=%d\n", ret);
    return -1;
  }

  RGA_ATTR_S stRgaAttr;
  memset(&stRgaAttr, 0, sizeof(stRgaAttr));
  stRgaAttr.bEnBufPool = RK_TRUE;
  stRgaAttr.u16BufPoolCnt = 3;
  stRgaAttr.u16Rotaion = 270;
  stRgaAttr.stImgIn.u32X = 0;
  stRgaAttr.stImgIn.u32Y = 0;
  stRgaAttr.stImgIn.imgType = IMAGE_TYPE_NV12;
  stRgaAttr.stImgIn.u32Width = video_width;
  stRgaAttr.stImgIn.u32Height = video_height;
  stRgaAttr.stImgIn.u32HorStride = video_width;
  stRgaAttr.stImgIn.u32VirStride = video_height;
  stRgaAttr.stImgOut.u32X = 0;
  stRgaAttr.stImgOut.u32Y = 0;
  stRgaAttr.stImgOut.imgType = IMAGE_TYPE_RGB888;
  stRgaAttr.stImgOut.u32Width = video_width;
  stRgaAttr.stImgOut.u32Height = video_height;
  stRgaAttr.stImgOut.u32HorStride = video_width;
  stRgaAttr.stImgOut.u32VirStride = video_height;
  ret = RK_MPI_RGA_CreateChn(1, &stRgaAttr);
  if (ret) 
  {
    printf("ERROR: create RGA[0:1] falied! ret=%d\n", ret);
    return -1;
  }

  VO_CHN_ATTR_S stVoAttr = {0};
  stVoAttr.pcDevNode = pcDevNode;
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

  MPP_CHN_S stSrcChn;
  MPP_CHN_S stDestChn;
  printf("Bind VI[0:1] to RGA[0:1]....\n");
  stSrcChn.enModId = RK_ID_VI;
  stSrcChn.s32DevId = s32CamId;
  stSrcChn.s32ChnId = 1;
  stDestChn.enModId = RK_ID_RGA;
  stDestChn.s32DevId = s32CamId;
  stDestChn.s32ChnId = 1;
  ret = RK_MPI_SYS_Bind(&stSrcChn, &stDestChn);
  if (ret) 
  {
    printf("ERROR: Bind VI[0:1] to RGA[0:1] failed! ret=%d\n", ret);
    return -1;
  }

  pthread_create(&rkmedia_vi_rockx_tidp, NULL, rkmedia_vi_rockx_thread, NULL);
  printf("%s initial finish\n", __func__);

  while (!quit) 
  {
    usleep(500000);
  }

  printf("%s exit!\n", __func__);
  printf("Unbind VI[0:1] to RGA[0:1]....\n");
  stSrcChn.enModId = RK_ID_VI;
  stSrcChn.s32DevId = s32CamId;
  stSrcChn.s32ChnId = 1;
  stDestChn.enModId = RK_ID_RGA;
  stSrcChn.s32DevId = s32CamId;
  stDestChn.s32ChnId = 1;
  ret = RK_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
  if (ret) 
  {
    printf("ERROR: unbind VI[0:1] to RGA[0:1] failed! ret=%d\n", ret);
    return -1;
  }

  printf("Destroy VI[0:1] channel\n");
  ret = RK_MPI_VI_DisableChn(s32CamId, 1);
  if (ret) 
  {
    printf("ERROR: destroy VI[0:1] error! ret=%d\n", ret);
    return -1;
  }

  printf("Destroy VO[0:0] channel\n");
  ret = RK_MPI_VO_DestroyChn(0);
  {
    printf("ERROR: destroy VO[0:0] error! ret=%d\n", ret);
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

    src_mb = RK_MPI_SYS_GetMediaBuffer(RK_ID_RGA, 1, -1);
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

      /*
      // 使用rockx的API来画矩形框和绘制文字
      rockx_image_draw_rect(&input_image, {left, top}, {right, bottom}, {255, 0, 0}, 2);
      rockx_image_draw_text(&input_image, show_str, {left, top-8}, {255, 0, 0}, 2);
      */
    }

    RK_MPI_SYS_SendMediaBuffer(RK_ID_VO, 0, src_mb);
    RK_MPI_MB_ReleaseBuffer(src_mb);
    src_mb = NULL;
  }
  
  rockx_destroy(object_det_handle);
  rockx_destroy(object_track_handle);
  return NULL;
}
