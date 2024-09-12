// ViViewNull.cpp : implementation file
//

#include "stdafx.h"
#include "ViDesktop.h"
#include "ViViewLogLog.h"


// CViViewLogLog

IMPLEMENT_DYNCREATE(CViViewLogLog, CWnd)

CViViewLogLog::CViViewLogLog():m_list(this)
{
	CSingleLock listLock(&theApp.m_listLock, TRUE);
	theApp.m_listLog.push_back(this);
	m_bFocus = FALSE;
}

CViViewLogLog::~CViViewLogLog()
{
	CSingleLock listLock(&theApp.m_listLock, TRUE);
	theApp.m_listLog.remove(this);
}


BEGIN_MESSAGE_MAP(CViViewLogLog, CWnd)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_TIMER()
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP(CViViewLogLog::CTRL, CListCtrl)
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, &CTRL::OnLvnItemchanged)
END_MESSAGE_MAP()

// CViViewLogLog message handlers




void CViViewLogLog::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: Add your message handler code here
					   // Do not call CWnd::OnPaint() for painting messages

	CRect rc;
	GetClientRect(&rc);
	dc.FillSolidRect(&rc, 0xFFFF00);
}


int CViViewLogLog::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (__super::OnCreate(lpCreateStruct) == -1)
		return -1;
	CSingleLock lock(&m_lock, true);

	CRect r;
	GetClientRect(&r);
	m_list.Create(WS_CHILD | WS_VISIBLE | WS_MAXIMIZE | LVS_SHOWSELALWAYS | LVS_SINGLESEL | LVS_REPORT | LVS_NOSORTHEADER | LVS_NOCOLUMNHEADER//|LVS_OWNERDRAWFIXED
		,
		r, this, 1);


	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_list.InsertColumn(0, _T(""));
	m_list.InsertColumn(1, _T(""));
	m_list.InsertColumn(2, _T(""));

	

	Align(FALSE);

	SetTimer(TIMER_CHECK, 100, 0);

	return 0;
}


void CViViewLogLog::OnDestroy()
{
	__super::OnDestroy();

	Purge(0);

	if (IsWindow(m_list))
		m_list.DestroyWindow();
}


void CViViewLogLog::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	Align();
}

void CViViewLogLog::Align(BOOL bLock)
{
	if (!m_hWnd)
		return;

	CSingleLock lock(&m_lock, bLock);

	CRect rc;
	GetClientRect(&rc);

	if (m_list.m_hWnd)
	{
		m_list.MoveWindow(&rc, FALSE);

		CRect r;
		m_list.GetClientRect(&r);
		int rw = r.Width(),cw;

		cw = rw / 4; rw -= cw;
		m_list.SetColumnWidth(0,  cw);

		cw = rw / 3; rw -= cw;
		m_list.SetColumnWidth(1, cw);

		m_list.SetColumnWidth(2, rw);
	}
}

void CViViewLogLog::AddLog(int iid, const CString& cam, const CString& text, const CString& file, bool bReplace)
{
	LOG log;
	log.bReplace = bReplace;
	log.iid = iid;
	log.t = COleDateTime::GetCurrentTime();
	log.cam = cam;
	log.file = file;
	log.text = text;
	log.tw = GetTickCount();

	CSingleLock lock(&m_lock, TRUE);

	m_log.push_back(log);
	while (m_log.size() > 1000)
		m_log.pop_front();
}

void CViViewLogLog::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent)
	{
	case TIMER_CHECK:
		OnTimerCheck();
		return;
	default: break;
	}

	__super::OnTimer(nIDEvent);
}


void CViViewLogLog::OnTimerCheck()
{
	CSingleLock lock(&m_lock, TRUE);
	if (m_log.empty())
		return;
	while (!m_log.empty())
	{
		AddLog(m_log.front());
		m_log.pop_front();
	}
	Purge(1000, FALSE);
}

void CViViewLogLog::Purge(int cnt, BOOL bLock)
{
	if (!m_list.m_hWnd)
		return;
	CSingleLock lock(&m_lock, bLock);
	int i;
	while ((i = m_list.GetItemCount()-1) >= cnt)
	{
		LOG *pLog = (LOG*)m_list.GetItemData(i);
		SAFE_DELETE(pLog);
		m_list.DeleteItem(i);
	}
}

bool CViViewLogLog::IsFocused()
{
	return m_bFocus;
}

void CViViewLogLog::AddLog(LOG& log)
{
	if (!m_list.m_hWnd)
		return;

	CSingleLock lock(&m_lock, TRUE);

	bool bFocus = IsFocused();
	
	int i;
	LOG *pLog = 0;
	
	if (log.bReplace)
	{
		int cnt = m_list.GetItemCount();
		for (int ii = 0; ii < cnt; ++ii)
		{
			LOG *pi = (LOG *)m_list.GetItemData(ii);
			if (pi->iid == log.iid)
			{
				pLog = pi;
				i = ii;

				break;
			}
		}
	}


	if (!pLog)
	{
		pLog = new LOG(log);
		i = m_list.InsertItem(0, log.t.Format(), 0);
		m_list.SetItemData(i, (DWORD_PTR)pLog);
	}
	else
	{
		*pLog = log;
		m_list.SetItemText(i, 0, log.t.Format());
	}

	m_list.SetItemText(i, 1, log.cam);
	m_list.SetItemText(i, 2, log.text);

	int nCount = m_list.GetItemCount();
	if (nCount > 0)
		m_list.EnsureVisible(nCount - 1, FALSE);

	
	if (!bFocus || !m_list.GetSelectedCount())
		m_list.EnsureVisible(i, FALSE);
	else
	{
		POSITION pos = m_list.GetFirstSelectedItemPosition();
		if (pos)
		{
			int ii = m_list.GetNextSelectedItem(pos);
			m_list.EnsureVisible(ii, FALSE);
		} 
		
	}
	

}


CViViewLogLog::LOG CViViewLogLog::GetSelectedLog()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	if (pos)
	{
		int ii = m_list.GetNextSelectedItem(pos);
		return *(LOG*)m_list.GetItemData(ii);
	}

	LOG log;
	log.iid = -1;
	return log;
}

void CViViewLogLog::CTRL::OnSetFocus(CWnd* pOldWnd)
{
	__super::OnSetFocus(pOldWnd);

	pBase->SetFocused();
}


void CViViewLogLog::CTRL::OnKillFocus(CWnd* pNewWnd)
{
	__super::OnKillFocus(pNewWnd);
}


void CViViewLogLog::CTRL::OnLvnItemchanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	if(pNMLV->uNewState != pNMLV->uOldState)
		pBase->SetFocused();
	*pResult = 0;
}


bool CViViewLogLog::SelectLog(int iid, const CString& file)
{
	CSingleLock lock(&m_lock, TRUE);
	int cnt = m_list.GetItemCount();
	for (int ii = 0; ii < cnt; ++ii)
	{
		LOG *pi = (LOG *)m_list.GetItemData(ii);
		if (pi->iid == iid && pi->file == file)
		{
			Deselect();
			m_list.SetItemState(ii, LVIS_SELECTED, LVIS_SELECTED | LVIS_FOCUSED);
			SetFocused();
			return true;
		}
	}
	return false;
}

void CViViewLogLog::Deselect()
{
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	while (pos)
	{
		int ii = m_list.GetNextSelectedItem(pos);
		m_list.SetItemState(ii, 0, LVIS_SELECTED | LVIS_FOCUSED);
	}
}
