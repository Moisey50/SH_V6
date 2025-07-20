#pragma once
#include <winnt.h>
#include <fxfc/fxfc.h>
#include <math/intf_sup.h>

enum Axis
{
  NotCorrectCoord ,
  Axis_Undef = -1 ,
  FromZaber = 0 ,
  Axis_Z = 1 ,
  Axis_X = 2 ,
  Axis_Y = 3 
};

#define MotionMaskX (2) //(1 << ((int)Axis::Axis_X)) // 2
#define MotionMaskY (4) //(1 << ((int)Axis::Axis_Y)) // 4
#define MotionMaskZ (8) //(1 << ((int)Axis::Axis_Z)) // 8

inline bool IsAllCoordsAvailable( DWORD dwMask )
{
  return
    ((dwMask & MotionMaskX) != 0)
    && ((dwMask & MotionMaskY) != 0)
    && ((dwMask & MotionMaskZ) != 0) ;
}

class DecodeZaberMotion
{
public:
  FXString          m_IDPrefix ;
  FXString          m_CoordPrefix ;
  CCoordinate       m_CurrentPos ;
  double            m_dLastCoordTime ;
  double            m_dAnswerTimeout_ms ;
  double            m_dLastCoordInterval ;
  bool              m_bTimeout ;
  int               m_MotionMask ;
  int               m_HomeSetMask ;
  int               m_iNWaitsForMotionAnswers ;
  DWORD             m_ReceivedCoords ;

  DecodeZaberMotion():
    m_IDPrefix( _T( "ID=" ) ) ,
    m_CoordPrefix( _T( "um)=" ) ),
    m_dLastCoordTime( 0. ),
    m_dLastCoordInterval( 0. ),
    m_dAnswerTimeout_ms( 
  #ifdef _DEBUG
    1000000.
  #else
    50000. 
  #endif
    ) ,
    m_bTimeout( false ) 
    , m_HomeSetMask( 0 )
    , m_ReceivedCoords( 0 )
  {
    m_MotionMask = 0 ;
    m_iNWaitsForMotionAnswers = 0 ;
  }

  ~DecodeZaberMotion()
  {
  }

  // returns axis number, if there is string not from  Zaber, returns -1
  Axis DecodeInputString( const FXString& InputString )
  {
    int iIDPos = (int) InputString.Find( m_IDPrefix ) ;
    int iCoordPos = (int) InputString.Find( m_CoordPrefix ) ;
    double dEntryTime = GetHRTickCount() ;
    if ( iIDPos < 0 )
      return Axis_Undef ;
    LPCTSTR pID = ((LPCTSTR) InputString) + iIDPos + (int) m_IDPrefix.GetLength() ;
    Axis iID = (Axis) atoi( pID ) ;
    if ( iCoordPos >= 0 )
    {
      LPCTSTR pCoord = ((LPCTSTR) InputString) + iCoordPos + (int) m_CoordPrefix.GetLength() ;
      double dCoord = atof( pCoord ) ;
      switch ( iID )
      {
        case Axis_X: m_CurrentPos.m_x = dCoord ; break ;
        case Axis_Y: m_CurrentPos.m_y = dCoord ; break ;
        case Axis_Z: m_CurrentPos.m_z = dCoord ; break ;
        default: return Axis_Undef ;
      }
      m_dLastCoordInterval = dEntryTime - m_dLastCoordTime ;
      if ( m_dLastCoordTime != 0. )
        m_bTimeout = ( m_dLastCoordInterval > m_dAnswerTimeout_ms ) ;
      m_dLastCoordTime = dEntryTime ;
      m_CurrentPos.m_theta = dEntryTime ;
      return iID ;
    }
    else
      return FromZaber ;
  }

  LPCTSTR GetAdditionToSettings()
  {
    LPCTSTR szAddition = _T(",EditBox(CoordPrefix)"
      ",EditBox(IdPrefix)") ;
    return szAddition ;
  }
  bool ScanProperties( LPCTSTR text , bool& Invalidate )
  {
    FXPropertyKit pk( text );
    pk.GetString( _T( "CoordPrefix" ) , m_CoordPrefix ) ;
    pk.GetString( _T( "IdPrefix" ) , m_IDPrefix ) ;
  }
  bool PrintProperties( FXString& text )
  {
    FXPropertyKit pk;
    pk.WriteString( _T( "CoordPrefix" ) , m_CoordPrefix ) ;
    pk.WriteString( _T( "IdPrefix" ) , m_IDPrefix ) ;
    text += pk;
    return true;
  }
  bool IsAllCoordsAvailable( DWORD * pdwMask = NULL )
  {
    DWORD dwMask = (pdwMask) ? *pdwMask : m_ReceivedCoords ;
    return ((dwMask & MotionMaskX) != 0)
      && ((dwMask & MotionMaskY) != 0)
      && ((dwMask & MotionMaskZ) != 0) ;
  }

  cmplx GetCurrentCmplxPos()
  {
    return cmplx( m_CurrentPos.m_x , m_CurrentPos.m_y ) ;
  }

};

