#pragma once

#include <atlimage.h>

class CImagePack
{
public:
	enum TYPES { BMP,JPG,PNG,GIF };
public:
	static std::vector<BYTE> Convert(std::vector<BYTE>& img,int type);
	static bool LoadImage(CImage& i, void* ptr, int size);
	static bool SaveImage(CImage& i, std::vector<BYTE> &img, int type);
	static bool SaveImage(CImage& i, std::vector<BYTE> &img, const CString& ext);
	static std::vector<BYTE> Downsize(CImage& i, int w, int h, int type = JPG);
	static std::vector<BYTE> Downsize(std::vector<BYTE>& i, int w, int h, int type = JPG);
	static std::vector<BYTE> Pack2Size(CImage& img, int size, bool bw);
	static BYTE* PrepareImage(CImage& img, int w, int h, int bpp);
	static BYTE* CopyImage(CImage& imgFrom,CImage& imgTo);
	static CSize GetFileInfo(LPCWSTR fileName,int *pbpp=0,int *ppitch=0);
	static bool Pack(void *ptr,std::vector<BYTE>& i, int w, int h,int bpp, int type = PNG);
	static bool SubImage(CImage& src,CImage& dst,CRect r,float scale=1,int nbpp=-1);
	static bool ReplaceColor(CImage& img,COLORREF c1,COLORREF c2);
};
