// BrowseRemoteGraphsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "tvdb400.h"
#include "BrowseRemoteGraphsDlg.h"


// CBrowseRemoteGraphsDlg dialog

IMPLEMENT_DYNAMIC(CBrowseRemoteGraphsDlg, CDialog)

CBrowseRemoteGraphsDlg::CBrowseRemoteGraphsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBrowseRemoteGraphsDlg::IDD, pParent)
{

}

CBrowseRemoteGraphsDlg::~CBrowseRemoteGraphsDlg()
{
}

void CBrowseRemoteGraphsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_GRAPHS, m_Graphs);
}


BEGIN_MESSAGE_MAP(CBrowseRemoteGraphsDlg, CDialog)
	ON_NOTIFY(NM_DBLCLK, IDC_GRAPHS, &CBrowseRemoteGraphsDlg::OnNMDblclkGraphs)
END_MESSAGE_MAP()


void CBrowseRemoteGraphsDlg::ChangeCurDir(CString& dir)
{
	int i;
	m_CurDir = dir;
	CStringArray Dirs, Files;
	if (m_CurDir != m_RootDir)
		Dirs.Add("..");
	CString lastDir = "";
	for (i = 0; i < m_GraphNames.GetSize(); i++)
	{
		CString graph = m_GraphNames.GetAt(i);
		if (!graph.Find(m_CurDir))
		{
			graph = graph.Mid(m_CurDir.GetLength() + 1);
			int pos = graph.Find('\\');
			if (pos < 0)
				Files.Add(graph);
			else
			{
				CString dir = graph.Left(pos);
				if (dir != lastDir)
				{
					Dirs.Add(dir);
					lastDir = dir;
				}
			}
		}
	}
	m_Graphs.DeleteAllItems();
	i = 0;
	while (Dirs.GetSize())
	{
		m_Graphs.InsertItem(i, Dirs.GetAt(0), 0);
		m_Graphs.SetItemData(i++, 0);
		Dirs.RemoveAt(0);
	}
	while (Files.GetSize())
	{
		m_Graphs.InsertItem(i, Files.GetAt(0), 1);
		m_Graphs.SetItemData(i++, 1);
		Files.RemoveAt(0);
	}
}

void CBrowseRemoteGraphsDlg::ProcessItem(int nItem)
{
	ASSERT(nItem != -1);
	CString sub = m_Graphs.GetItemText(nItem, 0);
	if (!m_Graphs.GetItemData(nItem)) // directory
	{
		CString dir;
		if (sub == "..")
		{
			dir = m_CurDir.Left(m_CurDir.ReverseFind('\\') + 1);
			dir.TrimRight("\\");
		}
		else
		{
			dir.Format("%s\\%s", m_CurDir, sub);
		}
		ChangeCurDir(dir);
	}
	else
	{
		m_SelGraph.Format("%s\\%s", m_CurDir, sub);
		CDialog::OnOK();
	}
}

// CBrowseRemoteGraphsDlg message handlers

BOOL CBrowseRemoteGraphsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CRect rc;
	m_Graphs.GetClientRect(&rc);
	m_Graphs.InsertColumn(0, "Graph", LVCFMT_LEFT, rc.Width() - 20);
	ASSERT(m_Icons.Create(IDB_BRSDLGICONS, 12, 2, RGB(0, 128, 128)));
	m_Graphs.SetImageList(&m_Icons, LVSIL_SMALL);

	CString graph = m_GraphNames.GetAt(0);
	int pos = graph.ReverseFind('\\');
	m_RootDir = graph.Left(pos);
	m_CurDir = m_RootDir;
	ChangeCurDir(m_RootDir);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CBrowseRemoteGraphsDlg::OnOK()
{
	UINT cSel = m_Graphs.GetSelectedCount();
	if (!cSel)
		return;
	int nItem = m_Graphs.GetNextItem(-1, LVNI_SELECTED);
	ProcessItem(nItem);
}

void CBrowseRemoteGraphsDlg::OnNMDblclkGraphs(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	ProcessItem(pNMItemActivate->iItem);
	*pResult = 0;
}
