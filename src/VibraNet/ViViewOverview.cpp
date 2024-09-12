// ViViewNull.cpp : implementation file
//

#include "stdafx.h"
#include "ViDesktop.h"
#include "ViViewOverview.h"
#include "ViViewCamera.h"
#include "ViViewLogLog.h"
#include "ViNet.h"

// CViViewOverview

IMPLEMENT_DYNCREATE(CViViewOverview, CWnd)

CViViewOverview::CViViewOverview() :m_imgVer(0), m_statusVer(0), m_imgWndRect(0, 0, 0, 0)
{
	m_statusCL = 60;
	CSingleLock listLock(&theApp.m_listLock, TRUE);
	theApp.m_listOverview.push_back(this);
}

CViViewOverview::~CViViewOverview()
{
	CSingleLock listLock(&theApp.m_listLock, TRUE);
	theApp.m_listOverview.remove(this);
}


BEGIN_MESSAGE_MAP(CViViewOverview, CWnd)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_SIZE()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()



// CViViewOverview message handlers




void CViViewOverview::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: Add your message handler code here
					   // Do not call CWnd::OnPaint() for painting messages

	CRect rc;
	GetClientRect(&rc);
#ifdef _DEBUG
	dc.FillSolidRect(&rc, 0x7F7FFF);
#else
	dc.FillSolidRect(&rc, 0xFFFFFF);
#endif
}


int CViViewOverview::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (__super::OnCreate(lpCreateStruct) == -1)
		return -1;

	ModifyStyle(0, WS_CLIPCHILDREN | WS_CLIPSIBLINGS);


	SetTimer(TIMER_IMAGE, 5, 0);

	return 0;
}




void CViViewOverview::OnDestroy()
{
	if (IsWindow(m_imgWnd))
		m_imgWnd.DestroyWindow();
	__super::OnDestroy();
}


void CViViewOverview::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent)
	{
	case TIMER_IMAGE:
		OnTimerImage();
		return;

	default: break;
	}
	__super::OnTimer(nIDEvent);
}


bool CViViewOverview::OnImageCam(CViViewCamera *pCam)
{
	if (!pCam)
	{
		return false;
	}


	if (pCam->m_imgVer == m_imgVer && m_statusVer != pCam->m_statusVer)
		return true;

	m_imgVer = pCam->m_imgVer;
	m_statusVer = pCam->m_statusVer;

	if (pCam->m_pNet)
	{
		CSingleLock lock(&pCam->m_pNet->m_imgSync, TRUE);

		Align(pCam->m_pNet->m_img);

		m_imgWnd.PutImage(pCam->m_pNet->m_img);
	}

	m_statusCL = pCam->m_statusCL;
	m_statusWnd = pCam->m_statusWnd;

	return true;
}

bool CViViewOverview::OnImageLog(CViViewLogLog *pLog)
{
	if (!pLog)
	{
		return false;
	}
	CViViewLogLog::LOG log = pLog->GetSelectedLog();
	
	if (log.iid <= 0)
		return false;

	if (log.tw == m_imgVer && m_statusVer != log.tw)
		return true;


	CImage img;
	if (img.Load(log.file) != S_OK)
		return false;

	m_imgVer = log.tw;
	m_statusVer = log.tw;

	Align(img);

	m_imgWnd.PutImage(img);

	m_statusCL = -1;
	m_statusWnd.SetPos(100);

	return true;
}

void CViViewOverview::OnTimerImage()
{
	if (OnImageLog(GetSelectedLog()))
		return;
	OnImageCam(GetSelectedCamera());
}


void CViViewOverview::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

}

void CViViewOverview::Align(CImage& img)
{
	CRect rc, rs;
	GetClientRect(&rc);
	rs = rc;

	int dh = GetSystemMetrics(SM_CYICON);
	rc.bottom = rs.top = rc.bottom - dh;

	if (!img.IsNull())
	{
		int iw = img.GetWidth();
		int ih = img.GetHeight();

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



	if (!IsWindow(m_imgWnd))
		m_imgWnd.Create(0, 0, WS_CHILD | WS_VISIBLE, rc, this, 0);

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





CViViewLogLog *CViViewOverview::GetSelectedLog()
{
	std::list< CViViewLogLog* >::iterator i, ie;
	for (i = theApp.m_listLog.begin(), ie = theApp.m_listLog.end(); i != ie; ++i)
	{
		if ((*i)->IsFocused())
			return *i;
	}
	return NULL;
}

CViViewCamera * CViViewOverview::GetSelectedCamera()
{
	CViViewCamera *pCam = 0;

	std::list< CViViewCamera* >::iterator i, ie;
	for (i = theApp.m_listCamera.begin(), ie = theApp.m_listCamera.end(); i != ie; ++i)
	{
		CViViewCamera *pCur = *i;
		if (pCur->IsFocused())
			return pCur;
		if (pCur->IsError())
			continue;
		if (!pCam) 
			pCam = pCur;
		else
		{
			float stCam = pCam->GetState(true);
			float stCur = pCur->GetState(true);
			if (stCam < stCur)
				pCam = pCur;
		}
	}
	return pCam;
}

void CViViewOverview::OnLButtonUp(UINT nFlags, CPoint point)
{
	__super::OnLButtonUp(nFlags, point);

	SetFocused();
}
