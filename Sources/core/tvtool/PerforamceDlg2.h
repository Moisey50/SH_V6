#pragma once
#include <gadgets\shkernel.h>


// CPerforamceDlg2 dialog

class CPerforamceDlg2 : public CDialog
{
	DECLARE_DYNAMIC(CPerforamceDlg2)

	struct SortStruct 
	{
	public:

		int iSortColumn;
		bool bUpToDownSortDirection;

		SortStruct()
		{
			iSortColumn = -1;
			bUpToDownSortDirection = true;
		}

		void setSortColumn(int iColumn)
		{
			bUpToDownSortDirection = (iSortColumn == iColumn) ? !bUpToDownSortDirection : true;
			iSortColumn = iColumn;
		}
	};
protected:
	CListCtrl       m_PerformanceList;
	IGraphbuilder*  m_pBuilder;
	SortStruct m_SortStruct;
	FXArray<FXString> m_LiveGadgetsArray;
public:
	CPerforamceDlg2(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPerforamceDlg2();
	void SetBuilder(IGraphbuilder* pBuilder) { m_pBuilder=pBuilder;}
	// Dialog Data
	enum { IDD = IDD_VIEW_PERFORANCEDLG };
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void    PopulateList(IGraphbuilder* pBuilder);
private:
	void	PlotRow(FXString name, CGadget *gadget, int& string);
	void	PlotGadget(CGadget *gadget, LPCTSTR sGadgetName, int& itemCnt);
	void	EnumGadgets(IGraphbuilder* pBuilder, CString& Prefix);
	void	plotList(CGadget *gadget, CString& Prefix, CString sGadget, int& itemCnt);
	int		ListFindItem(LPCTSTR itemname);
	void	ClearRemovedGadgets();

protected:
	DECLARE_MESSAGE_MAP()
	static int CALLBACK SortFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult); 
	afx_msg void OnDestroy();
	afx_msg void OnCancel();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
