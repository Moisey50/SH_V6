// BufferGadget.cpp: implementation of the Buffer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "helpers\FramesHelper.h"
#include "BufferGadget.h"
#include <gadgets\QuantityFrame.h>

IMPLEMENT_RUNTIME_GADGET_EX( Buffer , CFilterGadget , LINEAGE_GENERIC , TVDB400_PLUGIN_NAME );
#define BUFFER_LENGTH 40
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Buffer::Buffer() :
  m_pStack( NULL ) ,
  m_Transparent( TRUE ) ,
  m_BufferSize( BUFFER_LENGTH ) ,
  m_bPureFIFO( FALSE )
{
  m_OutputMode = modeReplace;
  m_pInput = new CInputConnector( transparent );
  m_pOutput = new COutputConnector( transparent );
  m_pControl = new CDuplexConnector( this , transparent , transparent );
  m_CrntOffset = 0;
  Resume();
}

void Buffer::ShutDown()
{
  CFilterGadget::ShutDown();
  delete m_pInput;
  m_pInput = NULL;
  delete m_pOutput;
  m_pOutput = NULL;
  delete m_pControl;
  m_pControl = NULL;
  ClearStack() ;
  if ( m_pStack )
    delete m_pStack;
  FXAutolock al( m_Lock ) ;
  ClearQueue() ;
}

CDataFrame* Buffer::GetFrame( int nmb )
{
  CDataFrame* pDF = NULL;
  if ( (m_pStack == NULL) 
    || (!m_pStack->Peep( m_pStack->ItemsInQueue() - 1 - nmb , pDF )) )
    return NULL;
  return pDF;
}

CDataFrame* Buffer::DoProcessing( const CDataFrame* pDataFrame )
{
  FXAutolock al( m_Lock );
  CDataFrame* Data = (CDataFrame*) pDataFrame; //we know what we do! )))
  if ( !m_bPureFIFO )
  {
    if ( m_pStack )
    {
      if ( m_pStack->PutQueueObject( Data ) )
        Data->AddRef();
      else
      {
        CDataFrame* df = NULL;
        if ( m_pStack->GetQueueObject( df ) )
          df->Release( df );
        if ( m_pStack->PutQueueObject( Data ) )
          Data->AddRef();
      }
      if ( m_Transparent )
      {
        CDataFrame* pDF = GetFrame( m_CrntOffset );
        if ( pDF )
          pDF->AddRef();
        return pDF;
      }
    }
  }
  else
  {
    while ( m_FIFO.size() >= (size_t)m_BufferSize )
    {
      CDataFrame * pFr = m_FIFO.front() ;
      pFr->Release() ;
      m_FIFO.pop() ;
    }
    Data->AddRef() ;
    m_FIFO.push( Data ) ;
  }
  return NULL;
}

void Buffer::ClearStack()
{
  FXAutolock ALock( m_Lock );
  if ( m_pStack )
  {
    while ( m_pStack->ItemsInQueue() )
    {
      CDataFrame* df;
      if ( m_pStack->GetQueueObject( df ) )
        df->Release( df );
    }
    m_CrntOffset = 0 ;
  }
}

int Buffer::GetDuplexCount()
{
  return 1;
}

CDuplexConnector* Buffer::GetDuplexConnector( int n )
{
  return (n) ? NULL : m_pControl;
}

void Buffer::AsyncTransaction( CDuplexConnector* pConnector , CDataFrame* pParamFrame )
{
  FXAutolock ALock( m_Lock );
  if ( !pParamFrame || Tvdb400_IsEOS( pParamFrame ) )
    return;
  const CTextFrame * pText = pParamFrame->GetTextFrame( _T( "BufControl" ) ) ;
  CBooleanFrame * pBool = pParamFrame->GetBooleanFrame() ;
  CQuantityFrame* QFrame = pParamFrame->GetQuantityFrame( DEFAULT_LABEL );
  if ( !m_bPureFIFO )
  {
    if ( m_pStack )
    {
      CQuantityFrame* QFrame = pParamFrame->GetQuantityFrame( DEFAULT_LABEL );
      if ( QFrame )
      {
        m_CrntOffset += (int) *QFrame; //was +=, can't understand why?!
        if ( m_CrntOffset < 0 )
          m_CrntOffset = 0;
        if ( m_CrntOffset >= m_pStack->ItemsInQueue() )
          m_CrntOffset = m_pStack->ItemsInQueue() - 1;
        CDataFrame* pDF = GetFrame( m_CrntOffset );
        if ( pDF )
        {
          pDF->AddRef();
          if ( (m_pOutput) && (!m_pOutput->Put( pDF )) )
            pDF->RELEASE( pDF );

        }
      }
      else // any other frame just push default packet
      {
        if ( pText )
        {
          FXString Content = pText->GetString() ;
          bool bOK = false ;
          if ( Content == _T( "Reset" ) )
          {
            ClearStack() ;
            bOK = true ;
          }
          else if ( Content.Find( _T( "SetPtr=" ) ) == 0 )
          {
            int iPtr = m_CrntOffset ;
            int iNewPtr = atoi( (LPCTSTR) Content + 7 ) ;
            if ( iNewPtr >= 0 && iNewPtr < m_pStack->ItemsInQueue() )
              m_CrntOffset = iNewPtr ;
            bOK = true ;
          }
          else if ( Content.Find( _T( "GetBufStatus" ) ) == 0 )
          {
            bOK = true ;
          }
          if ( bOK )
          {
            CTextFrame * pAnswer = CreateTextFrame( "" , "BufAnswer" ) ;
            pAnswer->GetString().Format( "Size=%d;Ptr=%d;" ,
              m_pStack->GetQueueSize() , m_CrntOffset ) ;
            PutFrame( m_pControl , pAnswer ) ;
          }
          else
          {
            CTextFrame * pAnswer = CreateTextFrame( "ERROR: "
              "Unknown command. \n"
              "Known are Reset, SetPtr=<ival> and GetBufStatus" ,
              "BufAnswer" ) ;
            PutFrame( m_pControl , pAnswer ) ;
          }
        }
        if ( !pBool || (bool) (*pBool) )
        {
          if ( m_CrntOffset < 0 )
            m_CrntOffset = 0;
          if ( m_CrntOffset >= m_pStack->ItemsInQueue() )
            m_CrntOffset = m_pStack->ItemsInQueue() - 1;
          CDataFrame* pDF = GetFrame( m_CrntOffset );
          if ( pDF )
          {
            pDF->AddRef();
            if ( (m_pOutput) && (!m_pOutput->Put( pDF )) )
              pDF->RELEASE( pDF );
          }
        }
      }
    }
  }
  else if ( pText  )
  {
    FXString Content = pText->GetString() ;
    CDataFrame * pNext = NULL ;
    if ( Content == _T( "Reset" ) )
    {
      ClearQueue() ;
      pNext = CreateTextFrame( "BuffOK" , "Buffer Cleared" ) ;
    }
    else if ( Content.Find( _T( "Next" ) ) == 0 )
    {
      if ( m_FIFO.size() )
      {
        pNext = m_FIFO.front() ;
        m_FIFO.pop() ;
      }
      else
        pNext = CreateTextFrame( "Buffer Empty" , "BuffError" ) ;
    }
    else if ( Content.Find( _T( "GetBufStatus" ) ) == 0 )
    {
      pNext = CreateTextFrame( "" , "BuffStatus" ) ;
      ((CTextFrame*)pNext)->GetString().Format( "BuffSize=%d;Filled=%d;" ,
        m_BufferSize , m_FIFO.size() ) ;
    }
    if ( !pNext )
    {
      pNext = CreateTextFrame( "ERROR: "
        "Unknown command. \n"
        "Known are Reset, Next and GetBufStatus" ,
        "BufError" ) ;
    }
    if ( pNext )
      PutFrame( m_pOutput , pNext ) ;
  }
  else if ( pBool || QFrame )
  {
    if ( m_FIFO.size() )
    {
      CDataFrame * pNext = m_FIFO.front() ;
      m_FIFO.pop() ;
      if ( pNext )
        PutFrame( m_pOutput , pNext ) ;
    }
  }
  pParamFrame->Release( pParamFrame );
}

bool Buffer::ScanSettings( FXString& text )
{
  text.Format( "template("
    "ComboBox(Transparent(true(1),false(0)))"
    ",Spin(BufferSize,1,500)"
    ",ComboBox(PureFIFO(true(1),false(0)))"
    ")" );
  return true;
}

void Buffer::CreateBuffer( int size )
{
  FXAutolock ALock( m_Lock );
  if ( (size != m_BufferSize) || (m_pStack == NULL) )
  {
    if ( m_pStack )
    {
      while ( m_pStack->ItemsInQueue() )
      {
        CDataFrame* df;
        if ( m_pStack->GetQueueObject( df ) )
        {
          df->Release( df );
        }
      }
      delete m_pStack;
    }
    m_pStack = new FXStaticQueue<CDataFrame*>( size );
    if ( m_pStack )
      m_BufferSize = size;
    else
      m_BufferSize = 0;
  }
}

bool Buffer::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  CFilterGadget::ScanProperties( text , Invalidate );
  FXPropertyKit pk( text );
  int buffersize = m_BufferSize;
  pk.GetInt( "Transparent" , m_Transparent );
  pk.GetInt( "BufferSize" , buffersize );
  BOOL bOldFIFOMode = m_bPureFIFO ;
  pk.GetInt( "PureFIFO" , m_bPureFIFO );
  if ( bOldFIFOMode != m_bPureFIFO )
  {
    if ( !m_bPureFIFO )
      CreateBuffer( buffersize );
    else
      ClearStack() ;

    ClearQueue() ;
  }
  else if ( buffersize != m_BufferSize )
    CreateBuffer( buffersize ) ;

  m_BufferSize = buffersize ;
  return true;
}

bool Buffer::PrintProperties( FXString& text )
{
  FXPropertyKit pk;
  CFilterGadget::PrintProperties( text );
  pk.WriteInt( "Transparent" , m_Transparent );
  pk.WriteInt( "BufferSize" , m_BufferSize );
  pk.WriteInt( "PureFIFO" , m_bPureFIFO );
  text += pk;
  return true;
}

