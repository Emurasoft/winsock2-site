//
// ChekMail.c �| POP�T�[�o�[��Ƀ��[�����ҋ@���Ă���
// ���ǂ������`�F�b�N����
//
// 16�r�b�gWindows�ł�LARGE���f���ŃR���p�C������
// 16�r�b�gWindows�ł�winsock.lib�Ń����N����
// 32�r�b�gWindows�ł�wsock32.lib�Ń����N����
				
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <winsock.h>

#include "chekmail.h"
#include "resource.h"

// --- �O���[�o���ϐ�
// POP3�z�X�g��
char		gszServerName[255];
// ���[�U�[ID
char		gszUserId[80];  
// �p�X���[�h
char		gszPassword[80];
// WSAAsyncGetHostByName()�̃n���h��
HANDLE		hndlGetHost;           
// WSAAsyncGetHostByName()�p��HostEnt�o�b�t�@
char		bufHostEnt[MAXGETHOSTSTRUCT];
// �\�P�b�g
SOCKET 		socketPOP;
// wsprintf()�p�̃X�N���b�`�o�b�t�@
char		gszTemp[255];
// ���ԊǗ��Ɏg�p����ϐ�
// �Ꭶ�̂݁i���ۂɂ͕K�v�Ȃ��j
DWORD		gdwTicks;
DWORD		gdwElapsed;
// recv()�f�[�^�o�b�t�@
char		gbufRecv[256];

// --- �A�v���P�[�V�����̏��
int			gnAppState;
#define		STATE_CONNECTING 	1
#define		STATE_USER			2
#define		STATE_PASS			3
#define		STATE_STAT			4
#define		STATE_QUIT			5

//
// WinMain() - �G���g���|�C���g
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
	
	// WSAStartup()�̃o�[�W�����̏���
	WORD wVersionRequired = MAKEWORD(MAJOR_VERSION_REQUIRED,
								     MINOR_VERSION_REQUIRED
									 );
    
	// WinSock DLL�̏�����
	nReturnCode = WSAStartup(wVersionRequired, &wsaData);
	if (nReturnCode != 0 ) 
	{
		MessageBox(NULL,"Error on WSAStartup()",
					"CheckMail", MB_OK);
    	return 1;
	}
	
	// �v�����ꂽ�o�[�W���������p�ł��邱�Ƃ��m�F
	if (wsaData.wVersion != wVersionRequired)
	{
    	// �K�v�ȃo�[�W���������p�ł��Ȃ�
		MessageBox(NULL,"Wrong WinSock Version",
					"CheckMail", MB_OK);
    	WSACleanup();
    	return 1; 
	}

	// �_�C�A���O�{�b�N�X�����C���E�B���h�E�Ƃ��Ďg�p
	DialogBox(
			  hinstCurrent, 
			  MAKEINTRESOURCE(IDD_DIALOG_MAIN),
			  NULL, MainDialogProc);
	
	// WinSock DLL�����
	WSACleanup();
    return 0;
}


//
// MainDialogProc() - ���C���E�B���h�E�v���V�[�W��
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
			// WSADATA����̏���\��
			Display("----- STARTUP -----\r\n");
			Display("WSAStartup() succeeded\r\n\r\n");
			break;
				
		case WM_COMMAND:
			switch(wParam)
			{ 
				// ���[�U�[���mCheck Mail�n�{�^�����N���b�N����
				case ID_CHECKMAIL:  
					// POP�����擾
              		// �T�[�o�[���A���[�U�[ID�A�p�X���[�h
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

					// gethostbyname()�̔񓯊��o�[�W������
					// �g�p���Ă���z�X�g���̎Q�Ƃ�v���B
					// ���������炱�̃E�B���h�E��
					// SM_GETHOST���b�Z�[�W�𑗐M����B
					Display("----- FIND HOST -----\r\n");
					Display("Calling WSAAsyncGetHostByName()"
							" to find server\r\n");
					// �Q�l�̂��߂�WSAAsyncGetHostByName()��
					// ���Ԃ��v��
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
					
				// ���[�U�[�����C���E�B���h�E��
				// �L�����Z���{�^����������
				case IDCANCEL:
					if (gnAppState)
						CloseSocket(socketPOP);
					PostQuitMessage(0);
					fRet = TRUE;
					break;
			}
			break;

			// �񓯊�gethostbyname()�̖߂�l������
			case SM_GETHOST:
				HandleGetHostMsg(hwndDlg, wParam, lParam);
				fRet = TRUE;
				break;

			// �񓯊����b�Z�[�W������
			case SM_ASYNC:  
				HandleAsyncMsg(hwndDlg, wParam, lParam);
				fRet = TRUE;
				break;
	}
	return fRet;
}

//
// HandleGetHostMsg()
// WSAAsyncGetHostByName()�����������Ƃ��ɌĂяo�����
void HandleGetHostMsg(
			HWND hwndDlg, 
			WPARAM wParam, 
			LPARAM lParam)
{
	SOCKADDR_IN saServ;		// �C���^�[�l�b�g�p�\�P�b�g�A�h���X
	LPHOSTENT	lpHostEnt;	// �z�X�g�G���g���ւ̃|�C���^
	LPSERVENT	lpServEnt;	// �T�[�o�[�G���g���ւ̃|�C���^
	int nRet;				// �߂�R�[�h
	
	Display("SM_GETHOST message received\r\n");
	if ((HANDLE)wParam != hndlGetHost)
		return;        
		
	// �Q�l�̂��߂ɁAWSAGetHostByName()��
	// �o�ߎ��Ԃ�\��
	gdwElapsed = (GetTickCount() - gdwTicks);
	wsprintf((LPSTR)gszTemp,
			(LPSTR)"WSAAsyncGetHostByName() took %ld "
			" milliseconds to complete\r\n", 
			gdwElapsed);
	Display(gszTemp);

	// �G���[�R�[�h���`�F�b�N
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
 
	// �T�[�o�[�����������̂ŁA
	// bufHostEnt�ɃT�[�o�[��񂪊i�[����Ă���
	Display("Server found OK\r\n\r\n");
	
	Display("----- CONNECT TO HOST -----\r\n");	
	// �\�P�b�g���쐬
	Display("Calling socket(AF_INET, SOCK_STREAM, 0);\r\n");
	socketPOP = socket(AF_INET, SOCK_STREAM, 0);
	if (socketPOP == INVALID_SOCKET)
	{
		Display("Could not create a socket\r\n");
		EnableButtons(TRUE);
		return;
	}
	
	// �\�P�b�g���u���b�L���O�ɂ��A
	// �񓯊��ʒm��o�^����
	Display("Calling WSAAsyncSelect()\r\n");
	if (WSAAsyncSelect(socketPOP, hwndDlg, SM_ASYNC,
				FD_CONNECT|FD_READ|FD_WRITE|FD_CLOSE))
	{         
		Display("WSAAsyncSelect() failed\r\n");
		EnableButtons(TRUE);
		return;
	}

	// getservbyname()���g�p����
	// �|�[�g�ԍ�����������B
	// �ʏ�͂����Ɋ�������̂ŁA�P����
	// �����o�[�W�������g�p����B
	// �Q�l�̂��߂�getservbyname()��
	// ���Ԃ��v��B
	gdwTicks = GetTickCount();
	lpServEnt = getservbyname("pop3", "tcp");
	gdwElapsed = (GetTickCount() - gdwTicks);
	wsprintf((LPSTR)gszTemp,
			(LPSTR)"getservbyname() took %ld milliseconds"
			" to complete\r\n", 
			gdwElapsed);
	Display(gszTemp);
	
	// servent���Ȃ��ꍇ
	if (lpServEnt == NULL) 
	{
	   // �悭�m��ꂽ�|�[�g���g�p
		saServ.sin_port = htons(110);
		Display("getservbyent() failed. Using port 110\r\n");
	}
	else
	{
		// servent�ɕԂ��ꂽ�|�[�g���g�p
		saServ.sin_port = lpServEnt->s_port;
	}
	// �T�[�o�[�A�h���X�\���̂��g�p
	saServ.sin_family = AF_INET;
	lpHostEnt = (LPHOSTENT)bufHostEnt;
	saServ.sin_addr = *((LPIN_ADDR)*lpHostEnt->h_addr_list);
	// �\�P�b�g��ڑ�
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
// WSA WSAAsyncSelct()�C�x���g���g���K�[���ꂽ�Ƃ��ɌĂяo�����
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
			// �ҋ@���̃f�[�^���擾
			nBytesRead = recv(
				socketPOP,  		// �\�P�b�g
				gbufRecv, 			// �o�b�t�@
				sizeof(gbufRecv), 	// �ǂݎ��ő咷
				0);					// ��M�t���O
			// recv()�̖߂�R�[�h���`�F�b�N
			if (nBytesRead == 0)
			{
				// �ڑ�������ꂽ
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
			// �o�b�t�@�̖�����NULL
			gbufRecv[nBytesRead] = '\0';
			// �����n���ĉ��߂�����
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
// ���M�f�[�^��ǂݎ��A���s����
//
void ProcessData(HWND hwndDlg, LPSTR lpBuf, int nBytesRead)
{
	static char szResponse[512];
	static int nLen = 0;
	char *cp;
                         
	// �V�����f�[�^���o�b�t�@�ɓ����
	// �]�n�����邩�ǂ������`�F�b�N
	if ((nLen + nBytesRead) > sizeof(szResponse))
	{
		Display("!!!!! Buffer overrun, data truncated\r\n");
		nLen = 0;
		szResponse[0] = '\0';
		return;
	}                           
	// �o�b�t�@�ɐV�����f�[�^��ǉ�
	strcat(szResponse, lpBuf);
	nLen = strlen(szResponse);

	// ���ׂĂ̊��S�ȍs������
	while(1)
	{
		// �o�b�t�@�Ɋ��S�ȍs���܂܂�Ă��邩
		cp = strchr(szResponse, '\n');
		if (cp == NULL)
			break;
		// CR/LF�̑g�ݍ��킹������
		// LF��NULL�Œu��������
		*cp = '\0';
		// ProcesLine()�Ɉ����n��
		ProcessLine(hwndDlg, szResponse);
		// �c��̃f�[�^���o�b�t�@�̐擪�Ɉړ�
		cp++;
		if (strlen(cp))
			memmove(szResponse, cp, strlen(cp)+1);
		else
			szResponse[0] = '\0';
	}
}          

//
// ProcessLine()
// �T�[�o�[����̉����s���������A
// ���̓�������肷��
//
void ProcessLine(HWND hwndDlg, LPSTR lpStr)
{                      
	int nRet;
	long lCount;
	long lSize;

	Display("Response from server:\r\n");
	Display(lpStr);
	Display("\r\n");
		
	// �����̃G���[���`�F�b�N
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
				socketPOP,  		// �\�P�b�g
				gszTemp, 			// �f�[�^�o�b�t�@
				strlen(gszTemp),	// �f�[�^�̒���
				0);					// ���M�t���O
		gnAppState = STATE_QUIT;
		return;
	}	                          

	// �m��̉�����������
	switch(gnAppState)
	{
		case STATE_CONNECTING:
			// ���O�C���v����USER�����𑗐M���A
			// �A�v���P�[�V�����̏�Ԃ�ݒ�
			Display("AppState = CONNECTING, "
			        "sending USER\r\n");
			wsprintf(gszTemp, "USER %s\r\n", gszUserId);
			nRet = send(
					socketPOP,		// �\�P�b�g
					gszTemp,		// �f�[�^�o�b�t�@
					strlen(gszTemp),// �f�[�^�̒���
					0);				// ���M�t���O
			gnAppState = STATE_USER;
			break;
			
		case STATE_USER:
			// ���O�C���v����PASSword�����𑗐M
			// �A�v���P�[�V�����̏�Ԃ�ݒ�
			Display("AppState = USER, sending PASS\r\n");
			wsprintf(gszTemp, "PASS %s\r\n", gszPassword);
			nRet = send(
					socketPOP,		// �\�P�b�g
					gszTemp,		// �f�[�^�o�b�t�@
					strlen(gszTemp),// �f�[�^�̒���
					0);				// ���M�t���O
			gnAppState = STATE_PASS;
			break;
			
		case STATE_PASS:
			// STAT�R�}���h�v���𑗐M
			// �A�v���P�[�V�����̏�Ԃ�ݒ�
			Display("AppState = PASS, sending STAT\r\n");
			wsprintf(gszTemp, "STAT\r\n");
			nRet = send(
					socketPOP,		// �\�P�b�g
					gszTemp,		// �f�[�^�o�b�t�@
					strlen(gszTemp),// �f�[�^�̒���
					0);				// ���M�t���O
			gnAppState = STATE_STAT;
			break;
			
		case STATE_STAT:
			// STAT������ǂݎ��
			// ���ʂ��o��
			Display("AppState = STAT, reading response\r\n");
			sscanf(lpStr, "%s %ld %ld", 
				   gszTemp, &lCount, &lSize);
			Display("----- RESULT -----\r\n");
			wsprintf(gszTemp, "%ld messages %ld bytes\r\n", 
					lCount, lSize);
			Display(gszTemp);
			// QUIT�R�}���h�𑗐M
			// �A�v���P�[�V�����̏�Ԃ�ݒ�
			Display("Sending QUIT\r\n");
			wsprintf(gszTemp, "QUIT\r\n");
			nRet = send(
					socketPOP,  		// �\�P�b�g
					gszTemp, 			// �f�[�^�o�b�t�@
					strlen(gszTemp),	// �f�[�^�̒���
					0);					// ���M�t���O
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
// �v���g�R���X�^�b�N�o�b�t�@���N���A�������
// �\�P�b�g�����
//
void CloseSocket(SOCKET sock)
{   
	int nRet;
	char szBuf[255];
	
	// ����ȏ�f�[�^�𑗐M���Ȃ�
	// ���Ƃ������[�g�ɒm�点��
	shutdown(sock, 1);
	while(1)
	{
		// �f�[�^����M���悤�Ƃ���B
		// ����ɂ��A�v���g�R���X�^�b�N����
		// �o�b�t�@����Ă����f�[�^��
		// �m���ɃN���A�����B
		nRet = recv(
				sock,			// �\�P�b�g
				szBuf,			// �f�[�^�o�b�t�@
				sizeof(szBuf),	// �o�b�t�@�̒���
				0);				// ��M�t���O
		// �ڑ�����邩�ق��̃G���[���N����
		// �ꍇ�̓f�[�^�̎�M���~
		if (nRet == 0 || nRet == SOCKET_ERROR)
			break;
	}
	// ����ȏ�f�[�^����M���Ȃ����Ƃ�
	// ����ɒm�点��
	shutdown(sock, 2);
	// �\�P�b�g�����
	closesocket(sock);
}