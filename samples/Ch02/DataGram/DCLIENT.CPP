//
// DClient.cpp
//
// きわめて単純で、このままでは実用性の薄いデータグラムクライアント例。
// DServer.cppとの連携で動作します。
//
// プログラムは、コマンドラインで指定したサーバーとポートに接続を試みます。
// DServerプログラムは起動時に必要な情報を出力します。
// 接続すると、プログラムはサーバーへデータを送信し、
// 反応を待ち、終了します。
//
// wsock32.libでコンパイル・リンクします。
//
// コマンドラインでサーバー名とポート番号を渡します。
//
// 例： DClient MyMachineName 2000
//

#include <stdio.h>
#include <string.h>
#include <winsock.h>

// Function prototype
void DatagramClient(char *szServer, short nPort);

// Helper macro for displaying errors
#define PRINTERROR(s)	\
		fprintf(stderr,"\n%s: %d\n", s, WSAGetLastError())

////////////////////////////////////////////////////////////

void main(int argc, char **argv)
{
	WORD wVersionRequested = MAKEWORD(1,1);
	WSADATA wsaData;
	int nRet;
	short nPort;

	//
	// Check for the host and port arguments
	//
	if (argc != 3)
	{
		fprintf(stderr,"\nSyntax: dclient ServerName PortNumber\n");
		return;
	}

	nPort = atoi(argv[2]);

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
	DatagramClient(argv[1], nPort);
	
	//
	// Release WinSock
	//
	WSACleanup();
}

////////////////////////////////////////////////////////////

void DatagramClient(char *szServer, short nPort)
{

	printf("\nDatagram Client sending to server: %s on port: %d",
				szServer, nPort);

	//
	// Find the server
	//
    LPHOSTENT lpHostEntry;

	lpHostEntry = gethostbyname(szServer);
    if (lpHostEntry == NULL)
    {
        PRINTERROR("gethostbyname()");
        return;
    }


	//
	// Create a UDP/IP datagram socket
	//
	SOCKET	theSocket;

	theSocket = socket(AF_INET,			// Address family
					   SOCK_DGRAM,		// Socket type
					   IPPROTO_UDP);	// Protocol
	if (theSocket == INVALID_SOCKET)
	{
		PRINTERROR("socket()");
		return;
	}

	//
	// Fill in the address structure for the server
	//
	SOCKADDR_IN saServer;

	saServer.sin_family = AF_INET;
	saServer.sin_addr = *((LPIN_ADDR)*lpHostEntry->h_addr_list);
										// ^ Server's address
	saServer.sin_port = htons(nPort);	// Port number from command line

	//
	// Send data to the server
	//
	char szBuf[256];
	int nRet;

	strcpy(szBuf, "From the Client");
	nRet = sendto(theSocket,				// Socket
				  szBuf,					// Data buffer
				  strlen(szBuf),			// Length of data
				  0,						// Flags
				  (LPSOCKADDR)&saServer,	// Server address
				  sizeof(struct sockaddr)); // Length of address
	if (nRet == SOCKET_ERROR)
	{
		PRINTERROR("sendto()");
		closesocket(theSocket);
		return;
	}

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
			 (LPSOCKADDR)&saServer,			// Buffer to receive sender's address
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
