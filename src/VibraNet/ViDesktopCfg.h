
#pragma once

class CViViewBase;

enum ITEM_TYPES {
	T_EMPTY,
	T_TYPE,
	T_SPLIT, T_SPLIT_ROWS, T_SPLIT_COLS,
	T_CAMERA, T_CAMERA_HOST, T_CAMERA_ID,
	T_LOG_LOG,
	T_LOG_IMG,
	T_LOG_IMG_COUNT,
	T_LOG_PATH,
	T_OWERVIEW
};

typedef struct tagVI_NET_ITEM
{
	int							iid;
	ITEM_TYPES					type;
	xml_node					xml;
	struct tagVI_NET_ITEM *		pParent;
	CMFCPropertyGridProperty *	pItem;
	COleVariant					value;

	CViViewBase*				pWnd;
	CWnd *						pParentWnd;
} VI_NET_ITEM;

class CViDesktopCfg : public CDockablePane
{
	// Construction
public:
	CViDesktopCfg();

	void AdjustLayout();

	// Attributes
public:
	void SetVSDotNetLook(BOOL bSet)
	{
		m_wndPropList.SetVSDotNetLook(bSet);
		m_wndPropList.SetGroupNameFullWidth(bSet);
	}

protected:
	enum TIMER { T_LAYOUT = 1 };
	enum MSG { ID_MSG_TYPECHANGE = 49981, ID_MSG_FILL_SPLIT, ID_MSG_RESTART };


	CFont					m_fntPropList;


	int							m_nID;
	std::map<int, VI_NET_ITEM>	m_items;
	int							m_nBusy;

	friend class CViViewBase;
	friend class CViViewSplit;
	friend class CViViewCamera;
	// Implementation
protected:

	class CTRL : public CMFCPropertyGridCtrl
	{
		friend class ITEM_PROP;

		CViDesktopCfg *pBase;
	public:
		int m_currentEdit;
	public:
		CTRL(CViDesktopCfg *pb) :pBase(pb), m_currentEdit(0) {}
		virtual ~CTRL() {}

		virtual BOOL EditItem(CMFCPropertyGridProperty* pProp, LPPOINT lptClick = NULL);
		virtual BOOL EndEditItem(BOOL bUpdateData = TRUE);
		
		virtual void OnChangeSelection(CMFCPropertyGridProperty* pNewSel, CMFCPropertyGridProperty* pOldSel)
		{
			if (pNewSel)
				pBase->OnChangeSelection((int)pNewSel->GetData());
			else
				pBase->OnChangeSelection(0);
		}

	};

	CTRL	m_wndPropList;
public:
	virtual ~CViDesktopCfg();

public:
	
	virtual bool Reload(const CString& path,BOOL bLock = TRUE);
	virtual bool ReloadString(const char* str, BOOL bLock = TRUE);
	virtual bool FillList(BOOL bLock = TRUE);
	virtual bool FillList(xml_node &xml,VI_NET_ITEM *pGroup, BOOL bLock = TRUE);
	virtual bool FillItem(VI_NET_ITEM *pItem, bool bAdd=false);
	virtual bool FillSplit(VI_NET_ITEM *pItem);
	virtual bool FillWindow(VI_NET_ITEM *pItem);
	virtual bool FillCamera(VI_NET_ITEM *pItem);
	virtual bool FillOwerview(VI_NET_ITEM *pItem);
	virtual bool FillLogImg(VI_NET_ITEM *pItem);
	virtual bool FillLogLog(VI_NET_ITEM *pItem);

	virtual void Clear(BOOL bLock = TRUE);

	virtual void OnChangeSelection(int iid);
	virtual void OnEdit(int iid);
	virtual void OnEdit(VI_NET_ITEM *pItem);
	virtual void OnTypeChange(int iid);
	virtual void OnFillSplit(int iid);
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);

	virtual LRESULT OnPropertyChanged(__in WPARAM wparam, __in LPARAM lparam);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()

	void InitPropList();
	void SetPropListFont();
	bool Kill(int iid, BOOL bLock);
	bool KillChild(VI_NET_ITEM* pItem, BOOL bLock);
	void KillSplit(VI_NET_ITEM *pItem);
	
	ITEM_TYPES GetType(const CStringA& sid);

	VI_NET_ITEM& NewItem();

	std::list<VI_NET_ITEM *> ChildItems(int iid,bool bWndOnly);
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};

