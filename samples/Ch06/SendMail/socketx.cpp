// SOCKETX.CPP -- CSocketクラスの拡張バージョン
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
	// タイムアウト値が指定されている場合は、それを設定する
	if (uTimeOut > 0)
		SetTimeOut(uTimeOut);

	// 基本クラスの関数を呼び出す
	int nRet = CSocket::Send(lpszStr, strlen(lpszStr), nFlags);

	// タイムアウト値が設定してある場合
	if (uTimeOut > 0)
	{
		KillTimeOut();
		// 操作がタイムアウトになった場合は、
		// もっと自然なエラーメッセージを表示する
		if (GetLastError() == WSAEINTR)
			SetLastError(WSAETIMEDOUT);
	}
	return nRet;
}


int CSocketX::Receive(CString& str, UINT uTimeOut, int nFlags)
{
	static char szBuf[256];
	memset(szBuf, 0, sizeof(szBuf));

	// タイムアウト値が指定されている場合は、それを設定する
	if (uTimeOut > 0)
		SetTimeOut(uTimeOut);

	// 基本クラスの関数を呼び出す
	int nRet = CSocket::Receive(szBuf, sizeof(szBuf), nFlags);

	// タイムアウト値を設定してある場合
	if (uTimeOut > 0)
	{
		KillTimeOut();
		// 操作がタイムアウトになった場合は、
		// もっと自然なエラーメッセージを表示する
		if (nRet == SOCKET_ERROR)
		{
			if (GetLastError() == WSAEINTR)
				SetLastError(WSAETIMEDOUT);
		}
	}

	// CString参照を設定する
	str = szBuf;
	return nRet;
}

BOOL CSocketX::OnMessagePending() 
{
	MSG msg;

	// タイマーメッセージを監視する
	if(::PeekMessage(&msg, NULL, WM_TIMER, WM_TIMER, PM_NOREMOVE))
	{
		// 指定のタイムアウト値が経過した場合
		if (msg.wParam == (UINT) m_nTimerID)
		{
			// メッセージを削除する
			::PeekMessage(&msg, NULL, WM_TIMER, WM_TIMER, PM_REMOVE);
			// 呼び出しを取り消す
			CancelBlockingCall();
			return FALSE;
		}
	}
	// 基本クラスの関数を呼び出す
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
