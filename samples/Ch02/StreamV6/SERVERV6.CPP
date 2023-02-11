//
// ServerV6.cpp
//
// きわめて単純なストリームサーバー例。
// ClientV6.cppとの連携で動作します。
//
// プログラムは、TCPプロトコルを使ったサーバーとして自身を設定し、
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
// ServerV6 portnumber bIPv6
//
// 1番目のパラメータportnumberは、サーバーがbind()するポート番号。 
// すでに使用されていないポート番号であれば、いずれも指定可能。
// 
// 2番目のパラメータbIPv6は、IPv6使用時は1、IPv4使用時は0。
//
// 例： ServerV6 2000 1
//

#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>

// Function prototype
void StreamServer(char* pszPort, char *pszIPV);

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
	// Check for port argument
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
	// Do the stuff a stream server does
	//
	StreamServer(argv[1], argv[2]);

	
	//
	// Release WinSock
	//
	WSACleanup();
}

////////////////////////////////////////////////////////////

void StreamServer(char* pszPort, char *pszIPV)
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
	// Create a TCP/IP stream socket to "listen" with
	//
	SOCKET	listenSocket;
	ADDRINFO Hints, *AI;
	int nRet;

    memset(&Hints, 0, sizeof(Hints));
    Hints.ai_family = aiFamily;
    Hints.ai_socktype = SOCK_STREAM;
    Hints.ai_flags = AI_NUMERICHOST | AI_PASSIVE;
    nRet = getaddrinfo(NULL, pszPort, &Hints, &AI);
    if (nRet != 0) {
        fprintf(stderr, "getaddrinfo failed with error %d: %s\n", nRet, gai_strerror(nRet));
        return;
    }

	listenSocket = socket(AI->ai_family,		// Address family
						  AI->ai_socktype,		// Socket type
						  AI->ai_protocol);		// Protocol

	if (listenSocket == INVALID_SOCKET)
	{
		PRINTERROR("socket()");
		return;
	}

	//
	// bind the name to the socket
	//

	nRet = bind(listenSocket,		// Socket 
				AI->ai_addr,		// Our address
				(int)AI->ai_addrlen);	// Size of address structure
	if (nRet == SOCKET_ERROR)
	{
		PRINTERROR("bind()");
		closesocket(listenSocket);
		return;
	}

	freeaddrinfo(AI);

	//
	// This isn't normally done or required, but in this 
	// example we're printing out where the server is waiting
	// so that you can connect the example client.
	//
	int nLen;
	nLen = sizeof(SOCKADDR);
	char szBuf[256];

	nRet = gethostname(szBuf, sizeof(szBuf));
	if (nRet == SOCKET_ERROR)
	{
		PRINTERROR("gethostname()");
		closesocket(listenSocket);
		return;
	}

	//
	// Show the server name and port number
	//
	printf("\nServer named %s waiting on port %s\n",
			szBuf, pszPort);

	//
	// Set the socket to listen
	//

	printf("\nlisten()");
	nRet = listen(listenSocket,					// Bound socket
				  SOMAXCONN);					// Number of connection request queue
	if (nRet == SOCKET_ERROR)
	{
		PRINTERROR("listen()");
		closesocket(listenSocket);
		return;
	}

	//
	// Wait for an incoming request
	//
	SOCKET	remoteSocket;

	printf("\nBlocking at accept()");
	remoteSocket = accept(listenSocket,			// Listening socket
						  NULL,					// Optional client address
						  NULL);
	if (remoteSocket == INVALID_SOCKET)
	{
		PRINTERROR("accept()");
		closesocket(listenSocket);
		return;
	}

	//
	// We're connected to a client
	// New socket descriptor returned already
	// has clients address

	//
	// Receive data from the client
	//
	memset(szBuf, 0, sizeof(szBuf));
	nRet = recv(remoteSocket,					// Connected client
				szBuf,							// Receive buffer
				sizeof(szBuf),					// Lenght of buffer
				0);								// Flags
	if (nRet == INVALID_SOCKET)
	{
		PRINTERROR("recv()");
		closesocket(listenSocket);
		closesocket(remoteSocket);
		return;
	}

	//
	// Display received data
	//
	printf("\nData received: %s", szBuf);

	//
	// Send data back to the client
	//
	strcpy(szBuf, "From the Server");
	nRet = send(remoteSocket,				// Connected socket
				szBuf,						// Data buffer
				(int)strlen(szBuf),			// Lenght of data
				0);							// Flags

	//
	// Close BOTH sockets before exiting
	//
	closesocket(remoteSocket);
	closesocket(listenSocket);
	return;
}
