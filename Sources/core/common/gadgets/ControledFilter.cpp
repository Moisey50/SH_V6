// ControledFilter.cpp: implementation of the CControledFilter class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <gadgets\ControledFilter.h>
#include <gadgets\ContainerFrame.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_RUNTIME_GADGET_EX( CControledFilter , CFilterGadget , LINEAGE_GENERIC , _T( "" ) );

CControledFilter::CControledFilter( int inputtype , int outputtype , int duplextype ) :
  m_bNoSync( true ) ,
  m_NeverSync( false ) ,
  m_LastDataFrame( NULL ) ,
  m_LastParamFrame( NULL )
{
  m_pInput = new CInputConnector( transparent );
  m_pOutput = new COutputConnector( transparent );
  m_pControl = new CDuplexConnector( this , transparent , transparent );
  Resume();
}

void CControledFilter::ShutDown()
{
  CFilterGadget::ShutDown();
  delete m_pInput;  m_pInput = NULL;
  delete m_pOutput; m_pOutput = NULL;
  delete m_pControl; m_pControl = NULL;
  m_pControl = NULL;

  if ( m_LastDataFrame )
    m_LastDataFrame->RELEASE( m_LastDataFrame );
  m_LastDataFrame = NULL;
  if ( m_LastParamFrame )
    m_LastParamFrame->RELEASE( m_LastParamFrame );
  m_LastParamFrame = NULL;
}

int CControledFilter::GetDuplexCount()
{
  return 1;
}

CDuplexConnector* CControledFilter::GetDuplexConnector( int n )
{
  return ( ( !n ) ? m_pControl : NULL );
}

void CControledFilter::AsyncTransaction( CDuplexConnector* pConnector , CDataFrame* pParamFrame )
{
  ASSERT( pParamFrame );
  m_Lock.Lock();
  if ( !Tvdb400_IsEOS( pParamFrame ) )
  {
    bool bNoSync = ( pParamFrame->GetId() == NOSYNC_FRAME ) || m_NeverSync;
    if ( m_LastParamFrame )
      m_LastParamFrame->RELEASE( m_LastParamFrame );
    m_LastParamFrame = pParamFrame;
    if ( bNoSync ) // sync off
    {
      Process( m_LastDataFrame , m_LastParamFrame );
      if ( m_LastDataFrame )
      {
        m_LastDataFrame->RELEASE( m_LastDataFrame );
        m_LastDataFrame = NULL;
      }
      if ( m_LastParamFrame )
      {
        m_LastParamFrame->RELEASE( m_LastParamFrame );
        m_LastParamFrame = NULL;
      }
    }
    else // sync on
    {
      if ( m_LastDataFrame )
      {
        if ( m_LastDataFrame->GetId() == m_LastParamFrame->GetId() )
        {
          Process( m_LastDataFrame , m_LastParamFrame );
          m_LastDataFrame->RELEASE( m_LastDataFrame );
          m_LastDataFrame = NULL;
          m_LastParamFrame->RELEASE( m_LastParamFrame );
          m_LastParamFrame = NULL;
        }
        else if ( m_bNoSync && ( m_LastDataFrame->GetId() < m_LastParamFrame->GetId() ) )
        {
          Process( m_LastDataFrame , NULL );
          m_LastDataFrame->RELEASE( m_LastDataFrame );
          m_LastDataFrame = NULL;
        }
      }
    }
    m_bNoSync = bNoSync;
  }
  else
  {
    pParamFrame->RELEASE( pParamFrame );
    m_bNoSync = true; // no more param frames expected
  }
  m_Lock.Unlock();
}

int CControledFilter::DoJob()
{
  CDataFrame* pDataFrame = NULL;
  while ( ( m_pInput ) && ( m_pInput->Get( pDataFrame ) ) )
  {
    m_Lock.Lock();
    ASSERT( pDataFrame );
    if ( Tvdb400_IsEOS( pDataFrame ) ) // if EOS - just pass dataframe through without processing for common types gadgets
    {
      if ( m_LastDataFrame )
      {
        if ( ( m_bNoSync ) || ( !m_pControl->IsConnected() ) )
          Process( m_LastDataFrame , NULL );
        m_LastDataFrame->RELEASE( m_LastDataFrame );
        m_LastDataFrame = NULL;
      }
      Process( pDataFrame , NULL );
      pDataFrame->RELEASE( pDataFrame );
      if ( m_LastParamFrame )
        m_LastParamFrame->RELEASE( m_LastParamFrame );
      m_LastParamFrame = NULL;
    }
    else switch ( m_Mode )
    {
      case mode_reject:
        pDataFrame->RELEASE( pDataFrame );
        if ( m_LastParamFrame )
          m_LastParamFrame->RELEASE( m_LastParamFrame );
        m_LastParamFrame = NULL;
        break;
      case mode_transmit:
        if ( !m_pOutput->Put( pDataFrame ) )
          pDataFrame->RELEASE( pDataFrame );
        if ( m_LastParamFrame )
          m_LastParamFrame->RELEASE( m_LastParamFrame );
        m_LastParamFrame = NULL;
        break;
      case mode_process:
      {
        double ts = ::GetHRTickCount();
        if ( m_LastDataFrame )
        {
          if ( ( m_bNoSync ) || ( !m_pControl->IsConnected() ) )
            Process( m_LastDataFrame , NULL );
          m_LastDataFrame->RELEASE( m_LastDataFrame );
          m_LastDataFrame = pDataFrame;
        }
        else
        {
          if ( ( m_bNoSync ) || ( !m_pControl->IsConnected() ) )
          {
            Process( pDataFrame , NULL );
            pDataFrame->Release( pDataFrame ) ;
            pDataFrame = NULL ;
          }
          else
          {
            if ( m_LastParamFrame )
            {
              if ( m_LastParamFrame->GetId() == pDataFrame->GetId() )
              {
                Process( pDataFrame , m_LastParamFrame );
                pDataFrame->RELEASE( pDataFrame );
                pDataFrame = NULL;
                m_LastParamFrame->RELEASE( m_LastParamFrame );
                m_LastParamFrame = NULL;
              }
              else
              {
                if ( m_LastParamFrame->GetId() < pDataFrame->GetId() )
                {
                  m_LastParamFrame->Release( m_LastParamFrame ) ;
                  m_LastParamFrame = NULL ;
                  m_LastDataFrame = pDataFrame ;
                }
                else
                {
                  pDataFrame->RELEASE( pDataFrame ) ;
                  pDataFrame = NULL ;
                }
              }
            }
            else
              m_LastDataFrame = pDataFrame;
          }
        }
        if ( m_LastParamFrame && m_LastDataFrame )
        {
          if ( m_NeverSync || m_bNoSync || ( m_LastParamFrame->GetId() == m_LastDataFrame->GetId() ) )
          {
            Process( m_LastDataFrame , m_LastParamFrame );
            m_LastDataFrame->RELEASE( m_LastDataFrame );
            m_LastDataFrame = NULL;
            m_LastParamFrame->RELEASE( m_LastParamFrame );
            m_LastParamFrame = NULL;
          }
        }
        AddCPUUsage( GetHRTickCount() - ts );
      }
    }
    m_Lock.Unlock();
  }
  return WR_CONTINUE;
}

void CControledFilter::Process( const CDataFrame* pDataFrame , const CDataFrame* pParamFrame )
{
  CDataFrame* pResultFrame = DoProcessing( pDataFrame , pParamFrame );
  if ( pResultFrame && ( !m_pOutput->Put( pResultFrame ) ) )
    pResultFrame->RELEASE( pResultFrame );
}

CDataFrame* CControledFilter::DoProcessing( const CDataFrame* pDataFrame , const CDataFrame* pParamFrame )
{
  return NULL;
}
