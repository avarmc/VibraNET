#pragma once


#include "ViViewBase.h"
// CViViewOverview

#include "DirectShowWnd.h"
#include "colorprocress.h"

class CVINetModuleDll;
class CVINetServerDoc;
class CViViewCamera;
class CViViewLogLog;

class CViViewOverview : public CWnd, public CViViewBase
{
	DECLARE_DYNCREATE(CViViewOverview)
protected:

	enum TIMER { T_CHECK = 101 };

	int					m_imgVer;
	CDirectShowWnd		m_imgWnd;
	CRect				m_imgWndRect;

	int					m_statusVer;
	CColorProcress		m_statusWnd;
	CRect				m_statusWndRect;
	float				m_statusCL;
public:
	CViViewOverview();
	virtual ~CViViewOverview();
	virtual CWnd* GetWindow() { return dynamic_cast<CWnd*>(this); }

	CViViewCamera *GetSelectedCamera();
	CViViewLogLog *GetSelectedLog();
protected:
	DECLARE_MESSAGE_MAP()

protected:
	void OnTimerImage();
	bool OnImageCam(CViViewCamera *pCam);
	bool OnImageLog(CViViewLogLog *pLog);

	void Align(CImage& img);
public:
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
};


