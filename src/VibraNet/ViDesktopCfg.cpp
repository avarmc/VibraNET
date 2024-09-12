// ViDesktopCfg.cpp : implementation file
//

#include "stdafx.h"
#include "ViDesktop.h"
#include "ViDesktopCfg.h"
#include "ViDesktopFrm.h"
#include "ViDesktopDoc.h"
#include "ViDesktopView.h"
#include "inc.h"
#include "ViViewNull.h"
#include "ViViewSplit.h"
#include "ViViewCamera.h"
#include "ViViewLogLog.h"
#include "ViViewLogImg.h"
#include "ViViewOverview.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar

CViDesktopCfg::CViDesktopCfg():m_wndPropList(this)
{
	m_nID = 50000;
	m_nBusy = 0;
}

CViDesktopCfg::~CViDesktopCfg()
{
	Clear();
}

BEGIN_MESSAGE_MAP(CViDesktopCfg, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_SETTINGCHANGE()
	ON_REGISTERED_MESSAGE(AFX_WM_PROPERTY_CHANGED, OnPropertyChanged)
	ON_WM_TIMER()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar message handlers

void CViDesktopCfg::AdjustLayout()
{
	if (GetSafeHwnd() == NULL || (AfxGetMainWnd() != NULL && AfxGetMainWnd()->IsIconic()))
	{
		return;
	}

	if (m_nBusy)
	{
		KillTimer(T_LAYOUT);
		SetTimer(T_LAYOUT, 10, 0);
		return;
	}

	CRect rectClient,rectView (0,0,100,100);
	GetClientRect(rectClient);

	

	int cyTlb = 0;

	m_wndPropList.SetWindowPos(NULL, rectClient.left, rectClient.top + cyTlb, rectClient.Width(), rectClient.Height() - (cyTlb), SWP_NOACTIVATE | SWP_NOZORDER);

	if (theApp.m_pView && IsWindow(theApp.m_pView->m_hWnd))
	{
		CSingleLock lock(&theApp.m_pDoc->m_xmlLock, TRUE);
		theApp.m_pView->GetClientRect(&rectView);

		for (std::map<int, VI_NET_ITEM>::iterator i = m_items.begin(), ie = m_items.end(); i != ie; ++i)
		{
			VI_NET_ITEM &ic = i->second;
			if (ic.pParent != NULL || !ic.pWnd || ! IsWindow(ic.pWnd->GetWindow()->m_hWnd))
				continue;
			
			CWnd *pWnd = ic.pWnd->GetWindow();

			ic.pParentWnd = theApp.m_pView;
			pWnd->SetParent(theApp.m_pView);
			pWnd->MoveWindow(&rectView);

			if (!pWnd->IsWindowVisible())
				pWnd->ShowWindow(SW_SHOWMAXIMIZED);

		}
	}

}

int CViDesktopCfg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();


	if (!m_wndPropList.Create(WS_VISIBLE | WS_CHILD, rectDummy, this, 2))
	{
		TRACE0("Failed to create Properties Grid \n");
		return -1;      // fail to create
	}

	InitPropList();

//	Reload();

//	AdjustLayout();

	return 0;
}

void CViDesktopCfg::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CViDesktopCfg::InitPropList()
{
	SetPropListFont();

	m_wndPropList.EnableHeaderCtrl(FALSE);
	m_wndPropList.EnableDescriptionArea();
	m_wndPropList.SetVSDotNetLook();
	m_wndPropList.MarkModifiedProperties();

}

void CViDesktopCfg::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
	m_wndPropList.SetFocus();
}

void CViDesktopCfg::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CDockablePane::OnSettingChange(uFlags, lpszSection);
	SetPropListFont();
}

void CViDesktopCfg::SetPropListFont()
{
	::DeleteObject(m_fntPropList.Detach());

	LOGFONT lf;
	afxGlobalData.fontRegular.GetLogFont(&lf);

	NONCLIENTMETRICS info;
	info.cbSize = sizeof(info);

	afxGlobalData.GetNonClientMetrics(info);

	lf.lfHeight = info.lfMenuFont.lfHeight;
	lf.lfWeight = info.lfMenuFont.lfWeight;
	lf.lfItalic = info.lfMenuFont.lfItalic;

	m_fntPropList.CreateFontIndirect(&lf);

	m_wndPropList.SetFont(&m_fntPropList);
}




bool CViDesktopCfg::Reload(const CString& path, BOOL bLock)
{
	CSingleLock lock(&theApp.m_pDoc->m_xmlLock, bLock);
	CInc busy(&m_nBusy);

	Clear(FALSE);

	if (theApp.m_pDoc->m_xml.first_child())
		theApp.m_pDoc->m_xml.remove_child(theApp.m_pDoc->m_xml.first_child());

	if( !theApp.m_pDoc->m_xml.load_file(path)	)
		return false;


	if (FillList(FALSE))
	{
		AdjustLayout();
		return true;
	}
	return false;
}

bool CViDesktopCfg::ReloadString(const char* str, BOOL bLock)
{
	CSingleLock lock(&theApp.m_pDoc->m_xmlLock, bLock);
	CInc busy(&m_nBusy);

	Clear(FALSE);

	if (theApp.m_pDoc->m_xml.first_child())
		theApp.m_pDoc->m_xml.remove_child(theApp.m_pDoc->m_xml.first_child());

	if (!theApp.m_pDoc->m_xml.load(str))
		return false;


	if (FillList(FALSE))
	{
		AdjustLayout();
		return true;
	}
	return false;
}

void CViDesktopCfg::Clear(BOOL bLock)
{
	CSingleLock lock(&theApp.m_pDoc->m_xmlLock, bLock);
	CInc busy(&m_nBusy);

	while (!m_items.empty())
		Kill(m_items.begin()->first, FALSE);

	
	while (!theApp.m_listCamera.empty())
		delete dynamic_cast<CViViewCamera*>(theApp.m_listCamera.back());

	while (!theApp.m_listSplit.empty())
		delete dynamic_cast<CViViewSplit*>(theApp.m_listSplit.back());

	while (!theApp.m_listLog.empty())
		delete dynamic_cast<CViViewLogLog*>(theApp.m_listLog.back());

	while (!theApp.m_listImg.empty())
		delete dynamic_cast<CViViewLogImg*>(theApp.m_listImg.back());

	if (IsWindow(m_hWnd) && IsWindow(m_wndPropList.m_hWnd))
		m_wndPropList.RemoveAll();
	
	m_items.clear();

	m_nID = 50000;

}

bool   CViDesktopCfg::FillList(BOOL bLock)
{
	CSingleLock lock(&theApp.m_pDoc->m_xmlLock, bLock);
	CInc busy(&m_nBusy);

	Clear(FALSE);

	xml_node xml = theApp.m_pDoc->m_xml.first_child();
	if (!xml)
		return false;

	
	return FillList(xml,0,FALSE);
}

bool CViDesktopCfg::FillList(xml_node &xml, VI_NET_ITEM *pGroup, BOOL bLock)
{
	CSingleLock lock(&theApp.m_pDoc->m_xmlLock, bLock);
	CInc busy(&m_nBusy);

	bool bSubItems = false;
	int iid = -1;
	VI_NET_ITEM* pItem = NULL;

	CStringA name = xml.name();
	CStringA type = xml.attribute_value("type");

	if (name == "config")
	{
		bSubItems = true;
	} else
	if (name == "item" && type != "" )
	{
		pItem = &NewItem();

		pItem->xml = xml;
		pItem->pItem = NULL;
		pItem->pParent = pGroup;
		pItem->type = T_EMPTY;

		bSubItems = FillItem(pItem);
	}

	if (bSubItems)
	{
		int pos = 0;
		for (xml_node_iterator i = xml.begin(), ie = xml.end(); i != ie; ++i,++pos)
		{
			FillList(*i, pItem, FALSE);
		}
	}

	if (pItem && pItem->pItem)
	{
		pItem->pItem->SetData(pItem->iid);
		if (pGroup)
			pGroup->pItem->AddSubItem(pItem->pItem);
		else
			m_wndPropList.AddProperty(pItem->pItem);
	}

	return true;
}

bool  CViDesktopCfg::FillItem(VI_NET_ITEM *pItem,bool bAdd)
{
	bool bSubItems = false;
	CInc busy(&m_nBusy);

	CStringA name = pItem->xml ? pItem->xml.name() : "";
	CStringA type = pItem->xml ? pItem->xml.attribute_value("type") : "";
	int iid = pItem->iid;

	if (type == "split")
	{
		pItem->type = T_SPLIT;
		if (!pItem->pItem)
		{
			pItem->pItem = new CMFCPropertyGridProperty(CString(type));
			if (bAdd && pItem->pParent && pItem->pParent->pItem)
				pItem->pParent->pItem->AddSubItem(pItem->pItem);
		}
		else
			pItem->pItem->SetName(CString(type));

		FillWindow(pItem);
	//	FillSplit(pItem);
		pItem->pItem->Expand(CStringA(pItem->xml["set"].attribute_value("expanded")) != "false");

		bSubItems = true;
	}
	else
	if (type == "camera" || type == "text_log" || type == "image_log")
	{
		pItem->type = GetType(type);
		if (!pItem->pItem)
		{
			pItem->pItem = new CMFCPropertyGridProperty(CString(type));
			if (bAdd && pItem->pParent && pItem->pParent->pItem)
				pItem->pParent->pItem->AddSubItem(pItem->pItem);
		}
		else
			pItem->pItem->SetName(CString(type));

		FillWindow(pItem);
		
		pItem->pItem->Expand(CStringA(pItem->xml["set"].attribute_value("expanded")) == "true");

		bSubItems = true;
	}
	else
	{
		if (!pItem->pParent || pItem->pParent->type != T_SPLIT)
			return Kill(iid, FALSE);

		pItem->type = GetType(type);
		if (!pItem->pItem)
		{
			pItem->pItem = new CMFCPropertyGridProperty(CString(type));
			if (bAdd && pItem->pParent && pItem->pParent->pItem)
				pItem->pParent->pItem->AddSubItem(pItem->pItem);
		}
		else
			pItem->pItem->SetName(CString(type));

		pItem->pItem->Expand(CStringA(pItem->xml["set"].attribute_value("expanded")) == "true");

		bSubItems = FillWindow(pItem);
	}

	return bSubItems;
}

bool CViDesktopCfg::FillWindow(VI_NET_ITEM *pItem)
{
	if (!pItem || !pItem->pItem)
		return false;
	CInc busy(&m_nBusy);

	bool bSubItems = false;

	VI_NET_ITEM &iType = NewItem();
		
	CStringA xType(pItem->xml.attribute_value("type"));

	iType.type = T_TYPE;
	iType.pParent = pItem;
	iType.pItem =  new CMFCPropertyGridProperty(_T("type"), CString(xType), _T(""));

	if (pItem->type == T_SPLIT)
		iType.pItem->AddOption(_T("new record"));

	if(pItem->pParent)
		iType.pItem->AddOption(_T("delete record"));

	iType.pItem->AddOption(_T("split"));

//	if (pItem->pParent )
	{
		iType.pItem->AddOption(_T("camera"));
		iType.pItem->AddOption(_T("overview"));
		iType.pItem->AddOption(_T("text_log"));
		iType.pItem->AddOption(_T("image_log"));
		iType.pItem->AllowEdit(FALSE);
	}
	iType.pItem->SetData(iType.iid);
	pItem->pItem->AddSubItem(iType.pItem);

	switch (pItem->type)
	{
	case T_SPLIT:
		FillSplit(pItem);
		bSubItems = true;
		break;
	case T_CAMERA:
		FillCamera(pItem);
		break;
	case T_OWERVIEW:
		FillOwerview(pItem);
		break;
	case T_LOG_IMG:
		FillLogImg(pItem);
		break;
	case T_LOG_LOG:
		FillLogLog(pItem);
		break;
	default: break;
	}
	

	return bSubItems;
}

bool CViDesktopCfg::FillSplit(VI_NET_ITEM *pItem)
{
	if (!pItem || !pItem->pItem)
		return false;
	CInc busy(&m_nBusy);

	VI_NET_ITEM &iRows = NewItem();
	VI_NET_ITEM &iCols = NewItem();

	long nRows = atoi(CStringA(pItem->xml["set"].attribute_value("rows")));
	long nCols = atoi(CStringA(pItem->xml["set"].attribute_value("cols")));

	iCols.type = T_SPLIT_COLS;
	iCols.pParent = pItem;
	iCols.pItem = new CMFCPropertyGridProperty(_T("Cols"), (_variant_t)nCols, _T("n Cols"));
	iCols.pItem->SetData(iCols.iid);
	pItem->pItem->AddSubItem(iCols.pItem);

	iRows.type = T_SPLIT_ROWS;
	iRows.pParent = pItem;
	iRows.pItem = new CMFCPropertyGridProperty(_T("Rows"), (_variant_t)nRows, _T("n Rows"));
	iRows.pItem->SetData(iRows.iid);
	pItem->pItem->AddSubItem(iRows.pItem);

	PostMessage(WM_COMMAND, ID_MSG_FILL_SPLIT, pItem->iid);
	return true;
}

bool CViDesktopCfg::KillChild(VI_NET_ITEM* pItem,BOOL bLock)
{
	CSingleLock lock(&theApp.m_pDoc->m_xmlLock, bLock);
	CInc busy(&m_nBusy);

	std::list<int> lKill;
	for (std::map<int, VI_NET_ITEM>::iterator i = m_items.begin(), ie = m_items.end(); i != ie; ++i)
	{
		VI_NET_ITEM &ic = i->second;

		if (ic.pParent == pItem)
			lKill.push_back(i->first);
	}

	if (lKill.empty())
		return false;

	while (!lKill.empty())
	{
		Kill(lKill.back(),FALSE);
		lKill.pop_back();
	}
	return true;
}

bool CViDesktopCfg::Kill(int iid,BOOL bLock)
{
	CSingleLock lock(&theApp.m_pDoc->m_xmlLock, bLock);
	CInc busy(&m_nBusy);

	std::map<int, VI_NET_ITEM>::iterator ii = m_items.find(iid);
	if (ii == m_items.end())
		return false;
	VI_NET_ITEM *pItem = &(ii->second);
	
	KillChild(pItem,FALSE);

	
	if (pItem->pParent)
	{
		if (m_hWnd && pItem->pParent->pItem)
			pItem->pParent->pItem->RemoveSubItem(pItem->pItem, TRUE);
	}
	else
	{
		if(IsWindow(m_wndPropList))
			m_wndPropList.DeleteProperty(pItem->pItem);
	}

	if (pItem->xml)
		pItem->xml.parent().remove_child(pItem->xml);

	if (ii->second.pWnd)
	{
		CWnd *pWnd = ii->second.pWnd->GetWindow();
		if (pWnd)
		{
			if (IsWindow(pWnd->m_hWnd))
				pWnd->DestroyWindow();
			delete (pWnd);
		}
	}
	m_items.erase(ii);
	return false;
}

ITEM_TYPES CViDesktopCfg::GetType(const CStringA& sid)
{
	if (sid == "split")		return T_SPLIT;
	if (sid == "camera")	return T_CAMERA;
	if (sid == "overview")	return T_OWERVIEW;
	if (sid == "image_log")	return T_LOG_IMG;
	if (sid == "text_log")	return T_LOG_LOG;

	return T_EMPTY;
}

VI_NET_ITEM& CViDesktopCfg::NewItem()
{
	int iid = m_nID++;
	VI_NET_ITEM& i = m_items[iid];
	i.iid = iid;
	i.pItem = 0;
	i.pParent = 0;
	i.type = T_EMPTY;
	i.pWnd = 0;
	return i;
}

bool CViDesktopCfg::FillCamera(VI_NET_ITEM *pItem)
{
	if (!pItem || !pItem->pItem)
		return false;
	CInc busy(&m_nBusy);

	VI_NET_ITEM &iHost = NewItem();
	VI_NET_ITEM &iID = NewItem();

	iID.type = T_CAMERA_ID;
	iID.pParent = pItem;
	iID.pItem = new CMFCPropertyGridProperty(_T("id"), (_variant_t)(LPCTSTR)CString(pItem->xml["set"].attribute_value("id")), _T("id"));
	iID.pItem->SetData(iID.iid);
	pItem->pItem->AddSubItem(iID.pItem);

	iHost.type = T_CAMERA_HOST;
	iHost.pParent = pItem;
	iHost.pItem = new CMFCPropertyGridProperty(_T("host"), (_variant_t)(LPCTSTR)CString(pItem->xml["set"].attribute_value("host")), _T("host"));
	iHost.pItem->SetData(iHost.iid);
	pItem->pItem->AddSubItem(iHost.pItem);


	VI_NET_ITEM &iPath = NewItem();

	iPath.type = T_LOG_PATH;
	iPath.pParent = pItem;
	iPath.pItem = new CMFCPropertyGridFileProperty(_T("path"), CString(pItem->xml["set"].attribute_value("path")));
	iPath.pItem->SetData(iPath.iid);
	pItem->pItem->AddSubItem(iPath.pItem);


	CViViewCamera *pCamera;
	pItem->pWnd = pCamera = new CViViewCamera();

	if(!pItem->pParentWnd)
		pItem->pParentWnd = theApp.m_pFrame;

	pCamera->pItem = pItem;
	pCamera->Create(0, 0, WS_CHILD, CRect(0, 0, 100, 100), pItem->pParentWnd, pItem->iid, 0);

	return true;
}

bool  CViDesktopCfg::FillOwerview(VI_NET_ITEM *pItem)
{
	if (!pItem || !pItem->pItem)
		return false;
	CInc busy(&m_nBusy);


	CViViewOverview *pOW;
	pItem->pWnd = pOW = new CViViewOverview();

	if (!pItem->pParentWnd)
		pItem->pParentWnd = theApp.m_pFrame;

	pOW->pItem = pItem;
	pOW->Create(0, 0, WS_CHILD, CRect(0, 0, 100, 100), pItem->pParentWnd, pItem->iid, 0);

	return true;
}

bool CViDesktopCfg::FillLogImg(VI_NET_ITEM *pItem)
{
	if (!pItem || !pItem->pItem)
		return false;
	CInc busy(&m_nBusy);

	VI_NET_ITEM &iCnt = NewItem();

	iCnt.type = T_LOG_IMG_COUNT;
	iCnt.pParent = pItem;
	iCnt.pItem = new CMFCPropertyGridProperty(_T("count"), (_variant_t)(long)atoi(pItem->xml["set"].attribute_value("count")),_T(""));
	iCnt.pItem->SetData(iCnt.iid);
	pItem->pItem->AddSubItem(iCnt.pItem);

	CViViewLogImg *pLI;
	pItem->pWnd = pLI = new CViViewLogImg();

	if (!pItem->pParentWnd)
		pItem->pParentWnd = theApp.m_pFrame;

	pLI->pItem = pItem;
	pLI->Create(0, 0, WS_CHILD, CRect(0, 0, 100, 100), pItem->pParentWnd, pItem->iid, 0);

	return true;
}

bool CViDesktopCfg::FillLogLog(VI_NET_ITEM *pItem)
{
	if (!pItem || !pItem->pItem)
		return false;
	CInc busy(&m_nBusy);



	CViViewLogLog *pLL;
	pItem->pWnd = pLL = new CViViewLogLog();

	if (!pItem->pParentWnd)
		pItem->pParentWnd = theApp.m_pFrame;

	pLL->pItem = pItem;
	pLL->Create(0, 0, WS_CHILD, CRect(0, 0, 100, 100), pItem->pParentWnd, pItem->iid, 0);

	return true;
}


void CViDesktopCfg::OnChangeSelection(int iid)
{
}

BOOL CViDesktopCfg::CTRL::EditItem(CMFCPropertyGridProperty* pProp, LPPOINT lptClick)
{
	ASSERT(!m_currentEdit);
	m_currentEdit = (int)pProp->GetData();
	return CMFCPropertyGridCtrl::EditItem(pProp, lptClick);
}

BOOL CViDesktopCfg::CTRL::EndEditItem(BOOL bUpdateData)
{
	BOOL ok = CMFCPropertyGridCtrl::EndEditItem(bUpdateData);

	m_currentEdit = 0;
	return ok;
}

void CViDesktopCfg::OnEdit(int iid)
{
	CInc busy(&m_nBusy);

	if (iid <= 0)
		return;
	std::map<int, VI_NET_ITEM>::iterator pi = m_items.find(iid);
	if (pi == m_items.end())
	{
		ASSERT(FALSE);
		return;
	}

	OnEdit(&pi->second);
}

void CViDesktopCfg::OnEdit(VI_NET_ITEM *pItem)
{
	CInc busy(&m_nBusy);

	COleVariant v;

	if (pItem->pItem)
	{
		v = pItem->pItem->GetValue();
		if (v == pItem->value)
			return;
		pItem->value = v;
	}
	switch (pItem->type)
	{
	case T_TYPE:
		PostMessage(WM_COMMAND, ID_MSG_TYPECHANGE, pItem->iid);
		break;
	case T_CAMERA_HOST:
		pItem->pParent->xml["set"].attribute_new("host").set_value(v.bstrVal);
		if (pItem->pParent->pWnd)
			pItem->pParent->pWnd->GetWindow()->PostMessage(WM_COMMAND, ID_MSG_RESTART,0);
		theApp.m_pDoc->SetModifiedFlag();
		break;
	case T_CAMERA_ID:
		pItem->pParent->xml["set"].attribute_new("id").set_value(v.bstrVal);
		theApp.m_pDoc->SetModifiedFlag();
		break;
	case T_LOG_IMG_COUNT:
		pItem->pParent->xml["set"].attribute_new("count").set_value(v.intVal);
		theApp.m_pDoc->SetModifiedFlag();
		break;
	case T_SPLIT_COLS:
		pItem->pParent->xml["set"].attribute_new("cols").set_value(v.intVal);
		PostMessage(WM_COMMAND, ID_MSG_FILL_SPLIT, pItem->pParent->iid);
		theApp.m_pDoc->SetModifiedFlag();
		break;
	case T_SPLIT_ROWS:
		pItem->pParent->xml["set"].attribute_new("rows").set_value(v.intVal);
		PostMessage(WM_COMMAND, ID_MSG_FILL_SPLIT, pItem->pParent->iid);
		theApp.m_pDoc->SetModifiedFlag();
		break;
	case T_LOG_PATH:
		pItem->pParent->xml["set"].attribute_new("path").set_value(v.bstrVal);
		theApp.m_pDoc->SetModifiedFlag();
		break;
	default: break;
	}
}

LRESULT CViDesktopCfg::OnPropertyChanged(__in WPARAM wparam, __in LPARAM lparam)
{
	CInc busy(&m_nBusy);

	// Parameters:
	// [in] wparam: the control ID of the CMFCPropertyGridCtrl that changed.
	// [in] lparam: pointer to the CMFCPropertyGridProperty that changed.

	// Cast the lparam to a property.
	CMFCPropertyGridProperty * pp = (CMFCPropertyGridProperty *)lparam;
	int iid = (int)pp->GetData();

	if(iid > 0)
		OnEdit(iid);

	return 0;
}




BOOL CViDesktopCfg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case ID_MSG_TYPECHANGE:
		OnTypeChange((int)lParam);
		return TRUE;
	case ID_MSG_FILL_SPLIT:
		OnFillSplit((int)lParam);
		return TRUE;
	default: break;
	}

	return CDockablePane::OnCommand(wParam, lParam);
}

void CViDesktopCfg::OnTypeChange(int iid)
{
	CSingleLock lock(&theApp.m_pDoc->m_xmlLock, TRUE);
	CInc busy(&m_nBusy);

	std::map<int, VI_NET_ITEM>::iterator i = m_items.find(iid);
	if (i == m_items.end())
		return;

	VI_NET_ITEM *pItem = &(i->second);
	VI_NET_ITEM *pParent = pItem->pParent;
	COleVariant v = pItem->value.bstrVal;

	if (v.vt == VT_BSTR)
	{
		if (CStringA(pItem->value.bstrVal) == "new record")
		{
			pItem->value = COleVariant(_T("split"));
			pItem->pItem->SetValue(pItem->value);

			VI_NET_ITEM &iNew = NewItem();
			iNew.xml = pItem->pParent->xml.append_child();
			iNew.xml.set_name("item");
			iNew.xml.append_attribute("type").set_value("null");
			iNew.pParent = pParent;

			FillItem(&iNew,true);
			
			iNew.pItem->SetData(iNew.iid);

			m_wndPropList.AdjustLayout();

			PostMessage(WM_COMMAND, ID_MSG_FILL_SPLIT, pItem->pParent->iid);
			theApp.m_pDoc->SetModifiedFlag();

			
		}
		else
		if (CStringA(pItem->value.bstrVal) == "delete record")
		{
			Kill(pParent->iid,FALSE);
			m_wndPropList.AdjustLayout();
			theApp.m_pDoc->SetModifiedFlag();
		}
		else
		{
			KillChild(pParent, FALSE);
			if (pParent->pWnd)
			{
				if (IsWindow(pParent->pWnd->GetWindow()->m_hWnd))
					pParent->pWnd->GetWindow()->DestroyWindow();
				delete pParent->pWnd->GetWindow();
				pParent->pWnd = 0;
			}
			pParent->xml.attribute("type").set_value(v.bstrVal);
			pParent->xml.remove_child("set");
			pParent->type = T_EMPTY;
			FillItem(pParent);
			m_wndPropList.AdjustLayout();
			theApp.m_pDoc->SetModifiedFlag();

			if(pParent->pParent && pParent->pParent->type == T_SPLIT)
				PostMessage(WM_COMMAND, ID_MSG_FILL_SPLIT, pParent->pParent->iid);
		}
	}
	else
	{
		ASSERT(FALSE);
	}

	AdjustLayout();
}

void CViDesktopCfg::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent)
	{
	case T_LAYOUT:
		if (!m_nBusy)
		{
			KillTimer(nIDEvent);
			AdjustLayout();
		}
		return;
	default: break;
	}
	CDockablePane::OnTimer(nIDEvent);
}

void CViDesktopCfg::KillSplit(VI_NET_ITEM *pItem)
{
	std::list<VI_NET_ITEM *> items = ChildItems(pItem->iid, true);

	for (std::list<VI_NET_ITEM *>::iterator i = items.begin(), ie = items.end(); i != ie; ++i)
	{
		if ((*i)->pWnd && IsWindow((*i)->pWnd->GetWindow()->m_hWnd))
		{
			(*i)->pParentWnd = theApp.m_pView;
			(*i)->pWnd->GetWindow()->SetParent(theApp.m_pView);
		}
	}

	if (pItem->pWnd && pItem->type == T_SPLIT)
	{
		if (IsWindow(pItem->pWnd->GetWindow()->m_hWnd))
			pItem->pWnd->GetWindow()->DestroyWindow();
		delete (pItem->pWnd->GetWindow());
		pItem->pWnd = 0;
		
	}

}


void CViDesktopCfg::OnFillSplit(int iid)
{
	CSingleLock lock(&theApp.m_pDoc->m_xmlLock, TRUE);
	CInc busy(&m_nBusy);

	std::map<int, VI_NET_ITEM>::iterator i = m_items.find(iid);
	if (i == m_items.end())
		return;

	VI_NET_ITEM *pItem = &(i->second);

	ASSERT(pItem->type == T_SPLIT);

	CWnd *pParentWnd = theApp.m_pView;

	if (pItem->pParentWnd && IsWindow(pItem->pParentWnd->m_hWnd))
		pParentWnd = pItem->pParentWnd;
	else
	if (pItem->pParent && pItem->pWnd && IsWindow(pItem->pWnd->GetWindow()->m_hWnd))
		pParentWnd = pItem->pWnd->GetWindow()->GetParent();
	
	KillSplit(pItem);


	CViViewSplit *pSplit;
	
	pItem->pWnd = pSplit = new CViViewSplit();
	pSplit->pItem = pItem;

	pItem->pParentWnd = pParentWnd;
	pSplit->Create(pParentWnd, pItem->xml,pItem->iid);
	
	AdjustLayout();
}

std::list<VI_NET_ITEM *> CViDesktopCfg::ChildItems(int iid, bool bWndOnly)
{
	std::list<VI_NET_ITEM *> items;
	for (std::map<int, VI_NET_ITEM>::iterator i = m_items.begin(), ie = m_items.end(); i != ie; ++i)
	{
		VI_NET_ITEM &ic = i->second;
		if ((ic.pParent && ic.pParent->iid == iid) || (!ic.pParent && iid < 0))
		{
			if(!bWndOnly || ic.pWnd || ic.type == T_SPLIT)
				items.push_back(&ic);
		}
	}

	return items;
}