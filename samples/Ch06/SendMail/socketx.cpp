// SOCKETX.CPP -- CSocket�N���X�̊g���o�[�W����
//

#include "stdafx.h"
#include "socketx.h"


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CSocketX, CSocket)

int CSocketX::Send(LPCTSTR lpszStr, UINT uTimeOut, int nFlags)
{
	// �^�C���A�E�g�l���w�肳��Ă���ꍇ�́A�����ݒ肷��
	if (uTimeOut > 0)
		SetTimeOut(uTimeOut);

	// ��{�N���X�̊֐����Ăяo��
	int nRet = CSocket::Send(lpszStr, strlen(lpszStr), nFlags);

	// �^�C���A�E�g�l���ݒ肵�Ă���ꍇ
	if (uTimeOut > 0)
	{
		KillTimeOut();
		// ���삪�^�C���A�E�g�ɂȂ����ꍇ�́A
		// �����Ǝ��R�ȃG���[���b�Z�[�W��\������
		if (GetLastError() == WSAEINTR)
			SetLastError(WSAETIMEDOUT);
	}
	return nRet;
}


int CSocketX::Receive(CString& str, UINT uTimeOut, int nFlags)
{
	static char szBuf[256];
	memset(szBuf, 0, sizeof(szBuf));

	// �^�C���A�E�g�l���w�肳��Ă���ꍇ�́A�����ݒ肷��
	if (uTimeOut > 0)
		SetTimeOut(uTimeOut);

	// ��{�N���X�̊֐����Ăяo��
	int nRet = CSocket::Receive(szBuf, sizeof(szBuf), nFlags);

	// �^�C���A�E�g�l��ݒ肵�Ă���ꍇ
	if (uTimeOut > 0)
	{
		KillTimeOut();
		// ���삪�^�C���A�E�g�ɂȂ����ꍇ�́A
		// �����Ǝ��R�ȃG���[���b�Z�[�W��\������
		if (nRet == SOCKET_ERROR)
		{
			if (GetLastError() == WSAEINTR)
				SetLastError(WSAETIMEDOUT);
		}
	}

	// CString�Q�Ƃ�ݒ肷��
	str = szBuf;
	return nRet;
}

BOOL CSocketX::OnMessagePending() 
{
	MSG msg;

	// �^�C�}�[���b�Z�[�W���Ď�����
	if(::PeekMessage(&msg, NULL, WM_TIMER, WM_TIMER, PM_NOREMOVE))
	{
		// �w��̃^�C���A�E�g�l���o�߂����ꍇ
		if (msg.wParam == (UINT) m_nTimerID)
		{
			// ���b�Z�[�W���폜����
			::PeekMessage(&msg, NULL, WM_TIMER, WM_TIMER, PM_REMOVE);
			// �Ăяo����������
			CancelBlockingCall();
			return FALSE;
		}
	}
	// ��{�N���X�̊֐����Ăяo��
	return CSocket::OnMessagePending();
} 


BOOL CSocketX::SetTimeOut(UINT uTimeOut) 
{ 
	m_nTimerID = SetTimer(NULL,0,uTimeOut,NULL);
	return m_nTimerID;
} 


BOOL CSocketX::KillTimeOut() 
{ 
	return KillTimer(NULL,m_nTimerID);
} 
