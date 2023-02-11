// view.cpp : CTermView�N���X�̓���̒�`���s��
//

#include "stdafx.h"
#include "wsterm.h"

#include "doc.h"
#include "termsock.h"
#include "view.h"
#include "connectd.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CTermView

IMPLEMENT_DYNCREATE(CTermView, CEditView)

BEGIN_MESSAGE_MAP(CTermView, CEditView)
	//{{AFX_MSG_MAP(CTermView)
	ON_COMMAND(ID_SOCKET_CONNECT, OnSocketConnect)
	ON_UPDATE_COMMAND_UI(ID_SOCKET_CONNECT, OnUpdateSocketConnect)
	ON_COMMAND(ID_SOCKET_CLOSE, OnSocketClose)
	ON_UPDATE_COMMAND_UI(ID_SOCKET_CLOSE, OnUpdateSocketClose)
	ON_WM_CHAR()
	ON_COMMAND(ID_VIEW_SOCKETNOTIFICATIONS, OnViewSocketNotifications)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SOCKETNOTIFICATIONS, OnUpdateViewSocketNotifications)
	ON_COMMAND(ID_EDIT_CLEARBUFFER, OnEditClearBuffer)
	//}}AFX_MSG_MAP
	// �W������R�}���h
	ON_COMMAND(ID_FILE_PRINT, CEditView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CEditView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTermView�N���X�̍쐬�Ɣj��

CTermView::CTermView()
{
	m_pSocket = NULL;
	m_fConnected = FALSE;
	m_fShowNotifications = FALSE;
}

CTermView::~CTermView()
{
	// �\�P�b�g�����蓖�ĂĂ���ꍇ
	if (m_pSocket != NULL)
	{            
		// �܂��ڑ����̏ꍇ
		if (m_fConnected)
			m_pSocket->Close();
		delete m_pSocket;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CTermView�N���X�̕`��

void CTermView::OnDraw(CDC* pDC)
{
	CTermDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// TODO: ���̏ꏊ�Ƀl�C�e�B�u�f�[�^�̕`��R�[�h��ǉ�����
}

/////////////////////////////////////////////////////////////////////////////
// CTermView�N���X�̈��

BOOL CTermView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// �f�t�H���g��CEditView�̏���
	return CEditView::OnPreparePrinting(pInfo);
}

void CTermView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	// �f�t�H���g��CEditView�̈���J�n
	CEditView::OnBeginPrinting(pDC, pInfo);
}

void CTermView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	// �f�t�H���g��CEditView�̈���I��
	CEditView::OnEndPrinting(pDC, pInfo);
}

/////////////////////////////////////////////////////////////////////////////
// CTermView�N���X�̐f�f

#ifdef _DEBUG
void CTermView::AssertValid() const
{
	CEditView::AssertValid();
}

void CTermView::Dump(CDumpContext& dc) const
{
	CEditView::Dump(dc);
}

CTermDoc* CTermView::GetDocument() // ��f�o�b�O�o�[�W�����̓C�����C��
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CTermDoc)));
	return (CTermDoc*)m_pDocument;
}
#endif //_DEBUG

CTermView *CTermView::GetView()
{
	CFrameWnd *pFrame = (CFrameWnd *)(AfxGetApp()->m_pMainWnd);
	CView *pView = pFrame->GetActiveView();
	if (!pView)
		return NULL;
	if (!pView->IsKindOf(RUNTIME_CLASS(CTermView)))
		return NULL;
	return (CTermView *)pView;
}

void CTermView::OnEditClearBuffer()
{
	SetWindowText(NULL);
}


// �G�f�B�b�g�R���g���[����
// �s��\������w���p�[�֐�
void CTermView::Display(LPCSTR lpFormat, ...)
{
	// �G�f�B�b�g�R���g���[���������ς����ǂ���
	CEdit& ed = GetEditCtrl();
	if (ed.GetLineCount() > 1000)
	{
		// �S�̂���ɂ���
		SetWindowText(NULL);
	}
	
	va_list Marker;
	static char szBuf[256];
	
	// �e�L�X�g�𕶎���ɏ������݁A
	// �G�f�B�b�g�R���g���[���ɒǉ�����
	va_start(Marker, lpFormat);
	vsprintf(szBuf, lpFormat, Marker);
	va_end(Marker);
	ed.SetSel(-1,-1);
	ed.ReplaceSel(szBuf);
}

/////////////////////////////////////////////////////////////////////////////
// CTermView�N���X�̃��b�Z�[�W�n���h��

// ���[�U�[���mENTER�n����������A
// ���݂̍s�𑗂�
void CTermView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
 	// �܂��ڑ����Ă��Ȃ��ꍇ
	if (!m_fConnected)
	{
		// �\�P�b�g��ڑ�����悤���[�U�[�Ƀ��b�Z�[�W��\������
		AfxMessageBox("Choose Socket/Connect to start\r\n");
		return;
	}   
	// ���[�U�[���mENTER�n���������ꍇ
	if (nChar == 13)
	{               
		// ���݂̍s�𒲂ׂ�
		CEdit& ed = GetEditCtrl();
		int iStart, iEnd;
		ed.GetSel(iStart, iEnd);
		int iLine = ed.LineFromChar(iStart);
		if (iLine > -1)
		{
			static char szLine[256];
			memset(szLine, 0, sizeof(szLine));
			// �s�S�̂��擾����
			int iNdx = ed.GetLine(iLine, szLine, sizeof(szLine)-1);
			if (iNdx > 0)
			{                    
				// �\�P�b�g���g���čs�𑗐M����
				strcat(szLine, "\r\n");
				m_pSocket->Send(szLine, strlen(szLine));
			}
		}
	}
	CEditView::OnChar(nChar, nRepCnt, nFlags);
}

void CTermView::OnViewSocketNotifications()
{
	if (m_fShowNotifications)
		m_fShowNotifications = FALSE;
	else
		m_fShowNotifications = TRUE;
}

void CTermView::OnUpdateViewSocketNotifications(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_fShowNotifications);
}


// �mSocket�n���mConnect�n���j���[�̃n���h��
void CTermView::OnSocketConnect()
{
	// �mConnect�n�_�C�A���O�{�b�N�X��\������
	CConnectDialog dlg;
	if (dlg.DoModal() != IDOK)
		return;
	Display("Connect to port %d on %s...\r\n", 
			dlg.m_nPort,
			dlg.m_strHostName);

	// �O��̐ڑ��̃\�P�b�g��
	// �c���Ă��Ȃ����Ƃ��m�F����
	if (m_pSocket != NULL)
	{
		delete m_pSocket;
		m_pSocket = NULL;
	}
			
	// �V����CTermSocket�\�P�b�g���쐬����
	m_pSocket = new CTermSocket();
	
	// ���ׂăf�t�H���g�l���g����
	// Create()�����o���Ăяo��
	if (!m_pSocket->Create())
	{
		AfxMessageBox("Socket creation failed");
		return;
	} 
	
	// �z�X�g�ɐڑ�����
	if (!m_pSocket->Connect(dlg.m_strHostName, dlg.m_nPort))
	{   
		if (m_pSocket->GetLastError() != WSAEWOULDBLOCK)
		{                                
			CString strError;
			strError.LoadString(m_pSocket->GetLastError());
			Display("Connect() failed: %s\r\n",
				strError);
			m_fConnected = FALSE;
			delete m_pSocket;
			m_pSocket = NULL;
		}
	}
	// OnConnect�ɂ���Ēʒm�����܂ł́A
	// �܂��ڑ����Ă�����̂Ƒz�肷��
	// ����́ApCmdUI���j���[���@�\�����邽�߂ɍs��
	m_fConnected = TRUE;
}

void CTermView::OnUpdateSocketConnect(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!m_fConnected);
}

void CTermView::OnSocketClose()
{                      
	// �\�P�b�g�̍s�V�悢�N���[�Y���J�n
	// ���葤�ɂ���ȏ�f�[�^��
	// ���M���Ȃ����Ƃ�m�点��
	m_pSocket->ShutDown(CAsyncSocket::sends);
	
	// �ۗ����̃f�[�^�����ׂĎ󂯎��
	int nRet;
	char szBuf[256];
	while(1)
	{
    	nRet = m_pSocket->Receive(szBuf, sizeof(szBuf));
    	if (nRet == 0 || nRet == SOCKET_ERROR)
    		break;
	}
	// �f�[�^������ȏ㑗��M���Ȃ����Ƃ�
	// ���葤�ɓ`����
	m_pSocket->ShutDown(CAsyncSocket::both);
	
	// �\�P�b�g�����
	m_pSocket->Close();
	// �\�P�b�g���폜����
	delete m_pSocket;
	m_pSocket = NULL;
	m_fConnected = FALSE;
}

void CTermView::OnUpdateSocketClose(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_fConnected);
}

void CTermView::OnConnect(int nErrorCode)
{
	if (m_fShowNotifications)
		Display("\tOnConnect(%d)\r\n", nErrorCode);
		
	if (nErrorCode)
	{
		m_fConnected = FALSE;
		Display("\tError OnConnect(): %d\r\n",
					nErrorCode);
	}
	else
	{
		m_fConnected = TRUE;
		Display("\tSocket connected\r\n");
	}
}

void CTermView::OnSend(int nErrorCode)
{
	if (m_fShowNotifications)
		Display("\tOnSend(%d)\r\n", nErrorCode);
}

void CTermView::OnReceive(int nErrorCode)
{
	if (m_fShowNotifications)
		Display("\tOnReceive(%d)\r\n", nErrorCode);
		
	static char szBuf[256];
	// �o�b�t�@�����ׂ�0�ɐݒ肷��
	// NULL�ŏI���
	memset(szBuf, 0, sizeof(szBuf));
	// �o�b�t�@�T�C�Y����1�o�C�g�������T�C�Y��
	// ��M�̍ő�e��
	// �Ō�̕����͕K��NULL
	int nBytes = m_pSocket->Receive(szBuf, 255, 0);
	if (nBytes == 0)
	{
		Display("Receive() indicates that socket is closed\r\n");
		return;
	}
	if (nBytes == SOCKET_ERROR)
	{                            
		CString strError;
		strError.LoadString(m_pSocket->GetLastError());
		Display("Receive error %s\r\n", strError);
		return;
	}
	Display(szBuf);
}

void CTermView::OnClose(int nErrorCode)
{                       
	if (m_fShowNotifications)
		Display("\tOnClose(%d)\r\n", nErrorCode);

	AfxMessageBox("Socket closed");
	m_fConnected = FALSE;
}



