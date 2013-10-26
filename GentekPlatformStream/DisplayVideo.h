#pragma once


//最多8个显示核心
#define MAX_SERFACE		8
struct VIDEOPLAY_CONFIG
{
	LPDIRECTDRAW7			m_pddraw[MAX_SERFACE];
	LPDIRECTDRAWSURFACE7	m_pPrimarySur[MAX_SERFACE];
	LPDIRECTDRAWSURFACE7	m_pSurface[MAX_SERFACE];//切换表面
	LPDIRECTDRAWSURFACE7	m_pBackSurface[MAX_SERFACE];
	LPDIRECTDRAWSURFACE7	m_pOverLaySur[MAX_SERFACE];
	DDSURFACEDESC2			m_PrimaryDesc[MAX_SERFACE];
	DDCAPS					m_ddcaps[MAX_SERFACE];
	DDSCAPS2				m_ddscaps[MAX_SERFACE];
	LPDIRECTDRAWCLIPPER		m_pClipper[MAX_SERFACE];
	HMONITOR				m_hMonitor[MAX_SERFACE];
	MONITORINFOEX			m_monitorInfo[MAX_SERFACE];
	unsigned	int			m_nDisplayDevice;
	bool					m_bUsingGDIPLUS;
	HWND					m_hWnd;
	bool					m_bFillWindow;
	unsigned	int			m_nWidth;
	unsigned	int			m_nHeight;
	Gdiplus::Graphics		*m_pgraphics;
	unsigned	int			m_nSurPixFMT;
};

//////////////////////////////////////////////////////////////////////////
//waveOutOpen 
//waveOutWrite 
//waveOutClose

HRESULT		InitDirectDraw(VIDEOPLAY_CONFIG *dp,unsigned int nWidth,unsigned int nHeight);
HRESULT		ReInitDirectDraw(VIDEOPLAY_CONFIG *dp,unsigned int nWidth,unsigned int nHeight);//初始化错误后 需要重新创建绘图表面
HRESULT		InitGDIPlus(VIDEOPLAY_CONFIG *dp);

HRESULT		UnInitDirectDraw(VIDEOPLAY_CONFIG *dp);
HRESULT		UnInitGDIPlus(VIDEOPLAY_CONFIG *dp);


HRESULT	FreshYUVSurface(VIDEOPLAY_CONFIG *dp,uint8_t *data[4],    int linesize[4],LPRECT Src = NULL);
HRESULT	FreshRGBSurface(VIDEOPLAY_CONFIG *dp,uint8_t *data[4],    int linesize[4],unsigned char *buf,LPRECT Src = NULL);

HRESULT PrintRGB(VIDEOPLAY_CONFIG *dp,uint8_t *data[4],    int linesize[4],unsigned char *buf,LPRECT Src = NULL);
HRESULT PrintYUV(VIDEOPLAY_CONFIG *dp,uint8_t *data[4],    int linesize[4],LPRECT Src = NULL);

