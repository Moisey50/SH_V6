// Dispatcher.h : Declaration of the Dispatcher class

#pragma once
#include "helpers\UserBaseGadget.h"

class Dispatcher : public UserBaseGadget
{
protected:

	Dispatcher(void);

  int    m_iNOutputs ;
  int    m_bSelectByLabel ;  // otherwise select by text
  FXStringArray m_Keys ;
  FXLockObject  m_Lock ;

public:

  void ConnectorsRegistration();

	CDataFrame* DoProcessing(const CDataFrame* pDataFrame);

	void PropertiesRegistration();

	void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);
  bool ScanProperties(LPCTSTR text, bool& Invalidate);
  bool PrintProperties(FXString& text);
  bool ScanSettings(FXString& text);
	DECLARE_RUNTIME_GADGET(Dispatcher);
};
