#ifndef TS_
#define TS_

#include "../Common.h"

struct ts_header
{
	unsigned int	pid;
	unsigned char	counter;
	bool			start_indicator;
	unsigned int	offset;
};
class ts_read:public demuxer
{
public:
	ts_read()
	{
		m_video_pid = 0;
		m_pmt_pid = 0;
		m_audio_pid = 0;
		read_ts_pat(NULL);
		read_ts_pmt(NULL);
	}
	~ts_read()
	{

	}
	int read_packet(void *lp,unsigned int len,offsetidx &oi,unsigned int *offset)
	{
		
		*offset = 0;
		if(len<=188)
		{
			return 0;
		}
		unsigned char *lpdata = (unsigned char *)lp;
		int  pos = find_start_code(lpdata,len,0);
		if(pos>=0)
		{
			ts_header	th;
			*offset = pos+188;
			int len = read_ts_header(lpdata+pos,th);
			if(len==0)
			{
				return 0;
			}
			char str[128];
			sprintf_s(str,"-------一read ts packet pid=%0x \n",th.pid);
			//OutputDebugStringA(str);
			if(th.pid==0)//PAT
			{
				read_ts_pat(lpdata+pos);
				return 0;
			}
			else if(th.pid==m_pmt_pid)
			{
				read_ts_pmt(lpdata+pos);
				return 0;
			}
			else if(th.pid==m_video_pid)
			{
				if(th.start_indicator)
				{
					read_ts_pes(lpdata+th.offset,m_videoix);
					oi.spos = m_videoix.spos+pos+th.offset;
					oi.epos = pos+188;
					oi.pltime = m_videoix.pltime;
					return 6;
				}
				else
				{
					//if(len==182)//没有好的方法判断 先用这种方法判断h3c TS流的
					if(len==184)//这是SCOM流
					{
						oi.spos = pos+th.offset;
						oi.epos = pos+188;
						oi.pltime = m_videoix.pltime;
						return 7;
					}
					else
					{
						oi.spos = pos+th.offset;
						oi.epos = pos+188;
						oi.pltime = m_videoix.pltime;
						return 8;
					}
					
				}
			}
			else if(th.pid==m_audio_pid)
			{
				if(th.start_indicator)
				{
					read_ts_pes(lpdata+th.offset,m_audioix);
					oi.spos = m_audioix.spos+pos+th.offset;
					oi.epos = pos+188;
					oi.pltime = m_audioix.pltime;
					return 3;
				}
				else
				{
				
					oi.spos = pos+th.offset;
					oi.epos = pos+188;
					oi.pltime = m_audioix.pltime;
					if(len==182)
					{
						return 4;
					}
					else
					{
						return 5;
					}
					
				}
			}
			else
			{
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
private:
	offsetidx		m_audioix;
	offsetidx		m_videoix;
	unsigned char	m_video_stream_id;
	unsigned char	m_audio_stream_id;
	unsigned int	m_video_pid;
	unsigned int	m_audio_pid;
	unsigned int	m_pmt_pid;
	LONGLONG		m_system_clock_referance;
	int find_start_code(unsigned char *lp,unsigned int len,int offset)
	{
		int sync_byte = 0x47;
		int nPos =offset;
		while(nPos+188<len)
		{
			if(lp[nPos]==0x47)
			{
				if(lp[nPos+188]==0x47)
				{
					return nPos;
				}
			}
			nPos++;
		}
		return -1;
	}
	//
	int read_ts_header(unsigned char *lp,ts_header &th)
	{
		unsigned char			tsc = 0; //加密控制
		unsigned char			afc = 0; //调整字段控制  00 ISO保留  01无调整字段 10仅包含调整字段  11
		tsc = (lp[3]&0xC0)>>6;
		afc = (lp[3]&0x30)>>4;
		th.counter = lp[3]&0x0F;
		unsigned short pid2 = ReadInt16(lp+1);
		th.pid = pid2&0x0FFF;
		th.start_indicator = pid2&0x4000;
		if(afc==0x01)
		{
			th.offset = 4;
			return 184;
		}
		else if(afc==0x03)
		{
			th.offset = lp[4]+5;
			return 188-lp[4]-5;
		}
		return 0;
	}
	int read_ts_pat(unsigned char *lp)
	{
		m_pmt_pid = 0x62;
		return 0;
	}
	int read_ts_pmt(unsigned char *lp)
	{
		//m_video_pid = 0x65;
		//m_audio_pid = 0x101;
		m_video_pid=0x21;
		m_audio_pid=0x22;
		return 0;
	}
	//统一为PTS 或者为DTS 只能选择其一
	int read_ts_pes(unsigned char *lp,offsetidx &oi)
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
	unsigned __int64 read_pes_pts(unsigned char *lp)
	{

		return (int64_t)(*lp & 0x0e) << 29 |
			(ReadInt16(lp+1) >> 1) << 15 |
			ReadInt16(lp+3) >> 1;
	}
};
#endif