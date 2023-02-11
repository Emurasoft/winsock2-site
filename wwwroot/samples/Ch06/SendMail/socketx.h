// SOCKETX.H -- CSocket�N���X�̊g���o�[�W����
//

#ifndef __SOCKETX_H__
#define __SOCKETX_H__

class CSocketX : public CSocket
{
	DECLARE_DYNAMIC(CSocketX);

// �C���v�������e�[�V����
public:
	int Send(LPCTSTR lpszStr, UINT uTimeOut = 0, int nFlags = 0);
	int Receive(CString& str, UINT uTimeOut = 0, int nFlags = 0);
	BOOL SetTimeOut(UINT uTimeOut);
	BOOL KillTimeOut();

protected: 
	virtual BOOL OnMessagePending();

private: 
	int m_nTimerID;
};
#endif // __SOCKETX_H__
