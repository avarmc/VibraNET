
// ViDesktop.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "ViDesktop.h"
#include "ViDesktopFrm.h"

#include "ViDesktopDoc.h"
#include "ViDesktopView.h"

#include "ViNet.h"
#include "ViViewLogLog.h"
#include "ViViewLogImg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define VI_VAR_MAKE_TAGS
#include "VIVarTag.h"
// CViDesktopApp

BEGIN_MESSAGE_MAP(CViDesktopApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CViDesktopApp::OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinAppEx::OnFilePrintSetup)
END_MESSAGE_MAP()


// CViDesktopApp construction

CViDesktopApp::CViDesktopApp():
	m_pDoc(0)
	, m_pFrame(0)
	, m_pView(0)
	, m_bPowerOff(FALSE)
	, m_bDone(FALSE)
{
	m_bHiColorIcons = TRUE;

	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
#ifdef _MANAGED
	// If the application is built using Common Language Runtime support (/clr):
	//     1) This additional setting is needed for Restart Manager support to work properly.
	//     2) In your project, you must add a reference to System.Windows.Forms in order to build.
	System::Windows::Forms::Application::SetUnhandledExceptionMode(System::Windows::Forms::UnhandledExceptionMode::ThrowException);
#endif

	// TODO: replace application ID string below with unique ID string; recommended
	// format for string is CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("ViDesktop.AppID.NoVersion"));

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

// The one and only CViDesktopApp object

CViDesktopApp theApp;


// CViDesktopApp initialization

BOOL CViDesktopApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	EnableTaskbarInteraction(FALSE);

	ParseCommandLine(m_cmdl);

	// AfxInitRichEdit2() is required to use RichEdit control	
	// AfxInitRichEdit2();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	m_regString = _T("ELSYS\\VibraNet");

	if (m_cmdl.IsSet(_T("/copy"), 2))
	{
		long ext_ver = str2int(m_cmdl.GetParam(_T("/copy")));
		m_regString += CString("\\copy_") + v2str(ext_ver);
	}

	SetRegistryKey(m_regString);

	LoadStdProfileSettings(0);  // Load standard INI file options (including MRU)


	InitContextMenuManager();

	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	m_pNet = new CVINetServerDoc();
//	m_pNet->Init();


	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CViDesktopDoc),
		RUNTIME_CLASS(CViDesktopFrame),       // main SDI frame window
		RUNTIME_CLASS(CViDesktopView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);


	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(m_cmdl))
		return FALSE;

	// The one and only window has been initialized, so show and update it
	m_pMainWnd->ShowWindow(SW_SHOWMAXIMIZED);
	m_pMainWnd->UpdateWindow();
	return TRUE;
}

int CViDesktopApp::ExitInstance()
{
	m_bDone = true;

	Sleep(500);

	if(m_pNet)
		m_pNet->Close();

	SAFE_DELETE(m_pNet);
	
	AfxOleTerm(FALSE);


	return CWinAppEx::ExitInstance();
}

// CViDesktopApp message handlers


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// App command to run the dialog
void CViDesktopApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CViDesktopApp customization load/save methods

void CViDesktopApp::PreLoadState()
{
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_EDIT_MENU);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
}

void CViDesktopApp::LoadCustomState()
{
}

void CViDesktopApp::SaveCustomState()
{
}

// CViDesktopApp message handlers




void CViDesktopApp::AddLog(int iid,const CString& cam, const CString& text, const CString& file,bool bReplace)
{
	CSingleLock lock(&m_listLock,TRUE);
	for (std::list< CViViewLogLog* >::iterator i = m_listLog.begin(), ie = m_listLog.end(); i != ie; ++i)
		(*i)->AddLog(iid,cam, text, file, bReplace);
	for (std::list< CViViewLogImg* >::iterator i = m_listImg.begin(), ie = m_listImg.end(); i != ie; ++i)
		(*i)->AddLog(iid, cam, text, file, bReplace);

}

BOOL CAboutDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	
	return TRUE;

}
