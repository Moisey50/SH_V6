#ifndef __INC__Spot_H__
#define __INC__Spot_H__

#include <complex>
#include "fxfc\fxfc.h"
#include "helpers\FXParser2.h"
#include "math\Intf_sup.h"
#include <math\PlaneGeometry.h>
#include "math\hbmath.h"
#include "classes\drect.h"


inline void ToString( cmplx& Val , FXString& Result )
{
  Result.Format( "%f, %f" , Val.real() , Val.imag() );
}

inline void ToStringFormatted( cmplx& Val , FXString& Result , 
  LPCTSTR pFormat = NULL )
{
  if ( !pFormat )
    pFormat = _T( "%f, %f" ) ;
  Result.Format( pFormat , Val.real() , Val.imag() );
}



class SizeD
{
private:
	double m_dWidth;
	double m_dHeight;
	double m_dArea;
  double m_dSize ;

public:
  inline void set(double Width, double Height)
  {
    m_dWidth = Width;
    m_dHeight = Height;
    m_dArea = m_dWidth*m_dHeight;
    m_dSize = (m_dWidth + m_dHeight) / 2. ;
  }
  inline void set( const SizeD& Other )
  {
    m_dWidth = Other.m_dWidth;
    m_dHeight = Other.m_dHeight;
    m_dArea = m_dWidth*m_dHeight;
    m_dSize = (m_dWidth + m_dHeight) / 2. ;
  }
  inline void set( const cmplx& Other )
  {
    set( Other.real() , Other.imag() ) ;
  }
  inline SizeD& operator=(const SizeD& other)
  {
    set(other.m_dWidth, other.m_dHeight);
    return *this;
  }
  inline SizeD& operator=(const cmplx& other)
  {
    set( other );
    return *this;
  }
  inline SizeD operator+( const SizeD& Other)
  {
    SizeD Sum( m_dWidth + Other.getWidth() , m_dHeight + Other.getHeight() ) ;
    return Sum ;
  }
  inline SizeD& operator+=( const SizeD& Other)
  {
    set( m_dWidth + Other.getWidth() , m_dHeight + Other.getHeight() ) ;
    return *this ;
  }
  inline SizeD operator*( const double dVal )
  {
    SizeD Mult( m_dWidth * dVal , m_dHeight * dVal ) ;
    return Mult ;
  }
  inline SizeD& operator*=( const double dVal )
  {
    set(  m_dWidth * dVal , m_dHeight * dVal ) ;
    return *this ;
  }
  inline SizeD operator/( const double dVal )
  {
    if ( dVal == 0. )
      return SizeD() ;
    SizeD Div( m_dWidth / dVal , m_dHeight / dVal ) ;
    return Div ;
  }
  inline SizeD& operator/=( const double dVal )
  {
    set(  m_dWidth / dVal , m_dHeight / dVal ) ;
    return *this ;
  }
	inline double getSize() const {	return m_dSize ; }
	inline double getWidth() const { return m_dWidth; }
	inline double getHeight() const { return m_dHeight; }
	inline SizeD() { set(0.0,0.0); }
	inline SizeD(double width, double height)	{	set(width,height); }
	inline SizeD(const SizeD& other) { *this = other;	}
};

inline SizeD GetAveSize( SizeD a , SizeD b )
{
  return (a + b)/2 ;
}

#define AXIS_0    1
#define AXIS_1    2
#define AXIS_X    AXIS_0
#define AXIS_Y    AXIS_1

enum SpotSector
{
  SS_UNKNOWN = 0 ,
  SS_NE ,
  SS_ES ,
  SS_SW ,
  SS_WN
};
class Spot
{
  friend class Node1D;

private:
  int m_iIndx;	
public:

  cmplx	m_imgCoord;
  cmplx mutable	m_WCoord;
  SizeD	m_Sizes;
  double m_dArea ;
  double m_dPerimeter ;
  cmplx m_cCentroidAsLineRegr ;
  cmplx m_cMidPointAsLineRegr ;
  DWORD m_OnLines ;
  int   m_iNUsers ;
  CPoint m_Indexes ;
  cmplx m_DirN ;
  cmplx m_DirE ;
  cmplx m_DirS ;
  cmplx m_DirW ;
  cmplx m_DirWN;
  cmplx m_DirWE ;
  cmplx m_DirWS ;
  cmplx m_DirWW ;
  Spot * m_SpotN ;
  Spot * m_SpotE ;
  Spot * m_SpotS ;
  Spot * m_SpotW ;


public:
  Spot() 
  { 
    Reset() ;
  }
  
  void Reset()
  {
    memset( this , 0 , sizeof( *this ) ) ;
    m_Indexes = CPoint( INT_MAX , INT_MAX ) ;
  }

  inline Spot(int Idx, 
    const double& dX , const double& dY , 
    const double& dWidth , const double& dHeight , DWORD OnLines = 0 ) 
  {
    Reset() ;
    m_imgCoord = cmplx( dX , dY ) ;
    m_Sizes = SizeD( dWidth , dHeight ) ;
    m_iIndx = Idx;
    m_OnLines = OnLines ;
    m_iNUsers = 1 ;
  }
  inline Spot(int Idx, const cmplx& coordinates, const SizeD& Sizes , DWORD OnLines = 0 ) 
  {
    Reset() ;
    m_imgCoord = coordinates ;
    m_Sizes = Sizes ;
    m_iIndx = Idx;
    m_OnLines = OnLines ;
    m_iNUsers = 1 ;
  }
  inline Spot(int Idx, cmplx& ImgPos , double width, double height , DWORD OnLines = 0 ) 
  {
    Reset() ;
    m_Sizes = SizeD( width , height ) ;
    m_imgCoord = ImgPos ;
    m_iIndx = Idx;
    m_OnLines = OnLines ;
    m_iNUsers = 1 ;
  }
  inline Spot(const Spot& other)
  {
    *this = other;
    m_iNUsers = 1 ;
  }
  inline int IncrementNUsers() { return ++m_iNUsers ; }
  inline int DecrementNUsers() 
  { 
    ASSERT( m_iNUsers >= 1 ) ;
    return --m_iNUsers ; 
  }
  
  int GetGlobalIndex() { return m_iIndx; }
	inline const bool checkLocation( /*const Spot& spot,*/ 
    const double x1, const double x2, const double y1, const double y2)
	{
		bool b = ( m_imgCoord.real() > x1 ) 
          && ( m_imgCoord.real() < x2 ) 
          && ( m_imgCoord.imag() > y1 ) 
          && ( m_imgCoord.imag() < y2 );

		return b;
	}
  inline const bool checkLocation( const cmplx& UpLeft , const cmplx DownRight )
  {
    bool b = ( m_imgCoord.real() > UpLeft.real() ) 
      && ( m_imgCoord.real() < DownRight.real() ) 
      && ( m_imgCoord.imag() > UpLeft.imag() ) 
      && ( m_imgCoord.imag() < DownRight.imag() );

    return b;
  }
  inline const bool IsInArea( const cmplx& UpLeft , const cmplx DownRight ) 
  {
    return checkLocation( UpLeft , DownRight );
  }
  inline bool MatchInImg( const cmplx& point , double dTol ) const
  {
    return ( abs(m_imgCoord - point) < dTol ) ;
  }
  inline bool MatchInWrld( const cmplx& point , double dTol ) const
  {
    return ( abs(m_WCoord - point) < dTol ) ;
  }
	inline void setCoordinates(const cmplx& Coordinates)
	{
		m_imgCoord = Coordinates;
	}
	inline void setSizes(const SizeD& Sizes) { m_Sizes = Sizes;	}
	inline void set(const cmplx& coordinates, const SizeD& Sizes)
	{			
		setCoordinates(coordinates);
		setSizes(Sizes);
	}
	inline void set(double x, double y, double width, double height)
	{	
		set(cmplx(x,y), SizeD(width, height));
	}
	inline double getDistance(const cmplx& point) const
	{
		return abs( m_imgCoord - point );
	}
	inline double getDistance(const Spot& spot) const
	{			
		return getDistance( spot.m_imgCoord );
	}
  inline double getAngle( const cmplx& point ) const
  {
    return arg( point - m_imgCoord ) ;
  }
  inline double getAngle( const Spot& spot) const
  {			
    return getAngle( spot.m_imgCoord );
  }

	inline double getSize() const
	{
		return m_Sizes.getSize();
	}
	inline void setAbsoluteCoordinates(double x, double y)
	{
		m_WCoord._Val[_RE] = x;
		m_WCoord._Val[_IM] = y;
	} const
	inline void setAbsoluteCoordinates(const cmplx& point)
	{
		m_imgCoord = point ;
	} const
	inline void updateAbsoluteCenter(double x, double y)
	{
		m_WCoord._Val[_RE] += x;
		m_WCoord._Val[_IM] += y;
	} const
	inline void updateAbsoluteCenter(const cmplx& point)
	{
		m_WCoord = point ;
	} const
	inline Spot& operator=(const Spot& other)
	{
		memcpy( this , &other , sizeof(*this) ) ;
		return *this;
	}

  bool SaveCalibData( FILE * fw , TCHAR * pAdd = NULL )
  {
    FXString Result ;
    if ( SaveCalibData( Result , pAdd ) )
    {
      _ftprintf( fw , _T("%s") , (LPCTSTR)Result ) ;
      return true ;
    }
    return false ;
  }
  bool SaveCalibData(FXString& Result , TCHAR * pAdd = NULL )
  {
    TCHAR buf[200] ;
    int iNWritten = _stprintf_s( buf , _T("Img(%8g,%8g) World(%8g,%8g) Indexes(%d,%d) Sizes(%g,%g) OnLines(0x%X) Used(%d)") , 
      m_imgCoord.real() , m_imgCoord.imag() , m_WCoord.real() , m_WCoord.imag() , 
      m_Indexes.x , m_Indexes.y , m_Sizes.getWidth() , m_Sizes.getHeight() , 
      m_OnLines , m_iNUsers ) ;
    if ( iNWritten )
    {
      Result += buf ;
      if ( pAdd )
        Result += pAdd ;
      return true ;
    }
    return false ;
  }

  int RestoreCalibData( FILE *fr , TCHAR FinishSymbol = _T('>') )
  {
    TCHAR ReadBuf[2000] ;
    if ( _fgetts( ReadBuf , 1999 , fr ) )
    {
      FXParser2 Buf( ReadBuf ) ;
      FXSIZE iPos = 0 ;
      iPos = Buf.Find( _T("No Spot") , iPos ) ;
      if ( iPos >= 0 )
        return -2 ;  // empty place

      cmplx Tmp ;
      iPos = Buf.Find( _T("Img(") , iPos ) ;
      if ( iPos < 0 )
      {
        if ( Buf.Find( _T('>') ) >= 0 )
          return 0 ;
        return -1 ;
      }
      if ( !StrToCmplx( (LPCTSTR)Buf + (iPos += 4) , Tmp  ) ) 
        return -1 ;
      m_imgCoord = Tmp ;
      iPos = Buf.Find( _T("World(") , iPos ) ;
      if ( iPos < 0 )
        return -1 ;
      if ( !StrToCmplx( (LPCTSTR)Buf + (iPos += 6) , Tmp  ) ) 
        return -1 ;
      m_WCoord = Tmp ;
      iPos = Buf.Find( _T("Indexes(") , iPos ) ;
      if ( iPos < 0 )
        return -1 ;
      CPoint PtTmp ;
      if ( !StrToIPt( (LPCTSTR)Buf + (iPos += 8) , PtTmp  ) ) 
        return -1 ;
      m_Indexes = PtTmp ;
      iPos = Buf.Find( _T("Sizes(") , iPos ) ;
      if ( iPos < 0 )
        return -1 ;
      if ( !StrToCmplx( (LPCTSTR)Buf + (iPos += 6) , Tmp  ) ) 
        return -1 ;
      m_Sizes.set( Tmp ) ;
      FXSIZE iTmp ;
      iPos = Buf.Find( _T("OnLines(") , iPos ) ;
      if ( iPos >= 0 )
        m_OnLines = (DWORD)( ( ConvToBinary( (LPCTSTR)Buf + (iPos += 8) , iTmp ) ) ? iTmp : 0 );
      else
        iPos = 0 ;
      iPos = Buf.Find( _T("Used(") , iPos ) ;
      if ( iPos >= 0 )
        m_iNUsers = (int) ( ( ConvToBinary( (LPCTSTR)Buf + (iPos += 5) , iTmp ) ) ? iTmp : 0 );
      return 1 ;
    };
    return -1 ;    
  }
  void SetWorldSteps( const cmplx& East , const cmplx& North )
  {
    m_DirWE = East ;
    m_DirWW = -East ;
    m_DirWN = North ;
    m_DirWS = -North ;
  }
};

typedef vector< Spot* > CalibSpotVector ;

inline Spot*  Find( CalibSpotVector& Spots , cmplx& Coord , double dAbsTol )
{
  for ( int i = 0 ; i < (int)Spots.size() ; i++ )
  {
    if ( abs( Coord - Spots[i]->m_imgCoord ) < dAbsTol )
      return Spots[i] ;
  }
  return NULL ;
}

inline const Spot * CheckForMinAndReplace( const Spot * pCurrent ,
  double& dCurrentMin , const Spot * pOther , const cmplx& Pt )
{
  if ( pOther )
  {
    double dDistToOther = pOther->getDistance( Pt ) ;
    if ( dDistToOther < dCurrentMin )
    {
      dCurrentMin = dDistToOther ;
      return pOther ;
    }
  }

  return pCurrent ;
}
inline const Spot * getNearest( const Spot * pInitial , const cmplx& Pt )
{
  double dMinDist = pInitial->getDistance( Pt ) ;
  const Spot * pMinDistSpot = pInitial ;
  pMinDistSpot = CheckForMinAndReplace( pMinDistSpot ,
    dMinDist , pInitial->m_SpotN , Pt ) ;
  pMinDistSpot = CheckForMinAndReplace( pMinDistSpot ,
    dMinDist , pInitial->m_SpotE , Pt ) ;
  pMinDistSpot = CheckForMinAndReplace( pMinDistSpot ,
    dMinDist , pInitial->m_SpotS , Pt ) ;
  pMinDistSpot = CheckForMinAndReplace( pMinDistSpot ,
    dMinDist , pInitial->m_SpotW , Pt ) ;
  return pMinDistSpot ;
}

inline SpotSector GetNearestDirection( const Spot * pSpot , const cmplx& Pt )
{
  cmplx cDirFromSpot = Pt - pSpot->m_imgCoord ;
  double dDistFromSpot = abs( cDirFromSpot ) ;
  if ( dDistFromSpot < EPSILON ) // unit is pixel (?)
    return SS_UNKNOWN ; // Point is too near to spot
  double dAngleToN = arg( cDirFromSpot / pSpot->m_DirN ) ; // to North spot
  if ( dAngleToN >= 0. )
  {
    double dAngleToE = arg( cDirFromSpot / pSpot->m_DirE ) ; // to East spot
    if ( dAngleToE < 0. )
      return SS_NE ;
    double dAngleToS = arg( cDirFromSpot / pSpot->m_DirS ) ; // to South spot
    if ( dAngleToS < 0. )
      return SS_ES ;
    double dAngleToW = arg( cDirFromSpot / pSpot->m_DirW ) ; // to West spot
    if ( dAngleToW < 0. )
      return SS_SW ;
    ASSERT( 0 ) ;
    return SS_WN ;
  }
  else
  {
    double dAngleToW = arg( cDirFromSpot / pSpot->m_DirW ) ; // to West spot
    if ( dAngleToW > 0. )
      return SS_WN ;
    double dAngleToS = arg( cDirFromSpot / pSpot->m_DirS ) ; // to South spot
    if ( dAngleToS > 0. )
      return SS_SW ;
    double dAngleToE = arg( cDirFromSpot / pSpot->m_DirE ) ; // to East spot
    if ( dAngleToE > 0. )
      return SS_ES ;
    ASSERT( 0 ) ;
    return SS_NE ;
  }
}

inline bool GetNearestCorresps( const Spot * pSpot , SpotSector Dir ,
  CoordsCorresp& ForSpot1 , CoordsCorresp& ForSpot2 )
{
  switch ( Dir )
  {
    case SS_NE:
    case SS_UNKNOWN: // to close to spot
      ForSpot1.FOV = pSpot->m_imgCoord + pSpot->m_DirN ;
      ForSpot1.World = pSpot->m_WCoord + pSpot->m_DirWN;
      ForSpot2.FOV = pSpot->m_imgCoord + pSpot->m_DirE ;
      ForSpot2.World = pSpot->m_WCoord + pSpot->m_DirWE;
      return true ;
    case SS_ES:
      ForSpot1.FOV = pSpot->m_imgCoord + pSpot->m_DirE ;
      ForSpot1.World = pSpot->m_WCoord + pSpot->m_DirWE;
      ForSpot2.FOV = pSpot->m_imgCoord + pSpot->m_DirS ;
      ForSpot2.World = pSpot->m_WCoord + pSpot->m_DirWS;
      return true ;
    case SS_SW:
      ForSpot1.FOV = pSpot->m_imgCoord + pSpot->m_DirS ;
      ForSpot1.World = pSpot->m_WCoord + pSpot->m_DirWS;
      ForSpot2.FOV = pSpot->m_imgCoord + pSpot->m_DirW ;
      ForSpot2.World = pSpot->m_WCoord + pSpot->m_DirWW;
      return true ;
    case SS_WN:
      ForSpot1.FOV = pSpot->m_imgCoord + pSpot->m_DirW ;
      ForSpot1.World = pSpot->m_WCoord + pSpot->m_DirWW;
      ForSpot2.FOV = pSpot->m_imgCoord + pSpot->m_DirN ;
      ForSpot2.World = pSpot->m_WCoord + pSpot->m_DirWN;
      return true ;
  }
  return false ;
}
#endif	//	__INC__Spot_H__



