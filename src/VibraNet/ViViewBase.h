#pragma once

#include "ViDesktopCfg.h"

class CViViewBase
{
public:
	VI_NET_ITEM *pItem;
public:
	CViViewBase();
	~CViViewBase();

	virtual CWnd* GetWindow() = 0;

	bool	m_bFocus;
	void	SetFocused();
	bool IsFocused() {	return m_bFocus; }
};

