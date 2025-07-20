// Dispatcher.h.h : Implementation of the Dispatcher class


#include "StdAfx.h"
#include "Dispatcher.h"


// Do replace "TO DO: FOLDER NAME" to necessary gadget folder name in gadgets tree
USER_FILTER_RUNTIME_GADGET(Dispatcher,"Generic");


Dispatcher::Dispatcher(void)
{
  init();	// Mandatory in constructor
  m_iNOutputs = 1 ;
  m_bSelectByLabel = 0 ;
  SetLocking( OUTPUTS_LOCKING ) ;
}


/*
DoProcessing function processing data received/send by Input/Output connectors (do not relating to Duplex connector)
*/
CDataFrame* Dispatcher::DoProcessing(const CDataFrame* pDataFrame)
{
  if ( !pDataFrame )
    return NULL ;
  FXAutolock al( m_Lock ) ;
  if ( m_bSelectByLabel )
  {
    FXString Label = pDataFrame->GetLabel() ;
    for ( int i = 0 ; i < m_Keys.GetCount() ; i++ )
    {
      if ( m_Keys[i] == Label )
      {
        COutputConnector * pOut = GetOutputConnector( i ) ;
        ((CDataFrame*)pDataFrame)->AddRef() ;  // casting because const attribute
        if ( !pOut->Put( (CDataFrame*)pDataFrame) )
          ((CDataFrame*)pDataFrame)->Release();
        return NULL ;
      }
    }
    CFramesIterator * pIter = pDataFrame->CreateFramesIterator( nulltype ) ;
    if ( pIter )
    {
      CDataFrame * pNext = NULL ;
      while ( pNext = pIter->Next() )
      {
        FXString Label = pNext->GetLabel() ;
        for ( int i = 0 ; i < m_Keys.GetCount() ; i++ )
        {
          if ( m_Keys[i] == Label )
          {
            COutputConnector * pOut = GetOutputConnector( i ) ;
            pNext->AddRef() ;
            if ( !pOut->Put( pNext) )
              pNext->Release();
            break ;
          }
        }
      };
      delete pIter ;
    }
  }
  else  // select by text
  {
    const CTextFrame * pt = pDataFrame->GetTextFrame() ;
    FXString Text = pt->GetString() ;
    FXSIZE iPos = 0 ;
    FXString Token = Text.Tokenize( _T(" \t:") , iPos ) ;
    if ( !Token.IsEmpty() )
    {
      for ( int i = 0 ; i < (int)m_Keys.GetCount() ; i++ )
      {
        if ( m_Keys[i] == Token )
        {
          COutputConnector * pOut = GetOutputConnector( i ) ;
          ((CDataFrame*)pDataFrame)->AddRef() ;  // casting because const attribute
          if ( !pOut->Put( (CDataFrame*)pDataFrame) )
            ((CDataFrame*)pDataFrame)->Release();
          break ;
        }
      }
    }
  }
  return NULL;
}


/*
AsyncTransaction function associated with Duplex connector data processing
*/
void Dispatcher::AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame) 
{
  FXAutolock al( m_PinsLock ) ;
  pParamFrame->Release( pParamFrame );
}


/*
PropertiesRegistration() function define shown and editable gadget properties
*/
void Dispatcher::PropertiesRegistration() 
{
  addProperty(SProperty::SPIN	 , "NOutputs" ,	&m_iNOutputs , SProperty::Long	,	1		,	10	);
  static const char * pList = "By Text; By Label" ;
  addProperty(SProperty::COMBO , "Output Selection" ,	&m_bSelectByLabel , SProperty::Int	,	pList	);
  /* 
  To fill PropertiesRegistration() Use: addProperty(SProperty::PropertyBox ePropertyBox, LPCTSTR sName, void* ptrVar, SProperty::EVariableType eVariableType, ...) function as described below

  EDITBOX property type: 
  addProperty(EDITBOX,	FXString sName, void* ptrVar, SProperty::EVariableType eVariableType)
  SPIN property type: 
  addProperty(SPIN,		FXString sName, void* ptrVar, SProperty::EVariableType eVariableType, int iSpinMin, int iSpinMax)
  SPIN_BOOL property type: 
  addProperty(SPIN_BOOL,	FXString sName, void* ptrVar, SProperty::EVariableType eVariableType, int iSpinMin, int iSpinMax, bool *ptrBool)
  COMBO property type: 
  addProperty(COMBO,		FXString sName, void* ptrVar, SProperty::EVariableType eVariableType, const char *pList)

  Avaliable variable types: 

  SProperty::Int				
  SProperty::Long			
  SProperty::Double			
  SProperty::Bool			
  SProperty::String

  Important: variable type should be matched to property type

  Example:

  static const char* pList = "AA1; AA2; AA3; BB1; BB2; BB3; CC3";
  double		dProp1;
  long		lProp3;
  int			iProp4;
  bool		bProp4;
  FXString	sProp5;
  bool		bProp6;

  void UserExampleGadget::PropertiesRegistration() 
  {
  addProperty(SProperty::EDITBOX		,	"double1"	,	&dProp1		,	SProperty::Double		);
  addProperty(SProperty::SPIN			,	"long2"		,	&lProp3		,	SProperty::Long	,	3		,	9	);
  addProperty(SProperty::SPIN_BOOL	,	"bool_int4"	,	&iProp4		,	SProperty::SpinBool	,	-10		,	20, &bProp4	);
  addProperty(SProperty::COMBO		,	"combo5"	,	&sProp5		,	SProperty::String	,	pList	);
  addProperty(SProperty::EDITBOX		,	"bool6"		,	&bProp6		,	SProperty::Bool		);
  };	
  */
}


/*
ConnectorsRegistration() function define Input/Output/Duplex connectors (Duplex connectors will be processed by AsyncTransaction function)
*/
void Dispatcher::ConnectorsRegistration() 
{
  addInputConnector( transparent , _T("Data") ) ;
  addDuplexConnector( text , text , _T("Control") ) ;
  addOutputConnector( transparent ) ;

  /* 
  To fill ConnectorsRegistration() use next functions:

  addInputConnector (int dataType, LPCTSTR name)
  addOutputConnector(int dataType, LPCTSTR name)
  addDuplexConnector(int outDataType, int inDataType, LPCTSTR name)

  Use valiable datatypes:

  transparent // unspecified format, needs run-time compatibility checking
  nulltype    // bare time-stamp (synchronization data)
  vframe      // video frame
  text        // text
  wave        // sound
  quantity    // quantity (integer, double etc)
  logical     // logical value (boolean true/false)
  rectangle   // rectangle
  figure      // figure - set of points connected with segments
  metafile    // windows metafile drawing
  userdata	// specific user-defined data
  arraytype	// array

  for define connector to send/receive different datatypes, please use the function create and register datatype called "complex"

  int createComplexDataType(int numberOfDataTypes, basicdatatype dataType, ...) that return complex datatype ID

  Example:

  addOutputConnector( text , "OutputName1");
  addInputConnector( transparent, "InputName1");
  addInputConnector( createComplexDataType(2, rectangle, text), "InputName2");
  addOutputConnector( text , "OutputName2");
  addOutputConnector( createComplexDataType(3, rectangle, text, vframe) , "OutputName3");
  addDuplexConnector( transparent, transparent, "DuplexName1");
  addDuplexConnector( transparent, createComplexDataType(3, rectangle, text, vframe), "DuplexName2");
  */
}

bool Dispatcher::ScanProperties(LPCTSTR text, bool& Invalidate)
{
  FXAutolock al( m_Lock ) ;
  FXPropertyKit pk(text);
  pk.GetInt( "NOutputs" , m_iNOutputs ) ;
  pk.GetInt( "Output_Selection" , m_bSelectByLabel ) ;
  Invalidate = m_iNOutputs != GetOutputsCount() ;
  if ( m_iNOutputs < GetOutputsCount() )
  {
    RemoveOutputConnectorsAfter( m_iNOutputs ) ;
//     m_Keys.SetSize( m_iNOutputs ) ;
  }
  else
  {
    while ( m_iNOutputs > GetOutputsCount() )
      addOutputConnector( transparent ) ;
  }
  for ( int i = 0 ; i < m_iNOutputs ; i++ )
  {
    FXString NextKey , Value ;
    NextKey.Format( _T("Key_%d") , i + 1 ) ;
    if ( m_Keys.GetUpperBound() < i )
    {
      m_Keys.SetSize( i + 1 ) ;
      Value.Format( _T("%d") , i + 1 ) ;
    }
    pk.GetString( NextKey , Value ) ;
    if ( !Value.IsEmpty() )
      m_Keys[ i ] = Value ;
  }
  Status().WriteBool(STATUS_REDRAW, true);
  return true;
}

bool Dispatcher::PrintProperties(FXString& text)
{
  FXPropertyKit pk;
  CFilterGadget::PrintProperties(text);
  pk.WriteInt( "NOutputs" , m_iNOutputs ) ;
  pk.WriteInt( "Output_Selection" , m_bSelectByLabel ) ;
  for ( int i = 0 ; i < m_iNOutputs ; i++ )
  {
    FXString NextKey , Value ;
    NextKey.Format( _T("Key_%d") , i + 1 ) ;
    if ( m_Keys.GetUpperBound() < i )
    {
      m_Keys.SetSize( i + 1 ) ;
      m_Keys[ i ].Format( _T("%d") , i + 1 ) ;
    }
    pk.WriteString( NextKey , m_Keys[ i ] ) ;
  }
  text += pk;
  return true;

}

bool Dispatcher::ScanSettings(FXString& text)
{
  text = _T("template(Spin(NOutputs,1,10),"
    "ComboBox(Output_Selection(ByText(0),ByLabel(1)))");
  for ( int i = 0 ; i < m_iNOutputs ; i++ )
  {
    FXString NextKey ;
    NextKey.Format( _T(",EditBox(Key_%d)") , i + 1 ) ;
    text += NextKey ;
  }
  text += _T(")") ;
  return true;
}

