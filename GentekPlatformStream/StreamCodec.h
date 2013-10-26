#pragma once

struct STREAMCODEC_CONFIG
{
	AVFormatContext			*m_pStreamFormatContext;
	AVStream				*m_pStreamAudio;
	AVStream				*m_pStreamVideo;
	AVCodecContext			*m_pCodecCtxAudio;
	AVCodecContext			*m_pCodecCtxVideo;
	SwsContext				*m_pSwsContextVideo;
	AVCodec					*m_pVideoCodec;
	AVCodec					*m_pAudioCodec;
	AVFrame					*m_pVideoFrame;
	AVFrame					*m_pVideo2Frame;
	AVFrame					*m_pAudioFrame;
	bool					m_bUp2Down;
	unsigned char			*m_pPCMBuffer;
	unsigned char			*m_pScreenBuffer;
	unsigned int			m_newpcmoffset;
	unsigned int			m_oldpcmoffset;
	PixelFormat				m_pixelFormat;
	LPRECT					m_pShowRect;
	RECT					m_ShowRect;
	int						m_brightness;
	int						m_contrast;
	int						m_saturation;
	HANDLE					m_pcm_cpy_Event;//有足够的数据复制到缓冲区中
	HANDLE					m_pcm_dec_Event;
	


};


HRESULT	InitStreamCodec(STREAMCODEC_CONFIG *pscc,DWORD nVideoCodec,DWORD nAudioCodec);

HRESULT setCopyFiltering(STREAMCODEC_CONFIG *pscc,int b, int c, int s);

HRESULT DeCodecVideo(STREAMCODEC_CONFIG *pscc,const unsigned char *pSrcData,const unsigned int dwDataLen,bool decode);

HRESULT DeCodecAudio(STREAMCODEC_CONFIG *pscc,const unsigned char *pSrcData,const unsigned int dwDataLen,int64_t dts,bool decode);

HRESULT CloseStreamCodec(STREAMCODEC_CONFIG *pscc);

