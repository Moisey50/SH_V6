// RelationalCheckList.cpp : implementation file
//

#include "stdafx.h"
#include "RelationalCheckList.h"
//#include "Winuser.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRelationalCheckList
#define BUT_NONE   0x0000
#define BUT_PUSHED 0x0001
#define BUT_RAISED 0x0002
#define ID_PANEL   0x2

CRelationalCheckList::CRelationalCheckList()
{
   m_pwndPanel     = new CStaticEx(this);
}

CRelationalCheckList::~CRelationalCheckList()
{
   m_pwndPanel->DestroyWindow();
   delete m_pwndPanel;
   for(int i=0; i<m_lItems.GetSize(); i++)
	   delete m_lItems[i];
}


BEGIN_MESSAGE_MAP(CRelationalCheckList, CWnd)
	//{{AFX_MSG_MAP(CRelationalCheckList)
	ON_WM_CREATE()
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
//	

/////////////////////////////////////////////////////////////////////////////
// CRelationalCheckList message handlers

int CRelationalCheckList::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
   if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
  //ASSERT(m_pwndPanel->Create(NULL,NULL,WS_BORDER|WS_CHILD|WS_CLIPCHILDREN,CRect(),GetDesktopWindow()/*GetParent()*/,ID_PANEL));
  m_pwndPanel->m_pBuddy = this;
  m_font.CreatePointFont(90, "Arial",GetDC());
  SetFont(&m_font);
  ASSERT(ModifyStyle(0,WS_CLIPSIBLINGS));
  return 0;
}

int CRelationalCheckList::AddString(LPCTSTR lpszItem)
{
  CRelationalCheckList::__item* pitem = new CRelationalCheckList::__item;
  pitem->m_nIndex      = GetCount();
  m_lItems.Add(pitem);
  return CListBox::AddString(lpszItem);
}

int CRelationalCheckList::AddRelationalString(LPCTSTR lpszItem)
{
  CRelationalCheckList::__item* pitem = new CRelationalCheckList::__item;
  pitem->m_bRelational = TRUE;
  pitem->m_nIndex      = GetCount();
  m_lItems.Add(pitem);
  return CListBox::AddString(lpszItem);
}


void CRelationalCheckList::ResetContent()
{
   for(int i=0; i<GetCount(); i++)
	   delete m_lItems[i];

   m_lItems.RemoveAll();
   CListBox::ResetContent();
}

int CRelationalCheckList::DeleteString(UINT nIndex)
{
  ASSERT(nIndex<(UINT)GetCount());
  delete m_lItems[nIndex];
  m_lItems.RemoveAt(nIndex);
  return CListBox::DeleteString(nIndex);
}

void CRelationalCheckList::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	 CDC dc;
	 dc.Attach(lpDrawItemStruct->hDC);
	 CFont* pOldFont = dc.SelectObject(&m_font);
	 int npos = lpDrawItemStruct->itemID;
     

	 CRect nCheckBoxRect(lpDrawItemStruct->rcItem);
	 

	 m_lItems[npos]->m_nRect = nCheckBoxRect;
	 nCheckBoxRect.DeflateRect(0,0,1,1);
	 nCheckBoxRect.OffsetRect(1,0);
     
	 //use this as text and actual rect
	 CRect acRect(nCheckBoxRect);
	 acRect.DeflateRect(16,0,0,0);

	 dc.FillSolidRect(nCheckBoxRect,::GetSysColor(COLOR_HIGHLIGHTTEXT));

	 nCheckBoxRect.DeflateRect(0,0,nCheckBoxRect.Width(),0);
	 nCheckBoxRect.InflateRect(0,0,15,0);


     // Save these value to restore them when done drawing.
     COLORREF crOldTextColor = dc.GetTextColor();
     COLORREF crOldBkColor = dc.GetBkColor();



    // If this item is selected, set the background color 
    // and the text color to appropriate values. Also, erase
    // rect by filling it with the background color.
    if ((lpDrawItemStruct->itemAction | ODA_SELECT) &&
      (lpDrawItemStruct->itemState & ODS_SELECTED))
    {
      dc.SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
      dc.SetBkColor(::GetSysColor(COLOR_HIGHLIGHT));
      dc.FillSolidRect(&acRect, 
         ::GetSysColor(COLOR_HIGHLIGHT));
    }
	else
	if ( !IsEnabled( npos ) )
	{
		dc.SetTextColor(::GetSysColor(COLOR_GRAYTEXT));
		dc.FillSolidRect(&acRect, ::GetSysColor(COLOR_WINDOW) );
	}
    else
	{
		dc.FillSolidRect(&acRect, ::GetSysColor(COLOR_WINDOW) );
		//dc.FillSolidRect(&acRect, crOldBkColor);
	}

	 //draw item's basic data
	 CString szpText;
	 GetText(npos,szpText);
	 CSize sz = dc.GetTextExtent(szpText);
	 int y = nCheckBoxRect.bottom-sz.cy;
     dc.ExtTextOut(nCheckBoxRect.right+2,y,ETO_CLIPPED,acRect,szpText,NULL);
     
	 //draw checkbox frame
	 dc.Draw3dRect( nCheckBoxRect, GetSysColor(COLOR_BTNTEXT), 
					GetSysColor(COLOR_BTNTEXT) );
	 
	 if(m_lItems[npos]->m_bRelational)
	 {
	   //draw frame for arrow bitmap
	   CRect nBmpFrm(nCheckBoxRect);
	   nBmpFrm.DeflateRect( 1,0,1,1 );
	   nBmpFrm.DeflateRect( 0, nCheckBoxRect.Height() / 2, 0, 0 );
       //dc.FillSolidRect(nBmpFrm, m_lItems[npos]->m_bEnabled ? ::GetSysColor(COLOR_3DFACE) : GetSysColor(COLOR_GRAYTEXT) );
		//int iColor = m_lItems[npos]->m_bEnabled ? ::GetSysColor(COLOR_3DFACE) : GetSysColor(COLOR_GRAYTEXT);
		int iColor = ::GetSysColor(COLOR_WINDOW);//m_lItems[npos]->m_bEnabled ? ::GetSysColor(COLOR_WINDOW) : GetSysColor(COLOR_WINDOW);
		dc.FillSolidRect( nBmpFrm, iColor );

	   //blit the arrow,if item is relational
	   
       CDC mdc;
	   mdc.CreateCompatibleDC(&dc);

	   int x = (nCheckBoxRect.Width()/2)-3;
	   x += nCheckBoxRect.left;

	   int y = (nCheckBoxRect.Height()/2)+2;
	   y += nCheckBoxRect.top;

	   //draw arrow
	   {
		 CPen npen;
		 
		 if(m_lItems[npos]->m_bChecked&&m_lItems[npos]->m_lwndItems.GetSize()>0)
			 npen.CreatePen(PS_SOLID,1,GetSysColor(COLOR_BTNTEXT));
		 else
			 npen.CreatePen(PS_SOLID,1,RGB(255,255,255));
		 CPen *pOldPen = dc.SelectObject(&npen);

         int x1 = x+7;
		 int y1 = y+4;
		 for(y; y<y1;y++)
		 {
           dc.MoveTo(x,y);
		   dc.LineTo(x1,y);
		   x1--;x++;
		 }
		 dc.SelectObject(pOldPen);
	   }

	   //draw check dot
	   if(m_lItems[npos]->m_bChecked)
	   {
		 //MessageBox("debug");
         nCheckBoxRect.DeflateRect(0,0,0,nCheckBoxRect.Height()/2);
		 int x = nCheckBoxRect.Width()/2-2;
		 int y = nCheckBoxRect.Height()/2+2;
		 nCheckBoxRect.DeflateRect(x,y,x,y);
         dc.FillSolidRect(nCheckBoxRect,GetSysColor(COLOR_BTNTEXT));
	   }
	 }
	 else
	 {
         //draw check
		 if(m_lItems[npos]->m_bChecked)
		 {
           CPen checkPen;
	       checkPen.CreatePen(PS_SOLID,2,GetSysColor(COLOR_BTNTEXT));
	       CPen* pOldPen = dc.SelectObject(&checkPen);

	       //draw check
	       //nCheckBoxRect.DeflateRect(2,2,2,2);
           int y = (nCheckBoxRect.Height()/2)+nCheckBoxRect.top;
	       int x = (nCheckBoxRect.Width()/2)+nCheckBoxRect.left;
           dc.MoveTo(nCheckBoxRect.left+3,y);
	       dc.LineTo(x,nCheckBoxRect.bottom-3);

		   dc.MoveTo(x,nCheckBoxRect.bottom-3);
		   dc.LineTo(nCheckBoxRect.right-3,nCheckBoxRect.top+3);
		 }
	 }
	 

    dc.SetTextColor(crOldTextColor);
    dc.SetBkColor(crOldBkColor);
	dc.SelectObject(pOldFont);
	dc.Detach();
}

void CRelationalCheckList::PreSubclassWindow() 
{
  ASSERT(m_pwndPanel->Create(GetParent()));
  m_font.CreatePointFont(90, "Arial",GetDC());
  SetFont(&m_font);
  ASSERT(ModifyStyle(0,WS_CLIPSIBLINGS));
  CListBox::PreSubclassWindow();
}

void CRelationalCheckList::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CRelationalCheckList::__item* pitem = getItem(point);
    CRect rcCheck;
	if(pitem != NULL)
	{
	  if(pitem->m_bRelational&&IsEnabled(pitem->m_nIndex))
	  {
		  //check box,upper half
        CRect nCheckRect(pitem->m_nRect);
	    nCheckRect.DeflateRect(0,0,nCheckRect.Width()-16,nCheckRect.Height()/2);

        rcCheck = nCheckRect;
		

		//rest of item rect excluding check and dropwdown rects
	    CRect acRect(pitem->m_nRect);
	    acRect.DeflateRect(16,0,0,0);

		//dropdown rect lower half of check
		CRect rcDropDown(pitem->m_nRect);
		rcDropDown.DeflateRect(0,rcDropDown.Height()/2,rcDropDown.Width()-16,0);

		if(rcDropDown.PtInRect(point)&&pitem->m_bChecked&&pitem->m_lwndItems.GetSize()>0)
		{
          //drop down panel
		  m_CurDropDownItem = pitem->m_nIndex;
		  showPanel(pitem);
		}
		else
	    if(nCheckRect.PtInRect(point))
		{
	      pitem->m_bChecked = !pitem->m_bChecked;


          DRAWITEMSTRUCT di;
		  di.hDC     = GetDC()->m_hDC;
		  di.itemID  = pitem->m_nIndex;
		  di.rcItem  = pitem->m_nRect;
		  di.CtlType = ODT_LISTBOX;
		  di.itemState = pitem->m_nIndex == GetCurSel()?ODS_SELECTED:ODS_DEFAULT;
		  DrawItem(&di);::ReleaseDC(m_hWnd,di.hDC);
		}
		else
		if(acRect.PtInRect(point))
          SetCurSel(pitem->m_nIndex);
	  }
	  else
	  if(IsEnabled(pitem->m_nIndex))
	  {
        CRect nCheckRect(pitem->m_nRect);
	    nCheckRect.DeflateRect(0,0,nCheckRect.Width()-16,0);
        rcCheck = nCheckRect;
	    CRect acRect(pitem->m_nRect);
	    acRect.DeflateRect(16,0,0,0);
	    if(nCheckRect.PtInRect(point))
		{
	      pitem->m_bChecked = !pitem->m_bChecked;

		  
          DRAWITEMSTRUCT di;
		  di.hDC     = GetDC()->m_hDC; 
		  di.itemID  = pitem->m_nIndex;
		  di.CtlType = ODT_LISTBOX;
		  di.rcItem  = pitem->m_nRect;
		  di.itemState = pitem->m_nIndex == GetCurSel()?ODS_SELECTED:ODS_DEFAULT;
		  DrawItem(&di);::ReleaseDC(m_hWnd,di.hDC);
		}
		else
		if(acRect.PtInRect(point))
          SetCurSel(pitem->m_nIndex);
	  }
	}

	//avoid selection check for click in check box
	if(pitem != NULL&&pitem->m_bEnabled&&!rcCheck.PtInRect(point))
	   CWnd::OnLButtonDown(nFlags, point);
}

CRelationalCheckList::__item* CRelationalCheckList::getItem(CPoint pt)
{
  CRect rc;
  CRect rt;
  GetClientRect(&rt);
  for(int npos =0; npos<GetCount(); npos++)
  {
    //if(m_lItems[npos]->m_nRect.PtInRect(pt))
	//	return m_lItems[npos];
	GetItemRect(npos,&rc);
	rc.right = rt.right;
	m_lItems[npos]->m_nRect = rc;
	if(rc.PtInRect(pt))return m_lItems[npos];
  }return NULL;
}


void CRelationalCheckList::showPanel(CRelationalCheckList::__item* pitem)
{
  if(pitem->m_lwndItems.GetSize()>0)
  {
    //save this for use in panel
	m_pCurArray = &pitem->m_lwndItems;

	//contrust panel rectangle relative to item
	CRect rc1(pitem->m_nRect);
	ClientToScreen(&rc1);
	//m_pwndPanel->GetParent()->ScreenToClient(&rc1);
	int x  = rc1.left; //pitem->m_nRect.left+4;
	int y  = rc1.bottom; //pitem->m_nRect.bottom+1;
	int cx = x+pitem->m_rcPanel.cx;
	int cy = y+pitem->m_rcPanel.cy;
	CRect rc(x,y,cx,cy);
	

	m_pwndPanel->ShowWindow(SW_SHOW);//call this,so we get WM_SHOWWINDOW message
	m_pwndPanel->MoveWindow(rc);

	
	//CWnd::AnimateWindow(200,AW_SLIDE);//if u have the mfc7.0 library,i don't
	//show windows
    for(int i=0; i<pitem->m_lwndItems.GetSize(); i++)
	{
	  pitem->m_lwndItems[i]->m_pwndOrgParent = pitem->m_lwndItems[i]->m_pwnd->SetParent(m_pwndPanel);
	  pitem->m_lwndItems[i]->m_pwnd->ShowWindow(SW_SHOW);
      pitem->m_lwndItems[i]->m_pwnd->MoveWindow(pitem->m_lwndItems[i]->m_rect);
	}
  }
}

void CRelationalCheckList::AddWnd(int nIndex,CWnd* pwnd,CRect rc)
{
  CRelationalCheckList::__item::__wnd* pwndItem = new CRelationalCheckList::__item::__wnd;
  pwndItem->m_pwnd = pwnd;
  pwndItem->m_rect = rc;
  m_lItems[nIndex]->m_lwndItems.Add(pwndItem);

  pwnd->ShowWindow(SW_HIDE);
  pwnd->MoveWindow(rc);
}

void CRelationalCheckList::SetPanelSize(int nIndex,CSize rc)
{
  ASSERT(nIndex<GetCount());
  m_lItems[nIndex]->m_rcPanel = rc;
}

void CRelationalCheckList::RemoveWnd(int nIndex,CWnd* pwnd)
{
  ASSERT(nIndex<GetCount());
  for(int i=0; i<m_lItems[nIndex]->m_lwndItems.GetSize();i++)
  {
    if(m_lItems[nIndex]->m_lwndItems[i]->m_pwnd == pwnd)
	{
       delete m_lItems[nIndex]->m_lwndItems[i];
	   m_lItems[nIndex]->m_lwndItems.RemoveAt(i);
	   break;
	}
  }
}



BOOL CRelationalCheckList::ItemIsChecked(int nIndex)
{
  ASSERT(nIndex<GetCount());
  return m_lItems[nIndex]->m_bChecked;
}

void CRelationalCheckList::SetCheck(int nIndex,int nCheck)
{
  ASSERT(nIndex<m_lItems.GetSize());
  m_lItems[nIndex]->m_bChecked = nCheck;
          
  DRAWITEMSTRUCT di;
  di.hDC     = GetDC()->m_hDC;
  di.itemID  = m_lItems[nIndex]->m_nIndex;
  di.CtlType = ODT_LISTBOX;
  di.rcItem  = m_lItems[nIndex]->m_nRect;
  di.itemState = m_lItems[nIndex]->m_nIndex == GetCurSel()?ODS_SELECTED:ODS_DEFAULT;
  DrawItem(&di);

  ::ReleaseDC(m_hWnd,di.hDC);
}


BOOL CRelationalCheckList::IsEnabled(int nIndex)
{
  ASSERT(nIndex<m_lItems.GetSize());
  return m_lItems[nIndex]->m_bEnabled;
}

void CRelationalCheckList::Enable(int nIndex,BOOL bEnabled /*= TRUE*/)
{
  ASSERT(nIndex<m_lItems.GetSize());
  m_lItems[nIndex]->m_bEnabled = bEnabled;
}

int CRelationalCheckList::GetCheck(int nIndex)
{
  ASSERT(nIndex<m_lItems.GetSize());
  return m_lItems[nIndex]->m_bChecked;
}


UINT CRelationalCheckList::GetCheckStyle()
{
 return BS_CHECKBOX;
}

void CRelationalCheckList::SetCheckStyle(UINT nStyle)
{

}


CRect CRelationalCheckList::OnGetCheckPosition(CRect rectItem,CRect rectCheckBox)
{
  return CRect();
}


