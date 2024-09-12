#pragma once

#include "ViViewBase.h"
// CViViewLogLog

class CViViewLogLog : public CWnd, public CViViewBase
{
	DECLARE_DYNCREATE(CViViewLogLog)
public:
	CCriticalSection			m_lock;
	class CTRL : public CListCtrl
	{
		
	protected:
		DECLARE_MESSAGE_MAP()
	public:
		CViViewLogLog *pBase;
		CTRL(CViViewLogLog *base):pBase(base) {};
		virtual ~CTRL() {}

		afx_msg void OnSetFocus(CWnd* pOldWnd);
		afx_msg void OnKillFocus(CWnd* pNewWnd);
		afx_msg void OnLvnItemchanged(NMHDR *pNMHDR, LRESULT *pResult);
	};
	CTRL			m_list;

	typedef struct tagLOG
	{
		int iid;
		COleDateTime t;
		CString cam;
		CString text;
		CString file;
		bool	bReplace;
		DWORD tw;
	} LOG;

	std::list<LOG>			m_log;

public:
	CViViewLogLog();
	virtual ~CViViewLogLog();
	virtual CWnd* GetWindow() { return dynamic_cast<CWnd*>(this); }

	void AddLog(int iid, const CString& cam, const CString& text, const CString& file, bool bReplace);
	bool IsFocused();
protected:
	void AddLog(LOG& log);
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();

	void Align(BOOL bLock = TRUE);
	afx_msg void OnSize(UINT nType, int cx, int cy);


	afx_msg void OnTimer(UINT_PTR nIDEvent);
	void OnTimerCheck();
	void Purge(int cnt, BOOL bLock = TRUE);

	LOG GetSelectedLog();

	bool SelectLog(int iid, const CString& file);
	void Deselect();
};


