#pragma once

#include "ViViewBase.h"
// CViViewNull

class CViViewNull : public CWnd, public CViViewBase
{
	DECLARE_DYNCREATE(CViViewNull)

public:
	CViViewNull();
	virtual ~CViViewNull();
	virtual CWnd* GetWindow() { return dynamic_cast<CWnd*>(this); }

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};


