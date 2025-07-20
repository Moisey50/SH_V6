//  $File : seekspot.h - utility for coordinating spots
//  (C) Copyright The File X Ltd 2009. 
//
//
//
//  Revision History (excluding minor changes for specific compilers)
//   04 Sep 09 Firts release version, all followed changes must be listed below  

#ifndef _SEEK_SPOTS_INC
#define _SEEK_SPOTS_INC

#include <classes\DRect.h>
#include <limits>
#include <video\tvframe.h>

#define DISPERTION_TRIGGER 5000
#define MIN_AMPL      15
#define MIN_DEVIATION 2
#define DEFAULT_NORM_THRES 0.5


class Profile
{
public:
  int m_iProfLen ;
  int m_iProfOrigin ;
  double m_dMinValue ;
  double m_dMaxValue ;
  double * m_pProfData ;
  Profile( int iOrderedlen = 0 )
  {
    m_iProfLen = iOrderedlen ;
    m_dMinValue = 1e30 ;
    m_dMaxValue = -1e30 ;
    m_iProfOrigin = 0 ;
    m_pProfData = ( iOrderedlen ) ? new double[ m_iProfLen ] : NULL ;
  }
  ~Profile() { if ( m_pProfData ) delete[] m_pProfData ; } ;
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
    m_dMinValue = 1e30 ;
    m_dMaxValue = -1e30 ;
    m_iProfOrigin = 0 ;
    if ( m_pProfData )
      memset( m_pProfData , 0 , sizeof(double) * m_iProfLen ) ;
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
        m_pProfData[i] += Addition.m_pProfData[i] ;
        if ( m_pProfData[i] < m_dMinValue )
          m_dMinValue = m_pProfData[i] ;
        if ( m_pProfData[i] > m_dMaxValue )
          m_dMaxValue = m_pProfData[i] ;
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
        m_pProfData[i] -= Addition.m_pProfData[i] ;
        if ( m_pProfData[i] < m_dMinValue )
          m_dMinValue = m_pProfData[i] ;
        if ( m_pProfData[i] > m_dMaxValue )
          m_dMaxValue = m_pProfData[i] ;
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
        m_pProfData[i] *= Addition.m_pProfData[i] ;
        if ( m_pProfData[i] < m_dMinValue )
          m_dMinValue = m_pProfData[i] ;
        if ( m_pProfData[i] > m_dMaxValue )
          m_dMaxValue = m_pProfData[i] ;
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
        if ( Addition.m_pProfData[i] > 1e-5 )
          m_pProfData[i] /= Addition.m_pProfData[i] ;
        else
          m_pProfData[i] *= 1e5 ;
        if ( m_pProfData[i] < m_dMinValue )
          m_dMinValue = m_pProfData[i] ;
        if ( m_pProfData[i] > m_dMaxValue )
          m_dMaxValue = m_pProfData[i] ;
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
        m_pProfData[i] -= m_dMinValue ;
        m_pProfData[i] /= dAmpl ;
      }
      m_dMaxValue = 1.0 ;
      m_dMinValue = 0. ;
    }
  }

} ;
typedef Profile * pProfile ;

__forceinline int _find_max(pTVFrame frame , CRect * Area = NULL )
{
  DWORD width = frame->lpBMIH->biWidth;
  DWORD height=frame->lpBMIH->biHeight;
  DWORD dwCompression = frame ->lpBMIH->biCompression ;
  int max = 0 ;
  CRect rc = ( Area )? *Area : CRect( 0 , 0 , width , height ) ;
  if ( dwCompression == BI_Y8  ||  dwCompression == BI_Y800 
    || dwCompression == BI_YUV9 )
  {
    LPBYTE pData=GetData(frame);

    for (int y= rc.top ; y <= rc.bottom ; y++)
    {
      LPBYTE p = pData + (width*y) + rc.left ;
      LPBYTE pEnd = p + rc.Width() + 1 ;
      do 
      {
        int iVal = *(p++) ;
        if ( max < iVal )
          max = iVal ;
      } while ( p <= pEnd );
    }
  }
  else if ( dwCompression == BI_Y16 )
  {
    LPWORD pData= (LPWORD) GetData(frame);

    for (int y= rc.top ; y <= rc.bottom ; y++)
    {
      LPWORD p = pData + (width*y) ;
      LPWORD pEnd = p + rc.Width() + 1 ;
      do 
      {
        int iVal = *(p++) ;
        if ( max < iVal )
          max = iVal ;
      } while ( p <= pEnd );
    }
  }
  else
  {
    ASSERT(0); // compression is not supported
    return 0 ;
  }
  return max ;
}

__forceinline int _find_min_max( const pTVFrame frame , 
  int& min , int& max  ,CRect& rc )
{
  DWORD width = frame->lpBMIH->biWidth;
  DWORD height=frame->lpBMIH->biHeight;
  DWORD dwCompression = frame ->lpBMIH->biCompression ;
  min = 10000000 ;
  max = 0 ;
  if ( dwCompression == BI_Y8  ||  dwCompression == BI_Y800 
    || dwCompression == BI_YUV9 )
  {
    LPBYTE pData=GetData(frame);

    for (int y= rc.top ; y <= rc.bottom ; y++)
    {
      LPBYTE p = pData + (width*y) + rc.left ;
      LPBYTE pEnd = p + rc.Width() + 1 ;
      do 
      {
        int val = (int) *(p++) ;
        if ( min > val )
          min = val ;
        if ( max < val )
          max = val ;
      } while ( p <= pEnd );
    }
  }
  else if ( dwCompression == BI_Y16 )
  {
    LPWORD pData= (LPWORD) GetData(frame);

    for (int y= rc.top ; y <= rc.bottom ; y++)
    {
      LPWORD p = pData + (width*y) + rc.left ;
      LPWORD pEnd = p + rc.Width() + 1 ;
      do 
      {
        int val = (int) *(p++) ;
        if ( min > val )
          min = val ;
        if ( max < val )
          max = val ;
      } while ( p <= pEnd );
    }
  }
  else
  {
    ASSERT(0); // compression is not supported
    return 0 ;
  }
  return (min+max)/2 ;
}

__forceinline int _find_min_max(const pTVFrame frame ,
  int& min , int& max )
{
  DWORD width = frame->lpBMIH->biWidth;
  DWORD height = frame->lpBMIH->biHeight;
  DWORD size = width * height;
  min = 100000 ;
  max = 0 ;
  if ( frame->lpBMIH->biCompression != BI_Y16 )
  {
    LPBYTE Data = GetData(frame);
    LPBYTE pEnd = Data + size ;

    while ( Data < pEnd )
    {
      int val = (int) *(Data++) ;
      if ( min > val )
        min = val ;
      if ( max < val )
        max = val ;
    }
  }
  else
  {
    LPWORD Data = (LPWORD) GetData(frame);
    LPWORD pEnd = Data + size ;

    while ( Data < pEnd )
    {
      int val = (int) *(Data++) ;
      if ( min > val )
        min = val ;
      if ( max < val )
        max = val ;
    }
  }
  return ( min + max)/2 ;
}

__forceinline double _find_min_max_and_profiles( 
   const pTVFrame frame ,
   pProfile pHProf , pProfile pVProf ,
   int& min , int& max )
{
  DWORD width = frame->lpBMIH->biWidth;
  DWORD height=frame->lpBMIH->biHeight;
  if ( width < 3  ||  height < 3 )
    return 0. ;
  pHProf->Realloc( width );
  pVProf->Realloc( height );
  min = 100000 ;
  max = 0 ;
  double avgV = 0 ;
  double * pV = pVProf->m_pProfData ;
  DWORD dwCompression = frame ->lpBMIH->biCompression ;
  if ( dwCompression == BI_Y8  ||  dwCompression == BI_Y800 
    || dwCompression == BI_YUV9 )
  {
    LPBYTE pData=GetData(frame);

    for (DWORD y=0; y < height ; y++)
    {
      LPBYTE p = pData + (width*y) ;
      double * pH = pHProf->m_pProfData ;
      *pV = 0. ;
      for ( DWORD x = 0 ; x < width ; x++ )
      {
        int val = (int) *(p++) ;
        if ( min > val )
          min = val ;
        if ( max < val )
          max = val ;

        (*pV) += val;
        (*(pH++)) += val;
      }
      (*pV) /= width ;
      if ( pVProf->m_dMinValue > *pV )
        pVProf->m_dMinValue = *pV ;
      if ( pVProf->m_dMaxValue < *pV )
        pVProf->m_dMaxValue = *pV ;
      avgV += *(pV++) ;
    }
    double * pH = pHProf->m_pProfData ;
    for ( DWORD x = 0 ; x < width ; x++ )
    {

      double dVal = (*(pH++) /= height) ;
      if ( pHProf->m_dMinValue > dVal )
        pHProf->m_dMinValue = dVal ;
      if ( pHProf->m_dMaxValue < dVal )
        pHProf->m_dMaxValue = dVal ;
    }
  }
  else if ( dwCompression == BI_Y16 )
  {
    LPWORD pData= (LPWORD) GetData(frame);

    for (DWORD y=0; y < height ; y++)
    {
      LPWORD p = pData + (width*y) ;
      double * pH = pHProf->m_pProfData ;
      *pV = 0. ;
      for ( DWORD x = 0 ; x < width ; x++ )
      {
        int val = (int) *(p++) ;
        if ( min > val )
          min = val ;
        if ( max < val )
          max = val ;

        (*pV) += val;
        (*(pH++)) += val;
      }
      (*pV) /= width ;
      if ( pVProf->m_dMinValue > *pV )
        pVProf->m_dMinValue = *pV ;
      if ( pVProf->m_dMaxValue < *pV )
        pVProf->m_dMaxValue = *pV ;
      avgV += *(pV++) ;
    }
    double * pH = pHProf->m_pProfData ;
    for ( DWORD x = 0 ; x < width ; x++ )
    {
      double dVal = (*(pH++) /= height) ;
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



__forceinline void _calc_diff_image(pTVFrame frame)
{
  LPBYTE Data = GetData(frame);
  DWORD width = frame->lpBMIH->biWidth;
  DWORD height = frame->lpBMIH->biHeight;
  DWORD size = width * height;

  LPBYTE Copy = (LPBYTE)malloc(size);

  for (DWORD y = 1; y < height - 1; y++)
  {
    int index = y * width + 1 ;
    BYTE * p = Data + index ;
    for (DWORD x = 1; x < width - 1; x++ , p++ )
    {
      int val = ( (int) *(p+1) - (int) *(p-1) + (int) *(p+width) - (int) *(p-width) );
      Copy[index++] = (BYTE)((val + 510) / 4);
    }
  }

  memcpy(Data, Copy, size);
  free(Copy);
}

__forceinline pTVFrame _calc_diff_image(const pTVFrame frame)
{
  LPBYTE Data = GetData(frame);
  DWORD width = frame->lpBMIH->biWidth;
  DWORD height = frame->lpBMIH->biHeight;
  DWORD size = width * height;

  pTVFrame pResult = makeTVFrame( frame->lpBMIH ) ;
  LPBYTE Copy = GetData( pResult ) ;

  for (DWORD y = 1; y < height - 1; y++)
  {
    int index = y * width + 1 ;
    BYTE * p = Data + index ;
    for (DWORD x = 1; x < width - 1; x++ , p++ )
    {
      int val = ( (int) *(p+1) - (int) *(p-1) + (int) *(p+width) - (int) *(p-width) );
      Copy[index++] = (BYTE)((val + 510) / 4);
    }
  }
  return pResult ;
}


__forceinline double calc_profiles( const pTVFrame frame ,
   double * hdistr , double * vdistr ,
   double& dHMin , double& dHMax , double& dVMin , double& dVMax ,
   CRect * pROI = NULL )
{
  DWORD width = frame->lpBMIH->biWidth;
  DWORD height=frame->lpBMIH->biHeight;
  if ( width < 3  ||  height < 3 )
    return 0. ;

  int iXLeft = (pROI)? pROI->left : 0 ;
  int iYtop = (pROI)? pROI->top : 0 ;
  int iXRight = (pROI)? pROI->left + pROI->right : width ;
  int iYBottom = (pROI)? pROI->top + pROI->bottom : height ;

  dVMin = dHMin = 10e30 ;
  dVMax = dHMax = -10e30 ;
  memset( hdistr , 0 , width * sizeof(double) ) ;
  double avgV = 0 ;
  double * pV = vdistr + iYtop ;
  DWORD dwCompression = frame ->lpBMIH->biCompression ;
  if ( dwCompression == BI_Y8  ||  dwCompression == BI_Y800 
    || dwCompression == BI_YUV9 )
  {
    LPBYTE pData=GetData(frame);

//     for (DWORD y=1; y < height-1 ; y++)
    for (int y=iYtop; y < iYBottom ; y++)
    {
      LPBYTE p = pData + (width*y) /*+ 1*/;
      double * pH = hdistr + iXLeft ;
      *pV = 0. ;
//       for ( DWORD x = 1 ; x < width-1 ; x++ )
      for ( int x = iXLeft ; x < iXRight ; x++ )
      {
        double v = *(p++) ;
        (*pV) += v;
        (*(pH++)) += v;
      }
//       (*pV) /= (width - 2) ;
      (*pV) /= iXRight - iXLeft ;
      if ( *pV < dVMin )
        dVMin = *pV ;
      if ( *pV > dVMax )
        dVMax = *pV ;
      avgV += *(pV++) ;
    }
  }
  else if ( dwCompression == BI_Y16 )
  {
    LPWORD pData= (LPWORD) GetData(frame);

//     for (DWORD y=1; y < height-1 ; y++)
    for (int y=iYtop; y < iYBottom ; y++)
    {
      LPWORD p = pData + (width*y) ;
      double * pH = hdistr ;
      *pV = 0. ;
//       for ( DWORD x = 1 ; x < width-1 ; x++ )
      for ( int x = iXLeft ; x < iXRight ; x++ )
      {
        double v = *(p++) ;
        (*pV) += v;
        (*(pH++)) += v;
      }
//       (*pV) /= (width - 2) ;
      (*pV) /= iXRight - iXLeft ;
      if ( *pV < dVMin )
        dVMin = *pV ;
      if ( *pV > dVMax )
        dVMax = *pV ;
      avgV += *(pV++) ;
    }
  }
  else
    return 0. ;

  //   for ( DWORD x=1 ; x < width-1 ; x++ )
  for ( DWORD x=0 ; x < width ; x++ )
  {
    hdistr[x] /= iYBottom - iYtop ;
    if ( hdistr[x] < dHMin )
      dHMin = hdistr[x] ;
    if ( hdistr[x] > dHMax )
      dHMax = hdistr[x] ;
  }
//   return avgV / (height - 2)  ;
  return avgV / (iYBottom - iYtop) ;
}

__forceinline double calc_profiles( const pTVFrame frame ,
  pProfile hProf , pProfile vProf , CRect * pROI = NULL )
{
  hProf->Realloc( frame->lpBMIH->biWidth );
  vProf->Realloc( frame->lpBMIH->biHeight );
  return calc_profiles( frame , hProf->m_pProfData , vProf->m_pProfData ,
    hProf->m_dMinValue , hProf->m_dMaxValue , 
    vProf->m_dMinValue , vProf->m_dMaxValue , pROI ) ;
}

__forceinline bool calc_diff_profiles( 
  const pTVFrame frame , int iDist , double * hdistr , double * vdistr ,
  double& dHMin , double& dHMax , double& dVMin , double& dVMax ,
  const CRect * pROI = NULL )
{
  bool bAbsValue = ( iDist < 0 ) ;
  iDist = abs( iDist ) ;
  int width = frame->lpBMIH->biWidth;
  int height=frame->lpBMIH->biHeight;
  if ( width < iDist + 3  ||  height < iDist + 3 )
    return false ;

  int iXLeft = (pROI)? pROI->left : 0 ;
  int iYtop = (pROI)? pROI->top : 0 ;
  int iXRight = (pROI)? pROI->left + pROI->right : width ;
  int iYBottom = (pROI)? pROI->top + pROI->bottom : height ;

  dVMin = dHMin = 10e30 ;
  dVMax = dHMax = -10e30 ;
  memset( hdistr , 0 , width * sizeof(double) ) ;
  double avgV = 0 ;
  double * pV = vdistr + iYtop + iDist ;
  DWORD dwCompression = frame ->lpBMIH->biCompression ;
  int iVertDist = iDist * width ;
  if ( dwCompression == BI_Y8  ||  dwCompression == BI_Y800 
    || dwCompression == BI_YUV9 )
  {
    LPBYTE pData=GetData(frame);

    for (int y=iYtop ; y < iYBottom ; y++)
    {
      LPBYTE p = pData + (width*y) + iXLeft + iDist ;
      double * pH = hdistr + iXLeft + iDist ;
      bool bYCalc = ( y >= (iYtop + iDist) ) ;
      *pV = 0. ;
      for ( int x = iXLeft + iDist ; x < iXRight ; x++ , p++ )
      {
        
        double v = *p - *(p-iDist) ;
        (*(pH++)) += ( bAbsValue ) ? fabs(v) : v ;
        if ( bYCalc )
        {
        v = *p - *(p - iVertDist) ;
        (*pV) += ( bAbsValue ) ? fabs(v) : v ;
      }
      }
      pH = hdistr + iXLeft ;
      double dFirstHVal = *(hdistr + iXLeft + iDist) ;
      for ( int x = 0 ; x < iDist ; x++ )
       *(pH++) = dFirstHVal ;
      
      (*pV) /= iXRight - iXLeft - iDist ;
      if ( *pV < dVMin )
        dVMin = *pV ;
      if ( *pV > dVMax )
        dVMax = *pV ;
      avgV += *(pV++) ;
    }
  }
  else if ( dwCompression == BI_Y16 )
  {
    LPWORD pData= (LPWORD) GetData(frame);

    for (int y=iYtop + iDist; y < iYBottom ; y++)
    {
      LPWORD p = pData + (width*y) + iXLeft + iDist ;
      double * pH = hdistr + iXLeft + iDist ;
      bool bYCalc = ( y >= (iYtop + iDist) ) ;
      *pV = 0. ;
      for ( int x = iXLeft + iDist ; x < iXRight ; x++ , p++ )
      {

        double v = *p - *(p-iDist) ;
        (*(pH++)) += ( bAbsValue ) ? fabs(v) : v ;
        if ( bYCalc )
        {
        v = *p - *(p - iVertDist) ;
        (*pV) += ( bAbsValue ) ? fabs(v) : v ;
      }
      }
      pH = hdistr + iXLeft ;
      double dFirstHVal = *(hdistr + iXLeft + iDist) ;
      for ( int x = 0 ; x < iDist ; x++ )
        *(pH++) = dFirstHVal ;

      (*pV) /= iXRight - iXLeft - iDist ;
      if ( *pV < dVMin )
        dVMin = *pV ;
      if ( *pV > dVMax )
        dVMax = *pV ;
      avgV += *(pV++) ;
    }
  }
  else
    return false ;

  for ( int x=0 ; x < width ; x++ )
  {
    hdistr[x] /= iYBottom - iYtop ;
    if ( hdistr[x] < dHMin )
      dHMin = hdistr[x] ;
    if ( hdistr[x] > dHMax )
      dHMax = hdistr[x] ;
  }
  return true ;
}


__forceinline bool calc_diff_profiles( const pTVFrame frame , int iDist ,
  pProfile hProf , pProfile vProf , const CRect * pROI = NULL )
{
  hProf->Realloc( frame->lpBMIH->biWidth );
  vProf->Realloc( frame->lpBMIH->biHeight );
  return calc_diff_profiles( frame , iDist ,
    hProf->m_pProfData , vProf->m_pProfData ,
    hProf->m_dMinValue , hProf->m_dMaxValue , 
    vProf->m_dMinValue , vProf->m_dMaxValue , pROI ) ;
}


__forceinline bool seekSpot(pTVFrame frame, DRECT& rc, 
  double * pHProf ,
  double * pVProf ,   
  DWORD White=0, //0 - Black-to-White, otherwise - white-to-black
  double dMinAmpl = MIN_AMPL ,
  double dNormThres = DEFAULT_NORM_THRES ) 
{
    unsigned x,y;

    ASSERT(frame!=NULL); 
    ASSERT(frame->lpBMIH!=NULL); 

    DWORD width=frame->lpBMIH->biWidth;
    DWORD height=frame->lpBMIH->biHeight;

    pProfile pH = (pProfile)pHProf ;
    pProfile pV = (pProfile)pVProf ;
    double dMean = calc_profiles( frame , pH , pV ) ;
    if ( dMean == 0. )
      return false ;

    double* hdistr = pH->m_pProfData ;
    double* vdistr = pV->m_pProfData ; 
    double dHMin = pH->m_dMinValue , dHMax = pH->m_dMaxValue ;
    double dVMin = pV->m_dMinValue , dVMax = pV->m_dMaxValue ;
    double avgV = dMean ;
    double hl=avgV;
    double vl=avgV;
//    TRACE(" Levels hl %d, vl %d\n", hl*width, vl*height);
    memset(&rc,0,sizeof(RECT));
    if ( White )
    {
        double min=0, max=-1;
        for (x=0; x < width ; x++)
        {
            if ((max<0) && (hdistr[x]>=hl))
            {
                min=x;
                max=x;
            }
            else if ((max>=0) && (hdistr[x]<=hl))
            {
                max = x ;
                if ((max-min)>(rc.right-rc.left))
                {
                    rc.left=min;
                    rc.right=max;
                }
                max=-1;
            }
        }
        min=0; max=-1;
        for ( y = 0 ; y<height ; y++)
        {
            if ((max<0) && (vdistr[y]>=hl))
            {
                min=y;
                max=y;
            }
            else if ((max>=0) && (vdistr[y]<=hl))
            {
                max = y ;
                if ((max-min)>(rc.bottom-rc.top))
                {
                    rc.top=min;
                    rc.bottom=max;
                }
                max=-1;
            }
        }
    }
    else
    {
        double min=0, max=-1;
        for ( x = 0; x < width ; x++)
        {
            //TRACE("%d ",hdistr[x]);
            if ((max<0) && (hdistr[x]<=hl))
            {
                min=x;
                max=x;
            }
            else if ((max>=0) && (hdistr[x]>=hl))
            {
                max = x ;
                if ((max-min)>(rc.right-rc.left))
                {
                    rc.left=min;
                    rc.right=max;
                }
                max=-1;
            }
        }
        //TRACE("\n");
        min=0; max=-1;
        for ( y = 0 ; y < height ; y++)
        {
            //TRACE("%d ",vdistr[y]);
            if ((max<0) && (vdistr[y]<=hl))
            {
                min=y;
                max=y;
            }
            else if ((max>=0) && (vdistr[y]>=hl))
            {
                max = y ;
                if ((max-min)>(rc.bottom-rc.top))
                {
                    rc.top=min;
                    rc.bottom=max;
                }
                max=-1;
            }
        }
        //TRACE("\n");
    }
    
    bool res=   (  ( (rc.bottom-rc.top) > 1 ) 
                && ( (rc.right-rc.left) > 1 ) 
                && ( rc.left > 0 ) 
                && ( rc.top > 0 ) 
                && ( rc.right < width-1 ) 
                && ( rc.bottom < height-1) );

    return res;
}

using namespace std;

__forceinline bool seekSpotAlt(const pTVFrame frame, DRECT& rc, 
  double * pHProf ,
  double * pVProf ,   
  DWORD White=0, //0 - Black-to-White, otherwise - white-to-black
  DWORD dir=3,   // 0x01 - vertical, 0x02 - horizontal 
  double dMinAmpl = MIN_AMPL ,
  double dNormThres = DEFAULT_NORM_THRES,
  bool straighten = true ) 
{
  unsigned x,y;

  ASSERT(frame!=NULL); 
  ASSERT(frame->lpBMIH!=NULL); 

  LPBYTE Data=GetData(frame);
  DWORD width=frame->lpBMIH->biWidth;
  DWORD height=frame->lpBMIH->biHeight;

  pProfile pH = (pProfile)pHProf ;
  pProfile pV = (pProfile)pVProf ;
  double dMean = calc_profiles( frame , pH , pV ) ;
  double* hdistr = pH->m_pProfData ;
  double* vdistr = pV->m_pProfData ; 
  if ( dMean == 0. )
    return false ;

  DWORD avgV = (DWORD)(dMean + 0.5) ;
  double hl=avgV;
  double vl=avgV;

  //"straighten" the distr array using linear interpolation
  if (straighten)
  {
    if (width>1)
    {
      double a = hdistr[width-1]-hdistr[1];
      for (x=0; x<width; x++)
      {
        hdistr[x]-=(a*(x))/(width-2);
      }
    }
    if (height>1)
    {
      double a=vdistr[height-1]-vdistr[1];
      for (y=0; y<height; y++)
      {
        vdistr[y]-=(a*(y))/(height-2);
      }
    }
  }


  //calculate dispersion
  double dispX=0, dispY=0;
  for (x=1; x<width-1; x++)
    dispX+=(hdistr[x]-hl)*(hdistr[x]-hl);

  dispX/=(width-2);

  for (y=1; y<height-1; y++)
  {
    dispY+=(vdistr[y]-vl)*(vdistr[y]-vl);
  }
  dispY/=(height-2);

  if ( ((dir==3 || dir==2) && dispX<dMinAmpl) || ((dir==3 || dir==1) && dispY<dMinAmpl) )
  {
    rc.left=numeric_limits<double>::quiet_NaN();
    rc.right=numeric_limits<double>::quiet_NaN();
    rc.bottom=numeric_limits<double>::quiet_NaN();
    rc.top=numeric_limits<double>::quiet_NaN();
    return true;
  }


  //    TRACE(" Levels hl %d, vl %d\n", hl*width, vl*height);
  memset(&rc,0,sizeof(DRECT));
  bool onSpot=false;
  if (White)
  {
    double min, max;

    if (dir==3 || dir==2) //any or vertical
    {
      min=0; max=-1;
      for (x=0; x<width; x++)
      {
        if ((max<0) && (hdistr[x]>=hl))
        {
          if (x==0)
          {
            min=x;
            //max=x;
          }
          else
          {
            if (hl-hdistr[x-1]==hdistr[x]-hl)
              min=x-0.5;
            else
              min=x-((double)hdistr[x]-hl)/(hdistr[x]-hdistr[x-1]);
            //max=min;
          }
          max=width;
        }
        else if ((max>=0) && (hdistr[x]<=hl))
        {
          if (hdistr[x-1]-hl==hl-hdistr[x])
            max=x-0.5;
          else
            max=x+((double)hdistr[x-1]-hl)/(hdistr[x-1]-hdistr[x]);

          if ((max-min)>(rc.right-rc.left))
          {
            rc.left=min;
            rc.right=max;
          }
          max=-1;
        }
      }
      if ((max-min)>(rc.right-rc.left))
      {
        rc.left=min;
        rc.right=max;
      }
    }
    else if (dir==1) //horizontal
    {
      rc.left=0;
      rc.right=width;
    }
    else
    {
      rc.left=numeric_limits<double>::quiet_NaN();
      rc.right=numeric_limits<double>::quiet_NaN();
    }

    if (dir==3 || dir==1) //any or vertical
    {
      min=0; max=-1;
      for (y=0; y<height; y++)
      {
        if ((max<0) && (vdistr[y]>=vl))
        {
          if (y==0)
          {        
            min=y;
            //max=y;
          }
          else
          {
            if (vl-vdistr[y-1]==vdistr[y]-vl)
              min=y-0.5;
            else
              min=y-((double)vdistr[y]-vl)/(vdistr[y]-vdistr[y-1]);
            //max=min;
          }
          max=height;
        }
        else if ((max>=0) && (vdistr[y]<=vl))
        {
          if (vdistr[y-1]-vl==vl-vdistr[y])
            max=y-0.5;
          else
            max=y+((double)vdistr[y-1]-vl)/(vdistr[y-1]-vdistr[y]);

          if ((max-min)>(rc.bottom-rc.top))
          {
            rc.top=min;
            rc.bottom=max;
          }
          max=-1;
        }
      }
      if ((max-min)>(rc.bottom-rc.top))
      {
        rc.top=min;
        rc.bottom=max;
      }
    }
    else if (dir==2) //horizontal
    {
      rc.top=0;
      rc.bottom=height;
    }
    else
    {
      rc.top=numeric_limits<double>::quiet_NaN();
      rc.bottom=numeric_limits<double>::quiet_NaN();
    }
  }
  else
  {
    double min, max;

    if (dir==3 || dir==2)
    {
      min=0; max=-1;
      for (x=0; x<width; x++)
      {
        //TRACE("%d ",hdistr[x]);
        if ((max<0) && (hdistr[x]<=hl))
        {
          if (x==0)
          {
            min=x;
            //max=x;
          }
          else
          {
            if (hl-hdistr[x]==hdistr[x-1]-hl)
              min=x-0.5;
            else
              min=x-((double)hl-hdistr[x])/(hdistr[x-1]-hdistr[x]);
            //max=min;
          }
          max=width;
        }
        else if ((max>=0) && (hdistr[x]>=hl))
        {
          if (hdistr[x]-hl==hl-hdistr[x-1])
            max=x-0.5;
          else
            max=x+((double)hl-hdistr[x-1])/(hdistr[x]-hdistr[x-1]);

          if ((max-min)>(rc.right-rc.left))
          {
            rc.left=min;
            rc.right=max;
          }
          max=-1;
        }
      }

      if ((max-min)>(rc.right-rc.left))
      {
        rc.left=min;
        rc.right=max;
      }
    }
    else if (dir==1)
    {
      rc.left=0;
      rc.right=width;
    }
    else
    {
      rc.left=numeric_limits<double>::quiet_NaN();
      rc.right=numeric_limits<double>::quiet_NaN();
    }

    if (dir==3 || dir==1)
    {
      //TRACE("\n");
      min=0; max=-1;
      for (y=0; y<height; y++)
      {
        //TRACE("%d ",vdistr[y]);
        if ((max<0) && (vdistr[y]<=vl*width))
        {
          if (y==0)
          {
            min=y;
            //max=y;
          }
          else
          {
            if (vl-vdistr[y]==vdistr[y-1]-vl)
              min=y-0.5;
            else
              min=y-((double)vl-vdistr[y])/(vdistr[y-1]-vdistr[y]);
            //max=min;
          }
          max=height;
        }
        else if ((max>=0) && (vdistr[y]>=vl))
        {
          if (vdistr[y]-vl==vl-vdistr[y-1])
            max=y-0.5;
          else
            max=y+((double)vl-vdistr[y-1])/(vdistr[y]-vdistr[y-1]);

          if ((max-min)>(rc.bottom-rc.top))
          {
            rc.top=min;
            rc.bottom=max;
          }
          max=-1;
        }
      }
      if ((max-min)>(rc.bottom-rc.top))
      {
        rc.top=min;
        rc.bottom=max;
      }
      //TRACE("\n");
    }
    else if (dir==2)
    {
      rc.top=0;
      rc.bottom=height;
    }
    else
    {
      rc.top=numeric_limits<double>::quiet_NaN();
      rc.bottom=numeric_limits<double>::quiet_NaN();
    }
  }

  bool res=(((rc.bottom-rc.top)>1) && ((rc.right-rc.left)>1)); 

  if (dir==1) //horizontal
  {
    if (rc.top==0 || rc.bottom==height)
      res=false;
  }
  else if (dir==2) //vertical
  {
    if (rc.left==0 || rc.right==width)
      res=false;
  }

  if (!res)
  {
    rc.left=numeric_limits<double>::quiet_NaN();
    rc.right=numeric_limits<double>::quiet_NaN();
    rc.bottom=numeric_limits<double>::quiet_NaN();
    rc.top=numeric_limits<double>::quiet_NaN();
    res=true;
  }
  return res;
}           



__forceinline double find_border_forw( double * pProf ,
  int iLength , double dThres , int iFilter = 0 )
{
  double *pD = pProf + 1 ;
  double *pEnd = pD + iLength - 1 ;

  double dl = *pD ;
  double dc = *(++pD) ;

  // Left border searching
  if ( dl >= dThres ) // find black border on white background
  {
    do 
    {
      if (dc < dThres )
        return (pD - pProf /*- 1*/)  + (dl - dThres)/(dl - dc) ;
      dl = dc ;
      dc = *(++pD) ;
    } while ( ( pD < pEnd - 1 ) ) ;
  }
  else                // find white border on black background
  {
    do 
    {
      if (dc > dThres )
        return (pD - pProf /*- 1*/)  + (dThres - dl)/(dc - dl) ;
      dl = dc ;
      dc = *(++pD) ;
    } while ( ( pD < pEnd - 1 ) ) ;
  }
  return 0. ;
}

__forceinline double find_border_back( double * pProf ,
  int iLength , double dThres , int iFilter = 0 )
{
  double *pD = pProf + iLength - 3 ;
  double *pEnd = pProf + 1 ;

  double dr = *pD ;
  double dc = *(--pD) ;

  // Left border searching
  if ( dr >= dThres ) // find black border on white background
  {
    do 
    {
      if (dc < dThres )
        return (pD - pProf)  + (dThres - dc)/(dr - dc) ;
      dr = dc ;
      dc = *(--pD) ;
    } while ( ( pD > pEnd ) ) ;
  }
  else                // find white border on black background
  {
    do 
    {
      if (dc > dThres )
        return (pD - pProf )  + (dc - dThres)/(dc - dr) ;
      dr = dc ;
      dc = *(--pD) ;
    } while ( ( pD > pEnd ) ) ;
  }
  return 0. ;
}

__forceinline bool seekBorders(DRECT& ResultRC, 
                               Profile& HProf ,
                               Profile& VProf ,   
                               DWORD White=0, //0 - Black-to-White, otherwise - white-to-black
                               DWORD dir=3,   // 0x01 - vertical, 0x02 - horizontal 
                               double dMinAmpl = MIN_AMPL ,
                               double dNormThres = DEFAULT_NORM_THRES )
{

  if ( ((dir & 1) && ( (HProf.m_dMaxValue - HProf.m_dMinValue) < dMinAmpl ) /*(dispX < 5)*/ /*DISPERTION_TRIGGER*/) 
    || ((dir & 2) && ( (VProf.m_dMaxValue - VProf.m_dMinValue) < dMinAmpl
    ) /*(dispY < 5)*/ /*DISPERTION_TRIGGER*/) )
  {
    return false;
  }
  double dHThres = HProf.m_dMinValue + (dNormThres * ( HProf.m_dMaxValue - HProf.m_dMinValue )) ;
  double dVThres = VProf.m_dMinValue + (dNormThres * (VProf.m_dMaxValue - VProf.m_dMinValue)) ;


  //    TRACE(" Levels hl %d, vl %d\n", hl*width, vl*height);
  memset(&ResultRC,0,sizeof(DRECT));
  bool onSpot=false;
  if ( dir & 0x01 ) // vertical borders           
  {
    ResultRC.left = find_border_forw( HProf.m_pProfData , HProf.m_iProfLen , dHThres ) ;
    ResultRC.right = find_border_back( HProf.m_pProfData , HProf.m_iProfLen , dHThres ) ;
  }
  if ( dir & 0x02 ) // Horizontal borders
  {
    ResultRC.top = find_border_forw( VProf.m_pProfData , VProf.m_iProfLen , dVThres ) ;
    ResultRC.bottom = find_border_back( VProf.m_pProfData , VProf.m_iProfLen , dVThres ) ;
  }
  return true ;
}           


__forceinline bool seekBorders(const pTVFrame frame, DRECT& rc, 
   pProfile pH ,
   pProfile pV ,   
   DWORD White=0, //0 - Black-to-White, otherwise - white-to-black
   DWORD dir=3,   // 0x01 - vertical, 0x02 - horizontal 
   double dMinAmpl = MIN_AMPL ,
   double dNormThres = DEFAULT_NORM_THRES )
{

  ASSERT(frame!=NULL); 
  ASSERT(frame->lpBMIH!=NULL); 

  if ( !frame  ||  !frame->lpBMIH )
    return false ;

  DWORD width=frame->lpBMIH->biWidth;
  DWORD height=frame->lpBMIH->biHeight;

  if ( dNormThres >= 1.0 )
    _calc_diff_image( frame ) ;

  double dMean = calc_profiles( frame , pH , pV ) ;
  if ( dMean == 0. )
    return false ;
  if ( ((dir & 1) && ( (pH->m_dMaxValue - pH->m_dMinValue) < dMinAmpl ) /*(dispX < 5)*/ /*DISPERTION_TRIGGER*/) 
    || ((dir & 2) && ( (pV->m_dMaxValue - pV->m_dMinValue) < dMinAmpl
    ) /*(dispY < 5)*/ /*DISPERTION_TRIGGER*/) )
  {
    return false;
  }

  double dThresAdd = dMean + ((White == 0) ? dNormThres : -dNormThres) ;

  double dHThres = (dNormThres >= 1.0) ?  
    dThresAdd : pH->m_dMinValue + (dNormThres * ( pH->m_dMaxValue - pH->m_dMinValue )) ;
  double dVThres = ((dNormThres >= 1.0) ? 
    dThresAdd : pV->m_dMinValue + (dNormThres * (pV->m_dMaxValue - pV->m_dMinValue))) ;


  //    TRACE(" Levels hl %d, vl %d\n", hl*width, vl*height);
  memset(&rc,0,sizeof(DRECT));
  bool onSpot=false;
  if ( dir & 0x01 ) // vertical borders           
  {
    rc.left = find_border_forw( pH->m_pProfData , width , dHThres ) ;
    rc.right = find_border_back( pH->m_pProfData , width , dHThres ) ;
  }
  if ( dir & 0x02 ) // Horizontal borders
  {
    rc.top = find_border_forw( pV->m_pProfData , height , dVThres ) ;
    rc.bottom = find_border_back( pV->m_pProfData , height , dVThres ) ;
  }
  return true ;
}           
__forceinline bool seekBorders(const pTVFrame frame, DRECT& rc, 
                               pProfile pHProf ,
                               pProfile pVProf ,   
                               double dNormThres = DEFAULT_NORM_THRES )
{
  return seekBorders( frame, rc, pHProf , pVProf , 0 , 3 , 
    MIN_AMPL , dNormThres ) ;

}


__forceinline bool seekBorders(const pTVFrame frame, DRECT& rc, 
                               Profile& HProf ,
                               Profile& VProf ,   
                               double dNormThres = DEFAULT_NORM_THRES )
{
  return seekBorders( frame, rc,  &HProf , &VProf , 0 , 3 , 
    MIN_AMPL , dNormThres ) ;

}
__forceinline bool seekMultiBorders(const pTVFrame frame, CDRectArray& Results, 
                               double * pHProf ,
                               double * pVProf ,   
                               DWORD White=0, //=0 - Black-to-White, =1 - white-to-black, =2 - any
                               DWORD dir=3,   // 0x01 - vertical, 0x02 - horizontal 
                               double dMinAmpl = MIN_AMPL ,
                               double dNormThres = DEFAULT_NORM_THRES )
{

  ASSERT(frame!=NULL); 
  ASSERT(frame->lpBMIH!=NULL); 

  if ( !frame  ||  !frame->lpBMIH )
    return false ;

  int width=frame->lpBMIH->biWidth;
  int height=frame->lpBMIH->biHeight;

  if ( dNormThres >= 1.0 )
    _calc_diff_image( frame ) ;

  pProfile pH = (pProfile)pHProf ;
  pProfile pV = (pProfile)pVProf ;
  double dMean = calc_profiles( frame , pH , pV ) ;
  if ( dMean == 0. )
    return false ;
  if ( ((dir & 1) && ( (pH->m_dMaxValue - pH->m_dMinValue) < dMinAmpl ) /*(dispX < 5)*/ /*DISPERTION_TRIGGER*/) 
    || ((dir & 2) && ( (pV->m_dMaxValue - pV->m_dMinValue) < dMinAmpl
    ) /*(dispY < 5)*/ /*DISPERTION_TRIGGER*/) )
  {
    return false;
  }

  double dThresAdd = dMean + ((White == 0) ? dNormThres : -dNormThres) ;

  double dHThres = (dNormThres >= 1.0) ?  
    dThresAdd : pH->m_dMinValue + (dNormThres * ( pH->m_dMaxValue - pH->m_dMinValue )) ;
  double dVThres = ((dNormThres >= 1.0) ? 
    dThresAdd : pV->m_dMinValue + (dNormThres * (pV->m_dMaxValue - pV->m_dMinValue))) ;


  //    TRACE(" Levels hl %d, vl %d\n", hl*width, vl*height);

  int iPos = 0 ;
  Results.RemoveAll() ;
  if ( dir & 0x01 ) // vertical borders           
  {
    if ( White == 0 )
    {  // find first black border, begin measurement after that
      while ( pH->m_pProfData[iPos++] >= dHThres )
      {
        if ( iPos > width - 2 )
          break ;
      }
    }
    else if ( White == 1 )
    {
      while ( pH->m_pProfData[iPos++] < dHThres )
      {
        if ( iPos > width - 2 )
          break ;
      }
    }
    while ( iPos < width - 2 ) 
    {
      CDRect rc ;
      rc.left = find_border_forw( &pH->m_pProfData[iPos] , width - iPos , dHThres ) ;
      if ( rc.left == 0. || ((rc.left += iPos) >= (width-2))  ) // border not found 
        break ;
      iPos = (int) ceil( rc.left ) ;
      rc.right = find_border_forw( &pH->m_pProfData[iPos] , width - iPos , dHThres ) ;
      if ( rc.right == 0. )
        break ; // second border is not found
      rc.right += iPos ;
      iPos = (int) ceil( rc.right ) ;
      Results.Add( rc ) ;
    }  ;
  }
  if ( dir & 0x02 ) // Horizontal borders
  {
    iPos = 0 ;
    if ( White == 0 )
    {  // find first black border, begin measurement after that
      while ( pV->m_pProfData[iPos++] >= dHThres )
      {
        if ( iPos > height - 2 )
          break ;
      }
    }
    else if ( White == 1 )
    {
      while ( pV->m_pProfData[iPos++] < dHThres )
      {
        if ( iPos > height - 2 )
          break ;
      }
    }
    while ( iPos < height - 2 )
    {
      CDRect rc ;
      rc.top = find_border_forw( &pV->m_pProfData[iPos] , height - iPos , dVThres ) ;
      if ( rc.top == 0.  ||  ((rc.top += iPos) >= (height - 2)) ) // border not found 
        break ;
      iPos = (int) ceil( rc.top ) ;
      rc.bottom = find_border_forw( &pV->m_pProfData[iPos] , height - iPos , dVThres ) ;
      if ( rc.bottom == 0. )
        break ; // second border is not found
      rc.bottom += iPos ;
      iPos = (int) ceil( rc.bottom ) ;
      Results.Add( rc ) ;
    }  ;
  }
  return (Results.GetCount() > 0) ;
}           

__forceinline double _calc_integral(const pTVFrame frame, CRect& rc)
{
  DWORD width = frame->lpBMIH->biWidth;
  DWORD height=frame->lpBMIH->biHeight;
  DWORD dwCompression = frame ->lpBMIH->biCompression ;
  double dRes = 0. ;

  if ( dwCompression == BI_Y8  ||  dwCompression == BI_Y800 
    || dwCompression == BI_YUV9 )
  {
    LPBYTE pData=GetData(frame);

    for (int y= rc.top ; y < rc.bottom ; y++)
    {
      LPBYTE p = pData + (width*y) + rc.left ;
      LPBYTE pEnd = p + rc.Width() + 1 ;
      do 
      {
        dRes += *(p++) ;
      } while ( p <= pEnd );
    }
  }
  else if ( dwCompression == BI_Y16 )
  {
    LPWORD pData= (LPWORD) GetData(frame);

    for (int y= rc.top ; y < rc.bottom ; y++)
    {
      LPWORD p = pData + (width*y) + rc.left ;
      LPWORD pEnd = p + rc.Width() ;
      do 
      {
        dRes += *(p++) ;
      } while ( p <= pEnd );
    }
  }
  else
  {
    ASSERT(0); // compression is not supported
    return 0. ;
  }

  return dRes ;
}
__forceinline double _calc_integral(
  const pTVFrame frame, CPoint& Cent , int iRadius )
{
  CRect Rect( Cent.x - iRadius , Cent.y - iRadius , 
    Cent.x + iRadius , Cent.y + iRadius ) ;
  return _calc_integral( frame , Rect ) ;
}

__forceinline double _calc_sum_over_thres(
  const pTVFrame frame , CRect& rc , int iThres )
{
  DWORD width = frame->lpBMIH->biWidth;
  DWORD height=frame->lpBMIH->biHeight;
  DWORD dwCompression = frame ->lpBMIH->biCompression ;
  double dRes = 0. ;

  if ( dwCompression == BI_Y8  ||  dwCompression == BI_Y800 
    || dwCompression == BI_YUV9 )
  {
    LPBYTE pData=GetData(frame);

    for (int y= rc.top ; y < rc.bottom ; y++)
    {
      LPBYTE p = pData + (width*y) + rc.left ;
      LPBYTE pEnd = p + rc.Width() + 1 ;
      do 
      {
        if ( *p >= iThres )
          dRes += *p ;
      } while ( ++p <= pEnd );
    }
  }
  else if ( dwCompression == BI_Y16 )
  {
    LPWORD pData= (LPWORD) GetData(frame);

    for (int y= rc.top ; y < rc.bottom ; y++)
    {
      LPWORD p = pData + (width*y) ;
      LPWORD pEnd = p + rc.Width() + 1 ;
      do 
      {
        if ( *p >= iThres )
          dRes += *p ;
      } while ( ++p <= pEnd );
    }
  }
  else
  {
    ASSERT(0); // compression is not supported
    return 0. ;
  }

  return dRes ;
}

__forceinline int calc_and_substruct_side_background( 
  pTVFrame frame , int iFromSides , 
  pProfile pHProf = NULL , pProfile pVProf = NULL )
{
  int width = frame->lpBMIH->biWidth;
  int height = frame->lpBMIH->biHeight;
  int iXOrig = 0 ;
  int iYOrig = 0 ;
  if ( pHProf && (pHProf->m_iProfLen < width) )
  {
    iXOrig = ( width - pHProf->m_iProfLen ) / 2 ; 
//     width = pHProf->m_iProfLen ;
  }
  if ( pVProf && (pVProf->m_iProfLen < height) )
  {
    iYOrig = ( height - pVProf->m_iProfLen ) / 2 ;
//     height = pVProf->m_iProfLen ;
  }
  int iMax = 0 ;
  int iSumBack = 0 ;

  DWORD dwCompression = frame ->lpBMIH->biCompression ;
  if ( dwCompression == BI_Y8  ||  dwCompression == BI_Y800 
    || dwCompression == BI_YUV9 )
  {
    LPBYTE pData=GetData(frame);
  
    for (int y=0; y < height ; y++)
    {
      int iBack = 0 ;
      LPBYTE pLeft = pData + (width*y) ;
      LPBYTE pRight = pLeft + width - iFromSides ;
      LPBYTE pRow = pLeft ;
      for ( int x = 0 ; x < iFromSides ; x++ )
        iBack += (int) (*(pLeft++)) + (int)(*(pRight++));
      iBack /= 2 * iFromSides ;
      for ( int x = 0 ; x < width ; x++ )
      {
        int iDiff = (int) (*pRow) - iBack ;
        *(pRow++) = ( iDiff >= 0)? (BYTE)iDiff : 0 ;
        if ( iMax < iDiff )
          iMax = iDiff ;
      }
      if ( pVProf && (y >= iYOrig) && (y < (pVProf->m_iProfLen + iYOrig) ) )
      {
        iSumBack += iBack ;
        pVProf->m_pProfData[y - iYOrig] -= iBack ;  
      }
    }
  }
  else if ( dwCompression == BI_Y16 )
  {
    LPWORD pData= (LPWORD) GetData(frame);

    for (int y=0; y < height ; y++)
    {
      int iBack = 0 ;
      LPWORD pLeft = pData + (width*y) ;
      LPWORD pRight = pLeft + width - iFromSides ;
      LPWORD pRow = pLeft ;
      for ( int x = 0 ; x < iFromSides ; x++ )
        iBack += (int) (*(pLeft++)) + (int)(*(pRight++));
      iBack /= 2 * iFromSides ;
      for ( int x = 0 ; x < width ; x++ )
      {
        int iDiff = (int) (*pRow) - iBack ;
        *(pRow++) = ( iDiff >= 0)? (WORD)iDiff : 0 ;
        if ( iMax < iDiff )
          iMax = iDiff ;
      }
      if ( pVProf && (y >= iYOrig) && (y < (pVProf->m_iProfLen + iYOrig) ) )
      {
        iSumBack += iBack ;
        pVProf->m_pProfData[y - iYOrig] -= iBack ;  
      }
    }
  }
  if ( iSumBack )
  {
    if ( pVProf  &&  pHProf )
    {
      double dAveBack = (double)iSumBack/(double)pVProf->m_iProfLen ;
      pVProf->m_dMinValue -= dAveBack ;
      if ( pVProf->m_dMinValue < 0. )
        pVProf->m_dMinValue = 0. ;
      for (int x=0; x < pHProf->m_iProfLen ; x++)
      {
        if ( pHProf->m_pProfData[x] >= dAveBack )
          pHProf->m_pProfData[x] -= dAveBack ;
        else
          pHProf->m_pProfData[x] = 0 ;
      }
    }
    return iMax ;
  }
  return -1 ;
}


#endif  //_SEEK_SPOTS_INC