// ViViewNull.cpp : implementation file
//

#include "stdafx.h"
#include "ViDesktop.h"
#include "ViViewNull.h"


// CViViewNull

IMPLEMENT_DYNCREATE(CViViewNull, CWnd)

CViViewNull::CViViewNull()
{

}

CViViewNull::~CViViewNull()
{
}


BEGIN_MESSAGE_MAP(CViViewNull, CWnd)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()



// CViViewNull message handlers




void CViViewNull::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: Add your message handler code here
					   // Do not call CWnd::OnPaint() for painting messages

	CRect rc;
	GetClientRect(&rc);
#ifdef _DEBUG
	dc.FillSolidRect(&rc, 0xFFFFFF);
#else
	dc.FillSolidRect(&rc, 0xaFaFaF);
#endif
}


int CViViewNull::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (__super::OnCreate(lpCreateStruct) == -1)
		return -1;

	ModifyStyle(0, WS_CLIPCHILDREN | WS_CLIPSIBLINGS);

	return 0;
}


void CViViewNull::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	CWnd* pWnd = 0;
	while (pWnd = FindWindowEx(m_hWnd, pWnd ? pWnd->m_hWnd : 0, 0, 0))
	{
		pWnd->MoveWindow(0, 0, cx, cy);
	}
}
