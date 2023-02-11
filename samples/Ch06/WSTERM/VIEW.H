// view.h : CTermView�N���X�̐錾����уC���^�[�t�F�C�X�̒�`���s��
//
/////////////////////////////////////////////////////////////////////////////

class CTermView : public CEditView
{
protected: // �V���A�����@�\�݂̂���쐬
	CTermView();
	DECLARE_DYNCREATE(CTermView)

// �A�g���r���[�g
public:
	CTermDoc* GetDocument();
	static CTermView *GetView();
	CTermSocket*	m_pSocket;
	BOOL			m_fConnected;
	BOOL			m_fShowNotifications;

// �I�y���[�V����
public:
	void Display(LPCSTR lpFormat, ...);
	void OnConnect(int nErrorCode);
	void OnSend(int nErrorCode);
	void OnReceive(int nErrorCode);
	void OnClose(int nErrorCode);

// �C���v�������e�[�V����
public:
	virtual ~CTermView();
	virtual void OnDraw(CDC* pDC);  // ���̃r���[��`�悷��ۂɃI�[�o�[���C�h�����
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	// ����T�|�[�g
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// �������ꂽ���b�Z�[�W�}�b�v�֐�
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

#ifndef _DEBUG  // view.cpp�t�@�C�����f�o�b�O���̂Ƃ��g�p�����
inline CTermDoc* CTermView::GetDocument()
   { return (CTermDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////