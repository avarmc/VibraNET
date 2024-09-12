
// ViDesktopView.h : interface of the CViDesktopView class
//

#pragma once


class CViDesktopView : public CView
{
protected: // create from serialization only
	CViDesktopView();
	DECLARE_DYNCREATE(CViDesktopView)

// Attributes
public:
	CViDesktopDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Implementation
public:
	virtual ~CViDesktopView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

#ifndef _DEBUG  // debug version in ViDesktopView.cpp
inline CViDesktopDoc* CViDesktopView::GetDocument() const
   { return reinterpret_cast<CViDesktopDoc*>(m_pDocument); }
#endif

