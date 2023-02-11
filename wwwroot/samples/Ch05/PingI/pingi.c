//
// PingI.c -- �ŗL��Microsoft ICMP API���g�p����
//			  Ping�v���O����
//
 
#include <windows.h>
#include <winsock.h>
#include <stdio.h>
#include <string.h>

// �G�R�[�v���I�v�V����
typedef struct tagIPINFO
{
	u_char Ttl;				// ��������
	u_char Tos;				// �T�[�r�X�̎��
	u_char IPFlags;			// IP�t���O
	u_char OptSize;			// �I�v�V�����f�[�^�̃T�C�Y
	u_char FAR *Options;	// �I�v�V�����f�[�^�o�b�t�@
}IPINFO, *PIPINFO;

// �G�R�[�����\����
typedef struct tagICMPECHO
{
	u_long Source;			// �\�[�X�A�h���X
	u_long Status;			// IP�X�e�[�^�X
	u_long RTTime;			// ���E���h�g���b�v���ԁi�~���b�j
	u_short DataSize;		// �����f�[�^�T�C�Y
	u_short Reserved;		// ����
	void FAR *pData;		// �����f�[�^�o�b�t�@
	IPINFO	ipInfo;			// �����I�v�V����
}ICMPECHO, *PICMPECHO;


// ICMP.DLL�G�N�X�|�[�g�֐��̃|�C���^
HANDLE (WINAPI *pIcmpCreateFile)(VOID);
BOOL (WINAPI *pIcmpCloseHandle)(HANDLE);
DWORD (WINAPI *pIcmpSendEcho)
	(HANDLE,DWORD,LPVOID,WORD,PIPINFO,LPVOID,DWORD,DWORD);

//
// main()
void main(int argc, char **argv)
{
	WSADATA wsaData;			// WSADATA
	ICMPECHO icmpEcho;			// ICMP�G�R�[�����o�b�t�@
	HANDLE hndlIcmp;			// ICMP.DLL�ւ�LoadLibrary()�n���h��
	HANDLE hndlFile;			// IcmpCreateFile()�ւ̃n���h��
    LPHOSTENT pHost;			// �z�X�g�G���g���\���̂ւ̃|�C���^
    struct in_addr iaDest;		// �C���^�[�l�b�g�A�h���X�\����
	DWORD *dwAddress;			// IP�A�h���X
	IPINFO ipInfo;				// IP�I�v�V�����\����
	int nRet;					// �ėp�̖߂�R�[�h
	DWORD dwRet;				// DWORD�߂�R�[�h
	int x;

	// �������`�F�b�N
	if (argc != 2)
	{
		fprintf(stderr,"\nSyntax: pingi HostNameOrIPAddress\n");
		return;
	}

	// ICMP.DLL�𓮓I�Ƀ��[�h
	hndlIcmp = LoadLibrary("ICMP.DLL");
	if (hndlIcmp == NULL)
	{
		fprintf(stderr,"\nCould not load ICMP.DLL\n");
		return;
	}
	// ICMP�֐��|�C���^���擾
	pIcmpCreateFile = (HANDLE (WINAPI *)(void))
		GetProcAddress(hndlIcmp,"IcmpCreateFile");
	pIcmpCloseHandle = (BOOL (WINAPI *)(HANDLE))
		GetProcAddress(hndlIcmp,"IcmpCloseHandle");
	pIcmpSendEcho = (DWORD (WINAPI *)
		(HANDLE,DWORD,LPVOID,WORD,PIPINFO,LPVOID,DWORD,DWORD))
		GetProcAddress(hndlIcmp,"IcmpSendEcho");
	// ���ׂĂ̊֐��|�C���^���`�F�b�N
	if (pIcmpCreateFile == NULL		|| 
		pIcmpCloseHandle == NULL	||
		pIcmpSendEcho == NULL)
	{
		fprintf(stderr,"\nError getting ICMP proc address\n");
		FreeLibrary(hndlIcmp);
		return;
	}

	// WinSock�̏�����
	nRet = WSAStartup(0x0101, &wsaData );
    if (nRet)
    {
        fprintf(stderr,"\nWSAStartup() error: %d\n", nRet); 
        WSACleanup();
		FreeLibrary(hndlIcmp);
        return;
    }
    // WinSock�̃o�[�W�������`�F�b�N
    if (0x0101 != wsaData.wVersion)
    {
        fprintf(stderr,"\nWinSock version 1.1 not supported\n");
        WSACleanup();
		FreeLibrary(hndlIcmp);
        return;
    }

	// ����̌���
	// inet_addr()���g�p���āA���O�ƃA�h���X��
	// �ǂ���������Ă��邩�𔻕ʂ���
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

	// ����󋵂����[�U�[�ɕ\��
	printf("\nPinging %s [%s]", pHost->h_name,
			inet_ntoa((*(LPIN_ADDR)pHost->h_addr_list[0])));

	// IP�A�h���X���R�s�[
	dwAddress = (DWORD *)(*pHost->h_addr_list);

	// ICMP�G�R�[�v���n���h�����擾
	hndlFile = pIcmpCreateFile();
	// Ping��4����s����
	for (x = 0; x < 4; x++)
	{
		// �K�؂ȃf�t�H���g�l��ݒ�
		ipInfo.Ttl = 255;
		ipInfo.Tos = 0;
		ipInfo.IPFlags = 0;
		ipInfo.OptSize = 0;
		ipInfo.Options = NULL;
		//icmpEcho.ipInfo.Ttl = 256;
		// ICMP�G�R�[��v��
		dwRet = pIcmpSendEcho(
			hndlFile,		// IcmpCreateFile()����̃n���h��
			*dwAddress,		// ����IP�A�h���X
			NULL,			// ���M����o�b�t�@�ւ̃|�C���^
			0,				// �o�b�t�@�̃T�C�Y�i�o�C�g���j
			&ipInfo,		// �v���I�v�V����
			&icmpEcho,		// �����o�b�t�@
			sizeof(struct tagICMPECHO),
			5000);			// �ҋ@���ԁi�~���b�j
		// ���ʂ��o��
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
	// �G�R�[�v���t�@�C���n���h�������
	pIcmpCloseHandle(hndlFile);
	FreeLibrary(hndlIcmp);
	WSACleanup();
}

