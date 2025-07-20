#pragma once

#include <Gadgets\vftempl.h>
#include <gadgets\gadbase.h>
#include "gadgets\textframe.h"
#include "gadgets\quantityframe.h"

class TextAggregatorSimple : public CCollectorGadget
{
	int      m_InputsQty;

	FXString m_AppendString;
  FXString m_Separator ;


	void InitInputs();


public:
	DECLARE_RUNTIME_GADGET(TextAggregatorSimple);

	TextAggregatorSimple();
	virtual void ShutDown();
	virtual CDataFrame* DoProcessing(CDataFrame const*const* frames, int nmb);

	int GetInputsCount() const;

	virtual bool PrintProperties(FXString& text);
	virtual bool ScanProperties(LPCTSTR text, bool& Invalidate);
	virtual bool ScanSettings(FXString& text);
};

class AggregateToTxt : public CCollectorGadget
{
  int      m_InputsQty;

  FXString m_AppendString;
  FXString m_Separator ;
  FXString m_EndOfString ;


  void InitInputs();


public:
  DECLARE_RUNTIME_GADGET( AggregateToTxt );

  AggregateToTxt();
  virtual void ShutDown();
  virtual CDataFrame* DoProcessing( CDataFrame const*const* frames , int nmb );

  int GetInputsCount() const;

  virtual bool PrintProperties( FXString& text );
  virtual bool ScanProperties( LPCTSTR text , bool& Invalidate );
  virtual bool ScanSettings( FXString& text );
};

class SetLabel : public CCollectorGadget
{
  int      m_InputsQty;

  void InitInputs();

public:
  DECLARE_RUNTIME_GADGET( SetLabel );

  SetLabel();
  virtual void ShutDown();
  virtual CDataFrame* DoProcessing( CDataFrame const*const* frames , int nmb );

  int GetInputsCount() const;
};