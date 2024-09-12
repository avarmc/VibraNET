
// ViDesktopFrm.h : interface of the CViDesktopFrame class
//

#pragma once

#include "ViDesktopCfg.h"

class CViDesktopFrame : public CFrameWndEx
{
	
protected: // create from serialization only
	CViDesktopFrame();
	DECLARE_DYNCREATE(CViDesktopFrame)

// Attributes
public:
	CViDesktopCfg	m_wndCfg;
public:

	
// Operations
public:

// Overrides
public:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CViDesktopFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	afx_msg BOOL OnQueryEndSession();

protected:  // control bar embedded members
	CMFCMenuBar       m_wndMenuBar;

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnViewCustomize();
	afx_msg LRESULT OnToolbarCreateNew(WPARAM wp, LPARAM lp);
	afx_msg void OnApplicationLook(UINT id);
	afx_msg void OnUpdateApplicationLook(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};


