// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 GENTEKPLATFORMSTREAM_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// GENTEKPLATFORMSTREAM_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifdef GENTEKPLATFORMSTREAM_EXPORTS
#define GENTEKPLATFORMSTREAM_API __declspec(dllexport)
#else
#define GENTEKPLATFORMSTREAM_API __declspec(dllimport)
#endif
//sdk status
enum G_Status{
	G_STATUS_UNKOWN=0,   //未初始化
	G_STATUS_INIT,		 //初始化完成
	G_STATUS_OPEN,		 //流被打开
	G_STATUS_PLAY,		   //流被播放
	G_STATUS_PLAY_ONEBYONE,	//逐帧播放
	G_STATUS_PAUSE,			//流被暂停
	G_STATUS_STOP,		   //流被停止
	G_STATUS_CLOSE		   //流被关闭
};

enum CODEC_TYPE_ID {
	CODEC_TYPE_NONE,

	
	CODEC_TYPE_MPEG1VIDEO,
	CODEC_TYPE_MPEG2VIDEO, ///< preferred ID for MPEG-1/2 video decoding
	CODEC_TYPE_MPEG2VIDEO_XVMC,
	CODEC_TYPE_H261,
	CODEC_TYPE_H263,
	CODEC_TYPE_RV10,
	CODEC_TYPE_RV20,
	CODEC_TYPE_MJPEG,
	CODEC_TYPE_MJPEGB,
	CODEC_TYPE_LJPEG,
	CODEC_TYPE_SP5X,
	CODEC_TYPE_JPEGLS,
	CODEC_TYPE_MPEG4,
	CODEC_TYPE_RAWVIDEO,
	CODEC_TYPE_MSMPEG4V1,
	CODEC_TYPE_MSMPEG4V2,
	CODEC_TYPE_MSMPEG4V3,
	CODEC_TYPE_WMV1,
	CODEC_TYPE_WMV2,
	CODEC_TYPE_H263P,
	CODEC_TYPE_H263I,
	CODEC_TYPE_FLV1,
	CODEC_TYPE_SVQ1,
	CODEC_TYPE_SVQ3,
	CODEC_TYPE_DVVIDEO,
	CODEC_TYPE_HUFFYUV,
	CODEC_TYPE_CYUV,
	CODEC_TYPE_H264,
	CODEC_TYPE_INDEO3,
	CODEC_TYPE_VP3,
	CODEC_TYPE_THEORA,
	CODEC_TYPE_ASV1,
	CODEC_TYPE_ASV2,
	CODEC_TYPE_FFV1,
	CODEC_TYPE_4XM,
	CODEC_TYPE_VCR1,
	CODEC_TYPE_CLJR,
	CODEC_TYPE_MDEC,
	CODEC_TYPE_ROQ,
	CODEC_TYPE_INTERPLAY_VIDEO,
	CODEC_TYPE_XAN_WC3,
	CODEC_TYPE_XAN_WC4,
	CODEC_TYPE_RPZA,
	CODEC_TYPE_CINEPAK,
	CODEC_TYPE_WS_VQA,
	CODEC_TYPE_MSRLE,
	CODEC_TYPE_MSVIDEO1,
	CODEC_TYPE_IDCIN,
	CODEC_TYPE_8BPS,
	CODEC_TYPE_SMC,
	CODEC_TYPE_FLIC,
	CODEC_TYPE_TRUEMOTION1,
	CODEC_TYPE_VMDVIDEO,
	CODEC_TYPE_MSZH,
	CODEC_TYPE_ZLIB,
	CODEC_TYPE_QTRLE,
	CODEC_TYPE_SNOW,
	CODEC_TYPE_TSCC,
	CODEC_TYPE_ULTI,
	CODEC_TYPE_QDRAW,
	CODEC_TYPE_VIXL,
	CODEC_TYPE_QPEG,
	CODEC_TYPE_XVID,
	CODEC_TYPE_PNG,
	CODEC_TYPE_PPM,
	CODEC_TYPE_PBM,
	CODEC_TYPE_PGM,
	CODEC_TYPE_PGMYUV,
	CODEC_TYPE_PAM,
	CODEC_TYPE_FFVHUFF,
	CODEC_TYPE_RV30,
	CODEC_TYPE_RV40,
	CODEC_TYPE_VC1,
	CODEC_TYPE_WMV3,
	CODEC_TYPE_LOCO,
	CODEC_TYPE_WNV1,
	CODEC_TYPE_AASC,
	CODEC_TYPE_INDEO2,
	CODEC_TYPE_FRAPS,
	CODEC_TYPE_TRUEMOTION2,
	CODEC_TYPE_BMP,
	CODEC_TYPE_CSCD,
	CODEC_TYPE_MMVIDEO,
	CODEC_TYPE_ZMBV,
	CODEC_TYPE_AVS,
	CODEC_TYPE_SMACKVIDEO,
	CODEC_TYPE_NUV,
	CODEC_TYPE_KMVC,
	CODEC_TYPE_FLASHSV,
	CODEC_TYPE_CAVS,
	CODEC_TYPE_JPEG2000,
	CODEC_TYPE_VMNC,
	CODEC_TYPE_VP5,
	CODEC_TYPE_VP6,
	CODEC_TYPE_VP6F,
	CODEC_TYPE_TARGA,
	CODEC_TYPE_DSICINVIDEO,
	CODEC_TYPE_TIERTEXSEQVIDEO,
	CODEC_TYPE_TIFF,
	CODEC_TYPE_GIF,
	CODEC_TYPE_FFH264,
	CODEC_TYPE_DXA,
	CODEC_TYPE_DNXHD,
	CODEC_TYPE_THP,
	CODEC_TYPE_SGI,
	CODEC_TYPE_C93,
	CODEC_TYPE_BETHSOFTVID,
	CODEC_TYPE_PTX,
	CODEC_TYPE_TXD,
	CODEC_TYPE_VP6A,
	CODEC_TYPE_AMV,
	CODEC_TYPE_VB,
	CODEC_TYPE_PCX,
	CODEC_TYPE_SUNRAST,
	CODEC_TYPE_INDEO4,
	CODEC_TYPE_INDEO5,
	CODEC_TYPE_MIMIC,
	CODEC_TYPE_RL2,
	CODEC_TYPE_8SVX_EXP,
	CODEC_TYPE_8SVX_FIB,
	CODEC_TYPE_ESCAPE124,
	CODEC_TYPE_DIRAC,
	CODEC_TYPE_HIK,

	/* various PCM "codecs" */
	CODEC_TYPE_PCM_S16LE= 0x10000,
	CODEC_TYPE_PCM_S16BE,
	CODEC_TYPE_PCM_U16LE,
	CODEC_TYPE_PCM_U16BE,
	CODEC_TYPE_PCM_S8,
	CODEC_TYPE_PCM_U8,
	CODEC_TYPE_PCM_MULAW,
	CODEC_TYPE_PCM_ALAW,
	CODEC_TYPE_PCM_S32LE,
	CODEC_TYPE_PCM_S32BE,
	CODEC_TYPE_PCM_U32LE,
	CODEC_TYPE_PCM_U32BE,
	CODEC_TYPE_PCM_S24LE,
	CODEC_TYPE_PCM_S24BE,
	CODEC_TYPE_PCM_U24LE,
	CODEC_TYPE_PCM_U24BE,
	CODEC_TYPE_PCM_S24DAUD,
	CODEC_TYPE_PCM_ZORK,
	CODEC_TYPE_PCM_S16LE_PLANAR,

	/* various ADPCM codecs */
	CODEC_TYPE_ADPCM_IMA_QT= 0x11000,
	CODEC_TYPE_ADPCM_IMA_WAV,
	CODEC_TYPE_ADPCM_IMA_DK3,
	CODEC_TYPE_ADPCM_IMA_DK4,
	CODEC_TYPE_ADPCM_IMA_WS,
	CODEC_TYPE_ADPCM_IMA_SMJPEG,
	CODEC_TYPE_ADPCM_MS,
	CODEC_TYPE_ADPCM_4XM,
	CODEC_TYPE_ADPCM_XA,
	CODEC_TYPE_ADPCM_ADX,
	CODEC_TYPE_ADPCM_EA,
	CODEC_TYPE_ADPCM_G726,
	CODEC_TYPE_ADPCM_CT,
	CODEC_TYPE_ADPCM_SWF,
	CODEC_TYPE_ADPCM_YAMAHA,
	CODEC_TYPE_ADPCM_SBPRO_4,
	CODEC_TYPE_ADPCM_SBPRO_3,
	CODEC_TYPE_ADPCM_SBPRO_2,
	CODEC_TYPE_ADPCM_THP,
	CODEC_TYPE_ADPCM_IMA_AMV,
	CODEC_TYPE_ADPCM_EA_R1,
	CODEC_TYPE_ADPCM_EA_R3,
	CODEC_TYPE_ADPCM_EA_R2,
	CODEC_TYPE_ADPCM_IMA_EA_SEAD,
	CODEC_TYPE_ADPCM_IMA_EA_EACS,
	CODEC_TYPE_ADPCM_EA_XAS,

	/* AMR */
	CODEC_TYPE_AMR_NB= 0x12000,
	CODEC_TYPE_AMR_WB,

	/* RealAudio codecs*/
	CODEC_TYPE_RA_144= 0x13000,
	CODEC_TYPE_RA_288,

	/* various DPCM codecs */
	CODEC_TYPE_ROQ_DPCM= 0x14000,
	CODEC_TYPE_INTERPLAY_DPCM,
	CODEC_TYPE_XAN_DPCM,
	CODEC_TYPE_SOL_DPCM,

	CODEC_TYPE_MP2= 0x15000,
	CODEC_TYPE_MP3, ///< preferred ID for decoding MPEG audio layer 1, 2 or 3
	CODEC_TYPE_AAC,

#if LIBAVCODEC_VERSION_INT < ((52<<16)+(0<<8)+0)
	CODEC_TYPE_MPEG4AAC,
#endif

	CODEC_TYPE_AC3,
	CODEC_TYPE_DTS,
	CODEC_TYPE_VORBIS,
	CODEC_TYPE_DVAUDIO,
	CODEC_TYPE_WMAV1,
	CODEC_TYPE_WMAV2,
	CODEC_TYPE_MACE3,
	CODEC_TYPE_MACE6,
	CODEC_TYPE_VMDAUDIO,
	CODEC_TYPE_SONIC,
	CODEC_TYPE_SONIC_LS,
	CODEC_TYPE_FLAC,
	CODEC_TYPE_MP3ADU,
	CODEC_TYPE_MP3ON4,
	CODEC_TYPE_SHORTEN,
	CODEC_TYPE_ALAC,
	CODEC_TYPE_WESTWOOD_SND1,
	CODEC_TYPE_GSM, ///< as in Berlin toast format
	CODEC_TYPE_QDM2,
	CODEC_TYPE_COOK,
	CODEC_TYPE_TRUESPEECH,
	CODEC_TYPE_TTA,
	CODEC_TYPE_SMACKAUDIO,
	CODEC_TYPE_QCELP,
	CODEC_TYPE_WAVPACK,
	CODEC_TYPE_DSICINAUDIO,
	CODEC_TYPE_IMC,
	CODEC_TYPE_MUSEPACK7,
	CODEC_TYPE_MLP,
	CODEC_TYPE_GSM_MS, /* as found in WAV */
	CODEC_TYPE_ATRAC3,
	CODEC_TYPE_VOXWARE,
	CODEC_TYPE_APE,
	CODEC_TYPE_NELLYMOSER,
	CODEC_TYPE_MUSEPACK8,
	CODEC_TYPE_SPEEX,
	CODEC_TYPE_WMAVOICE,
	CODEC_TYPE_WMAPRO,
	CODEC_TYPE_WMALOSSLESS,

	/* subtitle codecs */
	CODEC_TYPE_DVD_SUBTITLE= 0x17000,
	CODEC_TYPE_DVB_SUBTITLE,
	CODEC_TYPE_TEXT,  ///< raw UTF-8 text
	CODEC_TYPE_XSUB,
	CODEC_TYPE_SSA,
	CODEC_TYPE_MOV_TEXT,

	/* other specific kind of codecs (generally used for attachments) */
	CODEC_TYPE_TTF= 0x18000,

	CODEC_TYPE_MPEG2TS= 0x20000, /**< _FAKE_ codec to indicate a raw MPEG-2 TS
								   * stream (only used by libavformat) */
	CODEC_TYPE_WMV9 = 0x30000,
	CODEC_TYPE_WMA9,
	CODEC_TYPE_WVP2,
	CODEC_TYPE_WMSV,/* Windows media screen video codec */
};
enum STREAM_TYPE
{
	STREAMT_NULL,
	STREAMT_VIDEO,
	STREAMT_AUDIO,
	STREAMT_TXT,
};
enum STREAM_FORMAT
{
	STREAMF_NULL,
	STREAMF_PS,
	STREAMF_TS,
	STREAMF_ES,
	STREAMF_ES_ONLYVIDEO,
	STREAMF_PSOVERRTP,
};

extern "C" {
	GENTEKPLATFORMSTREAM_API	unsigned long	GS_GetVersion();
	GENTEKPLATFORMSTREAM_API	unsigned long	GS_GetLastError(unsigned long nPort);
	//播放流程和文件写入接口
	GENTEKPLATFORMSTREAM_API	unsigned long	GS_InitStream(unsigned long nVideoCodec,unsigned long nAudioCodec,unsigned long *nPort);
	GENTEKPLATFORMSTREAM_API	unsigned long	GS_OpenStream(unsigned long nPort,unsigned long nStreamFormat,unsigned long nMode);
	GENTEKPLATFORMSTREAM_API	unsigned long	GS_OpenFile(unsigned long nPort,char *pFileName,bool bRead=false);
	GENTEKPLATFORMSTREAM_API	unsigned long	GS_PlayStream(unsigned long nPort,HWND hWnd);
	GENTEKPLATFORMSTREAM_API	unsigned long	GS_StopStream(unsigned long nPort);
	GENTEKPLATFORMSTREAM_API	unsigned long	GS_CloseStream(unsigned long nPort);
	GENTEKPLATFORMSTREAM_API	unsigned long	GS_CloseFile(unsigned long nPort);
	GENTEKPLATFORMSTREAM_API	unsigned long	GS_UnInitStream(unsigned long nPort);
	
	//播放功能接口
	GENTEKPLATFORMSTREAM_API	unsigned long	GS_PauseStream(unsigned long nPort);
	GENTEKPLATFORMSTREAM_API	unsigned long	GS_SpeedStream(unsigned long nPort,unsigned long nSpeed);
	GENTEKPLATFORMSTREAM_API	unsigned long	GS_PlayStreamOneByOne(unsigned long nPort);
	GENTEKPLATFORMSTREAM_API	unsigned long	GS_SetStreamOverCallBack(unsigned long nPort,bool (CALLBACK *fOnStreamOver)(unsigned long nPort) = NULL);
	GENTEKPLATFORMSTREAM_API	unsigned long	GS_StartListenStreamOverCallBack(unsigned long nPort);//不再向GStream 送数据再调用此接口
	GENTEKPLATFORMSTREAM_API	unsigned long	GS_SetTimeNotifyCallBack(unsigned long nPort,bool (CALLBACK *fOnTime)(unsigned long nPort,unsigned __int64 nTime) = NULL);

	//数据接收接口	
	/////GB28281国标数据流
	GENTEKPLATFORMSTREAM_API	unsigned long	GS_InputPSOverRTPStream(unsigned long nPort,unsigned char * pBuf,unsigned long nSize);
	/////PS流 国标
	GENTEKPLATFORMSTREAM_API	unsigned long	GS_InputPSStream(unsigned long nPort,unsigned char * pBuf,unsigned long nSize);
	/////TS流 H3C
	GENTEKPLATFORMSTREAM_API	unsigned long	GS_InputTSStream(unsigned long nPort,unsigned char * pBuf,unsigned long nSize);
	/////ES流 
	GENTEKPLATFORMSTREAM_API	unsigned long	GS_InputESStream(unsigned long nPort,unsigned char * pBuf,unsigned long nSize,unsigned long nStreamType,unsigned __int64 nTime/*us*/,bool HaveStartCode=true);
	/////写入文件
	GENTEKPLATFORMSTREAM_API	unsigned long	GS_WriteFile(unsigned long nPort,unsigned char * pBuf,unsigned long nSize);

	//渲染接口
	GENTEKPLATFORMSTREAM_API	unsigned long	GS_ShowVideoUpDown(unsigned long nPort,bool bUpDown);
	GENTEKPLATFORMSTREAM_API	unsigned long	GS_SetShowSrcPos(unsigned long nPort,int x,int y,int sx,int sy,bool bshow);
	GENTEKPLATFORMSTREAM_API	unsigned long	GS_SetAudioVolmue(unsigned long nPort,unsigned long nVolume);
	GENTEKPLATFORMSTREAM_API	unsigned long	GS_GetAudioVolmue(unsigned long nPort,unsigned long *pnVolume);
	GENTEKPLATFORMSTREAM_API	unsigned long	GS_CloseAudio(unsigned long nPort);
	GENTEKPLATFORMSTREAM_API	unsigned long	GS_OpenAudio(unsigned long nPort);

	//其他功能接口
	GENTEKPLATFORMSTREAM_API	unsigned long	GS_TakePic(unsigned long nPort,unsigned char *pFileName);
	GENTEKPLATFORMSTREAM_API	unsigned long	GS_SetText(unsigned long nPort,unsigned char * pText,int x,int y,int font_size,unsigned int nIndex,bool bShow);
	

}