// VINetServerDoc.cpp : implementation of the CVINetServerDoc class
//

#include "stdafx.h"
#include "ViDesktop.h"
#include "ViDesktopFrm.h"
#include "ViDesktopDoc.h"
#include "ViDesktopView.h"
#include "ViNet.h"
#include "FileDB.h"
#include "ViViewCamera.h"


#define VI_VAR_MAKE_TAGS
#include "VIVarTag.h"
#include "imagepack.h"

#include "inc.h"

extern LPTSTR gVI_VAR_TAGS[];

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

std::list<CVINetServerDoc *> CVINetServerDoc::m_lDocs;

// CVINetServerDoc

// CVINetServerDoc construction/destruction

CVINetServerDoc::CVINetServerDoc() : pApp(&theApp)
{
	m_lDocs.push_back(this);

	
	m_pCamera = 0;
	m_bClose = 0;
	m_pItem = 0;
	m_lastLog.t = m_varVer = m_imgVer = GetTickCount() - 12000;

	if (!pApp->m_pNet)
		pApp->m_pNet = this;

	m_varStackProc = 0;
	m_pvi = 0;
	m_imgLock = false;
	m_imgFaceRectTime = GetTickCount() - 10000;


	m_tLastData = m_tLastImage = GetTickCount() - 10000;
	m_bRestarted = false;

	m_faceProcTh = 0;
	for (int i = 0; i < 4; ++i)
		m_imgPackThread[i] = 0;

}

CVINetServerDoc::~CVINetServerDoc()
{
	Close();

	if (IsMain())
		SAFE_DELETE( m_pvi );

	m_lDocs.remove(this);
}

BOOL CVINetServerDoc::Init()
{
	ITEM_TYPES type = T_EMPTY;
	if (m_pItem)
		type = m_pItem->type;

	if (!pApp->m_pNet)
		return FALSE;

	if (IsMain())
	{
		if (!m_pvi)
			m_pvi = new CVINetModuleDll;

		for (int i = 0; i < 4; ++i)
			m_imgPackThread[i] = CreateThread(0, 0, OnImagePackThread, this, 0, 0);

		m_bRestarted = false;
	}
	else
	if (type == T_CAMERA)
	{
		m_pvi = pApp->m_pNet->m_pvi;
		if (!m_pvi)
			return FALSE;
		m_bRestarted = false;
	}
	else
	if (type == T_LOG_LOG)
	{
	}
	else
	if (type == T_LOG_IMG)
	{
	}
	else
		return false;

	return TRUE;
}


void CVINetServerDoc::Close()
{
	CInc cls(&m_bClose);

	while (m_faceProcTh || m_varStackProc || m_imgPackThread[0] || m_imgPackThread[1] || m_imgPackThread[2] || m_imgPackThread[3])
	{
		Sleep(10);
	}

	if (m_pvi && IsMain())
	{
		m_pvi->Disconnect(0);
	}

	CSingleLock lock(&m_docLock, TRUE);
	
	std::map<CString, CVINetServerDoc*>::iterator i, ie;
	for (i = pApp->m_pNet->m_ips.begin(), ie = pApp->m_pNet->m_ips.end(); i != ie; )
	{
		if (i->second == this)
		{
			pApp->m_pNet->m_ips.erase(i);
			i = pApp->m_pNet->m_ips.begin();
			ie = pApp->m_pNet->m_ips.end();
		}
		else
			++i;
	}

}



// CVINetServerDoc commands


bool CVINetServerDoc::IsError()
{
	DWORD t = GetTickCount();
	DWORD dtImg = (t - m_imgVer);
	DWORD dtVar = (t - m_varVer);
#ifndef _DEBUG
	if (dtImg > 3000 || dtVar > 3000)
		return true;
#else
	if (dtImg > 60000 || dtVar > 60000)
		return true;
#endif
	if (!m_pvi || !m_pvi->IsOpen())
		return true;
	if (m_img.IsNull())
		return true;
	return false;
}

bool CVINetServerDoc::Connect(void)
{
	if (!m_pvi)
		return false;

	int				modPortTCP = 0;
	int				terPortTCP = 0;
	int				terPortUDP = 0;

	CSingleLock ilock(&m_imgSync, TRUE);
	m_varVer = m_imgVer = GetTickCount() - 12000;
	if (!m_img.IsNull())
		m_img.Destroy();

	ilock.Unlock();

	CStringA		remIPStr;
	LPCSTR			remIP = 0;

	xml_attribute a;
	if (m_pItem)
	{
		a = m_pItem->xml["set"].attribute("host");
		if (!a.empty()) remIPStr = a.value();
	}

	if (IsMain())
	{
		modPortTCP = 0;
		terPortTCP = 5221;
		terPortUDP = 5222;

		m_pvi->Disconnect(0);

		if (m_pItem)
		{
			xml_attribute a;

			a = m_pItem->xml["set"].attribute("modPortTCP");
			if (!a.empty()) modPortTCP = a.as_int();

			a = m_pItem->xml["set"].attribute("terPortTCP");
			if (!a.empty()) terPortTCP = a.as_int();

			a = m_pItem->xml["set"].attribute("terPortUDP");
			if (!a.empty()) terPortUDP = a.as_int();
		}
	}
	else
	if (remIPStr.IsEmpty())
		return false;
	else
	{

		int pos = remIPStr.Find('/');

		modPortTCP = 5220;
		if(pos > 0)
			modPortTCP += atoi(remIPStr.Mid(pos+1))*10;

		remIP = remIPStr;
	}


	if (!IsMain())
	{
		while (!m_ips.empty())
		{
			CString ipr = m_ips.begin()->first;
			m_pvi->Disconnect(CStringA(ipr));
			m_ips.erase(ipr);
			pApp->m_pNet->m_ips.erase(ipr);
		}
	}
	if (!m_pvi->Connect(remIP
		, modPortTCP
		, terPortTCP
		, terPortUDP))
		return false;


	if (!IsMain())
	{
		CSingleLock lock(&pApp->m_pNet->m_docLock, TRUE);
		pApp->m_pNet->m_ips[CString(remIP)] = m_ips[CString(remIP)] = this;
		return true;
	}

	m_pvi->SetCallbackOnImagePack(CallbackOnImagePack, this);
	m_pvi->SetCallbackOnVar(CallbackOnVar, this);


	m_pvi->AddTag(VI_VAR_STATE_VAR, CStringA(gVI_VAR_TAGS[VI_VAR_STATE_VAR]));
	m_pvi->AddTag(VI_VAR_STATE_CRITICAL, CStringA(gVI_VAR_TAGS[VI_VAR_STATE_CRITICAL]));
	m_pvi->AddTag(VI_VAR_STATE_CRITICAL_LEV, CStringA(gVI_VAR_TAGS[VI_VAR_STATE_CRITICAL_LEV]));
	m_pvi->AddTag(VI_VAR_STATE_CUSTOMER_LEV, CStringA(gVI_VAR_TAGS[VI_VAR_STATE_CUSTOMER_LEV]));
	m_pvi->AddTag(VI_VAR_RESET, CStringA(gVI_VAR_TAGS[VI_VAR_RESET]));
	m_pvi->AddTag(VI_VAR_RESET, CStringA(gVI_VAR_TAGS[VI_MODE_RESULT]));

	AddRqst(VI_VAR_STATE_CRITICAL);
	AddRqst(VI_VAR_STATE_CRITICAL_LEV);
	AddRqst(VI_VAR_STATE_CUSTOMER_LEV);
	AddRqst(VI_MODE_RESULT);
	
	AddRqst(VI_VAR_N0_RQST);
	AddRqst(VI_VAR_K);
	AddRqst(VI_VAR_TH);
	AddRqst(VI_VAR_SIZE);

	AddRqst(VI_VAR_STAT_INTEGR0B);
	AddRqst(VI_VAR_STAT_INTEGR0A);

	AddRqst(VI_VAR_AUDIO_LEVEL);
	AddRqst(VI_VAR_STAT_RES_F5X);
	AddRqst(VI_VAR_STAT_RES_P6);
	AddRqst(VI_VAR_STAT_RES_P7);
	AddRqst(VI_VAR_STAT_RES_P8);

	AddRqst(VI_VAR_FPSIN);
	AddRqst(VI_VAR_FPSOUTF);
	AddRqst(VI_VAR_FPSOUTR);
	

	return true;
}

void CVINetServerDoc::CallbackOnImagePack(void *UserData, VI_MODULE module, const char *ip, void *ptr, int size, void *pData, int dataSize)
{
	CVINetServerDoc *pThis = (CVINetServerDoc*)UserData;
	pThis->OnImagePack(ip, ptr, size, pData, dataSize);
}

void CVINetServerDoc::CallbackOnVar(void *UserData, VI_MODULE module, const char *ip, const int *idVar, const VI_VAR *pVar, int cntVar)
{
	CVINetServerDoc *pThis = (CVINetServerDoc*)UserData;
	pThis->OnVar(ip, idVar, pVar, cntVar);
}

void CVINetServerDoc::AddRqst(int id)
{
	if (id >= 0 && id < VI_VAR_CNT)
	{
		m_rqst.push_back(gVI_VAR_TAGS[id]);
		if (m_pvi)
			m_pvi->AddTag(id, CStringA(gVI_VAR_TAGS[id]));
	}
}

bool CVINetServerDoc::CkeckRqst(void)
{
	if (m_rqst.empty() || !m_pvi)
		return false;
	while (!m_rqst.empty())
	{
		if (m_pvi->RqstTag(CStringA(m_rqst.front()), 1))
			return false;
		m_rqst.pop_front();
	}
	return true;
}

DWORD	CVINetServerDoc::VarThread(LPVOID p)
{
	CVINetServerDoc *pThis = (CVINetServerDoc*)p;
	pThis->VarThreadLocal();
	pThis->m_varStackProc = 0;

	return 0;
}

UINT	CVINetServerDoc::VarThreadLocal()
{
	CSingleLock lock(&m_varStackLock);
	while (!pApp->m_bPowerOff && !pApp->m_bDone && !m_bClose)
	{
		lock.Lock();
		if (m_varStack.empty())
		{
			lock.Unlock();
			Sleep(1);
			continue;
		}
		VARSTACK a = m_varStack.front();


		m_varStack.pop_front();
		lock.Unlock();

		if (a.id.size() > 0)
			OnVarLocal(a.ip, &a.id[0], &a.var[0], (int)a.id.size());
	}

	CloseHandle(m_varStackProc);
	m_varStackProc = 0;

	return 0;
}

void CVINetServerDoc::OnVar(const char *ip, const int *idVar, const VI_VAR *pVar, int cntVar)
{
	if (pApp->m_bPowerOff || pApp->m_bDone)
		return;

	OnVarLocal(ip, idVar, pVar, cntVar);
}




void CVINetServerDoc::CheckLogAdd()
{
	if (IsMain() || !IsModule())
		return;

	DWORD t = GetTickCount();
	DWORD dt = t - m_imgVer;
	bool docOk = ((t - m_tLastData) < 4000) || ((t - m_tLastImage) < 1000);

	float vi = GetF1(VI_VAR_STATE_VAR);
	float vc = GetF1(VI_VAR_STATE_CRITICAL);

	if (!docOk)
	{
		CSingleLock ilock(&m_imgSync, TRUE);
		if (!m_img.IsNull())
			m_img.Destroy();
	}

	if (docOk && vi >= vc)
	{
		if (!m_img.IsNull())
			AddLog(vi,vc);
	}

}

void CVINetServerDoc::OnVarLocal(const char *ip, const int *idVar, const VI_VAR *pVar, int cntVar)
{
	for (int i = 0; i < cntVar; ++i)
		OnVarLocal(ip, idVar[i], pVar[i]);
}

void  CVINetServerDoc::OnVarLocal(const char *ip, const int id, const VI_VAR &var)
{
	if (pApp->m_bPowerOff)
		return;

	m_tLastData = GetTickCount();


	if (ip && strlen(ip) > 0)
	{
		CVINetServerDoc* pDoc = FindIP(CString(ip));

		if (pDoc)
		{
			pDoc->OnVarLocal(0, id, var);
		}


	}
	else
	{
		CSingleLock varLock(&m_varLock, false);

		varLock.Lock();
		m_var[id] = var;
		varLock.Unlock();

		OnNewVar(id, 0);

	}
}

DWORD CVINetServerDoc::FaceProc(LPVOID p)
{
	CVINetServerDoc *pThis = (CVINetServerDoc *)p;
	pThis->FaceProcLocal();
	return 0;
}

UINT CVINetServerDoc::FaceProcLocal()
{
	while (!pApp->m_bDone && !pApp->m_bPowerOff && !m_bClose)
	{
		OnFace();
	}
	CloseHandle(m_faceProcTh);
	m_faceProcTh = 0;
	return 0;
}

void CVINetServerDoc::OnFace()
{
	CSingleLock plock(&m_faceProcLock, TRUE);

	if (m_faceProc.empty())
	{
		plock.Unlock();
		Sleep(1);
		return;
	}

	FACE_PROC curData = m_faceProc.front();
	m_faceProc.pop_front();

	plock.Unlock();

	void *pData = curData.data.p;
	void *ptr = curData.img.p;
	int size = (int)curData.img.size();
	int dataSize = (int)curData.data.size();

	CRect & r = *(CRect*)pData;
	float s = *(float*)((&r) + 1);
	float vc = GetF1(VI_VAR_STATE_CRITICAL);
	if (vc == 0)
		return;
	if (r.right <= r.left || r.bottom <= r.top)
		return;

/*
	CImage imgFace;

	if (!CImagePack::LoadImage(imgFace, ptr, size))
		return;
*/
}

void CVINetServerDoc::OnFace(void* ptr, int size, void *pData, int dataSize)
{
	CRect & r = *(CRect*)pData;
	float s = *(float*)((&r) + 1);
	float vc = GetF1(VI_VAR_STATE_CRITICAL);
	if (vc == 0)
		return;
	if (r.right <= r.left || r.bottom <= r.top	)
		return;

	CSingleLock plock(&m_faceProcLock, TRUE);

	if (m_faceProc.size() > 1000)
	{
		AfxGetMainWnd()->PostMessage(WM_CLOSE);
		return;
	}

	m_faceProc.push_back(FACE_PROC());
	m_faceProc.back().data.resize(dataSize);
	m_faceProc.back().img.resize(size);
	if (m_faceProc.back().img.p && ptr)
		SSESafeMemcpy(m_faceProc.back().img.p, ptr, size);
	if (m_faceProc.back().data.p && pData)
		SSESafeMemcpy(m_faceProc.back().data.p, pData, dataSize);


	if (!m_faceProcTh)
		m_faceProcTh = CreateThread(0,0,FaceProc, this,0,0);

}

DWORD  CVINetServerDoc::OnImagePackThread(LPVOID p)
{
	CVINetServerDoc *pThis = (CVINetServerDoc*)p;
	pThis->OnImagePackThreadLocal();
	return 0;
}

void CVINetServerDoc::OnImagePackThreadLocal()
{
	int id = GetCurrentThreadId();

	if (IsMain())
	{
		CSingleLock lock(&m_imgPackLock, false);
		while (!pApp->m_bPowerOff && !pApp->m_bDone && !m_bClose)
		{
			lock.Lock();
			if (m_imgPack.empty())
			{
				lock.Unlock();
				Sleep(2);
				continue;
			}

			IMG_PACK a = m_imgPack.front();
			m_imgPack.pop_front();
			lock.Unlock();


			OnImagePackFull(a.ip, &a.ptr[0], (int)a.ptr.size(),a.pData.size() ? &a.pData[0] : 0, (int)a.pData.size());
		}
	}

	for (int i = 0; i < 4; ++i)
		if (m_imgPackThread[i] && GetThreadId(m_imgPackThread[i]) == id)
		{
			CloseHandle(m_imgPackThread[i]);
			m_imgPackThread[i] = 0;
			break;
		}
	

}
void CVINetServerDoc::OnImagePack(const char* ip, void* ptr, int size, void *pData, int dataSize)
{
	CSingleLock lock(&m_imgPackLock, true);
	if (m_imgPack.size() > 50 || !size)
		return;

	m_imgPack.push_back(IMG_PACK());
	IMG_PACK& a = m_imgPack.back();
	a.ip = ip;
	if (ptr && size)
	{
		a.ptr.resize(size);
		memcpy(&a.ptr[0], ptr, size);
	}
	if (pData && dataSize)
	{
		a.pData.resize(dataSize);
		memcpy(&a.pData[0], pData, dataSize);
	}
}

void CVINetServerDoc::OnImagePackFull(const char* ip, void* ptr, int size, void *pData, int dataSize)
{
	if (pApp->m_bPowerOff || pApp->m_bDone || m_bClose)
		return;

	m_tLastImage = GetTickCount();

	DWORD t = GetTickCount();

	if (ip)
	{
		CVINetServerDoc* pDoc = FindIP(CString(ip));
		if (pDoc)
		{
			pDoc->OnImagePackFull(0, ptr, size, pData, dataSize);
		}
	}
	else
	{
		CSingleLock lock(&m_docLock, true);


		if (!m_imgLock)
		{
			DWORD t = GetTickCount();

			{
				CSingleLock ilock(&m_imgSync, TRUE);
				CImagePack::LoadImage(m_img, ptr, size);
				PutImageFace(m_img);
				m_imgVer = GetTickCount();
			}

		}

		if (dataSize)
		{
			if (dataSize == sizeof(RECT) + sizeof(float))
			{
				OnFace(ptr, size, pData, dataSize);
			}
		}
		else
			CheckLogAdd();
	}
}

bool CVINetServerDoc::IsMain(void)
{
	return (pApp->m_pNet == this);
}

CString CVINetServerDoc::GetCfg(LPCTSTR id)
{
	if(!m_pItem)
		return CString();
	xml_attribute a = m_pItem->xml["set"].attribute(CStringA(id));
	if(a.empty())
		return CString();

	return a.wvalue().c_str();
}

CVINetServerDoc* CVINetServerDoc::FindIP(const CString& ip)
{
	CSingleLock lock(&m_docLock, true);
	std::map<CString, CVINetServerDoc*>::iterator i = m_ips.find(ip);
	if (i != m_ips.end())
		return i->second;

	if (ip.Find('/') < 0)
		return FindIP(ip + _T("/0"));
	return NULL;
}

void CVINetServerDoc::OnNewVar(int id, int subID)
{
	m_varVer = GetTickCount();
	if (m_pCamera)
		m_pCamera->OnNewVar(id, subID, m_var[id]);
}

void CVINetServerDoc::SetTitle(LPCTSTR lpszTitle)
{
}

void CVINetServerDoc::SendVar(int id, const VI_VAR& v)
{
	if (!m_pvi) return;
	CStringA		remIPStr(GetCfg(_T("remIP")));
	if (!remIPStr.IsEmpty())
		m_pvi->PutVar(remIPStr, &id, &v, 1);
}

void CVINetServerDoc::SendVarI1(int id, int v)
{
	VI_VAR a;
	ZeroMemory(&a, sizeof(a));
	a.iv1 = v;
	SendVar(id, a);
}

void CVINetServerDoc::SendVarF1(int id, float v)
{
	VI_VAR a;
	ZeroMemory(&a, sizeof(a));
	a.fv1 = v;
	SendVar(id, a);
}

void CVINetServerDoc::SetCaption(const CString& txt, bool bProc)
{
}

CString CVINetServerDoc::GetCaption()
{
	if(!m_pCamera || !m_pCamera->pItem)
		return _T("");
	return m_pCamera->pItem->xml["set"].attribute("id").wvalue().c_str();
}

void CVINetServerDoc::AddLog(float vi, float vc)
{

	LOG log;

	log.tc = COleDateTime::GetCurrentTime();
	log.t = GetTickCount();
	log.cam = GetCaption();

	log.txt.Format(_T("%0.2f > %0.2f"),vi,vc);

	log.vc = vc;
	log.vi = vi;
	log.file = WriteImage(log.tc.Format(_T("%y-%m-%d_%H_%M_%S")),TRUE);

	AddLog(log,true);
}


void CVINetServerDoc::PurgeImage(CString path, bool bLock)
{
	DWORD nMax = _tstoi(GetCfg(_T("log_max")));
	if (!nMax)
		nMax = 10000;
	path.TrimRight('\\');

	CFileFind ff;
	BOOL ok;
	ok = ff.FindFile(path + _T("\\*.jpg"));
	std::list<CString> files;
	while (ok)
	{
		ok = ff.FindNextFile();
		files.push_back(ff.GetFilePath());
	}
	ff.Close();

	ok = ff.FindFile(path + _T("\\*.png"));
	while (ok)
	{
		ok = ff.FindNextFile();
		files.push_back(ff.GetFilePath());
	}
	ff.Close();

	ok = ff.FindFile(path + _T("\\*.bmp"));
	while (ok)
	{
		ok = ff.FindNextFile();
		files.push_back(ff.GetFilePath());
	}
	ff.Close();

	if (files.size() < nMax)
		return;

	files.sort();

	CFileStatus fs0, fs1;
	CFile::GetStatus(files.back(), fs0);

	while (files.size() > nMax)
	{
		try
		{
			CFile::Remove(files.front());
		}
		catch (CFileException *)
		{
		}
		files.pop_front();
	}
}




CString CVINetServerDoc::FileCaption(const CString& cap)
{
	CString str;

	int len = cap.GetLength();
	if (len < 22)
		return cap;

	if (cap.GetAt(len - 4) == '.'
		&& cap.GetAt(len - 7) == '_'
		&& cap.GetAt(len - 10) == '_'
		&& cap.GetAt(len - 13) == '_'
		&& cap.GetAt(len - 16) == '-'
		&& cap.GetAt(len - 19) == '-'
		&& cap.GetAt(len - 22) == '_')
	{
		str = cap.Left(len - 22);

		std::list<CVINetServerDoc *>::iterator i, ie;
	
		for ( i = m_lDocs.begin(), ie = m_lDocs.end(); i != ie; ++i)
		{
			if ((*i)->GetCfg(_T("nid")) != str)
				continue;
			str = (*i)->GetCaption();
			break;
		}
	}
	else
		str = cap;


	return str;
}




bool CVINetServerDoc::DrawRect(CImage& src, CRect r, COLORREF c)
{
	if (src.IsNull())
		return false;

	CImageDC idc(src);
	CDC *pDC = CDC::FromHandle(idc);

	CPen pen, *old;
	pen.CreatePen(PS_DOT, 2, c);

	old = pDC->SelectObject(&pen);

	pDC->MoveTo(r.left, r.top);
	pDC->LineTo(r.right, r.top);
	pDC->LineTo(r.right, r.bottom);
	pDC->LineTo(r.left, r.bottom);
	pDC->LineTo(r.left, r.top);

	pDC->SelectObject(old);

	return true;
}

void CVINetServerDoc::CheckRestart()
{

	BOOL bMain = IsMain();

	DWORD dt = GetTickCount() - m_tLastImage;
	if (!m_bRestarted || dt > 3000 )
	{
		m_bRestarted = Connect();
	}

	if (bMain)
	{
		std::list<CVINetServerDoc *>::iterator i, ie;

		for (i = m_lDocs.begin(), ie = m_lDocs.end(); i != ie; ++i)
		{
			CVINetServerDoc* pd = *i;
			if (pd && !pd->IsMain())
			{
			//	pd->Connect();
				pd->CheckRestart();
			}
		}
	}
}

bool CVINetServerDoc::CheckDetector()
{
	return false;
}

void CVINetServerDoc::PutImageFace(CImage& img)
{
	if (img.IsNull())
		return;
	int w = img.GetWidth();
	int h = img.GetHeight();
	int bpp = img.GetBPP();
	int pitch = img.GetPitch();
	if (bpp != 24)
		return;
	BYTE *p = (BYTE*)img.GetBits();
	if (pitch < 0)
		p += pitch * (h - 1);

	int cnt = 0;
	for (int y = 0; y < h; ++y)
	{
		RGBTRIPLE * py = (RGBTRIPLE *)(p + y*abs(pitch)), *pe = py + w;
		for (; py != pe; ++py)
		{
			if (py->rgbtRed > 200 && py->rgbtGreen < 20 && py->rgbtBlue < 20)
				++cnt;
			if (py->rgbtBlue > 200 && py->rgbtGreen < 20 && py->rgbtRed  < 20)
				++cnt;
		}
	}

	DWORD t = GetTickCount();
	if (cnt)
	{
		CImagePack::CopyImage(img, m_imgFaceRect);
		m_imgFaceRectTime = t;
	}
}

bool CVINetServerDoc::IsModule(void)
{
	if (IsMain() || !m_pItem || m_pItem->type != T_CAMERA)
		return false;

	return true;
}


void CVINetServerDoc::AddLog(LOG& log, bool bReplace)
{
	if (!m_pCamera || !m_pCamera->pItem)
		return;

	if (!bReplace)
	{
		theApp.AddLog(m_pCamera->pItem->iid, log.cam, log.txt, log.file, false);
	}
	else
	{
		CSingleLock lock(&m_lastLog.lock, TRUE);
		DWORD t = GetTickCount();
		DWORD dt = t - m_lastLog.t;

		DWORD period = 1000;

		if (dt > period)
		{
			m_lastLog.vi = -1;
			m_lastLog.t = t - period*100;
		}

		if(m_lastLog.vi < log.vi)
		{
			bool bReplace = (dt <= period);

			CString lastFile = m_lastLog.file;

			m_lastLog.cam = log.cam;
			m_lastLog.file = log.file;
			m_lastLog.t = log.t;
			m_lastLog.tc = log.tc;
			m_lastLog.txt = log.txt;
			m_lastLog.vc = log.vc;
			m_lastLog.vi = log.vi;

			if (bReplace && CFileDB::IsFile(lastFile))
			{
				try {
					CFile::Remove(lastFile);
				}catch(CFileException*)
				{ }
			}
			theApp.AddLog(m_pCamera->pItem->iid, m_lastLog.cam, m_lastLog.txt, m_lastLog.file, bReplace);
		}
		else
		{
			if (CFileDB::IsFile(log.file))
			{
				try {
					CFile::Remove(log.file);
				}
				catch (CFileException*)
				{
				}
			}
		}
	}
	
	
}

bool CVINetServerDoc::OnTimer(UINT nIDEvent)
{
	switch (nIDEvent)
	{
	case TIMER_CHECK:
		pApp->m_pFrame->KillTimer(nIDEvent);
		CheckRestart();
		pApp->m_pFrame->SetTimer(nIDEvent, 1000, 0);
		return true;
	default: break;
	}

	return false;
}


CString CVINetServerDoc::WriteImage(const CString& text, bool bLock)
{
	CSingleLock lock(&m_docLock, bLock);

	CString id = GetCfg(_T("id"));

	CString path = GetCfg(_T("path")), file;
	if (path.IsEmpty() || m_img.IsNull())
		return _T("");
	path.TrimRight('\\');
	if (path.Find(':') < 0)
		path = GetDefaultPath() + path;
	
	for (int pos = 0; pos < 1000; ++pos)
	{
		if(!pos)
			file.Format(_T("%s\\%s_%s.jpg"), path, id, text);
		else
			file.Format(_T("%s\\%s_%s_%0.3d.jpg"), path, id, text,pos);

		if (!CFileDB::IsFile(file))
			break;
	}

	if (GetTickCount() - m_imgFaceRectTime < 500 && !m_imgFaceRect.IsNull())
	{
		SaveImage(m_imgFaceRect, file);
	}
	else
		SaveImage(m_img, file);

	PurgeImage(path, false);

	return file;
}


bool CVINetServerDoc::SaveImage(CImage& img, CString path)
{
	if (img.IsNull())
		return false;
	bool ok = (img.Save(path) == S_OK);

	return ok;
}