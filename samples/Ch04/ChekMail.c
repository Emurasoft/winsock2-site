//
// ChekMail.c － POPサーバー上にメールが待機している
// かどうかをチェックする
//
// 16ビットWindowsではLARGEモデルでコンパイルする
// 16ビットWindowsではwinsock.libでリンクする
// 32ビットWindowsではwsock32.libでリンクする
				
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <winsock.h>

#include "chekmail.h"
#include "resource.h"

// --- グローバル変数
// POP3ホスト名
char		gszServerName[255];
// ユーザーID
char		gszUserId[80];  
// パスワード
char		gszPassword[80];
// WSAAsyncGetHostByName()のハンドル
HANDLE		hndlGetHost;           
// WSAAsyncGetHostByName()用のHostEntバッファ
char		bufHostEnt[MAXGETHOSTSTRUCT];
// ソケット
SOCKET 		socketPOP;
// wsprintf()用のスクラッチバッファ
char		gszTemp[255];
// 時間管理に使用する変数
// 例示のみ（実際には必要ない）
DWORD		gdwTicks;
DWORD		gdwElapsed;
// recv()データバッファ
char		gbufRecv[256];

// --- アプリケーションの状態
int			gnAppState;
#define		STATE_CONNECTING 	1
#define		STATE_USER			2
#define		STATE_PASS			3
#define		STATE_STAT			4
#define		STATE_QUIT			5

//
// WinMain() - エントリポイント
//                    
int PASCAL WinMain(
				HINSTANCE 	hinstCurrent,
				HINSTANCE 	hinstPrevious,
   				LPSTR  		lpszCmdLine,
   				int    		nCmdShow)
{
	int nReturnCode;
	WSADATA wsaData;
	#define MAJOR_VERSION_REQUIRED 1
	#define MINOR_VERSION_REQUIRED 1
	
	// WSAStartup()のバージョンの準備
	WORD wVersionRequired = MAKEWORD(MAJOR_VERSION_REQUIRED,
								     MINOR_VERSION_REQUIRED
									 );
    
	// WinSock DLLの初期化
	nReturnCode = WSAStartup(wVersionRequired, &wsaData);
	if (nReturnCode != 0 ) 
	{
		MessageBox(NULL,"Error on WSAStartup()",
					"CheckMail", MB_OK);
    	return 1;
	}
	
	// 要求されたバージョンが利用できることを確認
	if (wsaData.wVersion != wVersionRequired)
	{
    	// 必要なバージョンが利用できない
		MessageBox(NULL,"Wrong WinSock Version",
					"CheckMail", MB_OK);
    	WSACleanup();
    	return 1; 
	}

	// ダイアログボックスをメインウィンドウとして使用
	DialogBox(
			  hinstCurrent, 
			  MAKEINTRESOURCE(IDD_DIALOG_MAIN),
			  NULL, MainDialogProc);
	
	// WinSock DLLを解放
	WSACleanup();
    return 0;
}


//
// MainDialogProc() - メインウィンドウプロシージャ
//
BOOL CALLBACK MainDialogProc(
					HWND hwndDlg,
   					UINT msg,
   					WPARAM wParam,
   					LPARAM lParam)
{
	BOOL fRet = FALSE;  
	
	switch(msg)
	{           
		case WM_INITDIALOG:               
			// WSADATAからの情報を表示
			Display("----- STARTUP -----\r\n");
			Display("WSAStartup() succeeded\r\n\r\n");
			break;
				
		case WM_COMMAND:
			switch(wParam)
			{ 
				// ユーザーが［Check Mail］ボタンをクリックした
				case ID_CHECKMAIL:  
					// POP情報を取得
              		// サーバー名、ユーザーID、パスワード
					if (!GetDlgItemText(hwndDlg, 
										IDC_SERVERNAME,
										gszServerName, 
										sizeof(gszServerName)
										))
					{         
						MessageBox(hwndDlg,
							"Please enter a server name",
							"POP Info", MB_OK);
						break;
					}
					if (!GetDlgItemText(hwndDlg, IDC_USERID,
						gszUserId, sizeof(gszUserId)))
					{         
						MessageBox(hwndDlg,
							"Please enter a user ID",
							"POP Info", MB_OK);
						break;
					}
					if (!GetDlgItemText(hwndDlg, 
										IDC_PASSWORD,
										gszPassword, 
										sizeof(gszPassword)
										))
					{         
						MessageBox(hwndDlg,
							"Please enter a password",
							"POP Info", MB_OK);
						break;
					}

					// gethostbyname()の非同期バージョンを
					// 使用しているホスト名の参照を要求。
					// 完了したらこのウィンドウに
					// SM_GETHOSTメッセージを送信する。
					Display("----- FIND HOST -----\r\n");
					Display("Calling WSAAsyncGetHostByName()"
							" to find server\r\n");
					// 参考のためにWSAAsyncGetHostByName()の
					// 時間を計る
					gdwTicks = GetTickCount();
					hndlGetHost = WSAAsyncGetHostByName(
										hwndDlg, SM_GETHOST,
										gszServerName,
										bufHostEnt,
										MAXGETHOSTSTRUCT);
					if (hndlGetHost == 0)
					{
						MessageBox(hwndDlg,
							"Error initiating "
							"WSAAsyncGetHostByName()",
							"CheckMail", MB_OK);
					}
					else
					{
						EnableButtons(FALSE);
						gnAppState = 0;
					}
					fRet = TRUE;
					break;
					
				// ユーザーがメインウィンドウの
				// キャンセルボタンを押した
				case IDCANCEL:
					if (gnAppState)
						CloseSocket(socketPOP);
					PostQuitMessage(0);
					fRet = TRUE;
					break;
			}
			break;

			// 非同期gethostbyname()の戻り値を処理
			case SM_GETHOST:
				HandleGetHostMsg(hwndDlg, wParam, lParam);
				fRet = TRUE;
				break;

			// 非同期メッセージを処理
			case SM_ASYNC:  
				HandleAsyncMsg(hwndDlg, wParam, lParam);
				fRet = TRUE;
				break;
	}
	return fRet;
}

//
// HandleGetHostMsg()
// WSAAsyncGetHostByName()が完了したときに呼び出される
void HandleGetHostMsg(
			HWND hwndDlg, 
			WPARAM wParam, 
			LPARAM lParam)
{
	SOCKADDR_IN saServ;		// インターネット用ソケットアドレス
	LPHOSTENT	lpHostEnt;	// ホストエントリへのポインタ
	LPSERVENT	lpServEnt;	// サーバーエントリへのポインタ
	int nRet;				// 戻りコード
	
	Display("SM_GETHOST message received\r\n");
	if ((HANDLE)wParam != hndlGetHost)
		return;        
		
	// 参考のために、WSAGetHostByName()の
	// 経過時間を表示
	gdwElapsed = (GetTickCount() - gdwTicks);
	wsprintf((LPSTR)gszTemp,
			(LPSTR)"WSAAsyncGetHostByName() took %ld "
			" milliseconds to complete\r\n", 
			gdwElapsed);
	Display(gszTemp);

	// エラーコードをチェック
	nRet = WSAGETASYNCERROR(lParam);
	if (nRet)
	{       
		wsprintf((LPSTR)gszTemp,
			(LPSTR)"WSAAsyncGetHostByName() error: %d\r\n", 
			nRet);
		Display(gszTemp);
		EnableButtons(TRUE);
		return;
	}     
 
	// サーバーが見つかったので、
	// bufHostEntにサーバー情報が格納されている
	Display("Server found OK\r\n\r\n");
	
	Display("----- CONNECT TO HOST -----\r\n");	
	// ソケットを作成
	Display("Calling socket(AF_INET, SOCK_STREAM, 0);\r\n");
	socketPOP = socket(AF_INET, SOCK_STREAM, 0);
	if (socketPOP == INVALID_SOCKET)
	{
		Display("Could not create a socket\r\n");
		EnableButtons(TRUE);
		return;
	}
	
	// ソケットを非ブロッキングにし、
	// 非同期通知を登録する
	Display("Calling WSAAsyncSelect()\r\n");
	if (WSAAsyncSelect(socketPOP, hwndDlg, SM_ASYNC,
				FD_CONNECT|FD_READ|FD_WRITE|FD_CLOSE))
	{         
		Display("WSAAsyncSelect() failed\r\n");
		EnableButtons(TRUE);
		return;
	}

	// getservbyname()を使用して
	// ポート番号を解決する。
	// 通常はすぐに完了するので、単純な
	// 同期バージョンを使用する。
	// 参考のためにgetservbyname()の
	// 時間を計る。
	gdwTicks = GetTickCount();
	lpServEnt = getservbyname("pop3", "tcp");
	gdwElapsed = (GetTickCount() - gdwTicks);
	wsprintf((LPSTR)gszTemp,
			(LPSTR)"getservbyname() took %ld milliseconds"
			" to complete\r\n", 
			gdwElapsed);
	Display(gszTemp);
	
	// serventがない場合
	if (lpServEnt == NULL) 
	{
	   // よく知られたポートを使用
		saServ.sin_port = htons(110);
		Display("getservbyent() failed. Using port 110\r\n");
	}
	else
	{
		// serventに返されたポートを使用
		saServ.sin_port = lpServEnt->s_port;
	}
	// サーバーアドレス構造体を使用
	saServ.sin_family = AF_INET;
	lpHostEnt = (LPHOSTENT)bufHostEnt;
	saServ.sin_addr = *((LPIN_ADDR)*lpHostEnt->h_addr_list);
	// ソケットを接続
	Display("Calling connect()\r\n");
	nRet = connect(socketPOP, 
				(LPSOCKADDR)&saServ, 
				sizeof(SOCKADDR_IN));
	if (nRet == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSAEWOULDBLOCK)
		{
			Display("Error connecting\r\n");
			EnableButtons(TRUE);
			return;
		}         
	}
	gnAppState = STATE_CONNECTING;
	Display("\r\n----- PROCESS MESSAGES -----\r\n");
}

//
// HandleAsyncMsg()
// WSA WSAAsyncSelct()イベントがトリガーされたときに呼び出される
void HandleAsyncMsg(
				    HWND hwndDlg, 
					WPARAM wParam, 
					LPARAM lParam)
{           
	int nBytesRead;
	
	switch(WSAGETSELECTEVENT(lParam))
	{
		case FD_CONNECT:
			Display("FD_CONNECT notification received\r\n");
			break;   
			
		case FD_WRITE:
			Display("FD_WRITE notification received\r\n");
			break;
			
		case FD_READ:
			Display("FD_READ notification received\r\n");
			Display("Calling recv()\r\n");       
			// 待機中のデータを取得
			nBytesRead = recv(
				socketPOP,  		// ソケット
				gbufRecv, 			// バッファ
				sizeof(gbufRecv), 	// 読み取る最大長
				0);					// 受信フラグ
			// recv()の戻りコードをチェック
			if (nBytesRead == 0)
			{
				// 接続が閉じられた
				MessageBox(hwndDlg, 
					"Connection closed unexpectedly",
					"recv() error", MB_OK);
				break;
			}
			if (nBytesRead == SOCKET_ERROR)
			{
				wsprintf((LPSTR)gszTemp,
					 (LPSTR)"recv() error: %d", nBytesRead);
				MessageBox(
					hwndDlg,
					gszTemp,
					"recv() error",
					MB_OK);
				break;
			}
			// バッファの末尾がNULL
			gbufRecv[nBytesRead] = '\0';
			// 引き渡して解釈させる
			ProcessData(hwndDlg, gbufRecv, nBytesRead);
			break;
		case FD_CLOSE:
			Display("FD_CLOSE notification received\r\n");
			EnableButtons(TRUE);
			break;
	}
}


//
// ProcessData()
// 着信データを読み取り、改行する
//
void ProcessData(HWND hwndDlg, LPSTR lpBuf, int nBytesRead)
{
	static char szResponse[512];
	static int nLen = 0;
	char *cp;
                         
	// 新しいデータをバッファに入れる
	// 余地があるかどうかをチェック
	if ((nLen + nBytesRead) > sizeof(szResponse))
	{
		Display("!!!!! Buffer overrun, data truncated\r\n");
		nLen = 0;
		szResponse[0] = '\0';
		return;
	}                           
	// バッファに新しいデータを追加
	strcat(szResponse, lpBuf);
	nLen = strlen(szResponse);

	// すべての完全な行を処理
	while(1)
	{
		// バッファに完全な行が含まれているか
		cp = strchr(szResponse, '\n');
		if (cp == NULL)
			break;
		// CR/LFの組み合わせがある
		// LFをNULLで置き換える
		*cp = '\0';
		// ProcesLine()に引き渡す
		ProcessLine(hwndDlg, szResponse);
		// 残りのデータをバッファの先頭に移動
		cp++;
		if (strlen(cp))
			memmove(szResponse, cp, strlen(cp)+1);
		else
			szResponse[0] = '\0';
	}
}          

//
// ProcessLine()
// サーバーからの応答行を処理し、
// 次の動作を決定する
//
void ProcessLine(HWND hwndDlg, LPSTR lpStr)
{                      
	int nRet;
	long lCount;
	long lSize;

	Display("Response from server:\r\n");
	Display(lpStr);
	Display("\r\n");
		
	// 応答のエラーをチェック
	if (lpStr[0] == '-')
	{
		Display("Negative response: ");
		switch(gnAppState)
		{
			case STATE_CONNECTING:
				Display("Connection denied\r\n");
				break;
			case STATE_USER:
				Display("Unknown UserID\r\n");
				break;
			case STATE_PASS:
				Display("Wrong Password\r\n");
				break;
			case STATE_STAT:
				Display("STAT command not supported\r\n");
				break;                                    
			case STATE_QUIT:
				Display("QUIT command not supported\r\n");
				break;                                    
		}
		Display("Sending QUIT\r\n");
		wsprintf(gszTemp, "QUIT\r\n");
		Display(gszTemp);
		nRet = send(
				socketPOP,  		// ソケット
				gszTemp, 			// データバッファ
				strlen(gszTemp),	// データの長さ
				0);					// 送信フラグ
		gnAppState = STATE_QUIT;
		return;
	}	                          

	// 肯定の応答があった
	switch(gnAppState)
	{
		case STATE_CONNECTING:
			// ログイン要求のUSER部分を送信し、
			// アプリケーションの状態を設定
			Display("AppState = CONNECTING, "
			        "sending USER\r\n");
			wsprintf(gszTemp, "USER %s\r\n", gszUserId);
			nRet = send(
					socketPOP,		// ソケット
					gszTemp,		// データバッファ
					strlen(gszTemp),// データの長さ
					0);				// 送信フラグ
			gnAppState = STATE_USER;
			break;
			
		case STATE_USER:
			// ログイン要求のPASSword部分を送信
			// アプリケーションの状態を設定
			Display("AppState = USER, sending PASS\r\n");
			wsprintf(gszTemp, "PASS %s\r\n", gszPassword);
			nRet = send(
					socketPOP,		// ソケット
					gszTemp,		// データバッファ
					strlen(gszTemp),// データの長さ
					0);				// 送信フラグ
			gnAppState = STATE_PASS;
			break;
			
		case STATE_PASS:
			// STATコマンド要求を送信
			// アプリケーションの状態を設定
			Display("AppState = PASS, sending STAT\r\n");
			wsprintf(gszTemp, "STAT\r\n");
			nRet = send(
					socketPOP,		// ソケット
					gszTemp,		// データバッファ
					strlen(gszTemp),// データの長さ
					0);				// 送信フラグ
			gnAppState = STATE_STAT;
			break;
			
		case STATE_STAT:
			// STAT応答を読み取る
			// 結果を出力
			Display("AppState = STAT, reading response\r\n");
			sscanf(lpStr, "%s %ld %ld", 
				   gszTemp, &lCount, &lSize);
			Display("----- RESULT -----\r\n");
			wsprintf(gszTemp, "%ld messages %ld bytes\r\n", 
					lCount, lSize);
			Display(gszTemp);
			// QUITコマンドを送信
			// アプリケーションの状態を設定
			Display("Sending QUIT\r\n");
			wsprintf(gszTemp, "QUIT\r\n");
			nRet = send(
					socketPOP,  		// ソケット
					gszTemp, 			// データバッファ
					strlen(gszTemp),	// データの長さ
					0);					// 送信フラグ
			gnAppState = STATE_QUIT;
			break;                 
			
		case STATE_QUIT:
			Display("Host QUIT OK\r\n");
			Display("Closing socket\r\n");
			CloseSocket(socketPOP);
	}	
}


//
// CloseSocket()
// プロトコルスタックバッファをクリアした後で
// ソケットを閉じる
//
void CloseSocket(SOCKET sock)
{   
	int nRet;
	char szBuf[255];
	
	// これ以上データを送信しない
	// ことをリモートに知らせる
	shutdown(sock, 1);
	while(1)
	{
		// データを受信しようとする。
		// これにより、プロトコルスタック内に
		// バッファされていたデータが
		// 確実にクリアされる。
		nRet = recv(
				sock,			// ソケット
				szBuf,			// データバッファ
				sizeof(szBuf),	// バッファの長さ
				0);				// 受信フラグ
		// 接続を閉じるかほかのエラーが起きた
		// 場合はデータの受信を停止
		if (nRet == 0 || nRet == SOCKET_ERROR)
			break;
	}
	// これ以上データを受信しないことを
	// 相手に知らせる
	shutdown(sock, 2);
	// ソケットを閉じる
	closesocket(sock);
}