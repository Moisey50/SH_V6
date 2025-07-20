// UndoManager.h: interface for the CUndoManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UNDOMANAGER_H__958A3666_A49F_4ABA_A0C8_D1DF47BDE116__INCLUDED_)
#define AFX_UNDOMANAGER_H__958A3666_A49F_4ABA_A0C8_D1DF47BDE116__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef _DEBUG
//#define DEBUG_UNDO_MANAGER
#endif

class CUndoManager  
{
private:
	CStringArray m_States;
	CStringArray m_StateIDs;
	int          m_nCurState;
    bool         m_Frozen;
    DWORD        m_ChangesKey;
public:
	CUndoManager();
	~CUndoManager();

	void	AddState(LPCTSTR state, LPCTSTR id = "");
	int		CanUndo(CStringArray* IDs = NULL);
	int		CanRedo(CStringArray* IDs = NULL);
	LPCTSTR UndoStart(int steps = 1); // negative steps -> Redo
    void	UndoEnd(); 
	void	Reset();
    DWORD   GetChanges() { return m_ChangesKey; };
private:
#ifdef DEBUG_UNDO_MANAGER
	void Dump();
#endif
};

#endif // !defined(AFX_UNDOMANAGER_H__958A3666_A49F_4ABA_A0C8_D1DF47BDE116__INCLUDED_)
