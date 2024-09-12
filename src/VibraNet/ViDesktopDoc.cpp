
// ViDesktopDoc.cpp : implementation of the CViDesktopDoc class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "ViDesktop.h"
#endif

#include "ViDesktopDoc.h"
#include "ViDesktopFrm.h"
#include "ViNet.h"
#include "FileDB.h"
#include <propkey.h>
#include <sstream>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CViDesktopDoc

IMPLEMENT_DYNCREATE(CViDesktopDoc, CDocument)

BEGIN_MESSAGE_MAP(CViDesktopDoc, CDocument)
END_MESSAGE_MAP()


// CViDesktopDoc construction/destruction

CViDesktopDoc::CViDesktopDoc()
{
	theApp.m_pDoc = this;

}

CViDesktopDoc::~CViDesktopDoc()
{
	if( theApp.m_pDoc == this)
		theApp.m_pDoc = NULL;
}

BOOL CViDesktopDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	CString path = GetLastDocument();

	if(CFileDB::IsFile(path))
		theApp.OpenDocumentFile(path);

	return TRUE;
}




// CViDesktopDoc serialization

void CViDesktopDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		std::ostringstream oss;
		m_xml.print(oss);
		
		std::string str = xml_document::xml_header(true)+ "\r\n" + oss.str();
		
		ar.Write( str.c_str(), (UINT)str.length() );


	}
	else
	{
		mmx_array<char> str(1024 * 1024);
		UINT size = ar.Read(str.p, (UINT)str.s);
		if(theApp.m_pFrame)
			theApp.m_pFrame->m_wndCfg.ReloadString(str.p);
	}
}

#ifdef SHARED_HANDLERS

// Support for thumbnails
void CViDesktopDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Modify this code to draw the document's data
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// Support for Search Handlers
void CViDesktopDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data. 
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CViDesktopDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CViDesktopDoc diagnostics

#ifdef _DEBUG
void CViDesktopDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CViDesktopDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CViDesktopDoc commands


BOOL CViDesktopDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	theApp.m_pNet->Init();

	theApp.WriteSectionString(_T("Settings"), _T("document"), lpszPathName);
	return TRUE;
}


void CViDesktopDoc::DeleteContents()
{
	theApp.m_pNet->Close();

	CDocument::DeleteContents();
}


CString CViDesktopDoc::GetLastDocument()
{
	return theApp.GetProfileString(_T("Settings"), _T("document"), GetDefaultPath() + _T("vibranet.xml") );

}


BOOL CViDesktopDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	BOOL ok =  CDocument::OnSaveDocument(lpszPathName);

	if(ok)
		theApp.WriteSectionString(_T("Settings"), _T("document"), lpszPathName);

	return ok;
}
