#ifndef PS_OVER_RTP
#define PS_OVER_RTP
#include "../Common.h"



class ps_over_rtp:public demuxer
{

	int read_packet(void *lp,unsigned int len,offsetidx &oi,unsigned int *offset)
	{
		
		return 9;
	}
};
#endif