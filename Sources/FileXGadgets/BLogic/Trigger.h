// Trigger.h: interface for the Trigger class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TRIGGER_H__944EC32B_5740_4C3A_9DBB_802755A1E0BF__INCLUDED_)
#define AFX_TRIGGER_H__944EC32B_5740_4C3A_9DBB_802755A1E0BF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <gadgets\gadbase.h>
#include <gadgets\ControledFilter.h>

class Trigger : public CControledFilter  
{
private:
    unsigned m_Count;
    int m_bLastOnly ;
    FXPropertyKit m_Attrib ;
public:
	Trigger();
	virtual CDataFrame* DoProcessing(const CDataFrame* pDataFrame, const CDataFrame* pParamFrame);
  bool ScanSettings(FXString& txt);
  bool ScanProperties(LPCTSTR txt, bool& Invalidate);
  bool PrintProperties(FXString& txt);
	DECLARE_RUNTIME_GADGET(Trigger);
};

// class ShiftRegister : public CControledFilter
// {
// private:
//   unsigned m_Count;
//   int m_bLastOnly ;
//   FXPropertyKit m_Attrib ;
//   FXArray<CDataFrame*> m_SavedFrames ;
// public:
//   ShiftRegister();
//   virtual CDataFrame* DoProcessing( const CDataFrame* pDataFrame , const CDataFrame* pParamFrame );
//   bool ScanSettings( FXString& txt );
//   bool ScanProperties( LPCTSTR txt , bool& Invalidate );
//   bool PrintProperties( FXString& txt );
//   DECLARE_RUNTIME_GADGET( ShiftRegister );
// };


#endif // !defined(AFX_TRIGGER_H__944EC32B_5740_4C3A_9DBB_802755A1E0BF__INCLUDED_)
