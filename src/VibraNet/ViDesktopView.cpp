
// ViDesktopView.cpp : implementation of the CViDesktopView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "ViDesktop.h"
#endif

#include "ViDesktopDoc.h"
#include "ViDesktopView.h"
#include "ViDesktopFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CViDesktopView

IMPLEMENT_DYNCREATE(CViDesktopView, CView)

BEGIN_MESSAGE_MAP(CViDesktopView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CViDesktopView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_SIZE()
END_MESSAGE_MAP()

// CViDesktopView construction/destruction

CViDesktopView::CViDesktopView()
{
	theApp.m_pView = this;

}

CViDesktopView::~CViDesktopView()
{
	theApp.m_pView = NULL;
}

BOOL CViDesktopView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CViDesktopView drawing

void CViDesktopView::OnDraw(CDC* /*pDC*/)
{
	CViDesktopDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}


// CViDesktopView printing


void CViDesktopView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CViDesktopView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CViDesktopView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CViDesktopView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CViDesktopView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CViDesktopView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CViDesktopView diagnostics

#ifdef _DEBUG
void CViDesktopView::AssertValid() const
{
	CView::AssertValid();
}

void CViDesktopView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CViDesktopDoc* CViDesktopView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CViDesktopDoc)));
	return (CViDesktopDoc*)m_pDocument;
}
#endif //_DEBUG


// CViDesktopView message handlers


void CViDesktopView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	if (m_hWnd && IsWindow(theApp.m_pFrame->m_wndCfg))
		theApp.m_pFrame->m_wndCfg.AdjustLayout();
}
