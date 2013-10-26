#ifndef NALU_H264
#define NALU_H264

#include "../Common.h"
char nalu_start_code[]={0,0,0,1} ;

class nalu_h264_es:public demuxer
{
public:
	int read_packet(void *lp,unsigned int len,offsetidx &oi,unsigned int *offset)
	{
		unsigned __int64	timet = 0;
		*offset = 0;
		unsigned char *lpdata = (unsigned char *)lp;
		int pos = find_start_code(lpdata,len,0);
		if(pos>=0)
		{
			int pos2 = find_start_code(lpdata,len,pos+4);
			if(pos2>0)
			{
				if((lpdata[pos+4]&0x1F)==7)
				{
					int pos3 = find_start_code(lpdata,len,pos2+4);
					if(pos3>0)
					{
						oi.spos = pos;
						oi.epos = pos3;
						oi.pltime = timet;
						*offset  = pos3;
						return 2;
					}
					else
					{
						*offset = pos;
						return 0;
					}
				}
				else
				{
					oi.spos = pos;
					oi.epos = pos2;
					oi.pltime = timet;
					*offset  = pos2;
					return 2;
				}
				
			}
			*offset = pos;
			return 0;
		}
		*offset = 0;
		return 0;
	}
private:
	int find_start_code(unsigned char *lp,unsigned int len,int offset)
	{
		unsigned int nCode	= 0;
		unsigned int nPos	= offset;
		while(nPos+4<len)
		{
			nCode = ReadInt32(lp+nPos);
			if(nCode==0x00000001)
			{
				return nPos;
			}
			nPos++;
		}
		return -1;
	}
};

#endif
