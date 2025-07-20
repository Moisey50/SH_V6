// FramesCounter.cpp: implementation of the FramesCounter class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FramesCounter.h"
#include <gadgets/QuantityFrame.h>
#include "TVGeneric.h"
#include <gadgets\ContainerFrame.h>
#include <helpers\FramesHelper.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_RUNTIME_GADGET_EX(FramesCounter, CFilterGadget, LINEAGE_GENERIC, TVDB400_PLUGIN_NAME);

FramesCounter::FramesCounter():
m_Count(0),
m_pResetPin(NULL)
{
    m_OutputMode=modeReplace;
	m_pInput = new CInputConnector(nulltype);
  m_pInput->SetName( _T( "CountIn" ) ) ;
  m_pResetPin = new CInputConnector(logical, _fn_frames_cntr, this);
  m_pResetPin->SetName( _T( "Reset" ) ) ;
	m_pOutput = new COutputConnector(quantity);
  m_pOutput->SetName( _T( "Frame#" ) ) ;
	Resume();
}

void FramesCounter::ShutDown()
{
    CFilterGadget::ShutDown();
	delete m_pInput;
	m_pInput = NULL;
	delete m_pResetPin;
	m_pResetPin = NULL;
	delete m_pOutput;
	m_pOutput = NULL;
}

int FramesCounter::GetInputsCount()
{
    return 2;
}

CInputConnector* FramesCounter::GetInputConnector(int n)
{
    switch (n)
    {
    case 0:
        return m_pInput;
    case 1:
        return m_pResetPin;
    }
    return NULL;
}

void FramesCounter::ResetCntr(CDataFrame* pDataFrame)
{
    FXAutolock al(m_InputLock);
    CBooleanFrame* bf=pDataFrame->GetBooleanFrame(DEFAULT_LABEL);
    if ((bf) && (bf->operator bool()))
        m_Count=0;
    pDataFrame->Release();
}

CDataFrame* FramesCounter::DoProcessing(const CDataFrame* pDataFrame)
{
    FXAutolock al(m_InputLock);
	if (pDataFrame)
	{
		CQuantityFrame* Frame = CQuantityFrame::Create(m_Count);
        m_Count++;
        Frame->CopyAttributes(pDataFrame);
		return Frame;
	}
    return NULL;
}

IMPLEMENT_RUNTIME_GADGET_EX(Decimator, CFilterGadget, LINEAGE_GENERIC, TVDB400_PLUGIN_NAME);

Decimator::Decimator():
  m_Count(0),
  m_DecimateFactor( 10 ),
  m_bBoolOut( FALSE )
{
  m_OutputMode=modeReplace;
  m_pInput = new CInputConnector(transparent, _fn_Decimator, this);
  m_pInput->SetName( _T( "Data" ) ) ;
  m_pResetInput = new CInputConnector( transparent , _fn_Reset , this );
  m_pResetInput->SetName( _T( "Reset" ) ) ;
  m_pOutput = new COutputConnector( transparent );
  Destroy();
}

void Decimator::ShutDown()
{
  CFilterGadget::ShutDown();
  delete m_pInput;
  m_pInput = NULL;
  delete m_pResetInput;
  m_pResetInput = NULL;
  delete m_pOutput;
  m_pOutput = NULL;
}

bool Decimator::PrintProperties(FXString& text)
{
  FXPropertyKit pk;
  pk.WriteInt( "DecimateFactor" , m_DecimateFactor );
  pk.WriteInt( "BoolOut" , m_bBoolOut );
  text = pk;
  return true;
}

bool Decimator::ScanProperties(LPCTSTR text, bool& Invalidate)
{
  FXPropertyKit pk(text);
  if ( pk.GetInt("DecimateFactor",m_DecimateFactor) )
    m_Count = 0 ;
  pk.GetInt( "BoolOut" , m_bBoolOut );
  return true;
}

bool Decimator::ScanSettings(FXString& text)
{
  text="template(Spin(DecimateFactor,0,5000)"
    ",ComboBox(BoolOut(No(0),Yes(1))))";
  return true;
}

IMPLEMENT_RUNTIME_GADGET_EX( GenRange , CFilterGadget , _T("Helpers") , TVDB400_PLUGIN_NAME );

GenRange::GenRange() :
m_Count( 0 ) ,
m_dBegin(0.0) ,
m_dStep( 1.0 ) ,
m_dEnd( 10.0 ) ,
m_iIntMask(0) ,
m_bIntFormat(0) ,
m_bLoop( FALSE ) ,
m_bWasReset( false ),
m_bWasInRange( false ),
m_bOutputSet( false ) ,
m_LabelFormat(_T("%.0f"))
{
  m_OutputMode = modeReplace;
  m_pInput = new CInputConnector( transparent );
  m_pInput->SetName( _T( "NextValue" ) ) ;
  m_pResetInput = new CInputConnector( transparent , _fn_ResetRange , this );
  m_pResetInput->SetName( _T( "Reset" ) ) ;
  m_pOutput = new COutputConnector( transparent );
  m_pOutput->SetName( _T( "Unformatted" ) ) ;
  m_pFormatted = new COutputConnector( text );
  m_pFormatted->SetName( _T( "Formatted" ) ) ;
  m_pDataInput = new CInputConnector( transparent , _fn_DataReceived , this ) ;
  m_pDataInput->SetName( _T( "ForLabeling" ) ) ;
  m_pLabeledData = new COutputConnector( transparent ) ;
  m_pLabeledData->SetName( "LabeledData" ) ;
  Resume();
}

void GenRange::ShutDown()
{
  CFilterGadget::ShutDown();
  delete m_pInput;
  m_pInput = NULL;
  delete m_pResetInput;
  m_pResetInput = NULL;
  delete m_pOutput;
  m_pOutput = NULL;
  delete m_pFormatted ;
  m_pFormatted = NULL ;
  delete m_pDataInput ;
  m_pDataInput = NULL ;
  delete m_pLabeledData ;
  m_pLabeledData = NULL ;
}

bool GenRange::PrintProperties( FXString& text )
{
  FXPropertyKit pk;
  pk.WriteDouble( _T("RangeBegin") , m_dBegin ) ;
  pk.WriteDouble( _T( "Step" ) , m_dStep ) ;
  pk.WriteDouble( _T( "RangeEnd" ) , m_dEnd ) ;
  pk.WriteString( _T( "FormatText" ) , m_Format ) ;
  pk.WriteInt( _T( "IsInteger" ) , m_bIntFormat ) ;
  pk.WriteInt( _T( "Loop" ) , m_bLoop ) ;
  pk.WriteString( _T( "DataLabelFormat" ) , m_LabelFormat ) ;
  text = pk;
  return true;
}

bool GenRange::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  FXPropertyKit pk( text );
  bool bInt = true ;
  FXString AsString ;
  if ( pk.GetString( _T( "RangeBegin" ) , AsString ) )
  {
    m_dBegin = atof( ( LPCTSTR ) AsString ) ;
    m_Count = 0 ;
  }
  if ( pk.GetString( _T( "Step" ) , AsString ) )
  {
    m_dStep = atof( ( LPCTSTR ) AsString ) ;
    m_Count = 0 ;
  }
  if ( pk.GetString( _T( "RangeEnd" ) , AsString ) )
  {
    m_dEnd = atof( ( LPCTSTR ) AsString ) ;
    m_Count = 0 ;
  }
  if ( pk.GetString( _T( "FormatText" ) , AsString ) )
  {
    AsString = AsString.Trim( _T( " \t" ) ) ;
    m_Format.Empty() ;
    for ( int i = 0 ; i < AsString.GetLength() ; i++ )
    {
      TCHAR Char = AsString[ i ] ;
      if ( Char == '\\' && ( i + 1 ) < AsString.GetLength() )
      {
        TCHAR NextChar = tolower( AsString[ i + 1 ] ) ;
        if ( NextChar == 'r' )
          m_Format += _T( '\r' ) ;
        else if ( NextChar == 'n' )
          m_Format == _T( '\n' ) ;
        i++ ;
      }
      else
        m_Format += Char ;
    }
    m_Count = 0 ;
  }
  pk.GetInt( _T( "IsInteger" ) , m_bIntFormat ) ;
  pk.GetInt( _T( "Loop" ) , m_bLoop ) ;
  pk.GetString( _T( "DataLabelFormat" ) , m_LabelFormat ) ;
  m_dLastValue = m_dBegin ;
  m_bWasReset = true ;
  return true;
}

bool GenRange::ScanSettings( FXString& text )
{
  text = "template("
    "EditBox(RangeBegin)"
    ",EditBox(Step)"
    ",EditBox(RangeEnd)"
    ",EditBox(FormatText)"
    ",EditBox(DataLabelFormat)"
    ",ComboBox(IsInteger(No(0),Yes(1)))"
    ",ComboBox(Loop(No(0),Yes(1)))"
    ")";
  return true;
}

CDataFrame* GenRange::DoProcessing( const CDataFrame* pDataFrame )
{
  double dOutputValue ;
  if ( (_tcscmp( pDataFrame->GetLabel() , _T( "Reset" ) ) == 0) 
    || m_bWasReset )
  {
    dOutputValue = m_dLastValue = m_dBegin ;
    m_Count = 0 ;
  }
  else
  {
    dOutputValue = ( m_dLastValue += m_dStep ) ;
    m_Count++ ;
  }
  bool bIsInRange = false ;
  if ( m_dStep >= 0. )
  {
    if ( m_dEnd >= m_dBegin )
      bIsInRange = ( m_dBegin <= dOutputValue && dOutputValue <= m_dEnd ) ;
    else
      bIsInRange = false ;
  }
  else
  {
    if ( m_dEnd < m_dBegin )
      bIsInRange = ( m_dBegin >= dOutputValue && dOutputValue >= m_dEnd ) ;
    else
      bIsInRange = false ;
  }
  
  if ( !bIsInRange && m_bLoop )
  {
    dOutputValue = m_dLastValue = m_dBegin ;
    bIsInRange = true ;
  }
  m_bWasReset = false ;
  
  if ( bIsInRange )
  {
    m_bWasInRange = true ;
    CTextFrame * pTextOut = ( !m_Format.IsEmpty() ) ? CTextFrame::Create() : NULL ;
    CQuantityFrame * pOut = ( m_bIntFormat ) ? 
      CQuantityFrame::Create(  ROUND( dOutputValue ) ) 
      : CQuantityFrame::Create( dOutputValue );
    if ( pTextOut )
    {
      if ( m_bIntFormat ) 
        pTextOut->GetString().Format( m_Format , ROUND(dOutputValue) ) ;
      else
        pTextOut->GetString().Format( m_Format , dOutputValue ) ;
      pTextOut->ChangeId( m_Count ) ;
      pTextOut->SetTime( GetHRTickCount() ) ;
      PutFrame( m_pFormatted , pTextOut ) ;
    }
    pOut->ChangeId( m_Count ) ;
    pOut->SetTime( GetHRTickCount() ) ;
    m_bOutputSet = true ;

    return pOut ;
  }
  m_bWasInRange = false ;
  return NULL ;
}

IMPLEMENT_RUNTIME_GADGET_EX(CompareFilter, CFilterGadget, 
                    LINEAGE_GENERIC, TVDB400_PLUGIN_NAME);

CompareFilter::CompareFilter():
m_Threshold(0),
m_operation(OP_EQUAL)
{
	m_pInput  = new CInputConnector(quantity);
	m_pOutput = new COutputConnector(logical);
	Resume();
}

void CompareFilter::ShutDown()
{
    CFilterGadget::ShutDown();
	delete m_pInput;
	m_pInput = NULL;
	delete m_pOutput;
	m_pOutput = NULL;
}

bool CompareFilter::PrintProperties(FXString& text)
{
	FXPropertyKit pk;
	pk.WriteInt("Operation",m_operation);
	pk.WriteString("Threshold",m_Threshold.ToString());
	text=pk;
	return true;
}

bool CompareFilter::ScanProperties(LPCTSTR text, bool& Invalidate)
{
    FXPropertyKit pk(text);
	pk.GetInt("Operation",m_operation);
	FXString tmpS;
	pk.GetString("Threshold",tmpS);
	m_Threshold.FromString(tmpS);
	return true;
}

bool CompareFilter::ScanSettings(FXString& text)
{
	FXString tmpS;
	text="template(ComboBox(Operation(";
	for (int i=OP_EQUAL; i<=OP_GREATER; i++)
	{
		if (i!=OP_EQUAL)
			text+=',';
		tmpS.Format("%s(%d)",Op2Str(i),i);
		text+=tmpS;
	}
	text+=")),EditBox(Threshold))";
	
    //text.Format("template(ComboBox(Quantity(False(%d),True(%d))),ComboBox(NoSync(False(%d),True(%d))),ComboBox(Manual(False(%d),True(%d))),Spin(FrameRate,1,25))",FALSE,TRUE,FALSE,TRUE,FALSE,TRUE);
    return true;
}


CDataFrame* CompareFilter::DoProcessing(const CDataFrame* pDataFrame)
{
	bool b = false;
	const CQuantityFrame* pFrame = pDataFrame->GetQuantityFrame(DEFAULT_LABEL);
	if (pFrame)
	{
		switch (m_operation)
		{
		case OP_EQUAL:
			b = (pFrame->operator == (m_Threshold));
			break;
		case OP_NOTEQUAL:
			b = (pFrame->operator != (m_Threshold));
			break;
		case OP_NOTGREATER:
			b = (pFrame->operator <= (m_Threshold));
			break;
		case OP_NOTLESS:
			b = (pFrame->operator >= (m_Threshold));
			break;
		case OP_LESS:
			b = (pFrame->operator < (m_Threshold));
			break;
		case OP_GREATER:
			b = (pFrame->operator > (m_Threshold));
			break;
		}
		CBooleanFrame* Frame = CBooleanFrame::Create(b);
        Frame->CopyAttributes(pDataFrame);
		return Frame;
	}
	return CBooleanFrame::Create(b);
}

int CompareFilter::Str2Op(LPCTSTR str)
{
	if (!strcmp(str, "=="))
		return OP_EQUAL;
	if (!strcmp(str, "!="))
		return OP_NOTEQUAL;
	if (!strcmp(str, "<="))
		return OP_NOTGREATER;
	if (!strcmp(str, ">="))
		return OP_NOTLESS;
	if (!strcmp(str, "<"))
		return OP_LESS;
	if (!strcmp(str, ">"))
		return OP_GREATER;
	ASSERT(FALSE);
	return -1;
}

LPCTSTR CompareFilter::Op2Str(int op)
{
	switch (op)
	{
	case OP_EQUAL:
		return "==";
	case OP_NOTEQUAL:
		return "!=";
	case OP_NOTGREATER:
		return "<=";
	case OP_NOTLESS:
		return ">=";
	case OP_LESS:
		return "<";
	case OP_GREATER:
		return ">";
	}
	ASSERT(FALSE);
	return "";
}
