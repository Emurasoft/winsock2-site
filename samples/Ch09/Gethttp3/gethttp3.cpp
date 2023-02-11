//
// GetHTTP3.cpp --	HTTPサーバーからファイルを取り寄せる
//
//					このバージョンでは重複I/Oと
//					完了関数を使用する
//
// ws2_32.libでコンパイル・リンクする
//

#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <winsock2.h>


void GetHTTP(LPCSTR lpServerName, LPCSTR lpFileName);

//
// 重複I/O完了関数
//
void CALLBACK RecvComplete(DWORD dwError, 
						   DWORD cbTransferred, 
						   LPWSAOVERLAPPED lpOverlapped, 
						   DWORD dwFlags); 

// エラー表示用のヘルパーマクロ
#define PRINTERROR(s)	\
		fprintf(stderr,"\n%s %d\n", s, WSAGetLastError())

#define BUFFER_SIZE 1024

//
// 完了関数に追加情報を
// 渡すための構造体
//
typedef struct tagIOREQUEST
{
	WSAOVERLAPPED	over; // これは最初に記述しなければならない
	SOCKET			Socket;
	BOOL			fFinished;
	LPBYTE			pBuffer;
}IOREQUEST, *LPIOREQUEST;


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
		fprintf(stderr, "\nWSAStartup() error (%d)\n", 
					nRet);
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
	// バイナリファイル（.gif、.jpg、.exeなど）の
	// リダイレクションを機能させるために、
	// stdoutをバイナリモードに設定する
	//
	_setmode(_fileno(stdout), _O_BINARY);

	//
	// GetHTTP()を呼び出してすべての処理を実行する
	//
	GetHTTP(argv[1], argv[2]);

	WSACleanup();
}


void GetHTTP(LPCSTR lpServerName, LPCSTR lpFileName)
{
	LPHOSTENT	lpHostEntry;
	SOCKADDR_IN saServer;
	SOCKET		Socket;
	int			nRet;

	//
	// ホストアドレスの検索
	//
	lpHostEntry = gethostbyname(lpServerName);
	if (lpHostEntry == NULL)
	{
		PRINTERROR("socket()");
		return;
	}
	
	// TCP/IPストリームソケットを作成する
	Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (Socket == INVALID_SOCKET)
	{
		PRINTERROR("socket()");
		return;
	}

	//
	// サーバーアドレス構造体の残りの部分を埋める
	//
	saServer.sin_family = AF_INET;
	saServer.sin_addr = *((LPIN_ADDR)*lpHostEntry->h_addr_list);
	saServer.sin_port = htons(80);

	//
	// ソケットを接続する
	//
	nRet = connect(Socket, (LPSOCKADDR)&saServer, sizeof(SOCKADDR_IN));
	if (nRet == SOCKET_ERROR)
	{
		PRINTERROR("connect()");
		closesocket(Socket);	
		return;
	}
	
	//
	// HTTP要求を書式化して
	// 送信する
	//
	char szBuffer[1024];
	sprintf(szBuffer, "GET %s\n", lpFileName);
	nRet = send(Socket, szBuffer, strlen(szBuffer), 0);
	if (nRet == SOCKET_ERROR)
	{
		PRINTERROR("send()");
		closesocket(Socket);	
		return;
	}

	//
	// 接続されたので、
	// 受信バッファを指定することができる
	//

	// 
	// ここではちょっとした工夫によって、
	// 完了関数に追加情報を渡している。
	// すべての追加情報をWSAOVERLAPPED構造体の
	// 最後に追加し、WSARecv()に
	// lpOverlappedパラメータとして渡す。
	// 完了関数が呼び出されるときに、
	// この追加情報を利用できる。
	//
	BYTE aBuffer[BUFFER_SIZE];
	IOREQUEST ioRequest;
	memset(&ioRequest.over, 0, sizeof(WSAOVERLAPPED));
	ioRequest.Socket = Socket;
	ioRequest.fFinished = FALSE;
	ioRequest.pBuffer = aBuffer;

	WSABUF wsabuf;
	wsabuf.len = BUFFER_SIZE;
	wsabuf.buf = (char *)aBuffer;

	DWORD dwRecv;
	DWORD dwFlags = 0;
	nRet = WSARecv(Socket, 
				   &wsabuf,
				   1,
				   &dwRecv,
				   &dwFlags,
				   (LPWSAOVERLAPPED)&ioRequest,
				   RecvComplete);
	if (nRet == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			PRINTERROR("WSARecv()");
			closesocket(Socket);
			return;
		}
	}

	// ファイルの内容を受け取り、stdoutに書き出す
	while(1)
	{
		//
		// その他の処理をここで行う
		//

		//
		// SleepEx()を使って、
		// 警告可能な待機状態にあることを通知する
		//
		SleepEx(0, TRUE);

		//
		// 完了関数によって完了が通知された場合
		//
		if (ioRequest.fFinished)
			break;
	}
	closesocket(Socket);	
}

void CALLBACK RecvComplete(DWORD dwError, 
						   DWORD cbRecv, 
						   LPWSAOVERLAPPED lpOver, 
						   DWORD dwFlags)
{
	//
	// エラーをチェックする
	//
	if (dwError)
	{
		fprintf(stderr,"\nRecvComplete() error: %ld", 
						dwError);
		return;
	}

	LPIOREQUEST pReq = (LPIOREQUEST)lpOver;

	//
	// エラーもデータもない場合は、
	// 接続が閉じている
	//	
	if (cbRecv == 0)
	{
		pReq->fFinished = TRUE;
		return;
	}

	fprintf(stderr,"\nRecvComplete(): %ld bytes received", cbRecv);
	//
	// 受信データをstdoutに書き込む
	//
	fwrite(pReq->pBuffer, cbRecv, 1, stdout);

	// 
	// 受信バッファをもう一度指定する
	//
	WSABUF wsabuf;
	wsabuf.len = BUFFER_SIZE;
	wsabuf.buf = (char *)pReq->pBuffer;

	DWORD dwRecv;
	dwFlags = 0;
	int nRet;
	nRet = WSARecv(pReq->Socket, 
				   &wsabuf,
				   1,
				   &dwRecv,
				   &dwFlags,
				   lpOver,
				   RecvComplete);
	if (nRet == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			PRINTERROR("RePost with WSARecv()");
			pReq->fFinished = TRUE;
		}
	}
}

