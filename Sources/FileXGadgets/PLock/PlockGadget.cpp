// UserExampleGadget.h.h : Implementation of the UserExampleGadget class


#include "StdAfx.h"
#include "PlockGadget.h"

USER_FILTER_RUNTIME_GADGET( PlockGadget , "PlockGadget" );	//	Mandatory

// VOID CALLBACK TimerFunction( UINT uTimerID , UINT uMsg , DWORD dwUser , DWORD dw1 , DWORD dw2 )
// {
//   PlockGadget* params = ( PlockGadget* ) dwUser;
// }


PlockGadget::PlockGadget()
{
  m_circleDetected = false;
  m_radius = -1;
  m_center = cmplx( 0 , 0 );
  m_counter = 0;
  m_OutputFormat = _T( "set FrameRate_x10(%c%.2f);" ) ;
  m_dwLastCorrectionID = 0 ;
  m_dwAdjustStep = 6 ;
  m_dTargetAngle_Deg = 0. ;
  m_dSpeedAccelThreshold = 0.3 ;
  m_dMaxSpeedAccel = 0.22 ;
  m_dAngleMult = 50. ;
  m_dMaxAngleAccel = 3. ;
  m_bAngleControl = false ;
  Reset() ;
  init();
}

CDataFrame* PlockGadget::DoProcessing( const CDataFrame* pDataFrame )
{
  double dStart = GetHRTickCount() ;
  const CFigureFrame* frame = pDataFrame->GetFigureFrame( DEFAULT_LABEL );
  if ( !frame )
  {
    const CTextFrame * pTxt = pDataFrame->GetTextFrame( _T( "Reset" ) ) ;
    if ( pTxt )
    {
      if ( pTxt->GetString() == "PID" )
      {
        m_PID.Reset() ;
        m_dAccumulatedAngle = 0. ;
        m_PrevPt = cmplx( 0. , 0. ) ;
      }
      else
        Reset( true ) ;
    }
    
    return NULL;
  }

  cmplx pt = CDPointToCmplx( frame->GetAt( 0 ) );
  FXString label = pDataFrame->GetLabel();
  FXSIZE tok = 0;
  FXString time = label.Tokenize( " " , tok );
  double dTime ;
  int iNFrames ;
  double dFPS ;
  int iNRes = sscanf( ( LPCTSTR ) label , "%lf %d %lf" ,
    &dTime , &iNFrames , &dFPS ) ;
  if ( iNRes != 3 )
    return NULL ;

  DWORD dwID = pDataFrame->GetId() ;
  FXString OutText , Addition ;
  if ( !m_circleDetected )
  {
    const CVideoFrame * pVF = pDataFrame->GetVideoFrame() ;
    if ( pVF )
    {
      int iWidth = GetWidth( pVF ) ;
      int iHeight = GetHeight( pVF ) ;
      cmplx center = cmplx( iWidth/2. , iHeight/2. ) ;

      if ( !m_Points.GetCount() )
      {
        m_dFirstPointTime = GetHRTickCount() ;
        if ( m_dOriginalFPS == 0. )
          m_dOriginalFPS = dFPS ;
      }

      m_Points.Add( pt ) ;
      double dDir = arg( pt - center ) ;
      dDir = NormTo2PI( dDir ) ;
      int iIndex = ( int ) ( dDir * 16. / M_2PI ) ;
      if ( iIndex >= 16 )
        iIndex = 15 ;
      m_dwMarkedDirections &= ~( 1 << iIndex ) ;
      if ( !m_dwMarkedDirections ) // points are in all 16 sectors of circle
      {
        int iNIterations = CircleFit( m_Points , m_center , m_radius ) ;
        if ( iNIterations >= 0 )
        {
          m_circleDetected = true ;
          m_PrevPt = cmplx( 0. , 0. ) ;
          m_dLastAngle = 0. ;
          m_dAccumulatedAngle = 0. ;
        }
        else
          Reset( true ) ;
      }
      else if ( GetHRTickCount() - m_dFirstPointTime > 1000. )
      {
        double dNewFPS = dFPS + 1. ;
        m_dFirstPointTime = GetHRTickCount() + 1e6 ;
        OutText.Format( "set Frame_rate_x10(%.2f);fn=%.3f" , dNewFPS * 10. ) ;
      }
    }
  }
  else
  {
    if ( ( dwID - m_dwLastCorrectionID ) >= m_dwAdjustStep ) 
    {
      double dNewAngle = GetAngle( pt ) ;
      // calculate full rotations
      // This code will work only for cases,
      // when rotation is less than PI/2 between video frames
      if ( m_dLastAngle <= -M_PI2 )
      { // previous point is from --180 to -90 degrees
        if ( dNewAngle > M_PI2 )
        {  // new point is between 90 and 180 degrees
          m_dAccumulatedAngle -= M_2PI ; // addition of full turn to minus
        }
      }
      else if ( m_dLastAngle >= M_PI2 )
      { // Prev point inside of from 90 to 180 degrees
        if ( dNewAngle <= -M_PI2 )
        { // new point inside of -180 to 90 degrees
          m_dAccumulatedAngle += M_2PI ; // addition of full turn to plus
        }
      }

      double dAngularDist = dNewAngle + m_dAccumulatedAngle - DegToRad(m_dTargetAngle_Deg) ;
      
      double dAction = m_PID.PeocessNextValue( dAngularDist ) ;
      if ( fabs( dAction ) > m_dMaxSpeedAccel )
        dAction = Sign( dAction ) * m_dMaxSpeedAccel ;

      double dNewFPS = dFPS + dAction ;
      OutText.Format( "set Frame_rate_x10(%.2f);fn=%.1f" , dNewFPS * 10. , RadToDeg( dAngularDist ) ) ;
      Addition.Format( "\nP=%.3f;I=%.3f;D=%.3f;A=%.3f;" ,
        m_PID.m_dLastVal * m_PID.m_dKp ,
        m_PID.m_dIntegral * m_PID.m_dKi ,
        m_PID.m_dLastDiff * m_PID.m_dKd , dAction ) ;
      m_dLastAngle = dNewAngle ;
      m_PrevPt = pt ;
      // Calculate speed difference
      //double dLastTurn = GetLastTurn() ;
      //double dCameraPeriod = 1. / dFPS ;
      //double dFanPeriod = ( 1. - ( dLastTurn / M_2PI ) ) * dCameraPeriod ;
      //double dFanFPS = 1. / dFanPeriod ; // this is speed for compensate the rotation
      //double dDeltaFPS = -( dFPS - dFanFPS ) ;
      //double dAbsDelta = fabs( dDeltaFPS ) ;
      // Calculate angle difference
      //double dLastAngle = GetPtAngle( 1 ) ;
      //double dTargetRadian = DegToRad( m_dTargetAngle_Deg ) ;
      //double dAngDiff = GetDeltaAngle( dLastAngle , dTargetRadian ) ;
      //double dFPSFromAng = ( ( dAngDiff / M_2PI ) / dFPS ) * m_dAngleMult ;

//       double dFullCorrection = dDeltaFPS + dFPSFromAng ;
//       if ( dAbsDelta > m_dSpeedAccelThreshold )
//       {
//         double dOrderFPS = dFanFPS ;
//         if ( dAbsDelta > m_dMaxSpeedAccel )
//           dOrderFPS = dFPS + ( m_dMaxSpeedAccel * ( ( dDeltaFPS > 0. ) ? 1. : -1. ) ) ;
// 
//         OutText.Format( "set Frame_rate_x10(%.2f);fn=%.3f" , dOrderFPS * 10. , dFanFPS ) ;
//         Addition.Format( "\nSP FPScf=%.2f;dAng=%.1f" , dFPS , RadToDeg( dAngDiff ) ) ;
//       }
//       else
//       {
//         double dCorrection = dFPSFromAng ;
//         double dAbsFC = fabs( dFPSFromAng ) ;
//         if ( dAbsFC > m_dMaxAngleAccel )
//         {
//           dCorrection = ( m_dMaxAngleAccel * ( ( dFPSFromAng > 0. ) ? 1. : -1. ) ) ;
//         }
//         double dTargetFPS = dFPS - dCorrection ;
// 
//         OutText.Format( "set Frame_rate_x10(%.2f);fn=%.3f" , dTargetFPS * 10. , dFanFPS ) ;
//         Addition.Format( "\nAn FPScf=%.2f;dAng=%.1f" , dFPS , RadToDeg( dAngDiff ) ) ;
//       }
//       m_CircleArr.RemoveAll();
    }
  }
  if ( !OutText.IsEmpty() )
  {
    OutText += Addition ;

    CTextFrame * ViewText = CTextFrame::Create( OutText );
    ViewText->ChangeId( pDataFrame->GetId() );
    ViewText->SetTime( pDataFrame->GetTime() );
    double dEnd = GetHRTickCount() ;
    double dProcTime = dEnd - dStart ;
    FXString Label ;
    Label.Format( _T( "Tp=%.3f" ) , dProcTime ) ;
    ViewText->SetLabel( Label ) ;
    if ( PutFrame( m_pOutput , ViewText ) )
    {
      m_dwLastCorrectionID = dwID ;
      m_dLastCorrectionTime = dEnd ;
    }
  }
  return NULL;
}
// double PlockGadget::GetSpeedCorrection()
// {
//   if ( m_CircleArr.GetSize() < 2 ) return 0;
// 
//   double dQ = GetLastTurn();
//   ang[ m_counter ] = dQ;
//   m_counter++;
//   m_counter = m_counter % 256;
//   return ( dQ / ( M_2PI ) ) * GetPtTime( 1 ) ;
// }
// 
// int PlockGadget::AccumCirclePoint( cmplx pt , FXString time )
// {
//   if ( pt.real() != -1 && pt.imag() != -1 )
//   {
//     PtOnCircle tmp;
//     tmp._point = pt;
//     tmp._time = atof( time );
//     m_CircleArr.Add( tmp );
//   }
//   return m_CircleArr.GetSize();
// }
// int PlockGadget::AccumCirclePoint( cmplx pt , double dTime )
// {
//   if ( pt.real() > 0. && pt.imag() > 0. )
//   {
//     PtOnCircle tmp;
//     tmp._point = pt;
//     tmp._time = dTime ;
//     m_CircleArr.Add( tmp );
//   }
//   return m_CircleArr.GetSize();
// }
// 
// double PlockGadget::GetCentralAngle()
// {
//   int size = m_CircleArr.GetSize();
//   cmplx pt1( m_CircleArr.GetAt( size - 1 )._point );
//   cmplx pt2( m_CircleArr.GetAt( size - 2 )._point );
// 
//   double dAng1 = arg( pt1 - m_center ) ;
//   double dAng2 = arg( pt2 - m_center ) ;
//   double dAlpha = GetDeltaAngle( dAng1 , dAng2 );
//   return dAlpha;
// }
// void PlockGadget::GetSidesLength( double & a , double & b , double & c )
// {
//   int size = m_CircleArr.GetSize();
//   cmplx pt1( m_CircleArr.GetAt( size - 1 )._point );
//   cmplx pt2( m_CircleArr.GetAt( size - 2 )._point );
// 
//   a = abs( m_center - pt1 ) ;
//   b = abs( m_center - pt2 ) ;
//   c = abs( pt1 - pt2 ) ;
// }
void PlockGadget::AsyncTransaction( CDuplexConnector* pConnector , CDataFrame* pParamFrame )
{
  pParamFrame->Release( pParamFrame );
};
// bool PlockGadget::GetCircle()
// {
//   cmplx pt1 , pt2 , pt3;
//   GetThreePoints( pt1 , pt2 , pt3 );
//   return GetEquation( pt1 , pt2 , pt3 );
// }
// bool PlockGadget::GetThreePoints( cmplx & pt1 , cmplx & pt2 , cmplx & pt3 )
// {
//   int size = m_CircleArr.GetSize();
//   if ( size < 3 )
//     return false;
// 
//   pt1 = m_CircleArr[ 0 ]._point;
//   pt2 = m_CircleArr[ 1 ]._point;
//   pt3 = m_CircleArr[ 2 ]._point;
//   //int areaSize = size / 3;
// 
//   //pt1 = m_CircleArr[ROUND(areaSize / 2)]._point;
// 
//   //pt2 = m_CircleArr[ROUND(areaSize + areaSize / 2)]._point;
// 
//   //pt3 = m_CircleArr[ROUND(2 * areaSize + areaSize / 2)]._point;
// 
//   return true;
// }
bool PlockGadget::GetEquation( cmplx pt1 , cmplx pt2 , cmplx pt3 )//(double _x1, double _x2, double _x3, double _y1, double _y2, double _y3)
{
  double ax , ay , bx , by , cx , cy , x1 , y11 , dx1 , dy1 , x2 , y2 , dx2 , dy2 , ox , oy , dx , dy , radius; // Variables Used and to Declared
  ax = pt1.real(); ay = pt1.imag(); //first Point X and Y
  bx = pt2.real(); by = pt2.imag(); // Second Point X and Y
  cx = pt3.real(); cy = pt3.imag(); // Third Point X and Y

  //****************Following are Basic Procedure**********************//

  x1 = ( bx + ax ) / 2;
  y11 = ( by + ay ) / 2;
  dy1 = bx - ax;
  dx1 = -( by - ay );

  //***********************

  x2 = ( cx + bx ) / 2;
  y2 = ( cy + by ) / 2;
  dy2 = cx - bx;
  dx2 = -( cy - by );

  // ****************************
  ox = ( y11 * dx1 * dx2 + x2 * dx1 * dy2 - x1 * dy1 * dx2 - y2 * dx1 * dx2 )
    / ( dx1 * dy2 - dy1 * dx2 );
  oy = ( ox - x1 ) * dy1 / dx1 + y11;
  //***********************************

  dx = ox - ax;
  dy = oy - ay;
  radius = ( int ) sqrt( dx * dx + dy * dy );

  if ( radius < 5 || ox - radius < 1 || oy - radius < 1 )
    return false;

  if ( !m_circleDetected )
  {
    cmplx p( ox , oy );
    m_center = p;

    m_radius = radius;
    m_circleDetected = true;

  }
  else if ( m_circleDetected )
  {
    double r = ( m_center.real() + ox ) / 2;
    double i = ( m_center.imag() + oy ) / 2;
    cmplx p( r , i );
    m_center = p;
    m_radius = radius;
  }
  return true;
}
void PlockGadget::PropertiesRegistration()
{
  addProperty( SProperty::EDITBOX , _T( "Format" ) , &m_OutputFormat , SProperty::String );
  addProperty( SProperty::SPIN , _T( "AdjustStep" ) , &m_dwAdjustStep ,
    SProperty::Long , 1 , 50 );
  addProperty( SProperty::EDITBOX , _T( "SpeedThres" ) ,
    &m_dSpeedAccelThreshold , SProperty::Double );
  addProperty( SProperty::EDITBOX , _T( "MaxSpeedDelta" ) ,
    &m_dMaxSpeedAccel , SProperty::Double );
  addProperty( SProperty::EDITBOX , _T( "Kp" ) ,
    &m_PID.m_dKp , SProperty::Double );
  addProperty( SProperty::EDITBOX , _T( "Ki" ) ,
    &m_PID.m_dKi , SProperty::Double );
  addProperty( SProperty::EDITBOX , _T( "Kd" ) ,
    &m_PID.m_dKd , SProperty::Double );

  addProperty( SProperty::EDITBOX , _T( "IntegralDecr" ) ,
    &m_PID.m_dDecrease , SProperty::Double );
//   addProperty( SProperty::EDITBOX , _T( "MaxAngleAccel" ) ,
//     &m_dMaxAngleAccel , SProperty::Double );
  addProperty( SProperty::EDITBOX , _T( "TargetDegrees" ) ,
    &m_dTargetAngle_Deg , SProperty::Double );

  //addProperty(SProperty::SPIN_BOOL	,	_T("bool_int4")	,	&iProp4		,	SProperty::SpinBool	,	-10		,	20, &bProp4	);
  //addProperty(SProperty::COMBO		,	_T("combo5")	,	&sProp5		,	SProperty::String	,	pList	);
  //addProperty(SProperty::EDITBOX		,	_T("bool6")		,	&bProp6		,	SProperty::Bool		);
};


void PlockGadget::ConnectorsRegistration()
{
  addInputConnector( text , "Coordinates" );
  addOutputConnector( text , "AngleSpeed" );
  //addDuplexConnector( transparent, transparent, "DuplexName1");

};




