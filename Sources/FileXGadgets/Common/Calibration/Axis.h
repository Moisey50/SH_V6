#ifndef __INC__Axis_H__
#define __INC__Axis_H__

#include "stdafx.h"

#include "Reservuar.h"

enum FindUpDownResult
{
  PT_NOT_FOUND = 0 ,
  PT_UP_FOUND = 1 ,
  PT_DOWN_FOUND = 2 ,
  
  PT_FIND_ERROR_UP = 0x1000 ,
  PT_FIND_ERROR_DOWN = 0x2000
};

static double g_dMaxAngleDeviation = 4.0 ;

inline Node1D* getNextLineSpot( const Node1DList* nodeList ,
  const Node1D* previous , const Node1D* current , const double dPosTol ,
  int iMaxOmitted , CalibSpotVector * pArtificial , UINT uiAxis )
{
  if ( !previous || !current || !nodeList )
    return NULL;

  cmplx Delta = current->m_Spot->m_imgCoord - previous->m_Spot->m_imgCoord ;

  Node1D* node = NULL;//nodeList->getNextLineNode( *(current->m_Spot), Delta , dPosTol);
  for ( int i = 1 ; i <= iMaxOmitted + 1 ; i++ )
  {
    cmplx NextDelta = Delta * (double) i ;
    node = nodeList->getNextLineNode( *(current->m_Spot) , NextDelta , dPosTol * i );
    if ( node )
    {
      if ( i > 1 )
      {
        if ( pArtificial ) // space between spot is more than one interval
        {                  // necessary to insert into artificial list   
          cmplx DeltaOmitted = (node->m_Spot->m_imgCoord - current->m_Spot->m_imgCoord) / (double) i ;
          for ( int j = 1 ; j < i ; j++ )
          {
            cmplx ForInsert( current->m_Spot->m_imgCoord + DeltaOmitted * (double) j ) ;
            Spot * pNewSpot = new Spot( -1 , ForInsert , 0. , 0. , uiAxis ) ;
            pNewSpot->m_Indexes = current->m_Spot->m_Indexes ;
            if (uiAxis == AXIS_0)
              pNewSpot->m_Indexes.x++ ;
            else
              pNewSpot->m_Indexes.y++ ;
            pArtificial->push_back( pNewSpot ) ;
            node = new Node1D( pNewSpot ) ;
          }
        }
      }
      break ;
    }
    if ( pArtificial ) // not found, go to Artificial array
    {
      cmplx Pos = current->m_Spot->m_imgCoord + NextDelta ;
      Spot * pFound = Find( *pArtificial , Pos , dPosTol * i ) ;
      if ( pFound )
      {
        node = new Node1D( pFound ) ;
        break ; // found in artificial spots 
      }
    }
  }
  return node;
}

inline Node1D* getNextLineSpot( const Node1DList* nodeList ,
  const Node1D* current , cmplx Delta , const double dPosTol ,
  int iMaxOmitted , CalibSpotVector * pArtificial , UINT uiAxis )
{
  if (!current || !nodeList)
    return NULL;

  Node1D* node = NULL;//nodeList->getNextLineNode( *(current->m_Spot), Delta , dPosTol);
  for (int i = 1 ; i <= iMaxOmitted + 1 ; i++)
  {
    if ( (dPosTol * i ) > 0.5)
      break ;
    cmplx NextDelta = Delta * ( double ) i ;
    node = nodeList->getNextLineNode( *( current->m_Spot ) , NextDelta , dPosTol /** i*/ );
    if (node)
    {
      if (i > 1)
      {
        if (pArtificial) // space between spot is more than one interval
        {                  // necessary to insert into artificial list   
          cmplx DeltaOmitted = ( node->m_Spot->m_imgCoord - current->m_Spot->m_imgCoord ) / ( double ) i ;
          for (int j = 1 ; j < i ; j++)
          {
            cmplx ForInsert( current->m_Spot->m_imgCoord + DeltaOmitted * ( double ) j ) ;
            Spot * pNewSpot = new Spot( -1 , ForInsert , 0. , 0. , uiAxis ) ;
            pNewSpot->m_Indexes = current->m_Spot->m_Indexes ;
            if (uiAxis == AXIS_0)
              pNewSpot->m_Indexes.x++ ;
            else
              pNewSpot->m_Indexes.y++ ;
            pArtificial->push_back( pNewSpot ) ;
            node = new Node1D( pNewSpot ) ;
          }
        }
      }
      break ;
    }
    if (pArtificial) // not found, go to Artificial array
    {
      cmplx Pos = current->m_Spot->m_imgCoord + NextDelta ;
      Spot * pFound = Find( *pArtificial , Pos , dPosTol * i ) ;
      if (pFound)
      {
        node = new Node1D( pFound ) ;
        break ; // found in artificial spots 
      }
    }
  }
  return node;
}


class Axis : public Node1DList
{
  cmplx  m_MainVector ; // one step between nodes as vector from first in FOV
  cmplx  m_CentMainVector ; // the same for central spot
  double m_dArg ;       // direction of this vector in FOV
  double m_dStep ;      // distance between nodes in FOV
  cmplx  m_WMainVector ;    // one step between nodes as vector from first in World
  double m_dWArg ;
  double m_dWStep ;
  double m_dImgToWrldAng ;  // Angle between Img and World
  double m_dImgToWrldScale ;// Scale from image to world

public:
  Axis( Spot * pInitialSpot = NULL )
    : m_dArg( 0. )
    , m_dStep( 0. )
  {
    if ( pInitialSpot )
    {
      insert( pInitialSpot ) ;
    }
  }
  inline cmplx& GetMainVector()
  {
    return m_MainVector ;
  }
  inline void SetMainVector( cmplx& Vect )
  {
    m_MainVector = Vect ;
    m_dArg = (abs( Vect ) > 0.) ? arg( m_MainVector ) : 0. ;
    m_dStep = abs( m_MainVector ) ;
    if ( m_dStep && m_dWStep )
    {
      m_dImgToWrldAng = m_dArg - m_dWArg ;
      m_dImgToWrldScale = m_dWStep / m_dStep ;
    }
    else
    {
      m_dImgToWrldAng = 0. ;
      m_dImgToWrldScale = 1.0 ;
    }
  }
  inline void SetWMainVector( cmplx& Vect )
  {
    m_WMainVector = Vect ;
    m_dWStep = abs( m_WMainVector ) ;
    m_dWArg = (m_dWStep > 0.) ? arg( m_WMainVector ) : 0. ;
    if ( m_dStep && m_dWStep )
    {
      m_dImgToWrldAng = m_dArg - m_dWArg ;
      m_dImgToWrldScale = m_dWStep / m_dStep ;
    }
    else
    {
      m_dImgToWrldAng = 0. ;
      m_dImgToWrldScale = 1.0 ;
    }
  }
  inline double GetArg()
  {
    return m_dArg ;
  }
  inline double GetStep()
  {
    return m_dStep ;
  }
  inline double GetImgToWrldAng()
  {
    return m_dImgToWrldAng ;
  }
  inline double GetImgToWrldScale()
  {
    return m_dImgToWrldScale ;
  }
  inline cmplx& GetWMainVector()
  {
    return m_WMainVector ;
  }

  inline bool CalcMainVectors()
  {
    if ( getCnt() >= 2 )
    {
      cmplx MainVect = (GetTail()->m_Spot->m_imgCoord - GetHead()->m_Spot->m_imgCoord) ;
      MainVect /= (double) (getCnt() - 1) ;
      SetMainVector( MainVect ) ;
      cmplx WMainVect = (GetTail()->m_Spot->m_WCoord - GetHead()->m_Spot->m_WCoord) ;
      WMainVect /= (double) (getCnt() - 1) ;
      SetWMainVector( WMainVect ) ;
      return true ;
    }
    return false ;
  }
  inline bool CalcMainVector( Node1D * pCenterNode , Node1D * pCenterOnSecondAxis )
  {
    try
    {
      bool bRes = true ;
      cmplx MainVector ;
      cmplx MainVectorWorld ;
      Node1D * pCenter = find( pCenterNode->m_Spot->m_imgCoord , 3.0 ) ;
      Node1D * pOtherCenter = find( pCenterOnSecondAxis->m_Spot->m_imgCoord , 3.0 ) ;
      // check, that second center node is on this axis
      if ( pCenter == pOtherCenter )
      {  // if yes, do calculation directly
        Node1D * pNext = pCenter->next ;
        if ( !pNext ) // Axis is going in revers order
        {
          Reverse() ;
          pNext = pCenter->next ;
          ASSERT( pNext ) ;
        }
        MainVector = pNext->m_Spot->m_imgCoord - pCenterNode->m_Spot->m_imgCoord ;
        MainVectorWorld = pNext->m_Spot->m_WCoord - pCenterNode->m_Spot->m_WCoord ;
      }
      else if ( GetHead()->m_Spot->m_imgCoord == pCenterNode->m_Spot->m_imgCoord )
      {
        MainVector = (GetTail()->m_Spot->m_imgCoord
          - GetHead()->m_Spot->m_imgCoord) / (double) (m_uiCnt - 1) ;
        MainVectorWorld = (GetTail()->m_Spot->m_WCoord
          - GetHead()->m_Spot->m_WCoord) / (double) (m_uiCnt - 1) ;
      }
      else if ( GetTail()->m_Spot->m_imgCoord == pCenterNode->m_Spot->m_imgCoord )
      {
        MainVector = (GetHead()->m_Spot->m_imgCoord
          - GetTail()->m_Spot->m_imgCoord) / (double) (m_uiCnt - 1) ;
        MainVectorWorld = (GetHead()->m_Spot->m_WCoord
          - GetTail()->m_Spot->m_WCoord) / (double) (m_uiCnt - 1);
        Reverse() ;
      }
      else
      {
        Node1D * pNode = GetHead() ;
        UINT iPosCnt = 0 ;
        while ( pNode )
        {
          if ( pNode->m_Spot->m_imgCoord == pCenterNode->m_Spot->m_imgCoord )
          {
            if ( iPosCnt < m_uiCnt / 2 )
            {
              MainVector = (GetTail()->m_Spot->m_imgCoord
                - GetHead()->m_Spot->m_imgCoord) / (double) (m_uiCnt - 1) ;
              MainVectorWorld = (GetTail()->m_Spot->m_WCoord
                - GetHead()->m_Spot->m_WCoord) / (double) (m_uiCnt - 1) ;
            }
            else if ( iPosCnt > m_uiCnt / 2 )
            {
              MainVector = (GetHead()->m_Spot->m_imgCoord
                - GetTail()->m_Spot->m_imgCoord) / (double) (m_uiCnt - 1) ;
              MainVectorWorld = (GetHead()->m_Spot->m_WCoord
                - GetTail()->m_Spot->m_WCoord) / (double) (m_uiCnt - 1);
              Reverse() ;
            }
            else if ( m_uiCnt & 1 ) // bad, full symmetry, we don't know axis direction
            {
              MainVector = (GetTail()->m_Spot->m_imgCoord
                - GetHead()->m_Spot->m_imgCoord) / (double) (m_uiCnt - 1) ;
              MainVectorWorld = (GetTail()->m_Spot->m_WCoord
                - GetHead()->m_Spot->m_WCoord) / (double) (m_uiCnt - 1) ;
              // do usual for image processing direction
              if ( fabs( MainVector.real() ) > fabs( MainVector.imag() ) )
              {
                if ( MainVector.real() < 0 )
                {
                  MainVector = -MainVector ;
                  MainVectorWorld = -MainVectorWorld ;
                  Reverse() ;
                }
              }
              else
              {
                if ( MainVector.imag() < 0 )
                {
                  MainVector = -MainVector ;
                  MainVectorWorld = -MainVectorWorld ;
                  Reverse() ;
                }
              }
              bRes = false ;
            }
            else
            {
              MainVector = (GetHead()->m_Spot->m_imgCoord
                - GetTail()->m_Spot->m_imgCoord) / (double) (m_uiCnt - 1) ;
              MainVectorWorld = (GetHead()->m_Spot->m_WCoord
                - GetTail()->m_Spot->m_WCoord) / (double) (m_uiCnt - 1);
              Reverse() ;
            }
            break ;
          }
          pNode = pNode->next ;
          iPosCnt++ ;
        }
      }
      SetMainVector( MainVector ) ;
      SetWMainVector( MainVectorWorld ) ;
      return bRes ;
    }
    catch ( int  )
    {
      return false ;
    }
    return false ;
  }

  void EnumerateAxis( const cmplx& CentralPt , const cmplx& CentralWCoord ,
    const cmplx& AbsStep , CPoint Indexes )
  {
    Node1D * pCenter = find( CentralPt ) ;
    if ( pCenter && (abs( AbsStep ) != 0.) )
    {
      m_WMainVector = AbsStep ;
      Spot * pSpot = pCenter->m_Spot ;
      pSpot->m_WCoord = CentralWCoord ;
      pSpot->m_Indexes = Indexes ;
      CSize Step( AbsStep.real() != 0. , AbsStep.imag() != 0. ) ;
      Node1D * pNext = pCenter->next ;
      cmplx NextPos = CentralWCoord  ;
      while ( pNext )
      {
        pNext->m_Spot->m_WCoord = (NextPos += AbsStep) ;
        pNext->m_Spot->m_Indexes = Indexes = Indexes + Step ;
        pNext = pNext->next ;
      }
      Node1D * pPrev = pCenter->previous ;
      NextPos = CentralWCoord  ;
      Indexes = pCenter->m_Spot->m_Indexes ;
      while ( pPrev )
      {
        pPrev->m_Spot->m_WCoord = (NextPos += -AbsStep) ;
        pPrev->m_Spot->m_Indexes = Indexes = Indexes - Step ;
        pPrev = pPrev->previous ;
      }
    }
    else
      ASSERT( 0 ) ;
  }

  bool SaveAxis( FILE *fw , TCHAR * pPrefix = NULL , TCHAR * pSuffix = NULL )
  {
    if ( !fw || !m_pHead )
      return false ;

    FXString Result ;

    if ( SaveAxis( Result , pPrefix , pSuffix ) )
    {
      _ftprintf( fw , _T( "%s" ) , (LPCTSTR) Result ) ;
      return true ;
    }
    return false ;
  }

  bool SaveAxis( FXString& Result , TCHAR * pPrefix = NULL , TCHAR * pSuffix = NULL )
  {
    if ( !m_pHead )
      return false ;

    if ( pPrefix )
      Result += pPrefix ;

    Node1D * pNext = m_pHead ;
    while ( pNext )
    {
      bool bRes = pNext->m_Spot->SaveCalibData( Result , _T( "\r\n" ) ) ;
      ASSERT( bRes ) ;
      if ( !bRes )
        return false ;
      pNext = pNext->next ;
    }
    if ( pSuffix )
      Result += pSuffix ;
    return true ;
  }

  bool RestoreAxis( FILE * fr , Reservuar * pSpots )
  {
    TCHAR Symb ;
    FXSIZE iNRead = 0 ;
    int iNSpots = 0 ;
    do
    {
      iNRead = fread_s( &Symb , sizeof( Symb ) , sizeof( Symb ) , 1 , fr ) ;
    } while ( (iNRead > 0) && (Symb != _T( '<' )) );
    if ( !iNRead )
      return false ;
    // pass CRLF
    TCHAR CRLF[ 2 ] ;
    fread_s( CRLF , sizeof( CRLF ) , sizeof( CRLF ) , 1 , fr ) ;
    Spot * pSpot = NULL ;
    do
    {
      pSpot = new Spot ;
      if ( pSpot )
      {
        int iRes = pSpot->RestoreCalibData( fr ) ;
        if ( iRes > 0 )
        {
          Node1D * pNodeInGlobal = pSpots->GetNodeList()->find( pSpot->m_imgCoord ) ;
          if ( !pNodeInGlobal )
          {
            pSpots->GetNodeList()->AddToTail( pSpot ) ; // add to reservuar new spot
            AddToTail( pSpot ) ; // add to this axis
            ASSERT( 0 ) ;
          }
          else
          {
            AddToTail( pNodeInGlobal->m_Spot ) ; // add to this axis
            delete pSpot ;
          }
          iNSpots++ ;
        }
        else
        {
          delete pSpot ;
          break ;
        }
      }
    } while ( pSpot );
    if ( m_uiCnt > 1 )
    {
      cmplx Begin = m_pHead->m_Spot->m_imgCoord ;
      cmplx End = m_pTail->m_Spot->m_imgCoord ;
      cmplx MainVector = (End - Begin) / (double) (m_uiCnt - 1) ;
      SetMainVector( MainVector ) ;
      Begin = m_pHead->m_Spot->m_WCoord ;
      End = m_pTail->m_Spot->m_WCoord ;
      MainVector = (End - Begin) / (double) (m_uiCnt - 1) ;
      SetWMainVector( MainVector ) ;
    }
    return true ;
  }

  inline int FindUpDown( cmplx& Pt , cmplx& DirUp ,
    double dAbsTol , cmplx& PtUp , cmplx& PtDown ,
    Reservuar * Spots , CalibSpotVector * pArtificial )
  {
    int iRes = 0 ;
    cmplx PtUpPos = Pt + DirUp ;
    Spot * pUp = Spots->FindSpot( PtUpPos , dAbsTol ) ;
    if ( pUp || (pUp = Find( *pArtificial , PtUpPos , dAbsTol )) )
    {
      iRes |= PT_UP_FOUND ;
      PtUp = pUp->m_imgCoord - Pt ;
    }
    else
      PtUp = cmplx( 0. , 0. ) ;

    cmplx PtDownPos = Pt - DirUp ;
    Spot * pDown = Spots->FindSpot( PtDownPos , dAbsTol ) ;
    if ( pDown || (pDown = Find( *pArtificial , PtUpPos , dAbsTol )) )
    {
      iRes |= PT_DOWN_FOUND ;
      PtDown = pDown->m_imgCoord - Pt ;
    }
    else
      PtDown = cmplx( 0. , 0. ) ;
    return iRes ;
  }

  inline int FindUpDown( Spot * Pt , cmplx& DirUp , 
    double dAbsTol , Reservuar * Spots , CalibSpotVector * pArtificial )
  {
    FXString Diagnostics ;
    Pt->m_DirWN = m_WMainVector * cmplx( 0. , 1. ) ;
    Pt->m_DirWS = -Pt->m_DirWN ;
    int iRes = 0 ;
    Spot * pUp = Pt->m_SpotN ;
    if ( !pUp )
    {
      cmplx PtUpPos = Pt->m_imgCoord + DirUp ;
      pUp = Spots->FindSpot( PtUpPos , dAbsTol ) ;
      bool bRealUp = ( pUp != NULL ) ;
      if ( pUp || ( pUp = Find( *pArtificial , PtUpPos , dAbsTol ) ) )
      {
        iRes |= PT_UP_FOUND ;
        Pt->m_SpotN = pUp ;
        Pt->m_DirN = pUp->m_imgCoord - Pt->m_imgCoord ;
        pUp->m_SpotS = Pt ;
        pUp->m_DirS = -Pt->m_DirN ;
        pUp->m_DirWS = -Pt->m_DirWN ;
      }
      else
      {
        Pt->m_SpotN = NULL ;
        Pt->m_DirN = DirUp ;
      }
    }
    else
      iRes |= PT_UP_FOUND ;

    if ( pUp )
    {
      CPoint UpIndexes = Pt->m_Indexes + CPoint( 0 , 1 ) ;
#ifdef _DEBUG
      if ( ( pUp->m_Indexes.x != INT_MAX ) || ( pUp->m_Indexes.y != INT_MAX ) )
      {
        if ( pUp->m_Indexes != UpIndexes )
          return iRes |= PT_FIND_ERROR_UP ;
      }
#endif
      pUp->m_Indexes = UpIndexes ;
//       cmplx cUpWorld = Pt->m_WCoord + Pt->m_DirWN ;
//       double dAbsWorld = abs( cUpWorld ) ;
//       if ( abs( pUp->m_WCoord ) != 0. )
//       {
//         if ( dAbsWorld != 0. )
//           ASSERT( (abs( cUpWorld - pUp->m_WCoord ) / dAbsWorld) < 0.01 )  ;
//       }
//       else
//         pUp->m_WCoord = cUpWorld ;
    }


    Spot * pDown = Pt->m_SpotS ;

    if ( !pDown )
    {
      cmplx PtDownPos = Pt->m_imgCoord - DirUp ;
      pDown = Spots->FindSpot( PtDownPos , dAbsTol ) ;
      if ( pDown || ( pDown = Find( *pArtificial , PtDownPos , dAbsTol ) ) )
      {
        iRes |= PT_DOWN_FOUND ;
        Pt->m_SpotS = pDown ;
        Pt->m_DirS = pDown->m_imgCoord - Pt->m_imgCoord ;
        pDown->m_SpotN = Pt ;
        pDown->m_DirN = -Pt->m_DirS ;
        pDown->m_DirWN = -Pt->m_DirWS ;
      }
      else
      {
        Pt->m_SpotS = NULL ;
        if ( abs( Pt->m_DirS ) == 0. )
          Pt->m_DirS = -DirUp ;
      }
    }

    if ( pDown )
    {
      CPoint DownIndexes = Pt->m_Indexes + CPoint( 0 , -1 ) ;
#ifdef _DEBUG
      if ( ( pDown->m_Indexes.x != INT_MAX ) || ( pDown->m_Indexes.y != INT_MAX ) )
      {
         if ( pDown->m_Indexes != DownIndexes )
            return iRes |= PT_FIND_ERROR_DOWN ;
      }
#endif
      pDown->m_Indexes = DownIndexes ;
//       cmplx cDownWorld = Pt->m_WCoord - Pt->m_DirWN ;
//       double dAbsWorld = abs( cDownWorld ) ;
//       if ( abs( pDown->m_WCoord ) != 0. )
//       {
//         if ( dAbsWorld != 0. )
//           ASSERT( (abs( cDownWorld - pDown->m_WCoord ) / dAbsWorld) < 0.01 )  ;
//       }
//       else
//         pDown->m_WCoord = cDownWorld ;
      if ( !pUp )
      {
        Pt->m_DirN = -Pt->m_DirS ;
        Pt->m_DirWN = -Pt->m_DirWS ;
      }
    }

    if ( pUp && !pDown )
    {
      Pt->m_DirS = -Pt->m_DirN ;
      Pt->m_DirWS = -Pt->m_DirWN ;
    }
    return iRes ;
  }

  inline int ExpandHorAxis( Reservuar*  pAllSpots , double dPosTol ,
    int iMaxOmitted , CalibSpotVector * pArtificial ,
    Spot * pCenterSpot , cmplx DirUp = cmplx( 0. , 0. ) )
  {
    FXString Diagnostics ;
    UINT uiBefore = getCnt() ;
    Node1D * pFirst = GetHead() ;
    Node1D * pSecond = pFirst->next ;
    cmplx cNorth = m_WMainVector * cmplx( 0. , 1.0 ) ; 
    double dAbsTol = dPosTol * abs( pFirst->m_Spot->m_imgCoord
      - pSecond->m_Spot->m_imgCoord ) ;

    Node1D * pIterKnown = pFirst ;
    bool bFound = false ;
    while ( pIterKnown != GetTail() )
    {
      if ( pIterKnown->m_Spot == pCenterSpot )
      {
        bFound = true ;
        break ;
      }
      else
        pIterKnown = pIterKnown->next ;
    }

    if ( !bFound )
      return 0 ;

//     if ( pIterKnown != pFirst ) // center is not on first spot
//       ASSERT( 0 ) ;
    pIterKnown->m_Spot->m_SpotW = NULL ;
    do
    {
      Node1D * pOnRight = pIterKnown->next ;
      Spot * pKnownSpot = pIterKnown->m_Spot ;
      Spot * pRightSpot = pOnRight->m_Spot ;
      pKnownSpot->m_SpotE = pRightSpot ;
      pRightSpot->m_SpotW = pKnownSpot ;
      pKnownSpot->m_DirE = pRightSpot->m_imgCoord - pKnownSpot->m_imgCoord ;
      if ( pIterKnown == pFirst )
        pKnownSpot->m_DirW = -pKnownSpot->m_DirE ;

      pKnownSpot->SetWorldSteps( m_WMainVector , cNorth ) ;
      pRightSpot->m_DirW = -pKnownSpot->m_DirE ;
      pRightSpot->SetWorldSteps( m_WMainVector , cNorth ) ;
      CPoint RightIndexX( pKnownSpot->m_Indexes + CPoint( 1 , 0 ) ) ;
      CPoint RightIndexY( pRightSpot->m_Indexes );
//       if ( pRightSpot->m_Indexes != RightIndexX )
//       {
        if ( pRightSpot->m_SpotN && ( pRightSpot->m_SpotN->m_Indexes.x != INT_MAX ) )
        {
          RightIndexY = pRightSpot->m_SpotN->m_Indexes + CPoint( 0 , -1 ) ;
        }
//       }
      ASSERT( RightIndexX == pRightSpot->m_Indexes ) ;
      ASSERT( RightIndexY == pRightSpot->m_Indexes ) ;
      //pRightSpot->m_Indexes = RightIndexX ;
      if ( abs( DirUp ) == 0. )
        DirUp = pKnownSpot->m_DirE * cmplx( 0. , 1. ) ;
      int iRes = FindUpDown( pKnownSpot ,  DirUp , dAbsTol , pAllSpots , pArtificial ) ;
      if ( iRes >= PT_FIND_ERROR_UP )
      {
        Diagnostics += ToString() ;
        ASSERT( 0 ) ;
      }
      if ( iRes & PT_UP_FOUND )
        DirUp = pKnownSpot->m_DirN ;
      else if ( iRes & PT_DOWN_FOUND )
        DirUp = -pKnownSpot->m_DirS ;
//       pKnownSpot->m_OnLines |= AXIS_X ;
//       pRightSpot->m_OnLines |= AXIS_X ;
      pIterKnown = pOnRight ;
    } while ( pIterKnown != GetTail() ) ;
    Spot * pKnownSpot = pIterKnown->m_Spot ;
    int iRes = FindUpDown( pKnownSpot , DirUp , 
      dAbsTol , pAllSpots , pArtificial ) ;
    if ( iRes >= PT_FIND_ERROR_UP )
    {
      Diagnostics += ToString() ;
      ASSERT( 0 ) ;
    }
    if ( iRes & PT_UP_FOUND )
      DirUp = pKnownSpot->m_DirN ;
    else if ( iRes & PT_DOWN_FOUND )
      DirUp = -pKnownSpot->m_DirS ;
    pKnownSpot->SetWorldSteps( m_WMainVector , cNorth ) ;
    pKnownSpot->m_SpotE = NULL ;
    pKnownSpot->m_DirE = -pKnownSpot->m_DirW ;
    Node1D * pNext ;
    const Node1DList * pAllNodes = pAllSpots->GetNodeList() ;
    DirUp = pFirst->m_Spot->m_DirN ;
    Spot * pFirstInRow = pFirst->m_Spot ;

    // Check for not too big deviation from axis Main Vector
    cmplx cDelta = pFirst->m_Spot->m_imgCoord - pSecond->m_Spot->m_imgCoord ;
    if ( abs(m_MainVector) > 0. )
    {
      double dAng = RadToDeg(arg( -cDelta / m_MainVector )) ;
      if (dAng < -g_dMaxAngleDeviation || dAng > g_dMaxAngleDeviation)
        cDelta = -m_MainVector ;
    }

    // find spots before first 
    while ( pNext = getNextLineSpot( pAllNodes , pFirst ,
      cDelta , dPosTol , iMaxOmitted , pArtificial , AXIS_0 ) )
    {
      Spot * pNextSpot = pNext->m_Spot ; // spot on the left side
      Spot * pBackSpot = pFirst->m_Spot ;
      if ((pNextSpot->GetGlobalIndex() < 0) && (pBackSpot == pFirstInRow) )
        break ;
      // Set world coordinate
      pNextSpot->m_WCoord = pBackSpot->m_WCoord - m_WMainVector ;
      // Set Indexes
      CPoint NextIndex( pBackSpot->m_Indexes + CPoint( -1 , 0 ) ) ;
    #ifdef _DEBUG
      if ( (pNextSpot->m_Indexes.x != INT_MAX ) || (pNextSpot->m_Indexes.y != INT_MAX ) )
      {
        //ASSERT( pNextSpot->m_Indexes == NextIndex ) ;
        if ( pNextSpot->m_Indexes != NextIndex )
        {
          Diagnostics += ToString() ;
          return 0 ;
          //ASSERT( 0 ) ;
        }
      }
    #endif
      pNextSpot->m_Indexes = NextIndex ;
      // Set directional spots and differences in image and world coordinates
      pBackSpot->m_SpotW = pNextSpot ;
      pNextSpot->m_SpotE = pBackSpot ;
      pBackSpot->m_DirW = pNextSpot->m_imgCoord - pBackSpot->m_imgCoord ;
      pNextSpot->m_DirE = -pBackSpot->m_DirW ;
      pNextSpot->SetWorldSteps( m_WMainVector , cNorth ) ;
      pBackSpot->SetWorldSteps( m_WMainVector , cNorth ) ;

      int iRes = FindUpDown( pNextSpot , DirUp , 
        dAbsTol , pAllSpots , pArtificial ) ;
      pNextSpot->m_OnLines |= AXIS_X ;
      AddToHead( pNext->m_Spot ) ;
      if ( iRes & PT_UP_FOUND )
        DirUp = pBackSpot->m_DirN ;
      else if ( iRes & PT_DOWN_FOUND )
        DirUp = -pBackSpot->m_DirS ;
      ASSERT( abs( DirUp ) ) ;

      pFirst = pNext ;
    }
    Spot * pLeft = GetHead()->m_Spot ;
    pLeft->m_SpotW = NULL ;
    pLeft->m_DirW = -pLeft->m_DirE ;
    int iCnt3 = pAllSpots->GetNodeList()->GetRealLength() ;
    Node1D * pLast = GetTail() ;
    Node1D * pBeforeLast = pLast->previous ;
    DirUp = GetTail()->m_Spot->m_DirN ;

    // Check for not too big deviation from axis Main Vector
    cDelta = pLast->m_Spot->m_imgCoord - pBeforeLast->m_Spot->m_imgCoord ;
    if (abs( m_MainVector ) > 0.)
    {
      double dAng = RadToDeg( arg( cDelta / m_MainVector ) ) ;
      if (dAng < -g_dMaxAngleDeviation || dAng > g_dMaxAngleDeviation)
        cDelta = m_MainVector ;
    }

    // find nodes after last
    while ( pNext = getNextLineSpot( pAllNodes , pLast ,
      cDelta , dPosTol , iMaxOmitted , pArtificial , AXIS_0 ) )
    {
      Spot * pNextSpot = pNext->m_Spot ; // spot on the right side
      Spot * pLastSpot = pLast->m_Spot ;
      // Set world coordinate
      pNextSpot->m_WCoord = pLastSpot->m_WCoord + m_WMainVector ;
      // Set Indexes
      CPoint RightIndex( pLastSpot->m_Indexes + CPoint( 1 , 0 ) ) ;
    #ifdef _DEBUG
      if ( ( pNextSpot->m_Indexes.x != INT_MAX ) || ( pNextSpot->m_Indexes.y != INT_MAX ) )
      {
        if ( pNextSpot->m_Indexes != RightIndex )
        {
          Diagnostics += ToString() ;
          return 0 ;
          //ASSERT( 0 ) ;
        }
      }
    #endif
      pNextSpot->m_Indexes = RightIndex ;
      // Set directional spots and differences in image and world coordinates
      pLastSpot->m_SpotE = pNextSpot ;
      pNextSpot->m_SpotW = pLastSpot ;
      pLastSpot->m_DirE = pNextSpot->m_imgCoord - pLastSpot->m_imgCoord ;
      pNextSpot->m_DirW = -pLastSpot->m_DirE ;
      pLastSpot->SetWorldSteps( m_WMainVector , cNorth ) ;
      pNextSpot->SetWorldSteps( m_WMainVector , cNorth ) ;

      int iRes = FindUpDown( pNextSpot , DirUp ,
        dAbsTol , pAllSpots , pArtificial ) ;
      pNextSpot->m_OnLines |= AXIS_X ;
      AddToTail( pNext->m_Spot ) ;
      if ( iRes & PT_UP_FOUND )
        DirUp = pLastSpot->m_DirN ;
      else if ( iRes & PT_DOWN_FOUND )
        DirUp = -pLastSpot->m_DirS ;
      ASSERT( abs( DirUp ) ) ;
      pBeforeLast = pLast ;
      pLast = pNext ;
    }
    Spot * pRight = GetTail()->m_Spot ;
    pRight->m_SpotE = NULL ;
    pRight->m_DirE = -pRight->m_DirW ;
    pRight->SetWorldSteps( m_WMainVector , cNorth ) ;
    iRes = FindUpDown( pRight , DirUp ,
      dAbsTol , pAllSpots , pArtificial ) ;
    cmplx FullDist = pRight->m_imgCoord - GetHead()->m_Spot->m_imgCoord ;
    cmplx MainVect = (FullDist / (double) (m_uiCnt - 1)) ;
    SetMainVector( MainVect ) ;
    cmplx FullDistWorld = pRight->m_WCoord - GetHead()->m_Spot->m_WCoord ;
    cmplx WMainVect = FullDistWorld / (double) (m_uiCnt - 1) ;
    SetWMainVector( WMainVect ) ;

    return getCnt() - uiBefore ;
  }

  inline bool FindNeighborsOnHLine( Node1DList*  pAllNodes ,
    double dPosTol , CalibSpotVector& Artificial ,
    cmplx& DefEast , cmplx& DefUp , cmplx& DefDown )
  {
    cmplx DirUp( DefUp ) ;
    cmplx DirDown( DefDown ) ;
    cmplx DirEast( DefEast ) ;
    cmplx DirWest( -DirEast ) ;
    UINT uiBefore = getCnt() ;
    Node1D * pFirst = GetHead() ;
    Node1D * pSecond = pFirst->next ;
    if ( !pSecond )
    {
      pFirst->m_Spot->m_DirE = DirEast ;
      pFirst->m_Spot->m_DirW = DirWest ;
      pFirst->m_Spot->m_DirN = DirUp ;
      pFirst->m_Spot->m_DirS = DirDown ;
      return true ;
    }
    Node1D * pNext = pSecond ;
    UINT uiCnt = 2 ;
    bool bIsHorHeghbors = true ;
    double dDist = 0. ;
    Node1D * pLast = pNext ;
    if ( getCnt() > 2 )
    {
      while ( pNext && uiCnt++ < getCnt() / 2 )
      {
        pNext = pNext->next ;
        if ( pNext )
          pLast = pNext ;
      }
    }
    pNext = pNext->previous ;
    while ( pNext )
    {
      cmplx NextICoord = pNext->m_Spot->m_imgCoord ;
      if ( pNext->next )
      {
        DirEast = pNext->m_Spot->m_DirE = pNext->next->m_Spot->m_imgCoord - NextICoord ;
        dDist = abs( pNext->m_Spot->m_DirE ) ;
        if ( pNext->previous )
        {
          DirWest = pNext->m_Spot->m_DirW = pNext->previous->m_Spot->m_imgCoord - NextICoord ;
          dDist = 0.5 * (dDist + abs( pNext->m_Spot->m_DirW )) ;
        }
        else
          DirWest = pNext->m_Spot->m_DirW = -pNext->m_Spot->m_DirE ;
      }
      else if ( pNext->previous )
      {
        DirWest = pNext->m_Spot->m_DirW = pNext->previous->m_Spot->m_imgCoord - NextICoord ;
        DirEast = pNext->m_Spot->m_DirE = -pNext->m_Spot->m_DirW ;
        dDist = abs( pNext->m_Spot->m_DirE ) ;
      }
      else // no spot on left side and no spot on right side 
      {
        bIsHorHeghbors = false ;
        ASSERT( 0 ) ;
        pNext->m_Spot->m_DirE = DirEast ;
        pNext->m_Spot->m_DirW = DirWest ;
      }
      double dAbsTol = dDist * dPosTol ;
      cmplx UpTarget = NextICoord + DirUp ;
      Node1D * pUp = pAllNodes->find( UpTarget , dAbsTol ) ;
      Spot * pUpSpot = (pUp) ? pUp->m_Spot : Find( Artificial , UpTarget , dAbsTol ) ;
      cmplx DownTarget = NextICoord + DirDown ;
      Node1D * pDown = pAllNodes->find( DownTarget , dAbsTol ) ;
      Spot * pDownSpot = (pDown) ? pDown->m_Spot : Find( Artificial , DownTarget , dAbsTol ) ;
      if ( pUpSpot )
      {
        DirUp = pNext->m_Spot->m_DirN = pUpSpot->m_imgCoord - NextICoord ;
        if ( pDownSpot )
          DirDown = pNext->m_Spot->m_DirS = pDownSpot->m_imgCoord - NextICoord ;
        else
          pNext->m_Spot->m_DirS = DirDown ;
      }
      else if ( pDownSpot )
      {
        DirDown = pNext->m_Spot->m_DirS = pDownSpot->m_imgCoord - NextICoord ;
        DirUp = pNext->m_Spot->m_DirN = -pNext->m_Spot->m_DirS ;
      }
      else // no vertical neighbors
      {
        pNext->m_Spot->m_DirN = DirUp ;
        pNext->m_Spot->m_DirS = DirDown ;
      }
      pNext = pNext->next ;
    }

    Node1D * pPrev = pLast->previous ;
    DirUp = DefUp  ;
    DirDown = DefDown ;
    DirEast = DefEast ;
    DirWest = -DirEast ;
    while ( pPrev )
    {
      cmplx NextICoord = pPrev->m_Spot->m_imgCoord ;
      if ( pPrev->next )
      {
        DirEast = pPrev->m_Spot->m_DirE = pPrev->next->m_Spot->m_imgCoord - NextICoord ;
        dDist = abs( pPrev->m_Spot->m_DirE ) ;
        if ( pPrev->previous )
        {
          DirWest = pPrev->m_Spot->m_DirW = pPrev->previous->m_Spot->m_imgCoord - NextICoord ;
          dDist = 0.5 * (dDist + abs( pPrev->m_Spot->m_DirW )) ;
        }
        else
          DirWest = pPrev->m_Spot->m_DirW = -pPrev->m_Spot->m_DirE ;
      }
      else if ( pPrev->previous )
      {
        DirWest = pPrev->m_Spot->m_DirW = pPrev->previous->m_Spot->m_imgCoord - NextICoord ;
        DirEast = pPrev->m_Spot->m_DirE = -pPrev->m_Spot->m_DirW ;
        dDist = abs( pPrev->m_Spot->m_DirE ) ;
      }
      else // no spot on left side and no spot on right side 
      {
        bIsHorHeghbors = false ;
        ASSERT( 0 ) ;
        pPrev->m_Spot->m_DirE = DirEast ;
        pPrev->m_Spot->m_DirW = DirWest ;
      }

      cmplx UpTarget = NextICoord + DirUp ;
      Node1D * pUp = pAllNodes->find( UpTarget , dPosTol ) ;
      Spot * pUpSpot = (pUp) ? pUp->m_Spot : Find( Artificial , UpTarget , GetStep() * dPosTol ) ;
      cmplx DownTarget = NextICoord + DirDown ;
      Node1D * pDown = pAllNodes->find( UpTarget , dPosTol ) ;
      Spot * pDownSpot = (pDown) ? pDown->m_Spot : Find( Artificial , DownTarget , GetStep() * dPosTol ) ;
      if ( pUpSpot )
      {
        DirUp = pPrev->m_Spot->m_DirN = pUpSpot->m_imgCoord - NextICoord ;
        if ( pDownSpot )
          DirDown = pPrev->m_Spot->m_DirS = pDownSpot->m_imgCoord - NextICoord ;
        else
          pPrev->m_Spot->m_DirS = DirDown ;
      }
      else if ( pDownSpot )
      {
        DirDown = pPrev->m_Spot->m_DirS = pDownSpot->m_imgCoord - NextICoord ;
        DirUp = pPrev->m_Spot->m_DirN = -pPrev->m_Spot->m_DirS ;
      }
      else // no vertical neighbors
      {
        pPrev->m_Spot->m_DirN = DirUp ;
        pPrev->m_Spot->m_DirS = DirDown ;
      }
      pPrev = pPrev->previous ;
    }
    return true ;
  };

  FXString ToString()
  {
    FXString Result , Addition ;

    Node1D * pIter = GetHead() ;
    int iCnt = 0 ;
    if ( pIter )
    {
      Result.Format( "Axis pix(%.2f,%.2f) abs(%.2f,%.2f) Ind=(%d,%d) CentMainV(%.2f,%.2f)\n" ,
        pIter->m_Spot->m_imgCoord , pIter->m_Spot->m_WCoord , 
        pIter->m_Spot->m_Indexes , m_CentMainVector ) ;
      while ( pIter )
      {
        Addition.Format( "%3d Ind=%d pix(%7.2f,%7.2f) Ind=(%4d,%4d) abs(%7.2f,%7.2f) \n" ,
          iCnt++ , pIter->m_Spot->GetGlobalIndex() , pIter->m_Spot->m_imgCoord , 
          pIter->m_Spot->m_Indexes , pIter->m_Spot->m_WCoord ) ;
        Result += Addition ;
        pIter = pIter->next ;
      }
    }
    return Result ;
  }
};

typedef vector<Axis*> Lines ;

inline Node1D* findCenter( Axis * Axis1 , Axis * Axis2 ,
  double dTol = 0. )
{
  CLine2d Line1( Axis1->GetHead()->m_Spot->m_imgCoord ,
    Axis1->GetTail()->m_Spot->m_imgCoord ) ;
  CLine2d Line2( Axis2->GetHead()->m_Spot->m_imgCoord ,
    Axis2->GetTail()->m_Spot->m_imgCoord ) ;

  cmplx CenterPt ;
  bool bRes = Line1.intersect( Line2 , CenterPt ) ;
  if ( !bRes )
    return NULL ;

  Node1D * pAx1Node = Axis1->GetHead() ;
  if ( dTol == 0. )
    dTol = Axis1->GetStep() / 5. ;

  Node1D * pCenterNode = NULL ;
  while ( pAx1Node )
  {
    Node1D * pAx2Node = Axis2->GetHead() ;
    while ( pAx2Node )
    {
      if ( pAx2Node->m_Spot->m_imgCoord == pAx1Node->m_Spot->m_imgCoord )
      {
        pCenterNode = pAx2Node ;
        return pCenterNode ;
      }
      pAx2Node = pAx2Node->next ;
    }
    //       if ( pAx1Node->m_Spot->MatchInImg( CenterPt , dTol ) )
    //       {
    //         pCenterNode = pAx1Node ;
    //         return pCenterNode ;
    //       }
    pAx1Node = pAx1Node->next ;
  }
  return NULL ;
}

inline cmplx GetAndCheckDirection( Node1DList* pNodes , Node1D& first ,
  double dPosTol , double dMinAngle = -360. , double dMaxAngle = -360. )
{
  if ( !pNodes || !pNodes->GetHead() )
    return NULL;

  bool bDirection = dMinAngle > -360. ;
  //   if ( first.m_Spot->m_OnLines & iAxisMask )
  //     return NULL ; // spot is already marked as placed on this axis

  Node1D* second = ( !bDirection ) ?
    pNodes->getNearestNeighbor( first.m_Spot )
    : pNodes->getNearestNeighbor( first.m_Spot , dMinAngle , dMaxAngle ) ;
}


inline Axis* getAxis( Node1DList* pNodes , Node1D& first ,
  double dPosTol , int iAxisMask ,
  double dMinAngle = -360. , double dMaxAngle = -360. )
{
  if ( !pNodes || !pNodes->GetHead() )
    return NULL;

  bool bDirection = dMinAngle > -360. ;
  //   if ( first.m_Spot->m_OnLines & iAxisMask )
  //     return NULL ; // spot is already marked as placed on this axis

  Node1D* second = (!bDirection) ?
    pNodes->getNearestNeighbor( first.m_Spot )
    : pNodes->getNearestNeighbor( first.m_Spot , dMinAngle , dMaxAngle ) ;

  if ( !second )
    return NULL ;
  // 
  //   if ( second->m_Spot->m_OnLines & iAxisMask )
  //     return NULL ; // spot is already marked as placed on this axis


  Axis* axis = new Axis();
  first.m_Spot->m_OnLines |= iAxisMask ;
  Node1D * pFirstEdge = axis->AddToTail( first.m_Spot );
  second->m_Spot->m_OnLines |= iAxisMask ;
  Node1D* pSecondEdge = axis->AddToTail( second->m_Spot );

  Node1D* node1 = pFirstEdge ;
  Node1D* node2 = pSecondEdge ;
  Node1D* node3 = NULL ;

  while ( node3 = getNextLineSpot( pNodes , node1 , node2 , dPosTol , 0 , NULL , iAxisMask ) )
  {
    //     if ( node3->m_Spot->m_OnLines & iAxisMask )
    //     {
    //     }
    //     else
    node3->m_Spot->m_OnLines |= iAxisMask ;

    pSecondEdge = axis->AddToTail( node3->m_Spot ); // mark this node as last

    node1 = node2;
    node2 = node3;
  }
  // look to opposite direction from first point
  node1 = pFirstEdge->next ;
  node2 = pFirstEdge ;

  while ( node3 = getNextLineSpot( pNodes , node1 , node2 , dPosTol , 0 , NULL , iAxisMask ) )
  {
    //     if ( node3->m_Spot->m_OnLines & iAxisMask )
    //     {
    //     }
    //     else
    node3->m_Spot->m_OnLines |= iAxisMask ;
    pFirstEdge = axis->AddToHead( node3->m_Spot );

    node1 = node2;
    node2 = node3;
    node3 = getNextLineSpot( pNodes , node1 , node2 , dPosTol , 0 , NULL , iAxisMask );
  }
  cmplx TmpMainVect( axis->GetTail()->m_Spot->m_imgCoord
    - axis->GetHead()->m_Spot->m_imgCoord ) ;
  TmpMainVect /= (double)(axis->getCnt() - 1) ;

//   cmplx TmpMainVect( axis->GetHead()->next->m_Spot->m_imgCoord
//     - axis->GetHead()->m_Spot->m_imgCoord ) ;
  axis->SetMainVector( TmpMainVect ) ;
  return axis;
}

inline Axis* getAxis( Node1DList* pNodes , Node1D * pFirst ,
  cmplx Direction , double dPosTol , int iAxisMask )
{
  if ( !pNodes || !pNodes->GetHead() )
    return NULL;

  Node1D* second = pNodes->find( pFirst->m_Spot->m_imgCoord + Direction , abs( Direction) * 0.1 ) ;
  if ( !second )
    return NULL ;


  Axis* axis = new Axis();
  pFirst->m_Spot->m_OnLines |= iAxisMask ;
  Node1D * pFirstEdge = axis->AddToTail( pFirst->m_Spot );
  second->m_Spot->m_OnLines |= iAxisMask ;
  Node1D* pSecondEdge = axis->AddToTail( second->m_Spot );

  Node1D* node1 = pFirstEdge ;
  Node1D* node2 = pSecondEdge ;
  Node1D* node3 = NULL ;

  while ( node3 = getNextLineSpot( pNodes , node1 , node2 , dPosTol , 0 , NULL , iAxisMask ) )
  {
    //     if ( node3->m_Spot->m_OnLines & iAxisMask )
    //     {
    //     }
    //     else
    node3->m_Spot->m_OnLines |= iAxisMask ;

    pSecondEdge = axis->AddToTail( node3->m_Spot ); // mark this node as last

    node1 = node2;
    node2 = node3;
  }
  // look to opposite direction from first point
  node1 = pFirstEdge->next ;
  node2 = pFirstEdge ;

  while ( node3 = getNextLineSpot( pNodes , node1 , node2 , dPosTol , 0 , NULL , iAxisMask ) )
  {
    //     if ( node3->m_Spot->m_OnLines & iAxisMask )
    //     {
    //     }
    //     else
    node3->m_Spot->m_OnLines |= iAxisMask ;
    pFirstEdge = axis->AddToHead( node3->m_Spot );

    node1 = node2;
    node2 = node3;
    node3 = getNextLineSpot( pNodes , node1 , node2 , dPosTol , 0 , NULL , iAxisMask );
  }
  cmplx TmpMainVect( axis->GetTail()->m_Spot->m_imgCoord
    - axis->GetHead()->m_Spot->m_imgCoord ) ;
  TmpMainVect /= ( double ) ( axis->getCnt() - 1 ) ;

//   cmplx TmpMainVect( axis->GetHead()->next->m_Spot->m_imgCoord
//     - axis->GetHead()->m_Spot->m_imgCoord ) ;
  axis->SetMainVector( TmpMainVect ) ;
  return axis;
}

// On input is vector of 5 heighbor nodes with central node as first
// if angles of opposite nodes are with ~180 degree difference and
// distance from central node to other nodes are approximately the same,
// function returns true
inline bool CheckNeighbors( Nodes1D& ForOrdering , 
  cmplx& cFirstDirection , cmplx& cSecondDirection )
{
  CmplxVector Directions ;
  cmplx cCenter = ForOrdering.front()->m_Spot->m_imgCoord ;
  int iFirst = -1 ;
  double dMinAngle = DBL_MAX ;
  for ( auto It = ForOrdering.begin() + 1 ; It < ForOrdering.end() ; It++ )
  {
    cmplx cDirection = ( *It )->m_Spot->m_imgCoord - cCenter ;
    double dAngle = arg( cDirection ) ;
    Directions.push_back( cDirection ) ;
    // find min to zero value
    if ( abs(dAngle) < abs(dMinAngle) )
    {
      dMinAngle = dAngle ;
      cFirstDirection = cDirection ;
      iFirst =(int)(It - ForOrdering.begin()) - 1;
    }
  }
  // checking for opposite direction
  int iMayBeSecondDir = -1 ;
  double dMainLength = abs( cFirstDirection ) ;
  bool bMainIsOK = false , bOrthoIsOK = false ;
  for ( size_t i = 0 ; i < Directions.size() ; i++ )
  {
    if ( i == iFirst )
      continue ;
    double dCurrentLength = abs( Directions[ i ] ) ;
    double dAngle = arg( Directions[ i ] / cFirstDirection ) ;
    if ( fabs( dAngle ) > M_PI * 0.9 )
    {
      if ( ( abs( dCurrentLength - dMainLength ) / dMainLength ) < 0.2 )
      {
        bMainIsOK = true ; // this direction is about the same length and ~180 degrees from First
      }
    }
    else if ( iMayBeSecondDir >= 0 )
    {
      double dSecondAngle = arg( Directions[ i ] / Directions[ iMayBeSecondDir ] ) ;
      if ( fabs( dSecondAngle ) > M_PI * 0.9 ) // the rest  directions are with angle ~180 degrees
      { // check lengths
        double dDiff = fabs(dCurrentLength - abs( Directions[ iMayBeSecondDir ] ) ) ;
        if ( ( dDiff / dCurrentLength ) < 0.2 )
        {
          cSecondDirection =  
            Directions[ ( abs( dAngle - M_PI_2 ) < M_PI_4 ) ? i :  iMayBeSecondDir ] ;
          bOrthoIsOK = true ;
        }
      }
    }
    else
      iMayBeSecondDir = (int)i ;
  }
  return ( bMainIsOK && bOrthoIsOK ) ;
}

inline Spot* getAxises( Reservuar& AllSpots ,
  unsigned int bigDotsX , unsigned int bigDotsY ,
  Axis*& AxisX , Axis*& AxisY , double dPosTol ,
  Node1D** pCenterNode )
{

  Axis * axis1 = NULL ;
  Axis * axis2 = NULL ;
  Node1DList* bigs = 
    ( bigDotsX && bigDotsY ) ? AllSpots.getBigs() : NULL ;
  if ( bigDotsX && bigDotsY && (!bigs || !(bigs->GetHead())))
    return NULL ;
  if ( bigs && bigs->GetHead() )
  {  // there is marking by spot size
    Node1D* initNode = bigs->GetHead() ;
    // Get one axis
    axis1 = getAxis( bigs , *initNode , dPosTol , AXIS_0 );
    if ( !axis1 )
      return NULL ;
    Node1D * pNode = initNode ;
    while ( pNode )
    {
      // find not marked spot
      if ( !(pNode->m_Spot->m_OnLines & AXIS_0) )
      {
        double dAngle = axis1->GetArg() ;

        axis2 = getAxis( bigs , *pNode , dPosTol , AXIS_1 ,
          dAngle - M_PI_2 - M_PI_4 , dAngle - M_PI_2 + M_PI_4 ) ;
        break ;
      }
      pNode = pNode->next ;
    }
    if ( !axis2 )
    {
      delete axis1 ;
      return NULL ;
    }
    if ( (axis1->getCnt() < axis2->getCnt()) ^ !(bigDotsX < bigDotsY) )
    {
      AxisX = axis1;
      AxisY = axis2;
    }
    else
    {
      AxisX = axis2;
      AxisY = axis1;
    }
    delete bigs;
    // do center node a first node on each axis and calculate main vectors
    if (!pCenterNode)
      return NULL;
    *pCenterNode = findCenter( axis1 , axis2 ) ;
    if ( ! AxisX->CalcMainVector( *pCenterNode , *pCenterNode ) )
      return NULL ;
    if ( ! AxisY->CalcMainVector( *pCenterNode , *pCenterNode ) )
      return NULL ;
  }
  else
  { // there is no marking by size. Axes and directions could be selected randomally.


//     // we will do X direction in +-45 degrees in FOV with plus to the right
//     // Y will be in -45-135 degrees (with plus to down side)
//     cmplx Center( AllSpots.m_NodesArea.left + AllSpots.m_NodesArea.Width() / 2. ,
//       AllSpots.m_NodesArea.top + abs( AllSpots.m_NodesArea.Height() / 2. ) ) ;
//     Node1D * pCenter = AllSpots.GetNodeList()->find_nearest( Center ) ;
//     cmplx LeftUpOnImage( 0. , 0. ) ;
//     Node1D * pLeftUp = AllSpots.GetNodeList()->find_nearest( LeftUpOnImage ) ;

    // 1. Find node with 4 neighbors on approximately the same distance
    Nodes1D * pCentralNodes = AllSpots.GetNodeList()->GetNodeWith4Neightbors( AllSpots.GetCentralNode() ) ;
    if ( !pCentralNodes )
      return NULL ;

    cmplx cDirection1 , cDirection2 ;
    if ( CheckNeighbors( *pCentralNodes , cDirection1 , cDirection2  ) )
    {
      axis1 = getAxis( AllSpots.GetNodeList() , pCentralNodes->front() , cDirection1 , dPosTol , AXIS_0 ) ;
      if ( !axis1 )
        return NULL ;

      axis2 = getAxis( AllSpots.GetNodeList() , pCentralNodes->front() , cDirection2 , dPosTol , AXIS_1 ) ;
      if ( !axis2 )
        return NULL ;

      if ( axis1->GetMainVector().real() < 0. )
        axis1->SetMainVector( -axis1->GetMainVector() ) ;
      if ( axis2->GetMainVector().imag() < 0. )
        axis2->SetMainVector( -axis2->GetMainVector() ) ;
      AxisX = axis1 ;
      AxisY = axis2 ;
      *pCenterNode = findCenter( axis1 , axis2 ) ;
    }
  }
  if ( !pCenterNode || !(*pCenterNode) )
    return NULL ;

  return (*pCenterNode)->m_Spot ;
}

#endif // __INC__Axis_H__




