//
// HostInfo.c  
//
// WinSockのgethostbyname()/gethostbyaddr()を使って、ホスト名または
// IPアドレスを探し、返されたhostent構造体に含まれる情報を出力する
//

//
// コマンドラインでサーバー名またはIPアドレスを渡す
//
// 例：
//        HostInfo www.idgbooks.com
//        HostInfo www.sockets.com
//        HostInfo 207.68.156.52
//

#include <stdio.h>
#include <winsock.h>

// hostent構造体の内容を出力する関数
int Printhostent(LPCSTR lpServerNameOrAddress);

void main(int argc, char **argv)
{
    WORD wVersionRequested = MAKEWORD(1,1);
    WSADATA wsaData;
    int nRC;

    // 引数をチェックする
    if (argc != 2)
    {
        fprintf(stderr,
            "\nSyntax: HostInfo ServerNameOrAddress\n");
        return;
    }

    // WinSock.dllを初期化する
    nRC = WSAStartup(wVersionRequested, &wsaData);
    if (nRC)
    {
        fprintf(stderr,"\nWSAStartup() error: %d\n", nRC); 
        WSACleanup();
        return;
    }

    // WinSockのバージョンをチェックする
    if (wVersionRequested != wsaData.wVersion)
    {
        fprintf(stderr,"\nWinSock version 1.1 not supported\n");
        WSACleanup();
        return;
    }

    // Printhostent()を呼び出す
    nRC = Printhostent(argv[1]);
    if (nRC)
        fprintf(stderr,"\nPrinthostent return code: %d\n", nRC);
    WSACleanup();
}

int Printhostent(LPCSTR lpServerNameOrAddress)
{
    LPHOSTENT lpHostEntry;     // hostent構造体を指すポインタ
    struct in_addr iaHost;     // インターネットアドレス構造体
    struct in_addr *pinAddr;   // インターネットアドレスを指すポインタ
    LPSTR lpAlias;             // 別名（エイリアス名）を指すキャラクタポインタ
    int iNdx;

    // inet_addr()を使って、渡された文字列がホスト名かアドレスかを
    // 判別する
    iaHost.s_addr = inet_addr(lpServerNameOrAddress);
    if (iaHost.s_addr == INADDR_NONE)
    {
        // IPアドレスでなかった場合は、ホスト名とみなす
        lpHostEntry = gethostbyname(lpServerNameOrAddress);
    }
    else
    {
        // 渡された文字列は有効なIPアドレス
        lpHostEntry = gethostbyaddr((const char *)&iaHost, 
                        sizeof(struct in_addr), AF_INET);
    }

    // 戻り値を調べる
    if (lpHostEntry == NULL)
    {
        fprintf(stderr,"\nError getting host: %d",
                 WSAGetLastError());
        return WSAGetLastError();
    }

    // 構造体の内容を出力する
    printf("\n\nHOSTENT");
    printf("\n-----------------");

    // ホスト名
    printf("\nHost Name........: %s", lpHostEntry->h_name);

    // ホストの別名のリスト
    printf("\nHost Aliases.....");
    for (iNdx = 0; ; iNdx++)
    {
        lpAlias = lpHostEntry->h_aliases[iNdx];
        if (lpAlias == NULL)
            break;
        printf(": %s", lpAlias);
        printf("\n                 ");
    }

    // アドレスの種類
    printf("\nAddress type.....: %d", lpHostEntry->h_addrtype);
    if (lpHostEntry->h_addrtype == AF_INET)
        printf(" (AF_INET)");
    else
        printf(" (UnknownType)");

    // アドレスのサイズ
    printf("\nAddress length...: %d", lpHostEntry->h_length);

    // IPアドレスのリスト
    printf("\nIP Addresses.....");
    for (iNdx = 0; ; iNdx++)
    {
        pinAddr = ((LPIN_ADDR)lpHostEntry->h_addr_list[iNdx]);
        if (pinAddr == NULL)
            break;
        printf(": %s", inet_ntoa(*pinAddr));
        printf("\n                 ");
    }
    printf("\n");
    return 0;
}
