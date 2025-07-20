#ifndef PLANE_MATH_H__
#define PLANE_MATH_H__

#include <math\intf_sup.h>
#include <math/Line2d.h>
// #include <Math/FRegression.h>
#include <gadgets\FigureFrame.h>
#include <helpers\FramesHelper.h>


class CoordsCorresp
{
public :
  cmplx FOV ;
  cmplx World ;

  CoordsCorresp() {}  ;
  CoordsCorresp( const cmplx& Img , const cmplx& W ) { FOV = Img ; World = W ; }

  // method calculates cScale form this point to other
  cmplx GetCScale( CoordsCorresp& Other )
  {
    cmplx cWorldVect( World - Other.World ) ;
    cmplx cFOVVect( FOV - Other.FOV ) ;
    cmplx cScale = (cWorldVect / cFOVVect) ;
    return cScale ;
  }
  cmplx GetConjCScale( CoordsCorresp& Other ) // for case, when FOV Y goes down
  {
    cmplx cWorldVect( World - Other.World ) ;
    cmplx cFOVVect( FOV - Other.FOV ) ;
    cmplx cScale = (cWorldVect / conj(cFOVVect)) ;
    return cScale ;
  }
  CoordsCorresp& operator=(const CoordsCorresp& Orig)
  {
	  FOV = Orig.FOV;
	  World = Orig.World;
	  return *this;
  }
}  ;

inline bool IsLeftDown(const cmplx& p1 , const cmplx p2 , double Epsilon = EPSILON) 
{

  return ((p1.real() < (p2.real() - Epsilon))
       || ((fabs( p1.real() - p2.real() ) < Epsilon ) && (p1.imag() < (p2.imag() - Epsilon)))) ;
}

inline bool IsPtInRect( CRect Rect , cmplx Pt )
{
  return ( Rect.left <= Pt.real()
    &&     Pt.real() <= Rect.right
    &&     Rect.top <= Pt.imag()
    &&     Pt.imag() <= Rect.bottom ) ;
}

inline bool IsPtOnRect( CRect Rect , cmplx Pt , double dToler = 1.0 )
{
  return ( (fabs( Rect.left - Pt.real() ) < dToler)
    || ( fabs( Pt.real() - Rect.right ) < dToler )
    || ( fabs( Rect.top - Pt.imag() ) - dToler )
    || ( fabs( Pt.imag() - Rect.bottom ) - dToler ) ) ;
}

inline bool FindSegmentPtsOnEdge( CRect Edge , cmplx * pSegment ,
  int& iIndex , int iLen , double dToler )
{
  int iEndIndex = iIndex + iLen ; // iLen + 1 will be checked
  while ( iIndex <= iEndIndex )
  {
    cmplx NextPt = pSegment[ iIndex++ ] ;
    if ( IsPtOnRect( Edge , NextPt , dToler ) )
      return true ;
  }
  return false ;
}

inline bool betw (double l, double r, double x) 
{
  return (min(l,r) <=( x + EPSILON)) && (x <= (max(l,r) + EPSILON));
}

inline void double_swap( double& a , double& b )
{
  double tmp = a ;
  a = b ;
  b = tmp ;
}

inline void cmplx_swap( cmplx& a, cmplx& b )
{
  cmplx tmp = a ;
  a = b ;
  b = tmp ;
}

inline bool intersect_1d (double a, double b, double c, double d) {
  if (a > b)  
    double_swap (a, b);
  if (c > d)  
    double_swap (c, d);
  return max(a, c) <=( min (b, d) + EPSILON) ;
}
    // intersection find for segments, returns true if there is intersect
bool inline intersect (
 cmplx L1_Pt1, cmplx L1_Pt2, // points of first segment
 cmplx L2_Pt1, cmplx L2_Pt2, // points of second segment
 cmplx & left, cmplx & right ) // cross point (overlap begin) and overlap end
{
  if ( ! intersect_1d (L1_Pt1.real(), L1_Pt2.real(), L2_Pt1.real(), L2_Pt2.real()) 
    || ! intersect_1d (L1_Pt1.imag(), L1_Pt2.imag(), L2_Pt1.imag(), L2_Pt2.imag()))
    return false;
  CLine2d m (L1_Pt1, L1_Pt2);
  CLine2d n (L2_Pt1, L2_Pt2);
  double zn = det ( m.m_a,  m.m_b, n.m_a, n.m_b);
  if (abs (zn) < EPSILON)  // approximately parallel lines
  {
    if (abs (m.dist (L2_Pt1)) > EPSILON || abs (n.dist (L1_Pt1)) > EPSILON)
      return false; // not the same line
    if ( IsLeftDown (L1_Pt2 , L1_Pt1) )  
      cmplx_swap (L1_Pt1, L1_Pt2);
    if ( IsLeftDown(L2_Pt2 , L2_Pt1) )  
      cmplx_swap (L2_Pt1, L2_Pt2);
    left = (IsLeftDown( L1_Pt1 , L2_Pt1 )) ? L2_Pt1 : L1_Pt1 ;
    right = (IsLeftDown(L1_Pt2, L2_Pt2)) ? L1_Pt2 : L2_Pt2 ;
    return true;
  }
  else //
  {
    cmplx result(- det ( m.m_c,  m.m_b, n.m_c, n.m_b) / zn , 
                 - det ( m.m_a,  m.m_c, n.m_a, n.m_c) / zn ) ;
    left = right = result ;
    return betw (L1_Pt1.real(), L1_Pt2.real(), left.real())
        && betw (L1_Pt1.imag(), L1_Pt2.imag(), left.imag())
        && betw (L2_Pt1.real(), L2_Pt2.real(), left.real())
        && betw (L2_Pt1.imag(), L2_Pt2.imag(), left.imag()) ;
  }
}

class straight_segment : public CLine2d
{
public:
  cmplx m_Pt1 ;
  cmplx m_Pt2 ;
  double m_dAngleFromPt1ToPt2 ;
  bool m_bZeroLength ;

  straight_segment( cmplx& p , cmplx& q ) : CLine2d( p , q )
  {
    m_Pt1 = p ;
    m_Pt2 = q ;
    m_bZeroLength = abs( p - q ) < EPSILON ;
    if ( !m_bZeroLength )
      m_dAngleFromPt1ToPt2 = (!m_bZeroLength) ? arg( m_Pt2 - m_Pt1 ) : 0. ;
  }

  straight_segment( const straight_segment& Orig )
  {
    memcpy( this , &Orig , sizeof( *this ) ) ;

  }
  // returns true is this segment is crossing another segment
  // i.e. cross is inside of each segment
  bool intersect( const straight_segment& Other , cmplx& res )
  {
    if ( m_bZeroLength )
    {
      if ( Other.dist( m_Pt1 ) < EPSILON )
      {
        res = m_Pt1 ;
        return true ;
      }
      return false ;
    }
    if ( Other.m_bZeroLength )
    {
      if ( dist( Other.m_Pt1 ) < EPSILON )
      {
        res = Other.m_Pt1 ;
        return true ;
      }
    }
    cmplx cSecondCross ;
    bool bRes = ::intersect(
      m_Pt1 , m_Pt2 , Other.m_Pt1 , Other.m_Pt2 ,
      res , cSecondCross ) ;
   
    return bRes ;
  }
  straight_segment& operator = ( const straight_segment& Orig )
  {
    memcpy( this , &Orig , sizeof( *this ) ) ;
    return *this ;
  }
};

typedef vector<straight_segment> SLines ;

inline double GetTriangleArea( const cmplx& Pt1, const cmplx& Pt2 , const cmplx& Pt3 )
{
  double dS =  ((Pt1.real() - Pt2.real()) * (Pt3.imag() - Pt2.imag())
    - (Pt3.real() - Pt2.real()) * (Pt1.imag() - Pt2.imag()))/2. ;
  double dLa = abs( Pt2 - Pt1 ) ;
  double dLb = abs( Pt2 - Pt3 ) ;
  double dLc = abs( Pt1 - Pt3 ) ;
  double dHalfPerimeter = (dLa + dLb + dLc) * 0.5 ;
  double dHeronS = sqrt( dHalfPerimeter * ( dHalfPerimeter - dLa )
    * ( dHalfPerimeter - dLb ) * ( dHalfPerimeter - dLc ) ) ;
  return dS ;
}

#define GetTriangleSquare GetTriangleArea

inline cmplx GetTriangleWeightCenter( const cmplx& Pt1 , const cmplx& Pt2 , const cmplx& Pt3 )
{
  return (Pt1 + Pt2 + Pt3) / 3. ;
}

inline double GetPtToLineDistance( 
  const cmplx& Pt, const cmplx& PtOnLine1 , const cmplx& PtOnLine2)
{
  double dSideOnLine = abs(PtOnLine1 - PtOnLine2) ;
  if ( dSideOnLine > EPSILON )
  {
    double dS = GetTriangleSquare( Pt , PtOnLine1 , PtOnLine2 ) ;
    double dDist = 2.0 * dS /  dSideOnLine ;
    return dDist ;
  }
  return 0 ;
}


inline bool ConvertByTwoPts( CoordsCorresp& Known1 , CoordsCorresp& Known2 , 
  CoordsCorresp& Measured , bool bTheSameOrientation = true ) // only FOV known, World will be filled
{
  cmplx KnownVectFOV = Known2.FOV - Known1.FOV ;
  cmplx KnownVectWorld = Known2.World - Known1.World ;
  double dFOVKnownSize = abs( KnownVectFOV ) ;
  if ( dFOVKnownSize < EPSILON  ||  abs(KnownVectWorld) )
    return false ;

  cmplx MeasTo1VectFOV = Measured.FOV - Known1.FOV ;
  double dAngleKnown1to2World = arg( KnownVectWorld ) ;
  double dAngleMeasToKnown1FOV = arg( MeasTo1VectFOV ) ;
  double dScale = abs( Known2.World - Known1.World ) / dFOVKnownSize ;
  double dAngleMeasWorld = dAngleKnown1to2World + 
    (bTheSameOrientation) ? dAngleMeasToKnown1FOV  : - dAngleMeasToKnown1FOV ;
  double dWorldMeasVectSize = abs( MeasTo1VectFOV ) * dScale ;
  cmplx WorldVector = polar( dWorldMeasVectSize , dAngleMeasWorld ) ;
  Measured.World = Known1.World + WorldVector ;

  return true ;
}

// From screen to world
bool ConvertBy3Pts( CoordsCorresp& Known1 , CoordsCorresp& Known2 ,
  CoordsCorresp Known3 , CoordsCorresp& Measured ) ;


  // angle from Vect1 to Vect2
inline double GetAngleBtwVects( cmplx FromVect , cmplx ToVect )
{
  if ( (abs(FromVect) < EPSILON)  || (abs(ToVect) < EPSILON) ) // 1e-9
    return 0.0 ;
  
  cmplx Div = FromVect / ToVect ;
  return arg( Div ) ;
}
inline double GetAbsAngleBtwVects( cmplx FromVect , cmplx ToVect )
{
  
  return fabs( GetAngleBtwVects( FromVect , ToVect ) ) ;
}

inline double CheckParallelism( cmplx Begin , cmplx End1 , cmplx End2 )
{
  return GetAngleBtwVects( End1 - Begin , End2 - Begin ) ;
}

inline double AbsCheckParallelism( cmplx Begin , cmplx End1 , cmplx End2 )
{
  return fabs( CheckParallelism( Begin , End1 , End2 ) ) ;
}
// Scalar  multiplication
inline double ScalarMult( cmplx& Vect1 , cmplx & Vect2 )
{
  double dResult = Vect1.real() * Vect2.real() + Vect1.imag() * Vect2.imag() ;
  return dResult ;
}

// returns number of 90 degree segment
// 0-45 - 1st segment, 45-90 - second and so on
// Argument is from -PI to +PI
inline int IsNearVert( double dAngleRad ) 
{
  double dAngleRadNorm = NormRad( dAngleRad ) ;
  double dAbsAngleNorm = fabs( dAngleRadNorm ) ;
  if ( dAbsAngleNorm < M_PI_4
    || dAbsAngleNorm > 3 * M_PI_4 )
    return 0 ;
  else
    return 1 ;
}

cmplx FindNearest( CFigure * pFigure , cmplx& Pt , int& iFoundIndex ,
  bool bIsSmoothCurve = true , int iEXtremIndex = -1 , bool bGoPlus = true );

cmplx FindNearestToY( const CFigure * pFigure , cmplx& Pt , bool bdXPositive );

int FindNearestToYIndex(
  const CFigure * pFigure , const cmplx& Pt , bool bdXPositive );

cmplx FindNearestToX( CFigure * pFigure , cmplx& Pt , bool bdYPositive );
cmplx FindNearestToX( CFigure * pFigure , int& iInitIndex , double dTargetX , bool bToPlus ) ;
size_t TrackToTargetX( CFigure * pFigure , CmplxVector& Pts ,
  int iInitIndex , double dStopX , bool bToPlus ) ;

// Find indexes of nearest pts in array pcTargetPts (array length is iNPts)
// !!! Points in pcTargetPts should be in proper order
size_t FindNearestToPts( const CFigure * pFigure ,
  cmplx * pcTargetPts , int iNPts , IntVector& Indexes , int iInitialIndex = 0 ) ;

inline void SetMinAndIndex(
  double dVal , double& dMin , int& iIndex , int iCurrIndex )
{
  if ( dVal < dMin )
  {
    dMin = dVal ;
    iIndex = iCurrIndex ;
  }
}
inline void SetMaxAndIndex(
  double dVal , double& dMax , int& iIndex , int iCurrIndex )
{
  if ( dVal > dMax )
  {
    dMax = dVal ;
    iIndex = iCurrIndex ;
  }
}

inline double  GetFigureLength( const CFigure * pFigure )
{
  return pFigure->GetFigureLength() ;
}

#define EXTREME_INDEX_LEFT 0
#define EXTREME_INDEX_RIGHT 2
#define EXTREME_INDEX_TOP 1
#define EXTREME_INDEX_BOTTOM 3

class Extremes_s
{
public:

  cmplx GetSize()
  {
    return cmplx( m_cRight.real() - m_cLeft.real() , m_cBottom.imag() - m_cTop.imag() ) ;
  } ;
  cmplx GetCenter()
  {
    return 0.5 * cmplx( m_cRight.real() + m_cLeft.real() , m_cBottom.imag() + m_cTop.imag() ) ;
  } ;
  cmplx m_cLeft ;
  cmplx m_cTop ;
  cmplx m_cRight ;
  cmplx m_cBottom ;
} ;

struct ExtrIndexes_s
{
  int m_iLeft ;
  int m_iTop ;
  int m_iRight ;
  int m_iBottom ;
} ;

cmplx FindExtrems( const cmplx * pData , size_t iSize ,
  Extremes_s& Expremes,        // Array of 4 values for extreme points coordinates
  ExtrIndexes_s * pIndexes = NULL ,  // Array of 4 values for extreme indexes
  cmplx * pSize = NULL ) ;

cmplx FindExtrems( const CFigure * pFigure ,
  CmplxVector& Extrems , IntVector * pIndexes , cmplx * pSize = NULL ) ;

cmplx FindExtrems( const cmplx * pData , size_t iSize ,
  CmplxArray& Extrems , FXIntArray * pIndexes , cmplx * pSize ) ;
  
cmplx FindExtrems( const CmplxArray& Data ,
  CmplxArray& Extrems , FXIntArray * pIndexes , cmplx * pSize ) ;

cmplx FindExtrems( const CmplxVector& Data ,
  CmplxArray& Extrems , FXIntArray * pIndexes , cmplx * pSize );

cmplx FindExtrems( const CFigure * pFigure ,
  CmplxArray& Extrems , FXIntArray * pIndexes = NULL , cmplx * pSize = NULL );

cmplx FindLeftPt( const CFigure * pFigure , int& iIndex ) ;

cmplx FindFarthestPt( cmplx& cBase , const CFigure * pFigure , int& iIndex ) ;
// cmplx FindFarthestPtFromSegment( cmplx cSegmBegin , cmplx cSegmEnd ,
//   const CFigure * pFigure , int& iIndex , double& dFarestDist ) ;

// function for corners finding, iIndexBegin should be less than iIndexEnd
cmplx FindFarthestPtFromFigSegment( int iIndexBegin , int iIndexEnd ,
  const CFigure * pFigure , int& iFarestIndex , double& dFarestDist ) ;

inline double Dist( CPoint Pt1 , CPoint Pt2 )
{
  CSize Diff = Pt1 - Pt2 ;
  return sqrt( Diff.cx * Diff.cx + Diff.cy * Diff.cy ) ;
}
inline double Dist( CPoint Pt1 , cmplx Pt2 )
{
  return abs( cmplx( Pt1.x , Pt1.y ) - Pt2 ) ;
}


// from screen to world
bool ConvertBy3Pts( CoordsCorresp& Known1 , CoordsCorresp& Known2 ,
  CoordsCorresp& Known3 , cmplx& Screen , cmplx& World ) ;



// from world to screen
bool ConvertWtoSBy3Pts( CoordsCorresp& Known1 , CoordsCorresp& Known2 ,
  CoordsCorresp Known3 , cmplx& Screen , cmplx& World ) ;// 


double GetDiffFromStraight( cmplx * pPts , int iLen ,
  int iInd1st , int iInd2nd ,
  double& dLengthRatio , double * pdStd = NULL );

// function returns maximal angle difference between neightbour segments
double CheckStraightness( cmplx * pPts , int iLen , // array of pts and it's length
  int iInd1st , int iInd2nd , int iCheckStep , // part for analysis begin and end, step for analysis 
  double& dMinDist , double& dMaxDist ) ; // Minimal and maximal subsegment lengths

cmplx GetCenterForFigure( cmplx * pFigure , 
  int iFigLen , ImgMoments& Moments );

cmplx GetCenterForFigure( cmplx * pFigure , int iFigLen ,
  cmplx cCenter , ImgMoments& Moments );

int GetMinMaxDistances( cmplx * pFigure , int iFigLen ,
  cmplx cCenter , ImgMoments& Moments ,
  CIntArray& Maxes , CIntArray& Mins , double dMinRelDiff ,
  int iWindowSize );


bool ConvertByNearestAndSegment( CoordsCorresp& Nearest ,
  CoordsCorresp& Known2 , CoordsCorresp& Known3 ,
  cmplx& Screen , cmplx& World ) ;

// function returns true if line does cross rectangle 
bool GetCrossOfLineAndRectangle( 
  straight_segment& Segment , CDRect& Rect , cmplx& cCross );

// function returns true and cCross point for line going from rectangle center
// to direction dDir_rad 
bool GetCrossOfDirectionAndRectangle( CRect& Rect , double dDir_Rad , cmplx& cCross ) ;

int FindFigurePtOnDist( const CFigure * pFig , int iInitialIndex ,
  int iStep , double dDist );

int FindNearestToDirection(
  const CFigure * pFigure , cmplx& cCentPt , 
  double dDirFromCent ) ;

// for approximate circle or ellipse, don't use for random contur
double GetMinMaxDia( const cmplx * pContur , int iConturLen ,
  double& dMinDia , double& dMaxDia , int iAverage = 0 ) ;

// for approximate circle or ellipse, don't use for random contur
double GetMinMaxDia( const cmplx * pContur , int iConturLen , cmplx cCenter ,
  double NPointsPerCircle , double& dMinDia , double& dMaxDia , int iAverage = 0 ) ;


inline double GetMinMaxDia( const CFigure * pFigure ,
  double& dMinDia , double& dMaxDia , int iAverage = 0 )
{
  const cmplx * pContur = ( const cmplx* ) ( pFigure->GetData() ) ;
  return GetMinMaxDia( pContur , (int)pFigure->GetCount() , dMinDia , dMaxDia , iAverage ) ;
}

// get center of circle passed through points Pt1 and Pt2 with radius dRadius
inline bool GetCircleCentOnVF( double dRadius , 
  cmplx cPt1 , cmplx cPt2 , bool bONleftFromPt1ToPt2 , cmplx& cResult )
{
  cmplx cVect( cPt2 - cPt1 ) ;
  double dDist = abs( cVect ) ;
  if ( dDist > dRadius * 2. )
    return false ;

  cmplx cCent = ( cPt1 + cPt2 ) * 0.5 ;
  cVect = GetNormalized( cVect ) ;
  cmplx cOrthoToVect = bONleftFromPt1ToPt2 ? GetOrthoLeftOnVF( cVect ) : -GetOrthoLeftOnVF( cVect ) ;
  double dDistFromVectToCent = sqrt( dRadius * dRadius - dDist * dDist * 0.25 ) ;
  cResult = cCent + dDistFromVectToCent * cOrthoToVect ;
  return true ;
}
// returns number of cross/touch points of vertical line and circle
//     0 - no cross, 1 - only touch (rarely), 2 - two crosses
//          
inline int GetYforXOnCircle( double dX , 
  cmplx cCenter , double dR , double& dY1 , double& dY2 )
{
  double dDistToCenter = dX - cCenter.real() ;
  if ( ( abs( dDistToCenter ) > dR ) )
    return 0 ;
  else if ( dDistToCenter == dR )
  {
    dY1 = dY2 = cCenter.imag() + dR ;
    return 1 ;
  }
  else if ( -dDistToCenter == dR )
  {
    dY1 = dY2 = cCenter.imag() - dR ;
    return 1 ;
  }
  
  // common case with two crosses
  double dY = sqrt( dR * dR - dDistToCenter * dDistToCenter ) ;
  dY1 = cCenter.imag() - dY ;
  dY2 = cCenter.imag() + dY ;
  return 2 ;
}

// returns number of cross/touch points of vertical line and circle
//     0 - no cross, 1 - only touch (rarely), 2 - two crosses
//          
inline int GetXforYOnCircle( double dY ,
  cmplx cCenter , double dR , double& dX1 , double& dX2 )
{
  double dDistToCenter = dY - cCenter.imag() ;
  if ( ( abs( dDistToCenter ) > dR ) )
    return 0 ;
  else if ( dDistToCenter == dR )
  {
    dX1 = dX2 = cCenter.real() + dR ;
    return 1 ;
  }
  else if ( -dDistToCenter == dR )
  {
    dX1 = dX2 = cCenter.real() - dR ;
    return 1 ;
  }

  // common case with two crosses
  double dX = sqrt( dR * dR - dDistToCenter * dDistToCenter ) ;
  dX1 = cCenter.real() - dY ;
  dX2 = cCenter.real() + dY ;
  return 2 ;
}
#endif   //PLANE_MATH_H__