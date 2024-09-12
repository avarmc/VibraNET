#pragma once

typedef struct tagCDirectShowWndLockPtr
{
	BYTE*				ptr;
	BYTE*				ptr2;
	int					pitch;
	SIZE				size;
	RECT				rc;
	int					bpp;
} CDirectShowWndLockPtr;

#define WM_DSW_LOCK		(WM_USER+1)
#define WM_DSW_UNLOCK		(WM_USER+2)

class CDirectShowWndLock
{
public:
	CDirectShowWndLock();
	CDirectShowWndLock(HWND hWnd,int w,int h, int bpp=32);
	virtual ~CDirectShowWndLock();

	BOOL Lock(HWND hWnd,int w,int h, int bpp=32);
	BOOL Unlock();
	void SetPtr(void *p,int w, int h, int bpp=32);
public:
	CDirectShowWndLockPtr	m_ptr;
	HWND					m_hWnd;
};

inline BOOL CDirectShowWndLock::Lock(HWND hWnd,int w,int h, int bpp)
{
	ZeroMemory(&m_ptr,sizeof(m_ptr));
	m_ptr.size.cx = w;
	m_ptr.size.cy = h;
	m_ptr.bpp	= bpp;
	m_hWnd = hWnd;
	return (BOOL)SendMessage(m_hWnd,WM_DSW_LOCK,0,(LPARAM)&m_ptr);
}
inline CDirectShowWndLock::CDirectShowWndLock(HWND hWnd,int w,int h, int bpp)
{
	Lock(hWnd,w,h,bpp);
}

inline CDirectShowWndLock::CDirectShowWndLock()
{
	ZeroMemory(&m_ptr,sizeof(m_ptr));
	m_hWnd = 0;
}

inline BOOL CDirectShowWndLock::Unlock()
{
	if(m_hWnd && m_ptr.ptr)
		return (BOOL)SendMessage(m_hWnd,WM_DSW_UNLOCK,0,(LPARAM)&m_ptr);
	return FALSE;
}
inline CDirectShowWndLock::~CDirectShowWndLock()
{
	Unlock();
}


inline void CDirectShowWndLock::SetPtr(void *p,int w, int h, int bpp)
{
	m_ptr.bpp = bpp;
	m_ptr.size.cx = w;
	m_ptr.size.cy = h;
	m_ptr.pitch = w*bpp/8;
	m_ptr.rc.top = m_ptr.rc.left = 0;
	m_ptr.rc.right = w;
	m_ptr.rc.bottom = h;
}
