// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"

ULONG_PTR	gdiplusToken;
unsigned long		G_port = 1;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			Gdiplus::GdiplusStartupInput	nGdiplusStartInput;
			Gdiplus::GdiplusStartup(&gdiplusToken,&nGdiplusStartInput,NULL);
			//avcodec_init();
			av_register_all();
			avcodec_register_all();
			

			break;
		}
	case DLL_THREAD_ATTACH:
		{
			
			break;
		}
	case DLL_THREAD_DETACH:
		{
			break;
		}
	case DLL_PROCESS_DETACH:
		{
			Gdiplus::GdiplusShutdown(gdiplusToken);
			break;
		}
		break;
	}
	return TRUE;
}

