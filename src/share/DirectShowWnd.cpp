// DirectShowWnd.cpp : implementation file
//

#include "stdafx.h"
#include "DirectShowWnd.h"
#include "ImagePack.h"

// CDirectShowWnd

//IMPLEMENT_DYNAMIC(CDirectShowWnd, CWnd)
IMPLEMENT_DYNCREATE(CDirectShowWnd, CWnd)

CDirectShowWnd::CDirectShowWnd()
{
	m_lpDDraw1 = 0;
	m_lpDDraw7 = 0;
	m_lpPrimarySurface = 0;
	m_lpClipper = 0;
	m_lpBackSurface = 0;
	m_dislpayRC = CRect(0,0,0,0);
	m_dislpayBPP = 0;
	m_imgSize = CSize(0,0);
	m_imgBpp = 0;
	m_bCompatMode = false;
}

CDirectShowWnd::~CDirectShowWnd()
{
	DDrawDone();
}


BEGIN_MESSAGE_MAP(CDirectShowWnd, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_PAINT()
END_MESSAGE_MAP()



// CDirectShowWnd message handlers



bool CDirectShowWnd::DDrawInit(void)
{
	CSingleLock lock(&m_lock,true);

	HRESULT hr;

	DDrawDone();

	if(m_bCompatMode)
		return false;

	hr = DirectDrawCreate(NULL, &(m_lpDDraw1), NULL);
    if (hr != DD_OK)
        return DDrawDone();

	hr = m_lpDDraw1->QueryInterface( IID_IDirectDraw7, (void**)&m_lpDDraw7 );
    if (hr != DD_OK)
        return DDrawDone();

	m_lpDDraw7->SetCooperativeLevel( GetSafeHwnd(), DDSCL_NORMAL );

	if(! DDrawDetectDisplayMode() )
        return DDrawDone();

	if(!DDrawCreateFlippingSurface())
        return DDrawDone();

    return TRUE;
}

bool CDirectShowWnd::DDrawDetectDisplayMode()
{
	DDSURFACEDESC2 desc;
	ZeroMemory( &desc, sizeof(desc) );
	desc.dwSize=sizeof(desc);
	if (m_lpDDraw7->GetDisplayMode( &desc )!=DD_OK)
		return FALSE;

	m_dislpayRC.left=0;
	m_dislpayRC.top=0;
	m_dislpayRC.right=desc.dwWidth;
	m_dislpayRC.bottom=desc.dwHeight;
	m_dislpayBPP=desc.ddpfPixelFormat.dwRGBBitCount;

	return TRUE;
}

bool CDirectShowWnd::DDrawCreateFlippingSurface(void)
{
	HRESULT hr;

	CRect rc;
	GetClientRect(&rc);
	ClientToScreen(&rc);

	DDSURFACEDESC2 desc;
	ZeroMemory(&desc,sizeof( desc ));
	desc.dwSize = sizeof( desc );
	desc.dwFlags = DDSD_CAPS;
	desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

	hr=m_lpDDraw7->CreateSurface( &desc, &m_lpPrimarySurface, 0 );
	if (hr!=DD_OK)
		return FALSE;

	hr=m_lpDDraw7->CreateClipper( 0, &m_lpClipper, 0 );
	if (hr!=DD_OK)
		return FALSE;

	hr=m_lpClipper->SetHWnd( 0, GetSafeHwnd() );
	if (hr!=DD_OK)
		return FALSE;

	hr=m_lpPrimarySurface->SetClipper( m_lpClipper );
	if (hr!=DD_OK)
		return FALSE;

	return TRUE;
}

bool CDirectShowWnd::DDrawSetSize(int w,int h,int bpp)
{
	HRESULT hr;
	if(!m_lpDDraw7)
		return false;

	if(m_lpBackSurface && (m_imgBpp != bpp || m_imgSize.cx != w || m_imgSize.cy != h))
	{
		m_lpBackSurface->Release();
		m_lpBackSurface = 0;
		m_imgBpp = m_imgSize.cx = m_imgSize.cy = 0;
	}
	if(!w || !h || !bpp)
		return true;

	if(!m_lpBackSurface)
	{
		DDSURFACEDESC2 desc;
		ZeroMemory( &desc, sizeof(desc) );
		desc.dwSize = sizeof(desc);
		desc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_PIXELFORMAT;
		desc.dwWidth = w;
		desc.dwHeight = h;
		desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;
		desc.ddpfPixelFormat.dwSize = sizeof(desc.ddpfPixelFormat);
		desc.ddpfPixelFormat.dwFlags = DDPF_RGB;
		desc.ddpfPixelFormat.dwRGBBitCount = bpp;
		desc.ddpfPixelFormat.dwRBitMask = 0x00ff0000;
		desc.ddpfPixelFormat.dwGBitMask = 0x0000ff00;
		desc.ddpfPixelFormat.dwBBitMask = 0x000000ff;

		hr=m_lpDDraw7->CreateSurface( &desc, &m_lpBackSurface, 0 );
		if (hr!=DD_OK || !m_lpBackSurface)
		{
			desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
			hr=m_lpDDraw7->CreateSurface( &desc, &m_lpBackSurface, 0 );
			if (hr!=DD_OK || !m_lpBackSurface)
				return FALSE;
		}
	
		m_imgBpp = bpp;
		m_imgSize.cx = w;
		m_imgSize.cy = h;
	}
	else
	{
		m_imgBpp = bpp;
		m_imgSize.cx = w;
		m_imgSize.cy = h;
	}

	return true;
}

bool CDirectShowWnd::PutImage(void *pImg, int w, int h, int bpp, int pitch,bool bRedraw)
{
	CSingleLock lock(&m_lock,true);

	int ah = abs(h);
	if(m_bCompatMode)
	{
		if(m_lpDDraw1)
			DDrawDone();

		CImagePack::PrepareImage(m_imgCompat,w,h,bpp);
		void *ptr = m_imgCompat.GetBits();
		int cpitch = m_imgCompat.GetPitch();
		if(pImg)
		{
			int size = w*bpp/8;
			for(int y = 0; y < ah; ++y)
			{
				void *pi = ((BYTE*)pImg)+ (h>0? y : (ah-1-y))*pitch;
				void *ps = ((BYTE*)ptr)+y*cpitch;

				SSEmemcpy(ps,pi,size);
			}
		}
		m_imgSize = CSize(w,h);
		if(bRedraw)
		{
#ifdef _DEBUG_CHECK_REDRAW
		_DEBUG_CHECK_REDRAW;
#endif
			RedrawWindow(0,0,RDW_INVALIDATE|RDW_UPDATENOW);
		}

		return true;
	}

	HRESULT hr;
	if(! DDrawSetSize(w,ah,32) )
		return false;

	if(!m_lpBackSurface)
		return false;

	if(!pImg)
		return true;

	DDSURFACEDESC2 ddsd;
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);

	CRect rc(0,0,w,ah);
	hr = m_lpBackSurface->Lock(&rc, &ddsd, DDLOCK_SURFACEMEMORYPTR, NULL);

	if(hr != DD_OK)
		return false;

	if(bpp == 32)
	{
		int size = w*bpp/8;
		for(int y = 0; y < ah; ++y)
		{
			void *pi = ((BYTE*)pImg)+ (h>0? y : (ah-1-y))*pitch;
			void *ps = ((BYTE*)ddsd.lpSurface)+y*ddsd.lPitch;

			SSEmemcpy(ps,pi,size);
		}
	} else
	if(bpp == 24)
	{
		for(int y = 0; y < ah; ++y)
		{
			RGBTRIPLE *pi = (RGBTRIPLE *)(((BYTE*)pImg)+ (h>0? y : (ah-1-y))*pitch);
			RGBQUAD *ps = (RGBQUAD *)(((BYTE*)ddsd.lpSurface)+y*ddsd.lPitch), *pe = ps+w;
			while(ps != pe)
			{
				ps->rgbBlue = pi->rgbtBlue;
				ps->rgbGreen = pi->rgbtGreen;
				ps->rgbRed = pi->rgbtRed;
				ps->rgbReserved = 0;

				++ps;
				++pi;
			}
		}
	}

	hr = m_lpBackSurface->Unlock(&rc);

	if(bRedraw)
	{
		if(!DDrawDraw(false))
			return false;
	}
	return true;
}

bool CDirectShowWnd::Lock(CDirectShowWndLockPtr *pLock)
{
	HRESULT hr;

	if(!pLock->size.cx || !pLock->size.cy)
		pLock->size.cx = m_imgSize.cx, pLock->size.cy = m_imgSize.cy;
	else
	if(! DDrawSetSize(pLock->size.cx,pLock->size.cy,pLock->bpp) )
		return false;

	if(!m_imgSize.cx || !m_imgSize.cy)
		return false;

	if(!m_lpBackSurface)
		return false;

	DDSURFACEDESC2 ddsd;
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);

	pLock->rc = CRect(0,0,pLock->size.cx,pLock->size.cy);
	hr = m_lpBackSurface->Lock(&(pLock->rc), &ddsd, DDLOCK_SURFACEMEMORYPTR, NULL);
	
	pLock->ptr = (BYTE*)ddsd.lpSurface;
	pLock->pitch = ddsd.lPitch;

	return (hr == DD_OK);
}

bool CDirectShowWnd::Unlock(CDirectShowWndLockPtr *pLock,bool bRedraw)
{
	HRESULT hr;
	if(!m_lpBackSurface)
		return false;
	hr = m_lpBackSurface->Unlock(&(pLock->rc));
	pLock->rc = CRect(0,0,0,0);
	pLock->ptr = 0;
	pLock->pitch = 0;
	if(hr != DD_OK)
		return false;

	if(bRedraw)
		return DDrawDraw();

	return true;
}

bool CDirectShowWnd::PutImage(CImage& img,bool bRedraw)
{
	if(img.IsNull())
		return false;
	int w = img.GetWidth();
	int h = img.GetHeight();
	int bpp = img.GetBPP();
	int pitch = img.GetPitch();
	void *p = img.GetBits();
	
	return PutImage(p,w,h,bpp,pitch,bRedraw);
}

bool CDirectShowWnd::DDrawDone(void)
{
	if (m_lpPrimarySurface)
		m_lpPrimarySurface->Release();
	m_lpPrimarySurface = 0;

	if(m_lpClipper)
		m_lpClipper->Release();
	m_lpClipper = 0;

	if(m_lpBackSurface)
		m_lpBackSurface->Release();
	m_lpBackSurface = 0;

	if (m_lpDDraw7)
		m_lpDDraw7->Release();
	m_lpDDraw7 = 0;

	if (m_lpDDraw1)
		m_lpDDraw1->Release();
	m_lpDDraw1 = 0;
	return false;
}

bool CDirectShowWnd::DDrawDraw(bool bLock)
{
	CSingleLock lock(&m_lock,bLock);

	HRESULT hr;

	if(!m_lpPrimarySurface || ! m_lpBackSurface)
		return false;

	hr = m_lpPrimarySurface->Restore();
	if(hr != DD_OK)
		return false;

	hr = m_lpBackSurface->Restore();
	if(hr != DD_OK)
		return false;

	LPDIRECTDRAWSURFACE7 destsurf=m_lpPrimarySurface;
	LPDIRECTDRAWSURFACE7 srcsurf=m_lpBackSurface;

	BOOL use_fastblt=false;

	DDSURFACEDESC2 destsurfdesc;
	ZeroMemory( &destsurfdesc, sizeof(destsurfdesc) );
	destsurfdesc.dwSize = sizeof(destsurfdesc);
	destsurf->GetSurfaceDesc( &destsurfdesc );

	CRect destrect;
	GetClientRect(&destrect);
	ClientToScreen(&destrect);

	DDSURFACEDESC2 srcsurfdesc;
	ZeroMemory( &srcsurfdesc, sizeof(srcsurfdesc) );
	srcsurfdesc.dwSize = sizeof(srcsurfdesc);
	srcsurf->GetSurfaceDesc( &srcsurfdesc );

	CRect srcrect;
	srcrect.left=0;
	srcrect.top=0;
	srcrect.right=m_imgSize.cx;
	srcrect.bottom=m_imgSize.cy;

	CRect dr = destrect;
	UINT flags=DDBLT_WAIT;
	hr = destsurf->Blt( &dr, srcsurf, &srcrect, flags, 0 );
	if(hr == DD_OK)
		return true;
	return false;
}

int CDirectShowWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	DDrawInit();

	return 0;
}

void CDirectShowWnd::OnDestroy()
{
	CSingleLock lock(&m_lock,true);
	DDrawDone();
	CWnd::OnDestroy();
}

void CDirectShowWnd::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	if(!IsWindow(m_hWnd))
		return;

	DDrawInit();
}

BOOL CDirectShowWnd::Create(LPCWSTR lpszClassName,LPCWSTR lpszWindowName,DWORD dwStyle,const RECT& rect,CWnd* pParentWnd,UINT nID,CCreateContext* pContext)
{
	return CWnd::Create(lpszClassName,lpszWindowName,dwStyle,rect,pParentWnd,nID,pContext);
}


HDC	CDirectShowWnd::GetDC()
{
	if(!m_lpPrimarySurface)
		return 0;
	HDC hdc;
	HRESULT hr = m_lpPrimarySurface->GetDC(&hdc);
	if(hr != DD_OK)
		return 0;
	return hdc;
}

HDC	CDirectShowWnd::GetBkDC()
{
	if(!m_lpBackSurface)
		return 0;
	HDC hdc;
	HRESULT hr = m_lpBackSurface->GetDC(&hdc);
	if(hr != DD_OK)
		return 0;
	return hdc;
}

void	CDirectShowWnd::ReleaseDC(HDC hDC)
{
	if(m_lpPrimarySurface)
		m_lpPrimarySurface->ReleaseDC(hDC);
}

void	CDirectShowWnd::ReleaseBkDC(HDC hDC)
{
	if(m_lpBackSurface)
		m_lpBackSurface->ReleaseDC(hDC);
}

BOOL CDirectShowWnd::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.dwExStyle |= WS_EX_TRANSPARENT;

	return CWnd::PreCreateWindow(cs);
}

LRESULT CDirectShowWnd::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_DISPLAYCHANGE:
		DDrawInit();
		break;
	case WM_POWERBROADCAST:
	{
		if (wParam != PBT_POWERSETTINGCHANGE || !lParam)
			break;
		POWERBROADCAST_SETTING *ps = (POWERBROADCAST_SETTING*)lParam;
		if (ps->PowerSetting == GUID_MONITOR_POWER_ON
			|| ps->PowerSetting == GUID_CONSOLE_DISPLAY_STATE
			|| ps->PowerSetting == GUID_SESSION_DISPLAY_STATUS)
		{
			DDrawInit();
		}
		break;
	}
	case WM_DSW_LOCK:
		return Lock((CDirectShowWndLockPtr*)lParam) ? TRUE:FALSE;
	case WM_DSW_UNLOCK:
		return Unlock((CDirectShowWndLockPtr*)lParam,true) ? TRUE:FALSE;
	case WM_LBUTTONDBLCLK:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDBLCLK:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDBLCLK:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEMOVE:
		{
			CWnd *pPWnd = GetParent();
			if(!pPWnd)
				return TRUE;

			CPoint p(LOWORD(lParam),HIWORD(lParam));
			ClientToScreen(&p);
			pPWnd->ScreenToClient(&p);
			return pPWnd->SendMessage(message,wParam,(p.x&0xFFFFF)|(p.y<<16));
		}
	case WM_MOUSEWHEEL:
			return GetParent()->SendMessage(message,wParam,lParam);
	default: 
		break;
	}

	return CWnd::WindowProc(message, wParam, lParam);
}

bool	CDirectShowWnd::ClearImg()
{
	if(!IsImage())
		return false;
	CDirectShowWndLockPtr ptr;
	ZeroMemory(&ptr,sizeof(ptr));
	if(!Lock(&ptr))
		return false;
	BYTE *p = (BYTE *)ptr.ptr;
	int size = ptr.size.cx*ptr.bpp/8;
	for( int y = 0; y < ptr.size.cy; ++y )
		SSEmemset(p+y*ptr.pitch,0,size);

	Unlock(&ptr);
	return true;
}

bool	CDirectShowWnd::ExportImg(CImage& img)
{
	if(!img.IsNull())
		img.Destroy();
	if(!IsImage())
		return false;
	CDirectShowWndLockPtr ds;
	ZeroMemory(&ds,sizeof(ds));
	if(!Lock(&ds))
		return false;

	img.Create(m_imgSize.cx,m_imgSize.cy,m_imgBpp);
	int pitch = img.GetPitch(), lsize = m_imgSize.cx*m_imgBpp/8;
	BYTE *pi = (BYTE *)img.GetBits();
	BYTE *pd = (BYTE*)ds.ptr;

	for( int y = 0; y < m_imgSize.cy; ++y)
	{
		SSEmemcpy(pi + pitch*y,pd + ds.pitch*y,lsize);
	}
	Unlock(&ds);
	return true;
}


bool	CDirectShowWnd::LP2DP(RECT *p)
{
	if(!LP2DP(p->left,p->top))
		return false;
	if(!LP2DP(p->right,p->bottom))
		return false;
	return true;
}
bool	CDirectShowWnd::LP2DP(long &x,long &y)
{
	if(!IsImage() || !m_hWnd)
		return false;
	CRect rc;
	GetClientRect(&rc);
	x = x * m_imgSize.cx / rc.Width();
	y = y * m_imgSize.cy / rc.Height();
	return true;
}

void CDirectShowWnd::OnPaint()
{
	CPaintDC dc(this); 
	if(!m_bCompatMode || m_imgCompat.IsNull() )
		return;

	CRect r;
	GetClientRect(&r);
	dc.SetStretchBltMode(COLORONCOLOR);
	m_imgCompat.Draw(dc,r);

}
