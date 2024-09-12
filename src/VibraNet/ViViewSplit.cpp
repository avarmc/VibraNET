// ViViewSplit.cpp : implementation file
//

#include "stdafx.h"
#include "ViDesktop.h"
#include "ViDesktopCfg.h"
#include "ViDesktopFrm.h"
#include "ViDesktopDoc.h"
#include "ViDesktopView.h"
#include "VectorT.h"
#include "ViViewSplit.h"
#include "ViViewNull.h"
#include "ViViewCamera.h"

enum HitTestValue
{
	noHit = 0,
	vSplitterBox = 1,
	hSplitterBox = 2,
	bothSplitterBox = 3,        // just for keyboard
	vSplitterBar1 = 101,
	vSplitterBar15 = 115,
	hSplitterBar1 = 201,
	hSplitterBar15 = 215,
	splitterIntersection1 = 301,
	splitterIntersection225 = 525
};

// CViViewSplit

IMPLEMENT_DYNCREATE(CViViewSplit, CSplitterWnd)

CViViewSplit::CViViewSplit()
{
	m_bSplitterCreated = FALSE;
	CSingleLock listLock(&theApp.m_listLock, TRUE);
	theApp.m_listSplit.push_back(this);
}

CViViewSplit::~CViViewSplit()
{
	CSingleLock listLock(&theApp.m_listLock, TRUE);
	theApp.m_listSplit.remove(this);
}


BEGIN_MESSAGE_MAP(CViViewSplit, CSplitterWnd)
	ON_WM_SIZE()
	ON_WM_DESTROY()
END_MESSAGE_MAP()



// CViViewSplit message handlers


BOOL CViViewSplit::Create(CWnd* pParentWnd, xml_node& xml,int iid)
{
	m_bSplitterCreated = FALSE;

	pItem->xml = xml;

	CCreateContext ctx;
	ctx.m_pCurrentFrame = theApp.m_pFrame;
	ctx.m_pLastView = theApp.m_pView;
	ctx.m_pCurrentDoc = theApp.m_pDoc;
	ctx.m_pNewViewClass = NULL;
	
	long nRows = atoi(CStringA(pItem->xml["set"].attribute_value("rows")));
	long nCols = atoi(CStringA(pItem->xml["set"].attribute_value("cols")));

	if (nRows < 1)
		nRows = 1;
	if (nCols < 1)
		nCols = 1;
	if (nRows*nCols == 1)
		nRows = 2;

	CreateStatic(pParentWnd, nRows, nCols, WS_CHILD | WS_VISIBLE, iid);

	CRect rw;
	pParentWnd->GetClientRect(&rw);

	CSize sDef(rw.Width()/nCols, rw.Height()/nRows );

	std::list<VI_NET_ITEM *> items = theApp.m_pFrame->m_wndCfg.ChildItems(pItem->iid,true);
	
	for (int y = 0; y < nRows; ++y)
	for (int x = 0; x < nCols; ++x)
		{
			BOOL ok = CreateView(y, x, RUNTIME_CLASS(CViViewNull), sDef, &ctx);
		}
	
	std::list<VI_NET_ITEM *>::iterator xi = items.begin(), xe = items.end();
	int cntx = 0;
	for (int y = 0; y < nRows; ++y)
	for (int x = 0; x < nCols; ++x)
		{
			CWnd *pWnd = GetPane(y, x);
			if (pWnd)
			{
				if (xi != xe)
				{
					VI_NET_ITEM *pCur = *xi;
					pCur->pParentWnd = pWnd;
					if (pCur->pWnd)
					{
						CWnd *pBase = pCur->pWnd->GetWindow();
						if (pBase->m_hWnd)
						{
							pBase->SetParent(pWnd);
							pBase->ShowWindow(SW_SHOWMAXIMIZED);
						}
					} 
					
					++xi;
					++cntx;
				}
			}
			else
			{
				ASSERT(FALSE);
			}
	}
	for (; xi != xe; ++xi)
	{
		theApp.m_pFrame->m_wndCfg.Kill((*xi)->iid, true);
	}

	m_bSplitterCreated = TRUE;

	Align(rw.Width(),rw.Height());
	ShowWindow(SW_SHOWMAXIMIZED);
	return true;
}

void CViViewSplit::Align(int cx, int cy)
{
	if (!m_bSplitterCreated)
		return;

	if (!cx || !cy)
	{
		CRect rw;
		GetParent()->GetClientRect(&rw);
		cx = rw.Width();
		cy = rw.Height();
	}


	int nCols = GetColumnCount();
	int nRows = GetRowCount();

	mmx_array2< Vector2f > sizes(nCols, nRows);
	mmx_array<float> sizesX(nCols);
	mmx_array<float> sizesY(nRows);
	float sizesXs = 0, sizesYs = 0;

	sizes.set(Vector2f(100.0f / nCols, 100.0f / nRows));

	xml_node_iterator xi, xe;
	xi = pItem->xml.begin();
	xe = pItem->xml.end();
	for (int x = 0; x < nCols; ++x)
	{
		for (int y = 0; y < nRows; ++y)
		{
			if (xi != xe)
			{
				CStringA sx = xi->attribute_value("w");
				CStringA sy = xi->attribute_value("h");

				if (!sx.IsEmpty())
					sizes[y][x].x = (float)atof(sx);
				if (!sy.IsEmpty())
					sizes[y][x].y = (float)atof(sy);

				++xi;
			}

			sizesX[x] = max(sizesX[x], sizes[y][x].x);
			sizesY[y] = max(sizesY[y], sizes[y][x].y);

		}
	}

	for (int x = 0; x < nCols; ++x)
		sizesXs += sizesX[x];
	for (int y = 0; y < nRows; ++y)
		sizesYs += sizesY[y];

	for (int x = 0; x < nCols && sizesXs > 0; ++x)
	{
		float fl = sizesX[x] / sizesXs;
		int l = (int)(fl * cx  );
		SetColumnInfo(x, l, 10);
	}

	for (int y = 0; y < nRows && sizesYs > 0; ++y)
	{
		float fl = sizesY[y] / sizesYs;
		int l = (int)(fl * cy);
		SetRowInfo(y, l, 10);
	}

}

void CViViewSplit::OnSize(UINT nType, int cx, int cy)
{
	CSplitterWnd::OnSize(nType, cx, cy);

	if (m_bSplitterCreated && IsWindow(m_hWnd))
		Align(cx,cy);

}


void CViViewSplit::RecalcLayout()
{
	CSplitterWnd::RecalcLayout();
}

void CViViewSplit::TrackRowSize(int y, int row)
{
	ASSERT_VALID(this);
	ASSERT(m_nRows > 1);

	CPoint pt(0, y);
	ClientToScreen(&pt);
	GetPane(row, 0)->ScreenToClient(&pt);
	m_pRowInfo[row].nIdealSize = pt.y;      // new size
	
	int prevS = m_pRowInfo[row].nIdealSize;

	int shift = m_pRowInfo[row].nIdealSize - prevS;

	for (int i = row + 1; i < m_nRows; ++i)
	{
		if (shift < 0)
		{
			m_pRowInfo[i].nIdealSize -= shift;
			shift = 0;
			break;
		}

		if (shift <= m_pColInfo[i].nIdealSize)
		{
			m_pRowInfo[i].nIdealSize -= shift;
			shift = 0;
			break;
		}

		shift -= m_pRowInfo[i].nIdealSize;

		m_pRowInfo[i].nIdealSize = 0;
	}
	ExportSizes();
}

void CViViewSplit::TrackColumnSize(int x, int col)
{
	ASSERT_VALID(this);
	ASSERT(m_nCols > 1);
	
	CPoint pt(x, 0);
	ClientToScreen(&pt);
	GetPane(0, col)->ScreenToClient(&pt);

	int prevS = m_pColInfo[col].nIdealSize;
	m_pColInfo[col].nIdealSize = pt.x;      // new size

	int shift = m_pColInfo[col].nIdealSize - prevS;

	for (int i = col + 1; i < m_nCols; ++i )
	{
		if (shift < 0 )
		{
			m_pColInfo[i].nIdealSize -= shift;
			shift = 0;
			break;
		}

		if (shift <= m_pColInfo[i].nIdealSize)
		{
			m_pColInfo[i].nIdealSize -= shift;
			shift = 0;
			break;
		}

		shift -= m_pColInfo[i].nIdealSize;

		m_pColInfo[i].nIdealSize = 0;
	}
	ExportSizes();
}

void CViViewSplit::ExportSizes()
{
	int sumC = 0, sumR = 0;
	for (int c = 0; c < m_nCols; ++c)
		sumC += m_pColInfo[c].nIdealSize;
	for (int r = 0; r < m_nRows; ++r)
		sumR += m_pRowInfo[r].nIdealSize;

	xml_node_iterator xi, xe;
	xi = pItem->xml.begin();
	xe = pItem->xml.end();
	for (int x = 0; x < m_nCols; ++x)
	{
		for (int y = 0; y < m_nRows; ++y)
		{
			xml_node xc;

			if (xi != xe)
			{
				
				xc = *xi;
				++xi;
			}
			else
			{
				xc = pItem->xml.append_child();
				xc.set_name("item");
				xc.append_attribute("type").set_value("null");
			}

			CStringA sw, sh;
			sw.Format("%g", 100.0*m_pColInfo[x].nIdealSize / static_cast<double>(sumC));
			sh.Format("%g", 100.0*m_pRowInfo[y].nIdealSize / static_cast<double>(sumR));

			
			xc["set"].attribute_new("w").set_value(sw);
			xc["set"].attribute_new("h").set_value(sh);

		}
	}

	theApp.m_pDoc->SetModifiedFlag();
}

void CViViewSplit::OnDestroy()
{
	if (m_bSplitterCreated)
	{
		int nW = GetColumnCount();
		int nH = GetRowCount();
		for (int x = 0; x < nW; ++x)
			for (int y = 0; y < nH; ++y)
			{
				CWnd *pWnd = GetPane(y, x);
				if (pWnd)
				{
					if (IsWindow(pWnd->m_hWnd))
						pWnd->DestroyWindow();
					delete pWnd;
				}
			}
	}
	m_bSplitterCreated = false;
	CSplitterWnd::OnDestroy();
}
