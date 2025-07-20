// SpotFinder1Doc.cpp : implementation of the CSpotFinder1Doc class
//

#include "stdafx.h"
// #include "math\intf_sup.h"  
#include <imageproc\clusters\segmentation.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


int CSegmentation::FindSpots( pTVFrame pFrame , int iDetailed_mask )
{
  if (!pFrame || !pFrame->lpBMIH)
    return 0 ;
  double dTimePt = m_dStartTime = GetHRTickCount() ;
  m_Status[ 0 ] = 0 ;
  m_WhatToMeasure = iDetailed_mask ;
  CalcImageMoments CalcMoments =
    ( m_WhatToMeasure & MEASURE_IMG_MOMENTS_W ) ? CalcWeighted :
    ( m_WhatToMeasure & MEASURE_IMG_MOMENTS_NW ) ? CalcNotWeighted : NoImageMoments ;
  m_Segm1.SetSize( 0 , 400 ) ;
  m_ColSpots.RemoveAll() ;
  m_ColSpots.SetSize( 0 , 7000 ) ;
  m_pBitBuf = GetData( pFrame ) ;
  DWORD Format = pFrame->lpBMIH->biCompression ;
  switch (Format)
  {
    case BI_Y8:
    case BI_YUV9:
    case BI_YUV12:
    case BI_Y800:
      m_pWordBuf = NULL ;
      break ;
    case BI_Y16:
      m_pWordBuf = ( WORD* ) m_pBitBuf ;
      m_pBitBuf = NULL ;
      break ;
    default:
      {
#ifndef TVDB300_APPL
        SENDLOGMSG( MSG_INFO_LEVEL , "Segmentation" , 0 , "Segmentation can only accept formats YUV9, Y8 and Y16" );
#else
        TRACE( "Segmentation can only accept formats YUV9, Y8 and Y16/n" );
#endif
        return NULL;
      }
  }
  m_pFrame = pFrame ;
  m_iHeight = abs( m_pFrame->lpBMIH->biHeight ) ;
  m_iWidth = m_pFrame->lpBMIH->biWidth ;
  m_Segm1.SetSize( 0 , 400 ) ;
  m_Segm2.SetSize( 0 , 400 ) ;
  m_pCurr = &m_Segm1 ;
  if (!SimpleThres( 0 , CalcMoments )) // some problem with parameters, like width...
    return 0 ;
  for (int iSegm = 0 ; iSegm < m_pCurr->GetSize() ; iSegm++)
  {
    CSegment& Segment = m_pCurr->ElementAt( iSegm ) ;
    Segment.m_iCont = iSegm ;
    CColorSpot NewSpot( Segment , iSegm , m_WhatToMeasure ) ;
    if (Segment.m_Segm.m_iB > 0)
      NewSpot.m_iBefore = GetPixel( pFrame , Segment.m_Segm.m_iB - 1 , Segment.m_iY ) ;
    NewSpot.m_iAfter = GetPixel( pFrame , Segment.m_Segm.m_iB , Segment.m_iY ) ;
    m_ColSpots.Add( NewSpot ) ;
  }
  IntVector Runs ;
  Runs.push_back( ( int ) m_pCurr->GetCount() ) ;
  DoubleVector Times ;
  double dTimePoint = GetHRTickCount() ;
  Times.push_back( dTimePoint - dTimePt ) ;
  dTimePt = dTimePoint ;

  m_pPrev = &m_Segm1 ;
  m_pCurr = &m_Segm2 ;

  for (int iY = 1 ; iY < m_iHeight ; iY++)
  {
    //m_pCurr->SetSize( 0 , 400 ) ;
    m_pCurr->ResetLength() ;
    if (!SimpleThres( iY , CalcMoments ))
      break ; //something strange

    Runs.push_back( ( int ) m_pCurr->GetCount() ) ;

    for (int iSeg = 0 ; iSeg < m_pPrev->GetSize() ; iSeg++)
      m_pPrev->ElementAt( iSeg ).m_iMatched = 0 ;
    CColorSpot * pSpot = &m_ColSpots[ 0 ] ;
    for (int iCont = 0 ; iCont < m_ColSpots.GetSize() ; iCont++)
      pSpot[ iCont ].m_iMatched = 0 ;
    //CSegment * pCurrLast = pCurr + m_pCurr->GetUpperBound() ;
    int iCurrLen = ( int ) m_pCurr->GetSize() ;
    int iPrevLen = ( int ) m_pPrev->GetSize() ;
    CSegment * pPrev = &( m_pPrev->ElementAt( 0 ) ) ;
    CSegment * pPrevEnd = pPrev + iPrevLen ;
    CSegment * pCurrEnd = m_pCurr->GetData() + m_pCurr->GetCount() ;
    int iLastAdded = -1 ;
    //CSegment * pPrevLast = pPrev + m_pPrev->GetUpperBound() ;
    for (int iCurr = 0 ; iCurr < iCurrLen ; iCurr++)
    {
      CSegment * pCurr = m_pCurr->GetData() + iCurr ;
      while (( pPrev < pPrevEnd ) // iPrev/pPrev cycle
        && ( pPrev->m_Segm.m_iB <= pCurr->m_Segm.m_iE ) )
      {
        if  (pCurr->Match( *pPrev )) // prev and current segments are overlapped
        {
          CColorSpot * pPrevSpot = &m_ColSpots[ pPrev->m_iCont ] ;
          while (pPrevSpot->m_Area < 0)
          {
            ASSERT( pPrevSpot->m_iAddedTo != pPrevSpot->m_iContNum ) ;
            pPrevSpot = &m_ColSpots[ pPrevSpot->m_iAddedTo ] ;
          }
          if ( (pCurr->m_iCont < 0) // current segment is not assigned to spot
            || (pCurr->m_iCont > pPrevSpot->m_iContNum)) // is uniton necessary (?)
          {
            pCurr->m_iCont = pPrevSpot->m_iContNum ; // union of current and previous
          }
          CColorSpot * pCurrSpot = &m_ColSpots[ pCurr->m_iCont ] ;
          if (iLastAdded < iCurr)
          {
            *pCurrSpot += *pCurr ; // add segment to spot
            iLastAdded = iCurr ;
          }
          //           else
          //             ASSERT( 0 ) ;

          pCurrSpot->m_iMatched++ ; // Contour has continuation
          pCurr->m_iMatched++ ; // Mark Segment as continuation

          if (pPrev->m_Segm.m_iE > pCurr->m_Segm.m_iE)
          {
            if (iCurr < iCurrLen - 1)
            {
              iCurr++ ;
              pCurr++ ;
              continue ; // while by iPrev/pPrev
            }
            else
              break ;
          }
          pPrev++ ;
          while (pPrev < pPrevEnd)
          {
            if (pPrev->m_Segm.m_iB > pCurr->m_Segm.m_iE) // prev ends after current ends
              break ;
            else
            {
              if (( pCurr->m_iColor == pPrev->m_iColor )  // colors are the same
                && ( pCurr->m_iCont != pPrev->m_iCont ))  // but spots are different
              {
                pPrevSpot = &m_ColSpots[ pPrev->m_iCont ] ;
                while (pPrevSpot->m_Area < 0)
                {
                  ASSERT( pPrevSpot->m_iAddedTo != pPrevSpot->m_iContNum ) ;
                  pPrevSpot = &m_ColSpots[ pPrevSpot->m_iAddedTo ] ;
                }
                if ((pPrevSpot->m_iContNum != pCurrSpot->m_iContNum) && (pCurrSpot->m_Area > 0) )
                {
                  if ( pPrevSpot->m_iContNum < pCurrSpot->m_iContNum )
                  {
                    *pPrevSpot += *pCurrSpot ; // add spot of current to current previous spot
                    ASSERT( pCurrSpot->m_Area < 0 && pCurrSpot->m_iAddedTo >= 0 ) ;
                  }
                  else
                  {
                    *pCurrSpot += *pPrevSpot ; // add spot of previous to current spot
                    ASSERT( pPrevSpot->m_Area < 0 && pPrevSpot->m_iAddedTo >= 0 ) ;
                  }
                }
              }
            }
            if (pPrev->m_Segm.m_iE <= pCurr->m_Segm.m_iE)
              pPrev++ ; // there is case, when previous segment added to current (union)
            else
              break ;
          }
        }
        else if (pPrev->m_Segm.m_iE <= pCurr->m_Segm.m_iE) // no matching
          pPrev++ ;                                  // go to analyze next previous segment
        else
          break ;
      }
    } // matching between previous segments and current is finished
    
    // Do mark segments which are not marked as matched and create new spots
    for (int iCurrNew = 0 ; iCurrNew < iCurrLen ; iCurrNew++)
    {
      CSegment * pCurr = m_pCurr->GetData() + iCurrNew ;
      if ( !pCurr->m_iMatched )
      {
        int iContNum = pCurr->m_iCont = ( int ) m_ColSpots.GetUpperBound() + 1 ;
        if (iContNum > m_iNMaxContours) // too many contours
          break ;
        CColorSpot NewSpot( *pCurr , pCurr->m_iCont , m_WhatToMeasure ) ;
        if (pCurr->m_Segm.m_iB > 0)
        {
          NewSpot.m_iBefore = GetPixel( pFrame ,
            pCurr->m_Segm.m_iB - 1 , pCurr->m_iY ) ;
        }
        NewSpot.m_iAfter = GetPixel( pFrame , pCurr->m_Segm.m_iB , pCurr->m_iY ) ;
        m_ColSpots.Add( NewSpot ) ;
      }
    }
    if (m_ColSpots.GetUpperBound() >= m_iNMaxContours)
    {
      sprintf_s( m_Status , "Too many spots %d Y=%d(%d)" ,
        ( int ) m_ColSpots.GetCount() , iY , m_iHeight ) ;
      m_ColSpots.RemoveAll() ;
      return 0 ;
    }
    if (IsTimeout())
    {  // Timeout
      sprintf_s( m_Status , "Segmentation timeout %.1f Nspots=%d Y=%d(%d)" ,
        GetWorkingTime() , ( int ) m_ColSpots.GetCount() , iY , m_iHeight ) ;
      m_ColSpots.RemoveAll() ;
      return 0 ;
    }
    SegmentArray * tmp = m_pCurr ;
    m_pCurr = m_pPrev ;
    m_pPrev = tmp ;

    double dTimePoint = GetHRTickCount() ;
    Times.push_back( dTimePoint - dTimePt ) ;
    dTimePt = dTimePoint ;
  }
  CColorSpot * pSpot = &m_ColSpots[ 0 ] ;

  // Do finish all not finished spots (continued till bottom edge)
  for (int iCont = 0 ; iCont < m_ColSpots.GetSize() ; iCont++)
  {
    CColorSpot& Spot = m_ColSpots.GetAt( iCont ) ;

    Spot.EndSpot( pFrame );
  }

  return ( int ) m_ColSpots.GetSize() ;
}

int CSegmentation::SimpleThres( int iY , CalcImageMoments CalcMoments )
{
  if (iY < 0 || iY >= m_iHeight
    || ( !m_pBitBuf && !m_pWordBuf ))
  {
    ASSERT( 0 ) ;
    return 0 ;
  }
  ImgMoments Moments ;

  BYTE * pByteBuf = ( m_pBitBuf ) ? m_pBitBuf + ( iY * m_iWidth ) : NULL ;
  WORD * pWordBuf = ( m_pWordBuf ) ? m_pWordBuf + ( iY * m_iWidth ) : NULL ;
  int iValue = ( pByteBuf ) ? *pByteBuf : *pWordBuf ;
  int iIndex = 0 ;
  int iColor = ( iValue >= m_MinColor ) && ( iValue <= m_MaxColor ) ;
  int iNewColor ;
  int iVal = ( pByteBuf ) ? *pByteBuf : *pWordBuf ;
  int iMaxPix = iVal ;
  int iMinPix = iVal ;
  while (iIndex < m_iWidth)
  {
    int iB = iIndex ;
//     if (CalcMoments != NoImageMoments)
//       Moments.Reset() ;
    do
    {
      SetMinMax( iVal , iMinPix , iMaxPix ) ;
//       if (CalcMoments == CalcWeighted)  // process image moments with brightness account
//         Moments.Add( iIndex , iY , ( double ) iVal ) ;
      if (++iIndex >= m_iWidth)
        break ; // next pixel is out of width
      if (pByteBuf)
      {
        iVal = *( ++pByteBuf ) ;
        iNewColor = ( iVal >= m_MinColor ) && ( iVal <= m_MaxColor ) ;
      }
      else
      {
        iVal = *( ++pWordBuf ) ;
        iNewColor = ( iVal >= m_wMinColor ) && ( iVal <= m_wMaxColor ) ;
      }
    } while (iNewColor == iColor);
    CSegment NewSegment( iY , iColor , iB , iIndex - 1 , iMaxPix , iMinPix ) ;
    switch (CalcMoments)
    {
      case NoImageMoments:  break ;
      case CalcNotWeighted: NewSegment.FormImgMoments() ;  break ;
      case CalcWeighted: 
        if ( m_pBitBuf )
          NewSegment.FormImgMoments( &NewSegment.m_ImgMoments , 
            m_pBitBuf + (iY * m_iWidth) + iB , // pointer to segment begin
            (iColor == 0) ? 255. : 0. ) ;  // if segment is black, brightness will be inverted
        else if ( m_pWordBuf )
          NewSegment.FormImgMoments( &NewSegment.m_ImgMoments ,
            m_pWordBuf + (iY * m_iWidth) + iB , // pointer to segment begin
            ( iColor == 0 ) ? 65535. : 0. ) ;  // if segment is black, brightness will be inverted
        break ;
    }
//     if (CalcMoments == CalcNotWeighted)
//       Moments.Add( iB , iIndex - 1 , iY ) ;
//     else if (CalcMoments != NoImageMoments)
//     {
//       NewSegment.m_ImgMoments = Moments ;
//       NewSegment.m_ImgMoments.m_dValMin = iMinPix ;
//       NewSegment.m_ImgMoments.m_dValMax = iMaxPix ;
//     }
    m_pCurr->Add( NewSegment ) ;
    iColor = iNewColor ;
    iMaxPix = iMinPix = iVal ;
  }
  return ( int ) m_pCurr->GetSize() ;
}
void CSegmentation::MeasureSpotSize( CColorSpot& Spot )
{
  if (!m_pBitBuf && !m_pWordBuf)
  {
    ASSERT( 0 ) ;
    return ;
  }
  // Find upper side
  double dUpper = 0. ;

  if (Spot.m_OuterFrame.top > 0) // spot is not on the upper edge
  {
    int iPixelsToBegin = Spot.m_OuterFrame.top * m_iWidth
      + Spot.m_OuterFrame.left ;
    LPBYTE pData = m_pBitBuf ? m_pBitBuf + iPixelsToBegin : NULL ;
    LPWORD pWordData = m_pWordBuf ? m_pWordBuf + iPixelsToBegin : NULL ;
    for (int i = Spot.m_OuterFrame.left ; i < Spot.m_OuterFrame.right ; i++)
    {
      int iUpperValue = pData ? *( pData - m_iWidth ) : *( pWordData - m_iWidth ) ;
      int iValue = pData ? *( pData++ ) : *( pWordData++ ) ;
      if (iUpperValue == iValue)
        continue ;
      int iUpperColor = ( iUpperValue >= m_MinColor ) && ( iUpperValue <= m_MaxColor ) ;
      int iColor = ( iValue >= m_MinColor ) && ( iValue <= m_MaxColor ) ;
      if (iUpperColor != iColor)
      {  // single threshold method
        double dAddition = Spot.m_iColor ?
          ( double ) ( iValue - m_MinColor ) / ( double ) ( iValue - iUpperValue )
          :
          ( double ) ( m_MinColor - iValue ) / ( double ) ( iUpperValue - iValue ) ;
        if (dUpper < dAddition)
          dUpper = dAddition ;
      }
    }
    dUpper = ( double ) Spot.m_OuterFrame.top - dUpper ;
  }
  else
    dUpper = 0. ;

  //Find Lower side
  double dLower = 0. ;

  if (Spot.m_OuterFrame.bottom < m_iHeight - 1) // spot is not on the bottom edge
  {
    int iPixelsToBegin = Spot.m_OuterFrame.bottom * m_iWidth
      + Spot.m_OuterFrame.left ;
    LPBYTE pData = m_pBitBuf ? m_pBitBuf + iPixelsToBegin : NULL ;
    LPWORD pWordData = m_pWordBuf ? m_pWordBuf + iPixelsToBegin : NULL ;
    for (int i = Spot.m_OuterFrame.left ; i < Spot.m_OuterFrame.right ; i++)
    {
      int iLowerValue = pData ? *( pData + m_iWidth ) : *( pWordData + m_iWidth ) ;
      int iValue = pData ? *( pData++ ) : *( pWordData++ ) ;
      if (iLowerValue == iValue)
        continue ;
      int iLowerColor = ( iLowerValue >= m_MinColor ) && ( iLowerValue <= m_MaxColor ) ;
      int iColor = ( iValue >= m_MinColor ) && ( iValue <= m_MaxColor ) ;
      if (iLowerColor != iColor)
      {  // single threshold method
        double dAddition = Spot.m_iColor ?
          ( double ) ( iValue - m_MinColor ) / ( double ) ( iValue - iLowerValue )
          :
          ( double ) ( m_MinColor - iValue ) / ( double ) ( iLowerColor - iValue ) ;
        if (dLower < dAddition)
          dLower = dAddition ;
      }
    }
    dLower += ( double ) Spot.m_OuterFrame.bottom ;
  }
  else
    dLower = ( double ) m_iHeight - 1. ;

  // Find left side
  double dLeft = 0. ;

  if (Spot.m_OuterFrame.left > 0) // spot is not on the left edge
  {
    int iPixelsToBegin = Spot.m_OuterFrame.top * m_iWidth
      + Spot.m_OuterFrame.left ;
    LPBYTE pData = m_pBitBuf ? m_pBitBuf + iPixelsToBegin : NULL ;
    LPWORD pWordData = m_pWordBuf ? m_pWordBuf + iPixelsToBegin : NULL ;
    for (int i = Spot.m_OuterFrame.top ; i < Spot.m_OuterFrame.bottom ; i++)
    {
      int iLeftValue = pData ? *( pData - 1 ) : *( pWordData - 1 ) ;
      int iValue = pData ? *( pData ) : *( pWordData ) ;
      if (pData)
        pData += m_iWidth ;
      else
        pWordData += m_iWidth ;

      if (iLeftValue == iValue)
        continue ;
      int iUpperColor = ( iLeftValue >= m_MinColor ) && ( iLeftValue <= m_MaxColor ) ;
      int iColor = ( iValue >= m_MinColor ) && ( iValue <= m_MaxColor ) ;
      if (iUpperColor != iColor)
      {  // single threshold method
        double dAddition = Spot.m_iColor ?
          ( double ) ( iValue - m_MinColor ) / ( double ) ( iValue - iLeftValue )
          :
          ( double ) ( m_MinColor - iValue ) / ( double ) ( iLeftValue - iValue ) ;
        if (dLeft < dAddition)
          dLeft = dAddition ;
      }
    }
    dLeft = ( double ) Spot.m_OuterFrame.left - dLeft ;
  }
  else
    dLeft = 0. ;

  // Find left side
  double dRight = 0. ;

  if (Spot.m_OuterFrame.right < m_iWidth - 1) // spot is not on the right edge
  {
    int iPixelsToBegin = Spot.m_OuterFrame.top * m_iWidth
      + Spot.m_OuterFrame.right ;
    LPBYTE pData = m_pBitBuf ? m_pBitBuf + iPixelsToBegin : NULL ;
    LPWORD pWordData = m_pWordBuf ? m_pWordBuf + iPixelsToBegin : NULL ;
    for (int i = Spot.m_OuterFrame.top ; i < Spot.m_OuterFrame.bottom ; i++)
    {
      int iRightValue = pData ? *( pData + 1 ) : *( pWordData + 1 ) ;
      int iValue = pData ? *( pData ) : *( pWordData ) ;
      if (pData)
        pData += m_iWidth ;
      else
        pWordData += m_iWidth ;

      if (iRightValue == iValue)
        continue ;
      int iUpperColor = ( iRightValue >= m_MinColor ) && ( iRightValue <= m_MaxColor ) ;
      int iColor = ( iValue >= m_MinColor ) && ( iValue <= m_MaxColor ) ;
      if (iUpperColor != iColor)
      {  // single threshold method
        double dAddition = Spot.m_iColor ?
          ( double ) ( iValue - m_MinColor ) / ( double ) ( iValue - iRightValue )
          :
          ( double ) ( m_MinColor - iValue ) / ( double ) ( iRightValue - iValue ) ;
        if (dRight < dAddition)
          dRight = dAddition ;
      }
    }
    dRight += ( double ) Spot.m_OuterFrame.right ;
  }
  else
    dRight = ( double ) m_iWidth - 1. ;

  Spot.m_dBlobWidth = dRight - dLeft ;
  Spot.m_dBlobHeigth = dLower - dUpper ;
  Spot.m_WhatIsMeasured |= MEASURE_SUBPIXEL ;

  return ;
}
