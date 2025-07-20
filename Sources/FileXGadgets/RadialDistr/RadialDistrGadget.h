// RadialDistrGadget.h : Declaration of the RadialDistr

#pragma once

#define THIS_MODULENAME "Radial.RadialDistr"

#include <math\intf_sup.h>
#include <Gadgets\gadbase.h>
#include <gadgets\videoframe.h>
#include <helpers\FramesHelper.h>

class RadialDistr : public CFilterGadget
{
public:
    RadialDistr(void);
    void ShutDown();
//
    CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	bool ScanProperties(LPCTSTR text, bool& Invalidate);
	bool PrintProperties(FXString& text);
	bool ScanSettings(FXString& text);

  int       m_iMinRadius ;
  int       m_iMaxRadius ;
  bool      m_FormatErrorProcessed;
  DWORD     m_LastFormat;
  double    m_dKampl ;

    DECLARE_RUNTIME_GADGET(RadialDistr);
};
