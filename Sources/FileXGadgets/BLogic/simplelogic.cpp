// B_or.cpp: implementation of the B_or class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BLogic.h"
#include "simplelogic.h"
#include <gadgets\quantityframe.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction B_or
//////////////////////////////////////////////////////////////////////
IMPLEMENT_RUNTIME_GADGET_EX( B_or , CCollectorGadget , LINEAGE_LOGIC , TVDB400_PLUGIN_NAME );

B_or::B_or()
{
  m_pOutput = new COutputConnector( logical );
  CreateInputs( 2 , transparent );
  Resume();
}

void B_or::ShutDown()
{
  CCollectorGadget::ShutDown();
}

CDataFrame* B_or::DoProcessing( CDataFrame const*const* frames , int nmb )
{
  bool res = false;
  const CDataFrame* reFrame = ( *frames );
  if ( reFrame == NULL )
    return NULL ;
  for ( int i = 0; i < nmb; i++ )
  {
    ASSERT( *( frames + i ) != NULL );
    const CBooleanFrame* LogicalFrame = ( *( frames + i ) )->GetBooleanFrame( DEFAULT_LABEL );
    if ( LogicalFrame )
      res |= ( ( bool ) ( *LogicalFrame ) );
    else
    {
      const CQuantityFrame* quantityframe = ( *( frames + i ) )->GetQuantityFrame( DEFAULT_LABEL );
      if ( quantityframe )
      {
        res |= ( 0 != ( int ) ( *quantityframe ) );
      }
      else
        res = false;
    }
  }
  CBooleanFrame* pResult = CBooleanFrame::Create( res );
  pResult->CopyAttributes( reFrame );
  return pResult;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction B_and
//////////////////////////////////////////////////////////////////////
IMPLEMENT_RUNTIME_GADGET_EX( B_and , CCollectorGadget , LINEAGE_LOGIC , TVDB400_PLUGIN_NAME );

B_and::B_and()
{
  m_pOutput = new COutputConnector( logical );
  CreateInputs( 2 , transparent );
  Resume();
}

void B_and::ShutDown()
{
  CCollectorGadget::ShutDown();
}

CDataFrame* B_and::DoProcessing( CDataFrame const*const* frames , int nmb )
{
  bool res = true;
  const CDataFrame* reFrame = ( *frames );
  if ( reFrame == NULL )
    return NULL ;
  for ( int i = 0; i < nmb; i++ )
  {
    ASSERT( *( frames + i ) != NULL );
    const CBooleanFrame* LogicalFrame = ( *( frames + i ) )->GetBooleanFrame( DEFAULT_LABEL );
    if ( LogicalFrame )
      res &= ( ( bool ) ( *LogicalFrame ) );
    else
    {
      const CQuantityFrame* quantityframe = ( *( frames + i ) )->GetQuantityFrame( DEFAULT_LABEL );
      if ( quantityframe )
      {
        res &= ( 0 != ( int ) ( *quantityframe ) );
      }
      else
        res = false;
    }
  }
  CBooleanFrame* pResult = CBooleanFrame::Create( res );
  pResult->CopyAttributes( reFrame );
  return pResult;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction B_xor
//////////////////////////////////////////////////////////////////////
IMPLEMENT_RUNTIME_GADGET_EX( B_xor , CCollectorGadget , LINEAGE_LOGIC , TVDB400_PLUGIN_NAME );

B_xor::B_xor()
{
  m_pOutput = new COutputConnector( logical );
  CreateInputs( 2 , transparent );
  Resume();
}

void B_xor::ShutDown()
{
  CCollectorGadget::ShutDown();
}

CDataFrame* B_xor::DoProcessing( CDataFrame const*const* frames , int nmb )
{
  bool res;
  const CDataFrame* reFrame = ( *frames );
  if ( reFrame == NULL )
    return NULL ;
  for ( int i = 0; i < nmb; i++ )
  {
    ASSERT( *( frames + i ) != NULL );
    const CBooleanFrame* LogicalFrame = ( *( frames + i ) )->GetBooleanFrame( DEFAULT_LABEL );
    if ( LogicalFrame )
    {
      if ( i == 0 )
        res = ( ( bool ) ( *LogicalFrame ) );
      else
        res ^= ( ( bool ) ( *LogicalFrame ) );
    }
    else
    {
      const CQuantityFrame* quantityframe = ( *( frames + i ) )->GetQuantityFrame( DEFAULT_LABEL );
      if ( quantityframe )
      {
        if ( i == 0 )
          res = ( 0 != ( int ) ( *quantityframe ) );
        else
          res ^= ( 0 != ( int ) ( *quantityframe ) );
      }
      else
        res = false;
    }
  }
  CBooleanFrame* pResult = CBooleanFrame::Create( res );
  pResult->CopyAttributes( reFrame );
  return pResult;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction B_and
//////////////////////////////////////////////////////////////////////

IMPLEMENT_RUNTIME_GADGET_EX( B_Not , CFilterGadget , LINEAGE_LOGIC , TVDB400_PLUGIN_NAME );
B_Not::B_Not( void )
{
  m_pInput = new CInputConnector( logical );
  m_pOutput = new COutputConnector( logical );
  Resume();
}

void B_Not::ShutDown()
{
  CFilterGadget::ShutDown();
  delete m_pInput;
  m_pInput = NULL;
  delete m_pOutput;
  m_pOutput = NULL;
}

CDataFrame* B_Not::DoProcessing( const CDataFrame* pDataFrame )
{
  const CBooleanFrame* LogicalFrame = pDataFrame->GetBooleanFrame();
  CBooleanFrame* pResult = NULL ;
  if ( LogicalFrame )
    pResult = CBooleanFrame::Create( !LogicalFrame->operator bool() );
  else
  {
    const CQuantityFrame * pQF = pDataFrame->GetQuantityFrame() ;
    if ( pQF )
      pResult = CBooleanFrame::Create( pQF->operator int() == 0 ) ;
  }
  if ( pResult )
    pResult->CopyAttributes( pDataFrame );
  return pResult;
}