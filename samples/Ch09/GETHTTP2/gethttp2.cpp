//
// GetHTTP2.cpp --	HTTP�T�[�o�[����t�@�C�������񂹂�
//
// 					���̃o�[�W�����ł́A�C�x���g�I�u�W�F�N�g��
// 					�l�b�g���[�N�C�x���g�̔񓯊��ʒm�p��
// 					WSASelectEvent()���g�p���Ă���
//
// 					ws2_32.lib�ŃR���p�C���E�����N����
//

#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <winsock2.h>

void GetHTTP(LPCSTR lpServerName, LPCSTR lpFileName);

// �G���[�\���p�̃w���p�[�}�N��
#define PRINTERROR(s)	\
		fprintf(stderr,"\n%s %d\n", s, WSAGetLastError())

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
		fprintf(stderr,"\nWSAStartup(): %d\n", nRet);
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
	// .gif�t�@�C�������.jpg�t�@�C����
	// ���_�C���N�V�������@�\�����邽�߂ɁA
	// stdout���o�C�i�����[�h�ɐݒ肷��
	//
	_setmode(_fileno(stdout), _O_BINARY);

	//
	// GetHTTP()���Ăяo���Ă��ׂĂ̏��������s����
	//
	GetHTTP(argv[1], argv[2]);

	//
	// WinSock���������
	//
	WSACleanup();
}

////////////////////////////////////////////////////////////

void GetHTTP(LPCSTR lpServerName, LPCSTR lpFileName)
{
	// 
	// �z�X�g�̌���
	//
	LPHOSTENT lpHostEntry;
	lpHostEntry = gethostbyname(lpServerName);
	if (lpHostEntry == NULL)
	{
		PRINTERROR("gethostbyname()");
		return;
	}

	//
	// �T�[�o�[�A�h���X�\���̂𖄂߂�
	//
	SOCKADDR_IN sa;
	sa.sin_family = AF_INET;
	sa.sin_addr = *((LPIN_ADDR)*lpHostEntry->h_addr_list);
	sa.sin_port = htons(80);	// �悭�m��ꂽHTTP�|�[�g

	//	
	// TCP/IP�X�g���[���\�P�b�g���쐬����
	//
	SOCKET	Socket;
	Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (Socket == INVALID_SOCKET)
	{
		PRINTERROR("socket()"); 
		return;
	}

	//
	// ���̃\�P�b�g�Ŏg�p����C�x���g�I�u�W�F�N�g���쐬����
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
	// �\�P�b�g���u���b�L���O�ɂ��A
	// �l�b�g���[�N�C�x���g���֘A�t����
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
	// �ڑ���v������
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
	// �񓯊��l�b�g���[�N�C�x���g����������
	//
	char szBuffer[4096];
	WSANETWORKEVENTS events;
	while(1)
	{
		//
		// ������������̂�ҋ@����
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
		// �ǂ̃C�x���g�������������𔻕ʂ���
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
		// �C�x���g���������� //
		//				 //

		// �ڑ��C�x���g���ǂ����H
		if (events.lNetworkEvents & FD_CONNECT)
		{
			fprintf(stderr,"\nFD_CONNECT: %d",
					events.iErrorCode[FD_CONNECT_BIT]);
			// http�v���𑗐M����
			sprintf(szBuffer, "GET %s\n", lpFileName);
			nRet = send(Socket, szBuffer, strlen(szBuffer), 0);
			if (nRet == SOCKET_ERROR)
			{
				PRINTERROR("send()");
				break;
			}
		}

		// �ǂݎ��C�x���g���ǂ����H
		if (events.lNetworkEvents & FD_READ)
		{
			fprintf(stderr,"\nFD_READ: %d",
					events.iErrorCode[FD_READ_BIT]);
			// �f�[�^��ǂݎ��Astdout�ɏ�������
			nRet = recv(Socket, szBuffer, sizeof(szBuffer), 0);
			if (nRet == SOCKET_ERROR)
			{
				PRINTERROR("recv()");
				break;
			}
			fprintf(stderr,"\nRead %d bytes", nRet);
			// stdout�ɏ�������
		    fwrite(szBuffer, nRet, 1, stdout);
		}

		// �I���C�x���g���ǂ����H
		if (events.lNetworkEvents & FD_CLOSE)
		{
			fprintf(stderr,"\nFD_CLOSE: %d",
					events.iErrorCode[FD_CLOSE_BIT]);
			break;
		}

		// �������݃C�x���g���ǂ����H
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

