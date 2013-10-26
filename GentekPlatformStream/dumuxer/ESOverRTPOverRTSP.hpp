#ifndef ESOVER_RTP_OVER_RTSP
#define ESOVER_RTP_OVER_RTSP

#include "../Common.h"

class es_over_rtp_over_rtsp:public demuxer
{
public:
	es_over_rtp_over_rtsp()
	{
		m_video_payload = 0;
		m_audio_payload = 0;

	}
	int read_packet(void *lp,unsigned int len,offsetidx &oi,unsigned int *offset)
	{
		if(len>20)
		{
			unsigned char *lpdata = (unsigned char *)lp;
			int packet_len = read_packet_len(lpdata);
			//char str[128];
			//sprintf_s(str,"-------发现数据数据包长度为 %d\n",packet_len);
			//OutputDebugStringA(str);
			*offset= packet_len+4;
			oi.spos = 16; //这里可能不准确 rtp没有扩展头的情况下才是的
			oi.epos = packet_len+4;
			oi.pltime = read_rtp_timestamp(lpdata+4);
			if(read_rtp_payload(lpdata+4)==m_video_payload)
			{				
				if((lpdata[oi.spos]&0x1F)==0x1C) //目前先支持FU-A
				{
					if((lpdata[oi.spos+1]&0x80)>>7) //帧头
					{
						char nal_type = MakeFrameHeader(lpdata[oi.spos],lpdata[oi.spos+1]);
						lpdata[oi.spos+1] = nal_type;
						lpdata[oi.spos] = 1;
						lpdata[oi.spos-1] = 0;
						lpdata[oi.spos-2] = 0;
						lpdata[oi.spos-3] = 0;
						oi.spos -= 3;
						m_video_packet_max_len = packet_len;
						return 6;
					}
					else
					{
						if(m_video_packet_max_len==packet_len)
						{
							oi.spos += 2;
							return 7;
						}
						else
						{
							oi.spos += 2;
							return 8;
						}
						
					}
				}
				else if((lpdata[16]&0x1F)<23)
				{
					//这里要还要加start_code
					lpdata[oi.spos-1] = 1;
					lpdata[oi.spos-2] = 0;
					lpdata[oi.spos-3] = 0;
					lpdata[oi.spos-4] = 0;
					oi.spos -= 4;
					return 2;
				}
			}
			else if(read_rtp_payload(lpdata+4)==m_audio_payload)
			{
				return 1;
			}
			else
			{
				return 9;
			}

		}
		else
		{
			*offset=0;
			return 0;
		}
		*offset=0;
		return 0;
	}
	int set_payload(int ntype,int payload)
	{
		if(STREAMT_VIDEO==ntype)
		{
			m_video_payload = payload;
		}
		if(STREAMT_AUDIO==ntype)
		{
			m_audio_payload = payload;
		}
		return 0;
	}
private:
	int read_packet_len(unsigned char *lp)
	{
		return ReadInt16(lp+2);
	}
	int read_rtp_payload(unsigned char *lp)
	{
		int payload_type;
		int seq = ReadInt16(lp);
		payload_type = seq&0x007F;
		return payload_type;
	}
	int read_rtp_timestamp(unsigned char *lp)
	{
		return ReadInt32(lp+4);
	}
protected:
	unsigned int m_video_payload;
	unsigned int m_audio_payload;
	unsigned int m_video_packet_max_len;
};
#endif