// ControledFilter.h: interface for the CControledFilter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CONTROLEDFILTER_H__04730E8C_F9BD_4853_98AA_4B300339FA1D__INCLUDED_)
#define AFX_CONTROLEDFILTER_H__04730E8C_F9BD_4853_98AA_4B300339FA1D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <gadgets\gadbase.h>

class FX_EXT_GADGET CControledFilter : public CFilterGadget
{
protected:
	volatile bool		m_bNoSync;
    bool                m_NeverSync;
	CDuplexConnector*	m_pControl;
	FXLockObject		m_Lock;
	CDataFrame*			m_LastDataFrame;
	CDataFrame*			m_LastParamFrame;
protected:
	CControledFilter(int inputtype=transparent, int outputtype=transparent, int duplextype=transparent);
public:
    virtual void ShutDown();
	virtual int GetDuplexCount();
	virtual CDuplexConnector* GetDuplexConnector(int n);
	virtual void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);
    void    NeverSync(bool Set) { m_NeverSync=Set; }
private:
	virtual int DoJob();
	void Process(const CDataFrame* pDataFrame, const CDataFrame* pParamFrame);
	virtual CDataFrame* DoProcessing(const CDataFrame* pDataFrame, const CDataFrame* pParamFrame);
	DECLARE_RUNTIME_GADGET(CControledFilter);
};

#endif // !defined(AFX_CONTROLEDFILTER_H__04730E8C_F9BD_4853_98AA_4B300339FA1D__INCLUDED_)
