// FieldChooser.h : Declaration of the FieldChooser class

#pragma once
#include <gadgets\gadbase.h>
#include <gadgets\figureframe.h>
#include <gadgets\textframe.h>
#include <gadgets\containerframe.h>
#include "helpers\FramesHelper.h"

#define DEFAULT_N_FIELDS 5


class FXFigArray: public FXArray<CFigure> {};

class FieldChooser : public CFilterGadget
{
private:
  FieldChooser(void);
  void ShutDown();
  int   m_iNSamples ;
  bool  m_bOneSample ;
  FXIntArray m_Fields ;
  FXString   m_SelectedNames ;
  FXStringArray m_SelectedFieldNames ;
  FXStringArray m_NamesForDeletion ;
  int           m_iNLastAutomaticNames ;
  int           m_iNAutomaticNamesForDeletion ;
  int           m_iLastSelectedField ;
  FXString   m_LastString ;
  FXStringArray m_AllLastFieldNames ; // received from measurement path
//   FXFigArray m_Data ;
  FXLockObject m_Lock ;
  int        m_iWorkingMode ; // 0 - form arrays, 1 - send last point only
  BOOL       m_bXYLissajous ;
  bool       m_bAttributesChanged ;
  FXString   m_DescriptionPrefix ;
  FXString   m_Attributes ;
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
  void ProcessDescriptionFrame( LPCTSTR pDescription ) ;
  DECLARE_RUNTIME_GADGET(FieldChooser);
  
  
  int BuildIndexes(void);
  int BuildFieldNames(void);
};
