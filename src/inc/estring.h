#pragma once

#include <list>
#include <vector>
#include <string>
#include <Wincrypt.h>
#include <time.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <afxinet.h>
#include "pugixml.hpp"


template <class T>
inline std::vector<T> explode(T str,const T& sep)
{
	std::vector<T> arr;
	arr.reserve(str.GetLength()/16);
	while(!str.IsEmpty())
	{
		int pos = str.Find(sep);
		if(pos < 0)
			break;
		arr.push_back(str.Left(pos));
		str = str.Mid(pos+sep.GetLength());
	}
	if(!str.IsEmpty())
		arr.push_back(str);
	return arr;
}

template <class T>
inline std::list<T> explode_list(T str, const T& sep)
{
	std::list<T> arr;
	while (!str.IsEmpty())
	{
		int pos = str.Find(sep);
		if (pos < 0)
			break;
		arr.push_back(str.Left(pos));
		str = str.Mid(pos + sep.GetLength());
	}
	if (!str.IsEmpty())
		arr.push_back(str);
	return arr;
}


inline CString	int2str(int v)
{
	CString str;
	str.Format(_T("%i"),v);
	return str;
}

inline CString	flt2str(double v,LPCWSTR fmt=_T("%lg"))
{
	CString str;
	str.Format(fmt,v);
	return str;
}

inline int str2int(LPCWSTR str)
{
	if( _tcslen(str) >= 2 && str[0] == '0' && str[1] == 'x')
	{
		int v;
		_stscanf_s(str+2,_T("%X"),&v);
		return v;
	}
	return _tstoi(str);
}

inline CString LocaleNumber(LPCWSTR num)
{
	CString str = num;
	static TCHAR dot = 0;

	if(!dot)
	{
		CString tmp;
		
		if(! GetNumberFormat(LOCALE_USER_DEFAULT,0,	_T("0.0"),	0,	tmp.GetBufferSetLength(10),	100) )
		{
			return str;
		}

		tmp.Remove('0');

		if(tmp.GetLength() > 0)
			dot = tmp.GetAt(0);
		else
			dot = '.';
	}

	str.Replace('.',dot);

	return str;
}

#ifdef _UNICODE
inline CStringA LocaleNumber(LPCSTR num)
{
	return CStringA( LocaleNumber( CString(num) ) );
}
#endif

inline CString LocaleNumber(double v,LPCWSTR fmt)
{
	CString str;
	str.Format(fmt,v);
	return str;
}
inline CString AddSpaces( const CString& str,int newLen,bool bRear=true,TCHAR ch=' ')
{
	int len = str.GetLength();
	if(len >= newLen)
		return str;
	if(bRear)
		return str + CString(ch,newLen-len);
	return CString(ch,newLen-len)+str;
}

inline double xatof(CString str)
{
	str.Replace(',','.');
	return _tstof(str);
}

#ifdef _UNICODE
inline double xatof(CStringA str)
{
	str.Replace(',','.');
	return atof(str);
}
#endif


inline bool estr_CryptInit(HCRYPTPROV* pc)
{
	HCRYPTPROV hCryptProv = NULL;
	LPCWSTR UserName = _T("Vibraimage10");  // name of the key container
	// to be used
	//--------------------------------------------------------------------
	// Attempt to acquire a context and a key
	// container. The context will use the default CSP
	// for the RSA_FULL provider type. DwFlags is set to zero
	// to attempt to open an existing key container.

	if(pc) *pc = 0;
	if(CryptAcquireContext(
		&hCryptProv,               // handle to the CSP
		UserName,                  // container name 
		NULL,                      // use the default provider
		PROV_RSA_FULL,             // provider type
		0))                        // flag values
	{
		// printf("A cryptographic context with the %s key container \n", UserName);
		// printf("has been acquired.\n\n");
	}
	else
	{ 
		//--------------------------------------------------------------------
		// An error occurred in acquiring the context. This could mean
		// that the key container requested does not exist. In this case,
		// the function can be called again to attempt to create a new key 
		// container. Error codes are defined in Winerror.h.
		if (GetLastError() == NTE_BAD_KEYSET)
		{
			if(CryptAcquireContext(
				&hCryptProv, 
				UserName, 
				NULL, 
				PROV_RSA_FULL, 
				CRYPT_NEWKEYSET)) 
			{
				 // printf("A new key container has been created.\n");
			}
			else
			{
				// printf("Could not create a new key container.\n");
				return false;
			}
		}
		else
		{
			//printf("A cryptographic service handle could not be acquired.\n");
			return false;
		}

	} // End of else.
		//--------------------------------------------------------------------
	// A cryptographic context and a key container are available. Perform
	// any functions that require a cryptographic provider handle.

	if(pc) *pc = hCryptProv;

	return true;
}

inline int estr_md5(const char* str,BYTE *vHash)
{
	HCRYPTPROV hCryptProv;
	HCRYPTHASH hHash;

	int hl = 0;
	if(!estr_CryptInit(&hCryptProv))
		return 0;

	if(!CryptCreateHash(hCryptProv,CALG_MD5,0,0,&hHash))
		return 0;

	DWORD hLen = 256;
	ZeroMemory(vHash,hLen);

	if(CryptHashData(hHash,(BYTE*)str,(DWORD)strlen(str),CRYPT_USERDATA))
	{
		if(CryptGetHashParam(hHash,HP_HASHVAL,vHash,&hLen,0))
		{
			hl = hLen;
		}
	}

	if(hHash) 
		CryptDestroyHash(hHash);
	if(hCryptProv) 
		CryptReleaseContext(hCryptProv,0);
	return hl;
}

inline CStringA estr_ToHex(BYTE* data, int len)
{
	std::vector<char> str;
	str.resize(len * 4+1);

	memset(&str[0], 0, str.size());

	char tmp[8];
	char *Temp = &str[0];

	for (int k = 0; k < len; ++k)
	{
		BYTE v = data[k];
		UINT u = v;
		sprintf_s(tmp, 8, "%0.2X", u);
		int k2 = k * 2;
		str[k2] = tmp[0];
		str[k2 + 1] = tmp[1];

//		strcat_s(Temp, str.size(), tmp);
	}
	return &str[0];
}

inline std::string estr_ToHexStr(BYTE* data, int len)
{
	std::vector<char> str;
	str.resize(len * 4 + 1);

	memset(&str[0], 0, str.size());

	char tmp[8];
	char *Temp = &str[0];

	for (int k = 0; k < len; ++k)
	{
		BYTE v = data[k];
		UINT u = v;
		sprintf_s(tmp, 8, "%0.2X", u);
		int k2 = k * 2;
		str[k2] = tmp[0];
		str[k2+1] = tmp[1];
	//	strcat_s(Temp, str.size(), tmp);
	}
	return &str[0];
}


inline CStringA estr_md5(const char* str)
{
	BYTE vHash[256];
	memset(vHash, 0, sizeof(vHash));
	int size = estr_md5(str, vHash);
	return estr_ToHex(vHash, size);
}

inline COleDateTimeSpan str2dts(const CString& stra)
{
	int hh = 0, mm = 0, ss = 0, pos;

	if ((pos = stra.ReverseFind(':')) >= 0)
	{
		ss = _tstoi(stra.Mid(pos + 1));
		CString str( stra.Left(pos) );

		if ((pos = str.ReverseFind(':')) >= 0)
		{
			mm = _tstoi(str.Mid(pos + 1));
			hh = _tstoi(str.Left(pos));
		}
		else
			mm = _tstoi(str.Mid(pos + 1));
	}
	else
		ss = _tstoi(stra);

	return COleDateTimeSpan(0, hh, mm, ss);
}


inline CString v2str(const CString& v) { return v; }
inline CString v2str(double v) { CString str; str.Format(_T("%g"), v); return LocaleNumber(str); }
inline CString v2str(float v) { CString str; str.Format(_T("%g"), v); return LocaleNumber(str); }
inline CString v2str(__int64 v) { CString str; str.Format(_T("%lli"), v); return str; }
inline CString v2str(int v) { CString str; str.Format(_T("%i"), v); return str; }
inline CString v2str(long v) { CString str; str.Format(_T("%i"), v); return str; }
inline CString v2str(short v) { CString str; str.Format(_T("%i"), (int)v); return str; }
inline CString v2str(char v) { CString str; str.Format(_T("%i"), (int)v); return str; }
inline CString v2str(unsigned __int64 v) { CString str; str.Format(_T("%llu"), v); return str; }
inline CString v2str(unsigned int v) { CString str; str.Format(_T("%u"), (unsigned int)v); return str; }
inline CString v2str(unsigned long v) { CString str; str.Format(_T("%u"), (unsigned int)v); return str; }
inline CString v2str(unsigned short v) { CString str; str.Format(_T("%u"), (unsigned int)v); return str; }
inline CString v2str(unsigned char v) { CString str; str.Format(_T("%u"), (unsigned int)v); return str; }
inline CString v2str(const CPoint& v) { CString str; str.Format(_T("%i;%i"), v.x, v.y); return str; }
inline CString v2str(const CSize& v) { CString str; str.Format(_T("%u;%u"), v.cx, v.cy); return str; }
inline CString v2str(const CRect& v) { CString str; str.Format(_T("%i;%i;%i;%i"), v.left, v.top, v.right, v.bottom); return str; }

inline void str2v(const CString& str, CString& v) { v = str; }
inline void str2v(const CString& str, double& v) { v = xatof(str); }
inline void str2v(const CString& str, float& v) { v = (float)xatof(str); }
inline void str2v(const CString& str, __int64& v) { v = _tstoi64(str); }
inline void str2v(const CString& str, int& v) { v = _tstoi(str); }
inline void str2v(const CString& str, long& v) { v = (long)_tstoi(str); }
inline void str2v(const CString& str, short& v) { v = (short)_tstoi(str); }
inline void str2v(const CString& str, char& v) { v = (char)_tstoi(str); }
inline void str2v(const CString& str, unsigned __int64& v) { v = (unsigned __int64)_tstoi64(str); }
inline void str2v(const CString& str, unsigned int& v) { v = (unsigned int)_tstoi(str); }
inline void str2v(const CString& str, unsigned long& v) { v = (unsigned long)_tstoi(str); }
inline void str2v(const CString& str, unsigned short& v) { v = (unsigned short)_tstoi(str); }
inline void str2v(const CString& str, unsigned char& v) { v = (unsigned char)_tstoi(str); }
inline void str2v(const CString& str, CPoint& v)
{
	std::vector<CString> arr = explode(str, CString(_T(";")));
	if (arr.size() == 2)
	{
		str2v(arr[0], v.x);
		str2v(arr[1], v.y);
	}
}
inline void str2v(const CString& str, CSize& v)
{
	std::vector<CString> arr = explode(str, CString(_T(";")));
	if (arr.size() == 2)
	{
		str2v(arr[0], v.cx);
		str2v(arr[1], v.cy);
	}
}
inline void str2v(const CString& str, CRect& v)
{
	std::vector<CString> arr = explode(str, CString(_T(";")));
	if (arr.size() == 4)
	{
		str2v(arr[0], v.left);
		str2v(arr[1], v.top);
		str2v(arr[2], v.right);
		str2v(arr[3], v.bottom);
	}
}

inline CString StrRemoveMulti(CString str, LPCTSTR rem)
{
	while (*rem)
	{
		str.Remove(*rem);
		++rem;
	}
	return str;
}

inline CString StrFileName(const CString& str)
{
	return StrRemoveMulti(str,_T("\\,/\r\n\a\t%?&*<>'`\"|"));
}

inline CString ReadURL(LPCTSTR url, LPTSTR pErrMsg = 0, int nMax = 1024)
{
	if (pErrMsg && nMax > 0)
		*pErrMsg = 0;

	CString str;
	try
	{
		CInternetSession session;
		CStdioFile *pFile = session.OpenURL(url, 1, INTERNET_FLAG_TRANSFER_BINARY | INTERNET_FLAG_DONT_CACHE);
		if (pFile)
		{
			std::vector<char> buf(65536);
			
			UINT n = (UINT)pFile->Read(&buf[0], (int)buf.size() - 1);
			str = pugi::as_utf16(&buf[0]).c_str();
			pFile->Close();
		}
	}
	catch (CInternetException *e)
	{
		if (pErrMsg && nMax > 0)
			e->GetErrorMessage(pErrMsg, nMax);
	}

	return str;

}



inline std::string UriDecode(const std::string & sSrc)
{
	// Note from RFC1630:  "Sequences which start with a percent sign
	// but are not followed by two hexadecimal characters (0-9, A-F) are reserved
	// for future extension"
	const char HEX2DEC[256] =
	{
		/*       0  1  2  3   4  5  6  7   8  9  A  B   C  D  E  F */
		/* 0 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
		/* 1 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
		/* 2 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
		/* 3 */  0, 1, 2, 3,  4, 5, 6, 7,  8, 9,-1,-1, -1,-1,-1,-1,

		/* 4 */ -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
		/* 5 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
		/* 6 */ -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
		/* 7 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,

		/* 8 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
		/* 9 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
		/* A */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
		/* B */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,

		/* C */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
		/* D */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
		/* E */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
		/* F */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1
	};

	const unsigned char * pSrc = (const unsigned char *)sSrc.c_str();
	const int SRC_LEN = (int)sSrc.length();
	const unsigned char * const SRC_END = pSrc + SRC_LEN;
	const unsigned char * const SRC_LAST_DEC = SRC_END - 2;   // last decodable '%' 

	char * const pStart = new char[SRC_LEN];
	char * pEnd = pStart;

	while (pSrc < SRC_LAST_DEC)
	{
		if (*pSrc == '%')
		{
			char dec1, dec2;
			if (-1 != (dec1 = HEX2DEC[*(pSrc + 1)])
				&& -1 != (dec2 = HEX2DEC[*(pSrc + 2)]))
			{
				*pEnd++ = (dec1 << 4) + dec2;
				pSrc += 3;
				continue;
			}
		}

		*pEnd++ = *pSrc++;
	}

	// the last 2- chars
	while (pSrc < SRC_END)
		*pEnd++ = *pSrc++;

	std::string sResult(pStart, pEnd);
	delete[] pStart;
	return sResult;
}


inline std::string UriEncode(const std::string & sSrc)
{
	// Only alphanum is safe.
	const char SAFE[256] =
	{
		/*      0 1 2 3  4 5 6 7  8 9 A B  C D E F */
		/* 0 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
		/* 1 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
		/* 2 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
		/* 3 */ 1,1,1,1, 1,1,1,1, 1,1,0,0, 0,0,0,0,

		/* 4 */ 0,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
		/* 5 */ 1,1,1,1, 1,1,1,1, 1,1,1,0, 0,0,0,0,
		/* 6 */ 0,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
		/* 7 */ 1,1,1,1, 1,1,1,1, 1,1,1,0, 0,0,0,0,

		/* 8 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
		/* 9 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
		/* A */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
		/* B */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,

		/* C */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
		/* D */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
		/* E */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
		/* F */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0
	};

	const char DEC2HEX[16 + 1] = "0123456789ABCDEF";
	const unsigned char * pSrc = (const unsigned char *)sSrc.c_str();
	const int SRC_LEN = (int)sSrc.length();
	unsigned char * const pStart = new unsigned char[SRC_LEN * 3];
	unsigned char * pEnd = pStart;
	const unsigned char * const SRC_END = pSrc + SRC_LEN;

	for (; pSrc < SRC_END; ++pSrc)
	{
		if (SAFE[*pSrc])
			*pEnd++ = *pSrc;
		else
		{
			// escape this char
			*pEnd++ = '%';
			*pEnd++ = DEC2HEX[*pSrc >> 4];
			*pEnd++ = DEC2HEX[*pSrc & 0x0F];
		}
	}

	std::string sResult((char *)pStart, (char *)pEnd);
	delete[] pStart;
	return sResult;
}


inline std::string UriEncode(BYTE *pData, int size)
{
	// Only alphanum is safe.
	const char SAFE[256] =
	{
		/*      0 1 2 3  4 5 6 7  8 9 A B  C D E F */
		/* 0 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
		/* 1 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
		/* 2 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
		/* 3 */ 1,1,1,1, 1,1,1,1, 1,1,0,0, 0,0,0,0,

		/* 4 */ 0,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
		/* 5 */ 1,1,1,1, 1,1,1,1, 1,1,1,0, 0,0,0,0,
		/* 6 */ 0,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
		/* 7 */ 1,1,1,1, 1,1,1,1, 1,1,1,0, 0,0,0,0,

		/* 8 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
		/* 9 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
		/* A */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
		/* B */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,

		/* C */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
		/* D */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
		/* E */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
		/* F */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0
	};

	const char DEC2HEX[16 + 1] = "0123456789ABCDEF";
	const unsigned char * pSrc = (const unsigned char *)pData;
	const int SRC_LEN = size;
	unsigned char * const pStart = new unsigned char[SRC_LEN * 3];
	unsigned char * pEnd = pStart;
	const unsigned char * const SRC_END = pSrc + SRC_LEN;

	memset(pStart, 0, SRC_LEN * 3);

	for (; pSrc < SRC_END; ++pSrc)
	{
		if (SAFE[*pSrc])
			*pEnd++ = *pSrc;
		else
		{
			// escape this char
			*pEnd++ = '%';
			*pEnd++ = DEC2HEX[*pSrc >> 4];
			*pEnd++ = DEC2HEX[*pSrc & 0x0F];
		}
	}

	std::string sResult((char *)pStart, (char *)pEnd);
	delete[] pStart;
	return sResult;
}
