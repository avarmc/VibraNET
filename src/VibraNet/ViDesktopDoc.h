
// ViDesktopDoc.h : interface of the CViDesktopDoc class
//


#pragma once


class CViDesktopDoc : public CDocument
{
protected: // create from serialization only
	CViDesktopDoc();
	DECLARE_DYNCREATE(CViDesktopDoc)

public:
	xml_document			m_xml;
	CCriticalSection		m_xmlLock;
// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// Implementation
public:
	virtual ~CViDesktopDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// Helper function that sets search content for a Search Handler
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
public:
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual void DeleteContents();
	CString GetLastDocument();
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
};
