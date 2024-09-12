#include "stdafx.h"
#include "ViViewBase.h"
#include "ViDesktop.h"
#include "ViViewLogLog.h"
#include "ViViewLogImg.h"
#include "ViViewLogLog.h"
#include "ViViewCamera.h"
#include "ViViewOverview.h"

CViViewBase::CViViewBase()
{
	pItem = 0;
}


CViViewBase::~CViViewBase()
{
}

void	CViViewBase::SetFocused()
{
	for (std::list< CViViewCamera* >::iterator i = theApp.m_listCamera.begin(), ie = theApp.m_listCamera.end(); i != ie; ++i)
		(*i)->m_bFocus = false;
	for (std::list< CViViewLogLog* >::iterator i = theApp.m_listLog.begin(), ie = theApp.m_listLog.end(); i != ie; ++i)
		(*i)->m_bFocus = false;
	for (std::list< CViViewOverview* >::iterator i = theApp.m_listOverview.begin(), ie = theApp.m_listOverview.end(); i != ie; ++i)
		(*i)->m_bFocus = false;

	m_bFocus = true;
}