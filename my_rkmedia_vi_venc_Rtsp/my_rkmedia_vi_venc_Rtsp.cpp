// Copyright 2020 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "my_rkmedia_vi_venc_Rtsp.h"


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
  int clients = 0;
	std::string ip;
  if(get_local_ip(IF_NAME,&ip))
  {
    printf("func get_local_ip() error\r\n");
    exit(EXIT_FAILURE);
  }
  std::string port = "5544";
	std::string rtsp_url = "rtsp://" + ip + ":" + port + "/live";

	std::shared_ptr<xop::EventLoop> event_loop(new xop::EventLoop());  
	std::shared_ptr<xop::RtspServer> server = xop::RtspServer::Create(event_loop.get());
	if (!server->Start("0.0.0.0", 5544)) {
		return -1;
	}
	
#ifdef AUTH_CONFIG
	server->SetAuthConfig("-_-", "admin", "12345");
#endif
	 
	xop::MediaSession *session = xop::MediaSession::CreateNew("live"); // url: rtsp://ip/live
	session->AddSource(xop::channel_0, xop::H264Source::CreateNew()); 
	session->AddSource(xop::channel_1, xop::AACSource::CreateNew(44100,2));
	// session->startMulticast(); /* 开启组播(ip,端口随机生成), 默认使用 RTP_OVER_UDP, RTP_OVER_RTSP */

	session->AddNotifyConnectedCallback([] (xop::MediaSessionId sessionId, std::string peer_ip, uint16_t peer_port){
		printf("RTSP client connect, ip=%s, port=%hu \n", peer_ip.c_str(), peer_port);
	});
   
	session->AddNotifyDisconnectedCallback([](xop::MediaSessionId sessionId, std::string peer_ip, uint16_t peer_port) {
		printf("RTSP client disconnect, ip=%s, port=%hu \n", peer_ip.c_str(), peer_port);
	});

	std::cout << "URL: " << rtsp_url << std::endl;
        
	xop::MediaSessionId session_id = server->AddSession(session); 
	//server->removeMeidaSession(session_id); /* 取消会话, 接口线程安全 */
  


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
    venc_chn_attr.stRcAttr.stH264Cbr.u32Gop = 30;
    venc_chn_attr.stRcAttr.stH264Cbr.u32BitRate = video_width * video_height * 3;
    // frame rate: in 30/1, out 30/1.
    venc_chn_attr.stRcAttr.stH264Cbr.fr32DstFrameRateDen = 1;
    venc_chn_attr.stRcAttr.stH264Cbr.fr32DstFrameRateNum = 30;
    venc_chn_attr.stRcAttr.stH264Cbr.u32SrcFrameRateDen = 1;
    venc_chn_attr.stRcAttr.stH264Cbr.u32SrcFrameRateNum = 30;
    break;
  }

  venc_chn_attr.stVencAttr.imageType = IMAGE_TYPE_NV12;
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
  printf("Bind VI[0:0] to VENC[0:0]....\n");
  stSrcChn.enModId = RK_ID_VI;
  stSrcChn.s32DevId = s32CamId;
  stSrcChn.s32ChnId = 0;
  stDestChn.enModId = RK_ID_VENC;
  stSrcChn.s32DevId = s32CamId;
  stDestChn.s32ChnId = 0;
  ret = RK_MPI_SYS_Bind(&stSrcChn, &stDestChn);
  if (ret) 
  {
    printf("ERROR: bind VI[0:0] to VENC[0:0] failed! ret=%d\n", ret);
    return -1;
  }

  std::thread thread(SendFrameThread, server.get(), session_id, std::ref(clients));
	thread.detach();
  printf("%s initial finish\n", __func__);

  while (!quit)
  {
    usleep(500000);
  }

  printf("UnBind VI[0:0] to VENC[0:0]....\n");
  stSrcChn.enModId = RK_ID_VI;
  stSrcChn.s32DevId = s32CamId;
  stSrcChn.s32ChnId = 0;
  stDestChn.enModId = RK_ID_VENC;
  stSrcChn.s32DevId = s32CamId;
  stDestChn.s32ChnId = 0;
  ret = RK_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
  if (ret) 
  {
    printf("ERROR: unbind VI[0:0] to VENC[0:0] failed! ret=%d\n", ret);
    return -1;
  }

  printf("Destroy VENC[0:0] channel\n");
  ret = RK_MPI_VENC_DestroyChn(0);
  if (ret)
  {
    printf("ERROR: Destroy VENC[0:0] error! ret=%d\n", ret);
    return 0;
  }

  RK_MPI_VI_DisableChn(s32CamId, 0);
  
  if (iq_file_dir) 
  {
#if RKAIQ
    SAMPLE_COMM_ISP_Stop(s32CamId);
#endif
  }
  return 0;
}

void SendFrameThread(xop::RtspServer* rtsp_server, xop::MediaSessionId session_id, int& clients)
{       
  MEDIA_BUFFER mb = NULL;
	while(1)
	{
    mb = RK_MPI_SYS_GetMediaBuffer(RK_ID_VENC, 0, -1);
    if (!mb)
    {
      printf("ERROR: RK_MPI_SYS_GetMediaBuffer get null buffer!\n");
      break;
    }
    //获取一帧 H264, 打包
    xop::AVFrame videoFrame = {0};
    videoFrame.type = 0; // 建议确定帧类型。I帧(xop::VIDEO_FRAME_I) P帧(xop::VIDEO_FRAME_P)
    videoFrame.size = RK_MPI_MB_GetSize(mb);  // 视频帧大小 
    videoFrame.timestamp = xop::H264Source::GetTimestamp(); // 时间戳, 建议使用编码器提供的时间戳
    videoFrame.buffer.reset(new uint8_t[videoFrame.size]);                    
    memcpy(videoFrame.buffer.get(), (unsigned char *)RK_MPI_MB_GetPtr(mb), videoFrame.size);					
                
    rtsp_server->PushFrame(session_id, xop::channel_0, videoFrame); //送到服务器进行转发, 接口线程安全
    RK_MPI_MB_ReleaseBuffer(mb);
		//xop::Timer::Sleep(1); /* 实际使用需要根据帧率计算延时! */
	}
}

// 获取本机ip  eth_inf网卡名称 调用方法get_local_ip("apcli0", ip);
int get_local_ip(const char *eth_inf,std::string *ip)
{
    int sd;
    struct sockaddr_in sin;
    struct ifreq ifr;
 
    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == sd)
    {
        printf("socket error: %s\n", strerror(errno));
        return -1;
    }
 
    strncpy(ifr.ifr_name, eth_inf, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = 0;
 
    // if error: No such device  
    if (ioctl(sd, SIOCGIFADDR, &ifr) < 0)
    {
        printf("ioctl error: %s\n", strerror(errno));
        close(sd);
        return -1;
    }
    *ip = inet_ntoa(((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr);
    printf("interfac: %s, ip: %s\n", eth_inf, inet_ntoa(((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr)); 

    close(sd);
    return 0;
}