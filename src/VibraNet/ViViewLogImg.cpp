// ViViewNull.cpp : implementation file
//

#include "stdafx.h"
#include "ViDesktop.h"
#include "ViViewLogImg.h"
#include "ViViewLogLog.h"

// CViViewLogImg

IMPLEMENT_DYNCREATE(CViViewLogImg, CWnd)

CViViewLogImg::CViViewLogImg()
{
	CSingleLock listLock(&theApp.m_listLock, TRUE);
	theApp.m_listImg.push_back(this);
	m_logVer2 = m_logVer = GetTickCount() - 100000;
}

CViViewLogImg::~CViViewLogImg()
{
	CSingleLock listLock(&theApp.m_listLock, TRUE);
	theApp.m_listImg.remove(this);
	listLock.Unlock();

	CSingleLock lock(&m_logLock, TRUE);
	while (m_log.size() > 0)
	{
		LOG* l = m_log.back();
		SAFE_DELETE(l);
		m_log.pop_back();
	}
}


BEGIN_MESSAGE_MAP(CViViewLogImg, CWnd)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_TIMER()
	ON_WM_SIZE()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()



// CViViewLogImg message handlers




void CViViewLogImg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: Add your message handler code here
					   // Do not call CWnd::OnPaint() for painting messages


	CRect rc;
	GetClientRect(&rc);
	CRgn rgn,rgn1;
	rgn.CreateRectRgnIndirect(&rc);
	rgn1.CreateRectRgnIndirect(&rc);
	rgn1.CombineRgn(&rgn, &rgn, RGN_XOR);

	dc.SetStretchBltMode(COLORONCOLOR);


	int N = 0;
	if (pItem)
		N = atoi(pItem->xml["set"].attribute_value("count"));
	if (!N)
	{
		return;
	}

	CSize s, step(0, 0);

	if (rc.Width() * 48 > rc.Height() * 64)
	{
		s = CSize(rc.Width() / N - 5, rc.Height());
		step.cx = s.cx+5;
	}
	else
	{
		s = CSize(rc.Width(), rc.Height() / N - 5);
		step.cy = s.cy+5;
	}

	CPoint p0 = rc.TopLeft();

	CSingleLock lock(&m_logLock, TRUE);
	for (std::list<LOG*>::iterator i = m_log.begin(), ie = m_log.end(); i != ie; ++i)
	{
		CRect r(p0, s);

		CImage& img = (*i)->img;
		if (!img.IsNull())
		{
			int iw = img.GetWidth();
			int ih = img.GetHeight();

			if (r.Width() *ih > r.Height() *iw)
			{
				int n = r.Height()*iw / ih;
				r.left += (r.Width() - n) / 2;
				r.right = r.left + n;
			}
			else
			{
				int n = r.Width()*ih / iw;
				r.top += (r.Height() - n) / 2;
				r.bottom = r.top + n;
			}

			CRgn rg;
			rg.CreateRectRgnIndirect(&r);
			rgn1.CombineRgn(&rgn1, &rg, RGN_OR);

			img.Draw(dc, r);
			(*i)->r = r;

		}
		p0.x += step.cx;
		p0.y += step.cy;
	}

	rgn.CombineRgn(&rgn, &rgn1, RGN_XOR);
	dc.SelectClipRgn(&rgn);

	dc.FillSolidRect(&rc, 0xFFFFFF);

	dc.SelectClipRgn(0);
}


void  CViViewLogImg::AddLog(int iid, const CString& cam, const CString& text, const CString& file, bool bReplace)
{
	int N = 0;
	if (pItem)
		N = atoi(pItem->xml["set"].attribute_value("count"));
	if (!N)
		return;

	LOG *pLog = 0;
	bool bPush = false;

	CSingleLock lock(&m_logLock, TRUE);

	if (bReplace)
	{
		for (std::list<LOG*>::iterator i = m_log.begin(), ie = m_log.end(); i != ie; ++i)
		{
			if ((*i)->iid == iid)
			{
				pLog = *i;
				break;
			}
		}
	}

	if (!pLog)
	{
		pLog = new LOG();
		bPush = true;
	}
	pLog->iid = iid;
	pLog->t = COleDateTime::GetCurrentTime();
	pLog->cam = cam;
	pLog->file = file;
	pLog->text = text;
	pLog->tw = GetTickCount();
	if (!pLog->img.IsNull())
		pLog->img.Destroy();
	pLog->img.Load(file);

	if(bPush) m_log.push_front(pLog);
	m_logVer = GetTickCount();


}

int CViViewLogImg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (__super::OnCreate(lpCreateStruct) == -1)
		return -1;

	SetTimer(TIMER_CHECK, 40, 0);

	return 0;
}


void CViViewLogImg::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent)
	{
	case TIMER_CHECK:
		if (m_logVer != m_logVer2)
		{
			RedrawWindow(0, 0);
			m_logVer2 = m_logVer;
		}
		Purge();
		return;
	default: break;
	}

	__super::OnTimer(nIDEvent);
}


void  CViViewLogImg::Purge()
{
	CSingleLock lock(&m_logLock, TRUE);

	int N = 0;
	if (pItem)
		N = atoi(pItem->xml["set"].attribute_value("count"));

	while (m_log.size() > N)
	{
		LOG* l = m_log.back();
		SAFE_DELETE(l);
		m_log.pop_back();
	}
}

void CViViewLogImg::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
}


void CViViewLogImg::OnLButtonUp(UINT nFlags, CPoint point)
{
	SetFocused();

	CSingleLock lock(&m_logLock, TRUE);
	for (std::list<LOG*>::iterator i = m_log.begin(), ie = m_log.end(); i != ie; ++i)
	{
		if ((*i)->r.PtInRect(point))
		{
			SelectLog(*i);
			break;
		}
	}

	__super::OnLButtonUp(nFlags, point);
}


bool CViViewLogImg::SelectLog(LOG* pLog)
{
	std::list< CViViewLogLog* >::iterator i, ie;
	for (i = theApp.m_listLog.begin(), ie = theApp.m_listLog.end(); i != ie; ++i)
	{
		if ((*i)->SelectLog(pLog->iid, pLog->file))
			return true;
	}
	return false;
}

