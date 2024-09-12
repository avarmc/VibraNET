#include "StdAfx.h"
#include "filedb.h"
#include "shellapi.h"

CFileDB::CFileDB(CString path) :
	m_bNewRecord(FALSE)
{
	SetPath(path);
}

CFileDB::~CFileDB(void)
{
}

void CFileDB::PackString(CString& str)
{
	str.Replace(_T("<"),_T("&lt;"));
	str.Replace(_T(">"),_T("&gt;"));
}

void CFileDB::UnpackString(CString& str)
{
	str.Replace(_T("&lt;"),_T("<"));
	str.Replace(_T("&gt;"),_T(">"));
}

void CFileDB::ListDir(const CString& path, bool bDir, std::list<CString>& result,bool bClear)
{
	if(bClear)
		result.clear();
	CFileFind ff;
	BOOL ok = ff.FindFile(path);
	while(ok)
	{
		ok = ff.FindNextFile();
		if(ff.IsDots()) continue;
		if( bDir && !ff.IsDirectory()) continue;
		if(!bDir && ff.IsDirectory()) continue;
		result.push_back(ff.GetFilePath());
	}
	ff.Close();
}

BOOL CFileDB::WriteMap(MAP& m, CString file)
{
	CFile f;
	if(! f.Open(file,CFile::modeCreate|CFile::modeWrite))
		return FALSE;
	MAP::iterator i;
	mmx_array<BYTE> tmp(65536);
	CArchive ar(&f,CArchive::store,(int)tmp.size(),tmp.p);
	WriteString(ar, CString( (xml_document::xml_header(true)+"\r\n").c_str() ));
	WriteString(ar,_T("<data>\r\n"));

	for(i = m.begin(); i != m.end(); ++i)
	{
		CString str;
		CString key1,key2;
		CString value = i->second;
		PackString(value);
		key1.Format(_T("<%s>"),i->first);
		key2.Format(_T("</%s>\r\n"),i->first);

		WriteString(ar,key1);
		WriteString(ar,value);
		WriteString(ar,key2);
	}
	WriteString(ar,_T("</data>\r\n"));

	ar.Close();
	if(f.m_hFile != CFile::hFileNull)
		f.Close();
	return TRUE;
}

BOOL CFileDB::ReadMap(MAP& m, CString file)
{
	m.clear();

	HRESULT hr;
	VARIANT_BOOL vok;
	bool ret = true;

	CComPtr<IXMLDOMDocument> doc;
	IXMLDOMElement*       pRoot;
	IXMLDOMNodeList*      pXMLNodeList;

	hr = doc.CoCreateInstance(__uuidof(DOMDocument));
	if(FAILED(hr))
		return FALSE;
	vok = false;
	hr = doc->load(CComVariant(file),&vok);
	if(FAILED(hr) || !vok)
		return FALSE;

	hr = doc->get_documentElement(&pRoot);
	if(SUCCEEDED(hr))
	{
		hr = pRoot->get_childNodes(&pXMLNodeList);
		if(SUCCEEDED(hr))
		{
			ReadMap(m,pXMLNodeList);
			pXMLNodeList->Release();
		}
	}
	pRoot->Release();
		
	return TRUE;
}

BOOL CFileDB::ReadMap(MAP& m, IXMLDOMNodeList* pXMLNodeList)
{
	HRESULT hr;
	long len = 0;
	hr = pXMLNodeList->get_length(&len);
	if( FAILED(hr) )
		return FALSE;
	for( long k = 0; k < len; ++k)
	{
		IXMLDOMNode *pXMLNode;
		hr = pXMLNodeList->nextNode(&pXMLNode);
		if(FAILED(hr))
			break;
		CComBSTR varNodeName;
		hr = pXMLNode->get_nodeName(&varNodeName);
		if(SUCCEEDED(hr))
		{
			DOMNodeType t;
			hr = pXMLNode->get_nodeType(&t);
			if(SUCCEEDED(hr) && t == NODE_ELEMENT)
			{
				IXMLDOMNode* pVar;
				hr = pXMLNode->get_firstChild(&pVar);
				if(hr == S_OK)
				{
					CComVariant           varNodeValue;
					hr = pVar->get_nodeValue(&varNodeValue);
					if (SUCCEEDED(hr) && varNodeValue.vt == VT_BSTR)
					{
						CString name(varNodeName);
						CString v(varNodeValue.bstrVal);
						UnpackString(v);
						m[name] = v;
					}
					pVar->Release();
				}
			}
			pXMLNode->Release();
		}
	}

	return TRUE;
}

CString CFileDB::AddNew(CString id)
{
	CString strTS14 = id.IsEmpty()?NewID():id;

	m_CurrentRecordMAP.clear();
	m_bNewRecord = TRUE;

	AddRaw(_T("ID"),strTS14);
	
	return strTS14;
}	

BOOL CFileDB::Update(void)
{
//	if(! m_bNewRecord) return FALSE;

	MAP::iterator id = m_CurrentRecordMAP.find(_T("ID"));
	if(id == m_CurrentRecordMAP.end())
	{
		m_bNewRecord = FALSE;
		return FALSE;
	}

	CString path = MakeFilePath(GetPath(),id->second);
	int status = GetFileStatus(path);
	if(status == 1) CFile::Remove(path);
	if(status == 2 && m_bNewRecord)
		Remove(id->second);
	m_bNewRecord = FALSE;
	if(GetFileStatus(path) != 2 && ! ::CreateDirectory(path,NULL))
			return FALSE;

	return WriteMap(m_CurrentRecordMAP,MakeFilePath(path,_T("db.info")));
}

BOOL CFileDB::Remove(CString id)
{
	SetCurrentDirectory(m_path);
	return RmDir(MakeFilePath(GetPath(),id));
}

CString CFileDB::MakeFilePath(CString path, CString file)
{
	if(path.IsEmpty()) return file;
	if(path.Right(1) == _T("\\"))
		return path + file;
	return path + _T("\\") + file;
}

int CFileDB::GetFileStatus(const CString& file)
{
	CFileStatus st;
	if(! CFile::GetStatus(file,st)) return 0;
	if((st.m_attribute & 0x18) != 0) return 2;
	return 1;
}

bool CFileDB::RmDir( const CString& csDeleteFolderPath_i )
{ 
	// Making the directory name double null terminated
	int nFolderPathLen = csDeleteFolderPath_i.GetLength();
	TCHAR *pszFrom = new TCHAR[nFolderPathLen + 2];
	_tcscpy_s(pszFrom, nFolderPathLen + 2, csDeleteFolderPath_i);
	pszFrom[nFolderPathLen] = 0;
	pszFrom[++nFolderPathLen] = 0;
	SHFILEOPSTRUCT stSHFileOpStruct = {0};
	// Delete operation
	stSHFileOpStruct.wFunc = FO_DELETE;
	// Folder name as double null terminated string
	stSHFileOpStruct.pFrom = pszFrom; 
	// Do not prompt the user
	stSHFileOpStruct.fFlags = FOF_NO_UI;
	// Delete operation can be undo (to recycle bin) 
	stSHFileOpStruct.fFlags = FOF_ALLOWUNDO; 
	//Check for any operation is aborted
	stSHFileOpStruct.fAnyOperationsAborted = FALSE; 
	int nFileDeleteOprnRet = SHFileOperation( &stSHFileOpStruct );
	delete []pszFrom;
	if( 0 != nFileDeleteOprnRet )
	{ 
		// Failed deletion
		return false; 
	} 
	// Deletion was successfull
	return true;
}

BOOL CFileDB::RmDirStd(CString path)
{
	CFileFind ff;
	BOOL ok = ff.FindFile(MakeFilePath(path,_T("*.*")));
	while(ok)
	{
		ok = ff.FindNextFile();
		if(ff.IsDots()) continue;
		if(ff.IsReadOnly()) continue;
		if(ff.IsDirectory()) 
			RmDir(ff.GetFilePath());
		else
			CFile::Remove(ff.GetFilePath());
	}
	ff.Close();

	BOOL rmOk = RemoveDirectory(path);
	return rmOk;
}

BOOL CFileDB::Select(CString id)
{
	m_bNewRecord = FALSE;
	CString path = MakeFilePath(GetPath(),id);
	return ReadMap(m_CurrentRecordMAP,MakeFilePath(path,_T("db.info")));
}

CString CFileDB::GetFileTitle(CString path)
{
	int pos;
	path = GetFileName(path);
	if((pos = path.Find('.')) < 0) return path;
	return path.Left(pos);
}

CString CFileDB::GetFileName(CString path)
{
	int pos;
	if((pos = path.ReverseFind('\\')) < 0) return path;
	return path.Mid(pos+1);
}

CString CFileDB::GetFileBase(CString path)
{
	int pos;
	if((pos = path.ReverseFind('.')) < 0) return path;
	return path.Left(pos);
}

CString CFileDB::GetFilePath(CString path)
{
	int pos;
	if((pos = path.ReverseFind('\\')) < 0) return _T("");
	return path.Left(pos+1);
}

CString CFileDB::GetFileExt(CString path)
{
	int pos1 = path.ReverseFind('\\');
	int pos2 = path.ReverseFind('.');
	if (pos2 > pos1)
		return path.Mid(pos2 + 1);
	return CString();
}

CString CFileDB::GetFileNoExt(CString path)
{
	int pos1 = path.ReverseFind('\\');
	int pos2 = path.ReverseFind('.');
	if (pos2 > pos1)
		return path.Left(pos2);
	return path;
}


CString CFileDB::NewID(void)
{
	return MakeTS14(COleDateTime::GetCurrentTime());
}

BOOL CFileDB::ReadString( CArchive& ar, CString& str )
{
	int cnt = 0;
	ASSERT(FALSE);
	return (cnt != 0);
}

BOOL CFileDB::WriteString( CArchive& ar, const CString& str )
{
#ifdef _UNICODE
	int len = str.GetLength();
	LPCWSTR p = str;
	char out[16] ;

	for(int k = 0; k < len; ++k)
	{
		int l = unichar_to_utf8(p[k],out);
		ar.Write(out,l);
	}
#else
	ar.Write((LPCSTR)str,str.GetLength());
#endif
	return true;
}
BOOL CFileDB::WriteStringA( CArchive& ar, const CString& str )
{
#ifdef _UNICODE
	CStringA s(str);
	ar.Write((LPCSTR)s,s.GetLength());
#else
	ar.Write((LPCSTR)str,str.GetLength());
#endif
	return true;
}

int CFileDB::unichar_to_utf8( UINT c, char *outbuf )
{
	unsigned int len = 0;    
	int first;
	int i;

	if (c < 0x80)
	{
		first = 0;
		len = 1;
	}
	else if (c < 0x800)
	{
		first = 0xc0;
		len = 2;
	}
	else if (c < 0x10000)
	{
		first = 0xe0;
		len = 3;
	}
	else if (c < 0x200000)
	{
		first = 0xf0;
		len = 4;
	}
	else if (c < 0x4000000)
	{
		first = 0xf8;
		len = 5;
	}
	else
	{
		first = 0xfc;
		len = 6;
	}

	if (outbuf)
	{
		for (i = len - 1; i > 0; --i)
		{
			outbuf[i] = (c & 0x3f) | 0x80;
			c >>= 6;
		}
		outbuf[0] = c | first;
	}

	return len;
}

bool CFileDB::IsFile(const CString& path)
{
	return (CFileDB::GetFileStatus(path) == 1);
}

bool CFileDB::IsDir(CString path)
{
	path.TrimRight('\\');
	CFileStatus fs;
	if(! CFile::GetStatus(path,fs) )
		return false;
	if( (fs.m_attribute & 0x10) == 0 )
		return false;
	return true;
}

bool CFileDB::IsWriteable(const CString& path)
{
	if(! IsFile(path) )
		return false;
	CFile file;
	try
	{
		if(! file.Open(path,CFile::modeNoTruncate|CFile::modeReadWrite|CFile::shareExclusive) )
			return false;
		file.Close();
	} catch(CFileException *)
	{
		return false;
	}
	return true;

}

BOOL CFileDB::CreateDirectory(const CString& path)
{
	if (IsDir(path))
		return TRUE;
	int pos = -1;
	do 
	{
		pos = path.Find('\\', pos+1);
		CString sp = (pos >= 0) ? path.Left(pos) : path;
		if (!IsDir(sp))
		{
			if (!::CreateDirectory(sp, 0))
				return FALSE;
		}
	} while (pos >= 0);

	return TRUE;
}

CString CFileDB::AddSlash(const CString& str)
{
	if (str.IsEmpty())
		return str;
	if (str.Right(1) == _T("\\"))
		return str;
	return str + _T("\\");
}

CString CFileDB::FileName(const CString& name)
{
	CString str = name;

	static const TCHAR* chars = _T("\\/><*|:\"\'");
	static const int len = (int)_tcslen(chars);

	for (int i = 0; i < len; ++i)
	{
		str.Replace(chars[i], '_');
	}
	return str;
}

CString CFileDB::SimpleFile(CString name)
{
	LPCWSTR str = L":\\/*?!%><&'\"\t\r\n\a;", p = str;

	while (*p)
	{
		name.Replace(*p,'_');
		++p;
	}
	return name;
}