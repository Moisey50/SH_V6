// interface for the logic classes.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SIMPLELOGIC_H__A1BFC4E5_DA8B_4FAD_8E07_E1E4CF8A2743__INCLUDED_)
#define AFX_SIMPLELOGIC_H__A1BFC4E5_DA8B_4FAD_8E07_E1E4CF8A2743__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <gadgets\shkernel.h>

class B_or: public CCollectorGadget
{
public:
  B_or();
  virtual void ShutDown();
  //
  CDataFrame* DoProcessing(CDataFrame const*const* frames, int nmb);
  DECLARE_RUNTIME_GADGET(B_or);
};

class B_and: public CCollectorGadget
{
public:
  B_and();
  virtual void ShutDown();
  //
  CDataFrame* DoProcessing(CDataFrame const*const* frames, int nmb);
  DECLARE_RUNTIME_GADGET(B_and);
};

class B_xor: public CCollectorGadget
{
public:
  B_xor();
  virtual void ShutDown();
  //
  CDataFrame* DoProcessing(CDataFrame const*const* frames, int nmb);
  DECLARE_RUNTIME_GADGET(B_xor);
};

class B_Not : public CFilterGadget
{
private:
    B_Not(void);
    void ShutDown();
public:
    CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	DECLARE_RUNTIME_GADGET(B_Not);
};

#endif // !defined(AFX_SIMPLELOGIC_H__A1BFC4E5_DA8B_4FAD_8E07_E1E4CF8A2743__INCLUDED_)
