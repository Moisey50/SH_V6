// PerforamceDlg2.cpp : implementation file
//

#include "stdafx.h"
#include "tvdb400.h"
#include "perforamcedlg2.h"

typedef struct tagITEMDATA
{
	FXString sGadgetName;
	double	 dProcTime;
	double   dCpuUsage;
}ITEMDATA;

// CPerforamceDlg2 dialog

IMPLEMENT_DYNAMIC(CPerforamceDlg2, CDialog)

CPerforamceDlg2::CPerforamceDlg2(CWnd* pParent /*=NULL*/)
: CDialog(CPerforamceDlg2::IDD, pParent),
m_pBuilder(NULL)
{

}

CPerforamceDlg2::~CPerforamceDlg2()
{
}

void CPerforamceDlg2::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PERFORMACE_LST, m_PerformanceList);
}


BEGIN_MESSAGE_MAP(CPerforamceDlg2, CDialog)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_PERFORMACE_LST ,OnColumnClick)
	ON_WM_DESTROY()
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CPerforamceDlg2 message handlers


BOOL CPerforamceDlg2::OnInitDialog()
{
	CDialog::OnInitDialog();
	CRect rc;
	m_PerformanceList.GetClientRect(&rc);
	DWORD dwExStyle =  LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES /*| LVS_EX_SUBITEMIMAGES |*/
		/* LVS_EX_HEADERDRAGDROP | LVS_EX_TRACKSELECT*/;
	m_PerformanceList.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LPARAM(dwExStyle));
	m_PerformanceList.InsertColumn(0,"Gadget name",LVCFMT_LEFT,rc.right/3,0);
	m_PerformanceList.InsertColumn(1,"Core % of usage",LVCFMT_LEFT,rc.right/4,1);
	m_PerformanceList.InsertColumn(2,"Last proccessing time ms",LVCFMT_LEFT,rc.right/3,1);
	SetTimer(IDD,1000,NULL);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

int CALLBACK CPerforamceDlg2::SortFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	int nRetVal = 0;

	ITEMDATA* pData1 = (ITEMDATA*)lParam1;
	ITEMDATA* pData2 = (ITEMDATA*)lParam2;

	SortStruct* pSortStruct = (SortStruct*)lParamSort;

	switch(pSortStruct->iSortColumn)
	{
		case 0:	// Gadget Name

			nRetVal = -(pData1->sGadgetName.Compare(pData2->sGadgetName));
			break;

		case 1:	// CPU Usage

			nRetVal = ((pData1->dCpuUsage) >= (pData2->dCpuUsage)) ? 1 :- 1;
			break;

		case 2:	// Proc Time

			nRetVal = ((pData1->dProcTime) >= (pData2->dProcTime)) ? 1 :- 1;
			break;

		default:

			break;
	}

	if(pSortStruct->bUpToDownSortDirection)
		nRetVal=-nRetVal;

	return nRetVal;
}
void CPerforamceDlg2::OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	int iColumn = pNMListView->iSubItem;

	m_SortStruct.setSortColumn(iColumn);
	m_PerformanceList.SortItems(SortFunc, (DWORD_PTR)(&m_SortStruct));

	*pResult = 0;
}


void CPerforamceDlg2::OnDestroy()
{
	KillTimer(IDD);

	int itmCnt = m_PerformanceList.GetItemCount();
	for (int iCnt=0; iCnt<itmCnt; iCnt++)
	{
		ITEMDATA* pItemData = (ITEMDATA*)m_PerformanceList.GetItemData(iCnt);
		if(pItemData)
		{
			delete pItemData;
		}
	}
	m_PerformanceList.DeleteAllItems();
	CDialog::OnDestroy();
}

void CPerforamceDlg2::OnCancel()
{
	DestroyWindow();
}

int  CPerforamceDlg2::ListFindItem(LPCTSTR itemname)
{
	LVFINDINFO info;

	info.flags = LVFI_STRING;
	info.psz = itemname;
	return m_PerformanceList.FindItem(&info);
}

void CPerforamceDlg2::PlotRow(FXString sGadgetName, CGadget *gadget, int& itemCnt)
{
	ITEMDATA* pItemData;

	int itemno=ListFindItem(sGadgetName);	

	if ( itemno>=0 )
	{		
		pItemData = (ITEMDATA*)(m_PerformanceList.GetItemData( itemno ));
	}
	else
	{
		itemno=itemCnt;
		m_PerformanceList.InsertItem(itemno,sGadgetName);
		itemCnt++;
		pItemData = new ITEMDATA;
	}

	CString tmpS;

	double dCpuUsage=gadget->GetCPUUsage();
	if (dCpuUsage>1) dCpuUsage=1.0;	
	tmpS.Format("%.4f",dCpuUsage*100);	
	m_PerformanceList.SetItemText(itemno,1,tmpS);

	double dProcUsage=gadget->GetLastProcTime();
	tmpS.Format("%.4f",dProcUsage);	
	m_PerformanceList.SetItemText(itemno,2,tmpS);


	pItemData->dProcTime = dProcUsage;
	pItemData->dCpuUsage = dCpuUsage;
	pItemData->sGadgetName = (LPTSTR)(LPCTSTR)sGadgetName;
	m_PerformanceList.SetItemData(itemno, (LPARAM)pItemData);

	m_LiveGadgetsArray.Add(sGadgetName);	//	Check for removed gadgets
}

void CPerforamceDlg2::PlotGadget(CGadget *gadget, LPCTSTR sGadgetName, int& itemCnt)
{
	if (gadget->IsKindOf(RUNTIME_GADGET(Complex)))
	{
		CString sComplexGadgetName=CString(sGadgetName)+'.';
		EnumGadgets(((Complex*)gadget)->Builder(), sComplexGadgetName);
	}
	else if (_tcscmp(gadget->GetRuntimeGadget()->m_lpszLineage, LINEAGE_DEBUG) == 0)
	{
		CWnd* rWnd=((CRenderGadget*)gadget)->GetRenderWnd();
		if ( rWnd && IsWindow( rWnd->m_hWnd ) && (rWnd->IsWindowVisible()))
		{
			PlotRow(sGadgetName, gadget, itemCnt);
		}
	}
	else
	{
		PlotRow(sGadgetName, gadget, itemCnt);
	}

}

void CPerforamceDlg2::EnumGadgets(IGraphbuilder* pBuilder, CString& Prefix)
{
	int itemCnt=0;

	if (pBuilder)
	{
		CStringArray sCaptureGadgets,sRendererGadgets,sAllGadgets;
		pBuilder->EnumGadgets(sCaptureGadgets,sRendererGadgets);

		sAllGadgets.Append(sCaptureGadgets);
		sAllGadgets.Append(sRendererGadgets);

		int iGadgetsCnt = (int)sAllGadgets.GetCount();

		for (int iCnt=0; iCnt<iGadgetsCnt; iCnt++)
		{
			CGadget *gadget=pBuilder->GetGadget(sAllGadgets[iCnt]);

			if (gadget)
			{
				CString sGadgetName=Prefix+sAllGadgets[iCnt];
				PlotGadget(gadget, sGadgetName, itemCnt);
			}
		}
	}
}

void CPerforamceDlg2::ClearRemovedGadgets()
{
	int itmCnt = m_PerformanceList.GetItemCount();

	for (int iCnt=0; iCnt<itmCnt; iCnt++)
	{
		ITEMDATA* pItemData = (ITEMDATA*)m_PerformanceList.GetItemData(iCnt);
		FXString sGadgetName = pItemData->sGadgetName;

		if(m_LiveGadgetsArray.Find(sGadgetName) < 0)
		{			
			ITEMDATA* pItemData = (ITEMDATA*)m_PerformanceList.GetItemData(iCnt);
			if(pItemData)
			{
				delete pItemData;
			}
			m_PerformanceList.DeleteItem(iCnt);

			itmCnt = m_PerformanceList.GetItemCount();
		}
	}

	m_LiveGadgetsArray.RemoveAll();
}

void CPerforamceDlg2::PopulateList(IGraphbuilder* pBuilder)
{
	CString Prefix("");

	EnumGadgets(pBuilder, Prefix);

	ClearRemovedGadgets();
}

void CPerforamceDlg2::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent==IDD)
	{
		PopulateList(m_pBuilder);
	}
	CDialog::OnTimer(nIDEvent);
}
