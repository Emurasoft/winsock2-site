//
// GetHTTP3.cpp --	HTTP�T�[�o�[����t�@�C�������񂹂�
//
//					���̃o�[�W�����ł͏d��I/O��
//					�����֐����g�p����
//
// ws2_32.lib�ŃR���p�C���E�����N����
//

#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <winsock2.h>


void GetHTTP(LPCSTR lpServerName, LPCSTR lpFileName);

//
// �d��I/O�����֐�
//
void CALLBACK RecvComplete(DWORD dwError, 
						   DWORD cbTransferred, 
						   LPWSAOVERLAPPED lpOverlapped, 
						   DWORD dwFlags); 

// �G���[�\���p�̃w���p�[�}�N��
#define PRINTERROR(s)	\
		fprintf(stderr,"\n%s %d\n", s, WSAGetLastError())

#define BUFFER_SIZE 1024

//
// �����֐��ɒǉ�����
// �n�����߂̍\����
//
typedef struct tagIOREQUEST
{
	WSAOVERLAPPED	over; // ����͍ŏ��ɋL�q���Ȃ���΂Ȃ�Ȃ�
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
	// �������`�F�b�N����
	//
	if (argc != 3)
	{
		fprintf(stderr,
			"\nSyntax: GetHTTP ServerName FullPathName\n");
		return;
	}

	//
	// WinSock.dll������������
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
	// WinSock�̃o�[�W�����𒲂ׂ�
	//
	if (wsaData.wVersion != wVersionRequested)
	{
		fprintf(stderr,"\nWinSock version not supported\n");
		WSACleanup();
		return;
	}

	//
	// �o�C�i���t�@�C���i.gif�A.jpg�A.exe�Ȃǁj��
	// ���_�C���N�V�������@�\�����邽�߂ɁA
	// stdout���o�C�i�����[�h�ɐݒ肷��
	//
	_setmode(_fileno(stdout), _O_BINARY);

	//
	// GetHTTP()���Ăяo���Ă��ׂĂ̏��������s����
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
	// �z�X�g�A�h���X�̌���
	//
	lpHostEntry = gethostbyname(lpServerName);
	if (lpHostEntry == NULL)
	{
		PRINTERROR("socket()");
		return;
	}
	
	// TCP/IP�X�g���[���\�P�b�g���쐬����
	Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (Socket == INVALID_SOCKET)
	{
		PRINTERROR("socket()");
		return;
	}

	//
	// �T�[�o�[�A�h���X�\���̂̎c��̕����𖄂߂�
	//
	saServer.sin_family = AF_INET;
	saServer.sin_addr = *((LPIN_ADDR)*lpHostEntry->h_addr_list);
	saServer.sin_port = htons(80);

	//
	// �\�P�b�g��ڑ�����
	//
	nRet = connect(Socket, (LPSOCKADDR)&saServer, sizeof(SOCKADDR_IN));
	if (nRet == SOCKET_ERROR)
	{
		PRINTERROR("connect()");
		closesocket(Socket);	
		return;
	}
	
	//
	// HTTP�v��������������
	// ���M����
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
	// �ڑ����ꂽ�̂ŁA
	// ��M�o�b�t�@���w�肷�邱�Ƃ��ł���
	//

	// 
	// �����ł͂�����Ƃ����H�v�ɂ���āA
	// �����֐��ɒǉ�����n���Ă���B
	// ���ׂĂ̒ǉ�����WSAOVERLAPPED�\���̂�
	// �Ō�ɒǉ����AWSARecv()��
	// lpOverlapped�p�����[�^�Ƃ��ēn���B
	// �����֐����Ăяo�����Ƃ��ɁA
	// ���̒ǉ����𗘗p�ł���B
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

	// �t�@�C���̓��e���󂯎��Astdout�ɏ����o��
	while(1)
	{
		//
		// ���̑��̏����������ōs��
		//

		//
		// SleepEx()���g���āA
		// �x���\�ȑҋ@��Ԃɂ��邱�Ƃ�ʒm����
		//
		SleepEx(0, TRUE);

		//
		// �����֐��ɂ���Ċ������ʒm���ꂽ�ꍇ
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
	// �G���[���`�F�b�N����
	//
	if (dwError)
	{
		fprintf(stderr,"\nRecvComplete() error: %ld", 
						dwError);
		return;
	}

	LPIOREQUEST pReq = (LPIOREQUEST)lpOver;

	//
	// �G���[���f�[�^���Ȃ��ꍇ�́A
	// �ڑ������Ă���
	//	
	if (cbRecv == 0)
	{
		pReq->fFinished = TRUE;
		return;
	}

	fprintf(stderr,"\nRecvComplete(): %ld bytes received", cbRecv);
	//
	// ��M�f�[�^��stdout�ɏ�������
	//
	fwrite(pReq->pBuffer, cbRecv, 1, stdout);

	// 
	// ��M�o�b�t�@��������x�w�肷��
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

