#include "StdAfx.h"
#include "StreamConfig.h"
#include "GentekPlatformStream.h"

#include "dumuxer/PS.hpp"
#include "dumuxer/TS.hpp"
#include "dumuxer/NALUH264.hpp"
#include "dumuxer/PSOverRTP.hpp"
unsigned __int64 GetCurrentUsTime(PLAY_SYNC_CONTRL_CONFIG *lp)
{
	LARGE_INTEGER	CurrentTick;
	QueryPerformanceCounter(&CurrentTick);
	return 	CurrentTick.QuadPart / lp->nTickPerUs;
}

HRESULT OpenStream(DWORD nFormatStream,PLAY_SYNC_CONTRL_CONFIG *pscc)
{
	if(nFormatStream>STREAMF_PSOVERRTP)
	{
		return S_FALSE;
	}
	pscc->nStatus = G_STATUS_OPEN;
	pscc->pPacketBuffer = (unsigned char *)malloc(MAX_CACHE_SIZE);
	pscc->pAudioBuffer = (unsigned char *)malloc(MAX_CACHE_SIZE);
	pscc->pVideoBuffer = (unsigned char *)malloc(MAX_CACHE_SIZE);
	pscc->pb_new_offset = 0;
	pscc->pb_old_offset = 0;
	pscc->ab_old_offset = 0;
	pscc->ab_new_offset = 0;
	pscc->vb_new_offset = 0;
	pscc->vb_old_offset = 0;
	
	InitializeCriticalSection(&pscc->cs_pool_sec_video);
	InitializeCriticalSection(&pscc->cs_pool_sec_audio);
	InitializeCriticalSection(&pscc->cs_pool_first);
	InitializeCriticalSection(&pscc->cs_list_sec_audio);
	InitializeCriticalSection(&pscc->cs_list_sec_video);

	if(nFormatStream==STREAMF_PS)
	{
		pscc->m_pdemuxer = new ps_read();
	}
	else if(nFormatStream==STREAMF_TS)
	{
		pscc->m_pdemuxer = new ts_read();
	}
	else if(nFormatStream==STREAMF_ES_ONLYVIDEO)
	{
		pscc->m_pdemuxer = new nalu_h264_es();
	}
	else if(nFormatStream==STREAMF_PSOVERRTP)
	{
		pscc->m_pdemuxer = new ps_over_rtp();
	}
	else 
	{
		pscc->m_pdemuxer = NULL;
	}

	return S_OK;
}
HRESULT PlayStream(PLAY_SYNC_CONTRL_CONFIG *pscc,STREAMCODEC_CONFIG *pcc,VIDEOPLAY_CONFIG *pvc,AUDIOPLAY_CONFIG *pac)
{
	pscc->pCodecConfig = pcc;
	pscc->pVideoConfig = pvc;
	pscc->pAudioConfig = pac;
	pscc->m_bCloseAudio = false;
	pscc->m_video_eoi.pltime = 0;
	pscc->m_video_eoi.epos = 0;
	pscc->m_audio_eoi.pltime = 0;
	pscc->m_audio_eoi.epos = 0;
	pscc->m_bTakePic = false;
	QueryPerformanceFrequency(&pscc->nTickPerSec);
	pscc->nTickPerUs = pscc->nTickPerSec.QuadPart/1000000;
	pscc->m_bDemuxRun = true;
	pscc->h_demuxer_thread = CreateThread(NULL,NULL,DemuxerThread,pscc,NULL,NULL);
	if(pscc->pVideoConfig)
	{
		pscc->m_bVideoRun = true;
		pscc->h_video_decode_thread = CreateThread(NULL,NULL,PlayVideoThread,pscc,NULL,NULL);
	}
	if(pscc->pAudioConfig)
	{
		pscc->m_bAudioRun = true;
		pscc->h_audio_decode_thread = CreateThread(NULL,NULL,PlayAudioThread,pscc,NULL,NULL);
		pscc->h_audio_pcm_thread = CreateThread(NULL,NULL,PlayPCMThread,pscc,NULL,NULL);		
	}
	pscc->bplay_onebyone  = false;
	pscc->nplay_speed = 3;  //常速播放
	pscc->nStatus = G_STATUS_PLAY;
	pscc->m_pause_n_frame = 0;
	OutputDebugStringA("线程模块初始化完成\n");
	return S_OK;
}
HRESULT PauseStream(PLAY_SYNC_CONTRL_CONFIG *pscc)
{
	if(pscc->nStatus==G_STATUS_PLAY)
	{
		pscc->nplay_speed+=100;
		pscc->m_pause_n_frame = 0;
		pscc->pAudioConfig->m_pDSBuffer->Stop();
		//SuspendThread(pscc->h_demuxer_thread);
		//SuspendThread(pscc->h_video_decode_thread);
		OutputDebugStringA("准备挂起声音线程\n");
		SuspendThread(pscc->h_audio_decode_thread);
		OutputDebugStringA("已经挂起声音线程\n");
		pscc->nStatus = G_STATUS_PAUSE;
		
	}
	else if(pscc->nStatus==G_STATUS_PAUSE)
	{
		pscc->nplay_speed-=100;
		//ResumeThread(pscc->h_demuxer_thread);
		//ResumeThread(pscc->h_video_decode_thread);
		OutputDebugStringA("回复挂起声音线程\n");
		ResumeThread(pscc->h_audio_decode_thread);
		OutputDebugStringA("已经回复声音线程\n");
		pscc->pAudioConfig->m_pDSBuffer->Play(0,0,DSBPLAY_LOOPING);
		pscc->nStatus = G_STATUS_PLAY;
		pscc->bplay_onebyone = false;
		pscc->m_pause_n_frame = 0;
	}

	return S_OK;
}
HRESULT StopStream(PLAY_SYNC_CONTRL_CONFIG *pscc)
{
	if(pscc->nStatus==G_STATUS_PLAY||pscc->nStatus==G_STATUS_PAUSE)
	{
		pscc->m_bDemuxRun = false;
		pscc->m_bVideoRun = false;
		pscc->m_bAudioRun = false;
		if(pscc->h_demuxer_thread&&WaitForSingleObject(pscc->h_demuxer_thread,500)!=WAIT_OBJECT_0)
		{
			DWORD	exitThread=0;
			DWORD	dwExitCode;
			GetExitCodeThread(pscc->h_demuxer_thread,&dwExitCode);
			if(dwExitCode == STILL_ACTIVE)
				TerminateThread(pscc->h_demuxer_thread,0); //阻塞在	
		}
		if(pscc->h_video_decode_thread&&WaitForSingleObject(pscc->h_video_decode_thread,500)!=WAIT_OBJECT_0)
		{
			DWORD	exitThread=0;
			DWORD	dwExitCode;
			GetExitCodeThread(pscc->h_demuxer_thread,&dwExitCode);
			if(dwExitCode == STILL_ACTIVE)
				TerminateThread(pscc->h_video_decode_thread,0); //阻塞在	
		}
		if(pscc->h_audio_decode_thread&&WaitForSingleObject(pscc->h_audio_decode_thread,500)!=WAIT_OBJECT_0)
		{
			DWORD	exitThread=0;
			DWORD	dwExitCode;
			GetExitCodeThread(pscc->h_audio_decode_thread,&dwExitCode);
			if(dwExitCode == STILL_ACTIVE)
				TerminateThread(pscc->h_audio_decode_thread,0); //阻塞在	
		}
		if(pscc->h_audio_pcm_thread&&WaitForSingleObject(pscc->h_audio_pcm_thread,500)!=WAIT_OBJECT_0)
		{
			DWORD	exitThread=0;
			DWORD	dwExitCode;
			GetExitCodeThread(pscc->h_audio_pcm_thread,&dwExitCode);
			if(dwExitCode == STILL_ACTIVE)
				TerminateThread(pscc->h_audio_pcm_thread,0); //阻塞在	
		}
	}
	pscc->nStatus = G_STATUS_STOP;
	return S_OK;
}
HRESULT CloseStream(PLAY_SYNC_CONTRL_CONFIG *pscc)
{
	if(pscc->nStatus = G_STATUS_STOP)
	{
		DeleteCriticalSection(&pscc->cs_pool_sec_video);
		DeleteCriticalSection(&pscc->cs_pool_sec_audio);
		DeleteCriticalSection(&pscc->cs_pool_first);
		DeleteCriticalSection(&pscc->cs_list_sec_audio);
		DeleteCriticalSection(&pscc->cs_list_sec_video);
		delete pscc->m_pdemuxer;
		pscc->m_pdemuxer = NULL;
		free(pscc->pPacketBuffer);
		pscc->pPacketBuffer = NULL;
		free(pscc->pAudioBuffer);
		pscc->pAudioBuffer = NULL;
		free(pscc->pVideoBuffer);
		pscc->pVideoBuffer = NULL;
	}
	pscc->nStatus = G_STATUS_CLOSE;
	return S_OK;

}
HRESULT PlayVideoOneByOne(PLAY_SYNC_CONTRL_CONFIG *pscc)
{
	if(pscc->nStatus!=G_STATUS_PAUSE)
	{
		return S_FALSE;
	}
	pscc->bplay_onebyone = true;
	pscc->m_pause_n_frame = 2;
	return S_OK;
}
HRESULT PlayStreamBySpeed(PLAY_SYNC_CONTRL_CONFIG *pscc,unsigned int nSpeed)
{
	pscc->nplay_speed = nSpeed;
	char	str[128];
	sprintf_s(str,"调节播放速度%d",nSpeed);
	OutputDebugStringA(str);
	if(nSpeed!=3&&pscc->pAudioConfig)
	{
		pscc->pAudioConfig->m_pDSBuffer->Stop();
	}
	else
	{
		ReSetAudioPlay(pscc);
	}
	return S_OK;
}
HRESULT FreshSurface(PLAY_SYNC_CONTRL_CONFIG *pscc)
{

	if(pscc->pCodecConfig->m_pixelFormat==PIX_FMT_RGB32&&pscc->pCodecConfig->m_pCodecCtxVideo->pix_fmt!=pscc->pCodecConfig->m_pixelFormat)
	{
		FreshRGBSurface(pscc->pVideoConfig,pscc->pCodecConfig->m_pVideo2Frame->data,pscc->pCodecConfig->m_pVideo2Frame->linesize,pscc->pCodecConfig->m_pScreenBuffer,pscc->pCodecConfig->m_pShowRect);
	}
	else if(pscc->pCodecConfig->m_pixelFormat==PIX_FMT_RGB32&&pscc->pCodecConfig->m_pCodecCtxVideo->pix_fmt==pscc->pCodecConfig->m_pixelFormat)
	{
		FreshRGBSurface(pscc->pVideoConfig,pscc->pCodecConfig->m_pVideoFrame->data,pscc->pCodecConfig->m_pVideoFrame->linesize,pscc->pCodecConfig->m_pScreenBuffer,pscc->pCodecConfig->m_pShowRect);
	}
	else if(pscc->pCodecConfig->m_pixelFormat==PIX_FMT_YUV420P&&pscc->pCodecConfig->m_pCodecCtxVideo->pix_fmt==pscc->pCodecConfig->m_pixelFormat)
	{
		FreshYUVSurface(pscc->pVideoConfig,pscc->pCodecConfig->m_pVideoFrame->data,pscc->pCodecConfig->m_pVideoFrame->linesize,pscc->pCodecConfig->m_pShowRect);
	}
	else if(pscc->pCodecConfig->m_pixelFormat==PIX_FMT_YUV420P&&pscc->pCodecConfig->m_pCodecCtxVideo->pix_fmt!=pscc->pCodecConfig->m_pixelFormat)
	{
		FreshYUVSurface(pscc->pVideoConfig,pscc->pCodecConfig->m_pVideo2Frame->data,pscc->pCodecConfig->m_pVideo2Frame->linesize,pscc->pCodecConfig->m_pShowRect);
	}
	return S_OK;
}
HRESULT	AddVideoPacket(PLAY_SYNC_CONTRL_CONFIG *pscc,char *buf,unsigned int buflen,__int64 time,bool HaveStartCode)
{
	offsetidx		eoi;
	if(TryEnterCriticalSection(&pscc->cs_list_sec_video))
	{
		//复制数据到二级缓冲区 并添加到索引去
		eoi.spos = pscc->vb_new_offset;
		if(HaveStartCode)
		{
			memcpy(pscc->pVideoBuffer+pscc->vb_new_offset,buf,buflen);
			pscc->vb_new_offset = pscc->vb_new_offset+buflen;
			eoi.epos = pscc->vb_new_offset;
		}
		else
		{
			pscc->pVideoBuffer[pscc->vb_new_offset]=0;
			pscc->pVideoBuffer[pscc->vb_new_offset+1]=0;
			pscc->pVideoBuffer[pscc->vb_new_offset+2]=0;
			pscc->pVideoBuffer[pscc->vb_new_offset+3]=1;
			memcpy(pscc->pVideoBuffer+pscc->vb_new_offset+4,buf,buflen);
			pscc->vb_new_offset = pscc->vb_new_offset+buflen+4;
			eoi.epos = pscc->vb_new_offset;
		}
		eoi.pltime = time;
		pscc->vidx.push_back(eoi);
		LeaveCriticalSection(&pscc->cs_list_sec_video);
		return S_OK;
	}
	return S_FALSE;

}
HRESULT AddAudioPacket(PLAY_SYNC_CONTRL_CONFIG *pscc,char *buf,unsigned int buflen,__int64 time)
{
	if(pscc->nplay_speed==3&&pscc->m_bCloseAudio==false)
	{
		offsetidx		aoi;
		if(TryEnterCriticalSection(&pscc->cs_list_sec_audio))
		{	
			//复制数据到二级缓冲区 并添加到索引去
			aoi.spos = pscc->ab_new_offset;
			memcpy(pscc->pAudioBuffer+pscc->ab_new_offset,buf,buflen);
			pscc->ab_new_offset = pscc->ab_new_offset+buflen;
			aoi.epos = pscc->ab_new_offset;
			aoi.pltime = time;
			pscc->aidx.push_back(aoi);
			if(pscc->stream_time_base==0)
			{
				pscc->stream_time_base = time;
			}
			LeaveCriticalSection(&pscc->cs_list_sec_audio);
			return S_OK;
		}
		
	}
	return S_FALSE;
}
DWORD WINAPI DemuxerThread(LPVOID lp)
{
	PLAY_SYNC_CONTRL_CONFIG *pscc = (PLAY_SYNC_CONTRL_CONFIG *)lp;
	offsetidx		oi;
	offsetidx		eoi;
	offsetidx		aoi;
	unsigned int	read_offset;
	bool			b_video_header = false;
	bool			b_audio_header = false;
	while(pscc->m_bDemuxRun&&pscc->m_pdemuxer)
	{
		//char str[128];
		//sprintf_s(str,"缓冲区数据位置--%d \n",pscc->pb_new_offset);
		//OutputDebugStringA(str);
		int ret = pscc->m_pdemuxer->read_packet(pscc->pPacketBuffer+pscc->pb_old_offset,pscc->pb_new_offset-pscc->pb_old_offset,oi,&read_offset);
		if(false)
		{
			//缓冲区满跳转这里,这里还要考虑一个缓冲区满而另一个是空的则需要将满的缓冲区清空
MEMFULL:
			Sleep(10);
		}
		if(ret!=0&&ret!=10)
		{
			//char str[128];
			//sprintf_s(str,"数据包类型 %d 时间%I64d  缓冲区位置 %d 数据类型 %d\n",ret,oi.pltime,pscc->pb_old_offset,(pscc->pPacketBuffer[oi.spos+4]&0x1F));
			//OutputDebugStringA(str);
		}
		if(ret==0)
		{
			pscc->pb_old_offset+=read_offset;
			Sleep(10);
			//OutputDebugStringA("分离器线程空闲\n");
		}
		else if(ret==1)
		{
			if(pscc->nplay_speed==3&&pscc->m_bCloseAudio==false)
			{			
				if(MAX_CACHE_SIZE-pscc->ab_new_offset<oi.epos-oi.spos)
					goto MEMFULL;
				EnterCriticalSection(&pscc->cs_list_sec_audio);
				//pscc->aidx.push_back(oi);
				//复制数据到二级缓冲区 并添加到索引去
				aoi.spos = pscc->ab_new_offset;
				memcpy(pscc->pAudioBuffer+pscc->ab_new_offset,pscc->pPacketBuffer+pscc->pb_old_offset+oi.spos,oi.epos-oi.spos);
				pscc->ab_new_offset = pscc->ab_new_offset+oi.epos-oi.spos;
				aoi.epos = pscc->ab_new_offset;
				aoi.pltime = oi.pltime;
				pscc->aidx.push_back(aoi);
				if(pscc->stream_time_base==0)
				{
					pscc->stream_time_base = oi.pltime;
				}
				LeaveCriticalSection(&pscc->cs_list_sec_audio);
			}
			pscc->pb_old_offset+=read_offset;
			//char str[128];
			//sprintf_s(str,"-------一加入一个数据包%d  %d %d \n",aoi.spos,aoi.epos,read_offset);
			//OutputDebugStringA(str);
		}
		else if(ret==2)
		{
			if(MAX_CACHE_SIZE-pscc->vb_new_offset<oi.epos-oi.spos)
				goto MEMFULL;
			EnterCriticalSection(&pscc->cs_list_sec_video);
			//pscc->vidx.push_back(oi);
			//复制数据到二级缓冲区 并添加到索引去
			//这里有可能是连包 sps pps 帧数据打包到一起了
			eoi.spos = pscc->vb_new_offset;
			memcpy(pscc->pVideoBuffer+pscc->vb_new_offset,pscc->pPacketBuffer+pscc->pb_old_offset+oi.spos,oi.epos-oi.spos);
			pscc->vb_new_offset = pscc->vb_new_offset+oi.epos-oi.spos;
			
			
			eoi.epos = pscc->vb_new_offset;
			/*
			int i= eoi.spos;
			int d = eoi.epos;
			int nidx = 0;
			int idx[3] = {0};
			while(i<eoi.epos&&nidx<3)
			{
				unsigned int ret = ReadInt32(pscc->pVideoBuffer+i);
				if(ret==1)
				{
					idx[nidx] = i;
					nidx++;
				}
				i++;
			}
			int j = 0;
			if(nidx>1)
			{
				while(j<nidx-1)
				{
					eoi.spos = idx[j];
					eoi.epos = idx[j+1];
					eoi.pltime = oi.pltime;
					pscc->vidx.push_back(eoi);
					j++;
				}
			}

			eoi.spos = idx[j];
			eoi.epos = d;
			*/
			eoi.pltime = oi.pltime;
			pscc->vidx.push_back(eoi);


			LeaveCriticalSection(&pscc->cs_list_sec_video);
			pscc->pb_old_offset+=read_offset;
			//char str[128];
			//int d = pscc->pVideoBuffer[eoi.spos+4]&0x1F;
			//sprintf_s(str,"------2加入一个数据包长度 %d  %d\n",eoi.epos-eoi.spos,d);
			//OutputDebugStringA(str);
		}
		else if(ret==3)
		{
			if(MAX_CACHE_SIZE-pscc->ab_new_offset<oi.epos-oi.spos)
				goto MEMFULL;
			EnterCriticalSection(&pscc->cs_list_sec_audio);
			////复制数据到二级缓冲区 
			aoi.spos = pscc->ab_new_offset;
			memcpy(pscc->pAudioBuffer+pscc->ab_new_offset,pscc->pPacketBuffer+pscc->pb_old_offset+oi.spos,oi.epos-oi.spos);
			pscc->ab_new_offset = pscc->ab_new_offset+oi.epos-oi.spos;
			LeaveCriticalSection(&pscc->cs_list_sec_audio);
			pscc->pb_old_offset+=read_offset;
			b_audio_header = true;
		}
		else if(ret==4)
		{
			if(MAX_CACHE_SIZE-pscc->ab_new_offset<oi.epos-oi.spos)
				goto MEMFULL;
			EnterCriticalSection(&pscc->cs_list_sec_audio);
			////复制数据到二级缓冲区
			memcpy(pscc->pAudioBuffer+pscc->ab_new_offset,pscc->pPacketBuffer+pscc->pb_old_offset+oi.spos,oi.epos-oi.spos);
			pscc->ab_new_offset = pscc->ab_new_offset+oi.epos-oi.spos;
			LeaveCriticalSection(&pscc->cs_list_sec_audio);
			pscc->pb_old_offset+=read_offset;
		}
		else if(ret==5)
		{
			if(MAX_CACHE_SIZE-pscc->ab_new_offset<oi.epos-oi.spos)
				goto MEMFULL;
			EnterCriticalSection(&pscc->cs_list_sec_audio);
			////复制数据到二级缓冲区
			memcpy(pscc->pAudioBuffer+pscc->ab_new_offset,pscc->pPacketBuffer+pscc->pb_old_offset+oi.spos,oi.epos-oi.spos);
			pscc->ab_new_offset = pscc->ab_new_offset+oi.epos-oi.spos;
			aoi.epos = pscc->ab_new_offset;
			aoi.pltime = oi.pltime;
			if(b_audio_header)
				pscc->aidx.push_back(aoi);
			LeaveCriticalSection(&pscc->cs_list_sec_audio);
			pscc->pb_old_offset+=read_offset;
			b_audio_header = false;
		}
		else if(ret==6)
		{
			if(MAX_CACHE_SIZE-pscc->vb_new_offset<oi.epos-oi.spos)
				goto MEMFULL;
			EnterCriticalSection(&pscc->cs_list_sec_video);
			//复制数据到二级缓冲区 并添加到索引去
			eoi.spos = pscc->vb_new_offset;
			memcpy(pscc->pVideoBuffer+pscc->vb_new_offset,pscc->pPacketBuffer+pscc->pb_old_offset+oi.spos,oi.epos-oi.spos);
			pscc->vb_new_offset = pscc->vb_new_offset+oi.epos-oi.spos;
			LeaveCriticalSection(&pscc->cs_list_sec_video);
			pscc->pb_old_offset+=read_offset;
			b_video_header = true;
		}
		else if(ret==7)
		{
			if(MAX_CACHE_SIZE-pscc->vb_new_offset<oi.epos-oi.spos)
				goto MEMFULL;
			EnterCriticalSection(&pscc->cs_list_sec_video);
			memcpy(pscc->pVideoBuffer+pscc->vb_new_offset,pscc->pPacketBuffer+pscc->pb_old_offset+oi.spos,oi.epos-oi.spos);
			pscc->vb_new_offset = pscc->vb_new_offset+oi.epos-oi.spos;
			LeaveCriticalSection(&pscc->cs_list_sec_video);
			pscc->pb_old_offset+=read_offset;
		}
		else if(ret==8)
		{
			if(MAX_CACHE_SIZE-pscc->vb_new_offset<oi.epos-oi.spos)
				goto MEMFULL;
			EnterCriticalSection(&pscc->cs_list_sec_video);
			memcpy(pscc->pVideoBuffer+pscc->vb_new_offset,pscc->pPacketBuffer+pscc->pb_old_offset+oi.spos,oi.epos-oi.spos);
			pscc->vb_new_offset = pscc->vb_new_offset+oi.epos-oi.spos;
			eoi.epos = pscc->vb_new_offset;
			eoi.pltime = oi.pltime;
			if(b_video_header)
				pscc->vidx.push_back(eoi);
			LeaveCriticalSection(&pscc->cs_list_sec_video);
			pscc->pb_old_offset+=read_offset;
			b_video_header = false;
			//char str[128];
			//sprintf_s(str,"-------一加入一个数据包%d  %d\n",eoi.spos,eoi.epos);
			//OutputDebugStringA(str);
		}
		else if(ret==9)
		{
			char str[128];
			sprintf_s(str,"-------一读取数据包异常%d  %d\n",pscc->pb_new_offset,pscc->pb_old_offset);
			OutputDebugStringA(str);
			EnterCriticalSection(&pscc->cs_list_sec_video);
			pscc->vidx.clear();
			pscc->pb_new_offset = pscc->pb_old_offset;
			LeaveCriticalSection(&pscc->cs_list_sec_video);
			Sleep(100);
		}
		else if(ret==10)
		{
			pscc->pb_old_offset+=read_offset;
		}
		
		//一级缓冲移动
		if(pscc->pb_old_offset>MAX_CACHE_SIZE/2)
		{
			//OutputDebugStringA("一级缓冲开始移动\n");
			EnterCriticalSection(&pscc->cs_pool_first);
			memmove(pscc->pPacketBuffer,pscc->pPacketBuffer+pscc->pb_old_offset,pscc->pb_new_offset-pscc->pb_old_offset);
			pscc->pb_new_offset = pscc->pb_new_offset-pscc->pb_old_offset;
			pscc->pb_old_offset = 0;
			LeaveCriticalSection(&pscc->cs_pool_first);
			//OutputDebugStringA("一级缓冲移动完成\n");
			
		}
	}
	return S_OK;
}
DWORD WINAPI PlayVideoThread(LPVOID lp)
{
	PLAY_SYNC_CONTRL_CONFIG *pscc = (PLAY_SYNC_CONTRL_CONFIG *)lp;
	offsetidx		oi;
	idxlist::iterator	item;
	bool			bPause = false;
	while(pscc->m_bVideoRun)
	{
		//char str[128];
		//sprintf_s(str,"视频数据包数量 %d\n",pscc->vidx.size());
		//OutputDebugStringA(str);
		if(pscc->vidx.size()>1)
		{
			bPause = false;
			item = pscc->vidx.begin();
			oi.epos = item->epos;
			oi.spos = item->spos;
			oi.pltime = item->pltime;
			//char str[128];
			//sprintf_s(str,"视频数据包数量 %d 当前包的播放时间%I64d \n",pscc->vidx.size(),oi.pltime);
			//OutputDebugStringA(str);
			if(pscc->bplay_onebyone==FALSE)
			{	
				if(pscc->nplay_speed<=5)
				{
					if(pscc->nplay_speed==3)
					{
						//检查播放时间是否到了,没到就继续等待 非变速模式
						if(pscc->stream_time_base==0) //还没有找到音频数据的时间
						{
							Sleep(35);
						}
						else
						{
							if(pscc->stream_time_base<oi.pltime)
							{
								//等待
								Sleep(1);
								continue;
							}
							if(pscc->fOnTime)
								pscc->fOnTime(pscc->nPort,oi.pltime);
						}
					}
					else if (pscc->nplay_speed==5)
					{
						//Sleep(1);
					}
					else if(pscc->nplay_speed==4)
					{
						Sleep(10);
					}
					else if(pscc->nplay_speed==2)
					{
						Sleep(50);
					}
					else if(pscc->nplay_speed==1)
					{
						Sleep(100);
					}
					EnterCriticalSection(&pscc->cs_list_sec_video);
					pscc->vidx.pop_front();
					LeaveCriticalSection(&pscc->cs_list_sec_video);
				}
				else
				{
					//暂停中  为了可以单帧播放 和暂停截图 
					Sleep(100);
					bPause = true;
				}

			}
			else
			{
				if(pscc->m_pause_n_frame>0)
				{
					EnterCriticalSection(&pscc->cs_list_sec_video);
					pscc->vidx.pop_front();
					LeaveCriticalSection(&pscc->cs_list_sec_video);
					pscc->m_pause_n_frame--;
				}
				else
				{
					Sleep(100);
					bPause = true;
				}
			}
	
			//char str[128];
			//sprintf_s(str,"************准备解码 数据包长度为 %d--%d-----%d\n",oi.spos,oi.epos,oi.epos-oi.spos);
			//OutputDebugStringA(str);
			//解码ES数据
			HRESULT	ret = S_OK;
			if(bPause==false) //暂停状态是只渲染不解码
			{
				ret = DeCodecVideo(pscc->pCodecConfig,pscc->pVideoBuffer+oi.spos,oi.epos-oi.spos,true);
			}
			if(ret==S_OK)
			{
				if(pscc->pVideoConfig->m_bUsingGDIPLUS==false)
				{	
					if(pscc->pCodecConfig->m_pCodecCtxVideo->width!=pscc->pVideoConfig->m_nWidth)
					{
						ReInitDirectDraw(pscc->pVideoConfig,pscc->pCodecConfig->m_pCodecCtxVideo->width,pscc->pCodecConfig->m_pCodecCtxVideo->height);
					}
					FreshSurface(pscc);					
				}
				else
				{
					PrintRGB(pscc->pVideoConfig,pscc->pCodecConfig->m_pVideo2Frame->data,pscc->pCodecConfig->m_pVideo2Frame->linesize,pscc->pCodecConfig->m_pScreenBuffer);
				}
				//判断是否有截图需求

				Take_Pic(pscc);

			}
			

			pscc->vb_old_offset=oi.epos;
			//char str[128];
			//sprintf_s(str,"视频缓冲区偏移量 %d--%d\n",pscc->vb_new_offset,pscc->vb_old_offset);
			//OutputDebugStringA(str);
			//二级视频缓冲移动
			if(pscc->vb_old_offset>MAX_CACHE_SIZE/2)
			{
				//OutputDebugStringA("二级视频缓冲开始移动\n");
				EnterCriticalSection(&pscc->cs_pool_sec_video);
				EnterCriticalSection(&pscc->cs_list_sec_video);
				memmove(pscc->pVideoBuffer,pscc->pVideoBuffer+pscc->vb_old_offset,pscc->vb_new_offset-pscc->vb_old_offset);
				//更新索引值
				idxlist::iterator item;
				for(item=pscc->vidx.begin();item!=pscc->vidx.end();item++)
				{
					item->epos-=pscc->vb_old_offset;
					item->spos-=pscc->vb_old_offset;
				}
				pscc->vb_new_offset -= pscc->vb_old_offset;
				pscc->vb_old_offset = 0;
				
				LeaveCriticalSection(&pscc->cs_list_sec_video);
				LeaveCriticalSection(&pscc->cs_pool_sec_video);
				//OutputDebugStringA("二级视频缓冲移动完成\n");
			}
		}
		else
		{
			Sleep(100);
			//OutputDebugStringA("视频解码线程空闲\n");
			if(pscc->bDataOver)
			{
				CreateThread(NULL,NULL,StreamOverThread,pscc,NULL,NULL);
				return S_OK;
			}
		}
	}
	GetCurrentUsTime(pscc);
	return S_OK;
}
DWORD WINAPI PlayAudioThread(LPVOID lp)
{
	PLAY_SYNC_CONTRL_CONFIG *pscc = (PLAY_SYNC_CONTRL_CONFIG *)lp;
	offsetidx		oi;
	idxlist::iterator	item;
	while(pscc->m_bAudioRun)
	{
		
		if(pscc->aidx.size())
		{
			item = pscc->aidx.begin();
			oi.epos = item->epos;
			oi.spos = item->spos;
			oi.pltime = item->pltime;
			
			//这里需要一个播放线程的通知,以便于知道音频播放的当前时间
			//char str[128];
			//sprintf_s(str," 音频缓冲区大小 %d \n",pscc->aidx.size());
			//OutputDebugStringA(str);
			EnterCriticalSection(&pscc->cs_list_sec_audio);
			pscc->aidx.pop_front();
			LeaveCriticalSection(&pscc->cs_list_sec_audio);
			//解码音频数据 解码后的数据已经复制到PCM缓冲区中了
			if(DeCodecAudio(pscc->pCodecConfig,pscc->pAudioBuffer+oi.spos,oi.epos-oi.spos,oi.pltime,true)==S_FALSE)
			{
				//音频复制失败的话把这段音频重新放回到索引表中
				pscc->aidx.push_front(oi);
				Sleep(1);
			}
			else
			{
				pscc->stream_time_base = oi.pltime;
				pscc->ab_old_offset=oi.epos;
			}
			//二级音频缓冲移动
			if(pscc->ab_old_offset>MAX_CACHE_SIZE/2)
			{
				EnterCriticalSection(&pscc->cs_pool_sec_audio);
				EnterCriticalSection(&pscc->cs_list_sec_audio);
				memmove(pscc->pAudioBuffer,pscc->pAudioBuffer+pscc->ab_old_offset,pscc->ab_new_offset-pscc->ab_old_offset);
				//更新索引值
				idxlist::iterator item;
				for(item=pscc->aidx.begin();item!=pscc->aidx.end();item++)
				{
					item->epos-=pscc->ab_old_offset;
					item->spos-=pscc->ab_old_offset;
				}
				pscc->ab_new_offset -= pscc->ab_old_offset;
				pscc->ab_old_offset = 0;
				LeaveCriticalSection(&pscc->cs_list_sec_audio);
				LeaveCriticalSection(&pscc->cs_pool_sec_audio);
			}
		}
		else
		{
			Sleep(100);
			//OutputDebugStringA("音频解码线程空闲\n");
			if(pscc->bDataOver)
			{
				//pscc->fOnStreamOver(pscc->nPort);
				CreateThread(NULL,NULL,StreamOverThread,pscc,NULL,NULL);
				return S_OK;
			}
		}
	}
	GetCurrentUsTime(pscc);
	return S_OK;
}
DWORD WINAPI StreamOverThread(LPVOID lp)
{
	PLAY_SYNC_CONTRL_CONFIG *pscc = (PLAY_SYNC_CONTRL_CONFIG *)lp;
	if(TryEnterCriticalSection(&pscc->cs_list_sec_audio))
	{
		if(pscc->bCallStreamOver==false)
		{
			pscc->bCallStreamOver = true;
			LeaveCriticalSection(&pscc->cs_list_sec_audio);
			pscc->fOnStreamOver(pscc->nPort);
		}
	}

	return 0;
}
DWORD WINAPI PlayPCMThread(LPVOID lp)
{
	LPVOID lplockbuf;
	DWORD  len;	
	HRESULT hr;
	DWORD  res=5;
	VOID* pDSLockedBuffer = NULL;
	VOID* pDSLockedBuffer2 = NULL;
	DWORD dwDSLockedBufferSize;
	DWORD dwDSLockedBufferSize2;
	DWORD lastTime = 0;
	PLAY_SYNC_CONTRL_CONFIG *pscc = (PLAY_SYNC_CONTRL_CONFIG *)lp;
	pscc->pAudioConfig->m_dwNextWriteOffset = 0;
	pscc->pAudioConfig->m_pDSBuffer->Lock(0,BUFFERNOTIFYSIZE*MAX_AUDIO_BUF,&lplockbuf,&len,NULL,NULL,0);
	memset(lplockbuf,0x0,len);
	//需要提前复制一段数据到缓冲区,防止声音不连续
	if(WaitForSingleObject(pscc->pCodecConfig->m_pcm_cpy_Event,INFINITE)==WAIT_OBJECT_0)//有新数据到达 在二级音频缓冲区中
	{
		memcpy((BYTE*)lplockbuf,pscc->pCodecConfig->m_pPCMBuffer+pscc->pCodecConfig->m_oldpcmoffset,BUFFERNOTIFYSIZE);
		pscc->pAudioConfig->m_dwNextWriteOffset+=BUFFERNOTIFYSIZE;
		pscc->pAudioConfig->m_dwNextWriteOffset %= (BUFFERNOTIFYSIZE * MAX_AUDIO_BUF);//实现一个类似循环缓冲的地方
		//ResetEvent(pscc->pCodecConfig->m_pcm_cpy_Event);
		pscc->pCodecConfig->m_oldpcmoffset+=BUFFERNOTIFYSIZE;

	}

	pscc->pAudioConfig->m_pDSBuffer->Unlock(lplockbuf,len,NULL,0);
	pscc->pAudioConfig->m_pDSBuffer->SetCurrentPosition(0);
	pscc->pAudioConfig->m_pDSBuffer->Play(0,0,DSBPLAY_LOOPING);
	while(pscc->m_bAudioRun)
	{
		res = WaitForMultipleObjects (MAX_AUDIO_BUF, pscc->pAudioConfig->m_event, FALSE, 500);
		if((res >=WAIT_OBJECT_0)&&(res <=WAIT_OBJECT_0+3))//播放完一个，要求填缓冲 
		{
			//char str[128];
			//DWORD nTime = GetTickCount();
			//sprintf_s(str,"---通知时间点 %d---%d\n",nTime,nTime-lastTime);
			//lastTime = nTime;
			//OutputDebugStringA(str);
			if(pscc->pCodecConfig->m_newpcmoffset-pscc->pCodecConfig->m_oldpcmoffset<BUFFERNOTIFYSIZE)
			{
				//通知解码器可以解码了,以此推断播放的声音是解码声音的200毫秒前
				SetEvent(pscc->pCodecConfig->m_pcm_dec_Event);
				//OutputDebugStringA("通知解码器解码\n");
			}
			if(WaitForSingleObject(pscc->pCodecConfig->m_pcm_cpy_Event,500)==WAIT_OBJECT_0)//有新数据到达 在二级音频缓冲区中
			{
				if(pscc->pCodecConfig->m_newpcmoffset-pscc->pCodecConfig->m_oldpcmoffset<BUFFERNOTIFYSIZE)
				{
					ResetEvent(pscc->pCodecConfig->m_pcm_cpy_Event);
					//在这里修改同步时间
					continue;
				}
				//char str[128];
				//DWORD nTime = GetTickCount();
				//sprintf_s(str,"---通知时间点 %d---%d\n",nTime,nTime-lastTime);
				//lastTime = nTime;
				//OutputDebugStringA(str);
				//OutputDebugStringA("复制到音频缓冲区\n");
				hr=pscc->pAudioConfig->m_pDSBuffer->Lock(pscc->pAudioConfig->m_dwNextWriteOffset,BUFFERNOTIFYSIZE,&pDSLockedBuffer,&dwDSLockedBufferSize, &pDSLockedBuffer2,&dwDSLockedBufferSize2,0);
				if(hr == DSERR_BUFFERLOST)
				{
					pscc->pAudioConfig->m_pDSBuffer->Restore();
					pscc->pAudioConfig->m_pDSBuffer->Lock(pscc->pAudioConfig->m_dwNextWriteOffset,BUFFERNOTIFYSIZE,&pDSLockedBuffer,&dwDSLockedBufferSize,&pDSLockedBuffer2,&dwDSLockedBufferSize2,0);
				}
				if(SUCCEEDED(hr))
				{
					//char str[128];
					//sprintf_s(str,"-------音频有效数据位置 %d----%d\n",pscc->pCodecConfig->m_oldpcmoffset,pscc->pCodecConfig->m_newpcmoffset);
					//OutputDebugStringA(str);
					memcpy((BYTE*)pDSLockedBuffer,pscc->pCodecConfig->m_pPCMBuffer+pscc->pCodecConfig->m_oldpcmoffset,BUFFERNOTIFYSIZE);
					pscc->pAudioConfig->m_dwNextWriteOffset+=dwDSLockedBufferSize;
					pscc->pAudioConfig->m_dwNextWriteOffset %= (BUFFERNOTIFYSIZE * MAX_AUDIO_BUF);//实现一个类似循环缓冲的地方
					hr = pscc->pAudioConfig->m_pDSBuffer->Unlock(pDSLockedBuffer,dwDSLockedBufferSize,pDSLockedBuffer2,dwDSLockedBufferSize2);//unlock这一片缓冲，准备被播放
					ResetEvent(pscc->pCodecConfig->m_pcm_cpy_Event);
					pscc->pCodecConfig->m_oldpcmoffset+=BUFFERNOTIFYSIZE;
					//缓冲区要移动啦
					if(pscc->pCodecConfig->m_oldpcmoffset>MAX_CACHE_SIZE/2)
					{
						OutputDebugStringA("音频PCM缓冲区开始移动\n");
						memmove(pscc->pCodecConfig->m_pPCMBuffer,pscc->pCodecConfig->m_pPCMBuffer+pscc->pCodecConfig->m_oldpcmoffset,pscc->pCodecConfig->m_newpcmoffset-pscc->pCodecConfig->m_oldpcmoffset);
						pscc->pCodecConfig->m_newpcmoffset -= pscc->pCodecConfig->m_oldpcmoffset;
						pscc->pCodecConfig->m_oldpcmoffset = 0;
						OutputDebugStringA("音频PCM缓冲区结束移动\n");

					}
				}
				else
				{
					hr = pscc->pAudioConfig->m_pDSBuffer->Unlock(pDSLockedBuffer,dwDSLockedBufferSize,pDSLockedBuffer2,dwDSLockedBufferSize2);//unlock这一片缓冲，准备被播放
				}
			}
		}
	}
	return S_OK;
}
HRESULT CloseAudio(PLAY_SYNC_CONTRL_CONFIG *pscc)
{
	pscc->m_bCloseAudio = true;
	pscc->pAudioConfig->m_pDSBuffer->Stop();
	return S_OK;
}
HRESULT ReSetAudioPlay(PLAY_SYNC_CONTRL_CONFIG *pscc)
{
	pscc->aidx.clear();
	memset(pscc->pCodecConfig->m_pPCMBuffer,0,MAX_CACHE_SIZE);
	pscc->pCodecConfig->m_newpcmoffset = 0;
	pscc->pCodecConfig->m_oldpcmoffset = 0;
	pscc->pAudioConfig->m_dwNextWriteOffset = 0;
	pscc->m_bCloseAudio = false;
	pscc->pAudioConfig->m_pDSBuffer->SetCurrentPosition(0);
	pscc->pAudioConfig->m_pDSBuffer->Play(0,0,DSBPLAY_LOOPING);
	return S_OK;
	
}
HRESULT	Take_Pic(PLAY_SYNC_CONTRL_CONFIG *pscc)
{
	if(pscc->m_bTakePic)
	{
		AVFrame	*pVideo2Frame =avcodec_alloc_frame();;
		uint8_t	*buffer = (uint8_t *)malloc(1920*1080*4);
		SwsContext *ctx = sws_getContext(
			pscc->pCodecConfig->m_pCodecCtxVideo->width,
			pscc->pCodecConfig->m_pCodecCtxVideo->height,
			pscc->pCodecConfig->m_pCodecCtxVideo->pix_fmt,
			pscc->pCodecConfig->m_pCodecCtxVideo->width,
			pscc->pCodecConfig->m_pCodecCtxVideo->height,
			PIX_FMT_BGR24,
			SWS_FAST_BILINEAR, NULL, NULL, NULL);

		avpicture_fill((AVPicture *)pVideo2Frame, buffer, PIX_FMT_BGR24,pscc->pCodecConfig->m_pCodecCtxVideo->width,pscc->pCodecConfig->m_pCodecCtxVideo->height);
		sws_scale(ctx,pscc->pCodecConfig->m_pVideoFrame->data,pscc->pCodecConfig->m_pVideoFrame->linesize,0,pscc->pCodecConfig->m_pCodecCtxVideo->height,pVideo2Frame->data,pVideo2Frame->linesize);		

		CxImage	*lpImage = new CxImage();
		lpImage->CreateFromArray(buffer,pscc->pCodecConfig->m_pCodecCtxVideo->width,pscc->pCodecConfig->m_pCodecCtxVideo->height,24,3*pscc->pCodecConfig->m_pCodecCtxVideo->width,true);
		bool ret = lpImage->Save((const TCHAR *)pscc->m_pTakePicFileName, CXIMAGE_FORMAT_JPG);
		delete lpImage;
		sws_freeContext(ctx);
		av_free(pVideo2Frame);
		free(buffer);
		buffer = NULL;
		pscc->m_bTakePic = false;
		return S_OK;
	}
	return S_FALSE;
}