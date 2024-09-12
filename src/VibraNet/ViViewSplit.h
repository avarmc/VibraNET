#pragma once

#include "ViViewBase.h"
// CViViewSplit

class CViViewSplit : public CSplitterWnd, public CViViewBase
{
	DECLARE_DYNCREATE(CViViewSplit)

	BOOL m_bSplitterCreated;
public:
	CViViewSplit();
	virtual ~CViViewSplit();
	virtual CWnd* GetWindow() { return dynamic_cast<CWnd*>(this); }

	BOOL Create(CWnd* pParentWnd,xml_node& xml, int iid);

	void Align(int cx, int cy);

	virtual void RecalcLayout();
	virtual void TrackRowSize(int y, int row);
	virtual void TrackColumnSize(int x, int col);
	void ExportSizes();
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
};


