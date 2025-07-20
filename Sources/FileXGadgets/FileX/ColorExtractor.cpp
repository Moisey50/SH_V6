// ColorExtractor.cpp : Implementation of the ColorExtractor class


#include "StdAfx.h"
#include <math\intf_sup.h>
#include "ColorExtractor.h"
#include "gadgets\VideoFrame.h"

// Decoders

__forceinline void yuvrgb(int y, int R, int G, int B, LPBYTE r, LPBYTE g, LPBYTE b)
{
  R+=y; *r=(R<0)?0:((R>255)?255:R);
  G+=y; *g=(G<0)?0:((G>255)?255:G);
  B+=y; *b=(B<0)?0:((B>255)?255:B);
}

__forceinline void yuv_to_color(int y, int R, LPBYTE r)
{
  R+=y; *r=(R<0)?0:((R>255)?255:R);
}

LPBITMAPINFOHEADER ExtractFromYUV9(LPBITMAPINFOHEADER src, LPBYTE lpData , OutputColor Color , LPVOID lpDst = NULL )
{
  if( src->biCompression != BI_YUV9  || ( (int)Color > 2 ) ) 
    return NULL;

  ASSERT(src->biWidth%4==0);
  ASSERT(src->biHeight%4==0);
  if ((src->biWidth & 0x3) || (src->biHeight & 0x3 ))
    return NULL ;
  
  DWORD newSize=src->biWidth*src->biHeight;
  DWORD OutWidth=src->biWidth;
  LPBITMAPINFOHEADER res;
  if (lpDst)
    res = (LPBITMAPINFOHEADER)lpDst;
  else
    res = (LPBITMAPINFOHEADER)malloc(src->biSize+newSize);
  memset(res,0,src->biSize+newSize);
  memcpy(res,src,src->biSize);
  res->biCompression = BI_Y8;
  res->biSizeImage   = newSize ;
  res->biBitCount    = 8;
  res->biClrUsed     = 0;
  LPBYTE srcb = (lpData) ? lpData : ((LPBYTE)src) + src->biSize ;
  LPBYTE dstb = ((LPBYTE)res) + res->biSize ;
  LPBYTE eod = srcb + src->biWidth * src->biHeight ;
  LPBYTE vsrc = eod ;
  LPBYTE usrc = eod + (src->biWidth * src->biHeight) / 16 ;
  int    u = (*usrc-128) << 13 ;
  int    v = (*vsrc-128) << 13 ;
  int   val = ( Color == Red ) ? v/7185 : 
              (Color == Green)? -(u/20687+v/14100) : u/4037 ;
  int    cWidth=src->biWidth/4;
  DWORD cntr=0;
  DWORD cntr4=0;
  int clrInfoOff=0;
  while (srcb<eod)
  {
    yuv_to_color( *(srcb++) , val , dstb++ );
    cntr++;
    yuv_to_color( *(srcb++) , val , dstb++ );
    cntr++;
    yuv_to_color( *(srcb++) , val , dstb++ );
    cntr++;
    yuv_to_color( *(srcb++) , val , dstb++ );
    cntr++;

    cntr4++;

    clrInfoOff = (cntr4 % cWidth) + (cntr4 / src->biWidth) * cWidth ;

    u=(*(usrc+clrInfoOff)-128)<<13;
    v=(*(vsrc+clrInfoOff)-128)<<13;
    val = ( Color == Red ) ? v/7185 : 
      (Color == Green)? -(u/20687+v/14100) : u/4037 ;
  }
  return res;
}
LPBITMAPINFOHEADER ExtractFromYUV12(LPBITMAPINFOHEADER src, LPBYTE lpData , OutputColor Color , LPVOID lpDst = NULL )
{
  if( src->biCompression != BI_YUV12  || ( (int)Color > 2 ) ) 
    return NULL;

  ASSERT(src->biWidth%2==0);
  ASSERT(src->biHeight%2==0);
  if ((src->biWidth & 1) || (src->biHeight & 1))
    return NULL ;

  DWORD newSize=src->biWidth*src->biHeight;
  DWORD OutWidth=src->biWidth;
  LPBITMAPINFOHEADER res;
  if (lpDst)
    res = (LPBITMAPINFOHEADER)lpDst;
  else
    res = (LPBITMAPINFOHEADER)malloc(src->biSize+newSize);
  memcpy(res,src,src->biSize);
  res->biCompression = BI_Y8;
  res->biSizeImage   = newSize ;
  res->biBitCount    = 8;
  res->biClrUsed     = 0;
  LPBYTE srcb = (lpData) ? lpData : ((LPBYTE)src) + src->biSize ;
  LPBYTE dstb = ((LPBYTE)res) + res->biSize ;
  LPBYTE eod = srcb + src->biWidth * src->biHeight ;
//   LPBYTE vsrc = eod ;
//   LPBYTE usrc = eod + (src->biWidth * src->biHeight) / 4 ;
  LPBYTE usrc = eod ;
  LPBYTE vsrc = eod + (src->biWidth * src->biHeight) / 4 ;
  int    u = (*usrc-128) << 13 ;
  int    v = (*vsrc-128) << 13 ;
  int   val = ( Color == Red ) ? v/7185 : 
    (Color == Green)? -(u/20687+v/14100) : u/4037 ;
  int    cWidth=src->biWidth/2;
  DWORD cntr=0;
  DWORD cntr2=0;
  int clrInfoOff=0;
  while (srcb<eod)
  {
    yuv_to_color( *(srcb++) , val , dstb++ );
    cntr++;
    yuv_to_color( *(srcb++) , val , dstb++ );
    cntr++;
    cntr2++ ;
    clrInfoOff = (cntr2 % cWidth) + (cntr2 / src->biWidth) * cWidth ;

    u=(*(usrc+clrInfoOff)-128)<<13;
    v=(*(vsrc+clrInfoOff)-128)<<13;
    val = ( Color == Red ) ? v/7185 : 
      (Color == Green)? -(u/20687+v/14100) : u/4037 ;
  }
  return res;
}


IMPLEMENT_RUNTIME_GADGET_EX(ColorExtractor, CFilterGadget, "Video.color&brightness", TVDB400_PLUGIN_NAME);

ColorExtractor::ColorExtractor(void):
    m_OutputColor( Red )
  , m_dContrast(1.0)
{
  m_MultyCoreAllowed=true;
  m_OutputMode = modeReplace;
  m_pInput = new CInputConnector(vframe);
  m_pOutput = new COutputConnector(vframe);
  Resume();
}

void ColorExtractor::ShutDown()
{
  //TODO: Add all destruction code here
  CFilterGadget::ShutDown();
  delete m_pInput;
  m_pInput = NULL;
  delete m_pOutput;
  m_pOutput = NULL;
}

CDataFrame* ColorExtractor::DoProcessing(const CDataFrame* pDataFrame)
{
  const CVideoFrame* vf=pDataFrame->GetVideoFrame(DEFAULT_LABEL);
  if (vf)
  {
    int extra=0;
    if ((vf->lpBMIH->biWidth*3)%4!=0)
    {
      extra=4-((vf->lpBMIH->biWidth*3)%4);
    }
    int bmSize=(vf->lpBMIH->biWidth + extra)*vf->lpBMIH->biHeight;

    pTVFrame retV=(pTVFrame)malloc(sizeof(TVFrame));
    if ( !retV )
      return NULL ;
    retV->lpData = NULL ;
    int iWidth = vf->lpBMIH->biWidth;
    int iHeight = vf->lpBMIH->biHeight;
    BYTE * pNewData = NULL ;
    if ( m_OutputColor != Red  &&  m_OutputColor != Green  && m_OutputColor != Blue )
    {
      retV->lpBMIH=(LPBITMAPINFOHEADER)malloc(sizeof(BITMAPINFOHEADER)+bmSize);
      if ( !retV->lpBMIH )
        return NULL ;
      memset(retV->lpBMIH , 0 , sizeof(BITMAPINFOHEADER) );
      pNewData = (BYTE*)&(retV->lpBMIH[1]) ;
      retV->lpData=NULL;
      retV->lpBMIH->biSize=sizeof(BITMAPINFOHEADER);
      retV->lpBMIH->biWidth  = iWidth ;
      retV->lpBMIH->biHeight = iHeight ;
      retV->lpBMIH->biPlanes = 1;
      retV->lpBMIH->biBitCount = 8;
      retV->lpBMIH->biCompression = BI_Y8;
      retV->lpBMIH->biSizeImage = bmSize ;
    }
    LPBYTE src = GetData(vf);
    LPWORD srcw = GetData16(vf);
    switch (vf->lpBMIH->biCompression)
    {
      case BI_Y8:
      case BI_Y800:
      {
        return CVideoFrame::Create(retV);
      }
    case BI_YUV9:
      {
        BYTE * pV = GetData( vf ) + iWidth * iHeight ;
        BYTE * pU = pV + (iWidth * iHeight)/16 ;
        switch( m_OutputColor )
        {
        case Red:
        case Green:
        case Blue:
          {
            retV->lpBMIH = ExtractFromYUV9( vf->lpBMIH , vf->lpData , m_OutputColor ) ;
            if ( retV->lpBMIH )
              return CVideoFrame::Create(retV);
          }
          break ;
        case U:
          for ( int iY = 0 ; iY < iHeight ; iY += 4 )
          {
            BYTE * pRow = pNewData + iWidth * iY ;
            for ( int iX = 0 ; iX < iWidth ; iX += 4 , pRow += 4 , pU++ )
            {
              double dVal = (( (double)*pU - 128 ) * m_dContrast) + 128 ;
              BYTE bOut = ( dVal > 255. )? 255 : (int)dVal ;
              memset( pRow , bOut , 4 ) ;
              memset( pRow + iWidth , bOut , 4 ) ;
              memset( pRow + iWidth * 2 , bOut , 4 ) ;
              memset( pRow + iWidth * 3 , bOut , 4 ) ;
            }
          }
          return CVideoFrame::Create(retV);
        case V:
          for ( int iY = 0 ; iY < iHeight ; iY += 4 )
          {
            BYTE * pRow = pNewData + iWidth * iY ;
            for ( int iX = 0 ; iX < iWidth ; iX += 4 , pRow += 4 , pV++ )
            {
              double dVal = (( (double)*pV - 128 ) * m_dContrast) + 128 ;
              BYTE bOut = ( dVal > 255. )? 255 : (int)dVal ;
              memset( pRow , bOut , 4 ) ;
              memset( pRow + iWidth , bOut , 4 ) ;
              memset( pRow + iWidth * 2 , bOut , 4 ) ;
              memset( pRow + iWidth * 3 , bOut , 4 ) ;
            }
          }
          return CVideoFrame::Create(retV);
        case U_DIV_V:
          for ( int iY = 0 ; iY < iHeight ; iY += 4 )
          {
            BYTE * pRow = pNewData + iWidth * iY ;
            for ( int iX = 0 ; iX < iWidth ; iX += 4 , pRow += 4 , pU++ , pV++ )
            {
              double dVal = m_dContrast * (double)*pU/(double)*pV ;
              BYTE bOut = ( dVal > 255. )? 255 : (int)dVal ;
              memset( pRow , bOut , 4 ) ;
              memset( pRow + iWidth , bOut , 4 ) ;
              memset( pRow + iWidth * 2 , bOut , 4 ) ;
              memset( pRow + iWidth * 3 , bOut , 4 ) ;
            }
          }
          return CVideoFrame::Create(retV);
        case V_DIV_U:
          for ( int iY = 0 ; iY < iHeight ; iY += 4 )
          {
            BYTE * pRow = pNewData + iWidth * iY ;
            for ( int iX = 0 ; iX < iWidth ; iX += 4 , pRow += 4 , pU++ , pV++ )
            {
              double dVal = m_dContrast * (double)*pV/(double)*pU ;
              BYTE bOut = ( dVal > 255. )? 255 : (int)dVal ;
              memset( pRow , bOut , 4 ) ;
              memset( pRow + iWidth , bOut , 4 ) ;
              memset( pRow + iWidth * 2 , bOut , 4 ) ;
              memset( pRow + iWidth * 3 , bOut , 4 ) ;
            }
          }
          return CVideoFrame::Create(retV);
        case Radius:
          for ( int iY = 0 ; iY < iHeight ; iY += 4 )
          {
            BYTE * pRow = pNewData + iWidth * iY ;
            for ( int iX = 0 ; iX < iWidth ; iX += 4 , pRow += 4 , pU++ , pV++ )
            {
              double dU = ((double)(*pU) - 128.)/128. ;
              double dV = ((double)(*pV) - 128.)/128. ;
              double dVal = m_dContrast * sqrt( dU * dU + dV * dV ) ;
              BYTE bOut = ( dVal > 255. )? 255 : (int)dVal ;
              memset( pRow , bOut , 4 ) ;
              memset( pRow + iWidth , bOut , 4 ) ;
              memset( pRow + iWidth * 2 , bOut , 4 ) ;
              memset( pRow + iWidth * 3 , bOut , 4 ) ;
            }
          }
          return CVideoFrame::Create(retV);
        }
      }
    case BI_YUV12:
      {
        BYTE * pV = GetData( vf ) + iWidth * iHeight ;
        BYTE * pU = pV + (iWidth * iHeight)/4 ;
        switch( m_OutputColor )
        {
        case Red:
        case Green:
        case Blue:
          {
            retV->lpBMIH = ExtractFromYUV12( vf->lpBMIH , vf->lpData , m_OutputColor ) ;
            if ( retV->lpBMIH )
              return CVideoFrame::Create(retV);
          }
          break ;
        case U:
          for ( int iY = 0 ; iY < iHeight ; iY += 2 )
          {
            BYTE * pRow = pNewData + iWidth * iY ;
            for ( int iX = 0 ; iX < iWidth ; iX += 2 , pRow += 2 , pU++ )
            {
              double dVal = (( (double)*pU - 128 ) * m_dContrast) + 128 ;
              BYTE bOut = ( dVal > 255. )? 255 : (int)dVal ;
              *pRow = bOut ;
              *(pRow + 1) = bOut ;
              *(pRow + iWidth) = bOut ;
              *(pRow + 1 + iWidth) = bOut ;
            }
          }
          return CVideoFrame::Create(retV);
        case V:
          for ( int iY = 0 ; iY < iHeight ; iY += 2 )
          {
            BYTE * pRow = pNewData + iWidth * iY ;
            for ( int iX = 0 ; iX < iWidth ; iX += 2 , pRow += 2 , pV++ )
            {
              double dVal = (( (double)*pV - 128 ) * m_dContrast) + 128 ;
              BYTE bOut = ( dVal > 255. )? 255 : (int)dVal ;
              *pRow = bOut ;
              *(pRow + 1) = bOut ;
              *(pRow + iWidth) = bOut ;
              *(pRow + 1 + iWidth) = bOut ;
            }
          }
          return CVideoFrame::Create(retV);
        case U_DIV_V:
          for ( int iY = 0 ; iY < iHeight ; iY += 2 )
          {
            BYTE * pRow = pNewData + iWidth * iY ;
            for ( int iX = 0 ; iX < iWidth ; iX += 2 , pRow += 2 , pU++ , pV++ )
            {
              double dVal = m_dContrast * (double)*pU/(double)*pV ;
              BYTE bOut = ( dVal > 255. )? 255 : (int)dVal ;
              *pRow = bOut ;
              *(pRow + 1) = bOut ;
              *(pRow + iWidth) = bOut ;
              *(pRow + 1 + iWidth) = bOut ;
            }
          }
          return CVideoFrame::Create(retV);
        case V_DIV_U:
          for ( int iY = 0 ; iY < iHeight ; iY += 2 )
          {
            BYTE * pRow = pNewData + iWidth * iY ;
            for ( int iX = 0 ; iX < iWidth ; iX += 2 , pRow += 2 , pU++ , pV++ )
            {
              double dVal = m_dContrast * (double)*pV/(double)*pU ;
              BYTE bOut = ( dVal > 255. )? 255 : (int)dVal ;
              *pRow = bOut ;
              *(pRow + 1) = bOut ;
              *(pRow + iWidth) = bOut ;
              *(pRow + 1 + iWidth) = bOut ;
            }
          }
          return CVideoFrame::Create(retV);
        case Radius:
          for ( int iY = 0 ; iY < iHeight ; iY += 2 )
          {
            BYTE * pRow = pNewData + iWidth * iY ;
            for ( int iX = 0 ; iX < iWidth ; iX += 2 , pRow += 2 , pU++ , pV++ )
            {
              double dU = (double)((char)(*pU))/128. ;
              double dV = (double)((char)(*pV))/128. ;
              double dVal = m_dContrast * sqrt( dU * dU + dV * dV ) ;
              BYTE bOut = ( dVal > 255. )? 255 : (int)dVal ;
              *pRow = bOut ;
              *(pRow + 1) = bOut ;
              *(pRow + iWidth) = bOut ;
              *(pRow + 1 + iWidth) = bOut ;
            }
          }
          return CVideoFrame::Create(retV);
        }
      }
    case BI_Y16:
      {
        return CVideoFrame::Create(retV);
      }
    case BI_RGB:
      {
        switch (vf->lpBMIH->biBitCount)
        {
        case 24:
          {
            return CVideoFrame::Create(retV);
          }
        }
        break ;
      }
    }
    if ( retV->lpBMIH )
      delete retV->lpBMIH ;
    delete retV ;
  }
  return NULL;
}

bool ColorExtractor::ScanProperties(LPCTSTR text, bool& Invalidate)
{
  CFilterGadget::ScanProperties(text,Invalidate);
  FXPropertyKit pk(text);
  pk.GetInt( "OutputMode" , (int&)m_OutputColor) ;
  pk.GetDouble( "Contrast" , m_dContrast ) ;
  return true;
}

bool ColorExtractor::PrintProperties(FXString& text)
{
  FXPropertyKit pk;
  CFilterGadget::PrintProperties(text);
  pk.WriteInt( "OutputMode" , (int)m_OutputColor) ;
  pk.WriteDouble( "Contrast" , m_dContrast ) ;
  text+=pk;
  return true;
}

bool ColorExtractor::ScanSettings(FXString& text)
{
  FXString param="template(ComboBox(OutputMode(Red(0),Green(1),Blue(2),U(3),V(4),U/V(5),V/U(6),Radius(7))),"
    "EditBox(Contrast))";
  text=param;
  return true;

  // Here should be commented samples for all types of  
  // dialog controls with short explanation


  //     text = "template(EditBox(Sigma_Pix),"
  //       "Spin(HalfWidth,3,40),"
  //       "Spin(MinRadius_pix,10,50),"
  //       "Spin(MaxRadius_pix,15,150),"
  //       "Spin(SZone,5,50),"
  //       "EditBox(Angle_Deg),"
  //       "Spin(AngleStep,1,10),"
  //       "EditBox(2ndThres),"
  //       "EditBox(Scale_nm),"
  //       "EditBox(MinSize_um),"
  //       "EditBox(MaxSize_um)," 
  //       "EditBox(SigSigma2),"
  //       "Spin(ViewMode,0,16))";
  //     return true;
}


