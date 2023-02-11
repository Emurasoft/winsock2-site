// view.cpp : CTermViewクラスの動作の定義を行う
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
	// 標準印刷コマンド
	ON_COMMAND(ID_FILE_PRINT, CEditView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CEditView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTermViewクラスの作成と破棄

CTermView::CTermView()
{
	m_pSocket = NULL;
	m_fConnected = FALSE;
	m_fShowNotifications = FALSE;
}

CTermView::~CTermView()
{
	// ソケットを割り当ててある場合
	if (m_pSocket != NULL)
	{            
		// まだ接続中の場合
		if (m_fConnected)
			m_pSocket->Close();
		delete m_pSocket;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CTermViewクラスの描画

void CTermView::OnDraw(CDC* pDC)
{
	CTermDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// TODO: この場所にネイティブデータの描画コードを追加する
}

/////////////////////////////////////////////////////////////////////////////
// CTermViewクラスの印刷

BOOL CTermView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// デフォルトのCEditViewの準備
	return CEditView::OnPreparePrinting(pInfo);
}

void CTermView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	// デフォルトのCEditViewの印刷開始
	CEditView::OnBeginPrinting(pDC, pInfo);
}

void CTermView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	// デフォルトのCEditViewの印刷終了
	CEditView::OnEndPrinting(pDC, pInfo);
}

/////////////////////////////////////////////////////////////////////////////
// CTermViewクラスの診断

#ifdef _DEBUG
void CTermView::AssertValid() const
{
	CEditView::AssertValid();
}

void CTermView::Dump(CDumpContext& dc) const
{
	CEditView::Dump(dc);
}

CTermDoc* CTermView::GetDocument() // 非デバッグバージョンはインライン
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


// エディットコントロールに
// 行を表示するヘルパー関数
void CTermView::Display(LPCSTR lpFormat, ...)
{
	// エディットコントロールがいっぱいかどうか
	CEdit& ed = GetEditCtrl();
	if (ed.GetLineCount() > 1000)
	{
		// 全体を空にする
		SetWindowText(NULL);
	}
	
	va_list Marker;
	static char szBuf[256];
	
	// テキストを文字列に書き込み、
	// エディットコントロールに追加する
	va_start(Marker, lpFormat);
	vsprintf(szBuf, lpFormat, Marker);
	va_end(Marker);
	ed.SetSel(-1,-1);
	ed.ReplaceSel(szBuf);
}

/////////////////////////////////////////////////////////////////////////////
// CTermViewクラスのメッセージハンドラ

// ユーザーが［ENTER］を押したら、
// 現在の行を送る
void CTermView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
 	// まだ接続していない場合
	if (!m_fConnected)
	{
		// ソケットを接続するようユーザーにメッセージを表示する
		AfxMessageBox("Choose Socket/Connect to start\r\n");
		return;
	}   
	// ユーザーが［ENTER］を押した場合
	if (nChar == 13)
	{               
		// 現在の行を調べる
		CEdit& ed = GetEditCtrl();
		int iStart, iEnd;
		ed.GetSel(iStart, iEnd);
		int iLine = ed.LineFromChar(iStart);
		if (iLine > -1)
		{
			static char szLine[256];
			memset(szLine, 0, sizeof(szLine));
			// 行全体を取得する
			int iNdx = ed.GetLine(iLine, szLine, sizeof(szLine)-1);
			if (iNdx > 0)
			{                    
				// ソケットを使って行を送信する
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


// ［Socket］→［Connect］メニューのハンドラ
void CTermView::OnSocketConnect()
{
	// ［Connect］ダイアログボックスを表示する
	CConnectDialog dlg;
	if (dlg.DoModal() != IDOK)
		return;
	Display("Connect to port %d on %s...\r\n", 
			dlg.m_nPort,
			dlg.m_strHostName);

	// 前回の接続のソケットが
	// 残っていないことを確認する
	if (m_pSocket != NULL)
	{
		delete m_pSocket;
		m_pSocket = NULL;
	}
			
	// 新しいCTermSocketソケットを作成する
	m_pSocket = new CTermSocket();
	
	// すべてデフォルト値を使って
	// Create()メンバを呼び出す
	if (!m_pSocket->Create())
	{
		AfxMessageBox("Socket creation failed");
		return;
	} 
	
	// ホストに接続する
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
	// OnConnectによって通知されるまでは、
	// まだ接続しているものと想定する
	// これは、pCmdUIメニューを機能させるために行う
	m_fConnected = TRUE;
}

void CTermView::OnUpdateSocketConnect(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!m_fConnected);
}

void CTermView::OnSocketClose()
{                      
	// ソケットの行儀よいクローズを開始
	// 相手側にこれ以上データを
	// 送信しないことを知らせる
	m_pSocket->ShutDown(CAsyncSocket::sends);
	
	// 保留中のデータをすべて受け取る
	int nRet;
	char szBuf[256];
	while(1)
	{
    	nRet = m_pSocket->Receive(szBuf, sizeof(szBuf));
    	if (nRet == 0 || nRet == SOCKET_ERROR)
    		break;
	}
	// データをこれ以上送受信しないことを
	// 相手側に伝える
	m_pSocket->ShutDown(CAsyncSocket::both);
	
	// ソケットを閉じる
	m_pSocket->Close();
	// ソケットを削除する
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
	// バッファをすべて0に設定する
	// NULLで終わる
	memset(szBuf, 0, sizeof(szBuf));
	// バッファサイズから1バイト引いたサイズが
	// 受信の最大容量
	// 最後の文字は必ずNULL
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



