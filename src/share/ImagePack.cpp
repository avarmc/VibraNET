#include "StdAfx.h"
#include "imagepack.h"
#include "minmaxSTL.h"

std::vector<BYTE> CImagePack::Convert(std::vector<BYTE>& img,int type)
{
	std::vector<BYTE> res;
	CImage i;
	if(! LoadImage(i,&img[0],(int)img.size()))
		return res;
	if(! SaveImage(i,res,type))
		return res;
	return res;
}


bool CImagePack::LoadImage(CImage& i, void* ptr, int size)
{
	if(!i.IsNull())
		i.Destroy();

	HGLOBAL hgl = GlobalAlloc(GHND,size);
	HRESULT hr = E_FAIL;

	if(! hgl ) return false;
	
	{
		void *p = GlobalLock(hgl);
		SSESafeMemcpy(p,ptr,size);
		GlobalUnlock(hgl);
	}

	IStream *	pStream;
	if(CreateStreamOnHGlobal(hgl,FALSE,&pStream) == S_OK && pStream)
	{
		hr = i.Load(pStream);
	}

	if(pStream)
		pStream->Release();
	GlobalFree(hgl);
	return SUCCEEDED(hr);
}

bool CImagePack::SaveImage(CImage& i, std::vector<BYTE> &img, const CString& ext)
{
	int pos1 = ext.ReverseFind('.'), pos2 = ext.ReverseFind('\\');
	CString e;
	if (ext.GetLength() == 3)
		e = ext;
	else
	{
		int pos1 = ext.ReverseFind('.'), pos2 = ext.ReverseFind('\\');
		if (pos1 < 0 || pos2 > pos1)
			return false;
		e = ext.Mid(pos1 + 1);
	}
	e.MakeLower();
	int type = BMP;
	if(e == "png") type = PNG;
	else
	if(e == "jpg") type = JPG;
	else
	if(e == "gif") type = GIF;

	return SaveImage(i,img,type);
}

bool CImagePack::SaveImage(CImage& i, std::vector<BYTE> &img, int type)
{
	if(i.IsNull()) return false;

	int w = i.GetWidth();
	int h = i.GetHeight();
	int bpp = i.GetBPP();

	int size = 0; //(w*h*bpp/8+1024)*4;

	HGLOBAL hgl = GlobalAlloc(GHND,size);
	if(! hgl ) return false;

	bool ok = false;
	IStream *	pStream;
	if(CreateStreamOnHGlobal(hgl,FALSE,&pStream) == S_OK && pStream)
	{
		GUID g;
		switch(type)
		{
		default:
		case BMP: g =(Gdiplus::ImageFormatBMP); break;
		case JPG: g =(Gdiplus::ImageFormatJPEG); break;
		case PNG: g =(Gdiplus::ImageFormatPNG); break;
		case GIF: g =(Gdiplus::ImageFormatGIF); break;
		}

		HRESULT hr = i.Save(pStream, g );
		if(hr == S_OK)
		{
			STATSTG stat;
			pStream->Stat(&stat,1);
			img.resize(stat.cbSize.LowPart);

			void *p = GlobalLock(hgl);
			memcpy(&img[0],p,img.size());
			GlobalUnlock(hgl);
			ok = (!img.empty());
		}

		pStream->Release();
	}
	GlobalFree(hgl);
	return ok;
}

std::vector<BYTE> CImagePack::Downsize(std::vector<BYTE>& i, int w, int h, int type)
{
	if(i.empty())
		return std::vector<BYTE>();
	CImage img;
	if(! LoadImage(img,&i[0], (int)i.size()) )
		return std::vector<BYTE>();

	return Downsize(img,w,h,type);
}

std::vector<BYTE> CImagePack::Downsize(CImage& i, int w, int h, int type)
{
	if(i.IsNull())
		return std::vector<BYTE>();

	int sw = i.GetWidth();
	int sh = i.GetHeight();
	
	if( sw * h > sh * w )
		w = h * sw / sh;
	else
	if( sw * h < sh * w )
		h = w * sh / sw;

	CImage ds;
	ds.Create(w,h,24);

	if(ds.IsNull())
		return std::vector<BYTE>();

	CImageDC idc(ds);
	
	::SetStretchBltMode(idc,COLORONCOLOR);

	i.Draw(idc,CRect( CPoint(0,0), CSize(w,h) ) );

	std::vector<BYTE> pack;

	if(! SaveImage(ds,pack,type) )
		return std::vector<BYTE>();

	return pack;
}

std::vector<BYTE> CImagePack::Pack2Size(CImage& img, int size, bool bw)
{
	std::vector<BYTE> tmp;
	int w = img.GetWidth();
	int h = img.GetHeight();

	SaveImage(img,tmp,BMP);

	CImage img2;
	LoadImage(img2,&tmp[0], (int)tmp.size());
	if(bw)
	{
		COLORREF c;
		RGBQUAD &rgb = *(RGBQUAD *)&c;

		int w = img2.GetWidth();
		int h = img2.GetHeight();
		for(int y = 0; y < h; ++y)
		for(int x = 0; x < w; ++x)
		{
			 c = img2.GetPixel(x,y);
			 BYTE m = max( rgb.rgbBlue, max(rgb.rgbGreen,rgb.rgbRed) );
			 rgb.rgbBlue = rgb.rgbGreen = rgb.rgbRed = m;
			 img2.SetPixel(x,y,c);
		}

	}

	tmp.clear();

	while(1)
	{
		std::vector<BYTE> tmpJPG,tmpPNG;
		tmpJPG = CImagePack::Downsize(img2,w,h,CImagePack::JPG);
		tmpPNG = CImagePack::Downsize(img2,w,h,CImagePack::PNG);

		int sJPG = (int)tmpJPG.size();
		int sPNG = (int)tmpPNG.size();
		
		if( sPNG <= size )
			tmp = tmpPNG;
		if( sJPG <= size )
			tmp = tmpJPG;
		else
			tmp = ( sJPG < sPNG ) ? tmpJPG : tmpPNG;

		if(((int)tmp.size()) <= size || w < 64 || h < 48)
			break;

		w -= 10, h = w*img.GetHeight()/img.GetWidth();
	}

	return tmp;
}


BYTE* CImagePack::PrepareImage(CImage& img, int w, int h, int bpp)
{
	if(img.IsNull() && (w < 0 || h < 0 || bpp < 0))
		return 0;

	if(w < 0) w = img.GetWidth();
	if(h < 0) h = img.GetHeight();
	if(bpp < 0) bpp = img.GetBPP();

	if(!img.IsNull() && (w != img.GetWidth() || h != img.GetHeight() || bpp != img.GetBPP() ))
		img.Destroy();
	if(!w || !h || !bpp)
		return 0;
	if(img.IsNull())
	{
		img.Create(w,h,bpp);
		if(bpp == 8)
		{
			RGBQUAD pal[256];
			for(int i = 0; i < 256; ++i)
				pal[i].rgbBlue = pal[i].rgbGreen = pal[i].rgbRed = (BYTE)i,pal[i].rgbReserved = 0;
			img.SetColorTable(0,256,pal);
		}
	}
	int pitch = img.GetPitch();
	BYTE *pb = (BYTE*) img.GetBits();
	if(pitch < 0)
		pb += pitch*(h-1);
	return pb;
}

BYTE* CImagePack::CopyImage(CImage& imgFrom,CImage& imgTo)
{
	if(imgFrom.IsNull())
	{
		if(!imgTo.IsNull())
			imgTo.Destroy();
		return 0;
	}
	int w = imgFrom.GetWidth();
	int h = imgFrom.GetHeight();
	int bpp = imgFrom.GetBPP();
	int pitch = imgFrom.GetPitch();

	BYTE *pF = PrepareImage(imgFrom,w,h,bpp);
	BYTE *pT = PrepareImage(imgTo,w,h,bpp);

	SSESafeMemcpy(pT,pF,abs(pitch)*h);
	return pT;
}

CSize CImagePack::GetFileInfo(LPCWSTR fileName,int *pbpp,int *ppitch)
{
	CImage img;
	
	if(img.Load(fileName) == S_OK)
	{
		if(pbpp) *pbpp = img.GetBPP();
		if(ppitch) *ppitch = img.GetPitch();
		return CSize( img.GetWidth(), img.GetHeight() );
	}

	if(pbpp) *pbpp = 0;
	if(ppitch) *ppitch = 0;
	return CSize(0,0);
}

bool CImagePack::Pack(void *ptr,std::vector<BYTE>& i, int w, int h,int bpp, int type)
{
	CImage img;
	BYTE * p = PrepareImage(img,w,h,bpp);
	if(!p)
		return false;
	
	SSESafeMemcpy(p,ptr,w*h*bpp/8);

	return SaveImage(img,i,type);
}


bool CImagePack::SubImage(CImage& src,CImage& dst,CRect r,float scale,int nbpp)
{
	if(!dst.IsNull())
		dst.Destroy();

	if(src.IsNull())
		return false;

	LONG w = src.GetWidth();
	LONG h = src.GetHeight();

	r.left = min(w,max((LONG)0, r.left));
	r.right = min(w, max((LONG)0, r.right));
	r.top = min(h, max((LONG)0, r.top));
	r.bottom = min(h, max((LONG)0, r.bottom));

	int sw = src.GetWidth();
	int sh = src.GetHeight();

	int nw = (int)(r.Width()*scale+0.5f);
	int nh = (int)(r.Height()*scale+0.5f);
	if(nbpp <= 0)
		nbpp = src.GetBPP();

	if(!dst.Create(nw,nh,nbpp))
		return false;

	if(nbpp == 8)
	{
		RGBQUAD pal[256];
		src.GetColorTable(0,256,pal);
		dst.SetColorTable(0,256,pal);
	}

	CImageDC idc(dst);
	SetStretchBltMode(idc,COLORONCOLOR);
	src.Draw(idc,CRect(0,0,nw,nh),r);

	return true;
}

bool CImagePack::ReplaceColor(CImage& img,COLORREF c1,COLORREF c2)
{
	if(img.IsNull())
		return false;

	int w = img.GetWidth();
	int h = img.GetHeight();

	for(int y = 0; y < h; ++y)
	for(int x = 0; x < w; ++x)
		if(img.GetPixel(x,y) == c1)
			img.SetPixel(x,y,c2);

	return true;
}