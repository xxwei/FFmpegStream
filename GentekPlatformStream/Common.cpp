#include "stdafx.h"
#include "Common.h"

__int32 ReadInt16(unsigned char * lp)
{		
	return lp[1]+(lp[0]<<8);
}
__int32 ReadInt24(unsigned char * lp)
{
	return lp[2]+(lp[1]<<8)+(lp[0]<<16);
}
__int32 ReadInt32(unsigned char * lp)
{		
	return lp[3]+(lp[2]<<8)+(lp[1]<<16)+(lp[0]<<24);	
}
__int64	ReadInt64(unsigned char * lp)
{
	__int64 re = 0;
	for(int i=0;i<8;i++)
	{
		re+=lp[i];
		if(i<7)
			re = re<<8;
	}
	return re;
}
__int64	ReReadInt64(unsigned char * lp)
{
	__int64 re = 0;
	for(int i=0;i<8;i++)
	{
		re+=lp[7-i];
		if(i<7)
			re = re<<8;
	}
	return re;
}

//Histogram Equalization
void HE(unsigned char *buf ,int width,int height,int channel)
{
	unsigned int r[256] = {0};
	unsigned int g[256] = {0};
	unsigned int b[256] = {0};
	unsigned int a[256] = {0};
	float fps_r[256] = {0.0};
	float fps_g[256] = {0.0};
	float fps_b[256] = {0.0};
	float fps_a[256] = {0.0};

	unsigned int offset = 0;
	//统计
	int wh = 0;

#pragma omp parallel for
	for(int i = 0;i<height;i++)
	{
		unsigned int cur = 0;
		unsigned int offset = 0;
		for(int j=0;j<width;j++)
		{
			offset = i*width*channel+j*channel;
			cur = buf[offset];
			b[cur]++;
			cur = buf[offset+1];
			g[cur]++;
			cur = buf[offset+2];
			r[cur]++;
			cur = buf[offset+3];
			a[cur]++;
		}
	}

	//密度计算
	for(int m =0;m<256;m++)
	{
		if(m==0)
		{
			fps_r[m] = r[m]/(width*height*1.0f);
			fps_g[m] = g[m]/(width*height*1.0f);
			fps_b[m] = b[m]/(width*height*1.0f);
			fps_a[m] = a[m]/(width*height*1.0f);
		}
		else
		{
			fps_r[m] = r[m]/(width*height*1.0f)+fps_r[m-1];
			fps_g[m] = g[m]/(width*height*1.0f)+fps_g[m-1];
			fps_b[m] = b[m]/(width*height*1.0f)+fps_b[m-1];
			fps_a[m] = a[m]/(width*height*1.0f)+fps_a[m-1];
		}
	}
	//累计分布取整
#pragma omp parallel for
	for(int n =0;n<256;n++)
	{
		r[n] = (int)(255.0f*fps_r[n]+0.5f);
		g[n] = (int)(255.0f*fps_g[n]+0.5f);
		b[n] = (int)(255.0f*fps_b[n]+0.5f);
		a[n] = (int)(255.0f*fps_a[n]+0.5f);
	}
	//灰度映射
#pragma omp parallel for
	for(int i = 0;i<height;i++)
	{
		unsigned int cur = 0;
		unsigned int offset = 0;
		for(int j=0;j<width;j++)
		{			
			offset = i*width*channel+j*channel;
			cur = buf[offset];
			buf[offset] = b[cur];
			cur = buf[offset+1];
			buf[offset+1] = g[cur];
			cur = buf[offset+2];
			buf[offset+2] = r[cur];
			cur = buf[offset+3];
			buf[offset+3] = a[cur];
		}
	}
}
void FFT(complex<double> *In,complex<double> *Out,int r)
{
	long count;
	double angle;
	count = 1<<r;
	complex<double> *W,*X1,*X2,*X;
	W = new complex<double>[count/2];
	X1 = new complex<double>[count];
	X2 = new complex<double>[count];
	//计算加权系数
	for(int i=0;i<count/2;i++)
	{
		angle = -i*PI*2/count;
		W[i] = complex<double>(cos(angle),sin(angle));
	}
	memcpy(X1,In,sizeof(complex<double>)*count);
	for(int k=0;k<r;k++)
	{
		for(int j=0;j<1<<k;j++)
		{
			int bfsize = 1<<(r-k);
			for(int i=0;i<bfsize/2;i++)
			{
				int p = j*bfsize;
				X2[i+p] = X1[i+p]+X1[i+p+bfsize/2];
				double re = real(X1[i+p]-X1[i+p+bfsize/2])*real(W[i*(1<<k)])-imag(X1[i+p]-X1[i+p+bfsize/2])*imag(W[i*(1<<k)]);
				double im = real(X1[i+p]-X1[i+p+bfsize/2])*imag(W[i*(1<<k)])+imag(X1[i+p]-X1[i+p+bfsize/2])*real(W[i*(1<<k)]);
				X2[i+p+bfsize/2] = complex<double>(re,im);
			}
		}
		X = X1;
		X1 = X2;
		X2 = X;
	}
	for(int j=0;j<count;j++)
	{
		int p = 0;
		for(int i=0;i<r;i++)
		{
			if(j&(1<<i))
			{
				p+=1<<(r-i-1);
			}
		}
		Out[j]=X1[p];
	}
	delete W;
	delete X1;
	delete X2;
}
void IFFT(complex<double> *In,complex<double> *Out,int r)
{
	unsigned long count;
	unsigned long	i;
	complex<double> *X;
	//计算福利也变换点数
	count = 1<<r;
	//分配运算所需存储器
	X = new complex<double>[count];
	//将频域点写入X
	memcpy(X,In,sizeof(complex<double> )*count);
	//求共轭
	for(i = 0;i<count;i++)
	{
		X[i] = complex<double>(real(X[i]),0-imag(X[i]));
	}

	FFT(X,Out,r);
	//求时域点的共轭
	for(i=0;i<count;i++)
	{
		Out[i]=complex<double>(real(Out[i])/count,0-imag(Out[i])/count);
	}
	delete X;

}
void HighPass(unsigned char * lpSrc,int hz,int x,int y,long lineBytes,int offset,int width,int mc)
{
	long i,j;
	long w,h;

	int wp,hp;

	//赋初值
	w = 1;
	h = 1;
	wp = 0;//次幂
	hp = 0;
	unsigned char *lp;
	lp = lpSrc;

	int dp = lineBytes/mc;

	wp = 3;
	hp = 3;
	w = mc;
	h = mc;

	while(w*2<=mc)
	{
		w*=2;
		wp++;
	}
	while(h*2<=mc)
	{
		h*=2;
		hp++;
	}
	complex<double> *TD = new complex<double>[w*h];
	complex<double> *FD = new complex<double>[w*h];

	for(i=0;i<h;i++)
	{
		for(j=0;j<w;j++)
		{
			lp = lpSrc+(i+y*mc)*width*lineBytes+x*lineBytes+j*dp+offset;
			//给时域赋值
			TD[j+w*i] = complex<double>((double)(*lp),0);
		}
	}
	for(i=0;i<h;i++)
	{
		//对Y方向进行快速傅里叶变换
		FFT(&TD[w*i],&FD[w*i],wp);
	}
	//保存变换结构
	for(i=0;i<h;i++)
	{
		for(j=0;j<w;j++)
		{
			TD[i+h*j]=FD[j+w*i];
		}
	}
	for(i=0;i<w;i++)
	{
		FFT(&TD[i*h],&FD[i*h],hp);
	}

	//高通滤波
	for(i=0;i<h;i++)
	{
		for(j=0;j<w;j++)
		{
			if(hz<sqrt((double)(i*i,j*j)))
			{
				FD[i*w+j] = complex<double>(0,0);
			}
		}
	}

	//还原
	for(i=0;i<h;i++)
	{
		//对Y方向进行快速傅里叶逆变换
		IFFT(&FD[w*i],&TD[w*i],wp);
	}
	for(i=0;i<h;i++)
	{
		for(j=0;j<w;j++)
		{
			FD[i+h*j] = TD[j+w*i];
		}
	}

	for(i=0;i<w;i++)
	{
		//对X方向进行的快速逆变换
		IFFT(&FD[i*h],&TD[i*h],hp);
	}

	for(i=0;i<h;i++)
	{
		for(j=0;j<w;j++)
		{
			*(lpSrc+(i+y*mc)*width*lineBytes+x*lineBytes+j*dp+offset)=(unsigned char)real(TD[i*w+j]);
		}
	}
	delete []TD;
	delete []FD;

}
void LowPass(unsigned char * lpSrc,int hz,int x,int y,long lineBytes,int offset,int width,int mc)
{
	long i,j;
	long w,h;

	int wp,hp;

	//赋初值
	w = 1;
	h = 1;
	wp = 0;//次幂
	hp = 0;
	unsigned char *lp;
	lp = lpSrc;

	int dp = lineBytes/mc;

	wp = 3;
	hp = 3;
	w = mc;
	h = mc;

	while(w*2<=mc)
	{
		w*=2;
		wp++;
	}
	while(h*2<=mc)
	{
		h*=2;
		hp++;
	}
	complex<double> *TD = new complex<double>[w*h];
	complex<double> *FD = new complex<double>[w*h];

	for(i=0;i<h;i++)
	{
		for(j=0;j<w;j++)
		{
			lp = lpSrc+(i+y*mc)*width*lineBytes+x*lineBytes+j*dp+offset;
			//给时域赋值
			TD[j+w*i] = complex<double>((double)(*lp),0);
		}
	}
	for(i=0;i<h;i++)
	{
		//对Y方向进行快速傅里叶变换
		FFT(&TD[w*i],&FD[w*i],wp);
	}
	//保存变换结构
	for(i=0;i<h;i++)
	{
		for(j=0;j<w;j++)
		{
			TD[i+h*j]=FD[j+w*i];
		}
	}
	for(i=0;i<w;i++)
	{
		FFT(&TD[i*h],&FD[i*h],hp);
	}

	//低通滤波
	for(i=0;i<h;i++)
	{
		for(j=0;j<w;j++)
		{
			if(hz>sqrt((double)(i*i,j*j)))
			{
				FD[i*w+j] = complex<double>(0,0);
			}
		}
	}

	//还原
	for(i=0;i<h;i++)
	{
		//对Y方向进行快速傅里叶逆变换
		IFFT(&FD[w*i],&TD[w*i],wp);
	}
	for(i=0;i<h;i++)
	{
		for(j=0;j<w;j++)
		{
			FD[i+h*j] = TD[j+w*i];
		}
	}

	for(i=0;i<w;i++)
	{
		//对X方向进行的快速逆变换
		IFFT(&FD[i*h],&TD[i*h],hp);
	}

	for(i=0;i<h;i++)
	{
		for(j=0;j<w;j++)
		{
			*(lpSrc+(i+y*mc)*width*lineBytes+x*lineBytes+j*dp+offset)=(unsigned char)real(TD[i*w+j]);
			//char str[128];
			//sprintf(str,"当前处理位置 %d  %d  %d %d\n",x, y,offset,(i+y)*width*lineBytes+x*lineBytes+j*dp+offset);
			//OutputDebugStringW(str);
		}
	}

	delete []TD;
	delete []FD;
}

char MakeFrameHeader(char Fi,char Fh)
{
	char Sh = (Fi&0xE0)|(Fh&0x1F);
	return Sh;
}