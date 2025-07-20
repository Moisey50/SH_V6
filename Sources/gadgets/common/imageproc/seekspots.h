//  $File : seekspot.h - utility for coordinating spots
//  (C) Copyright The File X Ltd 2009-2023 
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
#include <helpers\propertykitEx.h>
#include <imageproc/ImageProfile.h>
#include <ImageProc/LinearSignals.h>


#define DISPERTION_TRIGGER 5000
#define MIN_AMPL      20
#define MIN_DEVIATION 2
#define DEFAULT_NORM_THRES 0.5
#define DEFAULT_DEBOUNCING 2


enum VOBJ_TYPE
{
  TASK = -1 ,
  NONE = 0 ,
  LINE_SEGMENT =1, // line detection
  BORDER =2,       // edge detection
  AREA_BRIGHTNESS =3,
  SPOT =4,
  CONTOURS_QUANTITIES =5,
  OCR=6,                  // for OCR
  EDGE=7 // edge detection
};


enum VOBJ_DIR
{
  DIR_UNKNOWN = 65535 ,
  HORIZONTAL=1 ,
  VERTICAL=2 ,
  ANY_DIRECTION=255,
  DIR_00 = 1200 ,
  DIR_01_30 = 1330 ,
  DIR_03 = 1500 ,
  DIR_04_30 = 1630 ,
  DIR_06 = 1800 ,
  DIR_07_30 = 1930 ,
  DIR_09 = 2100 ,
  DIR_10_30 = 2230 ,
  DIR_LR=DIR_03,
  DIR_RL=DIR_09,
  DIR_UD=DIR_06,
  DIR_DU=DIR_00,
  DIR_N = 10000 ,
  DIR_NNE = 10022 ,
  DIR_NE = 10045 ,
  DIR_NEE = 10068 ,
  DIR_E = 10090 ,
  DIR_SEE = 10112 ,
  DIR_SE = 10135 ,
  DIR_SSE = 10158 ,
  DIR_S = 10180 ,
  DIR_SSW = 10202 ,
  DIR_SW = 10225 ,
  DIR_SWW = 10248 ,
  DIR_W = 10270 ,
  DIR_NWW = 10292 ,
  DIR_NW = 10315 ,
  DIR_NNW = 10338 ,
};

inline bool IsVertDir( VOBJ_DIR dir )
{
  return ( ( dir == VERTICAL ) || ( dir == DIR_00 ) || ( dir == DIR_06 )
    || ( dir == DIR_N ) || ( dir == DIR_S ) ) ;
}

inline bool IsHorizDir( VOBJ_DIR dir )
{
  return ( ( dir == HORIZONTAL ) || ( dir == DIR_03 ) || ( dir == DIR_09 )
    || ( dir == DIR_E ) || ( dir == DIR_W ) ) ;
}

enum VOBJ_CONTRAST
{
  WHITE_ON_BLACK , // for line or contours
  BLACK_ON_WHITE , // for line or contours
  ANY_CONTRAST ,   // for line or contours
  WHITE_TO_BLACK_BRD , // for borders  
  BLACK_TO_WHITE_BRD , // for borders 
  ANY_CONTRAST_BORDER
};

enum VOBJ_PLACEMENT
{
  PLACE_UNKNOWN = -1 ,
  PLACE_ABS = 0 ,
  PLACE_REL = 1 ,
  PLACE_REL_XY = 3 ,
  PLACE_COPY_REL = 4 // take all parameters of leader and only do ROI offset 
                     // NOT relatively to measured coordinates, but relatively to 
                     // xoffset and yoffset of leader
};


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

// using namespace std;

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

// This function find extreme sample and compare with previous and next samples
// Result is around extreme samples in range +-0.5 as dependence of
// neighbor samples values
// Exception - if several pixels have extreme values, then
// found point is in the middle of all samples with extreme values
// if extreme value is on the edge of input array, value zero will
// be returned
__forceinline double find_extrem_pos_by3( double * pProf ,
  int iLength , double dMin , double dMax , bool bMax )
{
  double *pD = pProf /*+ 1*/ ;
  double *pEnd = pD + iLength - 1 ;
  double dExtremVal = ( bMax ) ? dMax : dMin ;
  do 
  {
    if (fabs( dExtremVal - ( *pD ) ) < 1e-6)
      break ;
  } while ( ++pD < pEnd) ;
  if ( (pD >= (pEnd - 1)) // edge positions can't be extremum
    || (pD <= pProf) )
    return 0. ;

  double * pFoundPos = pD ;
  double dFound = ( double ) ( pD - pProf ) ;
  double dToPrev = fabs( *( pD - 1 ) - dExtremVal ) ;
  double dToNext = fabs( *( pD + 1 ) - dExtremVal ) ;
  if ( dToNext < 1e-6 ) // may be saturation or anti saturation
  {
    while ( fabs(*pD - dExtremVal) < 1e-6 ) 
    {
      if (++pD >= pEnd) 
        return 0. ;
    }
    dFound += ( double ) ( pD - pFoundPos ) / 2. ;
  }
  else
    dFound += -0.5 + (dToPrev / ( dToNext + dToPrev )) ;
  return dFound ;
}

__forceinline bool seekBorders(DRECT& ResultRC, 
    Profile& HProf ,
    Profile& VProf ,   
    DWORD White=0, //0 - Black-to-White, otherwise - white-to-black
    DWORD dir=3,   // 0x02 - vertical, 0x01 - horizontal 
    double dMinAmpl = MIN_AMPL ,
    double dNormThres = DEFAULT_NORM_THRES ,
    int iDebouncing = 0 )
{

  if ( ((dir & VERTICAL) && ( (HProf.m_dMaxValue - HProf.m_dMinValue) < dMinAmpl ) /*(dispX < 5)*/ /*DISPERTION_TRIGGER*/) 
    || ((dir & HORIZONTAL) && ( (VProf.m_dMaxValue - VProf.m_dMinValue) < dMinAmpl
    ) /*(dispY < 5)*/ /*DISPERTION_TRIGGER*/) )
  {
    return false;
  }
  double dHThres = HProf.m_dMinValue + (dNormThres * ( HProf.m_dMaxValue - HProf.m_dMinValue )) ;
  double dVThres = VProf.m_dMinValue + (dNormThres * (VProf.m_dMaxValue - VProf.m_dMinValue)) ;


  //    TRACE(" Levels hl %d, vl %d\n", hl*width, vl*height);
  memset(&ResultRC,0,sizeof(DRECT));
  bool onSpot=false;
  if ( dir & VERTICAL ) // vertical borders           
  {
    ResultRC.left = find_border_forw( HProf.m_pProfData , HProf.m_iProfLen , dHThres ) ;
    ResultRC.right = find_border_back( HProf.m_pProfData , HProf.m_iProfLen , dHThres ) ;
  }
  if ( dir & HORIZONTAL ) // Horizontal borders
  {
    ResultRC.top = find_border_forw( VProf.m_pProfData , VProf.m_iProfLen , dVThres ) ;
    ResultRC.bottom = find_border_back( VProf.m_pProfData , VProf.m_iProfLen , dVThres ) ;
  }
  return true ;
}           


__forceinline bool seekBorders(
  const pTVFrame frame ,
  DRECT& ResultRC ,
  pProfile pH ,
  pProfile pV ,
  DWORD White = 0 , //0 - Black-to-White, otherwise - white-to-black
  DWORD dir = 3 ,   // 0x02 - vertical, 0x01 - horizontal 
  double dMinAmpl = MIN_AMPL ,
  double dNormThres = DEFAULT_NORM_THRES ,
  int iDebouncing = 0 )
{

  ASSERT( frame != NULL );
  ASSERT( frame->lpBMIH != NULL );

  if ( !frame || !frame->lpBMIH )
    return false ;

  if ( dNormThres >= 1.0 )
    _calc_diff_image( frame ) ;

  double dMean = calc_profiles( frame , pH , pV ) ;
  if ( dMean == 0. )
    return false ;
  if ( ((dir & VERTICAL) && ((pH->m_dMaxValue - pH->m_dMinValue) < dMinAmpl) /*(dispX < 5)*/ /*DISPERTION_TRIGGER*/)
    || ((dir & HORIZONTAL) && ((pV->m_dMaxValue - pV->m_dMinValue) < dMinAmpl
    ) /*(dispY < 5)*/ /*DISPERTION_TRIGGER*/) )
  {
    return false;
  }

  double dThresAdd = dMean + ((White == 0) ? dNormThres : -dNormThres) ;

  double dHThres = (dNormThres >= 1.0) ?
    dThresAdd : pH->m_dMinValue + (dNormThres * (pH->m_dMaxValue - pH->m_dMinValue)) ;
  double dVThres = ((dNormThres >= 1.0) ?
    dThresAdd : pV->m_dMinValue + (dNormThres * (pV->m_dMaxValue - pV->m_dMinValue))) ;


  //    TRACE(" Levels hl %d, vl %d\n", hl*width, vl*height);
  memset( &ResultRC , 0 , sizeof( DRECT ) );
  bool onSpot = false;
  if ( dir & VERTICAL ) // vertical lines or borders left-to-right          
  {
    if ( (dir & DIR_09) == DIR_09 ) //border right-to-left
    {
      int iEnd = pH->m_iProfLen - 1 ;
      do
      {
        ResultRC.right = find_slope_back( pH->m_pProfData , iEnd , dHThres , White == 0 ) ;
        if ( ResultRC.right == 0. )
          break ; // not found
        iEnd = (int) floor( ResultRC.right ) - 1 ;
        if ( iEnd <= 0 )
          break ;
        ResultRC.left = find_slope_back(
          pH->m_pProfData , iEnd , dHThres , White != 0 ) ;
        if ( ResultRC.left == 0. )
          break ; // not found
        if ( ResultRC.right - ResultRC.left >= iDebouncing )
          break ;
        iEnd = (int) floor( ResultRC.left ) ;
      } while ( iEnd > 0 ) ;

    }
    else
    {
      int iStart = 0 ;
      do
      {
        ResultRC.left = find_slope_forw(
          pH->m_pProfData + iStart , pH->m_iProfLen - iStart , dHThres , White == 0 ) ;
        if ( ResultRC.left == 0. )
          break ; // not found
        ResultRC.left += iStart ;
        iStart = (int) ceil( ResultRC.left ) ;
        ResultRC.right = find_slope_forw(
          pH->m_pProfData + iStart , pH->m_iProfLen - iStart , dHThres , White != 0 ) ;
        if ( ResultRC.right == 0. )
          break ; // not found
        ResultRC.right += iStart ;
        if ( ResultRC.right - ResultRC.left >= iDebouncing )
          break ;
        iStart = (int) ceil( ResultRC.right ) ;
      } while ( iStart < pH->m_iProfLen ) ;
    }
  }
  if ( dir & HORIZONTAL ) // Horizontal borders
  {
    if ( (dir & DIR_00) == DIR_00 ) //border down-to-up
    {
      int iEnd = pV->m_iProfLen - 1 ;
      do
      {
        ResultRC.bottom = find_slope_back( pV->m_pProfData , iEnd , dVThres , White == 0 ) ;
        if ( ResultRC.bottom == 0. )
          break ; // not found
        iEnd = (int) floor( ResultRC.bottom ) - 1 ;
        if ( iEnd <= 0 )
          break ;
        ResultRC.top = find_slope_back(
          pV->m_pProfData , iEnd , dHThres , White != 0 ) ;
        if ( ResultRC.top == 0. )
          break ; // not found
        if ( ResultRC.bottom - ResultRC.top >= iDebouncing )
          break ;
        iEnd = (int) floor( ResultRC.top ) - 1 ;
      } while ( iEnd > 0 ) ;

    }
    else
    {
      int iStart = 0 ;
      do
      {
        ResultRC.top = find_slope_forw(
          pV->m_pProfData + iStart , pV->m_iProfLen - iStart , dVThres , White == 0 ) ;
        if ( ResultRC.top == 0. )
          break ; // not found
        ResultRC.top += iStart ;
        iStart = (int) ceil( ResultRC.top ) ;
        ResultRC.bottom = find_slope_forw(
          pV->m_pProfData + iStart , pV->m_iProfLen - iStart , dVThres , White != 0 ) ;
        if ( ResultRC.bottom == 0. )
          break ; // not found
        ResultRC.bottom += iStart ;
        if ( ResultRC.bottom - ResultRC.top >= iDebouncing )
          break ;
        iStart = (int) ceil( ResultRC.bottom ) ;
      } while ( iStart < pV->m_iProfLen ) ;
    }
  }
  return true ;
}

__forceinline bool seekEdges(
  const pTVFrame frame, 
   DRECT& ResultRC, 
   pProfile pH ,
   pProfile pV ,   
   DWORD WhiteToBlack=0, //0 - Black-to-White, otherwise - white-to-black
   DWORD dir = 3 ,    // DIR_LR = DIR_03 - seek from left to right
                      // DIR_RL = DIR_09 - seek from right to left
                      // DIR_UD = DIR_06 - seek from up to down
                      // DIR_DU = DIR_00 - seek from down to up
   double dMinAmpl = MIN_AMPL ,
   double dNormThres = DEFAULT_NORM_THRES ,
   int iDebouncing = 0 )
{
  ASSERT(frame!=NULL); 
  ASSERT(frame->lpBMIH!=NULL); 

  if ( !frame  ||  !frame->lpBMIH )
    return false ;

  if ( dNormThres >= 1.0 )
    _calc_diff_image( frame ) ;

  double dMean = calc_profiles( frame , pH , pV ) ;
  if ( dMean == 0. )
    return false ;
  bool bVert = (dir & VERTICAL) || ( dir == ANY_DIRECTION )
    || (dir == DIR_LR) || (dir == DIR_RL) ;
  bool bHoriz = ( dir & HORIZONTAL ) || ( dir == ANY_DIRECTION )
    || ( dir == DIR_UD ) || ( dir == DIR_DU ) ;
  if ( ((pH->m_dMaxValue - pH->m_dMinValue) < dMinAmpl) /*(dispX < 5)*/ /*DISPERSION_TRIGGER*/ )
      bVert = false ;
  if ( (pV->m_dMaxValue - pV->m_dMinValue) < dMinAmpl ) /*(dispY < 5)*/ /*DISPERSION_TRIGGER*/
    bHoriz = false;
  if ( !bVert && !bHoriz )
    return false ;

  double dThresAdd = dMean + ((WhiteToBlack == 0) ? dNormThres : -dNormThres) ;

  double dHThres = (dNormThres >= 1.0) ?  
    dThresAdd : pH->m_dMinValue + (dNormThres * ( pH->m_dMaxValue - pH->m_dMinValue )) ;
  double dVThres = ((dNormThres >= 1.0) ? 
    dThresAdd : pV->m_dMinValue + (dNormThres * (pV->m_dMaxValue - pV->m_dMinValue))) ;

  memset(&ResultRC,0,sizeof(DRECT));
  bool onSpot=false;
  switch ( dir )
  {
  case VERTICAL:
  case DIR_LR:
    {
      int iLimit = pH->m_iProfLen - 1 ;
      int iStart = 0 ;
      bool bNextFrontIsOK = (*( pH->m_pProfData ) >= dHThres)
        ^ (WhiteToBlack == 0) ;

      do
      {
        ResultRC.left = find_border_forw( pH->m_pProfData + iStart , iLimit - iStart , dHThres ) ;
        if ( ResultRC.left == 0./*(ResultRC.left += iStart) >= iLimit - 1*/ )
          break ; // not found

        ResultRC.left += iStart ;
        iStart = ( int ) ceil( ResultRC.left + 1. ) ;
        if ( !bNextFrontIsOK )
        { // we found front before necessary, continue 
          bNextFrontIsOK = true ;
          ResultRC.left = 0. ;
          continue ;
        }
        if ( iDebouncing == 0 )
          break ;

        // checking for debouncing
        ResultRC.right = find_border_forw(  // find opposite edge
          pH->m_pProfData + iStart , iLimit - iStart , dHThres ) ;
        if ( ResultRC.right == 0. )
          break ; // not found, OK
        if ( ResultRC.right >= iDebouncing )
        {
          ResultRC.right += iStart ;
          break ; // found far enough, OK
        }
        iStart = ((int) ceil( ResultRC.right )) + iStart ; // try to find next one
        ResultRC.left = 0. ; // mark, that edge not found
      } while ( iStart < iLimit ) ;
    }
    break ;

  case DIR_RL:
    {
      int iEnd = pH->m_iProfLen - 1 ;
      bool bNextFrontIsOK = (*(pH->m_pProfData + iEnd) >= dHThres)
        ^ (WhiteToBlack == 0) ;
      do
      {
        ResultRC.right = find_border_back( pH->m_pProfData , iEnd , dHThres ) ;
        if ( ResultRC.right <= 1. ) 
        {
          ResultRC.right = 0. ;
          break ; // not found
        }
        iEnd = (int) floor( ResultRC.right ) ;
        if ( !bNextFrontIsOK )
        { // we found front before necessary, continue 
          bNextFrontIsOK = true ;
          ResultRC.right = 0. ;
          continue ;
        }
        if ( iDebouncing == 0 )
          break ;

        // checking for debouncing
        ResultRC.left = find_border_back(  // find opposite edge
          pH->m_pProfData , iEnd , dHThres ) ;
        if ( ResultRC.left == 0. ) 
          break ; // not found, OK
        if ( ResultRC.right - ResultRC.left >= iDebouncing )
          break ; // found far enough, OK

        ResultRC.right = 0. ; // mark, that edge not found
        iEnd = (int) floor( ResultRC.left ) ;
      } while ( iEnd > 0 ) ;
    }
    break ;

  case DIR_DU:
    {
      int iEnd = pV->m_iProfLen - 1 ;
      bool bNextFrontIsOK = (*(pV->m_pProfData + iEnd) >= dVThres)
        ^ (WhiteToBlack == 0) ;
      do
      {
        ResultRC.bottom = find_border_back( pV->m_pProfData , iEnd , dVThres ) ;
        if ( ResultRC.bottom == 0. )
          break ; // not found
        if ( ResultRC.bottom <= 1. )
        {
          ResultRC.bottom = 0. ;
          break ;
        }
        iEnd = (int) floor( ResultRC.bottom ) - 1 ;
        if ( !bNextFrontIsOK )
        { // we found front before necessary, continue 
          bNextFrontIsOK = true ;
          ResultRC.bottom = 0. ;
          continue ;
        }
        if ( iDebouncing == 0 )
          break ;

        ResultRC.top = find_border_back(
          pV->m_pProfData , iEnd , dVThres ) ;
        if ( ResultRC.top == 0. )
          break ; // not found
        if ( ResultRC.bottom - ResultRC.top >= iDebouncing )
          break ; // found too far, OK
        iEnd = (int) floor( ResultRC.top ) - 1 ;
        ResultRC.bottom = 0. ; // mark, that edge not found
      } while ( iEnd > 0 ) ;
    }
    break ;
  case HORIZONTAL:
  case DIR_UD:
    {
      int iStart = 0 , iLimit = pV->m_iProfLen - 1 ;
      bool bNextFrontIsOK = ( *pV->m_pProfData >= dVThres)
        ^ (WhiteToBlack == 0) ;
      do
      {
        ResultRC.top = find_border_forw(
          pV->m_pProfData + iStart , iLimit - iStart , dVThres ) ;
        if ( (ResultRC.top += iStart) >= iLimit - 1 )
        {
          ResultRC.top = 0. ;
          break ; // not found
        }
        iStart = (int) ceil( ResultRC.top ) ;
        if ( !bNextFrontIsOK )
        { // we found front before necessary, continue 
          bNextFrontIsOK = true ;
          ResultRC.top = 0. ;
          continue ;
        }
        if ( iDebouncing == 0 )
          break ; // OK, no near opposite edge
        ResultRC.bottom = find_border_forw(
          pV->m_pProfData + iStart , iLimit - iStart , dVThres ) ;
        if ( ResultRC.bottom == 0. )
          break ; // not found, OK
        if ( ResultRC.bottom > iDebouncing )
        {
          ResultRC.bottom += iStart ;
          break ;  // found far enough, OK
        }
        iStart = (int) ceil( ResultRC.bottom ) + iStart ;
        ResultRC.top = 0. ; // mark, that edge not found
      } while ( iStart < pV->m_iProfLen ) ;
    }
    break ;
  }
  
  return true ;
}           


__forceinline bool seekBorders(const pTVFrame frame, DRECT& rc, 
                               pProfile pHProf ,
                               pProfile pVProf ,   
                               double dNormThres = DEFAULT_NORM_THRES ,
                               int iDebouncing = 0 )
{
  return seekBorders( frame, rc, pHProf , pVProf , 0 , 3 , 
    MIN_AMPL , dNormThres , iDebouncing ) ;

}


__forceinline bool seekBorders(const pTVFrame frame, DRECT& rc, 
                               Profile& HProf ,
                               Profile& VProf ,   
                               double dNormThres = DEFAULT_NORM_THRES ,
                               int iDebouncing = 0 )
{
  return seekBorders( frame, rc,  &HProf , &VProf , 0 , 3 , 
    MIN_AMPL , dNormThres , iDebouncing ) ;

}
__forceinline bool seekMultiBorders(
  const pTVFrame frame, CDRectArray& Results, 
  double * pHProf ,
  double * pVProf ,   
  DWORD WhiteToBlack =0, //=0 - Black-to-White, =1 - white-to-black, =2 - any
  DWORD dir=3,   // 2 - vertical, 1 - horizontal 
  double dMinAmpl = MIN_AMPL ,
  double dNormThres = DEFAULT_NORM_THRES ,
  int iDebouncing = 0 )
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

  bool bVert = ( dir & VERTICAL ) || ( dir == ANY_DIRECTION )
    || ( dir == DIR_LR ) || ( dir == DIR_RL ) ;
  bool bHoriz = ( dir & HORIZONTAL ) || ( dir == ANY_DIRECTION )
    || ( dir == DIR_UD ) || ( dir == DIR_DU ) ;
  if ( ( ( pH->m_dMaxValue - pH->m_dMinValue ) < dMinAmpl ) /*(dispX < 5)*/ /*DISPERSION_TRIGGER*/ )
    bVert = false ;
  if ( ( pV->m_dMaxValue - pV->m_dMinValue ) < dMinAmpl ) /*(dispY < 5)*/ /*DISPERSION_TRIGGER*/
    bHoriz = false;
  if ( !bVert && !bHoriz )
    return false ;

  double dThresAdd = dMean + ((WhiteToBlack == 0) ? dNormThres : -dNormThres) ;

  double dHThres = (dNormThres >= 1.0) ?  
    dThresAdd : pH->m_dMinValue + (dNormThres * ( pH->m_dMaxValue - pH->m_dMinValue )) ;
  double dVThres = ((dNormThres >= 1.0) ? 
    dThresAdd : pV->m_dMinValue + (dNormThres * (pV->m_dMaxValue - pV->m_dMinValue))) ;

  DRECT ResultRC ;

  memset( &ResultRC , 0 , sizeof( DRECT ) );
  bool onSpot = false;
  switch ( dir )
  {
  case VERTICAL:
  case DIR_LR:
    {
      int iLimit = pH->m_iProfLen - 1 ;
      int iStart = 0 ;
      bool bNextFrontIsOK = ( *( pH->m_pProfData ) >= dHThres )
        ^ ( WhiteToBlack == 0 ) ;

      do
      {
        ResultRC.left = find_border_forw( pH->m_pProfData + iStart , iLimit - iStart - iDebouncing + 1 , dHThres ) ;
        if ( ResultRC.left == 0. )
          break ; // not found or found near end of scan, finished

        ResultRC.left += iStart ;
        iStart = ( int ) ceil( ResultRC.left + 1. ) ;
        if ( !bNextFrontIsOK )
        { // we found front before necessary, continue 
          bNextFrontIsOK = true ;
          ResultRC.left = 0. ;
          continue ;
        }
        // checking for debouncing
        ResultRC.right = find_border_forw(  // find opposite edge
          pH->m_pProfData + iStart , iLimit - iStart , dHThres ) ;
        if ( ResultRC.right == 0. ) // opposite front is not found
        {    // 
//           ResultRC.right = pH->m_iProfLen - 1 ;
//           Results.Add( ResultRC ) ; // save front data
          break ; // not found, OK
        }
        if ( ResultRC.right >= iDebouncing )
        {
          ResultRC.left ;
          ResultRC.right += iStart ;
          Results.Add( ResultRC ) ;
        }
        else
          ResultRC.right += iStart ;
        iStart = ( ( int ) ceil( ResultRC.right ) ) + 1 ; // try to find next one
        ResultRC.left = 0. ; // mark, that edge not found
      } while ( iStart < iLimit ) ; // go to find next edge, if necessary
    }
    break ;

  case DIR_RL:
    {
      int iEnd = pH->m_iProfLen - 1 ;
      bool bNextFrontIsOK = ( *( pH->m_pProfData + iEnd ) >= dHThres )
        ^ ( WhiteToBlack == 0 ) ;
      do
      {
        ResultRC.right = find_border_back( pH->m_pProfData , iEnd , dHThres ) ;
        if ( ResultRC.right <= 1. )
          break ; // not found

        iEnd = ( int ) floor( ResultRC.right ) - 1 ;
        if ( !bNextFrontIsOK )
        { // we found front before necessary, continue 
          bNextFrontIsOK = true ;
          ResultRC.right = 0. ;
          continue ;
        }

        // checking for debouncing
        ResultRC.left = find_border_back(  // find opposite edge
          pH->m_pProfData , iEnd , dHThres ) ;
        if ( ResultRC.left == 0. )
        {
          Results.Add( ResultRC ) ; // last edge detected, add to results
          break ; // not found, OK
        }
        if ( ResultRC.right - ResultRC.left >= iDebouncing )
          Results.Add( ResultRC ) ;

        ResultRC.right = 0. ; // mark, that edge not found
        iEnd = ( int ) floor( ResultRC.left ) - 1 ;
      } while ( iEnd > 0 ) ;
    }
    break ;

  case HORIZONTAL:
  case DIR_UD:
    {
      int iLimit = pV->m_iProfLen - 1 ;
      int iStart = 0 ;
      bool bNextFrontIsOK = ( *pV->m_pProfData >= dVThres )
        ^ ( WhiteToBlack == 0 ) ;
      do
      {
        ResultRC.top = find_border_forw(
          pV->m_pProfData + iStart , iLimit - iStart , dVThres ) ;
        if ( ResultRC.top == 0. )
          break ; // not found

        if ( ( ResultRC.top += iStart ) >= iLimit - 1 )
        {
          ResultRC.top = 0. ;
          break ; // not found
        }
        iStart = ( int ) ceil( ResultRC.top ) ;
        if ( !bNextFrontIsOK )
        { // we found front before necessary, continue 
          bNextFrontIsOK = true ;
          ResultRC.top = 0. ;
          continue ;
        }
        ResultRC.bottom = find_border_forw(
          pV->m_pProfData + iStart , iLimit - iStart , dVThres ) ;
        if ( ResultRC.bottom == 0. )
        {
//           ResultRC.bottom = pV->m_iProfLen ;
//           Results.Add( ResultRC ) ; // add last edge info to results
          break ; // not found, OK
        }
        ResultRC.bottom += iStart ;
        if ( ResultRC.bottom - ResultRC.top > iDebouncing )
        {
          Results.Add( ResultRC ) ;
        }
        iStart = ( int ) ResultRC.bottom + 1 ;
        ResultRC.top = 0. ; // mark, that edge not found
      } while ( iStart < pV->m_iProfLen ) ; // go to search for next edge if necessary
    }
    break ;
  case DIR_DU:
    {
      int iEnd = pV->m_iProfLen - 1 ;
      bool bNextFrontIsOK = ( *( pV->m_pProfData + iEnd ) >= dVThres )
        ^ ( WhiteToBlack == 0 ) ;
      do
      {
        ResultRC.bottom = find_border_back( pV->m_pProfData , iEnd , dVThres ) ;
        if ( ResultRC.bottom <= 1. )
          break ;

        iEnd = ( int ) floor( ResultRC.bottom ) - 1 ;
        if ( !bNextFrontIsOK )
        { // we found front before necessary, continue 
          bNextFrontIsOK = true ;
          ResultRC.bottom = 0. ;
          continue ;
        }

        ResultRC.top = find_border_back(
          pV->m_pProfData , iEnd , dVThres ) ;
        if ( ResultRC.top == 0. )
        {
          Results.Add( ResultRC ) ; // last edge detected, add to results
          break ; // not found, OK
        }
        if ( ResultRC.bottom - ResultRC.top >= iDebouncing )
          Results.Add( ResultRC ) ;
        iEnd = ( int ) floor( ResultRC.top ) - 1 ;
        ResultRC.bottom = 0. ; // mark, that edge not found
      } while ( iEnd > 0 ) ;
    }
    break ;
  }
  return (Results.GetCount() > 0) ;
}           

__forceinline bool seekMultiBorders(
  CDRectArray& Results ,
  pProfile pH , pProfile pV ,
  DWORD WhiteToBlack = 0 , //=0 - Black-to-White, =1 - white-to-black, =2 - any
  DWORD dir = 3 ,   // 2 - vertical, 1 - horizontal 
  double dMinAmpl = MIN_AMPL ,
  double dNormThres = DEFAULT_NORM_THRES ,
  int iDebouncing = 0 )
{
  bool bVert = ( dir & VERTICAL ) || ( dir == ANY_DIRECTION )
    || ( dir == DIR_LR ) || ( dir == DIR_RL ) ;
  bool bHoriz = ( dir & HORIZONTAL ) || ( dir == ANY_DIRECTION )
    || ( dir == DIR_UD ) || ( dir == DIR_DU ) ;
  if ( ( ( pH->m_dMaxValue - pH->m_dMinValue ) < dMinAmpl ) /*(dispX < 5)*/ /*DISPERSION_TRIGGER*/ )
    bVert = false ;
  if ( ( pV->m_dMaxValue - pV->m_dMinValue ) < dMinAmpl ) /*(dispY < 5)*/ /*DISPERSION_TRIGGER*/
    bHoriz = false;
  if ( !bVert && !bHoriz )
    return false ;

  // 0. < dNormThres < 1.
  double dHThres = pH->m_dMinValue + ( dNormThres * ( pH->m_dMaxValue - pH->m_dMinValue ) ) ;
  double dVThres = pV->m_dMinValue + ( dNormThres * ( pV->m_dMaxValue - pV->m_dMinValue ) ) ;

  DRECT ResultRC ;

  memset( &ResultRC , 0 , sizeof( DRECT ) );
  bool onSpot = false;
  switch ( dir )
  {
    case VERTICAL:
    case DIR_LR:
    {
      int iLimit = pH->m_iProfLen - 1 ;
      int iStart = 0 ;
      while ( ( *( pH->m_pProfData + iStart ) >= dHThres ) ^ ( (WhiteToBlack & 1) != 0 ) )
      {
        if ( ++iStart >= pH->m_iProfLen )
          return false ; // no edges
      }
      do
      {
        ResultRC.left = find_border_forw( pH->m_pProfData + iStart , iLimit - iStart - iDebouncing + 1 , dHThres ) ;
        if ( ResultRC.left == 0. )
          break ; // not found or found near end of scan, finished

        ResultRC.left += iStart ;
        //iStart = ( int )ceil( ResultRC.left + 1. );
        iStart = ( int )ceil( ResultRC.left ) ;
        if (( iStart - ResultRC.left ) < 1e-10)
          iStart++;
        ResultRC.right = find_border_forw(  // find opposite edge
          pH->m_pProfData + iStart , iLimit - iStart , dHThres ) ;
        if ( ResultRC.right == 0. ) // opposite front is not found
        {    // 
//           ResultRC.right = pH->m_iProfLen - 1 ;
//           Results.Add( ResultRC ) ; // save front data
          break ; // not found, OK
        }
        // checking for debouncing
        if ( ResultRC.right >= iDebouncing )
        {
          ResultRC.left ;
          ResultRC.right += iStart ;
          Results.Add( ResultRC ) ;
        }
        else
          ResultRC.right += iStart ;
        //iStart = ( ( int )ceil( ResultRC.right ) ) + 1; // try to find next one
        iStart = ( ( int )ceil( ResultRC.right ) ) ; // try to find next one
        if (( iStart - ResultRC.right ) < 1e-10)
          iStart++;
        ResultRC.left = 0. ; // mark, that edge not found
      } while ( iStart < iLimit ) ; // go to find next edge, if necessary
    }
    break ;

    case DIR_RL:
    {
      int iEnd = pH->m_iProfLen - 1 ;
      while ( ( *( pH->m_pProfData + iEnd ) >= dHThres ) ^ ( ( WhiteToBlack & 1 ) != 0 ) )
      {
        if ( --iEnd <= 1 )
          return false ; // no edges
      }

      do
      {
        ResultRC.right = find_border_back( pH->m_pProfData , iEnd , dHThres ) ;
        if ( ResultRC.right <= 1. )
          break ; // not found

        iEnd = ( int ) floor( ResultRC.right ) ;
        if (( ResultRC.right - iEnd ) < 1e-10)
          iEnd--;

        // checking for debouncing
        ResultRC.left = find_border_back(  // find opposite edge
          pH->m_pProfData , iEnd , dHThres ) ;
        if ( ResultRC.left == 0. )
        {
          Results.Add( ResultRC ) ; // last edge detected, add to results
          break ; // not found, OK
        }
        if ( ResultRC.right - ResultRC.left >= iDebouncing )
          Results.Add( ResultRC ) ;

        ResultRC.right = 0. ; // mark, that edge not found
        iEnd = ( int ) floor( ResultRC.left ) ;
        if (( ResultRC.left - iEnd ) < 1e-10)
          iEnd--;
      } while ( iEnd > 0 ) ;
    }
    break ;

    case HORIZONTAL:
    case DIR_UD:
    {
      int iLimit = pV->m_iProfLen - 1 ;
      int iStart = 0 ;
      while ( ( *( pV->m_pProfData + iStart ) >= dVThres ) ^ ( ( WhiteToBlack & 1 ) != 0 ) )
      {
        if ( ++iStart >= pV->m_iProfLen )
          return false ; // no edges
      }
      do
      {
        ResultRC.top = find_border_forw(
          pV->m_pProfData + iStart , iLimit - iStart , dVThres ) ;
        if ( ResultRC.top == 0. )
          break ; // not found

        if ( ( ResultRC.top += iStart ) >= iLimit - 1 )
        {
          ResultRC.top = 0. ;
          break ; // not found
        }
        iStart = ( int ) ceil( ResultRC.top ) ;
        if (( iStart - ResultRC.left ) < 1e-10)
          iStart++;
        ResultRC.bottom = find_border_forw(
          pV->m_pProfData + iStart , iLimit - iStart , dVThres ) ;
        if ( ResultRC.bottom == 0. )
        {
//           ResultRC.bottom = pV->m_iProfLen ;
//           Results.Add( ResultRC ) ; // add last edge info to results
          break ; // not found, OK
        }
        ResultRC.bottom += iStart ;
        if ( ResultRC.bottom - ResultRC.top > iDebouncing )
        {
          Results.Add( ResultRC ) ;
        }
        iStart = ( int ) ceil( ResultRC.bottom );
        if (( iStart - ResultRC.bottom ) < 1e-10)
          iStart++;

        ResultRC.top = 0. ; // mark, that edge not found
      } while ( iStart < pV->m_iProfLen ) ; // go to search for next edge if necessary
    }
    break ;
    case DIR_DU:
    {
      int iEnd = pV->m_iProfLen - 1 ;
      while ( ( *( pV->m_pProfData + iEnd ) >= dHThres ) ^ ( ( WhiteToBlack & 1 ) != 0 ) )
      {
        if ( --iEnd <= 1 )
          return false ; // no edges
      }
      do
      {
        ResultRC.bottom = find_border_back( pV->m_pProfData , iEnd , dVThres ) ;
        if ( ResultRC.bottom <= 1. )
          break ;

        iEnd = ( int ) floor( ResultRC.bottom ) ;
        if (( ResultRC.bottom - iEnd ) < 1e-10)
          iEnd--;

        ResultRC.top = find_border_back(
          pV->m_pProfData , iEnd , dVThres ) ;
        if ( ResultRC.top == 0. )
        {
          Results.Add( ResultRC ) ; // last edge detected, add to results
          break ; // not found, OK
        }
        if ( ResultRC.bottom - ResultRC.top >= iDebouncing )
          Results.Add( ResultRC ) ;
        iEnd = ( int ) floor( ResultRC.top ) ;
        if (( ResultRC.top - iEnd ) < 1e-10)
          iEnd--;

        ResultRC.bottom = 0. ; // mark, that edge not found
      } while ( iEnd > 0 ) ;
    }
    break ;
  }
  return ( Results.GetCount() > 0 ) ;
}

__forceinline double _calc_integral(const pTVFrame frame, CRect& rc)
{
  DWORD width = frame->lpBMIH->biWidth;
  DWORD height=frame->lpBMIH->biHeight;
  DWORD dwCompression = frame ->lpBMIH->biCompression ;
  double dRes = 0. ;

  if ( dwCompression == BI_Y8  ||  dwCompression == BI_Y800 
    || dwCompression == BI_YUV9 || dwCompression == BI_YUV12 )
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
    || dwCompression == BI_YUV9  || dwCompression == BI_YUV12)
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
      LPWORD p = pData + (width*y) + rc.left ;
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
    || dwCompression == BI_YUV9  || dwCompression == BI_YUV12)
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

inline int GetHistogram( const pTVFrame frame , 
  FXIntArray& DataBuffer , double * dAver = NULL)
{
  DataBuffer.RemoveAll();
  int iWidth = GetWidth( frame ) ;
  int iHeigth = GetHeight( frame ) ;
  int iSize = iWidth * iHeigth ;
  LPBYTE pData = GetData( frame ) ;
  int iSum = 0;

  if ( iWidth && iHeigth )
  {
    switch ( frame->lpBMIH->biCompression )
    {
      case BI_YUV12:
      case BI_YUV9:
      case BI_Y8:
      case BI_Y800:
      {
        DataBuffer.SetSize( 256 ) ;
        int * pHist = DataBuffer.GetData() ;
        memset( pHist , 0 ,
          sizeof( DataBuffer[ 0 ] ) * DataBuffer.GetCount() )  ;
        LPBYTE pPix = pData ;
        LPBYTE pEnd = pPix + iSize ;
        while ( pPix < pEnd )
        {
          iSum += *pPix;
          pHist[ *(pPix++) ]++ ;
      }
      }
      if (dAver)
        *dAver += (double)iSum / (double)iSize;
      return 256 ;
      case BI_Y16:
      {
        DataBuffer.SetSize( 65536 ) ;
        int * pHist = DataBuffer.GetData() ;
        LPWORD pPix = (LPWORD)pData ;
        LPWORD pEnd = pPix + iWidth ;
        while ( pPix < pEnd )
        {
          iSum += *pPix;
          pHist[ *(pPix++) ]++ ;
      }
      }
      if (dAver)
        *dAver += (double)iSum / (double)iSize;
      return 65535 ;
    }
  }
  return 0 ;
}

inline int GetHistogram( const pTVFrame frame , 
  FXIntArray& DataBuffer , CRect rc , double * dAver = NULL )
{
  DataBuffer.RemoveAll();
  int iImageWidth = GetWidth( frame ) ;
  int iImageHeight = GetHeight( frame ) ;
  if ( rc.top < 0 )
    rc.top = 0 ;
  if ( rc.left < 0 )
    rc.left = 0 ;
  if ( rc.bottom > iImageHeight - 1 )
    rc.bottom = iImageHeight - 1 ;
  if ( rc.right > iImageWidth - 1 )
    rc.right = iImageWidth - 1 ;

  int iWidth = rc.Width() ;
  int iHeigth = rc.Height() ;
  int iSize = iWidth * iHeigth ;
  LPBYTE pData = GetData( frame ) ;
  int iSum = 0 ;
  
  if ( iWidth && iHeigth && frame )
  {
    switch ( frame->lpBMIH->biCompression )
    {
    case BI_YUV12:
    case BI_YUV9:
    case BI_Y8:
    case BI_Y800:
      {
        DataBuffer.SetSize( 256 ) ;
        int * pHist = DataBuffer.GetData() ;
        memset( pHist , 0 ,
          sizeof( DataBuffer[ 0 ] ) * DataBuffer.GetCount() )  ;
        for ( int iY = rc.top ; iY < rc.bottom ; iY++ )
        {
          LPBYTE pPix = pData + iY * iImageWidth + rc.left ;
          LPBYTE pEnd = pPix + iWidth ;
          while ( pPix < pEnd )
          {
            iSum += *pPix;
            pHist[ *(pPix++) ]++ ;
        }
      }
      }
      if (dAver)
        *dAver += (double)iSum / (double)iSize;
      return 256 ;
    case BI_Y16:
      {
        DataBuffer.SetSize( 65536 ) ;
        int * pHist = DataBuffer.GetData() ;
        memset( pHist , 0 ,
          sizeof( DataBuffer[ 0 ] ) * DataBuffer.GetCount() )  ;
        LPWORD pwData = (LPWORD) pData ;
        for ( int iY = rc.top ; iY < rc.bottom ; iY++ )
        {
          LPWORD pPix = pwData + iY * iImageWidth + rc.left ;
          LPWORD pEnd = pPix + iWidth ;
          while ( pPix < pEnd )
          {
            iSum += *pPix;
            pHist[ *(pPix++) ]++ ;
        }
      }
      }
      if (dAver)
        *dAver += (double)iSum / (double)iSize;
      return 65535 ;
    }
  }
  return 0 ;
}

inline int GetHProfile( const pTVFrame frame ,
  Profile& HProfile , CRect rc , double * dAver = NULL )
{
  HProfile.Reset();
  HProfile.Realloc( rc.Width() + 1 ) ;
  int iImageWidth = GetWidth( frame ) ;
  int iImageHeight = GetHeight( frame ) ;
  if ( rc.top < 0 )
    rc.top = 0 ;
  if ( rc.left < 0 )
    rc.left = 0 ;
  if ( rc.bottom > iImageHeight - 1 )
    rc.bottom = iImageHeight - 1 ;
  if ( rc.right > iImageWidth - 1 )
    rc.right = iImageWidth - 1 ;

  int iWidth = rc.Width() ;
  int iHeigth = rc.Height() ;
  LPBYTE pData = GetData( frame ) ;
  int iSum = 0 ;

  if ( iWidth && iHeigth && frame )
  {
    switch ( frame->lpBMIH->biCompression )
    {
    case BI_YUV12:
    case BI_YUV9:
    case BI_Y8:
    case BI_Y800:
      {
        for ( int iY = rc.top ; iY <= rc.bottom ; iY++ )
        {
          double * pdProfData = HProfile.m_pProfData ;
          LPBYTE pPix = pData + iY * iImageWidth + rc.left ;
          LPBYTE pEnd = pPix + iWidth ;
          while ( pPix < pEnd )
          {
            iSum += *pPix;
            *(pdProfData) += *(pPix++) ;
            if ( iY == rc.bottom )
            {
              *(pdProfData) /= (rc.Height() + 1) ;
              if ( HProfile.m_dMinValue > *pdProfData )
                HProfile.m_dMinValue = *pdProfData ;
              if ( HProfile.m_dMaxValue < *pdProfData )
                HProfile.m_dMaxValue = *pdProfData ;
            }
            pdProfData++ ;
          }
        }
      }
      if ( dAver )
        *dAver += (double) iSum / (double)iWidth ;
      return 256 ;
    case BI_Y16:
      {
        for ( int iY = rc.top ; iY <= rc.bottom ; iY++ )
        {
          double * pdProfData = HProfile.m_pProfData ;
          LPWORD pPix = (LPWORD)pData + iY * iImageWidth + rc.left ;
          LPWORD pEnd = pPix + iWidth ;
          while ( pPix < pEnd )
          {
            iSum += *pPix;
            *(pdProfData) += *(pPix++) ;
            if ( iY == rc.bottom )
            {
              *(pdProfData) /= (rc.Height() + 1) ;
              if ( HProfile.m_dMinValue > *pdProfData )
                HProfile.m_dMinValue = *pdProfData ;
              if ( HProfile.m_dMaxValue < *pdProfData )
                HProfile.m_dMaxValue = *pdProfData ;
            }
            pdProfData++ ;
          }
        }
      }
      if ( dAver )
        *dAver += (double) iSum / (double) iWidth ;
      return 65535 ;
    }
  }
  return 0 ;
}

#endif  //_SEEK_SPOTS_INC