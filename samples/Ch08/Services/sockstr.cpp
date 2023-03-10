//
// SOCKSTR.CPP SocketTypeToString()
//             与えられたソケットタイプの文字列(const)を返す
//

#include "stdafx.h"

////////////////////////////////////////////////////////////
LPCSTR SocketTypeToString(int nSocketType)
{
	static char szBuf[20];
	
	switch(nSocketType)
	{
		case SOCK_STREAM:
			return("SOCK_STREAM");
		case SOCK_DGRAM:
			return("SOCK_DGRAM");
		case SOCK_RAW:
			return("SOCK_RAW");
		case SOCK_RDM:
			return("SOCK_RDM");
		case SOCK_SEQPACKET:
			return("SOCK_SEQPACKET");
		default:
			wsprintf(szBuf, "%d", nSocketType);
			return(szBuf);
	}
}

