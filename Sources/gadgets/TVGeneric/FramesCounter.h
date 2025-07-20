// FramesCounter.h: interface for the FramesCounter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FRAMESCOUNTER_H__509667B3_11A3_4C05_9232_FF9A337D3214__INCLUDED_)
#define AFX_FRAMESCOUNTER_H__509667B3_11A3_4C05_9232_FF9A337D3214__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Gadgets\gadbase.h>
#include <Gadgets\QuantityFrame.h>
#include <helpers\FramesHelper.h>

class FramesCounter : public CFilterGadget
{
	int	              m_Count;
    CInputConnector*  m_pResetPin;
    FXLockObject       m_InputLock;
protected:
	FramesCounter();
public:
	virtual void ShutDown();
private:
    int GetInputsCount();
    CInputConnector* GetInputConnector(int n);
	virtual CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
    void ResetCntr(CDataFrame* pDataFrame);
	static friend void CALLBACK _fn_frames_cntr(CDataFrame* pDataFrame, void* pGadget, CConnector* lpInput)
	{
		((FramesCounter*)pGadget)->ResetCntr(pDataFrame);
	}
	DECLARE_RUNTIME_GADGET(FramesCounter);
};

class Decimator : public CFilterGadget
{
  CInputConnector * m_pResetInput ;
  int	              m_Count;
  int               m_DecimateFactor ;
  BOOL              m_bBoolOut ;
  FXLockObject      m_InputLock;
protected:
  Decimator();
public:
  virtual void ShutDown();
  virtual bool PrintProperties(FXString& text);
  virtual bool ScanProperties(LPCTSTR text, bool& Invalidate);
  virtual bool ScanSettings(FXString& text);
  virtual int GetInputsCount() { return 2 ; } ;
  virtual CInputConnector * GetInputConnector( int n )
  {
    if ( n == 0 )
      return m_pInput ;
    if ( n == 1 )
      return m_pResetInput ;
    return NULL ;
  }
private:
  static friend void CALLBACK _fn_Decimator( CDataFrame* pDataFrame ,
    void* pGadget , CConnector* lpInput )
  {
    Decimator * pDecimator = ( ( Decimator* )pGadget ) ;
    if ( ++( pDecimator->m_Count ) >= pDecimator->m_DecimateFactor )
    {
      pDecimator->m_Count = 0 ;
      if ( pDecimator->m_bBoolOut )
      {
        pDataFrame->Release() ;
        pDataFrame = CBooleanFrame::Create( true ) ;
      }
      if ( !pDecimator->m_pOutput || !pDecimator->m_pOutput->Put( pDataFrame ) )
        pDataFrame->Release() ;
    }
    else
    {
      pDataFrame->Release() ;
      if ( pDecimator->m_bBoolOut )
      {
        pDataFrame = CBooleanFrame::Create( true ) ;
        if ( !pDecimator->m_pOutput || !pDecimator->m_pOutput->Put( pDataFrame ) )
          pDataFrame->Release() ;
      }
    }
  }
  static friend void CALLBACK _fn_Reset( CDataFrame* pDataFrame ,
    void* pGadget , CConnector* lpInput )
  {
    Decimator * pDecimator = ( ( Decimator* )pGadget ) ;
    pDecimator->m_Count = 0 ;
    pDataFrame->Release() ;
  }
  DECLARE_RUNTIME_GADGET( Decimator );
};

#define INT_MASK_BEGIN 1
#define INT_MASK_STEP  2
#define INT_MASK_END   4

class GenRange : public CFilterGadget
{
  CInputConnector * m_pResetInput ;
  COutputConnector * m_pFormatted ;
  CInputConnector * m_pDataInput ;
  COutputConnector * m_pLabeledData ;
  int	              m_Count;
  double            m_dBegin ;
  double            m_dStep ;
  double            m_dEnd ;
  double            m_dLastValue ;
  int               m_iIntMask ;
  BOOL              m_bIntFormat ;
  BOOL              m_bLoop ;
  bool              m_bWasReset ;
  bool              m_bWasInRange ;
  bool              m_bOutputSet ;
  FXString          m_Format ;
  FXString          m_LabelFormat ;
  FXLockObject      m_InputLock;
protected:
  GenRange();
public:
  virtual void ShutDown();

  virtual bool PrintProperties( FXString& text );
  virtual bool ScanProperties( LPCTSTR text , bool& Invalidate );
  virtual bool ScanSettings( FXString& text );
  virtual int GetInputsCount() { return 3 ; } ;
  virtual CInputConnector * GetInputConnector( int n )
  {
    switch ( n )
    {
      case 0: return m_pInput ;
      case 1: return m_pResetInput ;
      case 2: return m_pDataInput ;
    }
    return NULL ;
  }
  virtual int GetOutputsCount() { return 3; } ;
  virtual COutputConnector * GetOutputConnector( int n )
  {
    switch ( n )
    {
      case 0: return m_pOutput ;
      case 1: return m_pFormatted ;
      case 2: return m_pLabeledData ;
    }
    return NULL ;
  }
private:
  static friend void CALLBACK _fn_ResetRange( CDataFrame* pDataFrame ,
    void* pGadget , CConnector* lpInput )
  {
    GenRange * pGenRange = ( ( GenRange* ) pGadget ) ;
    pGenRange->m_Count = 0 ;
    pGenRange->m_bWasReset = true ;
    pDataFrame->Release() ;
  };
  static friend void CALLBACK _fn_DataReceived( CDataFrame* pDataFrame ,
    void* pGadget , CConnector* lpInput )
  {
    GenRange * pGenRange = (GenRange*) pGadget ;
    COutputConnector * pPin = pGenRange->GetOutputConnector( 2 ) ;
    if ( pPin && pPin->IsConnected()
      && pGenRange->m_bOutputSet 
      && !pGenRange->m_LabelFormat.IsEmpty() )
    {
      FXString Label = pDataFrame->GetLabel() ;
      Label.Format( (LPCTSTR) pGenRange->m_LabelFormat , 
        pGenRange->m_dLastValue ) ;
      pDataFrame->SetLabel( Label ) ;
      PutFrame( pPin , pDataFrame ) ;
      pGenRange->m_bOutputSet = false ;
    }
    else
      pDataFrame->Release() ;
  };

  virtual CDataFrame* DoProcessing( const CDataFrame* pDataFrame ) ;
  DECLARE_RUNTIME_GADGET( GenRange );
};

class CompareFilterDialog;
class CompareFilter : public CFilterGadget
{
	int m_operation;
	enum { OP_EQUAL, OP_NOTEQUAL, OP_NOTGREATER, OP_NOTLESS, OP_LESS, OP_GREATER };
	CGenericQuantity m_Threshold;
protected:
	CompareFilter();
public:
	virtual void ShutDown();
	virtual bool PrintProperties(FXString& text);
	virtual bool ScanProperties(LPCTSTR text, bool& Invalidate);
	virtual bool ScanSettings(FXString& text);
private:
	virtual CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	int Str2Op(LPCTSTR str);
	LPCTSTR Op2Str(int op);
	friend class CompareFilterDialog;
	DECLARE_RUNTIME_GADGET(CompareFilter);
};

#endif // !defined(AFX_FRAMESCOUNTER_H__509667B3_11A3_4C05_9232_FF9A337D3214__INCLUDED_)
