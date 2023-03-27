// Copyright 2020 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "atk_mobilenet_object_classification.h"


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
  SAMPLE_COMM_ISP_SetFrameRate(s32CamId,fps);
#endif
  }

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
  stRgaAttr_01.u16Rotaion = 270;
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

  VO_CHN_ATTR_S stVoAttr = {0};
  stVoAttr.pcDevNode = "/dev/dri/card0";
  stVoAttr.emPlaneType = VO_PLANE_OVERLAY;
  stVoAttr.enImgType = IMAGE_TYPE_RGB888;
  stVoAttr.u16Zpos = 0;
  stVoAttr.stImgRect.s32X = 0;
  stVoAttr.stImgRect.s32Y = 0;
  stVoAttr.stImgRect.u32Width = disp_height;
  stVoAttr.stImgRect.u32Height = disp_width;
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

  pthread_t rkmedia_rknn_tidp;
  pthread_create(&rkmedia_rknn_tidp, NULL, rkmedia_rknn_thread, NULL);

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

  RK_MPI_VO_DestroyChn(0);
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


static unsigned char *load_model(const char *filename, int *model_size)
{
  FILE *fp = fopen(filename, "rb");
  if (fp == NULL)
  {
    printf("fopen %s fail!\n", filename);
    return NULL;
  }

  fseek(fp, 0, SEEK_END);
  unsigned int model_len = ftell(fp);
  unsigned char *model = (unsigned char *)malloc(model_len);
  fseek(fp, 0, SEEK_SET);

  if (model_len != fread(model, 1, model_len, fp))
  {
    printf("fread %s fail!\n", filename);
    free(model);
    return NULL;
  }
  *model_size = model_len;

  if (fp)
  {
    fclose(fp);
  }
  return model;
}


static void printRKNNTensor(rknn_tensor_attr *attr)
{
  printf("index=%d name=%s n_dims=%d dims=[%d %d %d %d] n_elems=%d size=%d "
         "fmt=%d type=%d qnt_type=%d fl=%d zp=%d scale=%f\n",
         attr->index, attr->name, attr->n_dims, attr->dims[3], attr->dims[2],
         attr->dims[1], attr->dims[0], attr->n_elems, attr->size, 0, attr->type,
         attr->qnt_type, attr->fl, attr->zp, attr->scale);
}


int rgb24_resize(unsigned char *input_rgb, unsigned char *output_rgb, 
                 int width,int height, int outwidth, int outheight)
{
  rga_buffer_t src =wrapbuffer_virtualaddr(input_rgb, width, height, RK_FORMAT_RGB_888);
  rga_buffer_t dst = wrapbuffer_virtualaddr(output_rgb, outwidth, outheight,RK_FORMAT_RGB_888);
  rga_buffer_t pat = {0};
  im_rect src_rect = {0, 0, width, height};
  im_rect dst_rect = {0, 0, outwidth, outheight};
  im_rect pat_rect = {0};
  IM_STATUS STATUS = improcess(src, dst, pat, src_rect, dst_rect, pat_rect, 0);
  if (STATUS != IM_STATUS_SUCCESS)
  {
    printf("imcrop failed: %s\n", imStrError(STATUS));
    return -1;
  }
  return 0;
}


static int rknn_GetTop(float *pfProb, float *pfMaxProb,uint32_t *pMaxClass, uint32_t outputCount, uint32_t topNum)
{
  uint32_t i, j;
#define MAX_TOP_NUM 20
  if (topNum > MAX_TOP_NUM)
  {
    return 0;
  }
  
  memset(pfMaxProb, 0, sizeof(float) * topNum);
  memset(pMaxClass, 0xff, sizeof(float) * topNum);

  for (j = 0; j < topNum; j++)
  {
    for (i = 0; i < outputCount; i++)
    {
      if ((i == *(pMaxClass + 0)) || (i == *(pMaxClass + 1)) || (i == *(pMaxClass + 2)) ||
          (i == *(pMaxClass + 3)) || (i == *(pMaxClass + 4)))
      {
        continue;
      }

      if (pfProb[i] > *(pfMaxProb + j))
      {
        *(pfMaxProb + j) = pfProb[i];
        *(pMaxClass + j) = i;
      }
    }
  }

  return 1;
}


void *rkmedia_rknn_thread(void *args)
{
  pthread_detach(pthread_self());

  int ret;
  rknn_context ctx;
  int model_len = 0;
  unsigned char *model;
  static char *model_path = "/demo/bin/mobilenet_v1_rv1109_rv1126.rknn";

  // Load RKNN Model
  printf("Loading model ...\n");            
  model = load_model(model_path, &model_len);
  ret = rknn_init(&ctx, model, model_len, 0);
  if (ret < 0)
  {
    printf("rknn_init fail! ret=%d\n", ret);
    return NULL;
  }

  // Get Model Input Output Info
  rknn_input_output_num io_num;
  ret = rknn_query(ctx, RKNN_QUERY_IN_OUT_NUM, &io_num, sizeof(io_num));
  if (ret != RKNN_SUCC)
  {
    printf("rknn_query fail! ret=%d\n", ret);
    return NULL;
  }
  printf("model input num: %d, output num: %d\n", io_num.n_input,io_num.n_output);

  // print input tensor
  printf("input tensors:\n");
  rknn_tensor_attr input_attrs[io_num.n_input];
  memset(input_attrs, 0, sizeof(input_attrs));
  for (unsigned int i = 0; i < io_num.n_input; i++)
  {
    input_attrs[i].index = i;
    ret = rknn_query(ctx, RKNN_QUERY_INPUT_ATTR, &(input_attrs[i]),sizeof(rknn_tensor_attr));
    if (ret != RKNN_SUCC)
    {
      printf("rknn_query fail! ret=%d\n", ret);
      return NULL;
    }
    printRKNNTensor(&(input_attrs[i]));
  }

  // print output tensor
  printf("output tensors:\n");
  rknn_tensor_attr output_attrs[io_num.n_output];
  memset(output_attrs, 0, sizeof(output_attrs));
  for (unsigned int i = 0; i < io_num.n_output; i++)
  {
    output_attrs[i].index = i;
    ret = rknn_query(ctx, RKNN_QUERY_OUTPUT_ATTR, &(output_attrs[i]),sizeof(rknn_tensor_attr));
    if (ret != RKNN_SUCC)
    {
      printf("rknn_query fail! ret=%d\n", ret);
      return NULL;
    }
    printRKNNTensor(&(output_attrs[i]));
  }

  int model_height = 0;
  int model_width = 0;
  int model_channel = 0;
  switch (input_attrs->fmt)
  {
  case RKNN_TENSOR_NHWC:
      model_height = input_attrs->dims[2];
      model_width = input_attrs->dims[1];
      model_channel = input_attrs->dims[0];
      break;
  case RKNN_TENSOR_NCHW:
      model_height = input_attrs->dims[1];
      model_width = input_attrs->dims[0];
      model_channel = input_attrs->dims[2];
      break;
  default:
      printf("meet unsupported layout\n");
      return NULL;
  }
  printf("###w=%d,h=%d,c=%d, fmt=%d\n", model_width, model_height, model_channel, input_attrs->fmt);

  while (!quit)
  {
    MEDIA_BUFFER src_mb = NULL;
    src_mb = RK_MPI_SYS_GetMediaBuffer(RK_ID_RGA, 0, -1);
    if (!src_mb)
    {
      printf("ERROR: RK_MPI_SYS_GetMediaBuffer get null buffer!\n");
      break;
    }

    /*================================================================================
      =========================使用drm拷贝，可以使用如下代码===========================
      ================================================================================*/
    /*
    rga_context rga_ctx;
    drm_context drm_ctx;
    memset(&rga_ctx, 0, sizeof(rga_context));
    memset(&drm_ctx, 0, sizeof(drm_context));

     // DRM alloc buffer
    int drm_fd = -1;
    int buf_fd = -1; // converted from buffer handle
    unsigned int handle;
    size_t actual_size = 0;
    void *drm_buf = NULL;

    drm_fd = drm_init(&drm_ctx);
    drm_buf = drm_buf_alloc(&drm_ctx, drm_fd, video_width, video_height, 3 * 8, &buf_fd, &handle, &actual_size);
    memcpy(drm_buf, (uint8_t *)RK_MPI_MB_GetPtr(src_mb) , video_width * video_height * 3);
    void *resize_buf = malloc(model_width * model_height * 3);
    // init rga context
    RGA_init(&rga_ctx);
    img_resize_slow(&rga_ctx, drm_buf, video_width, video_height, resize_buf, model_width, model_height);
    uint32_t input_model_image_size = model_width * model_height * 3;

    // Set Input Data
    rknn_input inputs[1];
    memset(inputs, 0, sizeof(inputs));
    inputs[0].index = 0;
    inputs[0].type = RKNN_TENSOR_UINT8;
    inputs[0].size = input_model_image_size;
    inputs[0].fmt = RKNN_TENSOR_NHWC;
    inputs[0].buf = resize_buf;
    ret = rknn_inputs_set(ctx, io_num.n_input, inputs);
    if (ret < 0)
    {
      printf("ERROR: rknn_inputs_set fail! ret=%d\n", ret);
      return NULL;
    }
    free(resize_buf);
    drm_buf_destroy(&drm_ctx, drm_fd, buf_fd, handle, drm_buf, actual_size);
    drm_deinit(&drm_ctx, drm_fd);
    RGA_deinit(&rga_ctx);
    */

    /*================================================================================
      =========================不使用drm拷贝，可以使用如下代码===========================
      ================================================================================*/
    
    unsigned char *orig_image_buf = (unsigned char *)RK_MPI_MB_GetPtr(src_mb);

    if (video_width != model_width || video_height != model_height)
    {
      uint32_t input_model_image_size = model_width * model_height * 3;
      unsigned char *input_model_image_buf = (unsigned char *)malloc(input_model_image_size);
      rgb24_resize(orig_image_buf, input_model_image_buf, video_width, video_height, model_width, model_height);

      // Set Input Data
      rknn_input inputs[1];
      memset(inputs, 0, sizeof(inputs));
      inputs[0].index = 0;
      inputs[0].type = RKNN_TENSOR_UINT8;
      inputs[0].size = input_model_image_size;
      inputs[0].fmt = RKNN_TENSOR_NHWC;
      inputs[0].buf = input_model_image_buf;
      ret = rknn_inputs_set(ctx, io_num.n_input, inputs);
      if (ret < 0)
      {
        printf("ERROR: rknn_inputs_set fail! ret=%d\n", ret);
        return NULL;
      }
      free(input_model_image_buf);
    }
    

    // Run
    printf("rknn_run\n");
    ret = rknn_run(ctx, nullptr);
    if (ret < 0)
    {
      printf("ERROR: rknn_run fail! ret=%d\n", ret);
      return NULL;
    }

    // Get Output
    rknn_output outputs[io_num.n_output];
    memset(outputs, 0, sizeof(outputs));
    for (unsigned int i = 0; i < io_num.n_output; i++)
    {
        outputs[i].want_float = 1;
    }
    ret = rknn_outputs_get(ctx, io_num.n_output, outputs, NULL);
    if (ret < 0)
    {
      printf("ERROR: rknn_outputs_get fail! ret=%d\n", ret);
      return NULL;
    }

    // Post Process
    for (unsigned int i = 0; i < io_num.n_output; i++)
    {
      uint32_t MaxClass[5];
      float fMaxProb[5];
      float *buffer = (float *)outputs[i].buf;
      uint32_t sz = outputs[i].size / 4;

      rknn_GetTop(buffer, fMaxProb, MaxClass, sz, 5);
      printf(" --- Top5 ---\n");
      for (int i = 0; i < 5; i++)
      {
        printf("%3d: %8.6f\n", MaxClass[i], fMaxProb[i]);

        // 采用opencv来绘制矩形框,颜色格式是B、G、R
        using namespace cv;
        Mat orig_img = Mat(video_height, video_width, CV_8UC3, RK_MPI_MB_GetPtr(src_mb));

        std::string index = std::to_string(MaxClass[i]-1);
        putText(orig_img, index, Point(32, 64+i*48), FONT_HERSHEY_TRIPLEX, 2, Scalar(0,0,255),2,8,0);

        std::string prob = std::to_string(fMaxProb[i]);
        putText(orig_img, prob, Point(260, 64+i*48), FONT_HERSHEY_TRIPLEX, 2, Scalar(0,0,255),2,8,0);
      }
    }

    rknn_outputs_release(ctx, io_num.n_output, outputs);

    rga_buffer_t src , dst ;
    MB_IMAGE_INFO_S dst_ImageInfo = {(RK_U32)disp_height, (RK_U32)disp_width, (RK_U32)disp_height, 
                                     (RK_U32)disp_width, IMAGE_TYPE_RGB888};
    MEDIA_BUFFER dst_mb = RK_MPI_MB_CreateImageBuffer(&dst_ImageInfo, RK_TRUE, 0);
    dst = wrapbuffer_fd(RK_MPI_MB_GetFD(dst_mb), disp_height, disp_width,RK_FORMAT_RGB_888);
    src = wrapbuffer_fd(RK_MPI_MB_GetFD(src_mb), video_width, video_height,RK_FORMAT_RGB_888);
    
    im_rect src_rect , dst_rect;
    src_rect = {0, 0, 1280, 720};
    dst_rect = {0};
    ret = imcheck(src, dst, src_rect, dst_rect, IM_CROP);
    if (IM_STATUS_NOERROR != ret) 
    {
      printf("%d, check error! %s", __LINE__, imStrError((IM_STATUS)ret));
      break;
    }

    IM_STATUS CROP_STATUS = imcrop(src, dst, src_rect);
    if (CROP_STATUS != IM_STATUS_SUCCESS)
    {
      printf("ERROR: imcrop failed: %s\n", imStrError(CROP_STATUS));
    }

    RK_MPI_SYS_SendMediaBuffer(RK_ID_VO, 0, dst_mb);
    RK_MPI_MB_ReleaseBuffer(dst_mb);
    RK_MPI_MB_ReleaseBuffer(src_mb);
  
    src_mb = NULL;
    dst_mb= NULL;
  }

  if (model)
  {
    free(model);
  }

  if (ctx >= 0)
  {
    rknn_destroy(ctx);
  }
    
  return NULL;
}

