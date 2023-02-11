// view.h : CTermViewクラスの宣言およびインターフェイスの定義を行う
//
/////////////////////////////////////////////////////////////////////////////

class CTermView : public CEditView
{
protected: // シリアル化機能のみから作成
	CTermView();
	DECLARE_DYNCREATE(CTermView)

// アトリビュート
public:
	CTermDoc* GetDocument();
	static CTermView *GetView();
	CTermSocket*	m_pSocket;
	BOOL			m_fConnected;
	BOOL			m_fShowNotifications;

// オペレーション
public:
	void Display(LPCSTR lpFormat, ...);
	void OnConnect(int nErrorCode);
	void OnSend(int nErrorCode);
	void OnReceive(int nErrorCode);
	void OnClose(int nErrorCode);

// インプリメンテーション
public:
	virtual ~CTermView();
	virtual void OnDraw(CDC* pDC);  // このビューを描画する際にオーバーライドされる
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	// 印刷サポート
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// 生成されたメッセージマップ関数
protected:
	//{{AFX_MSG(CTermView)
	afx_msg void OnSocketConnect();
	afx_msg void OnUpdateSocketConnect(CCmdUI* pCmdUI);
	afx_msg void OnSocketClose();
	afx_msg void OnUpdateSocketClose(CCmdUI* pCmdUI);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnViewSocketNotifications();
	afx_msg void OnUpdateViewSocketNotifications(CCmdUI* pCmdUI);
	afx_msg void OnEditClearBuffer();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // view.cppファイルがデバッグ環境のとき使用される
inline CTermDoc* CTermView::GetDocument()
   { return (CTermDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////
