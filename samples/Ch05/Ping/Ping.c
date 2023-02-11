//
// PING.C -- ICMP�Ɛ��\�P�b�g���g�p����Ping�v���O����
//

#include <stdio.h>
#include <stdlib.h>
#include <winsock.h>

#include "ping.h"

// �����֐�
void Ping(LPCSTR pstrHost);
void ReportError(LPCSTR pstrFrom);
int  WaitForEchoReply(SOCKET s);
u_short in_cksum(u_short *addr, int len);

// ICMP�G�R�[�v��/�����֐�
int		SendEchoRequest(SOCKET, LPSOCKADDR_IN);
DWORD	RecvEchoReply(SOCKET, LPSOCKADDR_IN, u_char *);


// main()
void main(int argc, char **argv)
{
    WSADATA wsaData;
    WORD wVersionRequested = MAKEWORD(1,1);
    int nRet;

	// �������`�F�b�N
    if (argc != 2)
    {
		fprintf(stderr,"\nUsage: ping hostname\n");
		return;
    }

	// WinSock�̏�����
    nRet = WSAStartup(wVersionRequested, &wsaData);
    if (nRet)
    {
		fprintf(stderr,"\nError initializing WinSock\n");
		return;
    }

	// �o�[�W�����̃`�F�b�N
	if (wsaData.wVersion != wVersionRequested)
	{
		fprintf(stderr,"\nWinSock version not supported\n");
		return;
	}

	// Ping�����s
	Ping(argv[1]);

	// WinSock�����
    WSACleanup();
}


// Ping()
// SendEchoRequest()��RecvEchoReply()��
// �Ăяo���Č��ʂ��o�͂���
void Ping(LPCSTR pstrHost)
{
	SOCKET	  rawSocket;
	LPHOSTENT lpHost;
	struct    sockaddr_in saDest;
	struct    sockaddr_in saSrc;
	DWORD	  dwTimeSent;
	DWORD	  dwElapsed;
	u_char    cTTL;
	int       nLoop;
	int       nRet;

	// ���\�P�b�g�̍쐬
	rawSocket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (rawSocket == SOCKET_ERROR) 
	{
		ReportError("socket()");
		return;
	}
	
	// �z�X�g�̌���
	lpHost = gethostbyname(pstrHost);
	if (lpHost == NULL)
	{
		fprintf(stderr,"\nHost not found: %s\n", pstrHost);
		return;
	}
	
	// ����\�P�b�g�A�h���X�̐ݒ�
	saDest.sin_addr.s_addr = *((u_long FAR *) (lpHost->h_addr));
	saDest.sin_family = AF_INET;
	saDest.sin_port = 0;

	// ����󋵂����[�U�[�ɕ\��
	printf("\nPinging %s [%s] with %d bytes of data:\n",
				pstrHost,
				inet_ntoa(saDest.sin_addr),
				REQ_DATASIZE);

	// Ping�����x�����s
	for (nLoop = 0; nLoop < 4; nLoop++)
	{
		// ICMP�G�R�[�v���𑗐M
		SendEchoRequest(rawSocket, &saDest);

		// select()���g�p���ăf�[�^�̎�M��ҋ@
		nRet = WaitForEchoReply(rawSocket);
		if (nRet == SOCKET_ERROR)
		{
			ReportError("select()");
			break;
		}
		if (!nRet)
		{
			printf("\nTimeOut");
			break;
		}

		// ��������M
		dwTimeSent = RecvEchoReply(rawSocket, &saSrc, &cTTL);

		// �o�ߎ��Ԃ��v�Z
		dwElapsed = GetTickCount() - dwTimeSent;
		printf("\nReply from: %s: bytes=%d time=%ldms TTL=%d", 
               inet_ntoa(saSrc.sin_addr), 
			   REQ_DATASIZE,
               dwElapsed,
               cTTL);
	}
	printf("\n");
	nRet = closesocket(rawSocket);
	if (nRet == SOCKET_ERROR)
		ReportError("closesocket()");
}


// SendEchoRequest()
// �G�R�[�v���w�b�_�[�ɏ���
// �ݒ肵�A����ɑ��M����
int SendEchoRequest(SOCKET s,LPSOCKADDR_IN lpstToAddr) 
{
	static ECHOREQUEST echoReq;
	static nId = 1;
	static nSeq = 1;
	int nRet;

	// �G�R�[�v���ɏ���ݒ�
	echoReq.icmpHdr.Type		= ICMP_ECHOREQ;
	echoReq.icmpHdr.Code		= 0;
	echoReq.icmpHdr.Checksum	= 0;
	echoReq.icmpHdr.ID			= nId++;
	echoReq.icmpHdr.Seq			= nSeq++;

	// ���M�f�[�^��ݒ�
	for (nRet = 0; nRet < REQ_DATASIZE; nRet++)
		echoReq.cData[nRet] = ' '+nRet;

	// ���M���̃e�B�b�N�J�E���g��ۑ�
	echoReq.dwTime				= GetTickCount();

	// �p�P�b�g���Ƀf�[�^�����A�`�F�b�N�T�����v�Z
	echoReq.icmpHdr.Checksum = in_cksum((u_short *)&echoReq, sizeof(ECHOREQUEST));

	// �G�R�[�v���𑗐M
	nRet = sendto(s,						// �\�P�b�g
				 (LPSTR)&echoReq,			// �o�b�t�@
				 sizeof(ECHOREQUEST),
				 0,							// �t���O
				 (LPSOCKADDR)lpstToAddr, // ����
				 sizeof(SOCKADDR_IN));   // �A�h���X�̒���

	if (nRet == SOCKET_ERROR) 
		ReportError("sendto()");
	return (nRet);
}


// RecvEchoReply()
// ���M�f�[�^����M���āA
// �t�B�[���h�ʂɉ�͂���
DWORD RecvEchoReply(SOCKET s, LPSOCKADDR_IN lpsaFrom, u_char *pTTL) 
{
	ECHOREPLY echoReply;
	int nRet;
	int nAddrLen = sizeof(struct sockaddr_in);

	// �G�R�[��������M
	nRet = recvfrom(s,					// �\�P�b�g
					(LPSTR)&echoReply,	// �o�b�t�@
					sizeof(ECHOREPLY),	// �o�b�t�@�̃T�C�Y
					0,					// �t���O
					(LPSOCKADDR)lpsaFrom,	// ���M���A�h���X
					&nAddrLen);			// �A�h���X���ւ̃|�C���^

	// �߂�l���`�F�b�N
	if (nRet == SOCKET_ERROR) 
		ReportError("recvfrom()");

	// ���M����IP TTL�i�������ԁj��Ԃ�
	*pTTL = echoReply.ipHdr.TTL;
	return(echoReply.echoRequest.dwTime);   		
}

// ������������̕�
void ReportError(LPCSTR pWhere)
{
	fprintf(stderr,"\n%s error: %d\n",
		WSAGetLastError());
}


// WaitForEchoReply()
// select()���g�p���āA�f�[�^��
// �ǂݎ��ҋ@�����ǂ����𔻕ʂ���
int WaitForEchoReply(SOCKET s)
{
	struct timeval Timeout;
	fd_set readfds;

	readfds.fd_count = 1;
	readfds.fd_array[0] = s;
	Timeout.tv_sec = 5;
    Timeout.tv_usec = 0;

	return(select(1, &readfds, NULL, NULL, &Timeout));
}


//
// Mike Muuss' in_cksum() function
// and his comments from the original
// ping program
//
// * Author -
// *	Mike Muuss
// *	U. S. Army Ballistic Research Laboratory
// *	December, 1983

/*
 *			I N _ C K S U M
 *
 * Checksum routine for Internet Protocol family headers (C Version)
 *
 */
u_short in_cksum(u_short *addr, int len)
{
	register int nleft = len;
	register u_short *w = addr;
	register u_short answer;
	register int sum = 0;

	/*
	 *  Our algorithm is simple, using a 32 bit accumulator (sum),
	 *  we add sequential 16 bit words to it, and at the end, fold
	 *  back all the carry bits from the top 16 bits into the lower
	 *  16 bits.
	 */
	while( nleft > 1 )  {
		sum += *w++;
		nleft -= 2;
	}

	/* mop up an odd byte, if necessary */
	if( nleft == 1 ) {
		u_short	u = 0;

		*(u_char *)(&u) = *(u_char *)w ;
		sum += u;
	}

	/*
	 * add back carry outs from top 16 bits to low 16 bits
	 */
	sum = (sum >> 16) + (sum & 0xffff);	/* add hi 16 to low 16 */
	sum += (sum >> 16);			/* add carry */
	answer = ~sum;				/* truncate to 16 bits */
	return (answer);
}
