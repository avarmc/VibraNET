#pragma once


#include "ViViewBase.h"
// CViViewCamera

#include "DirectShowWnd.h"
#include "colorprocress.h"

class CVINetModuleDll;
class CVINetServerDoc;
class CViViewOverview;

class CViViewCamera : public CWnd, public CViViewBase
{
	DECLARE_DYNCREATE(CViViewCamera)
protected:

	enum TIMER { T_CHECK = 101 };
	CVINetServerDoc *	m_pNet;

	friend class CViViewOverview;

	int					m_imgVer;
	CDirectShowWnd		m_imgWnd;
	CRect				m_imgWndRect;

	int					m_statusVer;
	CColorProcress		m_statusWnd;
	CRect				m_statusWndRect;
	float				m_statusCL;
public:
	CViViewCamera();
	virtual ~CViViewCamera();
	virtual CWnd* GetWindow() { return dynamic_cast<CWnd*>(this); }
	virtual void OnNewVar(int id, int subID, VI_VAR& var);
protected:
	DECLARE_MESSAGE_MAP()

protected:
	void OnTimerImage();
	void Align();
public:
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	float GetState(bool bRelative = false);
	bool IsError();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
};


