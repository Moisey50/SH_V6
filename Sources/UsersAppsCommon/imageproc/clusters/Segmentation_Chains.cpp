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

  ChainsSet ActiveChains ;
  
  for (int iSegm = 0 ; iSegm < m_pCurr->GetSize() ; iSegm++ )
  {
    CSegment& Segm = m_pCurr->ElementAt(iSegm) ; 
    Segm.m_iCont = iSegm ;
    CColorSpot NewSpot( Segm , iSegm , m_bDetailed ) ;
    Chain * pNewChainLeft = new Chain( Segm.m_Segm.m_iE , true , iSegm ) ;
    pNewChainLeft->Add( CDPoint( Segm.m_Segm.m_iB , Segm.m_iY ) ) ;
    ActiveChains.Add( pNewChainLeft ) ;
    Chain * pNewChainRight = new Chain( Segm.m_Segm.m_iE , false , iSegm ) ;
    ActiveChains.Add( pNewChainRight ) ;
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
    int iChainIndex = 0 ;
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
          pPrev[iPrev].m_iMatched++ ; // segment is continued
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
      // Check for not matched new segments - they are beginning of new contours
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
      // Check for not matched previous segments, necessary to close contours
//     for ( iPrev = 0 ; iPrev < iPrevLen ; iPrev++ )
//     {
//     }
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

int CSegmentation::FindSpotsByChains( pTVFrame pFrame , BOOL bDetailed )
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

  ChainsSet ActiveChains ;

  for (int iSegm = 0 ; iSegm < m_pCurr->GetSize() ; iSegm++ )
  {
    CSegment& Segm = m_pCurr->ElementAt(iSegm) ; 
    Segm.m_iCont = iSegm ;
    CColorSpot NewSpot( Segm , iSegm , m_bDetailed ) ;
    Chain * pNewChainLeft = new Chain( Segm.m_Segm.m_iE , true , iSegm ) ;
    pNewChainLeft->Add( CDPoint( Segm.m_Segm.m_iB , Segm.m_iY - 0.5 ) ) ;
    ActiveChains.Add( pNewChainLeft ) ;
    Chain * pNewChainRight = new Chain( Segm.m_Segm.m_iE , false , iSegm ) ;
    pNewChainLeft->Add( CDPoint( Segm.m_Segm.m_iE + 0.5 , Segm.m_iY ) ) ;
    ActiveChains.Add( pNewChainRight ) ;
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
    int iChainIndex = 0 ;
    //CSegment * pPrevLast = pPrev + m_pPrev->GetUpperBound() ;
    for ( int iCurr=0 ; iCurr < iCurrLen ; iCurr++ )
    {
       // close contours before analyzed segment
      while ( ActiveChains.GetCount()
            && (pCurr[ iCurr ].m_Segm.m_iB  - ActiveChains[iChainIndex]->GetLastPt().x > 0.5) 
            )
      {   // two chains (left and right) is on the left side of next segment
         if (pCurr[ iCurr ].m_Segm.m_iB  - ActiveChains[iChainIndex + 1]->GetLastPt().x > 0.5)
         {        // second chan also on the left side
          // add points from second (right) chain in reverse order
          // after addition this chain will be deleted
         int iAfterRevAdded = ActiveChains[iChainIndex]->AddReverseChain( ActiveChains[iChainIndex+1] ) ;
           if ( ActiveChains[iChainIndex]->m_InitialPoint == ActiveChains[iChainIndex+1]->m_InitialPoint )
           {  // contour closed
         delete ActiveChains[ iChainIndex + 1 ] ; // delete copied right chain 
                // Add closed contour to correspondent spot  
             m_ColSpots[ActiveChains[iChainIndex].m_iProperty].m_BorderChains.Add(ActiveChains[iChainIndex]) ;
             ActiveChains.RemoveAt( iChainIndex , 2 ) ; // remove two chains from active chains
           }
           else // not the same contour (initial point)
           {

           }
         }
         int iAfterForvAdded = 0 ;
         for ( int iChain = iChainIndex + 2 ; iChain < ActiveChains.GetCount() ; iChain += 2 )
         {
             // if we find chain from the same spot, we have to add this chain also
             // after copying this chain will be deleted 
           if ( ActiveChains[iChain]->m_iProperty == ActiveChains[iChainIndex]->m_iProperty )
           {
             iAfterForvAdded = ActiveChains[iChainIndex]->AddChain( ActiveChains[iChain] ) ;
             m_ColSpots[ ActiveChains[iChainIndex]->m_iProperty ].m_BorderChains.Add( ActiveChains[iChainIndex] ) ;
                // delete copied chain
             delete ActiveChains[ iChain ] ;
                // original chain should be settled on it's place (only pointer 
                // is copied)
             ActiveChains.SetAt( iChain , ActiveChains[iChainIndex ] ) ;
             break ;
           }
         }
            // remove 2 pointers from chains array
         ActiveChains.RemoveAt( iChainIndex , 2 ) ;
           // iChainindex here points on next chain
      }
      
      while ( pCurr[iCurr].m_Segm.m_iE 
        > ActiveChains[iChainIndex]->GetLastPt().x )
      {
        ActiveChains[iChainIndex]->Add( CDPoint( pCurr[iCurr].m_Segm.m_iB , iY ) ) ;
        for ( int iChain = iChainIndex + 2 ; 
          iChain < ActiveChains.GetUpperBound() ; iChain += 2 )
        {
          if ( ActiveChains[iChain]->GetLastPt().x <= pCurr[iCurr].m_Segm.m_iE ) 
          {
              // Is chain for the same spot number?
            if ( ActiveChains[iChain]->m_iProperty == ActiveChains[iChainIndex]->m_iProperty )
            {  // Yes, we are closing internal contour inside spot (hole)
               // Add point from segment under last right chain point
               // And add point from segment under last next chain point
                // Add whole next chain in reverse order
              ActiveChains[ iChainIndex + 1 ]->AddClosingPtsAndReverseChain(
                ActiveChains[ iChain ] , iY ) ;
                // Now we have full contour, do add this contour (chain) to spot information
              m_ColSpots[ ActiveChains[ iChainIndex + 1]->m_iProperty ].m_BorderChains.Add( 
                ActiveChains[ iChainIndex + 1 ] ) ;
                // two chains are finished, delete them from chain array
              delete ActiveChains[ iChainIndex + 1 ] ;
              delete ActiveChains[ iChainIndex + 2 ] ;
              ActiveChains.RemoveAt( iChainIndex + 1 , 2 ) ;
              iChainIndex -= 2 ;
            }
            else // no, different spot; it means, that we have two contours merging 
            {
              CColorSpot * pPrevSpot = &m_ColSpots[ActiveChains[ iChain ]->m_iProperty ] ;
              while (pPrevSpot->m_Area < 0)
              {
                ASSERT(pPrevSpot->m_iAddedTo != pPrevSpot->m_iContNum) ;
                pPrevSpot = &m_ColSpots[pPrevSpot->m_iAddedTo] ;
              }
              pCurr[iCurr].m_iCont = pPrevSpot->m_iContNum ;
              ActiveChains[ iChainIndex + 1 ]->AddClosingPtsAndReverseChain(
                ActiveChains[ iChain ] , iY ) ;  
              int iFindRight = iChain + 1 ;

              for (  ; iFindRight < ActiveChains.GetUpperBound() ; iFindRight += 2 )
              {
                if ( ActiveChains[iChain]->m_iProperty == ActiveChains[iFindRight]->m_iProperty )
                {
//                   ActiveChains[ iChainIndex + 1 ]->AddChain( ActiveChains[iFindRight] ) ;
//                   delete ActiveChains[ iFindRight ] ;
//                   ActiveChains[ iChainIndex + 1 ]->                                 
                }
              }
              delete ActiveChains[ iChain ] ;
              ActiveChains.RemoveAt( iFindRight ) ;
              ActiveChains.RemoveAt( iChain ) ;
            }
          }
        }
      }
      
      
      
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
          pPrev[iPrev].m_iMatched++ ; // segment is continued

//           if (  )
//           {
//             pCurrSpot->m_BorderChains[0]
//             ->Add( CDPoint( pCurr[iCurr].m_Segm.m_iB , pCurr[iCurr].m_iY ) ) ;
//           }
//           else
//           {
// 
//           }

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
    // Check for not matched new segments - they are beginning of new contours
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
    // Check for not matched previous segments, necessary to close contours
    for ( iPrev = 0 ; iPrev < iPrevLen ; iPrev++ )
    {
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
