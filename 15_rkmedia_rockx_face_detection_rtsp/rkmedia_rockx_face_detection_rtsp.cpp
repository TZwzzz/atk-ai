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

#include "rkmedia_rockx_face_detection_rtsp.h"

int main(int argc, char *argv[]) 
{
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


void *rkmedia_vi_rockx_thread(void *args) 
{
  pthread_detach(pthread_self());//将线程状态改为unjoinable状态，确保资源的释放

  rockx_ret_t ret;
  rockx_handle_t face_det_handle;
  rockx_handle_t face_5landmarks_handle;
  char *rockx_data = "/demo/src/rockx_data";
  rockx_config_t *config = rockx_create_config();
  rockx_add_config(config, ROCKX_CONFIG_DATA_PATH, rockx_data);

  ret = rockx_create(&face_det_handle, ROCKX_MODULE_FACE_DETECTION, config,sizeof(rockx_config_t));
  if (ret != ROCKX_RET_SUCCESS) 
  {
    printf("ERROR: init rockx module ROCKX_MODULE_FACE_DETECTION error %d\n", ret);
  }

  ret = rockx_create(&face_5landmarks_handle,ROCKX_MODULE_FACE_LANDMARK_5, config, sizeof(rockx_config_t));
  if (ret != ROCKX_RET_SUCCESS) 
  {
    printf("ERROR: init rockx module ROCKX_MODULE_FACE_LANDMARK_5 error %d\n",ret);
  }

  while (!quit) 
  {
    MEDIA_BUFFER src_mb = NULL;

    src_mb = RK_MPI_SYS_GetMediaBuffer(RK_ID_RGA, 0, -1);
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
    input_image.pixel_format = ROCKX_PIXEL_FORMAT_BGR888;
    input_image.size = RK_MPI_MB_GetSize(src_mb);
    input_image.data = (uint8_t *)RK_MPI_MB_GetPtr(src_mb);

    rockx_object_array_t face_array;
    memset(&face_array, 0, sizeof(face_array));
    ret =rockx_face_detect(face_det_handle, &input_image, &face_array, nullptr);
    if (ret != ROCKX_RET_SUCCESS) 
    {
      printf("ERROR: rockx_face_detect error %d\n", ret);
    }

    if (face_array.count > 0) 
    {
      for (int i = 0; i < face_array.count; i++) 
      {
        int is_false_face;
        ret = rockx_face_filter(face_5landmarks_handle, &input_image,&face_array.object[i].box, &is_false_face);
        if (ret != ROCKX_RET_SUCCESS) 
        {
          printf("ERROR: rockx_face_filter error %d\n", ret);
        }
        if (is_false_face)
        {
          continue;
        }
       
        int left = face_array.object[i].box.left;
        int top = face_array.object[i].box.top;
        int right = face_array.object[i].box.right;
        int bottom = face_array.object[i].box.bottom;
        float score = face_array.object[i].score;
        printf("box=(left,top,right,bottom)=(%d %d %d %d)\n", left, top, right, bottom);
        int w = face_array.object[i].box.right - face_array.object[i].box.left;
        int h = face_array.object[i].box.bottom - face_array.object[i].box.top;
        printf("w=right-left=%d\n", w);
        printf("h=right-left=%d\n", h);
        printf("score=%f\n\n", score);

        if (left < 0)
        {
          left = 0;
        }
        if (top < 0)
        { 
          top = 0;
        }
        
        while ((uint32_t)(left + w) >= input_image.width) 
        {
          w -= 16;
        }
        while ((uint32_t)(top + h) >= input_image.height) 
        {
          h -= 16;
        }

        /********************* 以下绘制矩形框和文字的代码可以选择任意一种********************/
        
        // nv12_border()用于在YUV格式（非彩色）的图像上画框，框的颜色为红色(R,G,B=255,0,0)
        // nv12_border((char *)input_image.data,input_image.width,input_image.height,left,top, w, h, 255, 0, 0);
        
        Mat show_img = Mat(input_image.height, input_image.width, CV_8UC3,RK_MPI_MB_GetPtr(src_mb));
        // 采用opencv来绘制矩形框,颜色格式是B、G、R
        cv::rectangle(show_img,cv::Point(left, top),cv::Point(right, bottom),cv::Scalar(255,0,0),3,8,0);
        std::string get_score = std::to_string(face_array.object[i].score);
        // 采用opencv在框的旁边绘制文字（人脸识别得分）,颜色格式是B、G、R
        cv::putText(show_img, get_score, cv::Point(left, top-16), cv::FONT_HERSHEY_TRIPLEX, 2, cv::Scalar(255,0,0),3,8,0);
        show_img.release();
      }
    }

    RK_MPI_SYS_SendMediaBuffer(RK_ID_VENC, 0, src_mb);
    RK_MPI_MB_ReleaseBuffer(src_mb);
    src_mb = NULL;
  }
  
  rockx_destroy(face_det_handle);
  rockx_destroy(face_5landmarks_handle);
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