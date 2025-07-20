// LineProfile.cpp : Implementation of the LineProfile class


#include "StdAfx.h"
#include "LineProfile.h"
#include <math\intf_sup.h>
#include <imageproc\imagebits.h>
#include <gadgets\FigureFrame.h>
#include <gadgets\VideoFrame.h>
#include <gadgets\TextFrame.h>
#include <gadgets\ContainerFrame.h>

#define PASSTHROUGH_NULLFRAME(vfr, fr)			\
{												\
	if (!(vfr) || ((vfr)->IsNullFrame()))	    \
    {	                                        \
		return NULL;			                    \
    }                                           \
}


inline void GetSection( const pTVFrame frame , CPoint a , CPoint b , CFigure* DataBuffer )
{
  int dx = b.x - a.x; int dy = b.y - a.y;
  int x = a.x , y = a.y; DPOINT data;
  DataBuffer->RemoveAll();
  if ( dx == 0 )
  {
    for ( y = a.y; y != b.y; y += ((dy < 0) ? -1 : 1) )
    {
      LPBYTE v = __getdata_I_XY( frame , x , y );
      if ( v )
      {
        data.x = y - a.y;
        data.y = *v;
        DataBuffer->AddPoint( data );
      }
    }
  }
  else if ( dy == 0 )
  {
    for ( x = a.x; x != b.x; x += ((dx < 0) ? -1 : 1) )
    {
      LPBYTE v = __getdata_I_XY( frame , x , y );
      if ( v )
      {
        data.x = x - a.x;
        data.y = *v;
        DataBuffer->AddPoint( data );
      }
    }
  }
  else if ( abs( dx ) > abs( dy ) )
  {

    double r = ((double) dy) / dx;
    for ( x = a.x; x != b.x; x += ((dx < 0) ? -1 : 1) )
    {
      y = a.y + (int) ((x - a.x)*r + 0.5);
      LPBYTE v = __getdata_I_XY( frame , x , y );
      if ( v )
      {
        double xx = x - a.x , yy = y - a.y;
        data.x = sqrt( xx*xx + yy * yy );
        data.y = *v;
        DataBuffer->AddPoint( data );
      }
    }
  }
  else
  {
    double r = ((double) dx) / dy;
    for ( y = a.y; y != b.y; y += ((dy < 0) ? -1 : 1) )
    {
      x = a.x + (int) ((y - a.y)*r + 0.5);
      LPBYTE v = __getdata_I_XY( frame , x , y );
      if ( v )
      {
        double xx = x - a.x , yy = y - a.y;
        data.x = sqrt( xx*xx + yy * yy );
        data.y = *v;
        DataBuffer->AddPoint( data );
      }
    }
  }
}


IMPLEMENT_RUNTIME_GADGET_EX( LineProfile , CFilterGadget , "Video.statistics" , TVDB400_PLUGIN_NAME );

LineProfile::LineProfile( void )
{
  m_LineSel.SetRectEmpty();
  m_pInput = new CInputConnector( vframe );
  m_pOutput = new COutputConnector( vframe*figure );
  m_pControl = new CDuplexConnector( this , text , text );
  Resume();
}

void LineProfile::ShutDown()
{
  CFilterGadget::ShutDown();
  delete m_pControl;
  m_pControl = NULL;
  delete m_pInput;
  m_pInput = NULL;
  delete m_pOutput;
  m_pOutput = NULL;
}

void LineProfile::AsyncTransaction( CDuplexConnector* pConnector , CDataFrame* pParamFrame )
{
  FXAutolock al( m_TransactionLock );
  CTextFrame* TextFrame = pParamFrame->GetTextFrame( DEFAULT_LABEL );
  if ( TextFrame )
  {
    FXPropertyKit pk = TextFrame->GetString();
    FXString    res;
    if ( pk.GetString( "Line" , res ) )
    {
      if ( sscanf( res , "%d,%d,%d,%d" ,
        &m_LineSel.left ,
        &m_LineSel.top ,
        &m_LineSel.right ,
        &m_LineSel.bottom ) != 4 )
      {
        m_LineSel.SetRectEmpty();
      }
    }
    else
      m_LineSel.SetRectEmpty();
  }
  pParamFrame->RELEASE( pParamFrame );
}

CDataFrame* LineProfile::DoProcessing( const CDataFrame* pDataFrame )
{
  FXAutolock al( m_TransactionLock );
  CFigureFrame* retVal = NULL;
  const CVideoFrame* vFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( vFrame , pDataFrame );
  if ( !m_LineSel.IsRectEmpty() )
  {
    retVal = CFigureFrame::Create();
    GetSection( vFrame ,
      CPoint( m_LineSel.left , m_LineSel.top ) ,
      CPoint( m_LineSel.right , m_LineSel.bottom ) ,
      retVal );
    retVal->CopyAttributes( vFrame );
    retVal->Attributes()->WriteString( "color" , "0xff0000" );
  }
  if ( retVal )
  {
    if ( !pDataFrame->IsContainer() )
    {
      CContainerFrame* cVal = CContainerFrame::Create();
      cVal->CopyAttributes( pDataFrame );
      cVal->AddFrame( pDataFrame );
      cVal->AddFrame( retVal );
      return cVal;
    }
    else
      return retVal;
  }
  else
  {
    ((CDataFrame*) pDataFrame)->AddRef();
    return (CDataFrame*) pDataFrame;
  }
}

bool LineProfile::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  FXPropertyKit pk( text );
  return true;
}

bool LineProfile::PrintProperties( FXString& text )
{
  return false;
}

int LineProfile::GetDuplexCount()
{
  return 1;
}

CDuplexConnector* LineProfile::GetDuplexConnector( int n )
{
  return ((!n) ? m_pControl : NULL);
}

