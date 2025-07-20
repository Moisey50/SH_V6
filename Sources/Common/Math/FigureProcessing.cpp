// FigureProcessing.cpp - functions for on plane operations
#include <stdafx.h>
#include <Math/FigureProcessing.h>
#include <math/FStraightLineRegression.h>

void LogFigure( LPCTSTR pFigName , FXSIZE iOper , FXSIZE iString , FXSIZE iNRef )
{
  //   FXString Msg;
  //   LPCTSTR pCase = NULL;
  //   switch ( iOper )
  //   {
  //   case Fig_Create: pCase = "Create"; break;
  //   case Fig_Release: pCase = "Release"; break;
  //   case Fig_AddRef: pCase = "AddRef"; break;
  //   case Fig_Touch: pCase = "Touched"; break;
  //   default: pCase = "Unknown"; break;
  //   }
  //   Msg.Format("%20s %s %d     %d    %s\n", pFigName,
  //     (iOper == 1) ? "Create" : ((iOper == 2) ? "Release" : "AddRef"),
  //     iNRef, iString , GetTimeAsString_ms() );
  //   FILE *fa = NULL;
  // 
  //   errno_t err = _tfopen_s(&fa, "FigLog.txt" , _T("a+"));
  //   if (err == 0)
  //   {
  //     int res = fseek(fa, 0, SEEK_END);
  //     LPCTSTR pString = Msg.GetBuffer();
  //     int iLen = Msg.GetLength();
  //     FXSIZE iNWritten = fwrite(pString, 1, iLen, fa);
  //     Msg.ReleaseBuffer();
  //     ASSERT(iNWritten == iLen);
  //     fclose(fa);
  //   }

  //   else
  //     ASSERT(0);
}


// THIS IS NOT FINISHED
int RemoveSmallOutGrowths( CFigureFrame& Result ,
  int iStep , double dMaxAngle , int iLongJump  )
{
  int iNRemoved = 0 ;
  int iLen = (int)Result.Count() ;
  
  cmplx * pContur = (cmplx*)Result.GetData() ;
  cmplx c1st = GetFigurePt( Result , 0 ) ;
  cmplx cCent = GetFigurePt( Result , iStep ) ;
  cmplx c2nd = GetFigurePt( Result , iStep * 2 ) ;
  for ( int i = 0 ; i < (int) Result.Count() ; i++ )
  {
    int iCentIndex = i + iStep ;
    cmplx c1st = GetFigurePt( Result , i ) ;
    cmplx cCent = GetFigurePt( Result , iCentIndex ) ;
    cmplx c2nd = GetFigurePt( Result , iCentIndex + iStep ) ;
    cmplx cVect1 = cCent - c1st ;
    cmplx cVect2 = c2nd - cCent ;
    double dAngle = arg( cVect2 / cVect1 ) ;
    if ( abs( dAngle ) > dMaxAngle ) 
    {
      cmplx * pcIterPlus = pContur + iCentIndex ;
      cmplx * pcIterMinus = pcIterPlus + iLongJump ;
      double dDist = abs( *pcIterMinus - cCent ) ;
      double dNextDist = abs( *(--pcIterMinus) - cCent );
      while ( dNextDist < dDist  )
      {
        dDist = dNextDist ;
        dNextDist = abs( *(--pcIterMinus) - cCent );
        if ( dNextDist >= dDist ) // first point with bigger distance
        {
          double dNextNextDist = abs( *(pcIterMinus - 1) - cCent ) ;
          if ( dNextNextDist >= dDist ) //second point with bigger distance
          {
            cmplx cOtherSide = *(++pcIterMinus) ;
            while ( dNextDist = (abs( *(++pcIterPlus) - cOtherSide )) < dDist )
              dDist = dNextDist ;

            int iIndexBegin = (int)(pcIterPlus - pContur) ;
            int iIndexEnd = (int) (pcIterMinus - pContur) ;
            Result.RemoveAt( iIndexBegin , iIndexEnd - iIndexBegin ) ;
            i = iIndexBegin ;
            break ; // from while( dNextDist < Dist )
          }
        }
      } ;
    }
  }
  return iNRemoved ;
}
// function marks points  with big distance from line between neighbors
// Another future enhancement - with big difference in distance to neighbors
// Bads is array
int MarkPointsWithBigDeviation( const CFigure& Fig , 
  double dMaxDeviation , int * Bads )
{
  double Dists[ 3000 ] ;
  double Diffs[ 3000 ] ;
  int iNMarked = 0 ;
  int iLen = (int) Fig.GetCount() ;
  memset( Bads , 0 , iLen * sizeof(int) ) ;
  // 1. Check for big differences in distance
  cmplx * pPt = (cmplx*)Fig.GetData() ;
  double dAveDist = 0. ;
  for ( int i = 0 ; i < iLen - 1 ; i++ )
  {
    double dDist = Dists[i] = abs( *(pPt + 1) - (*pPt) );
    dAveDist += dDist ;
    if ( i )
    {
      Diffs[ i ] = GetPtToLineDistance( *pPt , *(pPt - 1) , *(pPt + 1) ) ;
      if ( abs( Diffs[ i ] ) > dMaxDeviation )
      {
        Bads[ i ] = Bads[ i + 1 ] = 1 ;
        iNMarked++ ;
      }
    }
    pPt++ ;
  }
//   dAveDist /= ( Result.Count() - 1) ;
//   for ( int i = 0 ; i < (int) Result.GetCount() - 1 ; i++ )
//   {
//     if ( abs( Dists[ i ] - dAveDist ) / dAveDist > 0.2 )
//       Bads[ i ]++ ;
//   }

//   FXDblArray Angles ;
//   double dAvAngle = 0. ;
//   for ( int i = 0 ; i < (int) Result.GetCount() - 1 ; i++ )
//   {
//     double dAngle = arg( *(pPt + 1) / (*pPt) ) ;
//     dAvAngle += dAngle ;
//     Angles.Add( dAngle ) ;
//     pPt++ ;
//   }
// 
//   for ( int i = 0 ; i < (int) Result.GetCount() ; i += 5 )
//   {
//     cmplx Pt1 = GetFigurePt( Result , i ) ;
//     cmplx Pt2 = GetFigurePt( Result , i + iNLookForward ) ;
//     cmplx Cent = GetFigurePt( Result , i + (iNLookForward / 2) ) ;
//     double dist = GetPtToLineDistance( Cent , Pt1 , Pt2 ) ;
//   }
  return iNMarked ;
}




cmplx GetWeightCenter( const CFigure& Fig , ImgMoments * pMoments )
{
  cmplx Center ;
  cmplx * pcFig = (cmplx*) Fig.GetData() ;
  if ( Fig.GetCount() )
  {
    if ( !pMoments )
    {
      double dArea = 0. ;
      cmplx Accum ;
      cmplx Pt1 = pcFig[ 0 ] ;
      for ( int i = 1 ; i < (int) Fig.Count() ; i++ )
      {
        int iPt2Index = (i == ((int) Fig.GetCount() - 1)) ? 0 : i  ;
        cmplx Pt2 = pcFig[ iPt2Index ] ;
//         double dH = (Pt2.imag() - Pt1.imag()) ;
//         double dUpDownSum = (Pt1.real() + Pt2.real()) ;
//         double dTrapezArea = dUpDownSum * 0.5 * dH  ;
//         dArea += dTrapezArea ;
//         double dYWCent =
//           dH * (2. * Pt2.real() + Pt1.real()) / (3. * dUpDownSum) ;
//         cmplx cWCentPt( dUpDownSum * 0.5 , dYWCent + Pt1.imag() ) ;
//         Accum += cWCentPt * dTrapezArea ;
        double dX = Pt2.real() - Pt1.real();
        double dSumY = Pt2.imag() + Pt1.imag();
        cmplx Add( dX * (2.* Pt1.real() * Pt1.imag()
            + Pt1.real() * Pt2.imag() + Pt2.real() * Pt1.imag()
            + 2. * Pt2.real() * Pt2.imag()),
            dX * (dSumY * Pt1.imag() + Pt2.imag() * Pt2.imag()));
        Accum += Add;
        dArea += dX * dSumY;
        Pt1 = Pt2 ;
      }
      Center = (Accum /= dArea) / 3. ;
    }
    else
    {

    }
  }
  return Center ;
}

int GetNVerticesOnFigure( const CFigure& Fig  ,
  cmplx& Center , CmplxArray * pResults , double dMinMaxThres ,
  double *pdPerimeter , CIntArray * pIndexes )
{
  if ( abs( Center ) == 0. )
    Center = GetWeightCenter( Fig ) ;

  cmplx * pcFig = (cmplx*) Fig.GetData() ;
  FXDblArray Distances ;
  double dMin = DBL_MAX ;
  double dMax = -DBL_MAX ;
  int iMinIndex = 0 ;
  int iMaxIndex = 0 ;
  double dPerimeter = 0.;
  int iLen = (int) Fig.Count(); // for text shortening
  double dAverage = 0.;
  for ( int i = 0 ; i < iLen; i++ )
  {
    double dDistToCenter = abs( pcFig[ i ] - Center ) ;
    Distances.Add(dDistToCenter);
    dAverage += dDistToCenter;
    SetMinMax(dDistToCenter, dMin , dMax , i , iMinIndex , iMaxIndex );
    if ( pdPerimeter )
    {
      double dLen = abs( pcFig[i] - pcFig[(i + 1) % (int) Fig.Count()]);
      dPerimeter += dLen;
    }
  }
  dAverage /= iLen;
  if (pdPerimeter)
    *pdPerimeter = dPerimeter;
  double dAmpl = (dMax - dMin);
  if ( dAmpl / dMax < 0.03 )
  { // round form
    pResults->Add(cmplx(dAverage, 0.));
    return 0; // no vertices, round form
  }
  double dThres = dMinMaxThres * (dMax - dMin) /*+ dMin*/ ;
  int i = iMaxIndex ;
  int iLimit = iMaxIndex + (int) Distances.Count() ;
  if ( pResults )
  {
    pResults->Add( pcFig[ iMaxIndex ] ) ;
  }
  bool bSearchMin = true ;
  int iNMaxs = 1 ; // first maximum found
  int iNMins = 0 ;
  double dCurrMax = Distances[iMaxIndex] ;
  RotateArray(Distances.GetData(), (int) Distances.Count(), iMaxIndex);
  int iNScanned = 1 ;
  int iShift = iMaxIndex;
  int iSEnd = iLen - 10; // we don't have look up to end, end is on local maximum
  int iIter = 0 ;
  iNScanned += 10;
  double * pData = Distances.GetData();
  while ( iNScanned < iSEnd)
  {
    // now iCurrIndex is index of local maximum
    int iLocalMinIndex = FindLocalMinimum(pData + iNScanned, iSEnd - iNScanned, 10);
    // there is local minimum
    if (iLocalMinIndex < 0 )
      break;
    iNMins++;
    iLocalMinIndex += iNScanned;
    iNScanned = iLocalMinIndex + 1;
    int iLocalMaxIndex = FindLocalMaximum(pData + iNScanned, iSEnd - iNScanned, 10);
    if (iLocalMaxIndex < 0)
      break;
    iLocalMaxIndex += iNScanned;
    if ( iLocalMaxIndex - iLocalMinIndex > 10 )
    {
//       double dDistBefore = Distances[ (iLocalMaxIndex - 20) % Fig.Count() ];
//       double dDistCenter = Distances[ (iLocalMaxIndex) % Fig.Count() ];
//       double dDistAfter = Distances[ (iLocalMaxIndex + 20) % Fig.Count() ];
// 
//       if ( (dDistCenter - dDistBefore) > 3.0
//         && (dDistCenter - dDistAfter) > 3.0 )
      double dLocalMax = Distances[ iLocalMaxIndex ] ;
      double dLocalMin = Distances[ iLocalMinIndex ] ;
      double dLocalAmpl = dLocalMax - dLocalMin ;
      double dDiffOfLocalMaxFromMax = dMax - dLocalMax ;
      double dMaxRatio = dDiffOfLocalMaxFromMax / dMax ;
      if ( dLocalAmpl >= dThres || dMaxRatio <= 0.02)
      {
        iNMaxs++;
        if ( pResults )
        {
          pResults->Add( GetFigurePt( Fig , iShift + iLocalMaxIndex ) ) ;
          if (pIndexes)
            pIndexes->Add( (iShift + iLocalMaxIndex) % Fig.GetCount() ) ;
        }
      }
    }
    iNScanned = iLocalMaxIndex + 1;
  }
  return iNMaxs ;
}

// Function returns contour segments which are not touching ROI edge.
ActiveSegments * GetInternalSegments(
  const CFigureFrame * pContur , const CRect * ROI )
{
  ActiveSegments * pSegments = NULL;
  CSegmentInsideROI * pNewSegment = NULL;
  cmplx cCent( 0.5 * ((double) ROI->left + (double) ROI->right) ,
    0.5 * ((double) ROI->top + (double) ROI->bottom) );
  int i = 0;
  int iLastEdge = 0;
  int iThisEdge;
  for ( ; i < pContur->GetCount(); i++ )
  {
    cmplx& Pt = (cmplx&) pContur->GetAt( i );

    if ( iThisEdge = IsPtOnEdge( Pt , ROI ) )
    {
      if ( pNewSegment )
      {
        if ( !pSegments )
          pSegments = new ActiveSegments;

        pNewSegment->m_iIndexEnd = i - 1;
        pNewSegment->m_iLastEdge = iThisEdge;
        iLastEdge = 0;
        pSegments->Add( pNewSegment );

        pNewSegment = NULL;
      }
      iLastEdge = iThisEdge;
    }
    else
    {
      if ( !pNewSegment )
      {
        pNewSegment = new CSegmentInsideROI( i , pContur );
        pNewSegment->m_iFirstEdge = iLastEdge;
        iLastEdge = 0 ;
      }
      pNewSegment->AccountPt( Pt , &cCent );

      pNewSegment->Add( pContur->GetAt( i ) ) ;
    }
  }

  if ( pNewSegment )
  {
    if ( !pSegments )
      pSegments = new ActiveSegments;

    pNewSegment->m_iIndexEnd = i - 1;
    pNewSegment->m_iLastEdge = iLastEdge;
    pSegments->Add( pNewSegment );
    pNewSegment = NULL;
  }
  if ( pSegments && pSegments->GetCount() >= 2 )
  {
    CSegmentInsideROI * pFirst = pSegments->GetAt( 0 );
    CSegmentInsideROI * pLast = pSegments->GetAt( pSegments->GetUpperBound() );
    cmplx FirstOnFirst = *((cmplx*) (pContur->GetData() + pFirst->m_iIndexBegin));
    cmplx LastOnLast = *((cmplx*) (pContur->GetData() + pLast->m_iIndexEnd));
    if ( FirstOnFirst == LastOnLast )
    {
      (CDPointArray*) pFirst->Append( *((const CDPointArray*) pLast) ) ;
      pFirst->SetSize( pFirst->GetCount() - 1 ) ;
      if ( pFirst->m_dMinDistToCenter > pLast->m_dMinDistToCenter )
      {
        pFirst->m_dMinDistToCenter = pLast->m_dMinDistToCenter;
        pFirst->m_iMinDistToCenterIndex = pLast->m_iMinDistToCenterIndex + pFirst->m_iNElements ;
      }
      pFirst->m_iSecondBegin = pFirst->m_iIndexBegin;
      pFirst->m_iIndexBegin = pLast->m_iIndexBegin;
      pFirst->m_iSecondEnd = pLast->m_iIndexEnd;
      pFirst->m_Rect.Union( pLast->m_Rect );
      pFirst->m_iNElements += pLast->m_iNElements - 1;
      pFirst->m_iFirstEdge = pLast->m_iFirstEdge;
      pSegments->RemoveLast() ;

    }
  }
  return pSegments;
}

ActiveSegments * GetCrossROISegments(
  const CFigureFrame& Contur, CRect& ROI,
  int * piMinIndex , double * pdMinDist )
{
  int iMinIndex = -1;
  ActiveSegments * pSegments = NULL;
  CSegmentInsideROI * pNewSegment = NULL;
  cmplx cCent(0.5 * ((double)ROI.left + (double)ROI.right),
    0.5 * ((double)ROI.top + (double)ROI.bottom));
  int i = 0;
  int iLastEdge = 0;
  int iThisEdge;
  for (; i < Contur.GetCount(); i++)
  {
    cmplx& Pt = (cmplx&)Contur.GetAt(i);

    if (iThisEdge = IsPtOnEdge(Pt, ROI))
    {
      if (pNewSegment)
      {
        if (!pSegments)
          pSegments = new ActiveSegments;

        pNewSegment->m_iIndexEnd = i - 1;
        pNewSegment->m_iLastEdge = iThisEdge;
        iLastEdge = 0;
        pSegments->Add(pNewSegment);

        pNewSegment = NULL;
      }
      iLastEdge = iThisEdge;
    }
    else
    {
      if (!pNewSegment)
      {
        pNewSegment = new CSegmentInsideROI(i, &Contur);
        pNewSegment->m_iFirstEdge = iLastEdge;
        iLastEdge = 0;
      }
      double dDistToCenter = abs(Pt - cCent);
      if (dDistToCenter < pNewSegment->m_dMinDistToCenter)
      {
        pNewSegment->m_dMinDistToCenter = dDistToCenter;
        pNewSegment->m_iMinDistToCenterIndex = i;
      }
      pNewSegment->AccountPt(Pt, &cCent);
    }
  }

  if (pNewSegment)
  {
    if (!pSegments)
      pSegments = new ActiveSegments;

    pNewSegment->m_iIndexEnd = i - 1;
    pNewSegment->m_iLastEdge = iLastEdge;
    pSegments->Add(pNewSegment);
    pNewSegment = NULL;
  }
  if (pSegments)
  {
    if (pSegments->GetCount() >= 2)
    {
      CSegmentInsideROI * pFirst = pSegments->GetAt(0);
      CSegmentInsideROI * pLast = pSegments->GetAt(pSegments->GetUpperBound());
      cmplx FirstOnFirst = *((cmplx*)(Contur.GetData() + pFirst->m_iIndexBegin));
      cmplx LastOnLast = *((cmplx*)(Contur.GetData() + pLast->m_iIndexEnd));
      if (FirstOnFirst == LastOnLast)
      {
        pFirst->m_iSecondBegin = pFirst->m_iIndexBegin;
        pFirst->m_iIndexBegin = pLast->m_iIndexBegin;
        pFirst->m_iSecondEnd = pLast->m_iIndexEnd;
        pFirst->m_Rect.Union(pLast->m_Rect);
        pFirst->m_iNElements += pLast->m_iNElements - 1;
        pFirst->m_iFirstEdge = pLast->m_iFirstEdge;
        if (pFirst->m_dMinDistToCenter > pLast->m_dMinDistToCenter)
        {
          pFirst->m_dMinDistToCenter = pLast->m_dMinDistToCenter;
          pFirst->m_iMinDistToCenterIndex = pLast->m_iMinDistToCenterIndex;
        }
        pSegments->RemoveLast();
      }
    }
    if ( pdMinDist )
    {
      double dMinDist = DBL_MAX;
      for (int i = 0; i < pSegments->GetCount(); i++)
      {
        CSegmentInsideROI& Segment = *(pSegments->GetAt( i )) ;
        if (dMinDist > Segment.m_dMinDistToCenter)
        {
          cmplx * pCmplx = (cmplx*) Contur.GetData() ;
          cmplx Pt1 = pCmplx[ Segment.m_iIndexBegin ];
          cmplx Pt2 = pCmplx[ Segment.m_iIndexEnd ];
          double dDistBeginToEnd = abs( Pt1 - Pt2 );
          double dDistToCent = fabs( GetPtToLineDistance( cCent , Pt1 , Pt2 ) ) ;
          double dVect = abs( Pt1 - Pt2 );
          double dSum = dDistToCent + dVect ;
          cmplx Pts[ 2 ] = { Pt1 , Pt2 };
          if ( dDistBeginToEnd > (min( ROI.Width() , ROI.Height() ) / 3)
            && Segment.m_iNElements < dSum * 1.7 )
          {
            dMinDist = Segment.m_dMinDistToCenter;
            iMinIndex = i;
          }
        }
      }
      *pdMinDist = dMinDist;
      if (piMinIndex)
        *piMinIndex = iMinIndex;
    }
  }
  return pSegments;
}


// This function looks for vertices with distance to center
// around max distance.
// After that, if there is only 2 such vertices, it looks for 
// additional two vertices with maximal distance from line
// between found 2 vertices
int GetNFilteredVertices( const CFigure& Fig ,
  cmplx& Center , CmplxArray * pResults , double dMinMaxThres ,
  double *pdPerimeter , FXIntArray * pIndexes )
{
  if ( abs( Center ) == 0. )
    Center = GetWeightCenter( Fig ) ;

  cmplx * pcFig = (cmplx*) Fig.GetData() ;
  pResults->RemoveAll() ;
  if ( pIndexes )
    pIndexes->RemoveAll() ;
  FXDblArray Distances ;
  double dMin = DBL_MAX ;
  double dMax = -DBL_MAX ;
  int iMinIndex = 0 ;
  int iMaxIndex = 0 ;
  double dPerimeter = 0.;
  int iLen = (int) Fig.Count(); // for text shortening
  double dAverage = 0.;
  for ( int i = 0 ; i < iLen; i++ )
  {
    double dDistToCenter = abs( pcFig[ i ] - Center ) ;
    Distances.Add( dDistToCenter );
    dAverage += dDistToCenter;
    SetMinMax( dDistToCenter , dMin , dMax , i , iMinIndex , iMaxIndex );
    if ( pdPerimeter )
    {
      double dLen = abs( pcFig[ i ] - pcFig[ (i + 1) % Fig.Count() ] );
      dPerimeter += dLen;
    }
  }
  dAverage /= iLen;
  if ( pdPerimeter )
    *pdPerimeter = dPerimeter;
  if (!pResults)
    return 0;
  double dAmpl = (dMax - dMin);
  if ( dAmpl / dMax < 0.03 )
  { // round form
    pResults->Add( cmplx( dAverage , 0. ) );
    return 0; // no vertices, round form
  }
  double dThres = dMinMaxThres * (dMax - dMin) /*+ dMin*/ ;
  int i = iMaxIndex ;
  int iLimit = iMaxIndex + (int) Distances.Count() ;
  pResults->Add( pcFig[ iMaxIndex ] ) ;
  bool bSearchMin = true ;
  int iNMaxs = 1 ; // first maximum found
  int iNMins = 0 ;
  double dCurrMax = Distances[ iMaxIndex ] ;
  RotateArray( Distances.GetData() , (int) Distances.Count() , iMaxIndex );
  int iNScanned = 10 ;
  int iShift = iMaxIndex;
  int iSEnd = iLen - 10; // we don't have look up to end, end is on local maximum
  int iIter = 0 ;
  double * pData = Distances.GetData();
  FXIntArray LocalIndexes ;
  FXIntArray& Indexes = (pIndexes) ? *pIndexes : LocalIndexes ;
  Indexes.Add( 0 ) ;
  // first pass: find all vertices near maximum distance to center
  while ( iNScanned < iSEnd )
  {
    // now iCurrIndex is index of local maximum
    int iLocalMinIndex = FindLocalMinimum( pData + iNScanned , iSEnd - iNScanned , 10 );
    // there is local minimum
    if ( iLocalMinIndex < 0 )
      break;
    iNMins++;
    iLocalMinIndex += iNScanned;
    iNScanned = iLocalMinIndex + 1;
    int iLocalMaxIndex = FindLocalMaximum( pData + iNScanned , iSEnd - iNScanned , 10 );
    if ( iLocalMaxIndex < 0 )
      break;
    iLocalMaxIndex += iNScanned;
    int iIndexDiff = iLocalMaxIndex - iLocalMinIndex ;
    if ( iIndexDiff > 25 )
    {
      cmplx Pt = GetFigurePt( Fig , iShift + iLocalMaxIndex ) ;
      double dDistBefore = Distances[ (iLocalMaxIndex - 20) % Fig.Count() ];
      double dDistCenter = Distances[ (iLocalMaxIndex) % Fig.Count() ];
      double dDistAfter = Distances[ (iLocalMaxIndex + 20) % Fig.Count() ];

      double dDiffOfLocalMaxFromMax = dMax - dDistCenter ;
      double dMaxRatio = dDiffOfLocalMaxFromMax / dMax ;
      double dDistToPrevExtremum = abs( Pt - pResults->Last() ) ;
      if ( dDistToPrevExtremum > 0.03 * dPerimeter )
      {
        if ( dDistToPrevExtremum < 0.09 * dPerimeter ) // very short distance from 
        {
          if ( ((fabs( dMax - dDistCenter ) / dMax) < 0.015)  // Found maximum lies is near max distance
            && (dDistCenter - dDistAfter) > 2.0 ) // point after 20 pixels closer to center for 2 pixels
          {
            pResults->Add( Pt ) ;
            Indexes.Add( iLocalMaxIndex ) ;
          }
        }
        else if (dMaxRatio <= 0.015)
        {
          pResults->Add(Pt);
          Indexes.Add(iLocalMaxIndex);
        }
        else if (  ((dDistCenter - dDistBefore) > 2.0)
                && ((dDistCenter - dDistAfter) > 2.0) )
        {
          double dLocalMin = Distances[ iLocalMinIndex ] ;
          double dDistDiff = dDistCenter - dLocalMin ;
          if ( (dDistDiff > dThres * 0.5) )
          {
            pResults->Add( Pt ) ;
              Indexes.Add( iLocalMaxIndex ) ;
          }
        }
      }
    }
    iNScanned = iLocalMaxIndex + 1;
  }
  for ( int i = 0 ; i < Indexes.size() ; i++ )
    Indexes[ i ] = (Indexes[ i ] + iShift) % Fig.size() ;

  // remove false vertices (i.e. with another inside on small radius)
  if ( pResults->size() > 1 )
  {
    for ( int i = 0 ; i < pResults->size() ; i++ )
    {
      if ( abs(pResults->GetAt(i) - pResults->GetAt( (i + 1) % pResults->size() ) ) < (dAverage * 0.05) )
      {
        pResults->RemoveAt( i ) ;
        Indexes.RemoveAt( i ) ;
      }
    }
//     if ( pResults->Count() == 2 ) // we have to find short diameter
//     {
//       int iLastPassedVIndex = 0 ;
//       int iNextVertIndex = Indexes[ 1 ] ;
//       iNScanned = 10 ;
//       cmplx cVert1 = GetFigurePt( Fig , Indexes[ 0 ] ) ;
//       cmplx cVert2 = GetFigurePt( Fig , Indexes[ 1 ] ) ;
//       int iCentIndex = (Indexes[ 0 ] + Indexes[ 1 ]) / 2 ;
//       int iCentIndex2 = (iCentIndex + (Fig.size() / 2)) % Fig.size() ;
//       cmplx cCent1 = GetFigurePt( Fig , iCentIndex ) ;
//       cmplx cCent2 = 
//       cmplx cIterator = GetFigurePt( Fig , iShift + iNScanned ) ;
//       double dNextDist = fabs( GetPtToLineDistance( cIterator , cVert1 , cVert2 ) ) ;
//       double dDist ;
//       while ( ++iNScanned < iSEnd )
//       {
//         dDist = dNextDist ;
//         cIterator = GetFigurePt( Fig , iShift + iNScanned ) ;
//         double dNextDist = fabs( GetPtToLineDistance( cIterator , cVert1 , cVert2 ) ) ;
//         if ( dNextDist > dDist ) // distance is growing, continue
//           continue ;
//         int iLookForward = iNScanned + 1 ;
//         while ( iLookForward < iNScanned + 10
//           && iLookForward < iNextVertIndex - 5 )
//         {
//           cIterator = GetFigurePt( Fig , iShift + iLookForward ) ;
//           dNextDist = fabs( GetPtToLineDistance( cIterator , cVert1 , cVert2 ) ) ;
//           if ( dNextDist > dDist )
//           {
//             iNScanned = iLookForward + 1 ;
//             break ;
//           }
//           iLookForward++ ;
//         }
//         if ( dNextDist > dDist )
//           continue ;
// 
//         if ( iNScanned < iNextVertIndex )
//         {
//           pResults->InsertAt( 1 , GetFigurePt( Fig , iShift + iNScanned ) ) ;
//           Indexes.InsertAt( 1 , iNScanned ) ;
//           iNScanned = iNextVertIndex + 10 ;
//         }
//         else
//         {
//           pResults->Add( GetFigurePt( Fig , iShift + iNScanned ) ) ;
//           Indexes.Add( iNScanned ) ;
//           break ;
//         }
//         cIterator = GetFigurePt( Fig , iShift + iNScanned ) ;
//         dNextDist = fabs( GetPtToLineDistance( cIterator , cVert1 , cVert2 ) ) ; 
//       }
//     }
  }
  return (int) pResults->Count()  ;
}

bool CSegmentInsideROI::IsSegmentComplicated()
{
  int iLen = 0 ;
  if ( !m_pFigure || !(iLen = (int)m_pFigure->Count()) )
    return false ;

  if ( abs( m_iIndexEnd - m_iIndexBegin ) < 5  )
    return false ;

  cmplx * pCmplx = (cmplx*) m_pFigure->GetData() ;
  cmplx Pt1 = pCmplx[ m_iIndexBegin ];
  cmplx Pt2 = pCmplx[ m_iIndexEnd ];
  int iIndexCent = (m_iIndexBegin <= m_iIndexEnd) ? 
    (m_iIndexBegin + m_iIndexEnd) / 2 
    : ((m_iIndexBegin + m_iIndexEnd + iLen) / 2) % iLen ;
  cmplx cCent = pCmplx[ iIndexCent ] ;
  double dDistToCent = fabs( GetPtToLineDistance( cCent , Pt1 , Pt2 ) ) ;
  double dVect = abs( Pt1 - Pt2 );
  return dDistToCent > dVect * 2 ;
}

bool MeasureHorizAndVertEdges( const CFigureFrame * pFigure ,
  cmplx& cCent , cmplx& cSize , CContainerFrame * pView , CmplxVector * pCentsAndCorners )
{
  StraightLineRegression LeftRegr , TopRegr , RightRegr , BottomRegr ;
  cmplx cLT , cLB , cTL , cTR , cRT , cRB , cBL , cBR ;

  double dStdLeft = LeftRegr.GetAndCalc( (const CFigure*) pFigure , cCent , 
    M_PI , cSize.imag() * 0.4 , M_PI_2 , &cLB , &cLT ) ;
  double dStdTop = TopRegr.GetAndCalc( (const CFigure*) pFigure , cCent ,
    M_PI_2 , cSize.real() / 3 , 0. , &cTL , &cTR ) ;
  double dStdRight = RightRegr.GetAndCalc( (const CFigure*) pFigure , cCent ,
    0. , cSize.imag() / 3 , M_PI_2 , &cRT , &cRB ) ;
  double dStdBottom = BottomRegr.GetAndCalc( (const CFigure*) pFigure , cCent , 
    -M_PI_2 , cSize.real() / 3 , 0. , &cBL , &cBR ) ;

  if ( pCentsAndCorners )
  {
    pCentsAndCorners->clear() ;
    CLine2d LeftLine = LeftRegr.GetCLine2d() ;
    CLine2d TopLine = TopRegr.GetCLine2d() ;
    CLine2d RightLine = RightRegr.GetCLine2d() ;
    CLine2d BottomLine = BottomRegr.GetCLine2d() ;

    cmplx TopLeftCorner , TopRightCorner , BottomLeftCorner , BottomRightCorner ;
    if ( LeftLine.intersect( TopLine , TopLeftCorner )
      && RightLine.intersect( TopLine , TopRightCorner )
      && BottomLine.intersect( LeftLine , BottomLeftCorner )
      && BottomLine.intersect( RightLine , BottomRightCorner ) )
    {
      cmplx cLeftCent = (TopLeftCorner + TopRightCorner) / 2. ;
      cmplx cTopCent = (TopLeftCorner + BottomLeftCorner) / 2. ;
      cmplx cRightCent = (BottomRightCorner + TopRightCorner) / 2. ;
      cmplx cBottomCent = (BottomRightCorner + BottomLeftCorner) / 2. ;
      cmplx cCent = 0.25 * (TopLeftCorner + TopRightCorner + BottomLeftCorner + BottomRightCorner) ;

      pCentsAndCorners->push_back( cCent ) ;
      pCentsAndCorners->push_back( cLeftCent ) ;
      pCentsAndCorners->push_back( cTopCent ) ;
      pCentsAndCorners->push_back( cRightCent ) ;
      pCentsAndCorners->push_back( cBottomCent ) ;

      pCentsAndCorners->push_back( TopLeftCorner ) ;
      pCentsAndCorners->push_back( TopRightCorner ) ;
      pCentsAndCorners->push_back( BottomRightCorner ) ;
      pCentsAndCorners->push_back( BottomLeftCorner ) ;
      if ( pView )
      {
        FXPropertyKit Attributes ;
        Attributes.WriteInt( "Sz" , 4 ) ;
        Attributes.WriteInt( "thickness" , 3 ) ;
        Attributes.WriteString( "color" , "0xff0000" ) ;

        pView->AddFrame( CreateLineFrame(
          cLB , cLT , 0xffff00 , "LeftRegr" ) ) ;

        pView->AddFrame( CreateLineFrame( 
          TopLeftCorner , TopRightCorner , 0x00ff00 , "TopEdge" ) ) ;
        pView->AddFrame( CreatePtFrame( cTopCent , (LPCTSTR) Attributes , "TopCent" ) ) ;

        pView->AddFrame( CreateLineFrame( TopRightCorner ,
          BottomRightCorner , 0x00ff00 , "RightEdge" ) ) ;
        pView->AddFrame( CreatePtFrame( cRightCent , (LPCTSTR) Attributes , "RightCent" ) ) ;

        pView->AddFrame( CreateLineFrame( BottomLeftCorner ,
          BottomRightCorner , 0x00ff00 , "BottomEdge" ) ) ;
        pView->AddFrame( CreatePtFrame( cBottomCent , (LPCTSTR) Attributes , "BottomCent" ) ) ;

        pView->AddFrame( CreateLineFrame( BottomLeftCorner ,
          TopLeftCorner , 0x00ff00 , "LeftEdge" ) ) ;
        pView->AddFrame( CreatePtFrame( cLeftCent , (LPCTSTR) Attributes , "LeftCent" ) ) ;

        pView->AddFrame( CreatePtFrame( cCent , (LPCTSTR) Attributes , "Cent" ) ) ;
	    }
    }
    else
    {
      FxSendLogMsg( MSG_ERROR_LEVEL , "MeasureHorizAndVertEdges" , 0 , "Can't calculate corners" ) ;
      return false ;
    }
  }
  else if ( pView )
  {
    double dXLeftUpper = LeftRegr.GetX( cLT.imag() ) ;
    double dXLeftLower = LeftRegr.GetX( cLB.imag() ) ;
    cmplx cCalcFirst( dXLeftUpper , cLT.imag() ) ;
    cmplx cCalcSecond( dXLeftLower , cLB.imag() ) ;
    CFigureFrame * pLine = CreateLineFrame( cCalcFirst ,
      cCalcSecond , 0x00ff00 , "LeftEdge" ) ;
    pView->AddFrame( pLine ) ;
    cmplx cCent = (cCalcFirst + cCalcSecond) / 2. ;
    CFigureFrame * pEdgeCent = CreatePtFrame( cCent , GetHRTickCount() , 0x00ffff , "LeftCent" ) ;
    pEdgeCent->Attributes()->WriteInt( "Sz" , 6 ) ;
    pEdgeCent->Attributes()->WriteInt( "thickness" , 3 ) ;
    pView->AddFrame( pEdgeCent ) ;
    
    double dY1 = TopRegr.GetY( cTL.real() ) ;
    double dY2 = TopRegr.GetY( cTR.real() ) ;
    cCalcFirst =  cmplx( cTL.real() , dY1 ) ;
    cCalcSecond = cmplx( cTR.real() , dY2 ) ;
    pLine = CreateLineFrame( cCalcFirst ,
      cCalcSecond , 0x00ff00 , "TopEdge" ) ;
    pView->AddFrame( pLine ) ;
    cCent = (cCalcFirst + cCalcSecond) / 2. ;
    pEdgeCent = CreatePtFrame( cCent , GetHRTickCount() , 0x00ffff , "TopCent" ) ;
    pEdgeCent->Attributes()->WriteInt( "Sz" , 6 ) ;
    pEdgeCent->Attributes()->WriteInt( "thickness" , 3 ) ;
    pView->AddFrame( pEdgeCent ) ;

    double dRightUpper = RightRegr.GetX( cRT.imag() ) ;
    double dRightLower = RightRegr.GetX( cRB.imag() ) ;
    cCalcFirst  = cmplx( dXLeftUpper , cRT.imag() ) ;
    cCalcSecond = cmplx( dXLeftLower , cRB.imag() ) ;
    pLine = CreateLineFrame( cCalcFirst ,
      cCalcSecond , 0x00ff00 , "RightEdge" ) ;
    pView->AddFrame( pLine ) ;
    cCent = (cCalcFirst + cCalcSecond) / 2. ;
    pEdgeCent = CreatePtFrame( cCent , GetHRTickCount() , 0x00ffff , "RightCent" ) ;
    pEdgeCent->Attributes()->WriteInt( "Sz" , 6 ) ;
    pEdgeCent->Attributes()->WriteInt( "thickness" , 3 ) ;
    pView->AddFrame( pEdgeCent ) ;

    dY1 = BottomRegr.GetY( cBL.real() ) ;
    dY2 = BottomRegr.GetY( cBR.real() ) ;
    cCalcFirst = cmplx( cBL.real() , dY1 ) ;
    cCalcSecond = cmplx( cBR.real() , dY2 ) ;
    pLine = CreateLineFrame( cCalcFirst ,
      cCalcSecond , 0x00ff00 , "BottomEdge" ) ;
    pView->AddFrame( pLine ) ;
    cCent = (cCalcFirst + cCalcSecond) / 2. ;
    pEdgeCent = CreatePtFrame( cCent , GetHRTickCount() , 0x00ffff , "BottomCent" ) ;
    pEdgeCent->Attributes()->WriteInt( "Sz" , 6 ) ;
    pEdgeCent->Attributes()->WriteInt( "thickness" , 3 ) ;
    pView->AddFrame( pEdgeCent ) ;
  }

  return true ;
}

bool MeasureAndFilterHorizAndVertEdges( const CFigureFrame * pFigure ,
  cmplx& cCent , cmplx& cSize , CContainerFrame * pView ,
  CmplxVector * pCentsAndCorners , double dGoodStdThres_pix )
{
  StraightLineRegression LeftRegr , TopRegr , RightRegr , BottomRegr ;
  CFilterRegression LRegrFiltered , TRegrFiltered , RRegrFiltered , BRegrFiltered ;
  cmplx cLT , cLB , cTL , cTR , cRT , cRB , cBL , cBR ;
  double dStdLeftFiltered = DBL_MAX , dStdTopFiltered = DBL_MAX ,
    dStdRightFiltered = DBL_MAX , dStdBottomFiltered = DBL_MAX ;

  CmplxVector EdgePts;
  LPCTSTR pExtremeAttributes = _T( "Sz=6;color=0xff00ff;" ) ;
  LPCTSTR pSelectedPtsAttributes = _T( "Sz=2;color=0xff8000;" ) ;
#define RANGE_FROM_CENTER 0.40
  double dStdLeft = LeftRegr.GetAndCalc( (const CFigure*) pFigure , cCent ,
    M_PI , cSize.imag() * RANGE_FROM_CENTER, -M_PI_2 , &cLB , &cLT ) ;
  if ( dStdLeft == DBL_MAX )
	  return false;
  if ( dStdLeft > dGoodStdThres_pix )
  {
    // Abstract line on right side
    CLine2d RightLine( cmplx( 2000. , 0. ) , cmplx( 2000. , 2000. ) ) ;
    dStdLeftFiltered = LeftRegr.DoFilterFarestFromLine( RightLine ,
      dGoodStdThres_pix , LRegrFiltered , &EdgePts ) ;
    if ( dStdLeftFiltered != DBL_MAX )
    {
      pView->AddFrame( CreatePtFrame( EdgePts[ 0 ] , pExtremeAttributes ) ) ;
      pView->AddFrame( CreatePtFrame( EdgePts[ 1 ] , pExtremeAttributes ) ) ;
      for ( size_t i = 2; i < EdgePts.size(); i++ )
        pView->AddFrame( CreatePtFrame( EdgePts[ i ] , pSelectedPtsAttributes ) );
    }
  }
  CLine2d LeftLine = (dStdLeftFiltered != DBL_MAX) ? 
    LeftRegr.GetFilteredLine() : LeftRegr.GetCLine2d() ;

  EdgePts.clear() ;
  double dStdTop = TopRegr.GetAndCalc( (const CFigure*) pFigure , cCent ,
    -M_PI_2 , cSize.real() * RANGE_FROM_CENTER, 0. , &cTL , &cTR ) ;
  if ( dStdTop == DBL_MAX )
    return false;
  if ( dStdTop > dGoodStdThres_pix )
  {
    // Abstract line on bottom side
    CLine2d BottomLine( cmplx( 0. , 2000. ) , cmplx( 2000. , 2000. ) ) ;
    dStdTopFiltered = TopRegr.DoFilterFarestFromLine( BottomLine ,
      dGoodStdThres_pix , TRegrFiltered , &EdgePts ) ; 
    if ( dStdTopFiltered != DBL_MAX )
    {
      pView->AddFrame( CreatePtFrame( EdgePts[ 0 ] , pExtremeAttributes ) ) ;
      pView->AddFrame( CreatePtFrame( EdgePts[ 1 ] , pExtremeAttributes ) ) ;
      for ( size_t i = 2; i < EdgePts.size(); i++ )
        pView->AddFrame( CreatePtFrame( EdgePts[ i ] , pSelectedPtsAttributes ) );
    }
  }
  CLine2d TopLine = (dStdTopFiltered != DBL_MAX) ?
    TopRegr.GetFilteredLine() : TopRegr.GetCLine2d() ;
  
  EdgePts.clear() ;
  double dStdRight = RightRegr.GetAndCalc( (const CFigure*) pFigure , cCent ,
    0. , cSize.imag() * RANGE_FROM_CENTER, M_PI_2 , &cRT , &cRB ) ;
  if ( dStdRight == DBL_MAX )
	  return false;
  if ( dStdRight > dGoodStdThres_pix )
  {
    // Abstract line on left side
    CLine2d LeftLine( cmplx( 0. , 0. ) , cmplx( 0. , 2000. ) ) ;
    dStdRightFiltered = RightRegr.DoFilterFarestFromLine( LeftLine ,
      dGoodStdThres_pix , RRegrFiltered , &EdgePts ) ;
    if ( dStdRightFiltered != DBL_MAX )
    {
      pView->AddFrame( CreatePtFrame( EdgePts[ 0 ] , pExtremeAttributes ) ) ;
      pView->AddFrame( CreatePtFrame( EdgePts[ 1 ] , pExtremeAttributes ) ) ;
      for ( size_t i = 2; i < EdgePts.size(); i++ )
        pView->AddFrame( CreatePtFrame( EdgePts[ i ] , pSelectedPtsAttributes ) );
    }
  }
  CLine2d RightLine = (dStdRightFiltered != DBL_MAX) ?
    RightRegr.GetFilteredLine() : RightRegr.GetCLine2d() ;

  EdgePts.clear() ;
  double dStdBottom = BottomRegr.GetAndCalc( (const CFigure*) pFigure , cCent ,
    M_PI_2 , cSize.real() * RANGE_FROM_CENTER, M_PI , &cBL , &cBR ) ;
  if ( dStdBottom == DBL_MAX )
	  return false;
  if ( dStdBottom > dGoodStdThres_pix )
  {
    // Abstract line on up side
    CLine2d TopLine( cmplx( 0. , 0. ) , cmplx( 2000. , 0. ) ) ;
    dStdBottomFiltered = BottomRegr.DoFilterFarestFromLine( TopLine ,
      dGoodStdThres_pix , BRegrFiltered , &EdgePts ) ;
    if ( dStdBottomFiltered != DBL_MAX )
    {
      pView->AddFrame( CreatePtFrame( EdgePts[ 0 ] , pExtremeAttributes ) ) ;
      pView->AddFrame( CreatePtFrame( EdgePts[ 1 ] , pExtremeAttributes ) ) ;
      for ( size_t i = 2; i < EdgePts.size(); i++ )
        pView->AddFrame( CreatePtFrame( EdgePts[ i ] , pSelectedPtsAttributes ) );
    }
  }
  CLine2d BottomLine = (dStdBottomFiltered != DBL_MAX) ?
    BottomRegr.GetFilteredLine() : BottomRegr.GetCLine2d() ;

  for (size_t i = 0; i < EdgePts.size(); i++)
    pView->AddFrame(CreatePtFrame(EdgePts[i] , "color=0x00ffff;Sz=2;"));

  if ( pCentsAndCorners )
  {
    pCentsAndCorners->clear() ;

    cmplx TopLeftCorner , TopRightCorner , BottomLeftCorner , BottomRightCorner ;
    if ( LeftLine.intersect( TopLine , TopLeftCorner )
      && RightLine.intersect( TopLine , TopRightCorner )
      && BottomLine.intersect( LeftLine , BottomLeftCorner )
      && BottomLine.intersect( RightLine , BottomRightCorner ) )
    {
      cmplx cTopCent = (TopLeftCorner + TopRightCorner) / 2. ;
      cmplx cLeftCent = (TopLeftCorner + BottomLeftCorner) / 2. ;
      cmplx cRightCent = (BottomRightCorner + TopRightCorner) / 2. ;
      cmplx cBottomCent = (BottomRightCorner + BottomLeftCorner) / 2. ;
      cmplx cCent = 0.25 * (TopLeftCorner + TopRightCorner + BottomLeftCorner + BottomRightCorner) ;

      pCentsAndCorners->push_back( cCent ) ;
      pCentsAndCorners->push_back( cLeftCent ) ;
      pCentsAndCorners->push_back( cTopCent ) ;
      pCentsAndCorners->push_back( cRightCent ) ;
      pCentsAndCorners->push_back( cBottomCent ) ;

      pCentsAndCorners->push_back( TopLeftCorner ) ;
      pCentsAndCorners->push_back( TopRightCorner ) ;
      pCentsAndCorners->push_back( BottomRightCorner ) ;
      pCentsAndCorners->push_back( BottomLeftCorner ) ;
      if ( pView )
      {
         FXPropertyKit Attributes ;
         Attributes.WriteInt( "Sz" , 4 ) ;
//         //Attributes.WriteInt( "thickness" , 3 ) ;
//        
//         Attributes.WriteString( "color" , "0xff00ff" ) ;
//         for ( size_t i = 0 ; i < LRegrFiltered.m_Pts.size() ; i++ )
//           pView->AddFrame( CreatePtFrame( LRegrFiltered.m_Pts[ i ] , (LPCTSTR) Attributes , "SelectedPts" ) ) ;
//         for (size_t i = 0 ; i < TRegrFiltered.m_Pts.size() ; i++ )
//           pView->AddFrame( CreatePtFrame( TRegrFiltered.m_Pts[ i ] , (LPCTSTR) Attributes , "SelectedPts" ) ) ;
//         for (size_t i = 0 ; i < RRegrFiltered.m_Pts.size() ; i++ )
//           pView->AddFrame( CreatePtFrame( RRegrFiltered.m_Pts[ i ] , (LPCTSTR) Attributes , "SelectedPts" ) ) ;
//         for (size_t i = 0 ; i < BRegrFiltered.m_Pts.size() ; i++ )
//           pView->AddFrame( CreatePtFrame( BRegrFiltered.m_Pts[ i ] , (LPCTSTR) Attributes , "SelectedPts" ) ) ;
// 
         Attributes.WriteString( "color" , "0xff0000" ) ;
// 
//         pView->AddFrame( CreateLineFrame(
//           cLB , cLT , 0xffff00 , "LeftRegr" ) ) ;
//         pView->AddFrame( CreateLineFrame(
//           cLBFilt , cLTFilt , 0xff0000 , "LeftRegr" ) ) ;

        pView->AddFrame( CreateLineFrame(
          TopLeftCorner , TopRightCorner , 0x00ff00 , "TopEdge" ) ) ;
        pView->AddFrame( CreatePtFrame( cTopCent , (LPCTSTR) Attributes , "TopCent" ) ) ;

        pView->AddFrame( CreateLineFrame( TopRightCorner ,
          BottomRightCorner , 0x00ff00 , "RightEdge" ) ) ;
        pView->AddFrame( CreatePtFrame( cRightCent , (LPCTSTR) Attributes , "RightCent" ) ) ;

        pView->AddFrame( CreateLineFrame( BottomLeftCorner ,
          BottomRightCorner , 0x00ff00 , "BottomEdge" ) ) ;
        pView->AddFrame( CreatePtFrame( cBottomCent , (LPCTSTR) Attributes , "BottomCent" ) ) ;

        pView->AddFrame( CreateLineFrame( BottomLeftCorner ,
          TopLeftCorner , 0x00ff00 , "LeftEdge" ) ) ;
        pView->AddFrame( CreatePtFrame( cLeftCent , (LPCTSTR) Attributes , "LeftCent" ) ) ;

        pView->AddFrame( CreatePtFrame( cCent , (LPCTSTR) Attributes , "Cent" ) ) ;
      }
    }
    else
    {
      FxSendLogMsg( MSG_ERROR_LEVEL , "MeasureHorizAndVertEdges" , 0 , "Can't calculate corners" ) ;
      return false ;
    }
  }
  else if ( pView )
  {
    double dXLeftUpper = LeftRegr.GetX( cLT.imag() ) ;
    double dXLeftLower = LeftRegr.GetX( cLB.imag() ) ;
    cmplx cCalcFirst( dXLeftUpper , cLT.imag() ) ;
    cmplx cCalcSecond( dXLeftLower , cLB.imag() ) ;
    CFigureFrame * pLine = CreateLineFrame( cCalcFirst ,
      cCalcSecond , 0x00ff00 , "LeftEdge" ) ;
    pView->AddFrame( pLine ) ;
    cmplx cCent = (cCalcFirst + cCalcSecond) / 2. ;
    CFigureFrame * pEdgeCent = CreatePtFrame( cCent , GetHRTickCount() , 0x00ffff , "LeftCent" ) ;
    pEdgeCent->Attributes()->WriteInt( "Sz" , 6 ) ;
    pEdgeCent->Attributes()->WriteInt( "thickness" , 3 ) ;
    pView->AddFrame( pEdgeCent ) ;

    double dY1 = TopRegr.GetY( cTL.real() ) ;
    double dY2 = TopRegr.GetY( cTR.real() ) ;
    cCalcFirst = cmplx( cTL.real() , dY1 ) ;
    cCalcSecond = cmplx( cTR.real() , dY2 ) ;
    pLine = CreateLineFrame( cCalcFirst ,
      cCalcSecond , 0x00ff00 , "TopEdge" ) ;
    pView->AddFrame( pLine ) ;
    cCent = (cCalcFirst + cCalcSecond) / 2. ;
    pEdgeCent = CreatePtFrame( cCent , GetHRTickCount() , 0x00ffff , "TopCent" ) ;
    pEdgeCent->Attributes()->WriteInt( "Sz" , 6 ) ;
    pEdgeCent->Attributes()->WriteInt( "thickness" , 3 ) ;
    pView->AddFrame( pEdgeCent ) ;

    double dRightUpper = RightRegr.GetX( cRT.imag() ) ;
    double dRightLower = RightRegr.GetX( cRB.imag() ) ;
    cCalcFirst = cmplx( dXLeftUpper , cRT.imag() ) ;
    cCalcSecond = cmplx( dXLeftLower , cRB.imag() ) ;
    pLine = CreateLineFrame( cCalcFirst ,
      cCalcSecond , 0x00ff00 , "RightEdge" ) ;
    pView->AddFrame( pLine ) ;
    cCent = (cCalcFirst + cCalcSecond) / 2. ;
    pEdgeCent = CreatePtFrame( cCent , GetHRTickCount() , 0x00ffff , "RightCent" ) ;
    pEdgeCent->Attributes()->WriteInt( "Sz" , 6 ) ;
    pEdgeCent->Attributes()->WriteInt( "thickness" , 3 ) ;
    pView->AddFrame( pEdgeCent ) ;

    dY1 = BottomRegr.GetY( cBL.real() ) ;
    dY2 = BottomRegr.GetY( cBR.real() ) ;
    cCalcFirst = cmplx( cBL.real() , dY1 ) ;
    cCalcSecond = cmplx( cBR.real() , dY2 ) ;
    pLine = CreateLineFrame( cCalcFirst ,
      cCalcSecond , 0x00ff00 , "BottomEdge" ) ;
    pView->AddFrame( pLine ) ;
    cCent = (cCalcFirst + cCalcSecond) / 2. ;
    pEdgeCent = CreatePtFrame( cCent , GetHRTickCount() , 0x00ffff , "BottomCent" ) ;
    pEdgeCent->Attributes()->WriteInt( "Sz" , 6 ) ;
    pEdgeCent->Attributes()->WriteInt( "thickness" , 3 ) ;
    pView->AddFrame( pEdgeCent ) ;
  }

  return true ;
}

int FindStraightSegments( const CFigureFrame * pFF , int iGroupIndex ,
  const CRectFrame * pROI , StraightLines& Lines ,
  int iMinStraightLen , double dDiffFromLine_pix , int iNMaxDeviated ,
  FigFrames * pSavedIntSegments , int * piNLastSegments )
{
  cmplx * pCFig = (cmplx*) &pFF->GetAt( 0 ) ;
  ActiveSegments * pInternalSegments ;

  int iMaxStraightSegmentLen = 0 ;
  if ( pROI )
  {
    pInternalSegments = GetInternalSegments( pFF , (CRect*) ((LPRECT) pROI) ) ;
    
    int iWidth = (( CRect* ) ( ( LPRECT ) pROI ))->Width() ;
    int iHeight = ( ( CRect* ) ( ( LPRECT ) pROI ) )->Height() ;
    iMaxStraightSegmentLen = 3 * max( iWidth , iHeight ) ;
  }
  else
  {
    CSegmentInsideROI * pNewSegment = new CSegmentInsideROI;
    pNewSegment->AccountFigure( pFF ) ;
    pInternalSegments = new ActiveSegments ;
    pInternalSegments->Add( pNewSegment ) ;

    int iWidth = (int)(pNewSegment->m_Rect.Width()) ;
    int iHeight = (int)pNewSegment->m_Rect.Height() ;
    iMaxStraightSegmentLen = 3 * max( iWidth , iHeight ) ;
  }
  if ( !pInternalSegments || !pInternalSegments->GetCount() )
    return 0 ;
  int iFigLen = (int) pFF->GetCount()  ; // figure length

  int iNOld = (int) Lines.GetCount() ;

  if ( pSavedIntSegments )
  {
    for ( int i = 0 ; i < (int) pInternalSegments->GetCount() ; i++ )
    {
      CSegmentInsideROI& Segm = *(pInternalSegments->GetAt( i )) ;
      CFigureFrame * pSegmentView = CreateFigureFrame(
        (cmplx*) Segm.GetData() , (int) Segm.GetCount() , GetHRTickCount() ) ;
      pSegmentView->CopyAttributes( pFF ) ;
      pSavedIntSegments->Add( pSegmentView ) ;
    }
  }
  if ( piNLastSegments )
    *piNLastSegments = (int) pInternalSegments->GetCount() ;
  {
  }
  int iStepInside = iMinStraightLen / 2 ;
//   double dAngTolerance = m_dTolerance * 0.001 ;
//   double dFinalTolerance = m_dFinalTolerance * 0.001 ;

  for ( int iIntSegm = 0 ; iIntSegm < pInternalSegments->GetCount() ; iIntSegm++ )
  {
    CSegmentInsideROI& CurrSegm = *(pInternalSegments->GetAt( iIntSegm )) ;
    if ( CurrSegm.GetCount() < iMinStraightLen )
      continue ; // too short segment

    cmplx * pPoints = pCFig ;
    int iInSegmentIndex = CurrSegm.m_iSecondBegin ? CurrSegm.m_iSecondBegin : CurrSegm.m_iIndexBegin ;

    int iMaxInSegmentIndex = CurrSegm.m_iNElements - 1 ;
    int iNRestPtsInSegment = CurrSegm.GetNPoints() ;
    int iLastEndIndex = -1 ;
    do
    {
      int iLastBeginIndex = iInSegmentIndex ;
      int iSavedInSegmentIndex = iInSegmentIndex ;
      int iSecondSide = CurrSegm.m_pFigure->GetNormalizedIndex( iInSegmentIndex + iMinStraightLen ) ;
      int iMiddle = CurrSegm.m_pFigure->GetNormalizedIndex( iInSegmentIndex + iMinStraightLen / 2 ) ;
      while ( (iNRestPtsInSegment > 0) &&
        fabs( GetPtToLineDistance( pPoints[ iMiddle ] , pPoints[ iInSegmentIndex ] ,
        pPoints[ iSecondSide ] ) ) > dDiffFromLine_pix )
      {
        CurrSegm.m_pFigure->IncrementIndex( iInSegmentIndex ) ;
        CurrSegm.m_pFigure->IncrementIndex( iSecondSide ) ;
        CurrSegm.m_pFigure->IncrementIndex( iMiddle );
        iNRestPtsInSegment-- ;
      }
#define TMP_OK_FLAGS_LEN 10000
      bool OK_Flags[ TMP_OK_FLAGS_LEN ] ;
      bool bStraightFound = false ;
      int iStraightBegin = iInSegmentIndex ;
      int iNextIndex = iSecondSide + 1 ;
      int iFirstDeviated = -1 ;
      int iNDeviated = 0 ;
      int iNAfterOmitted = 0 ;
      int iNRestInitial = (iNRestPtsInSegment -= iMinStraightLen) - 1 ;
      int OK_FlagsIndex = 0 ;
      int iNGood = 1 ;
      OK_Flags[ OK_FlagsIndex++ ] = true ;
      int i = CurrSegm.m_pFigure->GetNormalizedIndex( iStraightBegin + 1 ) ;
      while ( i != iSecondSide )
      {
        double dDistToLine = GetPtToLineDistance( pPoints[ i ] , pPoints[ iStraightBegin ] ,
          pPoints[ iSecondSide ] ) ;
        if ( fabs( dDistToLine ) <= dDiffFromLine_pix )
        {
          OK_Flags[ OK_FlagsIndex++ ] = true ;
          iNGood++ ;
        }
        else
          OK_Flags[ OK_FlagsIndex++ ] = false ;
        CurrSegm.m_pFigure->IncrementIndex( i ) ;
        if ( OK_FlagsIndex > iMinStraightLen )
        {
          iNGood = 0 ;
          break ;
        }
      }
      CurrSegm.m_pFigure->IncrementIndex( i );
      int iOverEdgeCounter = 0 ;
      int iSavedEndIndex = -1 ;
      while ( (iNRestPtsInSegment > 0) && (iNGood > 0) )
      {
        bStraightFound = true ;
        double dDistToLine = GetPtToLineDistance( pPoints[ i ] , pPoints[ iStraightBegin ] ,
          pPoints[ iSecondSide ] ) ;
        if ( fabs( dDistToLine ) > dDiffFromLine_pix )
        {
          OK_Flags[ OK_FlagsIndex++ ] = false ;
          if ( iFirstDeviated == -1 )
            iFirstDeviated = iNextIndex  ;
          if ( ++iNDeviated > iNMaxDeviated )
            break ;
        }
        else
        {
          OK_Flags[ OK_FlagsIndex++ ] = true ;
          iNGood++ ;
          if ( iNDeviated )
          {
            if ( ++iNAfterOmitted >= iNMaxDeviated )
            {
              iNAfterOmitted = iNDeviated = 0 ;
              iFirstDeviated = -1 ;
            }
          }
          else
          { 
            if ( iOverEdgeCounter++ == 0 )
            {
              if ( fabs( dDistToLine ) < dDiffFromLine_pix * 0.33 )
                iSavedEndIndex = i ;
              else
                iOverEdgeCounter-- ;
            }
            else if ( iOverEdgeCounter == iNMaxDeviated * 2 )
            {
              if ( iSavedEndIndex > 0 )
                iSecondSide = iSavedEndIndex ;
              iOverEdgeCounter = 0 ;
            }
             

          }
        }
        CurrSegm.m_pFigure->IncrementIndex( i );
        iNRestPtsInSegment-- ;
      }

      if ( bStraightFound )
      {
        int iNPtsInFound = iNRestInitial - iNRestPtsInSegment ;
        int iLeft = CurrSegm.m_pFigure->GetNormalizedIndex( iStraightBegin + iNMaxDeviated ) ;
        int iRight = CurrSegm.m_pFigure->GetNormalizedIndex( iSecondSide - iNMaxDeviated ) ;
        OK_FlagsIndex = iNMaxDeviated ;
        cmplx Left = pPoints[ iLeft ] ;
        cmplx Right = pPoints[ iRight ] ;
        cmplx cDir = Right - Left ;
        int iLast = iRight + 1 ;
        cmplx cFirst , cLast ;

        StraightLine NewLine( Left , Right , iGroupIndex , iLeft , iRight ) ;
        double dSum = 0. ;
        //         CFRegression Regr ;
        for ( int i = iLeft ; i != iRight ; CurrSegm.m_pFigure->IncrementIndex(i) )
        {
          if ( OK_Flags[OK_FlagsIndex++] )
            NewLine.Add( pPoints[ i ] ) ;
        }

        if ( NewLine.m_iNSamples )
        {
          NewLine.Calculate() ;
          for ( int i = iLeft ; i != iRight ; CurrSegm.m_pFigure->IncrementIndex(i) )
          {
            double dDist = NewLine.GetDistFromPt( pPoints[ i ] ) ;
            dSum += dDist * dDist ;
            double dAbsDist = fabs( dDist ) ;
            if ( dAbsDist > NewLine.m_dMaxDiff )
              NewLine.m_dMaxDiff = dAbsDist ;
          }
          NewLine.m_dStd = sqrt( dSum / NewLine.m_iNSamples ) ;
          FXPropKit2 * pLineAttribs = (FXPropKit2*) pFF->Attributes() ;
          if ( pLineAttribs && !pLineAttribs->IsEmpty() )
          {
            pLineAttribs->GetUIntOrHex( "color" , (UINT&) NewLine.m_Color ) ;
            NewLine.m_Color = ~NewLine.m_Color & 0x00ffffff ;
          }
          else
            NewLine.m_Color = 1 ;
          NewLine.m_Begin = NewLine.GetPtOnLine( Left ) ;
          NewLine.m_End = NewLine.GetPtOnLine( Right );
          Lines.Add( NewLine ) ;
        }
        iInSegmentIndex = CurrSegm.m_pFigure->GetNormalizedIndex( iRight + iNMaxDeviated * 2 ) ;
        bStraightFound = false ;
        if ( iNRestPtsInSegment > 0 ) // necessary to continue scan
        {

        }
      }
    } while ( iNRestPtsInSegment >= iMinStraightLen ) ;
    delete pInternalSegments->GetAt( iIntSegm ) ;
  }
  delete pInternalSegments ;
  return (int) Lines.GetCount() ;
}

double GetMostOutlyingPoint( const cmplx& cPtOnLineBegin , const cmplx& cPtOnLineEnd , 
  const CFigure * pFigure , cmplx& cResult , FXSIZE& iMaxIndex , FXSIZE iBeginIndex , FXSIZE iEndIndex )
{
  double dMaxDistPlus = 0. ;
  double dMaxDistMinus = 0. ;
  FXSIZE iIndexPlus = -1 ;
  FXSIZE iIndexMinus = -1 ;
  const cmplx *pcBegin = (const cmplx *)(pFigure->GetData()) ;
  const cmplx * pcIter = pcBegin + iBeginIndex ;
  const cmplx * pcEnd = pcBegin + ((iEndIndex) ? iEndIndex : pFigure->Size()) ;

  while ( pcIter != pcEnd )
  {
    double dDist = GetPtToLineDistance( *pcIter , cPtOnLineBegin , cPtOnLineEnd ) ;
    if ( dDist >= 0 )
    {
      if ( dDist > dMaxDistPlus )
      {
        dMaxDistPlus = dDist ;
        iIndexPlus = pcIter - pcBegin ;
      }
    }
    else
    {
      if ( dDist < dMaxDistMinus ) // dDist is negative!!!
      {
        dMaxDistMinus = dDist ;
        iIndexMinus = pcIter - pcBegin ;
      }
    }
    pcIter++ ;
  }
  if ( dMaxDistPlus >= fabs( dMaxDistMinus ) )
  {
    iMaxIndex = iIndexPlus ;
    cResult = pcBegin[ iMaxIndex ] ;
    return dMaxDistPlus ;
  }
  iMaxIndex = iIndexMinus ;
  cResult = pcBegin[ iMaxIndex ] ;
  return dMaxDistMinus ;
}

int ExtractStraightSegment( const cmplx * pFF , int iLen ,
  int iInitIndex , int iInitialStep ,
  int& iMinIndex , int& iMaxIndex , double dMaxDistortion )
{
  int iMinInd = ( iInitIndex < iInitialStep ) ? 
    iLen + iInitIndex - iInitialStep  : // left point before first contur point
    iInitIndex - iInitialStep ;// normal case

  int iMaxInd = ( iInitIndex + iInitialStep < iLen ) ?
    iInitIndex + iInitialStep // normal case
    : iInitIndex + iInitialStep - iLen ; // right point after end of contur, i.e. in contur begin

  CFRegression Regr ;
  Regr.AddPtsToRegression( pFF , iLen , iMinInd , iMaxInd ) ;
  Regr.Calculate() ;

  CLine2d Line = Regr.GetCLine2d() ;

  double dDeviation = 0 ;
  int iLastAccumMinInd = iMinInd ;
  int iLastAccumMaxInd = iMaxIndex ;
  // Go to left side until big deviation
  do 
  {
    iMinInd = (int)GetIndexInRing( --iMinInd , iLen) ;
    dDeviation = Line.GetAbsDistFromPoint( pFF[iMinInd] ) ;
    if ( dDeviation <= dMaxDistortion )
    {
      Regr.Add( pFF[ iMinInd ] ) ;
      iLastAccumMinInd = iMinInd ;
    }
  } while ( dDeviation < 3. * dMaxDistortion );

//   Regr.Calculate() ;
//   Line = Regr.GetCLine2d() ;
    // Go to right side until big deviation
  do
  {
    iMaxInd = ( int ) GetIndexInRing( ++iMaxInd , iLen ) ;
    dDeviation = Line.GetAbsDistFromPoint( pFF[ iMaxInd ] ) ;
    if ( dDeviation <= dMaxDistortion )
    {
      Regr.Add( pFF[ iMaxInd ] ) ;
      iLastAccumMaxInd = iMaxInd ;
    }
  } while ( dDeviation < 3. * dMaxDistortion );
  Regr.Calculate() ;
  Line = Regr.GetCLine2d() ;

  iMinIndex = iLastAccumMinInd ;
  iMaxIndex = iLastAccumMaxInd ;

  return (int)Regr.Size() ;
}
