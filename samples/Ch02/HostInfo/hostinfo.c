//
// HostInfo.c  
//
// WinSock��gethostbyname()/gethostbyaddr()���g���āA�z�X�g���܂���
// IP�A�h���X��T���A�Ԃ��ꂽhostent�\���̂Ɋ܂܂������o�͂���
//

//
// �R�}���h���C���ŃT�[�o�[���܂���IP�A�h���X��n��
//
// ��F
//        HostInfo www.idgbooks.com
//        HostInfo www.sockets.com
//        HostInfo 207.68.156.52
//

#include <stdio.h>
#include <winsock.h>

// hostent�\���̂̓��e���o�͂���֐�
int Printhostent(LPCSTR lpServerNameOrAddress);

void main(int argc, char **argv)
{
    WORD wVersionRequested = MAKEWORD(1,1);
    WSADATA wsaData;
    int nRC;

    // �������`�F�b�N����
    if (argc != 2)
    {
        fprintf(stderr,
            "\nSyntax: HostInfo ServerNameOrAddress\n");
        return;
    }

    // WinSock.dll������������
    nRC = WSAStartup(wVersionRequested, &wsaData);
    if (nRC)
    {
        fprintf(stderr,"\nWSAStartup() error: %d\n", nRC); 
        WSACleanup();
        return;
    }

    // WinSock�̃o�[�W�������`�F�b�N����
    if (wVersionRequested != wsaData.wVersion)
    {
        fprintf(stderr,"\nWinSock version 1.1 not supported\n");
        WSACleanup();
        return;
    }

    // Printhostent()���Ăяo��
    nRC = Printhostent(argv[1]);
    if (nRC)
        fprintf(stderr,"\nPrinthostent return code: %d\n", nRC);
    WSACleanup();
}

int Printhostent(LPCSTR lpServerNameOrAddress)
{
    LPHOSTENT lpHostEntry;     // hostent�\���̂��w���|�C���^
    struct in_addr iaHost;     // �C���^�[�l�b�g�A�h���X�\����
    struct in_addr *pinAddr;   // �C���^�[�l�b�g�A�h���X���w���|�C���^
    LPSTR lpAlias;             // �ʖ��i�G�C���A�X���j���w���L�����N�^�|�C���^
    int iNdx;

    // inet_addr()���g���āA�n���ꂽ�����񂪃z�X�g�����A�h���X����
    // ���ʂ���
    iaHost.s_addr = inet_addr(lpServerNameOrAddress);
    if (iaHost.s_addr == INADDR_NONE)
    {
        // IP�A�h���X�łȂ������ꍇ�́A�z�X�g���Ƃ݂Ȃ�
        lpHostEntry = gethostbyname(lpServerNameOrAddress);
    }
    else
    {
        // �n���ꂽ������͗L����IP�A�h���X
        lpHostEntry = gethostbyaddr((const char *)&iaHost, 
                        sizeof(struct in_addr), AF_INET);
    }

    // �߂�l�𒲂ׂ�
    if (lpHostEntry == NULL)
    {
        fprintf(stderr,"\nError getting host: %d",
                 WSAGetLastError());
        return WSAGetLastError();
    }

    // �\���̂̓��e���o�͂���
    printf("\n\nHOSTENT");
    printf("\n-----------------");

    // �z�X�g��
    printf("\nHost Name........: %s", lpHostEntry->h_name);

    // �z�X�g�̕ʖ��̃��X�g
    printf("\nHost Aliases.....");
    for (iNdx = 0; ; iNdx++)
    {
        lpAlias = lpHostEntry->h_aliases[iNdx];
        if (lpAlias == NULL)
            break;
        printf(": %s", lpAlias);
        printf("\n                 ");
    }

    // �A�h���X�̎��
    printf("\nAddress type.....: %d", lpHostEntry->h_addrtype);
    if (lpHostEntry->h_addrtype == AF_INET)
        printf(" (AF_INET)");
    else
        printf(" (UnknownType)");

    // �A�h���X�̃T�C�Y
    printf("\nAddress length...: %d", lpHostEntry->h_length);

    // IP�A�h���X�̃��X�g
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
