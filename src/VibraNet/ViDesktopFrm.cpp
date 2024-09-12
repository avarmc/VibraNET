
// ViDesktopFrm.cpp : implementation of the CViDesktopFrame class
//

#include "stdafx.h"
#include "ViDesktop.h"

#include "ViDesktopFrm.h"
#include "resource.h"
#include "ViNet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CViDesktopFrame

IMPLEMENT_DYNCREATE(CViDesktopFrame, CFrameWndEx)

BEGIN_MESSAGE_MAP(CViDesktopFrame, CFrameWndEx)
	ON_WM_CREATE()
	ON_COMMAND(ID_VIEW_CUSTOMIZE, &CViDesktopFrame::OnViewCustomize)
	ON_REGISTERED_MESSAGE(AFX_WM_CREATETOOLBAR, &CViDesktopFrame::OnToolbarCreateNew)
	ON_COMMAND_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7, &CViDesktopFrame::OnApplicationLook)
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7, &CViDesktopFrame::OnUpdateApplicationLook)
	ON_WM_TIMER()
END_MESSAGE_MAP()

// CViDesktopFrame construction/destruction

CViDesktopFrame::CViDesktopFrame()
{
	theApp.m_pFrame = this;
	theApp.m_nAppLook = theApp.GetInt(_T("ApplicationLook"), ID_VIEW_APPLOOK_VS_2008);
}

CViDesktopFrame::~CViDesktopFrame()
{
	if (theApp.m_pFrame == this)
		theApp.m_pFrame = NULL;
}

int CViDesktopFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{

	if (CFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;


	if (!m_wndMenuBar.Create(this))
	{
		TRACE0("Failed to create menubar\n");
		return -1;      // fail to create
	}

	m_wndMenuBar.SetPaneStyle(m_wndMenuBar.GetPaneStyle() | CBRS_SIZE_DYNAMIC | CBRS_TOOLTIPS | CBRS_FLYBY);

	// prevent the menu bar from taking the focus on activation
	CMFCPopupMenu::SetForceMenuFocus(FALSE);


	if (!m_wndCfg.Create(_T("Cfg"), this, CRect(0, 0, 200, 200), TRUE, ID_VIEW_CONFIG, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create Properties window\n");
		return FALSE; // failed to create
	}


	// TODO: Delete these five lines if you don't want the toolbar and menubar to be dockable
	m_wndMenuBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndMenuBar);


	// enable Visual Studio 2005 style docking window behavior
	CDockingManager::SetDockingMode(DT_SMART);
	// enable Visual Studio 2005 style docking window auto-hide behavior
	EnableAutoHidePanes(CBRS_ALIGN_ANY);
	// set the visual manager and style based on persisted value
	OnApplicationLook(theApp.m_nAppLook);


	// enable quick (Alt+drag) toolbar customization
	// CMFCToolBar::EnableQuickCustomization();

	m_wndCfg.EnableDocking(CBRS_ALIGN_ANY);


	DockPane(&m_wndCfg);

	if (!m_wndCfg.IsAutoHideMode())
	{
		ShowPane(&m_wndCfg, TRUE, TRUE, FALSE);

		CPaneDivider* pDefaultSlider = m_wndCfg.GetDefaultPaneDivider();
		if (pDefaultSlider)
			m_wndCfg.SetAutoHideMode(TRUE, pDefaultSlider->GetCurrentAlignment());
		else
			m_wndCfg.SetAutoHideMode(TRUE, CBRS_LEFT);
	}

	SetTimer(TIMER_CHECK, 100, 0);
	// ShowWindow(SW_SHOWMAXIMIZED);
	return 0;
}

BOOL CViDesktopFrame::OnCreateClient(LPCREATESTRUCT lpcs,
	CCreateContext* pContext)
{
	return CFrameWndEx::OnCreateClient(lpcs,pContext);
}

BOOL CViDesktopFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWndEx::PreCreateWindow(cs) )
		return FALSE;
	
//	cs.style = WS_POPUP|WS_THICKFRAME|WS_VISIBLE;
//	cs.style &= (~FWS_ADDTOTITLE);
	return TRUE;
}

// CViDesktopFrame diagnostics

#ifdef _DEBUG
void CViDesktopFrame::AssertValid() const
{
	CFrameWndEx::AssertValid();
}

void CViDesktopFrame::Dump(CDumpContext& dc) const
{
	CFrameWndEx::Dump(dc);
}
#endif //_DEBUG


// CViDesktopFrame message handlers

void CViDesktopFrame::OnViewCustomize()
{
	CMFCToolBarsCustomizeDialog* pDlgCust = new CMFCToolBarsCustomizeDialog(this, TRUE /* scan menus */);
	pDlgCust->Create();
}

LRESULT CViDesktopFrame::OnToolbarCreateNew(WPARAM wp,LPARAM lp)
{
	LRESULT lres = CFrameWndEx::OnToolbarCreateNew(wp,lp);
	if (lres == 0)
	{
		return 0;
	}

	CMFCToolBar* pUserToolbar = (CMFCToolBar*)lres;
	ASSERT_VALID(pUserToolbar);

	BOOL bNameValid;
	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);

	pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
	return lres;
}

void CViDesktopFrame::OnApplicationLook(UINT id)
{
	CWaitCursor wait;

	theApp.m_nAppLook = id;

	switch (theApp.m_nAppLook)
	{
	case ID_VIEW_APPLOOK_WIN_2000:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManager));
		break;

	case ID_VIEW_APPLOOK_OFF_XP:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOfficeXP));
		break;

	case ID_VIEW_APPLOOK_WIN_XP:
		CMFCVisualManagerWindows::m_b3DTabsXPTheme = TRUE;
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
		break;

	case ID_VIEW_APPLOOK_OFF_2003:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2003));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	case ID_VIEW_APPLOOK_VS_2005:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2005));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	case ID_VIEW_APPLOOK_VS_2008:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2008));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	case ID_VIEW_APPLOOK_WINDOWS_7:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows7));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	default:
		switch (theApp.m_nAppLook)
		{
		case ID_VIEW_APPLOOK_OFF_2007_BLUE:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_LunaBlue);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_BLACK:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_SILVER:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Silver);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_AQUA:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Aqua);
			break;
		}

		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
		CDockingManager::SetDockingMode(DT_SMART);
	}

	RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ERASE);

	theApp.WriteInt(_T("ApplicationLook"), theApp.m_nAppLook);
}

void CViDesktopFrame::OnUpdateApplicationLook(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(theApp.m_nAppLook == pCmdUI->m_nID);
}



BOOL CViDesktopFrame::OnQueryEndSession()
{
	theApp.m_bPowerOff = true;

	if (!CFrameWndEx::OnQueryEndSession())
		return FALSE;

	SendMessage(WM_CLOSE, 0, 0);

	return TRUE;
}


void CViDesktopFrame::OnTimer(UINT_PTR nIDEvent)
{
	if (theApp.m_pNet && theApp.m_pNet->OnTimer((int)nIDEvent))
		return;

	CFrameWndEx::OnTimer(nIDEvent);
}
