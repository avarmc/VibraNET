#pragma once

#include "ViViewBase.h"
// CViViewLogImg

class CViViewLogImg : public CWnd, public CViViewBase
{
	DECLARE_DYNCREATE(CViViewLogImg)

public:	
	typedef struct tagLOG
	{
		int iid;
		DWORD tw;
		COleDateTime t;
		CImage img;
		CString file,cam,text;
		CRect r;
	} LOG;

	CCriticalSection	m_logLock;
	std::list<LOG*>		m_log;
	DWORD				m_logVer, m_logVer2;
public:
	CViViewLogImg();
	virtual ~CViViewLogImg();
	virtual CWnd* GetWindow() { return dynamic_cast<CWnd*>(this); }

	void AddLog(int iid, const CString& cam, const CString& text, const CString& file, bool bReplace);

	void Purge();
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

	bool SelectLog(LOG*pLog);
};


