// VINetServerDoc.h : interface of the CVINetServerDoc class
//


#pragma once

#include "ViNetCtrl.h"
#include "ViDesktopCfg.h"

class CViDesktopApp;
class CViDesktopFrame;
class CViViewCamera;


class CVINetServerDoc 
{
public:
	typedef struct tagLOG
	{
		CCriticalSection lock;
		CString cam, txt, file;
		COleDateTime tc;
		DWORD t;
		float vi;
		float vc;
	} LOG;

	typedef struct tagVARSTACK
	{
		DWORD t;
		CStringA ip;
		std::vector<int> id;
		std::vector<VI_VAR> var;
	}VARSTACK;

	CCriticalSection		m_varStackLock;
	std::list<VARSTACK>		m_varStack;
	HANDLE					m_varStackProc;
	static  DWORD WINAPI	VarThread(LPVOID p);
	UINT					VarThreadLocal();

	BOOL					m_bClose;
public:
	CViDesktopApp	*		pApp;
	VI_NET_ITEM *			m_pItem;
	CViViewCamera *			m_pCamera;

	static std::list<CVINetServerDoc *> m_lDocs;
public:
	CVINetModuleDll *					m_pvi;
	
	CCriticalSection					m_varLock;
	std::map<int, VI_VAR>				m_var;
	std::list<CString>					m_rqst;

	std::map<CString, CVINetServerDoc*>	m_ips;

	CCriticalSection					m_docLock;

	DWORD								m_imgVer;
	CImage								m_img;
	CImage								m_imgFaceRect;
	DWORD								m_imgFaceRectTime;
	CCriticalSection					m_imgSync;

	bool								m_imgLock;

	CCriticalSection					m_tmpImgLock;
	CImage								m_tmpImg;
	CString								m_tmpCaption;
	std::map<int, VI_VAR>				m_tmpVar;

	DWORD								m_varVer;


	DWORD								m_tLastData, m_tLastImage;
	bool								m_bRestarted;

	LOG									m_lastLog;

	typedef struct tagFACE_PROC
	{
		mmx_array<BYTE> img, data;
	}FACE_PROC;

	std::list<FACE_PROC> m_faceProc;
	CCriticalSection	 m_faceProcLock;
	HANDLE				 m_faceProcTh;


	// Operations
public:

	// Overrides
public:
	virtual BOOL Init();
	virtual void Close();
	virtual void Restart() { m_bRestarted = false; }
	// Implementation
public:
	CVINetServerDoc();
	virtual ~CVINetServerDoc();


protected:
	static void CallbackOnImagePack(void *UserData, VI_MODULE module, const char *ip, void *ptr, int size, void *pData, int dataSize);
	static void CallbackOnVar(void *UserData, VI_MODULE module, const char *ip, const int *idVar, const VI_VAR *pVar, int cntVar);
	void AddRqst(int id);
	bool CkeckRqst(void);
	void OnVarLocal(const char *ip, const int *idVar, const VI_VAR *pVar, int cntVar);
	void OnVarLocal(const char *ip, const int id, const VI_VAR &var);
	void OnVar(const char *ip, const int *idVar, const VI_VAR *pVar, int cntVar);
	void OnImagePack(const char* ip, void* ptr, int size, void *pData, int dataSize);
	void OnImagePackFull(const char* ip, void* ptr, int size, void *pData, int dataSize);
	void OnFace(void* ptr, int size, void *pData, int dataSize);
	void OnFace();

protected:
	typedef struct tagIMG_PACK
	{
		CStringA ip;
		std::vector<BYTE> ptr;
		std::vector<BYTE> pData;
	} IMG_PACK;
	CCriticalSection		m_imgPackLock;
	std::list< IMG_PACK >	m_imgPack;
	HANDLE					m_imgPackThread[4];


	static DWORD WINAPI OnImagePackThread(LPVOID p);
	void OnImagePackThreadLocal();
	// Generated message map functions

public:
	bool OnTimer(UINT nIDEvent);

	bool Connect(void);
	bool IsError();
	bool IsMain(void);
	CString GetCfg(LPCTSTR id);
	CVINetServerDoc* FindIP(const CString& ip);
	void OnNewVar(int id, int subID);
	virtual void SetTitle(LPCTSTR lpszTitle);

	void SendVar(int id, const VI_VAR& v);
	void SendVarI1(int id, int v);
	void SendVarF1(int id, float v);

	void SetCaption(const CString& txt, bool bProc);
	CString GetCaption();

	void AddLog(float vi, float vc);


	VI_VAR GetVar(int id);
	int GetI1(int id);
	float GetF1(int id);

	static bool DrawRect(CImage& src, CRect r, COLORREF c);

	CString FileCaption(const CString& cap);

	void CheckRestart();

	bool CheckDetector();

	void CheckLogAdd();


	static DWORD WINAPI FaceProc(LPVOID p);
	UINT FaceProcLocal();

	void PutImageFace(CImage& img);
	bool IsModule(void);

	void AddLog(LOG& log,bool bReplace);
	CString WriteImage(const CString& text, bool bLock);
	void PurgeImage(CString path, bool bLock);
	bool SaveImage(CImage& img, CString path);
};


inline VI_VAR CVINetServerDoc::GetVar(int id)
{
	CSingleLock lock(&m_varLock, TRUE);
	return m_var[id];
}

inline int CVINetServerDoc::GetI1(int id)
{
	return GetVar(id).iv1;
}

inline float CVINetServerDoc::GetF1(int id)
{
	return GetVar(id).fv1;
}
