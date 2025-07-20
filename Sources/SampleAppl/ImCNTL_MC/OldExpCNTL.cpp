// ExposureControl.cpp: implementation of the ExposureControl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ImCNTL.h"
#include "ExposureControl.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ExposureControl::ExposureControl()
{
  m_Channel = new CCSerIO( 6 , 4800 ) ;

  m_ExpControlBuffer[ 0 ] = ( char )0x80 ;
  m_ExpControlBuffer[ 1 ] = ( char )0x80 ;
  m_ExpControlBuffer[ 2 ] = ( char )( ( EXPOSURE_START + DEFAULT_EXPOSURE ) % 256 ) ;
  m_ExpControlBuffer[ 3 ] = ( EXPOSURE_START + DEFAULT_EXPOSURE ) / 256;
  m_ExpControlBuffer[ 4 ] = EXPOSURE_START % 256 ;
  m_ExpControlBuffer[ 5 ] = EXPOSURE_START / 256 ;
  m_ExpControlBuffer[ 6 ] = 0x04 ;

  m_PrIn = ( short* )&m_ExpControlBuffer[4];
  m_Ext = ( short* )&m_ExpControlBuffer[2];

  m_Channel->SendData( &m_ExpControlBuffer[ 2 ] , 5 ) ;
}

ExposureControl::~ExposureControl()
{
  delete m_Channel ;
}

int 
ExposureControl::GetExposure()
{
  return ( ( *m_Ext ) - ( *m_PrIn ) - 1 ) ;
}

int 
ExposureControl::SetExposure(int NScans)
{
  if ( NScans < 0 )
  {
   // m_ExpControlBuffer[ 6 ] = ( char )0x08 ;
    m_Channel->SendData( &m_ExpControlBuffer[ 2 ] , 5 ) ;
  }
  else
  {
    m_ExpControlBuffer[ 6 ] = 0x04 ;
    
    if ( NScans )
    {
      if ( NScans > MAX_EXPOSURE )
      {
        *m_Ext = ( *m_PrIn ) + MAX_EXPOSURE + 1 ;
        m_Channel->SendData( &m_ExpControlBuffer[ 2 ] , 5 ) ;
        return MAX_EXPOSURE ;
      }
      else
      {
        *m_Ext = ( *m_PrIn ) + NScans + 1 ;
        m_Channel->SendData( &m_ExpControlBuffer[ 2 ] , 5 ) ;
      }
    }
    else
    {
      *m_Ext = ( *m_PrIn ) + NScans + 1 ;
      m_Channel->SendData( &m_ExpControlBuffer[ 2 ] , 5 ) ;
    }
  }
  return 0 ;
}
