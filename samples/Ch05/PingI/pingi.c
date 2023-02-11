//
// PingI.c -- 固有のMicrosoft ICMP APIを使用した
//			  Pingプログラム
//
 
#include <windows.h>
#include <winsock.h>
#include <stdio.h>
#include <string.h>

// エコー要求オプション
typedef struct tagIPINFO
{
	u_char Ttl;				// 存続時間
	u_char Tos;				// サービスの種類
	u_char IPFlags;			// IPフラグ
	u_char OptSize;			// オプションデータのサイズ
	u_char FAR *Options;	// オプションデータバッファ
}IPINFO, *PIPINFO;

// エコー応答構造体
typedef struct tagICMPECHO
{
	u_long Source;			// ソースアドレス
	u_long Status;			// IPステータス
	u_long RTTime;			// ラウンドトリップ時間（ミリ秒）
	u_short DataSize;		// 応答データサイズ
	u_short Reserved;		// 未定
	void FAR *pData;		// 応答データバッファ
	IPINFO	ipInfo;			// 応答オプション
}ICMPECHO, *PICMPECHO;


// ICMP.DLLエクスポート関数のポインタ
HANDLE (WINAPI *pIcmpCreateFile)(VOID);
BOOL (WINAPI *pIcmpCloseHandle)(HANDLE);
DWORD (WINAPI *pIcmpSendEcho)
	(HANDLE,DWORD,LPVOID,WORD,PIPINFO,LPVOID,DWORD,DWORD);

//
// main()
void main(int argc, char **argv)
{
	WSADATA wsaData;			// WSADATA
	ICMPECHO icmpEcho;			// ICMPエコー応答バッファ
	HANDLE hndlIcmp;			// ICMP.DLLへのLoadLibrary()ハンドル
	HANDLE hndlFile;			// IcmpCreateFile()へのハンドル
    LPHOSTENT pHost;			// ホストエントリ構造体へのポインタ
    struct in_addr iaDest;		// インターネットアドレス構造体
	DWORD *dwAddress;			// IPアドレス
	IPINFO ipInfo;				// IPオプション構造体
	int nRet;					// 汎用の戻りコード
	DWORD dwRet;				// DWORD戻りコード
	int x;

	// 引数をチェック
	if (argc != 2)
	{
		fprintf(stderr,"\nSyntax: pingi HostNameOrIPAddress\n");
		return;
	}

	// ICMP.DLLを動的にロード
	hndlIcmp = LoadLibrary("ICMP.DLL");
	if (hndlIcmp == NULL)
	{
		fprintf(stderr,"\nCould not load ICMP.DLL\n");
		return;
	}
	// ICMP関数ポインタを取得
	pIcmpCreateFile = (HANDLE (WINAPI *)(void))
		GetProcAddress(hndlIcmp,"IcmpCreateFile");
	pIcmpCloseHandle = (BOOL (WINAPI *)(HANDLE))
		GetProcAddress(hndlIcmp,"IcmpCloseHandle");
	pIcmpSendEcho = (DWORD (WINAPI *)
		(HANDLE,DWORD,LPVOID,WORD,PIPINFO,LPVOID,DWORD,DWORD))
		GetProcAddress(hndlIcmp,"IcmpSendEcho");
	// すべての関数ポインタをチェック
	if (pIcmpCreateFile == NULL		|| 
		pIcmpCloseHandle == NULL	||
		pIcmpSendEcho == NULL)
	{
		fprintf(stderr,"\nError getting ICMP proc address\n");
		FreeLibrary(hndlIcmp);
		return;
	}

	// WinSockの初期化
	nRet = WSAStartup(0x0101, &wsaData );
    if (nRet)
    {
        fprintf(stderr,"\nWSAStartup() error: %d\n", nRet); 
        WSACleanup();
		FreeLibrary(hndlIcmp);
        return;
    }
    // WinSockのバージョンをチェック
    if (0x0101 != wsaData.wVersion)
    {
        fprintf(stderr,"\nWinSock version 1.1 not supported\n");
        WSACleanup();
		FreeLibrary(hndlIcmp);
        return;
    }

	// 宛先の検索
	// inet_addr()を使用して、名前とアドレスの
	// どちらを扱っているかを判別する
    iaDest.s_addr = inet_addr(argv[1]);
    if (iaDest.s_addr == INADDR_NONE)
        pHost = gethostbyname(argv[1]);
    else
        pHost = gethostbyaddr((const char *)&iaDest, 
                        sizeof(struct in_addr), AF_INET);
	if (pHost == NULL)
	{
		fprintf(stderr, "\n%s not found\n", argv[1]);
        WSACleanup();
		FreeLibrary(hndlIcmp);
		return;
	}

	// 動作状況をユーザーに表示
	printf("\nPinging %s [%s]", pHost->h_name,
			inet_ntoa((*(LPIN_ADDR)pHost->h_addr_list[0])));

	// IPアドレスをコピー
	dwAddress = (DWORD *)(*pHost->h_addr_list);

	// ICMPエコー要求ハンドルを取得
	hndlFile = pIcmpCreateFile();
	// Pingを4回実行する
	for (x = 0; x < 4; x++)
	{
		// 適切なデフォルト値を設定
		ipInfo.Ttl = 255;
		ipInfo.Tos = 0;
		ipInfo.IPFlags = 0;
		ipInfo.OptSize = 0;
		ipInfo.Options = NULL;
		//icmpEcho.ipInfo.Ttl = 256;
		// ICMPエコーを要求
		dwRet = pIcmpSendEcho(
			hndlFile,		// IcmpCreateFile()からのハンドル
			*dwAddress,		// 宛先IPアドレス
			NULL,			// 送信するバッファへのポインタ
			0,				// バッファのサイズ（バイト数）
			&ipInfo,		// 要求オプション
			&icmpEcho,		// 応答バッファ
			sizeof(struct tagICMPECHO),
			5000);			// 待機時間（ミリ秒）
		// 結果を出力
		iaDest.s_addr = icmpEcho.Source;
		printf("\nReply from %s  Time=%ldms  TTL=%d",
				inet_ntoa(iaDest),
				icmpEcho.RTTime,
				icmpEcho.ipInfo.Ttl);
		if (icmpEcho.Status)
		{
			printf("\nError: icmpEcho.Status=%ld",
				icmpEcho.Status);
			break;
		}
	}
	printf("\n");
	// エコー要求ファイルハンドルを閉じる
	pIcmpCloseHandle(hndlFile);
	FreeLibrary(hndlIcmp);
	WSACleanup();
}

