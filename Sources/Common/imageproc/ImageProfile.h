#ifndef IMAGE_PROFILE_H
#define IMAGE_PROFILE_H

// 13:16 Wednesday, July 19, 2023
// Moisey Bernstein


#include <video/TVFrame.h>

class Profile
{
public:
  int m_iProfLen ;
  int m_iProfOrigin ;
  int m_iActiveLen ;
  double m_dMinValue ;
  double m_dMaxValue ;
  double * m_pProfData ;
  Profile( int iOrderedlen = 0 )
  {
    m_iProfLen = iOrderedlen ;
    m_dMinValue = DBL_MAX ;
    m_dMaxValue = -DBL_MAX ;
    m_iProfOrigin = m_iActiveLen = 0 ;
    m_pProfData = ( iOrderedlen ) ? new double[ m_iProfLen ] : NULL ;
    if ( m_pProfData )
      memset( m_pProfData , 0 , sizeof( double ) * m_iProfLen ) ;
  }

  Profile( Profile& Orig )
  {
    memcpy( this , &Orig , ( LPBYTE ) ( &m_pProfData ) - ( LPBYTE ) this ) ;
    if ( m_iProfLen )
    {
      m_pProfData = new double[ m_iProfLen ] ;
      memcpy( m_pProfData , Orig.m_pProfData , m_iProfLen * sizeof( double ) ) ;
    }
    else
      m_pProfData = NULL ;
  }
  ~Profile() { if ( m_pProfData ) delete[] m_pProfData ; } ;

  void Reset()
  {
    if ( m_pProfData )
    {
      delete[] m_pProfData ;
      m_pProfData = NULL ;
      m_iProfOrigin = m_iActiveLen = 0 ;
      m_dMinValue = DBL_MAX ;
      m_dMaxValue = -DBL_MIN ;
      m_iProfOrigin = 0 ;
    }
  }

  void Realloc( int iNewLen )
  {
    if ( iNewLen > m_iProfLen )
    {
      if ( m_pProfData )
        delete m_pProfData ;
      m_iProfLen = iNewLen ;
      m_pProfData = new double[ m_iProfLen ] ;
    }
    else
      m_iProfLen = iNewLen ;
    m_dMinValue = DBL_MAX ;
    m_dMaxValue = -DBL_MAX ;
    m_iProfOrigin = 0 ;
    if ( m_pProfData )
      memset( m_pProfData , 0 , sizeof( double ) * m_iProfLen ) ;
  } ;
  void Realloc( const Profile & Orig )
  {
    Realloc( Orig.m_iProfLen ) ;
    if ( Orig.m_iProfLen )
    {
      m_dMinValue = Orig.m_dMinValue ;
      m_dMaxValue = Orig.m_dMaxValue ;
      m_iProfOrigin = Orig.m_iProfOrigin ;
      memcpy_s( m_pProfData , m_iProfLen * sizeof( double ) ,
        Orig.m_pProfData , m_iProfLen * sizeof( double ) ) ;
    }
  }
  void Add( const Profile& Addition )
  {
    if ( m_iProfLen == Addition.m_iProfLen )
    {
      m_dMinValue = 1e30 ;
      m_dMaxValue = -1e30 ;
      for ( int i = 0 ; i < m_iProfLen ; i++ )
      {
        m_pProfData[ i ] += Addition.m_pProfData[ i ] ;
        if ( m_pProfData[ i ] < m_dMinValue )
          m_dMinValue = m_pProfData[ i ] ;
        if ( m_pProfData[ i ] > m_dMaxValue )
          m_dMaxValue = m_pProfData[ i ] ;
      }
    }
  }
  void Sub( const Profile& Addition )
  {
    if ( m_iProfLen == Addition.m_iProfLen )
    {
      m_dMinValue = 1e30 ;
      m_dMaxValue = -1e30 ;
      for ( int i = 0 ; i < m_iProfLen ; i++ )
      {
        m_pProfData[ i ] -= Addition.m_pProfData[ i ] ;
        if ( m_pProfData[ i ] < m_dMinValue )
          m_dMinValue = m_pProfData[ i ] ;
        if ( m_pProfData[ i ] > m_dMaxValue )
          m_dMaxValue = m_pProfData[ i ] ;
      }
    }
  }
  void Mult( const Profile& Addition )
  {
    if ( m_iProfLen == Addition.m_iProfLen )
    {
      m_dMinValue = 1e30 ;
      m_dMaxValue = -1e30 ;
      for ( int i = 0 ; i < m_iProfLen ; i++ )
      {
        m_pProfData[ i ] *= Addition.m_pProfData[ i ] ;
        if ( m_pProfData[ i ] < m_dMinValue )
          m_dMinValue = m_pProfData[ i ] ;
        if ( m_pProfData[ i ] > m_dMaxValue )
          m_dMaxValue = m_pProfData[ i ] ;
      }
    }
  }
  void Div( const Profile& Addition )
  {
    if ( m_iProfLen == Addition.m_iProfLen )
    {
      m_dMinValue = 1e30 ;
      m_dMaxValue = -1e30 ;
      for ( int i = 0 ; i < m_iProfLen ; i++ )
      {
        if ( Addition.m_pProfData[ i ] > 1e-5 )
          m_pProfData[ i ] /= Addition.m_pProfData[ i ] ;
        else
          m_pProfData[ i ] *= 1e5 ;
        if ( m_pProfData[ i ] < m_dMinValue )
          m_dMinValue = m_pProfData[ i ] ;
        if ( m_pProfData[ i ] > m_dMaxValue )
          m_dMaxValue = m_pProfData[ i ] ;
      }
    }
  }
  void Normalize()
  {
    double dAmpl = m_dMaxValue - m_dMinValue ;
    if ( dAmpl > 1e-10 )
    {
      for ( int i = 0 ; i < m_iProfLen ; i++ )
      {
        m_pProfData[ i ] -= m_dMinValue ;
        m_pProfData[ i ] /= dAmpl ;
      }
      m_dMaxValue = 1.0 ;
      m_dMinValue = 0. ;
    }
  }

} ;

typedef Profile * pProfile ;

__forceinline int _find_max( pTVFrame frame , CRect * Area = NULL )
{
  DWORD width = frame->lpBMIH->biWidth;
  DWORD height = frame->lpBMIH->biHeight;
  DWORD dwCompression = frame->lpBMIH->biCompression ;
  int max = 0 ;
  CRect rc = ( Area ) ? *Area : CRect( 0 , 0 , width , height ) ;
  if ( dwCompression == BI_Y8 || dwCompression == BI_Y800
    || dwCompression == BI_YUV9 || dwCompression == BI_YUV12 )
  {
    LPBYTE pData = GetData( frame );

    for ( int y = rc.top ; y <= rc.bottom ; y++ )
    {
      LPBYTE p = pData + ( width*y ) + rc.left ;
      LPBYTE pEnd = p + rc.Width() + 1 ;
      do
      {
        int iVal = *( p++ ) ;
        if ( max < iVal )
          max = iVal ;
      } while ( p <= pEnd );
    }
  }
  else if ( dwCompression == BI_Y16 )
  {
    LPWORD pData = ( LPWORD ) GetData( frame );

    for ( int y = rc.top ; y <= rc.bottom ; y++ )
    {
      LPWORD p = pData + ( width*y ) ;
      LPWORD pEnd = p + rc.Width() + 1 ;
      do
      {
        int iVal = *( p++ ) ;
        if ( max < iVal )
          max = iVal ;
      } while ( p <= pEnd );
    }
  }
  else
  {
    ASSERT( 0 ); // compression is not supported
    return 0 ;
  }
  return max ;
}

__forceinline int _find_min_max( const pTVFrame frame ,
  int& min , int& max , CRect& rc )
{
  int width = frame->lpBMIH->biWidth;
  int height = frame->lpBMIH->biHeight;
  CRect ProcArea ;
  if ( rc.left >= width || rc.right <= 0 || rc.top >= height || rc.top <= 0 )
    return 0 ;
  ProcArea.left = rc.left ;
  ProcArea.top = rc.top ;
  ProcArea.right = rc.right > width ? width : rc.right ;
  ProcArea.bottom = rc.bottom > height ? height : rc.bottom ;
  DWORD dwCompression = frame->lpBMIH->biCompression ;
  min = 10000000 ;
  max = 0 ;
  if ( dwCompression == BI_Y8 || dwCompression == BI_Y800
    || dwCompression == BI_YUV9 || dwCompression == BI_YUV12 )
  {
    LPBYTE pData = GetData( frame );

    for ( int y = ProcArea.top ; y < ProcArea.bottom ; y++ )
    {
      LPBYTE p = pData + ( width*y ) + rc.left ;
      LPBYTE pEnd = p + ProcArea.Width() ;
      do
      {
        int val = ( int ) *( p++ ) ;
        if ( min > val )
          min = val ;
        if ( max < val )
          max = val ;
      } while ( p <= pEnd );
    }
  }
  else if ( dwCompression == BI_Y16 )
  {
    LPWORD pData = ( LPWORD ) GetData( frame );

    for ( int y = ProcArea.top ; y < ProcArea.bottom ; y++ )
    {
      LPWORD p = pData + ( width*y ) + rc.left ;
      LPWORD pEnd = p + ProcArea.Width() ;
      do
      {
        int val = ( int ) *( p++ ) ;
        if ( min > val )
          min = val ;
        if ( max < val )
          max = val ;
      } while ( p <= pEnd );
    }
  }
  else
  {
    ASSERT( 0 ); // compression is not supported
    return 0 ;
  }
  return ( min + max ) / 2 ;
}

__forceinline int _find_min_max( const pTVFrame frame ,
  int& min , int& max )
{
  DWORD width = frame->lpBMIH->biWidth;
  DWORD height = frame->lpBMIH->biHeight;
  DWORD size = width * height;
  min = 100000 ;
  max = 0 ;
  if ( frame->lpBMIH->biCompression != BI_Y16 )
  {
    LPBYTE Data = GetData( frame );
    LPBYTE pEnd = Data + size ;

    while ( Data < pEnd )
    {
      int val = ( int ) *( Data++ ) ;
      if ( min > val )
        min = val ;
      if ( max < val )
        max = val ;
    }
  }
  else
  {
    LPWORD Data = ( LPWORD ) GetData( frame );
    LPWORD pEnd = Data + size ;

    while ( Data < pEnd )
    {
      int val = ( int ) *( Data++ ) ;
      if ( min > val )
        min = val ;
      if ( max < val )
        max = val ;
    }
  }
  return ( min + max ) / 2 ;
}

__forceinline double _find_min_max_and_profiles(
  const pTVFrame frame ,
  pProfile pHProf , pProfile pVProf ,
  int& min , int& max )
{
  DWORD width = frame->lpBMIH->biWidth;
  DWORD height = frame->lpBMIH->biHeight;
  if ( width < 3 || height < 3 )
    return 0. ;
  pHProf->Realloc( width );
  pVProf->Realloc( height );
  min = 100000 ;
  max = 0 ;
  double avgV = 0 ;
  double * pV = pVProf->m_pProfData ;
  DWORD dwCompression = frame->lpBMIH->biCompression ;
  if ( dwCompression == BI_Y8 || dwCompression == BI_Y800
    || dwCompression == BI_YUV9 || dwCompression == BI_YUV12 )
  {
    LPBYTE pData = GetData( frame );

    for ( DWORD y = 0; y < height ; y++ )
    {
      LPBYTE p = pData + ( width*y ) ;
      double * pH = pHProf->m_pProfData ;
      *pV = 0. ;
      for ( DWORD x = 0 ; x < width ; x++ )
      {
        int val = ( int ) *( p++ ) ;
        if ( min > val )
          min = val ;
        if ( max < val )
          max = val ;

        ( *pV ) += val;
        ( *( pH++ ) ) += val;
      }
      ( *pV ) /= width ;
      if ( pVProf->m_dMinValue > *pV )
        pVProf->m_dMinValue = *pV ;
      if ( pVProf->m_dMaxValue < *pV )
        pVProf->m_dMaxValue = *pV ;
      avgV += *( pV++ ) ;
    }
    double * pH = pHProf->m_pProfData ;
    for ( DWORD x = 0 ; x < width ; x++ )
    {

      double dVal = ( *( pH++ ) /= height ) ;
      if ( pHProf->m_dMinValue > dVal )
        pHProf->m_dMinValue = dVal ;
      if ( pHProf->m_dMaxValue < dVal )
        pHProf->m_dMaxValue = dVal ;
    }
  }
  else if ( dwCompression == BI_Y16 )
  {
    LPWORD pData = ( LPWORD ) GetData( frame );

    for ( DWORD y = 0; y < height ; y++ )
    {
      LPWORD p = pData + ( width*y ) ;
      double * pH = pHProf->m_pProfData ;
      *pV = 0. ;
      for ( DWORD x = 0 ; x < width ; x++ )
      {
        int val = ( int ) *( p++ ) ;
        if ( min > val )
          min = val ;
        if ( max < val )
          max = val ;

        ( *pV ) += val;
        ( *( pH++ ) ) += val;
      }
      ( *pV ) /= width ;
      if ( pVProf->m_dMinValue > *pV )
        pVProf->m_dMinValue = *pV ;
      if ( pVProf->m_dMaxValue < *pV )
        pVProf->m_dMaxValue = *pV ;
      avgV += *( pV++ ) ;
    }
    double * pH = pHProf->m_pProfData ;
    for ( DWORD x = 0 ; x < width ; x++ )
    {
      double dVal = ( *( pH++ ) /= height ) ;
      if ( pHProf->m_dMinValue > dVal )
        pHProf->m_dMinValue = dVal ;
      if ( pHProf->m_dMaxValue < dVal )
        pHProf->m_dMaxValue = dVal ;
    }
  }
  else
    return 0. ;

  return avgV / height  ;
}

// right in ROI is width (i.e. right side is left + right)
// bottom in ROI holds height, i.e. real bottom is top + bottom
__forceinline double calc_profiles( const pTVFrame frame ,
  double * hdistr , double * vdistr ,
  double& dHMin , double& dHMax , double& dVMin , double& dVMax ,
  CRect * pROIWH = NULL )
{
  DWORD width = frame->lpBMIH->biWidth;
  DWORD height = frame->lpBMIH->biHeight;
  if ( width < 3 || height < 3 )
    return 0. ;

  int iXLeft = ( pROIWH ) ? pROIWH->left : 0 ;
  int iYtop = ( pROIWH ) ? pROIWH->top : 0 ;
  int iXRight = ( pROIWH ) ? pROIWH->left + pROIWH->right : width ;
  int iYBottom = ( pROIWH ) ? pROIWH->top + pROIWH->bottom : height ;

  dVMin = dHMin = DBL_MAX ;
  dVMax = dHMax = -DBL_MAX ;
  memset( hdistr , 0 , width * sizeof( double ) ) ;
  memset( vdistr , 0 , height * sizeof( double ) ) ;
  double avgV = 0 ;
  double * pV = vdistr + iYtop ;
  DWORD dwCompression = frame->lpBMIH->biCompression ;
  if ( dwCompression == BI_Y8 || dwCompression == BI_Y800
    || dwCompression == BI_YUV9 || dwCompression == BI_YUV12 )
  {
    LPBYTE pData = GetData( frame );

//     for (DWORD y=1; y < height-1 ; y++)
    for ( int y = iYtop; y < iYBottom ; y++ )
    {
      LPBYTE p = pData + ( width*y ) + iXLeft ;
      double * pH = hdistr + iXLeft ;
      *pV = 0. ;
//       for ( DWORD x = 1 ; x < width-1 ; x++ )
      for ( int x = iXLeft ; x < iXRight ; x++ )
      {
        double v = *( p++ ) ;
        ( *pV ) += v;
        ( *( pH++ ) ) += v;
      }
//       (*pV) /= (width - 2) ;
      ( *pV ) /= iXRight - iXLeft ;
      if ( *pV < dVMin )
        dVMin = *pV ;
      if ( *pV > dVMax )
        dVMax = *pV ;
      avgV += *( pV++ ) ;
    }
  }
  else if ( dwCompression == BI_Y16 )
  {
    LPWORD pData = ( LPWORD ) GetData( frame );

//     for (DWORD y=1; y < height-1 ; y++)
    for ( int y = iYtop; y < iYBottom ; y++ )
    {
      LPWORD p = pData + ( width*y ) + iXLeft ;
      double * pH = hdistr + iXLeft ;
      *pV = 0. ;
//       for ( DWORD x = 1 ; x < width-1 ; x++ )
      for ( int x = iXLeft ; x < iXRight ; x++ )
      {
        double v = *( p++ ) ;
        ( *pV ) += v;
        ( *( pH++ ) ) += v;
      }
//       (*pV) /= (width - 2) ;
      ( *pV ) /= iXRight - iXLeft ;
      if ( *pV < dVMin )
        dVMin = *pV ;
      if ( *pV > dVMax )
        dVMax = *pV ;
      avgV += *( pV++ ) ;
    }
  }
  else
    return 0. ;

  //   for ( DWORD x=1 ; x < width-1 ; x++ )
  double * pH = hdistr + iXLeft ;
  for ( int i = iXLeft ; i < iXRight ; i++ , pH++ )
  {
    *pH /= iYBottom - iYtop ;
    if ( *pH < dHMin )
      dHMin = *pH ;
    if ( *pH > dHMax )
      dHMax = *pH ;
  }
//   return avgV / (height - 2)  ;
  return avgV / ( iYBottom - iYtop ) ;
}

// right in ROI is width (i.e. right side is left + right)
// bottom in ROI holds height, i.e. real bottom is top + bottom
__forceinline double calc_profiles( const pTVFrame frame ,
  pProfile hProf , pProfile vProf , CRect * pROIWH = NULL )
{
  hProf->Realloc( frame->lpBMIH->biWidth );
  vProf->Realloc( frame->lpBMIH->biHeight );
  return calc_profiles( frame , hProf->m_pProfData , vProf->m_pProfData ,
    hProf->m_dMinValue , hProf->m_dMaxValue ,
    vProf->m_dMinValue , vProf->m_dMaxValue , pROIWH ) ;
}

// right in ROI is width (i.e. right side is left + right)
// bottom in ROI holds height, i.e. real bottom is top + bottom
__forceinline double calc_hprofile( const pTVFrame frame ,
  double * hdistr , double& dHMin , double& dHMax ,
  CRect * pROIWH = NULL )
{
  DWORD dwImageW = frame->lpBMIH->biWidth;
  DWORD dwImageH = frame->lpBMIH->biHeight;
  if ( dwImageW < 3 || dwImageH < 3 )
    return 0. ;

  int iXLeft = ( pROIWH ) ? pROIWH->left : 0 ;
  int iYTop = ( pROIWH ) ? pROIWH->top : 0 ;
  int iXRight = ( pROIWH ) ? pROIWH->left + pROIWH->right : dwImageW ;
  int iYBottom = ( pROIWH ) ? pROIWH->top + pROIWH->bottom : dwImageH ;

  DWORD dwActiveW = ( iXRight - iXLeft + 1 ) ; // both ends included
  DWORD dwActiveH = ( iYBottom - iYTop  + 1 ) ; // both ends included
  dHMin = DBL_MAX ;
  dHMax = -DBL_MAX ;
  //memset( hdistr , 0 , dwImageH * sizeof( double ) ) ;
  double avgV = 0 , dAvgAll = 0. ;
  double * pH = hdistr ; // ptr for vertical motion
  DWORD dwCompression = frame->lpBMIH->biCompression ;
  if ( dwCompression == BI_Y8 || dwCompression == BI_Y800
    || dwCompression == BI_YUV9 || dwCompression == BI_YUV12 )
  {
    LPBYTE pData = GetData( frame );

    LPBYTE pOrigin = pData + (iYTop * dwImageW ) + iXLeft ;
    LPBYTE pX = pOrigin ; // dynamic ptr to necessary data
    LPBYTE pEndX = pX + dwActiveW ; // both ends included
    LPBYTE p = pX ; // ptr for vertical motion in column
    LPBYTE pEndY = p + ( iYBottom - iYTop + 1 ) * dwImageW ;
    do
    {
      do 
      {
        avgV += *p ;
        p += dwImageW ;
      } while ( p < pEndY );
      p = ++pX ;
      *(pH++) = ( avgV /= dwActiveH );// avg, save and go to next column
      SetMinMax( avgV , dHMin , dHMax ) ;
      dAvgAll += avgV ;
      avgV = 0. ;
    } while ( p < pEndX ) ;
  }
  else if ( dwCompression == BI_Y16 )
  {
    LPWORD pwData = ( LPWORD ) GetData( frame );
    LPWORD pOrigin = pwData + ( iYTop * dwImageW ) + iXLeft ;
    LPWORD pX = pOrigin ; // dynamic ptr to necessary data
    LPWORD pEndX = pX + dwActiveW ; // both ends included
    LPWORD p = pX ; // ptr for vertical motion in column
    LPWORD pEndY = p + ( iYTop - iYBottom + 1 ) * dwImageW ;
    do
    {
      do
      {
        avgV += *p ;
        p += dwImageW ;
      } while ( p < pEndY );
      p = ++pX ;
      *( pH++ ) = ( avgV /= dwActiveH );// avg, save and go to next column
      avgV = 0. ;
    } while ( p < pEndX ) ;

  }
  else
    return 0. ;

  return dAvgAll / dwActiveW ;
}

// This version working for full profile length (m_Origin and m_ActiveLen is not used)
__forceinline double localize( pProfile pProf , int iLocalizeLen )
{
  double dMin = DBL_MAX , dProfMax = -DBL_MAX , dProfMin = DBL_MAX ;
  double dAvg = 0. ;
  double * pUpdated = new double[ pProf->m_iProfLen ] ;
  // find min value for left segment of (iLocalizeLen/2) length
  double * pValue = pProf->m_pProfData ;
  double * pEnd = pValue + iLocalizeLen/2 ;
  while ( pValue < pEnd )
  {
    double dValue = *( pValue++ ) ;
    if ( dValue < dMin )
      dMin = dValue ;
  }
  pValue = pProf->m_pProfData ;
  double * pSave = pUpdated ;
  double * pIn = pEnd ;

  // Fill first (iLocalizeLen / 2) elements with dMin update
  while ( pValue < pEnd )
  {
    double dSaveValue = *( pValue++ ) - dMin ; 
    *( pSave++ ) = dSaveValue ;// save updated value
    dAvg += dSaveValue ;
    SetMinMax( dSaveValue , dProfMin , dProfMax ) ;
    double dInValue = *( pIn++ ) ;
    if ( dInValue < dMin ) // analysis for minimum
      dMin = dInValue ;
  }

  double * pOut = pProf->m_pProfData ;
  pEnd = pOut + pProf->m_iProfLen - (iLocalizeLen / 2) ;
  // go to the end of profile minus iLocalizeLen/2
  // with minimum updates
  while ( pValue < pEnd )
  {
    double dSaveValue = *( pValue++ ) - dMin ;
    *( pSave++ ) = dSaveValue ;// save updated value
    dAvg += dSaveValue ;
    SetMinMax( dSaveValue , dProfMin , dProfMax ) ;
    if ( *(pOut++) <= dMin ) // minimal value go out of working interval
    {
      // find new minimum
      double * pLocalValue = pOut ;
      double * pLocalEnd = pLocalValue + iLocalizeLen - 1 ;
      dMin = DBL_MAX ;
      while ( pLocalValue < pLocalEnd )
      {
        double dValue = *( pLocalValue++ ) ;
        if ( dValue < dMin )
          dMin = dValue ;
      }
      pIn = pLocalEnd ;
    }
    else
    {
      double dInValue = *( pIn++ ) ;
      if ( dInValue < dMin ) // analysis for minimum
        dMin = dInValue ;
    }
  }
  // the rest - last iLocalizeLen/2 segment
  pEnd += ( iLocalizeLen / 2 ) ;
  while ( pValue < pEnd )
  {
    double dSaveValue = *( pValue++ ) - dMin ;
    *( pSave++ ) = dSaveValue ;// save updated value
    dAvg += dSaveValue ;
    SetMinMax( dSaveValue , dProfMin , dProfMax ) ;
  }

  memcpy( pProf->m_pProfData , pUpdated , pProf->m_iProfLen * sizeof( double) ) ;
  pProf->m_dMinValue = dProfMin ;
  pProf->m_dMaxValue = dProfMax ;
  delete[] pUpdated ;
  dAvg /= pProf->m_iProfLen ;
  return dAvg ;
}
// right in ROI is width (i.e. right side is left + right)
// bottom in ROI holds height, i.e. real bottom is top + bottom
__forceinline double calc_hprofile( const pTVFrame frame ,
  pProfile hProf , CRect * pROIWH = NULL )
{
  int iProfLength = ( pROIWH ) ? pROIWH->right : frame->lpBMIH->biWidth ;
  hProf->Realloc( iProfLength );
  return calc_hprofile( frame , hProf->m_pProfData , 
    hProf->m_dMinValue , hProf->m_dMaxValue , pROIWH ) ;
}

// right in ROI is width (i.e. right side is left + right)
// bottom in ROI holds height, i.e. real bottom is top + bottom
__forceinline double calc_vprofile( const pTVFrame frame ,
  double * vdistr , double& dVMin , double& dVMax ,
  CRect * pROIWH = NULL )
{
  DWORD dwImageW = frame->lpBMIH->biWidth;
  DWORD dwImageH = frame->lpBMIH->biHeight;
  if ( dwImageW < 3 || dwImageH < 3 )
    return 0. ;

  int iXLeft = ( pROIWH ) ? pROIWH->left : 0 ;
  int iYTop = ( pROIWH ) ? pROIWH->top : 0 ;
  int iXRight = ( pROIWH ) ? pROIWH->left + pROIWH->right : dwImageW ;
  int iYBottom = ( pROIWH ) ? pROIWH->top + pROIWH->bottom : dwImageH ;

  DWORD dwActiveW = ( iXRight - iXLeft + 1 ) ; // both ends included
  DWORD dwActiveH = ( iYBottom - iYTop + 1 ) ; // both ends included
  dVMin = DBL_MAX ;
  dVMax = -DBL_MAX ;
  //memset( hdistr , 0 , dwImageH * sizeof( double ) ) ;
  double avgH = 0 , dAvgAll = 0. ;
  double * pV = vdistr ; // ptr for horizontal motion
  DWORD dwCompression = frame->lpBMIH->biCompression ;
  if ( dwCompression == BI_Y8 || dwCompression == BI_Y800
    || dwCompression == BI_YUV9 || dwCompression == BI_YUV12 )
  {
    LPBYTE pData = GetData( frame );

    LPBYTE pOrigin = pData + ( iYTop * dwImageW ) + iXLeft ;
    LPBYTE pX = pOrigin ; // dynamic ptr to necessary data
    LPBYTE p = pX ; // ptr for horizontal motion in row
    LPBYTE pEndY = p + dwActiveH * dwImageW ;
    do
    {
      LPBYTE pEndX = pX + dwActiveW ; // both ends included
      do
      {
        avgH += *(p++) ;
      } while ( p < pEndX );
      p = (pX += dwImageW) ; // go to next row
      *( pV++ ) = ( avgH /= dwActiveW );// avg, save and go to next column
      SetMinMax( avgH , dVMin , dVMax ) ;
      dAvgAll += avgH ;
      avgH = 0. ;
    } while ( p < pEndY ) ;
  }
  else if ( dwCompression == BI_Y16 )
  {
    LPWORD pData = (LPWORD)GetData( frame );

    LPWORD pOrigin = pData + ( iYTop * dwImageW ) + iXLeft ;
    LPWORD pX = pOrigin ; // dynamic ptr to necessary data
    LPWORD p = pX ; // ptr for horizontal motion in row
    LPWORD pEndY = p + ( iYTop - iYBottom + 1 ) * dwImageW ;
    do
    {
      LPWORD pEndX = pX + dwActiveW ; // both ends included
      do
      {
        avgH += *( p++ ) ;
      } while ( p < pEndX );
      p = pX + dwImageW ; // go to next row
      *( pV++ ) = ( avgH /= dwActiveW );// avg, save and go to next column
      dAvgAll += avgH ;
      avgH = 0. ;
    } while ( p < pEndY ) ;
  }
  else
    return 0. ;

  return dAvgAll / dwActiveH ;
}

// right in ROI is width (i.e. right side is left + right)
// bottom in ROI holds height, i.e. real bottom is top + bottom
__forceinline double calc_vprofile( const pTVFrame frame ,
  pProfile vProf , CRect * pROIWH = NULL )
{
  int iProfLength = ( pROIWH ) ? pROIWH->bottom + 1 : frame->lpBMIH->biHeight ;
  vProf->Realloc( iProfLength );
  return calc_vprofile( frame , vProf->m_pProfData ,
    vProf->m_dMinValue , vProf->m_dMaxValue , pROIWH ) ;
}

__forceinline bool calc_diff_profiles(
  const pTVFrame frame , int iDist , double * hdistr , double * vdistr ,
  double& dHMin , double& dHMax , double& dVMin , double& dVMax ,
  const CRect * pROIWH = NULL )
{
  bool bAbsValue = ( iDist < 0 ) ;
  iDist = abs( iDist ) ;
  int width = frame->lpBMIH->biWidth;
  int height = frame->lpBMIH->biHeight;
  if ( width < iDist + 3 || height < iDist + 3 )
    return false ;

  int iXLeft = ( pROIWH ) ? pROIWH->left : 0 ;
  int iYtop = ( pROIWH ) ? pROIWH->top : 0 ;
  int iXRight = ( pROIWH ) ? pROIWH->left + pROIWH->right : width ;
  int iYBottom = ( pROIWH ) ? pROIWH->top + pROIWH->bottom : height ;

  dVMin = dHMin = 10e30 ;
  dVMax = dHMax = -10e30 ;
  memset( hdistr , 0 , width * sizeof( double ) ) ;
  double avgV = 0 ;
  double * pV = vdistr + iYtop + iDist ;
  DWORD dwCompression = frame->lpBMIH->biCompression ;
  int iVertDist = iDist * width ;
  if ( dwCompression == BI_Y8 || dwCompression == BI_Y800
    || dwCompression == BI_YUV9 || dwCompression == BI_YUV12 )
  {
    LPBYTE pData = GetData( frame );

    for ( int y = iYtop ; y < iYBottom ; y++ )
    {
      LPBYTE p = pData + ( width*y ) + iXLeft + iDist ;
      double * pH = hdistr + iXLeft + iDist ;
      bool bYCalc = ( y >= ( iYtop + iDist ) ) ;
      *pV = 0. ;
      for ( int x = iXLeft + iDist ; x < iXRight ; x++ , p++ )
      {

        double v = *p - *( p - iDist ) ;
        ( *( pH++ ) ) += ( bAbsValue ) ? fabs( v ) : v ;
        if ( bYCalc )
        {
          v = *p - *( p - iVertDist ) ;
          ( *pV ) += ( bAbsValue ) ? fabs( v ) : v ;
        }
      }
      pH = hdistr + iXLeft ;
      double dFirstHVal = *( hdistr + iXLeft + iDist ) ;
      for ( int x = 0 ; x < iDist ; x++ )
        *( pH++ ) = dFirstHVal ;

      ( *pV ) /= iXRight - iXLeft - iDist ;
      if ( *pV < dVMin )
        dVMin = *pV ;
      if ( *pV > dVMax )
        dVMax = *pV ;
      avgV += *( pV++ ) ;
    }
  }
  else if ( dwCompression == BI_Y16 )
  {
    LPWORD pData = ( LPWORD ) GetData( frame );

    for ( int y = iYtop + iDist; y < iYBottom ; y++ )
    {
      LPWORD p = pData + ( width*y ) + iXLeft + iDist ;
      double * pH = hdistr + iXLeft + iDist ;
      bool bYCalc = ( y >= ( iYtop + iDist ) ) ;
      *pV = 0. ;
      for ( int x = iXLeft + iDist ; x < iXRight ; x++ , p++ )
      {

        double v = *p - *( p - iDist ) ;
        ( *( pH++ ) ) += ( bAbsValue ) ? fabs( v ) : v ;
        if ( bYCalc )
        {
          v = *p - *( p - iVertDist ) ;
          ( *pV ) += ( bAbsValue ) ? fabs( v ) : v ;
        }
      }
      pH = hdistr + iXLeft ;
      double dFirstHVal = *( hdistr + iXLeft + iDist ) ;
      for ( int x = 0 ; x < iDist ; x++ )
        *( pH++ ) = dFirstHVal ;

      ( *pV ) /= iXRight - iXLeft - iDist ;
      if ( *pV < dVMin )
        dVMin = *pV ;
      if ( *pV > dVMax )
        dVMax = *pV ;
      avgV += *( pV++ ) ;
    }
  }
  else
    return false ;

  for ( int x = 0 ; x < width ; x++ )
  {
    hdistr[ x ] /= iYBottom - iYtop ;
    if ( hdistr[ x ] < dHMin )
      dHMin = hdistr[ x ] ;
    if ( hdistr[ x ] > dHMax )
      dHMax = hdistr[ x ] ;
  }
  return true ;
}

// right in ROI is width (i.e. right side is left + right)
// bottom in ROI holds height, i.e. real bottom is top + bottom
__forceinline bool calc_diff_profiles( const pTVFrame frame , int iDist ,
  pProfile hProf , pProfile vProf , const CRect * pROIWH = NULL )
{
  hProf->Realloc( frame->lpBMIH->biWidth );
  vProf->Realloc( frame->lpBMIH->biHeight );
  return calc_diff_profiles( frame , iDist ,
    hProf->m_pProfData , vProf->m_pProfData ,
    hProf->m_dMinValue , hProf->m_dMaxValue ,
    vProf->m_dMinValue , vProf->m_dMaxValue , pROIWH ) ;
}


// Find line position by two threshold crossing
// If Prof.m_iActiveLen is not zero, search and measure will be done 
// for active len only
// Function will find first edge (threshold cross) and following edge 
//    ( threshold cross to opposite direction)
// If first is not found, function returns zero
// If first edge found and following is not, function returns zero
// If both edges found, function returns center position between edges
//   relatively to profile begin (not m_iProfOrigin)


inline double FindProfileCrossesPosition( Profile& Prof , // profile data
  int& iStartIndex ,   // the first point for line search
  double dNormThres ,   // normalized threshold (0.,1.) range
  double& dWidth )
{
  double * p = Prof.m_pProfData + iStartIndex ;
  double * pEnd = Prof.m_pProfData + 
   ( ( Prof.m_iActiveLen ) ? Prof.m_iActiveLen : Prof.m_iProfLen) ;
  double dThres = Prof.m_dMinValue 
    + dNormThres * (Prof.m_dMaxValue - Prof.m_dMinValue) ;
  bool bPositive = (*p >= dThres) ;
  double * pFront = p ;
  if ( bPositive )
  {
    while( (*(++p) >= dThres) && (p < pEnd) ) ;
    if ( p >= pEnd )
      return 0. ;
    pFront = p  ;
    while ( ( *( ++p ) < dThres ) && ( p < pEnd ) ) ;
    if ( p >= pEnd )
      return 0. ;
  }
  else
  {
    while ( ( *( ++p ) < dThres ) && ( p < pEnd ) ) ;
    if ( p >= pEnd )
      return 0. ;
    pFront = p  ;
    while ( ( *( ++p ) >= dThres ) && ( p < pEnd ) ) ;
    if ( p >= pEnd )
      return 0. ;
  }
  double d1stPos = GetThresPosition( *( pFront - 1 ) , *pFront , dThres ) ;
  d1stPos += pFront - Prof.m_pProfData - 1 ;
  double d2ndPos = GetThresPosition( *( p - 1 ) , *p , dThres ) ;
  d2ndPos += p - Prof.m_pProfData - 1 ;
  double dPos = 0.5 * ( d1stPos + d2ndPos ) ;
  dWidth = d2ndPos - d1stPos ;
  iStartIndex = (int)(p - Prof.m_pProfData) ;
  return dPos ;
}

inline double ProfileLocalNormalize(
  Profile& InProfile , Profile& OutProfile , 
  double dMinAmpl , int iLocalRange , double dMaxValue )
{
  if ( OutProfile.m_pProfData && (OutProfile.m_iProfLen != InProfile.m_iProfLen) )
    OutProfile.Realloc( InProfile ) ;

  double dMin = DBL_MAX , dMax = -DBL_MAX ;
  int iRange = InProfile.m_iActiveLen ;
  if ( iRange > iLocalRange )
    iRange = iLocalRange ;
  double * pInData = InProfile.m_pProfData + InProfile.m_iProfOrigin ;
  double * pInDataOrigin = pInData ;
  double * pInEnd = pInData + iRange ;
  double * pInDataOut = pInData ; // sample going out of window
  double * pInDataOutEnd = pInDataOut + InProfile.m_iActiveLen - 1 ;

  double * pOutData = OutProfile.m_pProfData + OutProfile.m_iProfOrigin ;
  double * pOutDataOrigin = pOutData ;
  double * pOutEnd = pOutData + OutProfile.m_iActiveLen - 1 ;

  double dRunningSum = 0. ;
  double * pInitEnd = pInDataOrigin + iRange ;
  GetMinMaxDbl( pInData , iRange , dMin , dMax ) ;

  double dAbsAmpl = InProfile.m_dMaxValue - InProfile.m_dMinValue ;
  double dAbsMinAmpl = dAbsAmpl * dMinAmpl ;
  pInitEnd = pInDataOrigin + iRange / 2 ;
  double dDiff = ( dMax - dMin ) ;
//  double dK = dDiff >= dMinAmpl ? dMaxValue / dDiff : 0. ;
  double dK = (dDiff >= dAbsMinAmpl) ? dMaxValue / dDiff : 0. ;
  OutProfile.m_dMinValue = DBL_MAX , OutProfile.m_dMaxValue = -DBL_MAX ;
  if ( dK == 0. ) // no signal, simple copy
  {
//     memcpy( pOutDataOrigin , pInDataOrigin , iRange * sizeof( double ) / 2 ) ;
//     pInData += iRange / 2 ;
//     pOutData += iRange / 2 ;
    for ( int i = 0 ; i < iRange / 2 ; i++ )
      *( pOutData++ ) = dMaxValue ;
    pInData += iRange / 2 ;
  }
  else
  {
    while ( pInData < pInitEnd )
    {
      double dOldVal = *( pInData++ ) ;
      double dNewVal = ( dOldVal - dMin ) * dK ;
      *( pOutData++ ) = dNewVal ;
      SetMinMax( dNewVal , OutProfile.m_dMinValue , OutProfile.m_dMaxValue ) ;
    }
  }

  // Now Range/2 is processed, go forward
  do
  {
    int iInIndex = (int)(pInData - pInDataOrigin) ;
    int iOutIndex = ( int ) (pOutData - pOutDataOrigin) ;
    double dLastCorrect = dMaxValue ;
    double dInOutVal = *( pInDataOut++ ) ; // value going out of analysis window
//     if ( ((dInOutVal == dMin)  // minimal value going out
//          && ( *pInDataOut > dMin + 0.1 )) // if not about the same -> recalcultate
//       || (( dInOutVal == dMax ) // Maximal value is going out
//          && ( *pInDataOut < dMax - 0.1 )) // if not about the same -> recalcultatem_iS
//       || (dK == 0.) )
//     {  // recalculate min, max and K
      dMin = DBL_MAX , dMax = -DBL_MAX ;
      int iTheRest = ( int ) (pInDataOutEnd - pInDataOut) ;
      
      if ( iTheRest > iRange )
        GetMinMaxDbl( pInDataOut , iRange , dMin , dMax ) ;
      dDiff = ( dMax - dMin ) ;
      dK = dDiff >= dAbsMinAmpl ? dMaxValue / dDiff : 0. ;
//     }
    double dOldVal = *( pInData++ ) ;
    ASSERT( (dK == 0.) || (dOldVal >= dMin) ) ;
    double dNewVal = dK > 0. ?
      (dLastCorrect = (( dOldVal - dMin ) * dK)) : dLastCorrect ;
    *( pOutData++ ) = dNewVal ;
    SetMinMax( dNewVal , OutProfile.m_dMinValue , OutProfile.m_dMaxValue ) ;
  } while ( pOutData < pOutEnd );
  return 0.5 * ( OutProfile.m_dMinValue + OutProfile.m_dMaxValue ) ;
}

#endif //IMAGE_PROFILE_H