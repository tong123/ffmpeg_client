#include "msendian.h"

#if defined Q_WS_WIN
#pragma comment (lib,"Ws2_32.lib")
#include <winsock2.h>

#elif defined Q_OS_LINUX
#include<netinet/in.h>

#endif
//using namespace msendian;
#include<netinet/in.h>

unsigned short net_to_host_s( unsigned short s_val )
{
    return ntohs( s_val );
}


unsigned short host_to_net_s( unsigned short s_val )
{
	return htons( s_val );
}


unsigned long net_to_host_l( unsigned long l_val )
{
	return ntohl( l_val );
}


unsigned long host_to_net_l( unsigned long l_val )
{
	return htonl( l_val );
}

