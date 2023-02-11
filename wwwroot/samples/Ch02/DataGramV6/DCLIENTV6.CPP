//
// DClientV6.cpp
//
// きわめて単純で、このままでは実用性の薄いデータグラムクライアント例。
// DServerV6.cppとの連携で動作します。
//
// プログラムは、コマンドラインで指定したサーバーとポートに接続を試みます。
// DServerV6プログラムは起動時に必要な情報を出力します。
// 接続すると、プログラムはサーバーへデータを送信し、
// 反応を待ち、終了します。
//
// ws2_32.libでコンパイル・リンクします。
//
// IPv6でテストするには、まずWindowsにIPv6スタックを
// インストールする必要があります。
// Windows XPにIPv6をインストールするには、
// コマンドプロンプトを開き、下記のように入力します。
//
// ipv6 install
//
// 使用方法：
//
// DClientV6 ServerName portnumber bIPv6
//
// 1番目のパラメータServerNameは、サーバー名。
// 
// 2番目のパラメータportnumberは、サーバーがbind()したポート番号。 
// 
// 3番目のパラメータbIPv6は、IPv6使用時は1、IPv4使用時は0。
//
// 例： DClientV6 ServerName 2000 1
//
//

#include <stdio.h>
#include <string.h>
#include <winsock2.h> 
#include <Ws2tcpip.h> 

// Function prototype
void DatagramClient(char *szServer, char *pszPort, char *ipvNum);

// Helper macro for displaying errors
#define PRINTERROR(s)	\
		fprintf(stderr,"\n%s: %d\n", s, WSAGetLastError())

////////////////////////////////////////////////////////////

void main(int argc, char **argv)
{
	WORD wVersionRequested = MAKEWORD(1,1);
	WSADATA wsaData;
	int nRet;
	
	//
	// Check for the host and port and IP version arguments
	//
	if (argc != 4)
	{
		fprintf(stderr,"\nSyntax: client ServerName PortNumber bIPv6\n");
		return;
	}

	//
	// Initialize WinSock and check the version
	//
	nRet = WSAStartup(wVersionRequested, &wsaData);
	if (wsaData.wVersion != wVersionRequested)
	{	
		fprintf(stderr,"\n Wrong version\n");
		return;
	}

	//
	// Go do all the stuff a datagram client does
	//
	DatagramClient(argv[1], argv[2], argv[3]);
	
	//
	// Release WinSock
	//
	WSACleanup();
}

////////////////////////////////////////////////////////////

void DatagramClient(char *szServer, char *pszPort, char *pszIPV)
{
	int ipvVer;
	int aiFamily;

	if(strcmp(pszIPV, "0") == 0){
		ipvVer = 4;	
		aiFamily = AF_INET;
	}else if(strcmp(pszIPV, "1") == 0){
		ipvVer = 6;
		aiFamily = AF_INET6;
	}else{
		printf("\nIP version 1 = IPv6 or 0 = IPv4\n");
		return;
	}
	
	printf("\nDatagram Client sending to server: %s on port: %s using ipv: %d",
				szServer, pszPort, ipvVer);

	ADDRINFO Hints, *AI;
	int nRet;

    memset(&Hints, 0, sizeof(Hints));
    Hints.ai_family = aiFamily; 
    Hints.ai_socktype = SOCK_DGRAM;
    Hints.ai_flags = 0;

    nRet = getaddrinfo(szServer, pszPort, &Hints, &AI);
	if( nRet != 0 ){
        fprintf(stderr, "getaddrinfo failed with error %d: %s\n", nRet, gai_strerror(nRet));
        return;
    }

	//
	// Create a UDP/IP datagram socket
	//
	SOCKET	theSocket;

	theSocket = socket(AI->ai_family,		// Address family
					   SOCK_DGRAM,			// Socket type
					   IPPROTO_UDP);		// Protocol  

	if (theSocket == INVALID_SOCKET)
	{
		PRINTERROR("socket()");
		return;
	}
	
	//
	// Send data to the server
	//
	char szBuf[256];
	
	strcpy(szBuf, "From the Client");
	nRet = sendto(theSocket,				// Socket
				  szBuf,					// Data buffer
				  (int)strlen(szBuf),		// Length of data
				  0,						// Flags
				  AI->ai_addr,				// Server address
				  (int)AI->ai_addrlen);		// Length of address
	if (nRet == SOCKET_ERROR)
	{
		PRINTERROR("sendto()");
		closesocket(theSocket);
		return;
	}

	freeaddrinfo(AI);

	//
	// Wait for the reply
	//
	memset(szBuf, 0, sizeof(szBuf));
	int nFromLen;

	nFromLen = sizeof(struct sockaddr);
	recvfrom(theSocket,						// Socket
			 szBuf,							// Receive buffer
			 sizeof(szBuf),					// Length of receive buffer
			 0,								// Flags
			 AI->ai_addr,					// Buffer to receive sender's address
			 &nFromLen);					// Length of address buffer
	if (nRet == SOCKET_ERROR)
	{
		PRINTERROR("recvfrom()");
		closesocket(theSocket);
		return;
	}

	//
	// Display the data that was received
	//
	printf("\nData received: %s", szBuf);

	closesocket(theSocket);
	return;
}
