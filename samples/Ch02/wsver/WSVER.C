//
// WSVer.c
//
// WSAStartup()から返される情報を出力します。
//

//
// オプションとして、要求したいバージョンを
// コマンドラインで渡すことが可能。
// メジャーバージョン番号に続けて
// マイナーバージョン番号を渡すこと。
//
// 例：
// バージョン1.1を要求するには	WSVer 1 1
// バージョン2.0を要求するには	WSVer 2 0
//
// もし試行するバージョンを指定しない場合には、
// プログラムはバージョン1.1を要求します。
//

#include <stdio.h>
#include <winsock.h>

// Utility function in wsedesc.c
LPCSTR WSErrorDescription(int iErrorCode);

void PrintWSAData(LPWSADATA pWSAData);

void main(int argc, char **argv)
{
	WORD wVersionRequested = MAKEWORD(2,2);
	WSADATA wsaData;
	int rc;

	if (argc == 3)
		wVersionRequested = MAKEWORD(atol(argv[1]), 
								atol(argv[2]));

	printf("\nRequesting version %d.%d\n",
						LOBYTE(wVersionRequested),
						HIBYTE(wVersionRequested));

	rc = WSAStartup(wVersionRequested, &wsaData);
	if (!rc)
		PrintWSAData(&wsaData);
	else
		fprintf(stderr,"\nWSAStartup() error (%d) %s\n", 
										rc,
										WSErrorDescription(rc));
	WSACleanup();
}

void PrintWSAData(LPWSADATA pWSAData)
{
	printf("\nWSADATA");
	printf("\n----------------------");
	printf("\nVersion..............: %d.%d", 
			LOBYTE(pWSAData->wVersion),
			HIBYTE(pWSAData->wVersion));
	printf("\nHighVersion..........: %d.%d",
			LOBYTE(pWSAData->wHighVersion),
			HIBYTE(pWSAData->wHighVersion));
	printf("\nDescription..........: %s", 
			pWSAData->szDescription);
	printf("\nSystem status........: %s",
			pWSAData->szSystemStatus);
	printf("\nMax number of sockets: %d",
			pWSAData->iMaxSockets);
	printf("\nMAX UDP datagram size: %d\n",
			pWSAData->iMaxUdpDg);
}

