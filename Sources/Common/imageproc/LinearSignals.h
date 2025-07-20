//  $File : LinearSignal.h - utility for one dimensional arrays processing
//  (C) Copyright The File X Ltd  2009-2023 
//
//
//
//  Revision History (excluding minor changes for specific compilers)
//   04 Sep 09 Firts release version, all followed changes must be listed below  

#ifndef _LINEAR_SIGNALS_INC
#define _LINEAR_SIGNALS_INC

#include <classes\DRect.h>
#include <limits>
#include <helpers\propertykitEx.h>

__forceinline double find_border_forw( double * pSignal ,
  int iSignalLen , double dThres , int iFilter = 0 )
{
  double *pD = pSignal /*+ 1*/ ;
  double *pEnd = pD + iSignalLen - 1 ;

  double dl = *pD ;
  double dc = *( ++pD ) ;

  // Left border searching
  if ( dl >= dThres ) // find black border on white background
  {
    do
    {
      if ( dc < dThres )
        return ( pD - pSignal - 1 ) + ( dl - dThres ) / ( dl - dc ) ;
      dl = dc ;
      dc = *( ++pD ) ;
    } while ( ( pD < pEnd - 1 ) ) ;
  }
  else                // find white border on black background
  {
    do
    {
      if ( dc > dThres )
        return ( pD - pSignal - 1 ) + ( dThres - dl ) / ( dc - dl ) ;
      dl = dc ;
      dc = *( ++pD ) ;
    } while ( ( pD < pEnd - 1 ) ) ;
  }
  return 0. ;
}

__forceinline double find_slope_forw( double * pSignal ,
  int iSignalLen , double dThres , bool black_to_white )
{
  double *pD = pSignal /*+ 1*/ ;
  double *pEnd = pD + iSignalLen - 1 ;

  if ( black_to_white )
  {
    while ( *pD >= dThres )
    {
      if ( ++pD >= pEnd )
        return 0. ;
    }
  }
  else
  {
    while ( *pD < dThres )
    {
      if ( ++pD >= pEnd )
        return 0. ;
    }
  }
  double dBegin = ( double ) ( pD - pSignal ) ;
  double dFound = find_border_forw( pD , ( int ) ( pEnd - pD ) , dThres ) ;
  return ( dFound ) ? dBegin + dFound : 0.0  ;
}

__forceinline double find_border_back( double * pSignal ,
  int iSignalLen , double dThres , int iFilter = 0 )
{
  double *pD = pSignal + iSignalLen - 1 ;
  double *pEnd = pSignal + 1 ;

  double dr = *pD ;
  double dc = *( --pD ) ;

  // Left border searching
  if ( dr >= dThres ) // find black border on white background
  {
    do
    {
      if ( dc < dThres )
        return ( pD - pSignal ) + ( dThres - dc ) / ( dr - dc ) ;
      dr = dc ;
      dc = *( --pD ) ;
    } while ( ( pD > pEnd ) ) ;
  }
  else                // find white border on black background
  {
    do
    {
      if ( dc > dThres )
        return ( pD - pSignal ) + ( dc - dThres ) / ( dc - dr ) ;
      dr = dc ;
      dc = *( --pD ) ;
    } while ( ( pD > pEnd ) ) ;
  }
  return 0. ;
}

__forceinline double find_slope_back( double * pSignal ,
  int iSignalLen , double dThres , bool black_to_white )
{
  double *pD = pSignal + iSignalLen - 1 ;
  double *pEnd = pSignal + 1 ;

  if ( black_to_white )
  {
    while ( *pD >= dThres )
    {
      if ( --pD <= pEnd )
        return 0. ;
    }
  }
  else
  {
    while ( *pD < dThres )
    {
      if ( --pD <= pEnd )
        return 0. ;
    }
  }
  return find_border_back( pSignal , ( int ) ( pD - pSignal ) , dThres ) ;
}

__forceinline double find_diff_border_forw( double * pSignal ,
  int iSignalLen , double dThres , double * pdDiffData = NULL )
{
  double DiffProfile[ 16000 ] ;
  double *pD = pSignal /*+ 1*/ ;
  double *pEnd = pD + iSignalLen - 1 ;

  if ( !pdDiffData )
  {
    double * pDiff = DiffProfile , *pNext = pD + 1 ;;

    do
    {
      *( pDiff++ ) = *( pNext++ ) - *( pD++ ) ;
    } while ( pD < pEnd );

    pdDiffData = DiffProfile ;
  }

  double * pDiff = pdDiffData ;

  pEnd = pdDiffData + iSignalLen - 1 ;

  if ( dThres > 0. )
  {
    while ( ( *pDiff < dThres ) && ( ++pDiff < pEnd ) ) ;

    int iInitPos = ( int ) ( pDiff - pdDiffData ) ;
    if ( ( iInitPos < 2 ) || ( iInitPos > iSignalLen - 2 ) )
      return 0. ;

//     while (( pDiff < pEnd - 1 ) && ( *( pDiff + 1 ) >= *pDiff )) 
//       pDiff++ ;

    if ( pDiff < pEnd - 1 )
    {
      double dFound = ( double ) ( ++pDiff - pdDiffData ) - 0.5 ;// because differential values
      double dToPrev = fabs( *( pDiff - 1 ) - *pDiff ) ;
      double dToNext = fabs( *( pDiff + 1 ) - *pDiff ) ;
      if ( dToNext > 1e-6 ) // may be saturation or anti saturation
        dFound += ( dToPrev / ( dToNext + dToPrev ) ) ;
      return dFound ;
    }
  }
  else
  {
    while ( ( *pDiff > dThres ) && ( ++pDiff < pEnd ) ) ;

    int iInitPos = ( int ) ( pDiff - pdDiffData ) ;
    if ( ( iInitPos < 2 ) || ( iInitPos > iSignalLen - 2 ) )
      return 0. ;

//     while ( ( pDiff < pEnd - 1 ) && ( *( pDiff + 1 ) >= *pDiff ) )
//       pDiff++ ;

    if ( pDiff < pEnd - 1 )
    {
      double dFound = ( double ) ( ++pDiff - pdDiffData ) - 0.5 ;// because differential values
      double dToPrev = fabs( *( pDiff - 1 ) - *pDiff ) ;
      double dToNext = fabs( *( pDiff + 1 ) - *pDiff ) ;
      if ( dToNext > 1e-6 ) // may be saturation or anti saturation
        dFound += ( dToPrev / ( dToNext + dToPrev ) ) ;
      return dFound ;
    }
  }
  return 0. ;
}

__forceinline double find_slope_forw( const LPBYTE pSignal ,
  int iSignalLen , double dThres )
{
  BYTE *pD = pSignal /*+ 1*/ ;
  BYTE *pEnd = pD + iSignalLen - 1 ;

  if ( *pD >= dThres )
  {
    while ( *pD >= dThres )
    {
      if ( ++pD >= pEnd )
        return 0. ;
    }
  }
  else
  {
    while ( *pD < dThres )
    {
      if ( ++pD >= pEnd )
        return 0. ;
    }
  }
  double dEdgePos = GetThresPosition( *( pD - 1 ) , *pD , dThres ) + ( pD - pSignal ) - 1 ;
  return dEdgePos ;
}

__forceinline double find_slope_back( const LPBYTE pSignal ,// Right edge
  int iSignalLen , double dThres )
{
  LPBYTE pD = pSignal ;
  LPBYTE pEnd = pSignal - iSignalLen ;

  if ( *pD >= dThres )
  {
    while ( *pD >= dThres )
    {
      if ( --pD <= pEnd )
        return 0. ;
    }
  }
  else
  {
    while ( *pD < dThres )
    {
      if ( --pD <= pEnd )
        return 0. ;
    }
  }
  double dEdgePos = GetThresPosition( *pD , *( pD - 1 ) , dThres ) - ( pSignal - pD ) + 1 ;
  return dEdgePos ;
}

__forceinline double find_slope_forw( const LPWORD pSignal ,
  int iSignalLen , double dThres )
{
  WORD *pD = pSignal /*+ 1*/ ;
  WORD *pEnd = pD + iSignalLen - 1 ;

  if ( *pD >= dThres )
  {
    while ( *pD >= dThres )
    {
      if ( ++pD >= pEnd )
        return 0. ;
    }
  }
  else
  {
    while ( *pD < dThres )
    {
      if ( ++pD >= pEnd )
        return 0. ;
    }
  }
  double dEdgePos = GetThresPosition( *( pD - 1 ) , *pD , dThres ) + ( pD - pSignal ) - 1 ;
  return dEdgePos ;
}

__forceinline double find_slope_back( const LPWORD pSignal ,// Right edge
  int iSignalLen , double dThres )
{
  LPWORD pD = pSignal ;
  LPWORD pEnd = pSignal - iSignalLen ;

  if ( *pD >= dThres )
  {
    while ( *pD >= dThres )
    {
      if ( --pD <= pEnd )
        return 0. ;
    }
  }
  else
  {
    while ( *pD < dThres )
    {
      if ( --pD <= pEnd )
        return 0. ;
    }
  }
  double dEdgePos = GetThresPosition( *pD , *( pD - 1 ) , dThres ) - ( pSignal - pD ) + 1 ;
  return dEdgePos ;
}

// find line cross by scan to left and to right from center
// Center approximately on pSignal +  SignalLen/2
inline double find_line_pos_lr( const LPBYTE pSignal ,
  int iSignalLen , double dNormThres , int iMinAmpl , double& dWidth )
{
  int iRange = iSignalLen ;
  BYTE Min = 255 , Max = 0 ;
  GetMinMax( pSignal - iRange, iSignalLen , Min , Max ) ;
  if ( Max - Min < iMinAmpl )
    return iSignalLen ;

  double dThres = Min + ( Max - Min ) * dNormThres ;
  double dRightPos = find_slope_forw( pSignal , iRange , dThres ) ;
  if ( dRightPos != 0. )
  {
    double dLeftPos = find_slope_back( pSignal , iRange , dThres ) ;
    if ( dLeftPos != 0. )
    {
      dWidth = dRightPos - dLeftPos ;
      double dCenter = ( dRightPos + dLeftPos ) / 2. ;
      return dCenter ;
    }
  }
  return iSignalLen ;
}

// find line cross by scan to left and to right from center
// Center approximately on pSignal +  SignalLen/2
inline double find_line_pos_lr( const LPWORD pSignal ,
  int iSignalLen , double dNormThres , int iMinAmpl , double& dWidth )
{
  int iRange = iSignalLen ;
  WORD Min = 65535 , Max = 0 ;
  GetMinMax( pSignal , iSignalLen , Min , Max ) ;
  if ( Max - Min < iMinAmpl )
    return iSignalLen ;

  double dThres = Min + ( Max - Min ) * dNormThres ;
  double dRightPos = find_slope_forw( pSignal , iRange , dThres ) ;
  if ( dRightPos != 0. )
  {
    double dLeftPos = find_slope_back( pSignal , iRange , dThres ) ;
    if ( dLeftPos != 0. )
    {
      dWidth = dRightPos - dLeftPos ;
      double dCenter = ( dRightPos + dLeftPos ) / 2. ;
      return dCenter ;
    }
  }
  return iSignalLen ;
}

// Slope searching in vertical direction on image: iStep is row length
// pSignal is pointer to approximate central pixel, edge is below, i.e. to plus direction
__forceinline double find_slope_forw( const LPBYTE pSignal ,
  int iSignalLen , double dThres , int iStep )
{
  BYTE *pD = pSignal /*+ 1*/ ;
  BYTE *pEnd = pD + (iSignalLen - 1) * iStep ;

  if ( *pD >= dThres )
  {
    while ( *pD >= dThres )
    {
      if ( (pD += iStep) >= pEnd )
        return 0. ;
    }
  }
  else
  {
    while ( *pD < dThres )
    {
      if ( ( pD += iStep ) >= pEnd )
        return 0. ;
    }
  }
  double dEdgePos = GetThresPosition( *( pD - iStep ) , *pD , dThres ) + (( pD - pSignal )/iStep) - 1 ;
  return dEdgePos ;
}

// Slope searching in vertical direction on image: iStep is row length
// pSignal is pointer to approximate central pixel, edge is below, i.e. to plus direction
__forceinline double find_slope_forw( LPWORD pSignal ,
  int iSignalLen , double dThres , int iStep )
{
  LPWORD pD = pSignal /*+ 1*/ ;
  LPWORD pEnd = pD + ( iSignalLen - 1 ) * iStep ;

  if ( *pD >= dThres )
  {
    while ( *pD >= dThres )
    {
      if ( ( pD += iStep ) >= pEnd )
        return 0. ;
    }
  }
  else
  {
    while ( *pD < dThres )
    {
      if ( ( pD += iStep ) >= pEnd )
        return 0. ;
    }
  }
  double dEdgePos = GetThresPosition( *( pD - iStep ) , *pD , dThres ) + ( ( pD - pSignal ) / iStep ) - 1 ;
  return dEdgePos ;
}

// Slope searching in vertical direction on image: iStep is row length
// pSignal is pointer to approximate central pixel, edge is above, i.e. to minus direction
__forceinline double find_slope_back( const LPWORD pSignal ,
  int iSignalLen , double dThres , int iStep )
{
  LPWORD pD = pSignal ;
  LPWORD pEnd = pSignal - ( iSignalLen * iStep ) ;

  if ( *pD >= dThres )
  {
    while ( *pD >= dThres )
    {
      if ( ( pD -= iStep ) <= pEnd )
        return 0. ;
    }
  }
  else
  {
    while ( *pD < dThres )
    {
      if ( ( pD -= iStep ) <= pEnd )
        return 0. ;
    }
  }
  double dEdgePos = GetThresPosition( *pD , *( pD - iStep ) , dThres ) - ( ( pSignal - pD ) / iStep ) ;
  return dEdgePos ;
}

// Slope searching in vertical direction on image: iStep is row length
// pSignal is pointer to approximate central pixel, edge is above, i.e. to minus direction
__forceinline double find_slope_back( const LPBYTE pSignal ,
  int iSignalLen , double dThres , int iStep )
{
  LPBYTE pD = pSignal ;
  LPBYTE pEnd = pSignal - ( iSignalLen * iStep ) ;

  if ( *pD >= dThres )
  {
    while ( *pD >= dThres )
    {
      if ( ( pD -= iStep ) <= pEnd )
        return 0. ;
    }
  }
  else
  {
    while ( *pD < dThres )
    {
      if ( ( pD -= iStep ) <= pEnd )
        return 0. ;
    }
  }
  double dEdgePos = GetThresPosition( *pD , *( pD + iStep ) , dThres ) - ( ( pSignal - pD ) / iStep ) ;
  return dEdgePos ;
}

// line measurement in vertical direction on image: iStep is row length

inline double find_line_pos_ud( const LPBYTE pSignal ,
  int iSignalLen , double dNormThres , 
  int iMinAmpl , double& dWidth , int iImageWidth )
{
  BYTE Min = 255 , Max = 0 ;

  int iRange = ( iSignalLen / 2 ) ;
  GetMinMax( pSignal - iImageWidth * iRange , iSignalLen , Min , Max , iImageWidth ) ;
  if ( Max - Min < iMinAmpl )
    return iSignalLen ;

  double dThres = Min + ( Max - Min ) * dNormThres ;
  int iCenterOffset = iRange * iImageWidth ;
  double dPlusPos = find_slope_forw( pSignal , iRange , dThres , iImageWidth ) ;
  if ( dPlusPos != 0. )
  {
    double dMinusPos = find_slope_back( pSignal , iRange , dThres , iImageWidth ) ;
    if ( dMinusPos != 0. )
    {
      dWidth = dPlusPos - dMinusPos ;
      double dCenter = ( dPlusPos + dMinusPos ) / 2. ;
      return dCenter ;
    }
  }
  return iSignalLen ;
}

// line measurement in vertical direction on image: iStep is row length

inline double find_line_pos_ud( LPWORD pSignal ,
  int iSignalLen , double dNormThres ,
  int iMinAmpl , double& dWidth , int iStep )
{
  WORD Min = 65535 , Max = 0 ;
  GetMinMax( pSignal , iSignalLen , Min , Max , iStep ) ;
  if ( Max - Min < iMinAmpl )
    return iSignalLen ;

  double dThres = Min + ( Max - Min ) * dNormThres ;
  int iRange = ( iSignalLen / 2 ) ;
  int iCenterOffset = iRange * iStep ;
  double dPlusPos = find_slope_forw( pSignal + iCenterOffset , iRange , dThres , iStep ) ;
  if ( dPlusPos != 0. )
  {
    double dMinusPos = find_slope_back( pSignal + iCenterOffset , iRange , dThres , iStep ) ;
    if ( dMinusPos != 0. )
    {
      dWidth = dPlusPos - dMinusPos ;
      double dCenter = ( dPlusPos + dMinusPos ) / 2. ;
      return dCenter ;
    }
  }
  return iSignalLen ;
}

inline LPBYTE find_slice_max_ud( const LPBYTE pSignal ,
  int iSignalLen , double dNormThres ,
  int iMinAmpl , int iImageWidth )
{
  BYTE Min = 255 , Max = 0 ;

  int iRange = ( iSignalLen / 2 ) ;
  LPBYTE pMax = GetMaxPos( pSignal - iImageWidth * iRange , iSignalLen , Min , Max , iImageWidth ) ;
  if ( Max - Min < iMinAmpl )
    return NULL ;
  return pMax ;
}

inline LPWORD find_slice_max_ud( const LPWORD pSignal ,
  int iSignalLen , double dNormThres ,
  int iMinAmpl , int iImageWidth )
{
  WORD Min = 65535 , Max = 0 ;

  int iRange = ( iSignalLen / 2 ) ;
  LPWORD pMax = GetMaxPos( pSignal - iImageWidth * iRange , iSignalLen , Min , Max , iImageWidth ) ;
  if ( Max - Min < iMinAmpl )
    return NULL ;
  return pMax ;
}


#endif  // _LINEAR_SIGNALS_INC
