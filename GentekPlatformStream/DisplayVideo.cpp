#include "stdafx.h"
#include "StreamConfig.h"

#define NUM_OVERLAY_FORMATS	6
DDPIXELFORMAT ddpfPixelFormats[] = 
{
	{sizeof(DDPIXELFORMAT), DDPF_FOURCC,MAKEFOURCC('U','Y','V','Y'),0,0,0,0,0},   // UYVY
	{sizeof(DDPIXELFORMAT), DDPF_FOURCC,MAKEFOURCC('Y','U','Y','2'),0,0,0,0,0},   // YUY2
	{sizeof(DDPIXELFORMAT), DDPF_FOURCC,MAKEFOURCC('Y','V','1','2'),0,0,0,0,0},   // YV12 
	{sizeof(DDPIXELFORMAT), DDPF_FOURCC,MAKEFOURCC('Y','V','U','9'),0,0,0,0,0},   // YVU9
	{sizeof(DDPIXELFORMAT), DDPF_FOURCC,MAKEFOURCC('I','F','0','9'),0,0,0,0,0},   // IF09
	{sizeof(DDPIXELFORMAT), DDPF_RGB, 0, 32, 0x00FF0000,0x0000FF00,0x000000FF, 0} // RGB32
};

BOOL WINAPI DDEnumCallbackEx(
							 GUID FAR *lpGUID,    
							 LPSTR     lpDriverDescription, 
							 LPSTR     lpDriverName,        
							 LPVOID    lpContext,           
							 HMONITOR  hm)
{
	HRESULT	hr;
	VIDEOPLAY_CONFIG *dp = (VIDEOPLAY_CONFIG *)lpContext;
	if(lpGUID)
	{
		char str[128];
		sprintf_s(str,"lpDriverDescription %s %s %d %d %d %s\n",lpDriverDescription,lpDriverName,lpGUID->Data1,lpGUID->Data2,lpGUID->Data3,lpGUID->Data4);
		OutputDebugStringA(str);		
		hr = DirectDrawCreateEx(lpGUID,(VOID **)&dp->m_pddraw[dp->m_nDisplayDevice],IID_IDirectDraw7,NULL);
		if(hr!=DD_OK)
		{
			OutputDebugStringW(L"建立DirectDraw对象失败\n");
			return hr;
		}
		dp->m_hMonitor[dp->m_nDisplayDevice]=hm;
		dp->m_monitorInfo[dp->m_nDisplayDevice].cbSize=sizeof(MONITORINFOEX);
		GetMonitorInfo(hm,&dp->m_monitorInfo[dp->m_nDisplayDevice]);
		dp->m_nDisplayDevice+=1;
	}
	return DDENUMRET_OK;
}

HRESULT		InitDirectDraw(VIDEOPLAY_CONFIG *dp,unsigned int nWidth,unsigned int nHeight)
{
	HRESULT hr;
	//枚举显示设备
	dp->m_nSurPixFMT = DEFAULT_PIXFORMAT;
	if(DirectDrawEnumerateExA(DDEnumCallbackEx ,(LPVOID)dp,DDENUM_ATTACHEDSECONDARYDEVICES)!=DD_OK)
	{
		return S_FALSE;
	}
	if(dp->m_nDisplayDevice==0)
	{
		hr = DirectDrawCreateEx(NULL,(VOID **)&dp->m_pddraw[dp->m_nDisplayDevice],IID_IDirectDraw7,NULL);
		if(hr!=DD_OK)
		{
			OutputDebugStringW(L"建立DirectDraw对象失败\n");
			return hr;
		}
		dp->m_nDisplayDevice = 1;
	}
	dp->m_bFillWindow = true;
	for(unsigned int j=0;j<dp->m_nDisplayDevice;j++)
	{
		dp->m_pOverLaySur[j] = NULL;		
		ZeroMemory(&dp->m_ddcaps[j], sizeof(dp->m_ddcaps[j]));
		dp->m_ddcaps[j].dwSize = sizeof(dp->m_ddcaps[j]);
		BOOL	bOverLay = true;
		hr = dp->m_pddraw[j]->GetCaps(&dp->m_ddcaps[j],NULL);
		if(hr!=DD_OK)
		{
			OutputDebugStringW(L"获取显卡设备信息失败\n");
			return hr;
		}
		if (!(dp->m_ddcaps[j].dwCaps & DDCAPS_OVERLAY))
		{
			bOverLay = false;
			OutputDebugStringW(L"显卡不支持Overlay特性 或 系统没有启用DirectDraw加速\n");
		}
		hr = dp->m_pddraw[j]->SetCooperativeLevel(dp->m_hWnd ,DDSCL_NORMAL);
		if(hr!=DD_OK)
		{
			OutputDebugStringW(L"建立程序协调层级失败 \n");
			return hr;
		}
		bool flag = false;
		ZeroMemory(&dp->m_PrimaryDesc[j], sizeof(dp->m_PrimaryDesc[j]));
		dp->m_PrimaryDesc[j].dwSize = sizeof(dp->m_PrimaryDesc[j]);
		if (flag)
		{
			// under full screen 
			dp->m_PrimaryDesc[j].dwFlags        = DDSD_CAPS|DDSD_BACKBUFFERCOUNT;
			dp->m_PrimaryDesc[j].dwBackBufferCount = 1;
			dp->m_PrimaryDesc[j].ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE|DDSCAPS_COMPLEX|DDSCAPS_FLIP;
		}
		else
		{
			// not under full screen
			dp->m_PrimaryDesc[j].dwFlags = DDSD_CAPS;
			dp->m_PrimaryDesc[j].ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE|DDSCAPS_VIDEOMEMORY;    
		}
		if(FAILED(dp->m_pddraw[j]->CreateSurface(&dp->m_PrimaryDesc[j], &dp->m_pPrimarySur[j], NULL)))
		{
			OutputDebugStringW(L"建立绘画表面失败\n");
			return S_FALSE;
		}
		dp->m_ddscaps[j].dwCaps = DDSCAPS_BACKBUFFER;
		hr = dp->m_pPrimarySur[j]->GetAttachedSurface(&dp->m_ddscaps[j],&dp->m_pBackSurface[j]);
		dp->m_pddraw[j]->GetDisplayMode(&dp->m_PrimaryDesc[j]);
		if(dp->m_PrimaryDesc[j].ddpfPixelFormat.dwRGBBitCount!=32)
		{
			if(dp->m_PrimaryDesc[j].ddpfPixelFormat.dwRGBBitCount<16)
			{
				OutputDebugStringW(L"不支持当前显示模式\n");
				return DD_FALSE;
			}
			//dp->m_nSurPixFMT = 2;
			OutputDebugStringW(L"非32位真彩显示模式,可能影响一些功能使用或图像显示异常 \n");
		}
		//设置显示区域为窗口区域
		if(FAILED(hr = dp->m_pddraw[j]->CreateClipper(0, &dp->m_pClipper[j], NULL))) 
		{
			return E_FAIL;
		}	
		if(FAILED( hr = dp->m_pPrimarySur[j]->SetClipper(dp->m_pClipper[j]))) 
		{
			return E_FAIL;
		}
		if(FAILED(hr = dp->m_pClipper[j]->SetHWnd(0, dp->m_hWnd )))
		{
			return E_FAIL;
		}
		ZeroMemory(&dp->m_PrimaryDesc[j], sizeof(dp->m_PrimaryDesc[j]));
		dp->m_PrimaryDesc[j].dwSize = sizeof(dp->m_PrimaryDesc[j]);
		dp->m_PrimaryDesc[j].dwFlags        = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT|DDSD_PIXELFORMAT; 
		dp->m_PrimaryDesc[j].ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN|DDSCAPS_VIDEOMEMORY ;
		dp->m_PrimaryDesc[j].dwWidth        = nWidth;
		dp->m_PrimaryDesc[j].dwHeight       = nHeight;
		unsigned int i = dp->m_nSurPixFMT;
		while(i < NUM_OVERLAY_FORMATS){
			dp->m_PrimaryDesc[j].ddpfPixelFormat = ddpfPixelFormats[i];
			hr = dp->m_pddraw[j]->CreateSurface(&dp->m_PrimaryDesc[j], &dp->m_pSurface[j], NULL);
			if (hr!=DD_OK)
			{
				OutputDebugStringW(L"建立绘图表面失败 \n");
				i++;
				continue;
			}
			if(hr == DDERR_UNSUPPORTEDMODE)
			{
				dp->m_pSurface[j] = NULL;
				return hr;
			}
			break;
		}

	}
	OutputDebugStringA("视频模块初始化\n");
	dp->m_nWidth = nWidth;
	dp->m_nHeight = nHeight;
	return S_OK;
}
HRESULT		ReInitDirectDraw(VIDEOPLAY_CONFIG *dp,unsigned int nWidth,unsigned int nHeight)
{
	HRESULT hr;
	for(unsigned int j=0;j<dp->m_nDisplayDevice;j++)
	{

		dp->m_pSurface[j]->Unlock(NULL);
		if(dp->m_pSurface[j])
		{
			dp->m_pSurface[j]->Release();
			dp->m_pSurface[j]=NULL;
		}
		ZeroMemory(&dp->m_PrimaryDesc[j], sizeof(dp->m_PrimaryDesc[j]));
		dp->m_PrimaryDesc[j].dwSize = sizeof(dp->m_PrimaryDesc[j]);
		dp->m_PrimaryDesc[j].dwFlags        = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT|DDSD_PIXELFORMAT; 
		dp->m_PrimaryDesc[j].ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN ;
		dp->m_PrimaryDesc[j].dwWidth        = nWidth;
		dp->m_PrimaryDesc[j].dwHeight       = nHeight;


		int i= dp->m_nSurPixFMT;
		while(i < NUM_OVERLAY_FORMATS){
			dp->m_PrimaryDesc[j].ddpfPixelFormat = ddpfPixelFormats[i];
			dp->m_pClipper[j] = NULL;
			if (FAILED(hr = dp->m_pddraw[j]->CreateSurface(&dp->m_PrimaryDesc[j], &dp->m_pSurface[j], NULL)))
			{
				i++;
				continue;
			}	
			if(hr == DDERR_UNSUPPORTEDMODE)
			{
				dp->m_pSurface[j] = NULL;
				return hr;
			}
			break;
		}

		if(dp->m_pSurface[j]==NULL||dp->m_pPrimarySur[j]==NULL)
		{
			return hr;
		}
		dp->m_nWidth = nWidth;
		dp->m_nHeight = nHeight;
	}
	return hr;
}
HRESULT		InitGDIPlus(VIDEOPLAY_CONFIG *dp)
{
	dp->m_pgraphics = Gdiplus::Graphics::FromHWND(dp->m_hWnd);
	return S_OK;
}

HRESULT		UnInitDirectDraw(VIDEOPLAY_CONFIG *dp)
{

	return S_OK;
}
HRESULT		UnInitGDIPlus(VIDEOPLAY_CONFIG *dp)
{
	if(dp->m_pgraphics)
	{
		delete dp->m_pgraphics;
	}
	return S_OK;
}
HRESULT	FreshYUVSurface(VIDEOPLAY_CONFIG *dp,uint8_t *data[4],    int linesize[4],LPRECT pSrc)
{
	HRESULT	hr;
	RECT rs;
	uint8_t *data_0 = data[0];
	uint8_t *data_1 = data[1];
	uint8_t *data_2 = data[2];
	::GetWindowRect(dp->m_hWnd,&rs);
	int MonitorIdx = 0;
	if(dp->m_nDisplayDevice==1)
	{
		MonitorIdx = 0;
	}
	else
	{
		HMONITOR Cur = MonitorFromRect(&rs,MONITOR_DEFAULTTONEAREST);
		for(unsigned int i=0;i<dp->m_nDisplayDevice;i++)
		{
			if(Cur==dp->m_hMonitor[i])
			{
				MonitorIdx = i;
				break;
			}
		}
	}
	if(MonitorIdx)
	{
		rs.left -=dp->m_monitorInfo[MonitorIdx].rcMonitor.left;
		rs.right -=dp->m_monitorInfo[MonitorIdx].rcMonitor.left;
		rs.bottom -=dp->m_monitorInfo[MonitorIdx].rcMonitor.top;
		rs.top -=dp->m_monitorInfo[MonitorIdx].rcMonitor.top;
	}

	if(!dp->m_bFillWindow&&dp->m_PrimaryDesc[MonitorIdx].dwWidth)
	{
		int tmph,tmpw,offset;
		tmph = (rs.right-rs.left)*dp->m_PrimaryDesc[MonitorIdx].dwHeight/dp->m_PrimaryDesc[MonitorIdx].dwWidth;
		if(tmph>rs.bottom-rs.top)//以宽度为准保持比例后高度不够
		{
			//则以高度为准
			tmpw = (rs.bottom-rs.top)*dp->m_PrimaryDesc[MonitorIdx].dwWidth/dp->m_PrimaryDesc[MonitorIdx].dwHeight;
			offset=(rs.right-rs.left-tmpw)/2;
			rs.left+=offset;
			rs.right-=offset;
		}
		else
		{
			//以宽度为准
			offset=(rs.bottom-rs.top-tmph)/2;
			rs.top+=offset;
			rs.bottom-=offset;
		}
	}
	dp->m_pSurface[MonitorIdx]->Lock(NULL,&dp->m_PrimaryDesc[MonitorIdx],0,NULL);
	LPBYTE lpSurf = (LPBYTE)dp->m_PrimaryDesc[MonitorIdx].lpSurface;

	if(lpSurf) {
		unsigned int i;
		// fill Y data
		for(i = 0; i < dp->m_PrimaryDesc[MonitorIdx].dwHeight; i++)
		{
			memcpy(lpSurf, data_0, dp->m_PrimaryDesc[MonitorIdx].dwWidth );
			data_0 += linesize[0];
			lpSurf += dp->m_PrimaryDesc[MonitorIdx].lPitch;
		}
		// fill V data
		for(i = 0; i < dp->m_PrimaryDesc[MonitorIdx].dwHeight / 2; i++)
		{
			memcpy(lpSurf, data_2, dp->m_PrimaryDesc[MonitorIdx].dwWidth / 2);
			data_2 +=linesize[2];
			lpSurf += dp->m_PrimaryDesc[MonitorIdx].lPitch / 2;
		}
		// fill U data
		for(i = 0; i < dp->m_PrimaryDesc[MonitorIdx].dwHeight / 2; i++)
		{
			memcpy(lpSurf, data_1, dp->m_PrimaryDesc[MonitorIdx].dwWidth / 2);
			data_1 += linesize[1];
			lpSurf += dp->m_PrimaryDesc[MonitorIdx].lPitch / 2;
		}
	}
	dp->m_pSurface[MonitorIdx]->Unlock(NULL);


	if(dp->m_pPrimarySur[MonitorIdx])
	{
		hr = dp->m_pPrimarySur[MonitorIdx]->Blt(&rs, dp->m_pSurface[MonitorIdx], pSrc, DDBLT_WAIT,NULL);
		if(hr!=DD_OK)
		{
			OutputDebugStringW(L"Bit2 失败\n");
		}
		
		return hr;
		//m_pPrimarySur->Flip(NULL,DDFLIP_WAIT);
	}
	return S_FALSE;
}
HRESULT	FreshRGBSurface(VIDEOPLAY_CONFIG *dp,uint8_t *data[4],    int linesize[4],unsigned char *buf,LPRECT pSrc)
{
	HRESULT	hr;
	RECT rs;
	::GetWindowRect(dp->m_hWnd,&rs);
	int MonitorIdx = 0;
	if(dp->m_nDisplayDevice==1)
	{
		MonitorIdx = 0;
	}
	else
	{
		HMONITOR Cur = MonitorFromRect(&rs,MONITOR_DEFAULTTONEAREST);
		for(unsigned int i=0;i<dp->m_nDisplayDevice;i++)
		{
			if(Cur==dp->m_hMonitor[i])
			{
				MonitorIdx = i;
				break;
			}
		}
	}


	if(MonitorIdx)
	{
		rs.left -=dp->m_monitorInfo[MonitorIdx].rcMonitor.left;
		rs.right -=dp->m_monitorInfo[MonitorIdx].rcMonitor.left;
		rs.bottom -=dp->m_monitorInfo[MonitorIdx].rcMonitor.top;
		rs.top -=dp->m_monitorInfo[MonitorIdx].rcMonitor.top;
	}

	if(!dp->m_bFillWindow&&dp->m_PrimaryDesc[MonitorIdx].dwWidth)
	{
		int tmph,tmpw,offset;
		tmph = (rs.right-rs.left)*dp->m_PrimaryDesc[MonitorIdx].dwHeight/dp->m_PrimaryDesc[MonitorIdx].dwWidth;
		if(tmph>rs.bottom-rs.top)//以宽度为准保持比例后高度不够
		{
			//则以高度为准
			tmpw = (rs.bottom-rs.top)*dp->m_PrimaryDesc[MonitorIdx].dwWidth/dp->m_PrimaryDesc[MonitorIdx].dwHeight;
			offset=(rs.right-rs.left-tmpw)/2;
			rs.left+=offset;
			rs.right-=offset;
		}
		else
		{
			//以宽度为准
			offset=(rs.bottom-rs.top-tmph)/2;
			rs.top+=offset;
			rs.bottom-=offset;
		}
	}

	dp->m_pSurface[MonitorIdx]->Lock(NULL,&dp->m_PrimaryDesc[MonitorIdx],0,NULL);
	LPBYTE lpSurf = (LPBYTE)dp->m_PrimaryDesc[MonitorIdx].lpSurface;
	if(lpSurf) {
		memcpy(lpSurf,buf,dp->m_PrimaryDesc[MonitorIdx].dwWidth*dp->m_PrimaryDesc[MonitorIdx].dwHeight*4);
	}
	dp->m_pSurface[MonitorIdx]->Unlock(NULL);

	if(dp->m_pPrimarySur[MonitorIdx])
	{
		hr = dp->m_pPrimarySur[MonitorIdx]->Blt(&rs, dp->m_pSurface[MonitorIdx], pSrc, DDBLT_WAIT,NULL);
		if(hr!=DD_OK)
		{
			OutputDebugStringW(L"Bit 失败\n");
			
		}
		else
		{
			//成功
		}
		return hr;
		
		//m_pPrimarySur->Flip(NULL,DDFLIP_WAIT);
	}
	return S_FALSE;
}

HRESULT PrintRGB(VIDEOPLAY_CONFIG *dp,uint8_t *data[4],    int linesize[4],unsigned char *buf,LPRECT Src)
{
	RECT rc;
	::GetWindowRect(dp->m_hWnd,&rc);
	Gdiplus::Bitmap	bmp(dp->m_nWidth,dp->m_nHeight,dp->m_nWidth*3,PixelFormat24bppRGB,buf);
	dp->m_pgraphics->DrawImage(&bmp,rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top);
	return S_FALSE;
}
HRESULT PrintYUV(VIDEOPLAY_CONFIG *dp,uint8_t *data[4],    int linesize[4],LPRECT Src)
{
	return S_FALSE;
}