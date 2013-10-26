// GentekPlatformStream.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "StreamConfig.h"
#include "GentekPlatformStream.h"

extern	unsigned long	G_port;
stream_map		G_map;


PSTREAMCONFIG GetStream(unsigned long nPort)
{
	if(G_map.find(nPort)!=G_map.end())
	{
		return G_map.find(nPort)->second;
	}
	else
	{
		return NULL;
	}
}
unsigned long	GS_GetVersion()
{
	return 0x00000001;
}
unsigned long	GS_GetLastError(unsigned long nPort)
{
	PSTREAMCONFIG pm = GetStream(nPort);
	if(pm==NULL)
	{
		return GENTEK_NOT_FIND_PORT;
	}
	else
	{
		return pm->nLastError;
	}
}
//播放流程接口
unsigned long	GS_InitStream(unsigned long nVideoCodec,unsigned long nAudioCodec,unsigned long *nPort)
{
	PSTREAMCONFIG psc = new _STREAM_CONFIG();
	psc->nVideoCodec  = nVideoCodec;
	psc->nAudioCodec = nAudioCodec;
	psc->nTimebase = 0;
	psc->nLastError = 0;
	if(nAudioCodec!=CODEC_TYPE_NONE)
		psc->pAudioConfig = new AUDIOPLAY_CONFIG();
	else
		psc->pAudioConfig = NULL;
	if(nVideoCodec!=CODEC_TYPE_NONE)
		psc->pVideoConfig = new VIDEOPLAY_CONFIG();
	else
		psc->pVideoConfig  = NULL;
	psc->pCodecConfig = new STREAMCODEC_CONFIG();
	psc->pContrlConfig = new PLAY_SYNC_CONTRL_CONFIG();
	psc->nStreamFormat = NULL;
	psc->pContrlConfig->fOnStreamOver = NULL;
	psc->pContrlConfig->fOnTime = NULL;
	G_port++;
	*nPort = G_port;
	G_map.insert(map<unsigned long,PSTREAMCONFIG>::value_type(G_port,psc));
	char str[128];
	sprintf(str,"初始化GentekPlatformStream %d\n",nPort);
	OutputDebugStringA(str);
	return S_OK;
}
unsigned long	GS_OpenStream(unsigned long nPort,unsigned long nStreamFormat,unsigned long nMode)
{
	//打开编解码器 内存分配
	PSTREAMCONFIG pm = GetStream(nPort);
	if(pm==NULL)
	{
		return S_FALSE;
	}
	pm->nStreamFormat = nStreamFormat;
	if(OpenStream(nStreamFormat,pm->pContrlConfig)==S_FALSE)
	{
		pm->nLastError = GENTEK_NOT_SUPPORT_STREAM;
		return S_FALSE;
	}
	if(InitStreamCodec(pm->pCodecConfig,pm->nVideoCodec,pm->nAudioCodec)==S_FALSE)
	{
		pm->nLastError = GENTEK_OPEN_CODEC_FAILED;
		return S_FALSE;
	}
	char str[128];
	sprintf(str," Open GentekPlatformStream %d\n",nPort);
	OutputDebugStringA(str);
	return S_OK;
}
unsigned long	GS_OpenFile(unsigned long nPort,CHAR *pFileName,bool bRead)
{
	PSTREAMCONFIG pm = GetStream(nPort);
	if(pm==NULL)
	{
		return S_FALSE;
	}
	pm->pCodecConfig->m_pStreamFormatContext = avformat_alloc_context();
	pm->pCodecConfig->m_pStreamFormatContext->oformat = av_guess_format(NULL,pFileName,NULL);
	pm->pCodecConfig->m_pStreamFormatContext->oformat->audio_codec = (CodecID)pm->nAudioCodec;
	pm->pCodecConfig->m_pStreamFormatContext->oformat->video_codec = (CodecID)pm->nVideoCodec;
	strcpy_s(pm->pCodecConfig->m_pStreamFormatContext->filename,strlen(pFileName),pFileName);
	//pm->m_video_st = av_new_stream(pm->m_pFormatContext,0);
	pm->nLastError = GENTEK_NOT_IMPLEMENT;
	return S_FALSE;
}
unsigned long	GS_PlayStream(unsigned long nPort,HWND hWnd)
{
	//初始化渲染设备,开启线程
	PSTREAMCONFIG pm = GetStream(nPort);
	if(pm==NULL)
	{
		return S_FALSE;
	}
	if(pm->pVideoConfig)
	{
		//初始化视频渲染设备
		pm->pVideoConfig->m_hWnd = hWnd;
		if(InitDirectDraw(pm->pVideoConfig,1920,1080)==S_FALSE)
		{
			pm->nLastError = GENTEK_DDRAW_INIT_FAILED;
			if(InitGDIPlus(pm->pVideoConfig)==S_FALSE)
			{
				pm->nLastError = GENTEK_VIDEO_RENDER_FAILED;
				return S_FALSE;
			}
			else
			{
				pm->pVideoConfig->m_bUsingGDIPLUS = true;
			}
		}
		else
		{
			pm->pVideoConfig->m_bUsingGDIPLUS = false;
		}
	}
	if(pm->pAudioConfig)
	{
		//为初始化音频做准备
		pm->pAudioConfig->m_hwnd = pm->pVideoConfig->m_hWnd;
		//初始化音频设备
		if(InitDirectSound(pm->pAudioConfig)==S_FALSE)
		{
			pm->nLastError = GENTEK_AUDIO_RENDER_FAILED;
			return S_FALSE;
		}

	}
	//创建线程
	pm->pContrlConfig->nPort = nPort;
	pm->pContrlConfig->bDataOver = false;
	pm->pContrlConfig->bCallStreamOver = false;
	if(PlayStream(pm->pContrlConfig,pm->pCodecConfig,pm->pVideoConfig,pm->pAudioConfig)==S_OK)
	{
		char str[128];
		sprintf(str," Play GentekPlatformStream %d\n",nPort);
		OutputDebugStringA(str);
		return S_OK;
	}

	pm->nLastError = GENTEK_THREAD_CREAT_FAILED;
	return S_FALSE;
}
unsigned long	GS_StopStream(unsigned long nPort)
{
	//恢复渲染,停止线程
	PSTREAMCONFIG pm = GetStream(nPort);
	if(pm==NULL)
	{
		return S_FALSE;
	}
	if(StopStream(pm->pContrlConfig)==S_OK)
	{
		if(pm->pAudioConfig)
			UninitDirectSound(pm->pAudioConfig);
		if(pm->pVideoConfig->m_bUsingGDIPLUS)
		{
			UnInitGDIPlus(pm->pVideoConfig);
		}
		else
		{
			UnInitDirectDraw(pm->pVideoConfig);
		}
	}
	char str[128];
	sprintf(str," Stop GentekPlatformStream %d\n",nPort);
	OutputDebugStringA(str);
	return S_OK;
}
unsigned long	GS_CloseStream(unsigned long nPort)
{
	//关闭编解码器
	PSTREAMCONFIG pm = GetStream(nPort);
	if(pm==NULL)
	{
		return S_FALSE;
	}
	CloseStream(pm->pContrlConfig);
	CloseStreamCodec(pm->pCodecConfig);
	char str[128];
	sprintf(str," Close GentekPlatformStream %d\n",nPort);
	OutputDebugStringA(str);
	return S_OK;
}
unsigned long	GS_CloseFile(unsigned long nPort)
{
	PSTREAMCONFIG pm = GetStream(nPort);
	if(pm==NULL)
	{
		return S_FALSE;
	}
	pm->nLastError = GENTEK_NOT_IMPLEMENT;
	return S_FALSE;
}
unsigned long	GS_UnInitStream(unsigned long nPort)
{
	//删除对应port
	PSTREAMCONFIG pm = GetStream(nPort);
	if(pm==NULL)
	{
		return S_FALSE;
	}
	if(pm->pAudioConfig)
		delete pm->pAudioConfig;
	pm->pAudioConfig = NULL;
	if(pm->pVideoConfig)
		delete pm->pVideoConfig;
	pm->pVideoConfig = NULL;
	if(pm->pCodecConfig)
		delete pm->pCodecConfig;
	pm->pCodecConfig = NULL;
	if(pm->pContrlConfig)
		delete pm->pContrlConfig;
	pm->pContrlConfig = NULL;
	delete pm;
	G_map.erase(nPort);
	char str[128];
	sprintf(str," 反初始化 GentekPlatformStream %d\n",nPort);
	OutputDebugStringA(str);
	return S_OK;
}
//播放功能接口
unsigned long	GS_PauseStream(unsigned long nPort)
{
	//线程暂停 ,清空音频播放缓冲区
	PSTREAMCONFIG pm = GetStream(nPort);
	if(pm==NULL)
	{
		return S_FALSE;
	}
	if(PauseStream(pm->pContrlConfig))
	{
		return S_OK;
	}
	return 0;
}
unsigned long	GS_SpeedStream(unsigned long nPort,unsigned long nSpeed)
{
	//非正常速度取消声音播放
	PSTREAMCONFIG pm = GetStream(nPort);
	if(pm==NULL)
	{
		return S_FALSE;
	}
	if(PlayStreamBySpeed(pm->pContrlConfig,nSpeed))
	{

	}
	return 0;
}
unsigned long	GS_PlayStreamOneByOne(unsigned long nPort)
{
	//非正常速度取消声音播放
	PSTREAMCONFIG pm = GetStream(nPort);
	if(pm==NULL)
	{
		return S_FALSE;
	}
	if(PlayVideoOneByOne(pm->pContrlConfig))
	{

	}
	return 0;
}
unsigned long	GS_SetStreamOverCallBack(unsigned long nPort,bool (CALLBACK *fOnStreamOver)(unsigned long nPort))
{
	PSTREAMCONFIG pm = GetStream(nPort);
	if(pm==NULL)
	{
		return S_FALSE;
	}
	pm->pContrlConfig->fOnStreamOver = fOnStreamOver;
	return S_OK;
}
unsigned long	GS_StartListenStreamOverCallBack(unsigned long nPort)
{
	PSTREAMCONFIG pm = GetStream(nPort);
	if(pm==NULL)
	{
		return S_FALSE;
	}
	pm->pContrlConfig->bDataOver = true;
	return S_OK;
}
unsigned long	GS_SetTimeNotifyCallBack(unsigned long nPort,bool (CALLBACK *fOnTime)(unsigned long nPort,unsigned __int64 nTime))
{
	PSTREAMCONFIG pm = GetStream(nPort);
	if(pm==NULL)
	{
		return S_FALSE;
	}
	pm->pContrlConfig->fOnTime = fOnTime;
	return S_OK;
}

//数据接收接口
unsigned long	GS_InputPSStream(unsigned long nPort,unsigned char * pBuf,unsigned long nSize)
{
	PSTREAMCONFIG pm = GetStream(nPort);
	if(pm==NULL)
	{
		return S_FALSE;
	}
	if(pm->nStreamFormat!=STREAMF_PS)
	{
		pm->nLastError = GENTEK_STREAM_FORMAT_NOT_MATCH;
		return S_FALSE;
	}
	if(pm->pContrlConfig->nStatus!=G_STATUS_PLAY&&pm->pContrlConfig->nStatus!=G_STATUS_PAUSE)
	{
		pm->nLastError = GENTEK_STATUS_ERROR;
		return S_FALSE;
	}
	if(pm->pContrlConfig->pb_new_offset+nSize>MAX_CACHE_SIZE)
	{
		pm->nLastError = GENTEK_CAHCE_FULL;
		return S_FALSE;
	}
	EnterCriticalSection(&pm->pContrlConfig->cs_pool_first);
	//OutputDebugStringA("收到原始数据块\n");
	memcpy(pm->pContrlConfig->pPacketBuffer+pm->pContrlConfig->pb_new_offset,pBuf,nSize);
	pm->pContrlConfig->pb_new_offset+=nSize;
	LeaveCriticalSection(&pm->pContrlConfig->cs_pool_first);
	return S_OK;
}
unsigned long	GS_InputTSStream(unsigned long nPort,unsigned char * pBuf,unsigned long nSize)
{
	char str[128];
	sprintf(str,"准备插入数据 %d\n",nPort);
	//OutputDebugStringA(str);
	PSTREAMCONFIG pm = GetStream(nPort);
	if(pm==NULL)
	{
		sprintf(str,"没有找到播放通道 %d\n",nPort);
		OutputDebugStringA(str);
		return S_FALSE;
	}
	if(pm->nStreamFormat!=STREAMF_TS)
	{
		sprintf(str,"流格式不符 %d\n",nPort);
		OutputDebugStringA(str);
		pm->nLastError = GENTEK_STREAM_FORMAT_NOT_MATCH;
		return S_FALSE;
	}
	if(pm->pContrlConfig->nStatus!=G_STATUS_PLAY&&pm->pContrlConfig->nStatus!=G_STATUS_PAUSE)
	{
		pm->nLastError = GENTEK_STATUS_ERROR;
		return S_FALSE;
	}
	if(pm->pContrlConfig->pb_new_offset+nSize>MAX_CACHE_SIZE)
	{
		sprintf(str,"缓冲区已经满了 %d\n",nPort);
		//OutputDebugStringA(str);
		pm->nLastError = GENTEK_CAHCE_FULL;
		return S_FALSE;
	}
	EnterCriticalSection(&pm->pContrlConfig->cs_pool_first);
	sprintf(str,"开始复制数据%d--数据偏移量%d---数据长度%d\n",nPort,pm->pContrlConfig->pb_new_offset,nSize);
	//OutputDebugStringA(str);
	memcpy(pm->pContrlConfig->pPacketBuffer+pm->pContrlConfig->pb_new_offset,pBuf,nSize);
	pm->pContrlConfig->pb_new_offset+=nSize;
	sprintf(str,"数据复制完成 %d\n",nPort);
	//OutputDebugStringA(str);
	LeaveCriticalSection(&pm->pContrlConfig->cs_pool_first);
	return S_OK;
}
unsigned long	GS_InputESStream(unsigned long nPort,unsigned char * pBuf,unsigned long nSize,unsigned long nStreamType,unsigned __int64 nTime/*us*/,bool HaveStartCode)
{
	PSTREAMCONFIG pm = GetStream(nPort);
	if(pm==NULL)
	{
		return S_FALSE;
	}
	if(pm->nStreamFormat!=STREAMF_ES&&pm->nStreamFormat!=STREAMF_ES_ONLYVIDEO)
	{
		pm->nLastError = GENTEK_STREAM_FORMAT_NOT_MATCH;
		return S_FALSE;
	}
	if(pm->pContrlConfig->nStatus!=G_STATUS_PLAY&&pm->pContrlConfig->nStatus!=G_STATUS_PAUSE)
	{
		pm->nLastError = GENTEK_STATUS_ERROR;
		return S_FALSE;
	}
	if (pm->nStreamFormat==STREAMF_ES_ONLYVIDEO)
	{
		if(pm->pContrlConfig->pb_new_offset+nSize>MAX_CACHE_SIZE)
		{
			pm->nLastError = GENTEK_CAHCE_FULL;
			return S_FALSE;
		}
		EnterCriticalSection(&pm->pContrlConfig->cs_pool_first);
		if(HaveStartCode)
		{
			memcpy(pm->pContrlConfig->pPacketBuffer+pm->pContrlConfig->pb_new_offset,pBuf,nSize);
			pm->pContrlConfig->pb_new_offset+=nSize;
		}
		else
		{
			pm->pContrlConfig->pPacketBuffer[pm->pContrlConfig->pb_new_offset]=0;
			pm->pContrlConfig->pPacketBuffer[pm->pContrlConfig->pb_new_offset+1]=0;
			pm->pContrlConfig->pPacketBuffer[pm->pContrlConfig->pb_new_offset+2]=0;
			pm->pContrlConfig->pPacketBuffer[pm->pContrlConfig->pb_new_offset+3]=1;
			memcpy(pm->pContrlConfig->pPacketBuffer+pm->pContrlConfig->pb_new_offset+4,pBuf,nSize);
			pm->pContrlConfig->pb_new_offset+=nSize;
			pm->pContrlConfig->pb_new_offset+=4;
		}
		
		LeaveCriticalSection(&pm->pContrlConfig->cs_pool_first);
		return S_OK;
	}
	//这里要直接复制到二级缓冲区中
	if(nStreamType==STREAMT_VIDEO)
	{
		if(MAX_CACHE_SIZE-pm->pContrlConfig->vb_new_offset<nSize)
		{	
			pm->nLastError = GENTEK_CAHCE_FULL;
			return S_FALSE;
		}
		return AddVideoPacket(pm->pContrlConfig,(char *)pBuf,nSize,nTime,HaveStartCode);
	}
	else if(nStreamType==STREAMT_AUDIO)
	{
		if(MAX_CACHE_SIZE-pm->pContrlConfig->ab_new_offset<nSize)
		{	
			pm->nLastError = GENTEK_CAHCE_FULL;
			return S_FALSE;
		}
		return AddAudioPacket(pm->pContrlConfig,(char *)pBuf,nSize,nTime);
	}
	return S_FALSE;
}
unsigned long	GS_InputPSOverRTPStream(unsigned long nPort,unsigned char * pBuf,unsigned long nSize)
{
	PSTREAMCONFIG pm = GetStream(nPort);
	if(pm==NULL)
	{
		return S_FALSE;
	}
	if(pm->nStreamFormat!=STREAMF_PSOVERRTP)
	{
		pm->nLastError = GENTEK_STREAM_FORMAT_NOT_MATCH;
		return S_FALSE;
	}
	if(pm->pContrlConfig->nStatus!=G_STATUS_PLAY&&pm->pContrlConfig->nStatus!=G_STATUS_PAUSE)
	{
		pm->nLastError = GENTEK_STATUS_ERROR;
		return S_FALSE;
	}
	//这里的RTP信息对流来说 无关紧要
	EnterCriticalSection(&pm->pContrlConfig->cs_pool_first);
	if(pm->pContrlConfig->pb_new_offset+nSize>MAX_CACHE_SIZE)
	{
		pm->nLastError = GENTEK_CAHCE_FULL;
		LeaveCriticalSection(&pm->pContrlConfig->cs_pool_first);
		return S_FALSE;
	}
	memcpy(pm->pContrlConfig->pPacketBuffer+pm->pContrlConfig->pb_new_offset,pBuf,nSize);
	pm->pContrlConfig->pb_new_offset+=nSize;
	LeaveCriticalSection(&pm->pContrlConfig->cs_pool_first);
	return S_OK;
}
unsigned long	GS_WriteFile(unsigned long nPort,unsigned char * pBuf,unsigned long nSize)
{
	PSTREAMCONFIG pm = GetStream(nPort);
	if(pm==NULL)
	{
		return S_FALSE;
	}
	pm->nLastError = GENTEK_NOT_IMPLEMENT;
	return S_FALSE;
}
unsigned long	GS_ShowVideoUpDown(unsigned long nPort,bool bUpDown)
{
	PSTREAMCONFIG pm = GetStream(nPort);
	if(pm==NULL)
	{
		return S_FALSE;
	}
	pm->pCodecConfig->m_bUp2Down = bUpDown;
	return S_OK;
}

unsigned long	GS_SetShowSrcPos(unsigned long nPort,int x,int y,int sx,int sy,bool bshow)
{
	PSTREAMCONFIG pm = GetStream(nPort);
	if(pm==NULL)
	{
		return S_FALSE;
	}
	if(bshow)
	{
		pm->pCodecConfig->m_pShowRect = NULL;
	}
	else
	{
		RECT rc;
		::GetClientRect(pm->pVideoConfig->m_hWnd,&rc);
		int nx,ny,nsx,nsy;
		nx = x*pm->pCodecConfig->m_pCodecCtxVideo->width/(rc.right-rc.left);
		ny = y*pm->pCodecConfig->m_pCodecCtxVideo->height/(rc.bottom-rc.top);
		nsx = sx*pm->pCodecConfig->m_pCodecCtxVideo->width/(rc.right-rc.left);
		nsy = sy*pm->pCodecConfig->m_pCodecCtxVideo->height/(rc.bottom-rc.top);
		if(nsx>=nsy)
		{
			if(nsy*16/9<nsx)
			{
				nsy = nsx*9/16;
			}
			if(ny+nsy>pm->pCodecConfig->m_pCodecCtxVideo->height)
			{
				return S_FALSE;
			}
		}
		else
		{
			nsx = nsy*4/3;
			if(nx+nsx>pm->pCodecConfig->m_pCodecCtxVideo->width)
			{
				return S_FALSE;
			}
		}
		pm->pCodecConfig->m_ShowRect.top = ny;
		pm->pCodecConfig->m_ShowRect.left = nx;
		pm->pCodecConfig->m_ShowRect.right = nx+nsx;
		pm->pCodecConfig->m_ShowRect.bottom = ny+nsy;
		pm->pCodecConfig->m_pShowRect = &pm->pCodecConfig->m_ShowRect;
	}
	return S_OK;
}
unsigned long	GS_SetAudioVolmue(unsigned long nPort,unsigned long nVolume)
{

	PSTREAMCONFIG pm = GetStream(nPort);
	if(pm==NULL)
	{
		return S_FALSE;
	}
	return SetVolume(pm->pAudioConfig,nVolume);
}
unsigned long	GS_GetAudioVolmue(unsigned long nPort,unsigned long *pnVolume)
{
	PSTREAMCONFIG pm = GetStream(nPort);
	if(pm==NULL)
	{
		return S_FALSE;
	}
	return GetVolume(pm->pAudioConfig,(LONG *)pnVolume);
}
unsigned long	GS_CloseAudio(unsigned long nPort)
{
	PSTREAMCONFIG pm = GetStream(nPort);
	if(pm==NULL)
	{
		return S_FALSE;
	}
	return CloseAudio(pm->pContrlConfig);
}
unsigned long	GS_OpenAudio(unsigned long nPort)
{
	PSTREAMCONFIG pm = GetStream(nPort);
	if(pm==NULL)
	{
		return S_FALSE;
	}
	return ReSetAudioPlay(pm->pContrlConfig);
}
unsigned long	GS_TakePic(unsigned long nPort,unsigned char *pFileName)
{
	PSTREAMCONFIG pm = GetStream(nPort);
	if(pm==NULL)
	{
		return S_FALSE;
	}
	if(pm->pContrlConfig->m_bTakePic)
	{
		pm->nLastError = GENTEK_CALLED_FAST;
		return S_FALSE;
	}
	pm->pContrlConfig->m_bTakePic = true;
	memset(pm->pContrlConfig->m_pTakePicFileName,0,512);
	strcpy((char *)pm->pContrlConfig->m_pTakePicFileName,(const char *)pFileName);

	/*
	AVFrame	*pVideo2Frame =avcodec_alloc_frame();;
	uint8_t	*buffer = (uint8_t *)malloc(1920*1080*4);
	SwsContext *ctx = sws_getContext(
		pm->pContrlConfig->pCodecConfig->m_pCodecCtxVideo->width,
		pm->pContrlConfig->pCodecConfig->m_pCodecCtxVideo->height,
		pm->pContrlConfig->pCodecConfig->m_pCodecCtxVideo->pix_fmt,
		pm->pContrlConfig->pCodecConfig->m_pCodecCtxVideo->width,
		pm->pContrlConfig->pCodecConfig->m_pCodecCtxVideo->height,
		PIX_FMT_BGR24,
		SWS_FAST_BILINEAR, NULL, NULL, NULL);

	avpicture_fill((AVPicture *)pVideo2Frame, buffer, PIX_FMT_BGR24,pm->pContrlConfig->pCodecConfig->m_pCodecCtxVideo->width,pm->pContrlConfig->pCodecConfig->m_pCodecCtxVideo->height);
	sws_scale(ctx,pm->pContrlConfig->pCodecConfig->m_pVideoFrame->data,pm->pContrlConfig->pCodecConfig->m_pVideoFrame->linesize,0,pm->pContrlConfig->pCodecConfig->m_pCodecCtxVideo->height,pVideo2Frame->data,pVideo2Frame->linesize);		

	
	CxImage	*lpImage = new CxImage();
	lpImage->CreateFromArray(buffer,pm->pContrlConfig->pCodecConfig->m_pCodecCtxVideo->width,pm->pContrlConfig->pCodecConfig->m_pCodecCtxVideo->height,24,3*pm->pContrlConfig->pCodecConfig->m_pCodecCtxVideo->width,true);
	bool ret = lpImage->Save((const TCHAR *)pFileName, CXIMAGE_FORMAT_JPG);
	delete lpImage;
	sws_freeContext(ctx);
	av_free(pVideo2Frame);
	free(buffer);
	buffer = NULL;
	*/

	return S_OK;
}
unsigned long	GS_SetText(unsigned long nPort,unsigned char * pText,int x,int y,int font_size,unsigned int nIndex,bool bShow)
{
	PSTREAMCONFIG pm = GetStream(nPort);
	if(pm==NULL)
	{
		return S_FALSE;
	}
	pm->nLastError = GENTEK_NOT_IMPLEMENT;
	return S_FALSE;
}