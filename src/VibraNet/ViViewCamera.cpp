// ViViewNull.cpp : implementation file
//

#include "stdafx.h"
#include "ViDesktop.h"
#include "ViViewCamera.h"
#include "ViNet.h"
#include "ImagePack.h"
#include "ViDesktopCfg.h"

// CViViewCamera

IMPLEMENT_DYNCREATE(CViViewCamera, CWnd)

CViViewCamera::CViViewCamera():m_pNet(0), m_imgVer(0), m_statusVer(0), m_imgWndRect(0,0,0,0)
{
	m_statusCL = 60;
	CSingleLock listLock(&theApp.m_listLock,TRUE);
	theApp.m_listCamera.push_back(this);
}

CViViewCamera::~CViViewCamera()
{
	SAFE_DELETE(m_pNet);

	CSingleLock listLock(&theApp.m_listLock, TRUE);
	theApp.m_listCamera.remove(this);
}


BEGIN_MESSAGE_MAP(CViViewCamera, CWnd)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_SIZE()
	ON_WM_ACTIVATE()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()



// CViViewCamera message handlers




void CViViewCamera::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: Add your message handler code here
					   // Do not call CWnd::OnPaint() for painting messages

	CRect rc;
	GetClientRect(&rc);
#ifdef _DEBUG
	dc.FillSolidRect(&rc, 0xFF7FFF);
#else
	dc.FillSolidRect(&rc, 0xFFFFFF);
#endif
}


int CViViewCamera::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (__super::OnCreate(lpCreateStruct) == -1)
		return -1;

	ModifyStyle(0, WS_CLIPCHILDREN | WS_CLIPSIBLINGS);

	SAFE_DELETE(m_pNet);

	m_pNet = new CVINetServerDoc;
	m_pNet->m_pItem = pItem;
	m_pNet->m_pCamera = this;

	if (!m_pNet->Init())
		SetTimer(TIMER_INIT, 250, 0);

	Align();

	SetTimer(TIMER_IMAGE, 5, 0);
	return 0;
}




void CViViewCamera::OnDestroy()
{
	if (m_pNet)
		m_pNet->Close();
	if (IsWindow(m_imgWnd))
		m_imgWnd.DestroyWindow();
	__super::OnDestroy();
}


void CViViewCamera::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent)
	{
	case TIMER_INIT:
		if (m_pNet && m_pNet->Init())
			KillTimer(nIDEvent);
		return;
	case TIMER_IMAGE:
		OnTimerImage();
		return;

	default: break;
	}
	__super::OnTimer(nIDEvent);
}

void CViViewCamera::OnTimerImage()
{
	if (!m_pNet || m_pNet->m_imgVer == m_imgVer)
		return;
	m_imgVer = m_pNet->m_imgVer;

	Align();

	if (m_pNet->IsError())
	{
		CImage img;
		COLORREF *pImg = (COLORREF *)CImagePack::PrepareImage(img, 1, 1, 32);
		pImg[0] = RGB(0, 0, 224);

		CSingleLock imgLock(&m_pNet->m_imgSync, TRUE);
		m_imgWnd.PutImage(img);
		m_statusWnd.SetPos(100.0f);
	}
	else
	{
		CSingleLock imgLock(&m_pNet->m_imgSync, TRUE);
		m_imgWnd.PutImage(m_pNet->m_img);
	}
}

bool CViViewCamera::IsError() 
{
	return !m_pNet || m_pNet->IsError(); 
}

void CViViewCamera::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	Align();
}


void CViViewCamera::Align()
{
	if (!m_hWnd || !m_pNet)
		return;
	CRect rc,rs;
	GetClientRect(&rc);
	rs = rc;

	int dh = GetSystemMetrics(SM_CYICON);
	rc.bottom = rs.top = rc.bottom - dh;

	CSingleLock imgLock(&m_pNet->m_imgSync,TRUE);

	if (!m_pNet->m_img.IsNull())
	{
		int iw = m_pNet->m_img.GetWidth();
		int ih = m_pNet->m_img.GetHeight();

		if (rc.Width()*ih > rc.Height()*iw)
		{
			int n = rc.Height()*iw / ih;
			rc.left = (rc.Width() - n) / 2;
			rc.right = rc.left + n;
		}
		else
		{
			int n = rc.Width()*ih / iw;
			rc.top = (rc.Height() - n) / 2;
			rc.bottom = rc.top + n;
		}
	}

	imgLock.Unlock();

	if (!IsWindow(m_imgWnd))
		m_imgWnd.Create(0, 0, WS_CHILD|WS_VISIBLE, rc, this, 0);
	
	if (!IsWindow(m_statusWnd))
	{
		m_statusWnd.Create(0, WS_CHILD | WS_VISIBLE, rc, this);
		m_statusWnd.SetRange(0, 100);

		m_statusWnd.m_cx.clear();
		m_statusWnd.m_cx[0] = RGB(0, 224, 0);
		m_statusWnd.m_cx[30] = RGB(224, 224, 0);
		m_statusWnd.m_cx[60] = RGB(224, 0, 0);
		m_statusWnd.m_bAbsColor = true;
	}

	if (m_imgWndRect != rc)
	{
		m_imgWnd.MoveWindow(&rc);
		m_imgWndRect = rc;
	}

	if (m_statusWndRect != rs)
	{
		m_statusWnd.MoveWindow(&rs);
		m_statusWndRect = rs;
	}

}

void CViViewCamera::OnNewVar(int id, int subID, VI_VAR& var)
{
	switch (id)
	{
	case VI_VAR_STATE_VAR:
		if(m_pNet && m_pNet->IsError())
			m_statusWnd.SetPos(100.0f);
		else
			m_statusWnd.SetPos(var.fv1*100.0f);
		return;
	case VI_VAR_STATE_CRITICAL:
		if(m_statusCL != var.fv1)
		{
			m_statusWnd.m_cx.clear();
			m_statusWnd.m_cx[0] = RGB(0, 224, 0);
			m_statusWnd.m_cx[var.fv1*50.0f] = RGB(224, 224, 0);
			m_statusWnd.m_cx[var.fv1*100.0f] = RGB(224, 0, 0);
			m_statusCL = var.fv1;
		}
		return;
	default: break;
	}
}

BOOL CViViewCamera::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case (CViDesktopCfg::ID_MSG_RESTART):
		if (m_pNet)
			m_pNet->Restart();
		return TRUE;
	default: break;
	}

	return __super::OnCommand(wParam, lParam);
}


float CViViewCamera::GetState(bool bRelative) 
{ 
	float pos = m_statusWnd.GetPos();
	if(!bRelative)
		return pos; 
	float rel = 100;
	if (!m_statusWnd.m_cx.empty())
		rel = m_statusWnd.m_cx.rbegin()->first;

	if (rel == 0)
		rel = 0.00001f;

	return pos / rel;
}

void CViViewCamera::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	__super::OnActivate(nState, pWndOther, bMinimized);

}


void CViViewCamera::OnLButtonUp(UINT nFlags, CPoint point)
{
	__super::OnLButtonUp(nFlags, point);

	SetFocused();
}
