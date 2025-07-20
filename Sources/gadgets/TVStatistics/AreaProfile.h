// AreaProfile.h : Declaration of the CAreaProfile class

#pragma once
#include <math\intf_sup.h>
#include <gadgets\gadbase.h>
#include <helpers\propertykitEx.h>
#include <imageproc\seekspots.h>

#define THIS_MODULENAME "AreaProfile"

extern char TVDB400_PLUGIN_NAME[APP_NAME_MAXLENGTH] ; 




class CAreaProfile : public CFilterGadget
{
  enum ProfileType
  {
    PROFILE_HORIZ,
    PROFILE_VERT ,
    PROFILE_BOTH
  };
  enum ProfileNorm
  {
    NOT_NORMALIZED ,
    NORMALIZED 
  };
  enum InterProfileOperation
  {
    OPER_NO ,
    OPER_ADD ,
    OPER_MULT ,
    OPER_SUB ,
    OPER_DIV
  };
private:
    CAreaProfile(void);
    void ShutDown();
    FXLockObject m_Lock;
public:
//
  CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	bool ScanProperties(LPCTSTR text, bool& Invalidate);
	bool PrintProperties(FXString& text);
	bool ScanSettings(FXString& text);

  int	 GetInputsCount();
  CInputConnector* GetInputConnector(int n);
  int	 GetOutputsCount();
  COutputConnector* GetOutputConnector(int n);
  CDuplexConnector * m_pDuplexConnector ;
  int GetDuplexCount();
  CDuplexConnector* GetDuplexConnector(int n);
  virtual void AsyncTransaction(
    CDuplexConnector* pConnector, CDataFrame* pParamFrame);

  ProfileType m_ProfType ;
  ProfileNorm m_Normalization ;
  CRect     m_ROI ;
  int       m_iDifferential ;
  cmplx     m_RepeatNormalized ;
  CSize     m_NSteps ;
  InterProfileOperation       m_Operation ;
  int       m_iViewMode ;
  bool      m_bReset ;

  DECLARE_RUNTIME_GADGET(CAreaProfile);
};
