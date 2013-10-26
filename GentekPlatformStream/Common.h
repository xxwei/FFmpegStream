

#ifndef COMMON_
#define COMMON_

#define PACK_START_CODE             ((unsigned int)0x000001ba)
#define SYSTEM_HEADER_START_CODE    ((unsigned int)0x000001bb)
#define SEQUENCE_END_CODE           ((unsigned int)0x000001b7)
#define PACKET_START_CODE_MASK      ((unsigned int)0xffffff00)
#define PACKET_START_CODE_PREFIX    ((unsigned int)0x00000100)
#define ISO_11172_END_CODE          ((unsigned int)0x000001b9)


struct offsetidx
{
	unsigned int spos;
	unsigned int epos;
	unsigned __int64 pltime;	
};
typedef list<offsetidx>			idxlist;

__int32 ReadInt16(unsigned char * lp);
__int32 ReadInt24(unsigned char * lp);
__int32 ReadInt32(unsigned char * lp);
__int64	ReadInt64(unsigned char * lp);
__int64	ReReadInt64(unsigned char * lp);

//#define PI		3.1414926535
void HE(unsigned char *buf, int width,int height,int channel);
void FFT(complex<double> *In,complex<double> *Out,int r);
void IFFT(complex<double> *In,complex<double> *Out,int r);
void HighPass(unsigned char * lpSrc,int hz,int x,int y,long lineBytes,int offset,int width,int mc=8);
void LowPass(unsigned char * lpSrc,int hz,int x,int y,long lineBytes,int offset,int width,int mc=8);

char MakeFrameHeader(char Fi,char Fh);
//read_packet 返回值
/*
*	0 没有读到数据包
*	1 读到一个音频包
*	2 读到一个视频包
*	3 读到一个音频包的起始一块 
*	4 读到一个音频包的中间一块
*	5 读到一个音频包的最后一块 
*	6 读到一个视频包的起始一块
*	7 读到一个视频包的中间一块
*	8 读到一个视频包的最后一块
*	9 其他数据 待定
*   10 跳过一下无用数据
*/
class demuxer
{
public:
	virtual int read_packet(void *lp,unsigned int len,offsetidx &oi,unsigned int *offset){return 0;};
};
#endif


