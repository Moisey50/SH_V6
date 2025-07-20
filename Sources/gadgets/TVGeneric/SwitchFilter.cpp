// SwitchFilter.cpp: implementation of the SwitchFilter class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SwitchFilter.h"
#include <gadgets\QuantityFrame.h>
#include "TVGeneric.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_RUNTIME_GADGET_EX( Switch , CControledFilter , LINEAGE_GENERIC , TVDB400_PLUGIN_NAME );

Switch::Switch() :
  m_bOn( false )
{}

CDataFrame* Switch::DoProcessing( const CDataFrame* pDataFrame , const CDataFrame* pParamFrame )
{
  if ( Tvdb400_IsEOS( pDataFrame ) )
  {
    m_bOn = FALSE ;
    ((CDataFrame*) pDataFrame)->AddRef(); // should be released once in parent class
    return (CDataFrame*) pDataFrame ;
  }
  if ( pParamFrame )
  {
    const CBooleanFrame* bFrame = pParamFrame->GetBooleanFrame( DEFAULT_LABEL );
    if ( bFrame )
      m_bOn = (int)(*bFrame);
    const CQuantityFrame * pFrameAsQauntity = pParamFrame->GetQuantityFrame() ;
    if ( pFrameAsQauntity )
      m_bOn =((int) (*pFrameAsQauntity) != 0 ) ;

  }
  if ( !pDataFrame || !m_bOn )
    return NULL;

  ((CDataFrame*)pDataFrame)->AddRef(); // should be released once in parent class
  return (CDataFrame*)pDataFrame ;
}

bool Switch::ScanSettings( FXString& txt )
{
  txt.Format( "template(ComboBox(SwitchOn(true(1),false(0))))" );
  return true;
}

bool Switch::ScanProperties( LPCTSTR txt , bool& Invalidate )
{
  CControledFilter::ScanProperties( txt , Invalidate );
  FXPropertyKit pk( txt );
  pk.GetInt( "SwitchOn" , (int&)m_bOn );
  return true;
}

bool Switch::PrintProperties( FXString& txt )
{
  FXPropertyKit pk;
  CControledFilter::PrintProperties( txt );
  pk.WriteInt( "SwitchOn" , m_bOn );
  txt += pk;
  return true;
}
