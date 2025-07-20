#include "stdafx.h"
// #include <windows.h>
// #include <gdiplusinit.h>
#include "GMarking.h"
#include "helpers/FramesHelper.h"
#include "fxfc/FXRegistry.h"
#include "math/PlaneGeometry.h"

#define THIS_MODULENAME "GMarking"

int DecodePoints( FXPropKit2& Src , CmplxVector& Result )
{
  Result.clear() ;
  FXSIZE iPos = 0 ;
  while ( (iPos = Src.Find( '(' , iPos )) >= 0 )
  {
    iPos++ ;
    FXString Token = Src.Tokenize( "," , iPos ) ;
    double dX = atof( Token ) ;
    Token = Src.Tokenize( ",)" , iPos ) ;
    double dY = atof( Token ) ;
    cmplx cVal( dX , dY ) ;
    Result.push_back( cVal ) ;
    iPos++ ;
  }
  return (int)Result.size() ;
}

int GMarkObject::AnalyseCoordsForCircle()
{
  if ( m_Type != GM_Arc )
    return -1 ;
  if ( m_Coords.size() >= 3 )
  {
    double dDist1 = abs( m_Coords[ 0 ] - m_Coords[ 1 ] ) ;
    double dDist2 = abs( m_Coords[ 0 ] - m_Coords[ 2 ] ) ;
    double dDist3 = abs( m_Coords[ 2 ] - m_Coords[ 1 ] ) ;

    double dMaxDist = max( max( dDist1 , dDist2 ) , dDist3 ) ;
    if ( dMaxDist < EPSILON )
      m_bClose = FALSE ;
    else
    {
      CLine2d Side( m_Coords[ 0 ] , m_Coords[ 1 ] ) ;
      double dDist2to01 = Side.dist( m_Coords[ 2 ] ) ;
      m_bClose = ( fabs( dDist2to01 ) > EPSILON ) ;
    }
  }
  else
    m_bClose = FALSE ;

  return m_bClose ;
}

int GMarkObject::ScanProperties( LPCTSTR pAsText )
{
  FXPropKit2 pk( pAsText ) ;
  FXString Type ;
  FXString Text ;
  int iRes = (pk.GetString( "Text" , m_Text ) == true);
  pk.MakeLower() ;
  if ( pk.GetString( "type" , Type ) )
  {
    if ( Type == "text" )
      m_Type = GM_Text ;
    else if ( Type == "figure" )
      m_Type = GM_Figure ;
    else if ( Type == "Arc" )
      m_Type = GM_Arc ;
    else
      m_Type = GM_Unknown ;
    iRes++ ;
  }
  iRes += ( pk.GetUIntOrHex( "color" , (UINT&)m_Color ) == true ) ;
  iRes += ( pk.GetInt( "size" , ( int& ) m_Size )
    || pk.GetInt( "sz" , ( int& ) m_Size ) ) == true ;
  if ( m_Type == GM_Text )
    iRes += ( pk.GetUIntOrHex( "back" , ( UINT& ) m_Back ) == true ) ;
  else if ( m_Type == GM_Figure )
    iRes += ( pk.GetInt( "close" , m_bClose ) == true ) ;
  else if ( m_Type == GM_Arc )
  {
    iRes += pk.GetDouble( "rad" , m_dRadius_pix ) ;
    iRes += pk.GetDouble( "beg_deg" , m_dAngFrom_deg ) ;
    iRes += pk.GetDouble( "end_deg" , m_dAngTo_deg ) ;
    m_bIsCircle = ( m_dAngFrom_deg == 0.
      && ( ( m_dAngTo_deg == 0. ) || fabs( m_dAngTo_deg - 360. ) < 1e-10 ) ) ;
  }
  iRes += ( pk.GetInt( "thick" , ( int& ) m_Thickness ) == true ) 
    || ( pk.GetInt( "thickness" , ( int& ) m_Thickness ) == true ) ;
  if ( pk.GetString( "points" , m_CoordsAsText ) ) 
  {
    DecodePoints( ( FXPropKit2& ) m_CoordsAsText , m_Coords ) ;
    iRes++ ;
    AnalyseCoordsForCircle() ;
  }
  return iRes ;
}

GMarkObject::GMarkObject( LPCTSTR pAsText )
{
  if ( !pAsText )
  {
    m_Type = GM_Text ;
    m_Color = 0x0000ff ;
    m_Size = 10 ;
    m_Back = 1 ;
    m_Thickness = 1 ;
    m_bClose = FALSE ;
    m_CoordsAsText = "(0.5,0.5);" ;
    m_Coords.push_back( cmplx( 0.5 , 0.5 ) ) ;
    m_Text = "Unfilled" ;
    m_dRadius_pix = 100. ;
    m_dAngFrom_deg = 0. ; // default arc is circle
    m_dAngTo_deg = 360. ;
  }
  else
    ScanProperties( pAsText ) ;

  GetProperties() ;
  m_bIsInitialized = true ;
}

GMarkObject::GMarkObject( GMarkObject& Origin )
{
  memcpy( this , &Origin , (LPBYTE)(&m_Text) - (LPBYTE)this ) ;
  m_Text = Origin.m_Text ;
  m_ParamsAsString = Origin.m_ParamsAsString ;
  m_CoordsAsText = Origin.m_CoordsAsText ;
  m_Coords = Origin.m_Coords ;
}

GMarkObject& GMarkObject::operator=( GMarkObject& Origin )
{
  memcpy( this , &Origin , ( LPBYTE ) ( &m_Text ) - ( LPBYTE ) this ) ;
  m_Text = Origin.m_Text ;
  m_ParamsAsString = Origin.m_ParamsAsString ;
  m_CoordsAsText = Origin.m_CoordsAsText ;
  m_Coords = Origin.m_Coords ;
  return *this ;
}

CTextFrame * GMarkObject::GetTextFrame( const cmplx& cSize )
{
  if ( m_Type != GM_Text || !m_bIsInitialized || !m_Coords.size() )
    return NULL ;

  cmplx cPt = m_Coords[0] ;
  if ( (fabs(cPt.real()) <= 1) && (fabs( cPt.imag() ) <= 1.) )
  {
    cPt._Val[ _RE ] *= cSize.real() ;
    cPt._Val[ _IM ] *= cSize.imag() ;
  }
  CTextFrame * pOut = CreateTextFrameEx( cPt , m_Text , m_Color , m_Size ) ;
  if ( pOut )
  {
    if ( m_Back != 1 )
      pOut->Attributes()->WriteInt( "back" , m_Back ) ;
    return pOut ;
  }
  return NULL ;
}

CFigureFrame * GMarkObject::GetFigureFrame( const cmplx& cVFSize )
{
  if ( m_Type != GM_Figure || !m_bIsInitialized )
    return NULL ;

  CmplxVector Converted ;
  for ( size_t i = 0 ; i < m_Coords.size() ; i++ )
  {
    if ( ( fabs( m_Coords[ i ].real() ) <= 1. ) && ( fabs( m_Coords[ i ].imag() ) <= 1. ) )
    {
      Converted.push_back( cmplx( m_Coords[ i ].real() * cVFSize.real() ,
        m_Coords[ i ].imag() * cVFSize.imag() ) );
    }
    else
      Converted.push_back( m_Coords[ i ] ) ;
  }
  if ( m_bClose )
    Converted.push_back( Converted[ 0 ] ) ;
  CFigureFrame * pOut = CreateFigureFrameEx( 
    Converted.data() , (int) Converted.size() , m_Color  ) ;
  if ( pOut )
  {
    if ( m_Thickness != 1 )
      pOut->Attributes()->WriteInt( "thickness" , m_Thickness ) ;
    if ( (m_Size != 1) && (m_Coords.size() == 1) )
      pOut->Attributes()->WriteInt( "Sz" , m_Size ) ;

    return pOut ;
  }
  return NULL ;
}

CDataFrame * GMarkObject::GetArcFrame( const cmplx& cVFSize )
{
  if ( m_Type != GM_Arc || !m_bIsInitialized )
    return NULL ;

  CContainerFrame * pCont = CContainerFrame::Create() ;
  cmplx cCenter( m_Coords[ 0 ] ) ;
  if ( m_bClose && (m_Coords.size() >= 3) )
  {
    cmplx p0 = m_Coords[ 0 ] ;
    cmplx p1 = m_Coords[ 1 ] ; 
    cmplx p2 = m_Coords[ 2 ];
    if ( ( fabs( p0.real() ) <= 1. ) && ( fabs( p0.imag() ) <= 1. ) )
    {
      p0._Val[ _RE ] *= cVFSize.real() ;
      p0._Val[ _IM ] *= cVFSize.imag() ;
    }
    if ( ( fabs( p1.real() ) <= 1. ) && ( fabs( p1.imag() ) <= 1. ) )
    {
      p1._Val[ _RE ] *= cVFSize.real() ;
      p1._Val[ _IM ] *= cVFSize.imag() ;
    }
    if ( ( fabs( p2.real() ) <= 1. ) && ( fabs( p2.imag() ) <= 1. ) )
    {
      p2._Val[ _RE ] *= cVFSize.real() ;
      p2._Val[ _IM ] *= cVFSize.imag() ;
    }
    cmplx Segm0( p0 - p2 ) ;
    cmplx Segm1( p1 - p2 ) ;
    cmplx cCent0 = p2 + 0.5 * Segm0 ;
    cmplx cCent1 = p2 + 0.5 * Segm1 ;
    cmplx cOrtho0 = GetOrthoLeftOnVF( Segm0 ) ;
    cmplx cOrtho1 = GetOrthoLeftOnVF( Segm1 ) ;
    CLine2d OrthoLine0 , OrthoLine1 ;
    OrthoLine0.ByPointAndDir( cCent0 , cOrtho0 ) ;
    OrthoLine1.ByPointAndDir( cCent1 , cOrtho1 ) ;
    if ( OrthoLine0.intersect( OrthoLine1 , cCenter ) )
    {
      m_dRadius_pix = abs( cCenter - p0 ) ;
      cmplx cVect0 = p0 - cCenter ;
      cmplx cVect1 = p1 - cCenter ;
      cmplx cVect2 = p2 - cCenter ;
      double dAng1to0 = -arg( cVect1 / cVect0 ) ;
      double dAng2to0 = -arg( cVect2 / cVect0 ) ;

      double dAng0 = -arg( p0 - cCenter ) * 360. / M_2PI ;
      double dAng1 = dAng0 + (dAng1to0 * 360. / M_2PI) ;
      double dAng2 = dAng0 + (dAng2to0 * 360. / M_2PI) ;
      if ( sign( dAng1to0 ) == sign( dAng2to0 ) )
      {
        m_dAngFrom_deg = dAng0 ;
        m_dAngTo_deg = ( fabs( dAng1to0 ) < fabs( dAng2to0 ) ) ? dAng2 : dAng1 ;
      }
      else
      {
        m_dAngFrom_deg = dAng1 ;
        m_dAngTo_deg = dAng2 ;
      }
      m_bIsCircle = FALSE ;
      pCont->AddFrame( CreatePtFrameEx( p0 , m_Color ) ) ;
      pCont->AddFrame( CreatePtFrameEx( p1 , m_Color ) ) ;
      pCont->AddFrame( CreatePtFrameEx( p2 , m_Color ) ) ;
    }
  }
  if ( ( fabs( cCenter.real() ) <= 1. ) && ( fabs( cCenter.imag() ) <= 1. ) )
  {
    cCenter._Val[ _RE ] *= cVFSize.real() ;
    cCenter._Val[ _IM ] *= cVFSize.imag() ;
  }
  pCont->AddFrame( CreatePtFrameEx( cCenter , m_Color ) ) ;

  CmplxVector Arc ;

  double dAngleRange_deg = (m_bIsCircle ) ? 360. : ( m_dAngTo_deg - m_dAngFrom_deg ) ;
  double dAngleRange_rad = M_2PI * dAngleRange_deg / 360. ;
  double dLength_pix = m_dRadius_pix * fabs(dAngleRange_rad) ;
  double dAngle = m_dAngFrom_deg * M_2PI / 360. ;

  int iNPoints = ( int ) ( dLength_pix / 5. ) ;
  if ( iNPoints == 0 )
    iNPoints = 1 ;
  double dStep = dAngleRange_rad / iNPoints ;
  Arc.resize( ++iNPoints ) ;
  for ( int i = 0 ; i < iNPoints ; i++ )
  {
    cmplx cVect = polar( m_dRadius_pix , -dAngle - dStep * i ) ; 
    cVect += cCenter ;
    Arc[i] = cVect ;
  }

  CFigureFrame * pOut = CreateFigureFrameEx(
    Arc.data() , ( int ) Arc.size() , m_Color ) ;
  if ( pOut )
  {
    if ( m_Thickness != 1 )
      pOut->Attributes()->WriteInt( "thickness" , m_Thickness ) ;
    if ( pCont->GetFramesCount() )
    {
      pCont->AddFrame( pOut ) ;
      return pCont ;
    }
    else
      pCont->Release() ;
    return pOut ;
  }
  return NULL ;
}

CDataFrame * GMarkObject::GetFrameForOutput( const cmplx& cVFSize )
{
  switch ( m_Type )
  {
    case GM_Text: return GetTextFrame( cVFSize ) ;
    case GM_Figure: return GetFigureFrame( cVFSize ) ;
    case GM_Arc: return GetArcFrame( cVFSize ) ;
  }
  return NULL ;
}

void GMarkObject::GetProperties()
{
  FXPropKit2 pk ;
//  pk.WriteString( "Type" , GetTypeAsString() ) ;
  pk.WriteUIntNotDecimal( "color" , m_Color , _T( 'X' ) ) ;
  if ( m_Type == GM_Text )
    pk.WriteUIntNotDecimal( "back" , m_Back , _T('X') ) ;
  if ( ( m_Type == GM_Text ) || ( ( m_Type == GM_Figure ) && ( m_Coords.size() == 1 ) ) )
    pk.WriteInt( "Sz" , (int)m_Size ) ;
  if ( m_Type == GM_Figure )
  {
    pk.WriteInt( "thickness" , ( int ) m_Thickness ) ;
    pk.WriteInt( "close" , m_bClose ) ;
  }
  else if ( m_Type == GM_Arc )
  {
    pk.WriteInt( "thick" , ( int ) m_Thickness ) ;
    pk.WriteDouble( "rad" , m_dRadius_pix , _T( "%.3f" ) ) ;
    pk.WriteDouble( "beg_deg" , m_dAngFrom_deg , _T( "%.3f" ) ) ;
    pk.WriteDouble( "end_deg" , m_dAngTo_deg , _T( "%.3f" ) ) ;

  }
  m_ParamsAsString = pk ;
}

USER_FILTER_RUNTIME_GADGET(GMarking, "Helpers");

GMarking::GMarking() 
{
  m_OutputMode = modeReplace;
  
  init();
};

void GMarking::OnScanSettings()
{
  if ( m_bRescanProp )
  {
    PropertiesReregistration() ;
    m_bRescanProp = false ;
  }
  return ;
}

void GMarking::ConfigParamChange( LPCTSTR pName ,
  void* pObject , bool& bInvalidate , bool& bInitRescan )
{
  GMarking * pGadget = ( GMarking* ) pObject;
  if ( pGadget )
  {
    LPCTSTR pFoundName = NULL ;
    if ( !_tcsicmp( pName , _T( "#Markings" ) ) )
      bInitRescan = true ;
    else if ( ( pFoundName = _tcsstr( pName , _T( "Type" ) ) ) )
      bInvalidate = bInitRescan = true ; 
    else if ( ( pFoundName = _tcsstr( pName , _T( "Params" ) ))
      && isdigit( *(pFoundName + 6) ))
    {
      int iIndex = atoi( pFoundName + 6 ) ;
      if ( iIndex-- > 0 )
      {
        pGadget->m_GObjects[ iIndex ].ScanProperties(
          pGadget->m_GObjects[ iIndex ].m_ParamsAsString ) ;
        pGadget->m_GObjects[ iIndex ].GetProperties() ;
      }
    }
    else if ( ( pFoundName = _tcsstr( pName , _T( "Coords" ) ) )
      && isdigit( *( pFoundName + 6 ) ) )
    {
      int iIndex = atoi( pFoundName + 6 ) ;
      if ( iIndex-- > 0 )
      {
        int iNPts = DecodePoints(
          ( FXPropKit2& ) pGadget->m_GObjects[ iIndex ].m_CoordsAsText ,
          pGadget->m_GObjects[ iIndex ].m_Coords ) ;
        pGadget->m_GObjects[ iIndex ].AnalyseCoordsForCircle() ;
      }
    }
  }
}
static const char * pObjectType = "Text;Figure;Arc;";
static const __int64 Indexes[] = { 1 , 2 , 3 } ;

void GMarking::PropertiesRegistration()
{
//   FXAutolock al(m_Lock , "GMarking::PropertiesRegistration") ;
  addProperty(SProperty::SPIN, "#Markings", &m_iNObjects , SProperty::Int, 0, 100);
  SetChangeNotification( _T( "#Markings" ) , ConfigParamChange , this );
  // add marking objects to array if necessary
//   while ( m_iNObjects > (int)m_Objects.size() )
//   {
// //     GMarkObject NewObject ;
//     m_Objects.push_back( GMarkObject() ) ;
//   }
  for ( int i = 0 ; i < m_iNObjects ; i++ )
  {
    FXString ObjNum ;
    ObjNum.Format( "%d" , i + 1 ) ;

    FXString Type( "Type" ) ; Type += ObjNum ;
    addProperty( SProperty::COMBO , Type , &m_GObjects[ i ].m_Type ,
      SProperty::Int , pObjectType /*, Indexes*/ );
    SetChangeNotificationForLast( ConfigParamChange , this );
    FXString Params( "Params" ) ; Params += ObjNum ;
    addProperty( SProperty::EDITBOX , Params , &m_GObjects[ i ].m_ParamsAsString ,
      SProperty::String );
    SetChangeNotificationForLast( ConfigParamChange , this );

    FXString Coords( "Coords" ) ; Coords += ObjNum ;
    addProperty( SProperty::EDITBOX , Coords , &m_GObjects[ i ].m_CoordsAsText ,
      SProperty::String );
    SetChangeNotificationForLast( ConfigParamChange , this );

    if ( m_GObjects[i].m_Type == GM_Text )
    {
      FXString Text( "Text" ) ; Text += ObjNum ;
      addProperty( SProperty::EDITBOX , Text , &m_GObjects[ i ].m_Text ,
        SProperty::String );
    }
    m_GObjects[ i ].GetProperties() ;
  }
}

void GMarking::ConnectorsRegistration()
{
  addInputConnector(transparent, "InputWithVideo");
  addOutputConnector(transparent, "VideoWithGraphics");
};

void GMarking::AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame)
{
  if (pParamFrame)
  {
    const CTextFrame * pCommand = pParamFrame->GetTextFrame();

    if (pCommand)
    {
      FXString Label = pCommand->GetLabel();
      if ( Label.MakeLower().Find( "formarking" ) >= 0 )
      {
        if ( _tcsstr( pCommand->GetString() , _T("set") ) )
        {
          bool Invalidate = false;
          ScanProperties(( (LPCTSTR)(pCommand->GetString()) + 4 ) , Invalidate);
        }
      }
    }
    pParamFrame->Release(pParamFrame);
  }
};

#define ORIGINALS_SIZE 10

CDataFrame * GMarking::DoProcessing(const CDataFrame * pDataFrame)
{
  if (m_GadgetName.IsEmpty())
  {
    m_GadgetName = GetGadgetInfo() ;
  }

  CContainerFrame * pOut = CContainerFrame::Create() ;
  pOut->CopyAttributes( pDataFrame ) ;
  pOut->AddFrame( pDataFrame ) ;

  cmplx cVideoSize( 1. , 1. ) ;
  const CVideoFrame * pV = pDataFrame->GetVideoFrame() ;
  if ( pV )
    cVideoSize = cmplx( GetWidth(pV) , GetHeight(pV) ) ;

  for ( FXSIZE i = 0 ; i < m_iNObjects ; i++ )
  {
    CDataFrame * pNewFrame = m_GObjects[ i ].GetFrameForOutput( cVideoSize ) ;
    if ( pNewFrame )
      pOut->AddFrame( pNewFrame ) ;
  }

  return pOut;
}

