#include "StdAfx.h"
#include "TextAggregatorSimple.h"


IMPLEMENT_RUNTIME_GADGET_EX( TextAggregatorSimple , 
  CCollectorGadget , "Helpers" , TVDB400_PLUGIN_NAME );

IMPLEMENT_RUNTIME_GADGET_EX( AggregateToTxt , 
  CCollectorGadget , "Helpers" , TVDB400_PLUGIN_NAME );
IMPLEMENT_RUNTIME_GADGET_EX( SetLabel ,
  CCollectorGadget , "Helpers" , TVDB400_PLUGIN_NAME );

#define PASSTHROUGH_NULLFRAME(vfr, fr)		\
{											\
	if (!(vfr) || ((vfr)->IsNullFrame()))	\
{	                                    \
	return NULL;                        \
}                                       \
}

#define SETUP_PINS_MIN 2
#define SETUP_PINS_MAX 50

TextAggregatorSimple::TextAggregatorSimple() :
    m_InputsQty(2)
  , m_Separator(_T(" "))
{
	RemoveInputs();

	m_Lock.Lock();
	m_AppendString.Empty();
	m_Lock.Unlock();

	m_pOutput = new COutputConnector(text);
  InitInputs() ;

	Resume();
}

void TextAggregatorSimple::ShutDown()
{
	CCollectorGadget::ShutDown();
}


CDataFrame* TextAggregatorSimple::DoProcessing(CDataFrame const*const* frames, int nmb)
{
	FXString txt;
	for (int i = 0; i < nmb && i < m_InputsQty; i++)
	{
    const CTextFrame* tf = ( *( frames + i ) )->GetTextFrame();
    if ( tf )
    {
      txt += tf->GetString();
      if ( i < m_InputsQty - 1 )
        txt += m_Separator ;
    }
  }
	if(txt.GetLength() > 0)
  {
    CTextFrame* retVal = CTextFrame::Create();
    retVal->CopyAttributes( *frames );
    retVal->SetLabel( "TextAggregator" );
    retVal->SetString( txt );
    return retVal;
  }
  return NULL ;
}

void TextAggregatorSimple::InitInputs()
{
	Suspend();
	ClearBuffers();

	CreateInputs(m_InputsQty, text, false);
	Resume();
  Status().WriteBool( STATUS_REDRAW , true );
}

bool TextAggregatorSimple::PrintProperties(FXString& text)
{
	CCollectorGadget::PrintProperties(text);
	FXPropertyKit pc;
  pc.WriteInt( "InputPins" , m_InputsQty ) ;
  pc.WriteString( _T("Separator") , m_Separator ) ;
	text+=pc;
	return true;
}

bool TextAggregatorSimple::ScanProperties(LPCTSTR text, bool& Invalidate)
{
	int pinsReq = SETUP_PINS_MIN;
	bool res = false;
	CCollectorGadget::ScanProperties(text , Invalidate);
	FXPropertyKit pc(text);

	res = pc.GetInt( "InputPins" , pinsReq );
	if( res  &&  (pinsReq != m_InputsQty) )
	{
		m_InputsQty = pinsReq;
		InitInputs();
	}
  pc.GetString( _T("Separator") , m_Separator ) ;
	return true ;
}

bool TextAggregatorSimple::ScanSettings(FXString& text)
{
  FXString Pattern = "template(Spin(InputPins,2,16),"
    "EditBox(Separator))" ;
  text += Pattern ;
	return true;
}

// AggregateToTxt gadget takes Quantity frames from many inputs 
// and converts to one string of text

AggregateToTxt::AggregateToTxt() :
m_InputsQty( 2 )
, m_Separator( _T( " " ) )
, m_EndOfString( _T("\n") )
{
  RemoveInputs();

  m_Lock.Lock();
  m_AppendString.Empty();
  m_Lock.Unlock();

  m_pOutput = new COutputConnector( text );
  InitInputs() ;

  Resume();
}

void AggregateToTxt::ShutDown()
{
  CCollectorGadget::ShutDown();
}

CDataFrame* AggregateToTxt::DoProcessing( 
  CDataFrame const*const* frames , int nmb )
{
  FXString txt;
  for ( int i = 0; i < nmb && i < m_InputsQty; i++ )
  {
    const CQuantityFrame* qf = ( *( frames + i ) )->GetQuantityFrame();
    if ( qf )
      txt += (qf) ? qf->ToString() : _T( "Omitted" ) ;
    if ( i < m_InputsQty - 1 )
      txt += m_Separator ;
    else
      txt += m_EndOfString ;
  }
  if ( txt.GetLength() > 0 )
  {
    CTextFrame* retVal = CTextFrame::Create();
    retVal->CopyAttributes( *frames );
    retVal->SetLabel( "DataAggregator" );
    retVal->SetString( txt );
    return retVal;
  }
  return NULL ;
}

void AggregateToTxt::InitInputs()
{
  Suspend();
  ClearBuffers();

  CreateInputs( m_InputsQty , transparent , false );
  Resume();
  Status().WriteBool( STATUS_REDRAW , true );
}

bool AggregateToTxt::PrintProperties( FXString& text )
{
  bool res = false;
  CCollectorGadget::PrintProperties( text );
  FXPropertyKit pc;
  pc.WriteInt( "InputPins" , m_InputsQty ) ;
  FXString Separator ;
  for ( int i = 0 ; i < m_Separator.GetLength() ; i++ )
  {
    switch ( m_Separator[ i ] )
    {
      case _T( '\n' ): Separator += "\n" ; break ;
      case _T( '\r' ): Separator += "\r" ; break ;
      case _T( '\0' ): Separator += "\0" ; break ;
      default: Separator += m_Separator[ i ] ; break ;
    }
  }

  pc.WriteString( _T( "Separator" ) , Separator ) ;
  FXString EndOfString ;
  for ( int i = 0 ; i < m_EndOfString.GetLength() ; i++ )
  {
    switch ( m_EndOfString[ i ] )
    {
      case _T( '\n' ): EndOfString += "\n" ; break ;
      case _T( '\r' ): EndOfString += "\r" ; break ;
      case _T( '\0' ): EndOfString += "\0" ; break ;
      default: EndOfString += m_EndOfString[ i ] ; break ;
    }
  }
  pc.WriteString( _T( "EndOfString" ) , EndOfString ) ;
  text += pc ;
  return res;
}

bool AggregateToTxt::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  int pinsReq = SETUP_PINS_MIN;
  bool res = false;
  CCollectorGadget::ScanProperties( text , Invalidate );
  FXPropertyKit pc( text );

  res = pc.GetInt( "InputPins" , pinsReq );
  if ( res && ( pinsReq != m_InputsQty ) )
  {
    m_InputsQty = pinsReq;
    InitInputs();
  }
  FXString Separator ;
  if ( pc.GetString( _T( "Separator" ) , Separator ) )
  {
    m_Separator.Empty() ;
    for ( int i = 0 ; i < Separator.GetLength() ; i++ )
    {
      switch ( Separator[ i ] )
      {
        case _T( '\n' ): m_Separator += "\n" ; break ;
        case _T( '\r' ): m_Separator += "\r" ; break ;
        case _T( '\0' ): m_Separator += "\0" ; break ;
        default: m_Separator += Separator[ i ] ; break ;
      }
    }
  }
  FXString EndOfString ;
  if ( pc.GetString( _T( "EndOfString" ) , EndOfString ) )
  {
    m_EndOfString.Empty() ;
    for ( int i = 0 ; i < EndOfString.GetLength() ; i++ )
    {
      switch ( EndOfString[ i ] )
      {
        case _T( '\n' ): m_EndOfString += "\n" ; break ;
        case _T( '\r' ): m_EndOfString += "\r" ; break ;
        case _T( '\0' ): m_EndOfString += "\0" ; break ;
        default: m_EndOfString += EndOfString[ i ] ; break ;
      }
    }
  }
  return true ;
}

bool AggregateToTxt::ScanSettings( FXString& text )
{
  FXString Pattern = "template(Spin(InputPins,2,16),"
    "EditBox(Separator),"
    "EditBox(EndOfString))" ;
  text += Pattern ;
  return true;
}

// SetLabel gadget takes Quantity frames from many inputs 
// and converts to one string of text

SetLabel::SetLabel() :
m_InputsQty( 2 )
{
  RemoveInputs();

  m_pOutput = new COutputConnector( transparent );
  InitInputs() ;

  Resume();
}

void SetLabel::ShutDown()
{
  CCollectorGadget::ShutDown();
}

CDataFrame* SetLabel::DoProcessing(
  CDataFrame const*const* frames , int nmb )
{
  const CDataFrame * pFrame = *frames ; // first input is data for labeling
  const CTextFrame * pText = (*( frames + 1 ))->GetTextFrame() ;
  if ( !pText )
    return NULL ;
  
  CDataFrame * pCopy = pFrame->Copy() ;
  pCopy->SetLabel( pText->GetString() ) ;

  return pCopy ;
}

void SetLabel::InitInputs()
{
  Suspend();
  ClearBuffers();

  CreateInputs( m_InputsQty , transparent , false );
  Resume();
  Status().WriteBool( STATUS_REDRAW , true );
}
