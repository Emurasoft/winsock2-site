//
// ClientV6.cpp
//
// きわめて単純なストリームクライアント例。
// ServerV6.cppとの連携で動作します。
//
// プログラムは、コマンドラインで指定したサーバーとポートに接続を試みます。
// ServerV6プログラムは起動時に必要な情報を出力します。
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
// ClientV6 ServerName portnumber bIPv6
//
// 1番目のパラメータServerNameは、サーバー名。
// 
// 2番目のパラメータportnumberは、サーバーがbind()したポート番号。 
// 
// 3番目のパラメータbIPv6は、IPv6使用時は1、IPv4使用時は0。
//
// 例： ClientV6 ServerName 2000 1
//

#include <stdio.h>
#include <winsock2.h>
#include <Ws2tcpip.h>

// Function prototype
void StreamClient(char *szServer, char* pszPort, char *ipvNum);

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
	// Go do the stuff a stream client does
	//
	StreamClient(argv[1], argv[2], argv[3]);

	
	//
	// Release WinSock
	//
	WSACleanup();
}

////////////////////////////////////////////////////////////

void StreamClient(char *szServer, char* pszPort, char *pszIPV)
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

	printf("\nStream Client connecting to server: %s on port: %s using ipv: %d",
				szServer, pszPort, ipvVer);
	ADDRINFO Hints, *AI;
	int nRet;

    memset(&Hints, 0, sizeof(Hints));
    Hints.ai_family = aiFamily; 
    Hints.ai_socktype = SOCK_STREAM;
    Hints.ai_flags = 0;

    nRet = getaddrinfo(szServer, pszPort, &Hints, &AI);
	if( nRet != 0 ){
        fprintf(stderr, "getaddrinfo failed with error %d: %s\n", nRet, gai_strerror(nRet));
        return;
    }

	//
	// Create a TCP/IP stream socket
	//
	SOCKET	theSocket;

	theSocket = socket(AI->ai_family,			// Address family
					   SOCK_STREAM,				// Socket type
					   0);						// Protocol
	if (theSocket == INVALID_SOCKET)
	{
		PRINTERROR("socket()");
		return;
	}

	nRet = connect(theSocket,				// Socket
				   AI->ai_addr,				// Server address
				   (int)AI->ai_addrlen);			// Length of server address structure
	if (nRet == SOCKET_ERROR)
	{
		PRINTERROR("socket()");
		closesocket(theSocket);
		return;
	}

	freeaddrinfo( AI );

	//
	// Send data to the server
	//
	char szBuf[256];

	strcpy(szBuf, "From the Client");
	nRet = send(theSocket,				// Connected socket
				szBuf,					// Data buffer
				(int)strlen(szBuf),			// Length of data
				0);						// Flags
	if (nRet == SOCKET_ERROR)
	{
		PRINTERROR("send()");
		closesocket(theSocket);
		return;
	}


	//
	// Wait for a reply
	//
	nRet = recv(theSocket,				// Connected socket
				szBuf,					// Receive buffer
				sizeof(szBuf),			// Size of receive buffer
				0);						// Flags
	if (nRet == SOCKET_ERROR)
	{
		PRINTERROR("recv()");
		closesocket(theSocket);
		return;
	}


	//
	// Display the received data
	//
	printf("\nData received: %s", szBuf);


	closesocket(theSocket);
	return;
}
