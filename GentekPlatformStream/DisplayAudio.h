#pragma once

#define MAX_AUDIO_BUF 4 //播放缓冲的通知索引

#define BUFFERNOTIFYSIZE 2048/*8192*//*192000*/ //缓冲通知位置的大小，请参见DirectSound应用程序开发快速入门
#define SAMPLE_RATE      8000/*44100*/ //pcm 的采样率  8k
#define N_CHANNEL          1/*2*/   //PCM 声道数 单通道
#define BITS_PER_SAMPLE    16  //每个采样点的比特数
#define BUF_SIZE			 400*1024
#define DECBUF_SIZE			 4*1024

struct AUDIOPLAY_CONFIG
{
	WAVEFORMATEX			m_wfex; 
	DSBUFFERDESC			m_dsbd;
	LPDIRECTSOUND8			m_lpds;
	LPDIRECTSOUNDBUFFER		m_pDSB;   
	LPDIRECTSOUNDBUFFER8	m_pDSBuffer;
	LPDIRECTSOUNDNOTIFY8	m_pSoundNotify;
	DSBPOSITIONNOTIFY		m_arrayPosNotify[MAX_AUDIO_BUF];//设置通知位置标志的数组
	HANDLE					m_event[MAX_AUDIO_BUF];//DS事件的句柄
	BOOL					m_bPlay;
	HWND					m_hwnd;
	DWORD					m_dwNextWriteOffset;
	unsigned	int			m_nSamplesPerSec;
	unsigned	int			m_nchannel;

};

HRESULT InitDirectSound(AUDIOPLAY_CONFIG *ds,unsigned int nChannel=1,unsigned int nSamplesPerSec=8000,unsigned int wBitsPerSample=16);
HRESULT InitWav(AUDIOPLAY_CONFIG *ds);

HRESULT SetVolume(AUDIOPLAY_CONFIG *ds,LONG lVolume);
HRESULT GetVolume(AUDIOPLAY_CONFIG *ds,LONG *lVolume);

HRESULT UninitDirectSound(AUDIOPLAY_CONFIG *ds);
HRESULT UninitWav(AUDIOPLAY_CONFIG *ds);




