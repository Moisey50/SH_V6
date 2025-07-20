
#pragma once

#include "fxfc/fxfc.h"
#include "Math/Intf_sup.h"
#include "helpers\propertykitEx.h"

enum Edge_Quality
{
  EQ_Normal = 0 ,
  EQ_Defects ,
  EQ_Invisible = -1
};

typedef enum
{ 
  CS_Wall = 0 ,
  CS_Dark , 
  CS_Unknown = -1
}
CS_MeasAlg ;

class ConturSample
{
public:
  int         m_iIndex ;    // in all measurements in scan
  int         m_iSubindex ; // inside segment
  int         m_iSegmentNumInScan;
  CCoordinate m_RobotPos ;
  cmplx       m_MiddlePt ;  // Robot pos for point in FOV center
  cmplx       m_cPartCenter ;
  double      m_dAveBurrWidth_um ;
  double      m_dAveBurrHeight_um ; // or Width measured by Std
  double      m_dMaxBurrWidth_um ;
  double      m_dMaxBurrHeight_um ;  // or Width measured by Std
  double      m_dEdgeWithBurrLen_um ;
  double      m_dBurrVolume_qum ;
  double      m_dLastContrast ;
  double      m_dLastNextPtContrast ;
  double      m_dLastTime ;
  cmplx       m_cEdgePt ; // In FOV
  cmplx       m_cSegmentVect ; // XY direction in FOV
  CPoint      m_PointOnMap ;
  CS_MeasAlg  m_Algorithm ;
  Edge_Quality m_iBadEdge ;


  ConturSample()
  {
    Reset() ;
  }
  ConturSample( int iIndex , CCoordinate& RobotPos ,
    cmplx& MiddlePt , double& dAveBurrWidth_um ,
    double& dAveBurrHeight_um , double& dMaxBurrWidth_um ,
    double& dMaxBurrHeight_um , double& dEdgeWithBurrLen_um ,
    double& dBurrVolume_qum , double dContrast ,
    double dNextContrast , double dTime )
    : m_iIndex(iIndex) , m_RobotPos(RobotPos) 
    , m_MiddlePt(MiddlePt)
    , m_dAveBurrWidth_um( dAveBurrWidth_um )
    , m_dAveBurrHeight_um( dAveBurrHeight_um )
    , m_dMaxBurrWidth_um( dMaxBurrWidth_um )
    , m_dMaxBurrHeight_um( dMaxBurrHeight_um )
    , m_dEdgeWithBurrLen_um( dEdgeWithBurrLen_um )
    , m_dBurrVolume_qum( dBurrVolume_qum )
    , m_dLastContrast( dContrast )
    , m_dLastNextPtContrast( dNextContrast )
    , m_dLastTime( dTime )
    , m_Algorithm( CS_Wall )
    , m_iBadEdge( EQ_Normal )
    , m_iSubindex( 0 )
  { }
  // constructor without heights and next contrast
  ConturSample( int iIndex , CCoor3t& RobotPos ,
    cmplx& MiddlePt , double& dAveBurrWidth_um ,
    double dAvContrast , double dMaxContrast )
    : m_iIndex( iIndex ) 
    , m_RobotPos( RobotPos.m_x , RobotPos.m_y , RobotPos.m_z , RobotPos.m_time ) //time instead theta
    , m_MiddlePt( MiddlePt )
    , m_dAveBurrWidth_um( dAveBurrWidth_um )
    , m_dAveBurrHeight_um( 0. )
    , m_dMaxBurrWidth_um( 0. )
    , m_dMaxBurrHeight_um( 0. )
    , m_dEdgeWithBurrLen_um( 0. )
    , m_dBurrVolume_qum( 0. )
    , m_dLastContrast( dAvContrast )
    , m_dLastNextPtContrast( dMaxContrast )
    , m_dLastTime( m_RobotPos.m_theta ) // there is measurement time
    , m_Algorithm( CS_Dark )
    , m_iBadEdge( EQ_Normal )
  {}
  ~ConturSample() {};

  void Reset()
  {
    memset( this , 0 , sizeof( *this ) ) ;
  }
  int GetNComponents()
  {
    return 13; // 13 members
  }
  ConturSample& operator=( const ConturSample& Orig )
  {
    memcpy( this , &Orig , sizeof( *this ) ) ;
    return *this ;
  }
  static LPCTSTR GetCaption()
  {
    return _T( "  Pt Smpl Xrob   Yrob   Zrob      Trob     "
      "  Xsmpl  Yxmpl   Wav    Wmx    Hav    Hmx   L_um     Vol       Stat   Alg"
      "   Cav        EdgePt(X,Y)       EdgeDir(X,Y)     Tmeas    Sgm# \n" ) ;
  }
  int ToString( FXString& Result , LPCTSTR pOpenBracket=NULL , LPCTSTR pCloseBracket=NULL )
  {
    Result.Format( //"%s" // Bracket
      "%4d,%4d,"             // Index
      "%6d,%6d,%6d,%10.1f,"    // Robot position; usually theta is zero or time
      "   %6d,%6d,"           // Part center 
      "%6.2f,%6.2f,%6.2f,%6.2f," // Average Burr width, height; Max burr width and height
      "%6.2f,%8.2f,"  // Length of edge with detected burrs, burr volume in qubic um
      "%9s %s,  "           // Bad edge mark and Algorithm
      "%5.1f,   %7.2f,%7.2f,   " // Contrast, EdgePt(X,Y), 
      "%6.3f,%6.3f,   %8.1f, %d" // EdgeDir(X,Y), Last time, SegmentInScan
      ///="%s"   // Closing bracket
      //, pOpenBracket
      , m_iIndex , m_iSubindex ,                                   // 0,1
      ROUND(m_RobotPos.m_x) , ROUND(m_RobotPos.m_y) ,              // 2,3
      ROUND( m_RobotPos.m_z) , m_RobotPos.m_theta ,                // 4,5
      ROUND( m_MiddlePt.real()) , ROUND( m_MiddlePt.imag()) ,      // 6,7
      m_dAveBurrWidth_um , m_dMaxBurrWidth_um ,                    // 8,9
      m_dAveBurrHeight_um , m_dMaxBurrHeight_um ,                  // 10,11
      m_dEdgeWithBurrLen_um , m_dBurrVolume_qum ,                  // 12,13
      (m_iBadEdge == EQ_Normal) ? "OK" :                           // 14 edge quality decoding
        (m_iBadEdge == EQ_Defects) ? "Bad" : "Invisible" ,  
      (m_Algorithm == CS_Wall ) ? "Wall" : "Dark" ,                // 15 
      m_dLastContrast , m_cEdgePt.real() , m_cEdgePt.imag() ,      // 16..18
      m_cSegmentVect.real() , m_cSegmentVect.imag(), m_dLastTime , // 19..21
      m_iSegmentNumInScan ) ;                                      // 22
    if ( pOpenBracket )
      Result.Insert( 0 , pOpenBracket ) ;
    if ( pCloseBracket )
      Result += pCloseBracket ;
    return (int) Result.GetLength() ;
  }
  int FromString( FXString& AsString )
  {
    AsString.TrimLeft( _T( " \t\n\r,;:" ) ) ;
    TCHAR c = AsString[ 0 ] ;
    // passing first bracket
    FXSIZE iPos = (c == _T( '(' )) || (c == _T( '[' )) || (c == _T( '{' )) ;

    FXString Token ;

//     Token = AsString.Tokenize( _T( " \t\n\r,;:" ) , iPos ) ;
//     if ( Token.IsEmpty() )
//       return 0 ;
//     int iRes = sscanf( (LPCTSTR)Token , "%d" , &m_iIndex ) ;
//     if ( !iRes )
//       return 0 ;
    int iItemCnt = 0 ;
    double * pAsDbl = &m_RobotPos.m_x ;
    while ( !(Token = AsString.Tokenize( _T( " \t\n\r,;:" ) , iPos )).IsEmpty() )
    {
      switch ( iItemCnt )
      {
      case 0: m_iIndex = atoi( Token ) ; break ;
      case 1: m_iSubindex = atoi( Token ) ; break ;
      case 2: m_RobotPos.m_x = atof( Token ) ; break ;
      case 3: m_RobotPos.m_y = atof( Token ) ; break ;
      case 4: m_RobotPos.m_z = atof( Token ) ; break ;
      case 5: m_RobotPos.m_theta = atof( Token ) ; break ;
      case 8: m_dAveBurrWidth_um = atof( Token ) ; break ;
      case 9: m_dMaxBurrWidth_um = atof( Token ) ; break ;
      case 10: m_dAveBurrHeight_um = atof( Token ) ; break ;
      case 11: m_dMaxBurrHeight_um = atof( Token ) ; break ;
      case 12: m_dEdgeWithBurrLen_um = atof( Token ) ; break ;
      case 13: m_dBurrVolume_qum = atof( Token ) ; break ;
      case 14: m_iBadEdge = (Edge_Quality)((Token == "OK") ? EQ_Normal
        : (Token == "Bad") ? EQ_Defects : EQ_Invisible );
        break ;
      case 15: m_Algorithm = (Token == "Dark") ? CS_Dark
        : (Token == "Wall") ? CS_Wall : CS_Unknown ;
        break ;
      case 16: m_dLastContrast = atof( Token ) ; break ;
      case 17: m_cEdgePt._Val[_RE] = atof( Token ) ; break ;
      case 18: m_cEdgePt._Val[ _IM ] = atof( Token ) ; break ;
      case 19:  m_cSegmentVect._Val[ _RE ] = atof( Token ) ; break ;
      case 20:  m_cSegmentVect._Val[ _IM ] = atof( Token ) ; break ;
      case 21: m_dLastTime = atof( Token ) ; break ;
      case 22: m_iSegmentNumInScan = atoi( Token ) ; break ;

      default:
        {
          double dVal = 0. ;
          int iRes = sscanf( (LPCTSTR) Token , "%lf" , &dVal ) ;
          if ( iRes ) // ptr is on last member
            *(pAsDbl++) = dVal ;
          //           else
          //             return iItemCnt ;
        }
        break ;
      }
      iItemCnt++ ;
    }
    return iItemCnt ; // too short string
  }

  int GetNItems() { return 23 ; }

  void SetAlgorithm( CS_MeasAlg Alg )
  {
    m_Algorithm = Alg ; 
  }
};

class ConturData : public FXArray<ConturSample>
{};

typedef struct
{
  cmplx m_cEdgePt ;    // External contour
  cmplx m_cBurrEndPt ; // Internal contour
  cmplx m_cFirst ;
  cmplx m_cEdgeDir ;
  cmplx m_cOrtho ;
  CCoor3t m_RobotPos ;
  double m_dBurrValue_um ;
  double m_dAverFocus ;
  double m_dMaxFocus ;
  double m_dPos ;
  double m_dSampleTime ;
} OneMeasPt ;

class FocusValues
{
public:
  double      m_dLastAvFocusNearBegin ;
  double      m_dLastAvFocusInCenter ;
  double      m_dLastAvFocusNearEnd ;
  double      m_dLastMaxFocusNearBegin ;
  double      m_dLastMaxFocusInCenter ;
  double      m_dLastMaxFocusNearEnd  ;
  double      m_dLastMinAvFocus ;
  double      m_dLastMaxAvFocus ;
  double      m_dLastMinMaxFocus ;
  double      m_dLastMaxMaxFocus ;

  bool bIsFilled()
  {
    return !(m_dLastAvFocusNearBegin == 0.
      || m_dLastAvFocusInCenter == 0.
      || m_dLastAvFocusNearEnd == 0.
      || m_dLastMaxFocusNearBegin == 0.
      || m_dLastMaxFocusInCenter == 0.
      || m_dLastMaxFocusNearEnd == 0.) ;
  }
  void UpdateBestFocusValues( FocusValues& Other )
  {
    SetToMax( m_dLastAvFocusNearBegin , Other.m_dLastAvFocusNearBegin ) ;
    SetToMax( m_dLastAvFocusInCenter , Other.m_dLastAvFocusInCenter ) ;
    SetToMax( m_dLastAvFocusNearEnd , Other.m_dLastAvFocusNearEnd );
    SetToMax( m_dLastMaxFocusNearBegin , Other.m_dLastMaxFocusNearBegin ) ;
    SetToMax( m_dLastMaxFocusInCenter , Other.m_dLastMaxFocusInCenter ) ;
    SetToMax( m_dLastMaxFocusNearEnd , Other.m_dLastMaxFocusNearEnd );
  }

} ;

#define LAST_MEAS_VALUES_ARR_SIZE 250

class MeasuredValues
{
public:
  CCoor3t m_RobotPos ;
  double  m_dLastZStep ;
  OneMeasPt m_Results[ LAST_MEAS_VALUES_ARR_SIZE ] ;
  FocusValues m_FV ;
  int       m_iPlusIndex ;
  int       m_iMinusIndex ;
  int       m_iMaxAvMeasNumber ; // This is for m_CurrentViewMeasStatus 
                                 // what measurement from 
                                 // m_DataForPosition is taken
  int       m_iMaxAvIndex ;
  bool      m_bFromZero ;
  double    m_dAvFocus ;
  int       m_iNWithGoodFocus ; // i.e.with focus better than threshold
  int       m_iNNewWithGoodFocus ;
  int       m_iNWithGoodFocusOnXYPosition ; // taken from m_CurrentViewMeasStatus
  int       m_iLeftMeasured ;
  int       m_iRightMeasured ;
  int       m_iLeftMeasBegin ;
  CDataFrame * m_pView ;
  void Reset( bool bFromZero = true )
  {
    if ( m_pView )
    {
      datatype t = m_pView->GetDataType() ;
      TRACE( "MeasuredValues::Reset Release m_pView=%p Type=%s Nfr=%d" ,
        m_pView , (LPCTSTR) Tvdb400_TypeToStr( t ) ,
        m_pView->IsContainer() ?
        ((CContainerFrame*) m_pView)->GetFramesCount() : 1 ) ;

      m_pView->Release() ;
    }

    memset( this , 0 , sizeof( *this ) ) ;
    if ( !bFromZero )
    {
      m_iPlusIndex = LAST_MEAS_VALUES_ARR_SIZE / 2 ;
      m_iMinusIndex = (LAST_MEAS_VALUES_ARR_SIZE / 2) - 1 ;
    }
    else
    {
      m_iMinusIndex = m_iPlusIndex = 0 ;
      m_bFromZero = true ;
    }
    m_FV.m_dLastMinAvFocus = m_FV.m_dLastMinMaxFocus = DBL_MAX ;
    m_FV.m_dLastMaxAvFocus = m_FV.m_dLastMaxMaxFocus = -DBL_MAX ;
    m_iRightMeasured = m_iLeftMeasured = -1 ;
  }
  MeasuredValues( bool bFromZero = true )
  {
    m_pView = NULL ;
    Reset( bFromZero ) ;
  }
  ~MeasuredValues()
  {
    if ( m_pView )
      m_pView->Release() ;
  }
  bool PutToPlus( OneMeasPt& NewData )
  {
    m_Results[ m_iPlusIndex++ ] = NewData ;
    if ( m_iPlusIndex >= LAST_MEAS_VALUES_ARR_SIZE )
    {
      m_iPlusIndex-- ;
      return false ;
    }
    return true ;
  }
  bool PutToMinus( OneMeasPt& NewData )
  {
    m_Results[ m_iMinusIndex-- ] = NewData ;
    if ( m_iMinusIndex < 0 )
    {
      m_iMinusIndex++ ;
      return false ;
    }
    return true ;
  }
  OneMeasPt * GetBeginPt()
  {
    if ( !m_bFromZero )
    {
      if ( m_iMinusIndex < (LAST_MEAS_VALUES_ARR_SIZE / 2) - 1 )
        return &m_Results[ m_iMinusIndex + 1 ] ;
    }
    else if ( m_iPlusIndex > 0 )
      return &m_Results[ 0 ] ;
    return NULL ;
  }
  OneMeasPt * GetEndPt()
  {
    if ( !m_bFromZero )
    {
      if ( m_iPlusIndex > (LAST_MEAS_VALUES_ARR_SIZE / 2) )
        return &m_Results[ m_iPlusIndex - 1 ] ;
    }
    else if ( m_iPlusIndex > 0 )
      return &m_Results[ m_iPlusIndex - 1 ] ;
    return NULL ;
  }
  OneMeasPt * GetCentralPt()
  {
    if ( !m_bFromZero )
    {
      if ( m_iPlusIndex > (LAST_MEAS_VALUES_ARR_SIZE / 2) )
        return &m_Results[ (LAST_MEAS_VALUES_ARR_SIZE / 2) ] ;
    }
    else if ( m_iPlusIndex > 0 )
      return &m_Results[ m_iPlusIndex / 2 ] ;
    else
      return NULL ;
  }
  void CheckAndSetFocusMinMax( double dAv , double dMax , int iIndex )
  {
    if ( m_FV.m_dLastMinAvFocus > dAv )
      m_FV.m_dLastMinAvFocus = dAv ;
    if ( m_FV.m_dLastMinMaxFocus > dMax )
      m_FV.m_dLastMinMaxFocus = dMax ;
    if ( m_FV.m_dLastMaxAvFocus < dAv )
    {
      m_FV.m_dLastMaxAvFocus = dAv ;
      m_iMaxAvIndex = iIndex ;
    }
    if ( m_FV.m_dLastMaxMaxFocus < dMax )
      m_FV.m_dLastMaxMaxFocus = dMax ;
  }
  bool GetAverageFocusValues( double& dAv , double& dMax )
  {
    if ( !m_FV.bIsFilled() )
      return false ;
    dAv = (m_FV.m_dLastAvFocusNearBegin
      + m_FV.m_dLastAvFocusInCenter
      + m_FV.m_dLastAvFocusNearEnd) / 3. ;
    dMax = (m_FV.m_dLastMaxFocusNearBegin
      + m_FV.m_dLastMaxFocusInCenter
      + m_FV.m_dLastMaxFocusNearEnd) / 3. ;
    return true ;
  }
  bool GetGoodValues( double dThres , int& iFirstIndex , int& iLastIndex )
  {
    int iNGoodCntr = 0 ;
    for ( iFirstIndex = m_iMinusIndex ; iFirstIndex < m_iPlusIndex ; iFirstIndex++ )
    {
      if ( m_Results[ iFirstIndex ].m_dAverFocus >= dThres )
        break ;
    }
    if ( iFirstIndex >= m_iPlusIndex )
    {
      iFirstIndex = -1 ;
      return false ;
    }
    for ( iLastIndex = iFirstIndex ; iLastIndex < m_iPlusIndex ; iLastIndex++ )
    {
      if ( m_Results[ iLastIndex ].m_dAverFocus >= dThres )
      {
        iNGoodCntr++;
        continue ;
      }
      else
      {
        if ( iLastIndex < m_iPlusIndex - 1 )
        {
          if ( m_Results[ iLastIndex + 1 ].m_dAverFocus >= dThres )
            continue ;
          if ( iLastIndex < m_iPlusIndex - 2 )
          {
            if ( m_Results[ iLastIndex + 2 ].m_dAverFocus >= dThres )
              continue ;
          }
          m_iNWithGoodFocus = iNGoodCntr;
          return true ; // minimum one good sample we did see
        }
      }
    }
    iLastIndex-- ;
    m_iNWithGoodFocus = iNGoodCntr;
    return true ;
  }
  int GetAvForGoodValues( double& dAvWidth , double& dMaxWidth ,
    int& iMaxWidthIndex , double& dMinFocus ,
    double& dMaxFocus , double& dAverAverFocus , double dThres )
  {
    dAverAverFocus = dAvWidth = dMaxWidth = dMaxFocus = 0. ;
    dMinFocus = DBL_MAX ;
    int iNGood = 0 ;
    for ( int i = m_iMinusIndex ; i < m_iPlusIndex ; i++ )
    {
      dAverAverFocus += m_Results[ i ].m_dAverFocus ;
      if ( m_Results[ i ].m_dAverFocus >= dThres )
      {
        if ( m_Results[ i ].m_dAverFocus < dMinFocus )
          dMinFocus = m_Results[ i ].m_dAverFocus ;
        if ( m_Results[ i ].m_dAverFocus > dMaxFocus )
          dMaxFocus = m_Results[ i ].m_dAverFocus ;

        if ( (m_Results[ i ].m_dBurrValue_um > 7.)
          && (m_Results[ i ].m_dBurrValue_um < 100.) )
        {
          dAvWidth += m_Results[ i ].m_dBurrValue_um ;
          iNGood++ ;
          if ( m_Results[ i ].m_dBurrValue_um > dMaxWidth )
          {
            dMaxWidth = m_Results[ i ].m_dBurrValue_um ;
            iMaxWidthIndex = i ;
          }
        }
      }
    }
    if ( iNGood )
      dAvWidth /= iNGood ;
    else
    {
      dMinFocus = 0. ;
      iMaxWidthIndex = -1 ;
    }
    if ( (m_iPlusIndex - m_iMinusIndex - 1) > 1 )
      dAverAverFocus /= (m_iPlusIndex - m_iMinusIndex - 1) ;
    m_dAvFocus = dAverAverFocus ;
    return iNGood ;
  }
  void SetView( CDataFrame * pView )
  {
    if ( m_pView )
    {
      datatype t = m_pView->GetDataType() ;
      TRACE( "MeasuredValues::SetView Release m_pView=%p New=%p Type=%s Nfr=%d" ,
        m_pView , pView , (LPCTSTR) Tvdb400_TypeToStr( t ) ,
        m_pView->IsContainer() ?
        ((CContainerFrame*) m_pView)->GetFramesCount() : 1 ) ;
      m_pView->Release() ;
    }
    m_pView = pView ;
  }
  CDataFrame * GetViewAndClear()
  {
    CDataFrame * pView = m_pView ;
    m_pView = NULL ;
    return pView ;
  }
  void SetAvValues()
  {
    double dAverAverFocus = 0. ;
    for ( int i = m_iMinusIndex ; i < m_iPlusIndex ; i++ )
      dAverAverFocus += m_Results[ i ].m_dAverFocus ;

    if ( (m_iPlusIndex - m_iMinusIndex - 1) > 1 )
      dAverAverFocus /= (m_iPlusIndex - m_iMinusIndex - 1) ;
    m_dAvFocus = dAverAverFocus ;
  }
};

typedef FXArray<MeasuredValues> DataForPosition ;

// temporary array of pointers to MeasuredValues aligned by Z
typedef FXArray<MeasuredValues*> AlignedByZPtrs ;

class AveragedDataAroundPoint
{
public:
  cmplx  m_cTextPt ;     // in pixels on map
  cmplx  m_cForwardPt ; // ------"---------
  cmplx  m_cBackwardPt ;// ------"---------
  DWORD  m_Color ;
  double m_dValue ;
  TCHAR  m_AsText[ 30 ] ;

  AveragedDataAroundPoint()
  {
    m_Color = 0 ;
    m_dValue = 0. ;
    m_AsText[ 0 ] = 0 ;
  }
  AveragedDataAroundPoint( CPoint Pt , CPoint PtBack ,
    CPoint PtForw , cmplx cCent , DWORD dwColor , double dVal )
  {
    m_cBackwardPt = cmplx( PtBack.x , PtBack.y ) ;
    m_cForwardPt = cmplx( PtForw.x , PtForw.y ) ;
    cmplx cForwVect = m_cForwardPt - m_cBackwardPt;
    cmplx cNormForw = GetNormalized(cForwVect);
    cmplx cOutSide = GetOrthoLeftOnVF(cNormForw) ;
    cmplx cShift = cOutSide * 20. ;
    m_cForwardPt += cShift;
    m_cBackwardPt += cShift;
    double cShiftAngle = arg(cOutSide);
    double dAbsAngle = abs(cShiftAngle);
    m_cTextPt = cmplx( Pt.x , Pt.y ) 
      + cShift * (((cShiftAngle < -M_PI_2) || (cShiftAngle > M_PI * 0.65))? 3. : 1.5)
      + cmplx( 0. , -7. ) ;
    m_Color = dwColor ;
    m_dValue = dVal ;
    sprintf_s( m_AsText , "%.1f" , dVal ) ;
  }
};

typedef FXArray<AveragedDataAroundPoint> AveragedSelectedPoints ;