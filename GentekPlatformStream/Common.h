

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
//read_packet ����ֵ
/*
*	0 û�ж������ݰ�
*	1 ����һ����Ƶ��
*	2 ����һ����Ƶ��
*	3 ����һ����Ƶ������ʼһ�� 
*	4 ����һ����Ƶ�����м�һ��
*	5 ����һ����Ƶ�������һ�� 
*	6 ����һ����Ƶ������ʼһ��
*	7 ����һ����Ƶ�����м�һ��
*	8 ����һ����Ƶ�������һ��
*	9 �������� ����
*   10 ����һ����������
*/
class demuxer
{
public:
	virtual int read_packet(void *lp,unsigned int len,offsetidx &oi,unsigned int *offset){return 0;};
};
#endif


