#ifndef PS_
#define PS_
#include "../Common.h"







class ps_read:public demuxer
{
public:
	int  read_packet(void *lp,unsigned int len,offsetidx &oi,unsigned int *offset)
	{
		*offset = 0;
		unsigned char *lpdata = (unsigned char *)lp;
		int pos = find_start_code(lpdata,len,0);
		offsetidx  oix;
		if(pos>=0)
		{

			int pos2 = find_start_code(lpdata,len,pos+4);
			if(pos2>0)
			{
				//char str[128];
				//sprintf_s(str,"-------一find ps stream %0x \n",lpdata[pos+3]);
				//OutputDebugStringA(str);
				if(lpdata[pos+3]==0xBA)
				{
					//OutputDebugStringA("read ps header\n");
					read_ps_header(lpdata+pos);
					*offset = pos2;
					return 10;
				}
				else if(lpdata[pos+3]==0xBB)
				{
					//OutputDebugStringA("read ps sys header\n");
					read_ps_system_header(lpdata+pos);
					*offset = pos2;
					return 10;
				}
				else if(lpdata[pos+3]==0xBC)
				{
					//OutputDebugStringA("read ps bc sys header\n");
					//read_ps_system_header(lpdata+pos);
					*offset = pos2;
					return 10;
				}
				else if(lpdata[pos+3]==m_audio_stream_id)
				{
					//OutputDebugStringA("read pes header for audio \n");
					int pes_len = read_ps_pes(lpdata+pos,oix);
					if(pes_len==0)
					{
						oi.spos = oix.spos+pos;
						oi.epos = oix.epos+pos;
						oi.pltime = oix.pltime;
						if(oi.epos>len)
						{
							*offset = pos;
							return 0;
						}
						*offset = oi.epos;
						return 1;
					}
					else
					{
						*offset = pes_len+6;
						return 10;
					}

				}
				else if(lpdata[pos+3]==m_video_stream_id)
				{
					//OutputDebugStringA("read pes header for video \n");
					int pes_len = read_ps_pes(lpdata+pos,oix);
					if(pes_len==0)
					{
						oi.spos = oix.spos+pos;
						oi.epos = oix.epos+pos;
						oi.pltime = oix.pltime;
						if(oi.epos>len)
						{
							*offset = pos;
							return 10;
						}
						*offset = oi.epos;
						if(lpdata[oi.epos+3]==m_video_stream_id)
						{
							if(m_b_ps_over)
							{
								m_b_ps_over = false;
								return 6;
							}
							return 7;
						}
						else
						{
							if(m_b_ps_over==false)
							{
								m_b_ps_over = true;
								return 8;
							}
							else
							{
								return 2;
							}	
						}
					}
					else
					{
						*offset = pes_len+6;
						return 10;
					}
				}
				else
				{
					*offset = pos2;
					OutputDebugStringA("read data error \n");
				}
			
			}
			else
			{
				*offset = pos;
				return 0;
			}
		}
		else
		{
			*offset = len;
			return 0;
		}
		
		return 0;
	}
	int set_payload(int ntype,int payload)
	{
		return 0;
	}
private:
	unsigned char	m_video_stream_id;
	unsigned char	m_audio_stream_id;
	LONGLONG		m_system_clock_referance;
	bool			m_b_ps_over;
	int read_ps_header(unsigned char *lp)
	{
		m_system_clock_referance = 0;
		m_video_stream_id = 0xE0;
		m_audio_stream_id = 0xC0;
		m_b_ps_over = true;
		return 0;
	}
	int read_ps_system_header(unsigned char *lp)
	{
		m_video_stream_id = 0xE0;
		m_audio_stream_id = 0xC0;
		return 0;
	}
	//统一为PTS 或者为DTS 只能选择其一
	int read_ps_pes(unsigned char *lp,offsetidx &oi)
	{
		
		int pes_len = ReadInt16(lp+4);
		int pes_header_len = lp[8];
		oi.spos = pes_header_len + 6 + 3;
		oi.epos = pes_len+6;
		
		//char str[128];
		//sprintf_s(str,"-------一pes len and header len %d  %d\n",pes_len,pes_header_len);
		//OutputDebugStringA(str);
		if(lp[7]&0x80)
			oi.pltime = read_pes_pts(lp+9);
		else
			oi.pltime = 0;
		if(pes_header_len>pes_len)
		{
			return pes_len;
		}
		return 0;
	}
	int find_start_code(unsigned char *lp,unsigned int len,int offset)
	{
		unsigned int nCode	= 0;
		unsigned int nPos	= offset;
		while(nPos+4<len)
		{
			nCode = ReadInt24(lp+nPos);
			if(nCode==0x000001)
			{
				/*
				if(nPos>0)
				{
					if(lp[nPos-1]!=0)
					{
						return nPos;
					}
				}
				else
				*/
				{
					return nPos;
				}
			}
			nPos++;
		}
		return -1;
	}
	unsigned __int64 read_pes_pts(unsigned char *lp)
	{
		
		return (int64_t)(*lp & 0x0e) << 29 |
			(ReadInt16(lp+1) >> 1) << 15 |
			ReadInt16(lp+3) >> 1;
	}
	
};
#endif