//
// DServerV6.cpp
//
// きわめて単純で、このままでは実用性の薄いデータグラムサーバー例。
// DClientV6.cppとの連携で動作します。
//
// プログラムは、UDPプロトコルを使ったサーバーとして自身を設定し、
// クライアントからのデータを待ち、届いたデータを表示し、
// クライアントへメッセージを送り返し、終了します。
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
// DServerV6 portnumber bIPv6
//
// 1番目のパラメータportnumberは、サーバーがbind()するポート番号。 
// すでに使用されていないポート番号であれば、いずれも指定可能。
// 
// 2番目のパラメータbIPv6は、IPv6使用時は1、IPv4使用時は0。
//
// 例： DServerV6 2000 1
//
//

#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>

// Function prototype
void DatagramServer(char *pszPort, char *pszIPV);

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
	// Check for port and IPVersion argument
	//
	if (argc != 3)
	{
		fprintf(stderr,"\nSyntax: server PortNumber bIPv6\n");
		return;
	}
	
	//
	// Initialize WinSock and check version
	//
	nRet = WSAStartup(wVersionRequested, &wsaData);
	if (wsaData.wVersion != wVersionRequested)
	{	
		fprintf(stderr,"\n Wrong version\n");
		return;
	}

	//
	// Do all the stuff a datagram server does
	//
	DatagramServer(argv[1], argv[2]);
	
	//
	// Release WinSock
	//
	WSACleanup();
}

////////////////////////////////////////////////////////////

void DatagramServer(char *pszPort, char *pszIPV)
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

	//
	// Create a UDP/IP datagram socket
	//
	SOCKET theSocket;

	ADDRINFO Hints, *AI;
	int nRet;

    memset(&Hints, 0, sizeof(Hints));
    Hints.ai_family = aiFamily; 
    Hints.ai_socktype = SOCK_DGRAM;
    Hints.ai_flags = AI_NUMERICHOST | AI_PASSIVE;
    nRet = getaddrinfo(NULL, pszPort, &Hints, &AI);
    if (nRet != 0) {
        fprintf(stderr, "getaddrinfo failed with error %d: %s\n", nRet, gai_strerror(nRet));
        return;
    }

	theSocket = socket(AI->ai_family,			// Address family
						  AI->ai_socktype,		// Socket type
						  AI->ai_protocol);		// Protocol


	if (theSocket == INVALID_SOCKET)
	{
		PRINTERROR("socket()");
		return;
	}

	nRet = bind(theSocket,				// Socket 
				AI->ai_addr,			// Our address
				(int)AI->ai_addrlen);		// Size of address structure
	
	
	if (nRet == SOCKET_ERROR)
	{
		PRINTERROR("bind()");
		closesocket(theSocket);
		return;
	}

	//
	// This isn't normally done or required, but in this 
	// example we're printing out where the server is waiting
	// so that you can connect the example client.
	//

	char szBuf[256];

	nRet = gethostname(szBuf, sizeof(szBuf));
	if (nRet == SOCKET_ERROR)
	{
		PRINTERROR("gethostname()");
		closesocket(theSocket);
		return;
	}

	//
	// Show the server name and port number
	//
	printf("\nServer named %s waiting on port %s using ipv: %d\n",
			szBuf, pszPort, ipvVer);
			
	//
	// Wait for data from the client
	//

	int nLen = (int)AI->ai_addrlen;

	memset(szBuf, 0, sizeof(szBuf));
	nRet = recvfrom(theSocket,				// Bound socket
					szBuf,					// Receive buffer
					sizeof(szBuf),			// Size of buffer in bytes
					0,						// Flags
					AI->ai_addr,			// Buffer to receive client address 
					&nLen);					// Length of client address buffer

	if (nRet == SOCKET_ERROR)
	{
		PRINTERROR("recvfrom()");
		closesocket(theSocket);
		return;
	}
	//
	// Show that we've received some data
	//
	printf("\nData received: %s", szBuf);


	//
	// Send data back to the client
	//
	strcpy(szBuf, "From the Server");
	sendto(theSocket,						// Bound socket
		   szBuf,							// Send buffer
		   (int)strlen(szBuf),				// Length of data to be sent
		   0,								// Flags
		   AI->ai_addr, 					// Address to send data to
		   (int)AI->ai_addrlen ); 			// Length of address

	//
	// Normally a server continues to run so that
	// it is available to other clients. In this
	// example, we exit as soon as one transaction
	// has taken place.
	//
	freeaddrinfo(AI);

	closesocket(theSocket);
	return;
}
