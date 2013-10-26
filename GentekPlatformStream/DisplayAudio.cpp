#include "StdAfx.h"
#include "StreamConfig.h"


#define MAX_AUDIO_BUF 4 //播放缓冲的通知索引
#define BUFFERNOTIFYSIZE 2048/*8192*//*192000*/ //缓冲通知位置的大小，请参见DirectSound应用程序开发快速入门
#define SAMPLE_RATE      8000/*44100*/ //pcm 的采样率  8k
#define N_CHANNEL          1/*2*/   //PCM 声道数 单通道
#define BITS_PER_SAMPLE    16  //每个采样点的比特数
#define BUF_SIZE			 400*1024
#define DECBUF_SIZE			 4*1024

HRESULT InitDirectSound(AUDIOPLAY_CONFIG *ds_pam,unsigned int nChannel,unsigned int nSamplesPerSec,unsigned int wBitsPerSample)
{
	HRESULT hr = DirectSoundCreate8(NULL, &ds_pam->m_lpds, NULL);
	if(FAILED(hr))
	{
		return hr;
	}
	hr = ds_pam->m_lpds->SetCooperativeLevel(ds_pam->m_hwnd, DSSCL_PRIORITY);
	ds_pam->m_wfex.wFormatTag = WAVE_FORMAT_PCM;
	//ds_pam->m_wfex.nChannels = 2;
	//ds_pam->m_wfex.nSamplesPerSec = 441000;
	//ds_pam->m_wfex.wBitsPerSample = 16;
	ds_pam->m_wfex.nChannels = nChannel;
	ds_pam->m_wfex.nSamplesPerSec = nSamplesPerSec;
	ds_pam->m_wfex.wBitsPerSample = wBitsPerSample;
	ds_pam->m_wfex.nBlockAlign = (ds_pam->m_wfex.wBitsPerSample*ds_pam->m_wfex.nChannels )/8;
	ds_pam->m_wfex.nAvgBytesPerSec = ds_pam->m_wfex.nSamplesPerSec*(ds_pam->m_wfex.wBitsPerSample/8)*ds_pam->m_wfex.nChannels;
	ds_pam->m_wfex.cbSize=0;
	memset(&ds_pam->m_dsbd, 0, sizeof(DSBUFFERDESC));
	ds_pam->m_dsbd.dwSize = sizeof(DSBUFFERDESC);
	ds_pam->m_dsbd.dwFlags = DSBCAPS_CTRLVOLUME|DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLPOSITIONNOTIFY |DSBCAPS_GETCURRENTPOSITION2|DSBCAPS_CTRLPAN|DSBCAPS_CTRLFREQUENCY|DSBCAPS_CTRLFX;
	// 流buffer
	ds_pam->m_dsbd.dwBufferBytes = BUFFERNOTIFYSIZE*MAX_AUDIO_BUF;
	ds_pam->m_dsbd.lpwfxFormat = &ds_pam->m_wfex;
	hr = ds_pam->m_lpds->CreateSoundBuffer(&ds_pam->m_dsbd,&ds_pam->m_pDSB,NULL);
	if (SUCCEEDED(hr))
	{
		hr = ds_pam->m_pDSB->QueryInterface(IID_IDirectSoundBuffer8, (LPVOID*)&ds_pam->m_pDSBuffer);
		if(FAILED(hr))
		{
			return S_FALSE;
		}
		ds_pam->m_pDSBuffer->SetVolume(0);
		ds_pam->m_pDSB->Release();
	}
	else
	{
		return S_FALSE;
	}
	if(FAILED(hr=(ds_pam->m_pDSBuffer->QueryInterface(IID_IDirectSoundNotify,(LPVOID*)&ds_pam->m_pSoundNotify))))
	{
		switch(hr)
		{
		case E_NOINTERFACE:
			printf("E_NOINTERFACE hr\n");
			break;
		default:
			printf("an unknow error hr,%d\n",hr);
			break;
		}
		return S_FALSE ;
	}
	for(int i =0;i<MAX_AUDIO_BUF;i++)
	{
		ds_pam->m_arrayPosNotify[i].dwOffset =i*BUFFERNOTIFYSIZE;
		ds_pam->m_event[i]=::CreateEvent(NULL,false,false,NULL);//默认安全级别，自动控制，初始为无信号，默认系统对象名
		ds_pam->m_arrayPosNotify[i].hEventNotify=ds_pam->m_event[i];
	}	
	ds_pam->m_pSoundNotify->SetNotificationPositions(MAX_AUDIO_BUF,ds_pam->m_arrayPosNotify);
	ds_pam->m_pSoundNotify->Release();
	OutputDebugStringA("音频模块初始化\n");
	return S_OK;
}
HRESULT SetVolume(AUDIOPLAY_CONFIG *ds,LONG lVolume)
{
	char str[128];
	sprintf_s(str,"设置当前播放器音量为 %d\n",lVolume);
	OutputDebugStringA(str);
	return ds->m_pDSBuffer->SetVolume(lVolume);
}
HRESULT  GetVolume(AUDIOPLAY_CONFIG *ds,LONG *lVolume)
{
	ds->m_pDSBuffer->GetVolume(lVolume);
	char str[128];
	sprintf_s(str,"获取当前播放器音量为 %d\n",*lVolume);
	OutputDebugStringA(str);
	return S_OK;
}
HRESULT UninitDirectSound(AUDIOPLAY_CONFIG *ds_pam)
{
	if(ds_pam->m_pDSBuffer)
	{
		ds_pam->m_pDSBuffer->Unlock(NULL,NULL,NULL,NULL);
		ds_pam->m_pDSBuffer->Stop();
		ds_pam->m_pDSBuffer->Release();
		ds_pam->m_pDSBuffer = NULL;
		ds_pam->m_pDSB=NULL;
	}
	if(ds_pam->m_lpds)
	{
		ds_pam->m_lpds->Release();
		ds_pam->m_lpds = NULL;
	}
	for(int i =0;i<MAX_AUDIO_BUF;i++)
	{
		CloseHandle(ds_pam->m_event[i]);
	}
	return S_OK;
}
HRESULT InitWav(AUDIOPLAY_CONFIG *ds)
{
	return S_OK;
}

HRESULT UninitWav(AUDIOPLAY_CONFIG *ds)
{
	return S_OK;
}

