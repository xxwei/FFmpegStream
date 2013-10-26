#pragma once
#include "DisplayVideo.h"
#include "DisplayAudio.h"
#include "StreamCodec.h"
#include "Common.h"


struct PLAY_SYNC_CONTRL_CONFIG
{
	unsigned __int64		time_base;
	unsigned __int64		stream_time_base;
	HANDLE					h_demuxer_thread;
	HANDLE					h_video_decode_thread;
	HANDLE					h_audio_decode_thread;
	HANDLE					h_audio_pcm_thread;
	unsigned int			nplay_speed;
	bool					bplay_onebyone;

	CRITICAL_SECTION		cs_pool_sec_video;
	CRITICAL_SECTION		cs_pool_sec_audio;
	CRITICAL_SECTION		cs_pool_first;
	unsigned char			*pPacketBuffer;
	unsigned int			pb_new_offset;
	unsigned int			pb_old_offset;
	unsigned char			*pAudioBuffer;
	unsigned int			ab_new_offset;
	unsigned int			ab_old_offset;
	unsigned char			*pVideoBuffer;
	unsigned int			vb_new_offset;
	unsigned int			vb_old_offset;

	AUDIOPLAY_CONFIG		*pAudioConfig;
	VIDEOPLAY_CONFIG		*pVideoConfig;
	STREAMCODEC_CONFIG		*pCodecConfig;
	CRITICAL_SECTION		cs_list_sec_audio;
	CRITICAL_SECTION		cs_list_sec_video;
	idxlist					aidx;
	idxlist					vidx;
	unsigned int			nStatus;
	LARGE_INTEGER			nTickPerSec;
	unsigned __int64		nTickPerUs;
	bool					m_bDemuxRun;
	bool					m_bVideoRun;
	bool					m_bAudioRun;
	demuxer					*m_pdemuxer;
	bool					m_bCloseAudio;

	bool					m_bTakePic;
	unsigned char			m_pTakePicFileName[512];

	unsigned	int			m_pause_n_frame;
	BOOL					bDataOver;
	DWORD					nPort;
	BOOL					bCallStreamOver;
	offsetidx				m_video_eoi;
	offsetidx				m_audio_eoi;
	bool(CALLBACK *fOnStreamOver)(DWORD nPort);
	bool(CALLBACK *fOnTime)(unsigned long nPort,unsigned __int64 nTime);

};

HRESULT OpenStream(DWORD nFormatStream,PLAY_SYNC_CONTRL_CONFIG *pscc);
HRESULT PlayStream(PLAY_SYNC_CONTRL_CONFIG *pscc,STREAMCODEC_CONFIG *pcc,VIDEOPLAY_CONFIG *pvc,AUDIOPLAY_CONFIG *pac);
HRESULT PauseStream(PLAY_SYNC_CONTRL_CONFIG *pscc);
HRESULT StopStream(PLAY_SYNC_CONTRL_CONFIG *pscc);
HRESULT CloseStream(PLAY_SYNC_CONTRL_CONFIG *pscc);
HRESULT PlayVideoOneByOne(PLAY_SYNC_CONTRL_CONFIG *pscc);
HRESULT PlayStreamBySpeed(PLAY_SYNC_CONTRL_CONFIG *pscc,unsigned int nSpeed);

HRESULT FreshSurface(PLAY_SYNC_CONTRL_CONFIG *pscc);

HRESULT	AddVideoPacket(PLAY_SYNC_CONTRL_CONFIG *pscc,char *buf,unsigned int buflen,__int64 time,bool HaveStartCode=true);
HRESULT AddAudioPacket(PLAY_SYNC_CONTRL_CONFIG *pscc,char *buf,unsigned int buflen,__int64 time);
//分离线程
DWORD WINAPI DemuxerThread(LPVOID lp);
//视频解码和渲染线程
DWORD WINAPI PlayVideoThread(LPVOID lp);
//音频解码线程
DWORD WINAPI PlayAudioThread(LPVOID lp);
//声音播放是反向通知 必须有个独立线程
DWORD WINAPI PlayPCMThread(LPVOID lp);

DWORD WINAPI StreamOverThread(LPVOID lp);


HRESULT CloseAudio(PLAY_SYNC_CONTRL_CONFIG *pscc);
HRESULT ReSetAudioPlay(PLAY_SYNC_CONTRL_CONFIG *pscc);

HRESULT	Take_Pic(PLAY_SYNC_CONTRL_CONFIG *pscc);


