// TVRotate.h : Implementation of the Rotate class


#include "StdAfx.h"
#include <Gadgets\VideoFrame.h>
#include <Gadgets\TextFrame.h>
#include <Gadgets\vftempl.h>
#include <imageproc\rotate.h>
#include "TVRotate.h"

IMPLEMENT_RUNTIME_GADGET_EX( Rotate , CFilterGadget , LINEAGE_VIDEO".frame" , TVDB400_PLUGIN_NAME );

Rotate::Rotate( void ) :
m_AngleDeg( 0 ) ,
m_RotateRgn( false ) ,
m_Rect( 0 , 0 , 0 , 0 ) ,
m_Mode( WM_ROTATE )
{
  m_pInput = new CInputConnector( vframe );
  m_pOutput = new COutputConnector( vframe );
  m_pControlPin = new CDuplexConnector( this , text , text );
  Resume();
}

void Rotate::ShutDown()
{
  CFilterGadget::ShutDown();
  delete m_pInput;
  m_pInput = NULL;
  delete m_pOutput;
  m_pOutput = NULL;
  delete m_pControlPin;
  m_pControlPin = NULL;
}

CDataFrame* Rotate::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( VideoFrame , pDataFrame );
  pTVFrame frame = makecopyTVFrame( VideoFrame );

  if ( m_RotateRgn )
  {
    if ( ( m_Rect.Width() == 0 ) || ( m_Rect.Height() == 0 ) )
    {
      freeTVFrame( frame );
      return NULL;
    }
    switch ( m_Mode )
    {
      case WM_ROTATE:
        _rotate_frame_rect( frame , m_Rect , DegToRad( m_AngleDeg ) );
        break ;
      case WM_HSKEW:
        _skew_frame_rect( frame , m_Rect , DegToRad( m_AngleDeg ) );
        break ;
    }
  }
  else
    _rotate_frame( frame , DegToRad( m_AngleDeg ) );

  CVideoFrame* retVal = CVideoFrame::Create( frame );
  retVal->CopyAttributes( pDataFrame );
  return retVal;
}

bool Rotate::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  CFilterGadget::ScanProperties( text , Invalidate );
  FXString tmpS;
  FXPropertyKit pk( text );
  if ( pk.GetString( "Angle" , tmpS ) )
    m_AngleDeg = atof( tmpS );
  pk.GetInt( "RotateRgn" , ( int& )m_RotateRgn );
  CRect rc;
  if ( ( m_RotateRgn ) &&
    pk.GetString( "Rect" , tmpS ) &&
    ( sscanf( tmpS , "%d,%d,%d,%d" , &rc.left , &rc.top , &rc.right , &rc.bottom ) == 4 ) )
  {
    m_Rect = rc;
  }
  pk.GetInt( "Mode" , ( int& )m_Mode ) ;
  Invalidate = true;
  return true;
}

bool Rotate::PrintProperties( FXString& text )
{
  CFilterGadget::PrintProperties( text );
  CString tmpS; tmpS.Format( "%.2f" , m_AngleDeg );
  FXPropertyKit pc;
  pc.WriteString( "Angle" , tmpS );
  pc.WriteInt( "RotateRgn" , m_RotateRgn );
  if ( m_RotateRgn )
  {
    tmpS.Format( "%d,%d,%d,%d" , m_Rect.left , m_Rect.top , m_Rect.right , m_Rect.bottom );
    pc.WriteString( "Rect" , tmpS );
  }
  pc.WriteInt( "Mode" , ( int )m_Mode ) ;
  text += pc;
  return true;
}

bool Rotate::ScanSettings( FXString& text )
{
  CFilterGadget::ScanSettings( text );
  FXString tmpS;
  if ( m_RotateRgn )
    tmpS.Format( "template("
    "ComboBox(RotateRgn(RotateFrame(%d),RotateRegion(%d)))"
    ",EditBox(Angle),EditBox(Rect)"
    ",ComboBox(Mode(Rotate(0),HSkew(1),VSkew(2))))" ,
    false , true );
  else
    tmpS.Format( "template("
    "ComboBox(RotateRgn(RotateFrame(%d),RotateRegion(%d)))"
    ",EditBox(Angle)"
    ",ComboBox(Mode(Rotate(0),HSkew(1),VSkew(2))))" ,
    false , true );
  text += tmpS;
  return true;
}

void Rotate::AsyncTransaction( CDuplexConnector* pConnector , CDataFrame* pDataFrame )
{
  CTextFrame* TextFrame = pDataFrame->GetTextFrame( DEFAULT_LABEL );
  if ( ( !TextFrame ) ||
    ( TextFrame->GetString().GetLength() == 0 ) ||
    ( TextFrame->GetString() == "?" ) ||
    ( TextFrame->GetString().CompareNoCase( "help" ) == 0 )
    )
  { // it's a reqest of info
    pDataFrame->Release();
    FXString tmpS;
    PrintProperties( tmpS );
    CTextFrame* retV = CTextFrame::Create( tmpS );
    retV->ChangeId( NOSYNC_FRAME );
    if ( !m_pControlPin->Put( retV ) )
      retV->RELEASE( retV );
    return;
  }
  CString pk = TextFrame->GetString();
  bool Invalidate;
  ScanProperties( pk , Invalidate );
  pDataFrame->Release();

}
