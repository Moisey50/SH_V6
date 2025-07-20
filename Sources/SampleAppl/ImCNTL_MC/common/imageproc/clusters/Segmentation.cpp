// SpotFinder1Doc.cpp : implementation of the CSpotFinder1Doc class
//

#include "stdafx.h"
#include <imageproc\clusters\segmentation.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


int CSegmentation::FindSpots( pTVFrame pFrame , BOOL bDetailed )
{
  if ( !pFrame  ||  !pFrame->lpBMIH )
    return 0 ;
  m_bDetailed = bDetailed != FALSE;
  m_Segm1.RemoveAll() ;
  m_ColSpots.RemoveAll() ;
  m_pBitBuf = GetData( pFrame ) ;
  DWORD Format = pFrame->lpBMIH->biCompression ;
  if ((Format!=BI_YUV9) && (Format!=BI_Y8)) 
  {
    if ( Format == BI_Y16 )
    {
      m_pWordBuf = (WORD*)m_pBitBuf ;
      m_pBitBuf = NULL ;
    }
    else
    {
#ifndef TVDB300_APPL
      SENDLOGMSG(MSG_INFO_LEVEL,"Segmentation",0 ,"Segmentation can only accept formats YUV9, Y8 and Y16");
#else
      TRACE("Segmentation can only accept formats YUV9, Y8 and Y16/n");
#endif
      return NULL;
    }
  }
  else
    m_pWordBuf = NULL ;
  m_pFrame = pFrame ;
  m_iHeight = abs(m_pFrame->lpBMIH->biHeight) ;
  m_iWidth = m_pFrame->lpBMIH->biWidth ;
  m_pCurr = &m_Segm1 ;
  if (!SimpleThres(0)) // some problem with parameters, like width...
    return 0 ;

  for (int iSegm = 0 ; iSegm < m_pCurr->GetSize() ; iSegm++ )
  {
    m_pCurr->ElementAt(iSegm).m_iCont = iSegm ;
    CColorSpot NewSpot( m_pCurr->ElementAt(iSegm) , iSegm , m_bDetailed ) ;
    m_ColSpots.Add(NewSpot) ;
  }
  m_pPrev = &m_Segm1 ;
  m_pCurr = &m_Segm2 ;

  for (int iY = 1 ; iY < m_iHeight ; iY++ )
  {
    m_pCurr->RemoveAll() ;
    if ( !SimpleThres(iY) )
      break ; //something strange

    for (int iSeg = 0 ; iSeg < m_pPrev->GetSize() ; iSeg++ )
      m_pPrev->ElementAt(iSeg).m_iMatched = 0 ; 
    CColorSpot * pSpot = &m_ColSpots[0] ;
    for (int iCont = 0 ; iCont < m_ColSpots.GetSize() ; iCont++ )
      pSpot[iCont].m_iMatched = 0 ;
    CSegment * pCurr = &(m_pCurr->ElementAt(0)) ;
    //CSegment * pCurrLast = pCurr + m_pCurr->GetUpperBound() ;
    int iCurrLen = m_pCurr->GetSize() ;
    CSegment * pPrev = &(m_pPrev->ElementAt(0)) ;
    int iPrevLen = m_pPrev->GetSize() ;
    int iPrev = 0 ;
    //CSegment * pPrevLast = pPrev + m_pPrev->GetUpperBound() ;
    for ( int iCurr=0 ; iCurr < iCurrLen ; iCurr++ )
    {
      while( ( iPrev < iPrevLen) 
          && (pPrev[iPrev].m_Segm.m_iB <= pCurr[iCurr].m_Segm.m_iE) )
      {
        if (pCurr[iCurr].Match(pPrev[iPrev]))
        {
          CColorSpot * pPrevSpot = &m_ColSpots[pPrev[iPrev].m_iCont] ;
          while (pPrevSpot->m_Area < 0)
          {
            ASSERT(pPrevSpot->m_iAddedTo != pPrevSpot->m_iContNum) ;
            pPrevSpot = &m_ColSpots[pPrevSpot->m_iAddedTo] ;
          }
          pCurr[iCurr].m_iCont = pPrevSpot->m_iContNum ;
          CColorSpot * pCurrSpot = &m_ColSpots[pCurr[iCurr].m_iCont] ;
          *pCurrSpot += pCurr[iCurr] ;
          pCurrSpot->m_iMatched++ ; // Contour has continuation
          pCurr[iCurr].m_iMatched++ ; // Segment is continuation
          if (pPrev[iPrev].m_Segm.m_iE > pCurr[iCurr].m_Segm.m_iE)
          {
            if (iCurr < iCurrLen - 1)
            {
              iCurr++ ;
              continue ;
            }
            else
             break ;
          }
          iPrev++ ;
          while (iPrev < iPrevLen)
          {
            if (pPrev[iPrev].m_Segm.m_iB > pCurr[iCurr].m_Segm.m_iE)
              break ;
            else
            {
              if ( (pCurr[iCurr].m_iColor == pPrev[iPrev].m_iColor)
                && (pCurr[iCurr].m_iCont != pPrev[iPrev].m_iCont) )
              {
                pPrevSpot = &m_ColSpots[pPrev[iPrev].m_iCont] ;
                while (pPrevSpot->m_Area < 0)
                {
                  ASSERT(pPrevSpot->m_iAddedTo != pPrevSpot->m_iContNum) ;
                  pPrevSpot = &m_ColSpots[pPrevSpot->m_iAddedTo] ;
                }
                *pCurrSpot += *pPrevSpot ;
              }
            }
            if (pPrev[iPrev].m_Segm.m_iE <= pCurr[iCurr].m_Segm.m_iE )
              iPrev++ ;
            else
              break ;
          }
        }
        else if (pPrev[iPrev].m_Segm.m_iE <= pCurr[iCurr].m_Segm.m_iE )
          iPrev++ ;
        else
          break ;
      }
    }
    for (int iCurrNew = 0 ; iCurrNew < iCurrLen ; iCurrNew++)
    {
      if (pCurr[iCurrNew].m_iMatched == 0)
      {
        int iContNum = pCurr[iCurrNew].m_iCont = m_ColSpots.GetUpperBound() + 1 ;
        if ( iContNum > m_iNMaxContours ) // too many conturs
          break ;
        CColorSpot NewSpot( pCurr[iCurrNew] , pCurr[iCurrNew].m_iCont , m_bDetailed ) ;
        m_ColSpots.Add(NewSpot) ;
      }
    }
    if (m_ColSpots.GetUpperBound() >= m_iNMaxContours)
      break ;
    SegmentArray * tmp = m_pCurr ;
    m_pCurr = m_pPrev ;
    m_pPrev = tmp ;
  }
  if ( m_ColSpots.GetSize() > m_iNMaxContours )
  {
    m_ColSpots.RemoveAll() ;
    return 0 ;
  }
  CColorSpot * pSpot = &m_ColSpots[0] ;
  for (int iCont = 0 ; iCont < m_ColSpots.GetSize() ; iCont++ )
    pSpot[iCont].EndSpot();

  return m_ColSpots.GetSize() ;
}

int CSegmentation::SimpleThres( int iY )
{
   if (iY >= m_iHeight)
     return 0 ;
  if ( m_pBitBuf ) 
  {
    BYTE * p = m_pBitBuf + (iY * m_iWidth ) ;
    int iIndex = 0 ;
    int iColor = ( *p >= m_MinColor ) && ( *p <= m_MaxColor ) ;
    int iNewColor ;
    while (iIndex < m_iWidth) 
    {
      int iB = iIndex ;
      int iMaxPix = *p ;
      while ( (++iIndex < m_iWidth) )
      {
        BYTE bVal = *(++p) ;
        if ( bVal > iMaxPix )
          iMaxPix = bVal ;
        iNewColor = ( bVal >= m_MinColor ) && ( bVal <= m_MaxColor ) ;
        if ( iNewColor != iColor )
          break ;
      } ;
      CSegment NewSegment( iY , iColor , iB , iIndex - 1 ) ;
      NewSegment.m_iMaxPixel = iMaxPix ;
      m_pCurr->Add( NewSegment ) ;
      iColor = iNewColor ;
    }
  }
  else if ( m_pWordBuf )
  {
    WORD * p = m_pWordBuf + (iY * m_iWidth ) ;
    int iIndex = 0 ;
    int iColor = ( *p >= m_wMinColor ) && ( *p <= m_wMaxColor ) ;
    int iNewColor ;
    while (iIndex < m_iWidth) 
    {
      int iB = iIndex ;
      int iMaxPix = *p ;
      while ( (++iIndex < m_iWidth) )
      {
        WORD wVal = *(++p) ;
        if ( wVal > iMaxPix )
          iMaxPix = wVal ;
        iNewColor = ( wVal >= m_wMinColor ) && ( wVal <= m_wMaxColor ) ;
        if ( iNewColor != iColor )
          break ;
      } ;
      CSegment NewSegment( iY , iColor , iB , iIndex - 1 ) ;
      NewSegment.m_iMaxPixel = iMaxPix ;
      m_pCurr->Add( NewSegment ) ;
      iColor = iNewColor ;
    }
  }
  else
    return 0 ;
  return m_pCurr->GetSize() ;
}
