// GraphComposer.h: interface for the CGraphComposer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GRAPHCOMPOSER_H__70FA5A9B_DB46_46B1_A2D5_B4363D8953DD__INCLUDED_)
#define AFX_GRAPHCOMPOSER_H__70FA5A9B_DB46_46B1_A2D5_B4363D8953DD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include <Gadgets\tvdbase.h>
#include <Gadgets\shkernel.h>

class CGraphComposer  
{
	CMapStringToPtr m_GadgetsPositions;
public:
	CGraphComposer() {};
	virtual ~CGraphComposer();

	POSITION GetStartPosition() { return m_GadgetsPositions.GetStartPosition(); };
	void GetNextGadget(POSITION& pos, FXString& uid, CPoint& pt);
	virtual void ComposeGraph(IGraphbuilder* GraphBuilder, CStringArray* SrcGadgets = NULL, CStringArray* DstGadgets = NULL) {};

protected:
	void AddGadgetPosition(CString& uid, CPoint* pt) { m_GadgetsPositions.SetAt(uid, pt); };
};

__forceinline CGraphComposer::~CGraphComposer()
{
	POSITION pos = GetStartPosition();
	CString uid;
	CPoint* pt;
	while (pos)
	{
		m_GadgetsPositions.GetNextAssoc(pos, uid, (void*&)pt);
		if (pt)
			delete pt;
	}
	m_GadgetsPositions.RemoveAll();
}

__forceinline void CGraphComposer::GetNextGadget(POSITION& pos, FXString& uid, CPoint& pt)
{
	CPoint* point;
    CString _uid;
	m_GadgetsPositions.GetNextAssoc(pos, _uid, (void*&)point);
    uid=_uid;
	if (point)
		pt = *point;
}


#endif // !defined(AFX_GRAPHCOMPOSER_H__70FA5A9B_DB46_46B1_A2D5_B4363D8953DD__INCLUDED_)
