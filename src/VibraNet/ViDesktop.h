
// ViDesktop.h : main header file for the ViDesktop application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols
#include "ViNetCtrl.h"
#include "Cmdl.h"

class CViDesktopDoc;
class CViDesktopFrame;
class CViDesktopView;

class CViViewCamera;
class CViViewSplit;
class CViViewLogLog;
class CViViewLogImg;
class CViViewOverview;

class CVINetServerDoc;

// CViDesktopApp:
// See ViDesktop.cpp for the implementation of this class
//

enum VI_TIMER { TIMER_CHECK = 1,TIMER_IMAGE,TIMER_INIT};

class CViDesktopApp : public CWinAppEx
{
public:
	CViDesktopApp();

protected:
	DECLARE_MESSAGE_MAP()

// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
public:
	CCmdl		m_cmdl;
	CString		m_regString;

// Implementation
	CViDesktopDoc *		m_pDoc;
	CViDesktopFrame *	m_pFrame;
	CViDesktopView *	m_pView;
	CVINetServerDoc *	m_pNet;

	CCriticalSection				m_listLock;
	std::list< CViViewCamera* >		m_listCamera;
	std::list< CViViewSplit* >		m_listSplit;
	std::list< CViViewLogLog* >		m_listLog;
	std::list< CViViewLogImg* >		m_listImg;
	std::list< CViViewOverview* >	m_listOverview;


	BOOL		m_bPowerOff, m_bDone;
	UINT		m_nAppLook;
	BOOL		m_bHiColorIcons;


	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	
	void AddLog(int iid, const CString& cam, const CString& text, const CString& file, bool bReplace);
};

extern CViDesktopApp theApp;
