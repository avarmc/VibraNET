#pragma once

#include <ddraw.h>
// ddraw.lib dxguid.lib

#include "DirectShowWndData.h"

// CDirectShowWnd

class  CDirectShowWnd : public CWnd
{
//	DECLARE_DYNAMIC(CDirectShowWnd)
	DECLARE_DYNCREATE(CDirectShowWnd)
public:
	CDirectShowWnd();
	virtual ~CDirectShowWnd();

protected:
	DECLARE_MESSAGE_MAP()
public:
	CRect				m_dislpayRC;
	int					m_dislpayBPP;
	CSize				m_imgSize;
	int					m_imgBpp;

	CCriticalSection		m_lock;

	bool				m_bCompatMode;
	CImage				m_imgCompat;

protected:
	LPDIRECTDRAW			m_lpDDraw1;
	LPDIRECTDRAW7			m_lpDDraw7;
	LPDIRECTDRAWSURFACE7	m_lpPrimarySurface;
	LPDIRECTDRAWCLIPPER		m_lpClipper;
	LPDIRECTDRAWSURFACE7	m_lpBackSurface;
protected:
	bool DDrawDetectDisplayMode();
	bool DDrawCreateFlippingSurface(void);
	bool DDrawInit(void);
	bool DDrawDone(void);
	bool DDrawDraw(bool bLock=true);
	bool DDrawSetSize(int w,int h,int bpp);
public:
	HDC		GetDC();
	void	ReleaseDC(HDC hDC);
	HDC		GetBkDC();
	void	ReleaseBkDC(HDC hDC);

	bool	Redraw();
	bool	PutImage(void *pImg, int w, int h, int bpp, int pitch,bool bRedraw=true);
	bool	PutImage(CImage& img,bool bRedraw=true);

	bool	IsImage();

	bool	ClearImg();

	bool	Lock(CDirectShowWndLockPtr *pLock);
	bool	Unlock(CDirectShowWndLockPtr *pLock,bool bRedraw=false);

	bool	ExportImg(CImage& img);

	bool	LP2DP(RECT *p);
	bool	LP2DP(long &x,long &y);
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
public:
	virtual BOOL Create(LPCWSTR lpszClassName,LPCWSTR lpszWindowName,DWORD dwStyle,const RECT& rect,CWnd* pParentWnd,UINT nID,CCreateContext* pContext = NULL);

protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnPaint();
};


inline bool	CDirectShowWnd::IsImage()
{
	if(!m_lpBackSurface)
		return false;
	if(!m_imgSize.cx || !m_imgSize.cy || ! m_imgBpp)
		return false;
	return true;
}

inline bool CDirectShowWnd::Redraw()
{
	return DDrawDraw();
}
