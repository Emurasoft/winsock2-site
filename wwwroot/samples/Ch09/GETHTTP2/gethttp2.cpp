//
// GetHTTP2.cpp --	HTTPサーバーからファイルを取り寄せる
//
// 					このバージョンでは、イベントオブジェクトと
// 					ネットワークイベントの非同期通知用の
// 					WSASelectEvent()を使用している
//
// 					ws2_32.libでコンパイル・リンクする
//

#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <winsock2.h>

void GetHTTP(LPCSTR lpServerName, LPCSTR lpFileName);

// エラー表示用のヘルパーマクロ
#define PRINTERROR(s)	\
		fprintf(stderr,"\n%s %d\n", s, WSAGetLastError())

void main(int argc, char **argv)
{
	WORD wVersionRequested = WINSOCK_VERSION;
	WSADATA wsaData;
	int nRet;

	//
	// 引数をチェックする
	//
	if (argc != 3)
	{
		fprintf(stderr,
			"\nSyntax: GetHTTP ServerName FullPathName\n");
		return;
	}

	//
	// WinSock.dllを初期化する
	//
	nRet = WSAStartup(wVersionRequested, &wsaData);
	if (nRet)
	{
		fprintf(stderr,"\nWSAStartup(): %d\n", nRet);
		WSACleanup();
		return;
	}

	//
	// WinSockのバージョンを調べる
	//
	if (wsaData.wVersion != wVersionRequested)
	{
		fprintf(stderr,"\nWinSock version not supported\n");
		WSACleanup();
		return;
	}

	//
	// .gifファイルおよび.jpgファイルの
	// リダイレクションを機能させるために、
	// stdoutをバイナリモードに設定する
	//
	_setmode(_fileno(stdout), _O_BINARY);

	//
	// GetHTTP()を呼び出してすべての処理を実行する
	//
	GetHTTP(argv[1], argv[2]);

	//
	// WinSockを解放する
	//
	WSACleanup();
}

////////////////////////////////////////////////////////////

void GetHTTP(LPCSTR lpServerName, LPCSTR lpFileName)
{
	// 
	// ホストの検索
	//
	LPHOSTENT lpHostEntry;
	lpHostEntry = gethostbyname(lpServerName);
	if (lpHostEntry == NULL)
	{
		PRINTERROR("gethostbyname()");
		return;
	}

	//
	// サーバーアドレス構造体を埋める
	//
	SOCKADDR_IN sa;
	sa.sin_family = AF_INET;
	sa.sin_addr = *((LPIN_ADDR)*lpHostEntry->h_addr_list);
	sa.sin_port = htons(80);	// よく知られたHTTPポート

	//	
	// TCP/IPストリームソケットを作成する
	//
	SOCKET	Socket;
	Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (Socket == INVALID_SOCKET)
	{
		PRINTERROR("socket()"); 
		return;
	}

	//
	// このソケットで使用するイベントオブジェクトを作成する
	//
	WSAEVENT hEvent;
	hEvent = WSACreateEvent();
	if (hEvent == WSA_INVALID_EVENT)
	{
		PRINTERROR("WSACreateEvent()");
		closesocket(Socket);
		return;
	}

	//
	// ソケットを非ブロッキングにし、
	// ネットワークイベントを関連付ける
	//
	int nRet;
	nRet = WSAEventSelect(Socket,
						  hEvent,
						  FD_READ|FD_CONNECT|FD_CLOSE);
	if (nRet == SOCKET_ERROR)
	{
		PRINTERROR("EventSelect()");
		closesocket(Socket);
		WSACloseEvent(hEvent);
		return;
	}

	//
	// 接続を要求する
	//
	nRet = connect(Socket, 
	               (LPSOCKADDR)&sa, 
				   sizeof(SOCKADDR_IN));
	if (nRet == SOCKET_ERROR)
	{
		nRet = WSAGetLastError();
		if (nRet == WSAEWOULDBLOCK)
		{
			fprintf(stderr,"\nConnect would block");
		}
		else
		{
			PRINTERROR("connect()");
			closesocket(Socket);
			WSACloseEvent(hEvent);
			return;
		}
	}
	
	//
	// 非同期ネットワークイベントを処理する
	//
	char szBuffer[4096];
	WSANETWORKEVENTS events;
	while(1)
	{
		//
		// 何か発生するのを待機する
		//
		fprintf(stderr,"\nWaitForMultipleEvents()");
		DWORD dwRet;
		dwRet = WSAWaitForMultipleEvents(1,
									 &hEvent,
									 FALSE,
									 10000,
									 FALSE);
		if (dwRet == WSA_WAIT_TIMEOUT)
		{
			fprintf(stderr,"\nWait timed out");
			break;
		}

		//
		// どのイベントが発生したかを判別する
		//
		fprintf(stderr,"\nWSAEnumNetworkEvents()");
		nRet = WSAEnumNetworkEvents(Socket,
								 hEvent,
								 &events);
		if (nRet == SOCKET_ERROR)
		{
			PRINTERROR("WSAEnumNetworkEvents()");
			break;
		}

		//				 //
		// イベントを処理する //
		//				 //

		// 接続イベントかどうか？
		if (events.lNetworkEvents & FD_CONNECT)
		{
			fprintf(stderr,"\nFD_CONNECT: %d",
					events.iErrorCode[FD_CONNECT_BIT]);
			// http要求を送信する
			sprintf(szBuffer, "GET %s\n", lpFileName);
			nRet = send(Socket, szBuffer, strlen(szBuffer), 0);
			if (nRet == SOCKET_ERROR)
			{
				PRINTERROR("send()");
				break;
			}
		}

		// 読み取りイベントかどうか？
		if (events.lNetworkEvents & FD_READ)
		{
			fprintf(stderr,"\nFD_READ: %d",
					events.iErrorCode[FD_READ_BIT]);
			// データを読み取り、stdoutに書き込む
			nRet = recv(Socket, szBuffer, sizeof(szBuffer), 0);
			if (nRet == SOCKET_ERROR)
			{
				PRINTERROR("recv()");
				break;
			}
			fprintf(stderr,"\nRead %d bytes", nRet);
			// stdoutに書き込む
		    fwrite(szBuffer, nRet, 1, stdout);
		}

		// 終了イベントかどうか？
		if (events.lNetworkEvents & FD_CLOSE)
		{
			fprintf(stderr,"\nFD_CLOSE: %d",
					events.iErrorCode[FD_CLOSE_BIT]);
			break;
		}

		// 書き込みイベントかどうか？
		if (events.lNetworkEvents & FD_WRITE)
		{
			fprintf(stderr,"\nFD_WRITE: %d",
					events.iErrorCode[FD_WRITE_BIT]);
		}

	}
	closesocket(Socket);	
	WSACloseEvent(hEvent);
	return;
}

