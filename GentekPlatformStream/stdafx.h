// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"




#ifndef POINTER_64

#if !defined(_MAC) && (defined(_M_MRX000) || defined(_M_AMD64) || defined(_M_IA64)) && (_MSC_VER >= 1100) && !(defined(MIDL_PASS) || defined(RC_INVOKED))
#define POINTER_64 __ptr64
typedef unsigned __int64 POINTER_64_INT;
#if defined(_WIN64)
#define POINTER_32 __ptr32
#else
#define POINTER_32
#endif
#else
#if defined(_MAC) && defined(_MAC_INT_64)
#define POINTER_64 __ptr64
typedef unsigned __int64 POINTER_64_INT;
#else
#if (_MSC_VER >= 1300) && !(defined(MIDL_PASS) || defined(RC_INVOKED))
#define POINTER_64 __ptr64
#else
#define POINTER_64
#endif
typedef unsigned long POINTER_64_INT;
typedef unsigned long UINT64_C;
#endif
#define POINTER_32
#endif
#endif
//#define WIN32_LEAN_AND_MEAN             // 从 Windows 头中排除极少使用的资料
// Windows 头文件:
#include <windows.h>
#pragma warning(disable:4786)

//gdi plus sdk
#include <GdiPlus.h>
#pragma  comment(lib,"gdiplus.lib")
//using namespace Gdiplus;


#define		DEFAULT_PIXFORMAT   2
extern "C"
{
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
	//#include <libavcodec/dxva2.h>
	//#include <libavcodec/vaapi.h>
#ifdef _DEBUG
#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avcodec.lib")	
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"swscale.lib")
#else
#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avcodec.lib")	
#pragma comment(lib,"swscale.lib")
#pragma comment(lib,"avutil.lib")
#endif

}




//system audio sdk
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

//directsound sdk
#include <dsound.h>
#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "dxguid.lib")

//directdraw sdk
#include <ddraw.h>
#pragma comment(lib,"ddraw.lib")


//ffmpeg and other codec
#include "./codec/g711.h"

//openmp sdk
#include<omp.h>

#include <ximage.h>
#ifdef _DEBUG
#pragma comment(lib,"../lib/CxImage/lib/Debug/cximage.lib")
#pragma comment(lib,"../lib/CxImage/lib/Debug/Jpeg.lib")	
#pragma comment(lib,"../lib/CxImage/lib/Debug/png.lib")
#pragma comment(lib,"../lib/CxImage/lib/Debug/zlib.lib")
#else
#pragma comment(lib,"../lib/CxImage/lib/Release/cximage.lib")
#pragma comment(lib,"../lib/CxImage/lib/Release/Jpeg.lib")	
#pragma comment(lib,"../lib/CxImage/lib/Release/png.lib")
#pragma comment(lib,"../lib/CxImage/lib/Release/zlib.lib")
#endif
#include <map>
#include <list>
#include <complex>
using namespace std;



#define MAX_CACHE_SIZE		2097152
#define SCREEN_BUF_SIZE		8294400  //1920*1080*4


#include "GentekPlatformError.h"










