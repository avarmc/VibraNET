#pragma once
#include <list>
#include <map>

class CFileDB
{
public:
	typedef std::map<CString,CString> MAP;
private:
	CString	m_path;
private:
	BOOL		m_bNewRecord;
	MAP			m_CurrentRecordMAP;
public:
	CFileDB(CString path=_T(""));
	~CFileDB(void);
private:
	static void PackString(CString& str);
	static void UnpackString(CString& str);
	static BOOL WriteMap(MAP& m, CString file);
	static BOOL ReadMap(MAP& m, CString file);
	static BOOL ReadMap(MAP& m, IXMLDOMNodeList*      pXMLNodeList);
public:
	static void ListDir(const CString& path, bool bDir, std::list<CString>& result,bool bClear=true);
	static int GetFileStatus(const CString& file);
	static bool RmDir( const CString& csDeleteFolderPath_i );
	static BOOL RmDirStd(CString path);
	static CString MakeFilePath(CString path, CString file=_T(""));
	static CString GetFileTitle(CString path);
	static CString GetFileName(CString path);
	static CString GetFileBase(CString path);
	static CString GetFilePath(CString path);
	static CString GetFileExt(CString path);
	static CString GetFileNoExt(CString path);
	static bool IsFile(const CString& path);
	static bool IsDir(CString path);
	static bool IsWriteable(const CString& path);
	static CString AddSlash(const CString& str);
	static BOOL CreateDirectory(const CString& path);
	static CString FileName(const CString& name);
	static CString SimpleFile(CString name);
public:
	static CString MakeTS14(const COleDateTime& t);
	static CString NewID(void);
	static BOOL ReadString( CArchive& ar, CString& str );
	static BOOL WriteString( CArchive& ar, const CString& str );
	static BOOL WriteStringA( CArchive& ar, const CString& str );
	static int unichar_to_utf8( UINT c, char *outbuf );

public:
	CString AddNew(CString id=_T(""));
	void AddRaw(const CString& key, const CString& value);
	CString GetRaw(CString key);
	BOOL Update(void);
	BOOL Remove(CString id);

public:
	MAP& GetMap();
	CString GetPath(void);
	void SetPath(const CString& path);
	BOOL Select(CString id);
};

inline CFileDB::MAP& CFileDB::GetMap()
{
	return m_CurrentRecordMAP;
}

inline void CFileDB::AddRaw(const CString& key, const CString& value)
{
	m_CurrentRecordMAP.erase(key);
	m_CurrentRecordMAP.insert(MAP::value_type (key,value));
}

inline CString CFileDB::GetPath(void)
{
	return m_path;
}

inline void CFileDB::SetPath(const CString& path)
{
	m_path = path;
	m_path.TrimRight(_T("\\"));
}

inline CString CFileDB::MakeTS14(const COleDateTime& t)
{
	CString strTS14;
	strTS14.Format(_T("%0.4u-%0.2u-%0.2u %0.2u_%0.2u_%0.2u"),
		t.GetYear(),t.GetMonth(),t.GetDay(),
		t.GetHour(),t.GetMinute(),t.GetSecond());
	return strTS14;
}

inline CString CFileDB::GetRaw(CString key)
{
	MAP::iterator i = m_CurrentRecordMAP.find(key);
	if(i == m_CurrentRecordMAP.end())
		return CString();
	return i->second;
}

