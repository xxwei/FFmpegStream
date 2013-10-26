#pragma once

#ifndef	STREAM_CONFIG
#define STREAM_CONFIG

#include "stdafx.h"

#include "PlayAndSyncControl.h"
// TODO: 在此处引用程序需要的其他头文件
typedef struct _STREAM_CONFIG 
{
	unsigned int			nVideoCodec;
	unsigned int			nAudioCodec;
	__int64					nTimebase;
	DWORD					nLastError;
	AUDIOPLAY_CONFIG		*pAudioConfig;
	VIDEOPLAY_CONFIG		*pVideoConfig;
	STREAMCODEC_CONFIG		*pCodecConfig;
	PLAY_SYNC_CONTRL_CONFIG	*pContrlConfig;
	DWORD					nStreamFormat;


}* PSTREAMCONFIG;

typedef map<DWORD,PSTREAMCONFIG>		stream_map;

PSTREAMCONFIG GetStream(DWORD nPort);

#endif