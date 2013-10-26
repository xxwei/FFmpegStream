#include "StdAfx.h"
#include "StreamConfig.h"

HRESULT	InitStreamCodec(STREAMCODEC_CONFIG *pscc,DWORD nVideoCodec,DWORD nAudioCodec)
{
	pscc->m_pVideoCodec = avcodec_find_decoder((CodecID)nVideoCodec);
	pscc->m_pCodecCtxVideo = avcodec_alloc_context3(pscc->m_pVideoCodec);
	avcodec_get_context_defaults3(pscc->m_pCodecCtxVideo,pscc->m_pVideoCodec);
	
	if(nVideoCodec==AV_CODEC_ID_H264||nVideoCodec==AV_CODEC_ID_H263||nVideoCodec==AV_CODEC_ID_MPEG2VIDEO||nVideoCodec==AV_CODEC_ID_WMV3||nVideoCodec==AV_CODEC_ID_VC1)
	{
		//pscc->m_pCodecCtxVideo->
		//va_dxva2_t	
	}
	if(avcodec_open(pscc->m_pCodecCtxVideo, pscc->m_pVideoCodec)<0) 
	{ 
		return S_FALSE;
	}
	pscc->m_pCodecCtxVideo->coded_width = 1920;
	pscc->m_pCodecCtxVideo->coded_height = 1080;
	pscc->m_pVideoFrame = avcodec_alloc_frame();
	pscc->m_pVideo2Frame = avcodec_alloc_frame();
	if(DEFAULT_PIXFORMAT==2)
	{
		pscc->m_pixelFormat = PIX_FMT_YUV420P;
	}
	else if(DEFAULT_PIXFORMAT==5)
	{
		pscc->m_pixelFormat = PIX_FMT_RGB32;
	}	
	//  //PIX_FMT_YUV420P   PIX_FMT_RGB32 PIX_FMT_BGR32
	pscc->m_pScreenBuffer = (unsigned char *)malloc(SCREEN_BUF_SIZE);
	pscc->m_pShowRect = NULL;
	pscc->m_pAudioCodec = avcodec_find_decoder((CodecID)nAudioCodec);
	pscc->m_pCodecCtxAudio = avcodec_alloc_context3(pscc->m_pAudioCodec);
	avcodec_get_context_defaults3(pscc->m_pCodecCtxAudio,pscc->m_pAudioCodec);
	pscc->m_pCodecCtxAudio->channels=1;
	//pscc->m_pCodecCtxAudio->sample_rate = 8000;
	//int ret = avcodec_open2(pscc->m_pCodecCtxAudio,pscc->m_pAudioCodec,NULL);
	if(avcodec_open(pscc->m_pCodecCtxAudio,pscc->m_pAudioCodec)<0) 
	{ 
		return S_FALSE;
	}
	pscc->m_pAudioFrame = avcodec_alloc_frame();
	pscc->m_pPCMBuffer = (unsigned char *)malloc(MAX_CACHE_SIZE);
	pscc->m_pcm_cpy_Event = ::CreateEvent(NULL,TRUE,FALSE,NULL);//默认安全级别，人工控制，初始为无信号，默认系统对象名
	pscc->m_pcm_dec_Event = ::CreateEvent(NULL,TRUE,TRUE,NULL);//默认安全级别，人工控制，初始为有信号，默认系统对象名
	return S_OK;
}
HRESULT CloseStreamCodec(STREAMCODEC_CONFIG *pscc)
{
	if(pscc->m_pCodecCtxAudio)
		avcodec_close(pscc->m_pCodecCtxAudio);
	pscc->m_pAudioCodec = NULL;
	if(pscc->m_pCodecCtxVideo)
		avcodec_close(pscc->m_pCodecCtxVideo);
	pscc->m_pVideoCodec = NULL;
	av_free(pscc->m_pAudioFrame);
		pscc->m_pAudioFrame = NULL;
	av_free(pscc->m_pVideoFrame);
		pscc->m_pVideoFrame = NULL;
	sws_freeContext(pscc->m_pSwsContextVideo);
		pscc->m_pSwsContextVideo=NULL;
	free(pscc->m_pScreenBuffer);
	pscc->m_pScreenBuffer = NULL;
	free(pscc->m_pPCMBuffer);
	pscc->m_pPCMBuffer = NULL;
	CloseHandle(pscc->m_pcm_cpy_Event);
	pscc->m_pcm_cpy_Event = NULL;
	CloseHandle(pscc->m_pcm_dec_Event);
	pscc->m_pcm_dec_Event = NULL;
	return S_OK;
}
HRESULT setCopyFiltering(STREAMCODEC_CONFIG *pscc,int b, int c, int s){
	// check values
	if ( !pscc->m_pCodecCtxVideo|| ( b < -100 && b > 100) || ( c < -100 && c > 100) || ( s < -100 && s > 100) )
		return S_FALSE;

	// Modify the sws converter used internally
	int *inv_table, srcrange, *table, dstrange, brightness, contrast, saturation;
	if ( -1 != sws_getColorspaceDetails(pscc->m_pSwsContextVideo, &inv_table, &srcrange, &table, &dstrange, &brightness, &contrast, &saturation) ) {
		// ok, can modify the converter
		brightness = (( b <<16) + 50)/100;
		contrast = ((( c +100)<<16) + 50)/100;
		saturation = ((( s +100)<<16) + 50)/100;
		sws_setColorspaceDetails(pscc->m_pSwsContextVideo, inv_table, srcrange, table, dstrange, brightness, contrast, saturation);
	}
	return S_OK;

}
HRESULT DeCodecVideo(STREAMCODEC_CONFIG *pscc,const unsigned char *pSrcData,const unsigned int dwDataLen,bool decode)
{
	AVPacket	packet;
	int	  bytesDecoded = 0;
	int   bytesRemaining=0;
	int   nGot=0;
	bytesRemaining =dwDataLen;
	av_init_packet(&packet);
	int	  loop = 0;
	while(bytesRemaining > 0)
	{
		//  解码下一块数据
		if(decode)
		{
			packet.data = (uint8_t *)pSrcData;
			packet.size = dwDataLen;
			__try 
			{
				bytesDecoded = avcodec_decode_video2(pscc->m_pCodecCtxVideo,pscc->m_pVideoFrame,&nGot,&packet);
			}
			__except(EXCEPTION_CONTINUE_SEARCH)
			{
				OutputDebugStringW(L"解码异常...........\n");
			}
			loop++;
			if(bytesDecoded < 0||loop>3)
			{
				//fprintf(stderr, "Error while decoding frame\n");
				bytesRemaining = 0;
				continue;
			}
			bytesRemaining-=bytesDecoded;
			pSrcData+=bytesDecoded;	
		}
		else
		{
			nGot = 200;
			bytesRemaining = 0;
		}
		if(nGot)
		{	 	
			if(pscc->m_pSwsContextVideo==NULL)
			{
				SwsFilter filter;
				filter.lumH = filter.lumV = filter.chrH = filter.chrV = sws_getGaussianVec(0.25, 3.0);
				sws_normalizeVec(filter.lumH, 1.0);
				pscc->m_pSwsContextVideo = sws_getContext(
					pscc->m_pCodecCtxVideo->width,
					pscc->m_pCodecCtxVideo->height,
					pscc->m_pCodecCtxVideo->pix_fmt,
					pscc->m_pCodecCtxVideo->width,
					pscc->m_pCodecCtxVideo->height,
					pscc->m_pixelFormat,
					SWS_POINT, NULL, NULL, NULL);
			}
			if(pscc->m_bUp2Down)
			{
				pscc->m_pVideoFrame->data[0] += pscc->m_pVideoFrame->linesize[0] * (pscc->m_pCodecCtxVideo->height - 1); 
				pscc->m_pVideoFrame->linesize[0] *= -1; 
				pscc->m_pVideoFrame->data[1] += pscc->m_pVideoFrame->linesize[1] * (pscc->m_pCodecCtxVideo->height / 2 - 1); 
				pscc->m_pVideoFrame->linesize[1] *= -1; 
				pscc->m_pVideoFrame->data[2] += pscc->m_pVideoFrame->linesize[2] * (pscc->m_pCodecCtxVideo->height / 2 - 1); 
				pscc->m_pVideoFrame->linesize[2] *= -1;
			}
			if(pscc->m_pCodecCtxVideo->pix_fmt!=pscc->m_pixelFormat)
			{
				setCopyFiltering(pscc,pscc->m_brightness,pscc->m_contrast,pscc->m_saturation);
				avpicture_fill((AVPicture *)pscc->m_pVideo2Frame, pscc->m_pScreenBuffer, pscc->m_pixelFormat, pscc->m_pCodecCtxVideo->width, pscc->m_pCodecCtxVideo->height);
				int h = sws_scale(pscc->m_pSwsContextVideo,pscc->m_pVideoFrame->data,pscc->m_pVideoFrame->linesize,0,pscc->m_pCodecCtxVideo->height,pscc->m_pVideo2Frame->data,pscc->m_pVideo2Frame->linesize);
				

				if(!decode)
					avpicture_fill((AVPicture *)pscc->m_pVideo2Frame, pscc->m_pScreenBuffer, pscc->m_pixelFormat, pscc->m_pCodecCtxVideo->width, pscc->m_pCodecCtxVideo->height);	
				else
					avpicture_layout((AVPicture *)pscc->m_pVideo2Frame,pscc->m_pixelFormat,pscc->m_pCodecCtxVideo->width, pscc->m_pCodecCtxVideo->height,pscc->m_pScreenBuffer,SCREEN_BUF_SIZE);
			}
			else
			{
				if(!decode)
					avpicture_fill((AVPicture *)pscc->m_pVideoFrame, pscc->m_pScreenBuffer, pscc->m_pixelFormat, pscc->m_pCodecCtxVideo->width, pscc->m_pCodecCtxVideo->height);	
				else
					avpicture_layout((AVPicture *)pscc->m_pVideoFrame,pscc->m_pixelFormat,pscc->m_pCodecCtxVideo->width, pscc->m_pCodecCtxVideo->height,pscc->m_pScreenBuffer,SCREEN_BUF_SIZE);

			}
			return S_OK;
		}
	}
	return S_FALSE;
}
HRESULT DeCodecAudio(STREAMCODEC_CONFIG *pscc,const unsigned char *pSrcData,const unsigned int dwDataLen,int64_t dts,bool decode)
{
	AVPacket	packet;
	av_init_packet(&packet);
	packet.data = (uint8_t *)pSrcData;
	packet.size = dwDataLen;
	packet.pts = dts-15;
	packet.dts = dts;
	if(pscc->m_newpcmoffset+AVCODEC_MAX_AUDIO_FRAME_SIZE>MAX_CACHE_SIZE)
	{
		return S_FALSE;
	}
	if(pscc->m_newpcmoffset-pscc->m_oldpcmoffset>BUFFERNOTIFYSIZE)//大于125毫秒的数据时触发  这样实时数据,仅在这个地方就会有125毫秒的延时
	{
		SetEvent(pscc->m_pcm_cpy_Event);
	}
	if(WaitForSingleObject(pscc->m_pcm_dec_Event,100)!=WAIT_OBJECT_0)
	{
		return S_FALSE;
	}
	while(packet.size>0)
	{
		//char str[128];
		//sprintf_s(str,"音频缓冲区数据量%d \n",pscc->m_newpcmoffset-pscc->m_oldpcmoffset);
		//OutputDebugStringA(str);
		int out_size = MAX_CACHE_SIZE - pscc->m_newpcmoffset;
		int len = avcodec_decode_audio3(pscc->m_pCodecCtxAudio, (short *)(pscc->m_pPCMBuffer+pscc->m_newpcmoffset), &out_size, &packet);
		if(len<=0)
		{
			//0xbebbb1b7
			return S_FALSE;
		}
		pscc->m_newpcmoffset+=out_size;
		//pscc+=out_size;
		packet.size-=len;
		packet.data+=len;
	}
	if(pscc->m_newpcmoffset-pscc->m_oldpcmoffset>BUFFERNOTIFYSIZE)//大于125毫秒的数据时触发  这样实时数据,仅在这个地方就会有125毫秒的延时
	{
		SetEvent(pscc->m_pcm_cpy_Event);
		ResetEvent(pscc->m_pcm_dec_Event);
	}

	return S_OK;
}