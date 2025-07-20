#ifndef __INC__Grid_H__
#define __INC__Grid_H__

#include "Axis.h"
#include "Parser.h"

#define THIS_MODULENAME "CoordMap"


struct Cell
{
  int up;
  int down;
  int left;
  int right;

  Cell( int Left , int Right , int Up , int Down )
  {
    left = Left;
    right = Right;
    up = Up;
    down = Down;
  }
};

class ThreePts
{
public:
  Spot * m_Pt1 ;
  Spot * m_Pt2 ;
  Spot * m_Pt3 ;

  ThreePts( Spot * Pt1 = NULL , Spot * Pt2 = NULL , Spot * Pt3 = NULL )
  {
    m_Pt1 = Pt1 ; m_Pt2 = Pt2 ; m_Pt3 = Pt3 ;
  }
};
class DistIndex
{
public:
  double m_dDist ;
  Spot * m_pSpot ;

  DistIndex( double dDist = DBL_MAX , Spot * pSpot = NULL )
  {
    m_dDist = dDist ;
    m_pSpot = pSpot ;
  }
};

class PtAndDir
{
public:
  const Spot * m_pNearest ;
  SpotSector m_Dir ;
  PtAndDir( const Spot * pNearest = NULL , SpotSector Dir = SS_UNKNOWN )
  {
    m_pNearest = pNearest ;
    m_Dir = Dir ;
  }
};

class ThreeDists
{
public:
  DistIndex D1 ;
  DistIndex D2 ;
  DistIndex D3 ;

  inline int Insert( DistIndex NewDI )
  {
    if ( ( NewDI.m_pSpot == D1.m_pSpot )
      || ( NewDI.m_pSpot == D2.m_pSpot )
      || ( NewDI.m_pSpot == D3.m_pSpot ) )
      return 0 ;
    if ( NewDI.m_dDist < D1.m_dDist )
    {
      D3 = D2 ;
      D2 = D1 ;
      D1 = NewDI ;
      return 1 ;
    }
    else if ( NewDI.m_dDist < D2.m_dDist )
    {
      D3 = D2 ;
      D2 = NewDI ;
      return 2 ;
    }
    else if ( NewDI.m_dDist < D3.m_dDist )
    {
      if ( !CheckXY( NewDI.m_pSpot->m_Indexes ) )
      {
        D3 = NewDI ;
      }
      return 3 ;
    }
    return 0 ;
  }
  inline int CheckXY( CPoint Indexes )
  {
    int iTheSameX = ( Indexes.x == D1.m_pSpot->m_Indexes.x ) && ( Indexes.x == D2.m_pSpot->m_Indexes.x ) ;
    int iTheSameY = ( Indexes.y == D1.m_pSpot->m_Indexes.y ) && ( Indexes.y == D2.m_pSpot->m_Indexes.y ) ;
    return ( iTheSameX + ( iTheSameY << 1 ) )  ;
  }
  inline bool IsValid()
  {
    return ( D1.m_pSpot && D2.m_pSpot && D3.m_pSpot ) ;
  }
};

typedef FXArray<ThreeDists , ThreeDists&> NearestCalibPts ;
typedef FXArray<ThreePts , ThreePts&> Nearest3Pts ;
typedef FXArray<PtAndDir , PtAndDir&> NearestPtsAndDirs ;


class Grid
{
private:

  Reservuar*  m_Spots;
  Node2DList* m_SpotsArr2D;
  Spot        m_CenterSpot ;
  bool        m_bUseCenter ;
  bool        m_bIsCalibrated ;
  Axis *      m_pAxisX ;
  Axis *      m_pAxisY ;
  Lines       m_HorLines ;
  Lines       m_VertLines ;
  Lines       m_FullMatrix ;
  CalibSpotVector   m_AddedSpots ;
  Spot **     m_pSpotsAs2DArray ;
  CSize       m_SpotArrSize ;
  CPoint      m_IndexOffset ;
  CPointArray m_CCWSearchAroundPt ;
  CSize       m_ImageSize ;
  NearestCalibPts m_NearestCalibPts ;
  Nearest3Pts m_Nearest3Pts ;
  CalibSpotVector   m_NearestSpots ;

  NearestPtsAndDirs m_SpotsAndDirs ;
  int         m_iBasicStep ; // decimation coeff for defined points
  CSize       m_PtsAndDirsSize ;
  int         m_iInsertMissed ;

  CSize       m_NBigSpots ;
  double      m_dPosTol ;
  double      m_dInterpointStep ;

  double     m_dLoadTime ;
  double     m_dBeginTime ;
  double     m_dFirstAxisTime ;
  double     m_dAllAxisTime ;
  double     m_dNeightboursTime ;
  double     m_dTableFormTime ;
  int        m_iNFilled ;
  int        m_iNDoubled ;
  int        m_iNearestNFilled ;
  int        m_iNearestDoubled ;



public:
  Grid()
  {
    m_Spots = NULL ;
    m_SpotsArr2D = NULL ;
    m_pAxisX = m_pAxisY = NULL ;
    m_pSpotsAs2DArray = NULL ;
    m_CCWSearchAroundPt.Add( CPoint( 1 , 0 ) );
    m_CCWSearchAroundPt.Add( CPoint( 0 , 1 ) );
    m_CCWSearchAroundPt.Add( CPoint( -1 , 0 ) );
    m_CCWSearchAroundPt.Add( CPoint( -1 , 0 ) );
    m_CCWSearchAroundPt.Add( CPoint( 0 , -1 ) );
    m_CCWSearchAroundPt.Add( CPoint( 0 , -1 ) );
    m_CCWSearchAroundPt.Add( CPoint( 1 , 0 ) );
    m_CCWSearchAroundPt.Add( CPoint( 1 , 0 ) );
    m_CCWSearchAroundPt.Add( CPoint( 0 , -1 ) );
    m_ImageSize.cx = m_ImageSize.cy = 0 ;
    m_iBasicStep = 4 ;
    m_iInsertMissed = 1 ;
    m_iNDoubled = m_iNFilled = 0 ;
  }
  ~Grid()
  {
    Clean() ;
  }

  void SetBasisStep( int iStep )
  {
    m_iBasicStep = iStep ;
  }
  int  GetBasisStep()
  {
    return m_iBasicStep ;
  }
  void SetInsertMissed( int iNMaxInsert )
  {
    m_iInsertMissed = iNMaxInsert ;
  }
  int  GetInsertMissed()
  {
    return m_iInsertMissed ;
  }
  Node1D * GetCentralNode()
  {
    return m_Spots->GetCentralNode() ;
  }
  bool IsPtInImage( cmplx& Pt )
  {
    return ( ( 0 <= Pt.real() ) && ( Pt.real() < m_ImageSize.cx )
      && ( 0 <= Pt.imag() ) && ( Pt.imag() < m_ImageSize.cy ) ) ;
  }
  void Clean()
  {
    if ( m_pAxisX )
    {
      size_t i = 0 ;
      for ( ; i < m_HorLines.size() ; i++ )
      {
        if ( m_pAxisX == m_HorLines[ i ] )
          break ;
      }
      if ( i >= m_HorLines.size() )
        delete m_pAxisX ;
    }
    if ( m_pAxisY )
    {
      size_t i = 0 ;
      for ( ; i < m_VertLines.size() ; i++ )
      {
        if ( m_pAxisY == m_VertLines[ i ] )
          break ;
      }
      if ( i >= m_VertLines.size() )
        delete m_pAxisY ;
    }
    for ( size_t i = 0 ; i < m_HorLines.size() ; i++ )
    {
      delete m_HorLines[ i ] ;
    }
    m_HorLines.clear() ;
    for ( size_t i = 0 ; i < m_VertLines.size() ; i++ )
    {
      delete m_VertLines[ i ] ;
    }
    m_VertLines.clear() ;
    m_pAxisX = m_pAxisY = NULL ;
    if ( m_pSpotsAs2DArray )
    {
      delete m_pSpotsAs2DArray ;
      m_pSpotsAs2DArray = NULL ;
    }
    if ( m_Spots )
    {
      delete m_Spots ;
      m_Spots = NULL ;
    }
    if ( m_SpotsArr2D )
    {
      delete m_SpotsArr2D ;
      m_SpotsArr2D = NULL ;
    }
    for ( size_t i = 0 ; i < m_AddedSpots.size() ; i++ )
    {
      delete m_AddedSpots[ i ] ;
    }
    m_AddedSpots.clear() ;
    m_pAxisX = m_pAxisY = NULL ;
    m_NearestSpots.clear() ;
    m_NearestCalibPts.RemoveAll() ;
    m_Nearest3Pts.RemoveAll() ;
    m_bIsCalibrated = false ;
  }

  bool InitReservuar()
  {
    bool bRes = false ;
    if ( m_Spots )
    {
      delete m_Spots ;
      m_Spots = NULL ;
      bRes = true ;
    }
    m_Spots = new Reservuar ;
    return bRes ;
  }
  inline bool setCenter( cmplx Img )
  {
    if ( !m_SpotsArr2D )
      return false;

    m_bUseCenter = false ;
    ImgToWorld( Img , m_CenterSpot.m_WCoord ) ;
    m_CenterSpot.m_imgCoord = Img ;
    m_bUseCenter = true ;

    return true;
  }

public:

  inline bool init( const FXString& gridString ,
    int bigDotsX , int bigDotsY ,
    double interpointStep , double dPosTol ,
    bool bForCalibration = true )
  {
    m_NBigSpots.cx = bigDotsX ;
    m_NBigSpots.cy = bigDotsY ;
    m_dPosTol = dPosTol ;
    m_dInterpointStep = interpointStep ;
    Clean() ;
    double dBeginTime = GetHRTickCount() ;
    m_Spots = Parser::load( gridString , m_ImageSize ) ;
    m_dLoadTime = GetHRTickCount() - dBeginTime ;
    if ( !m_Spots || !m_Spots->GetNodeList()->GetHead() )
      return false;
#ifdef _DEBUG
    m_Spots->CheckConsistence() ;
#endif
    return InitFromSpotData( bForCalibration ) ;
  }

  cmplx GetWorldForFirst( Axis * pAxis )
  {
    Spot * pSpot = pAxis->GetHead()->m_Spot ;
    int iNIterations = 0 ;
    while ( pSpot )
    {
      if ( pSpot->m_SpotN )
      {
        cmplx World = pSpot->m_SpotN->m_WCoord
          + cmplx( m_dInterpointStep * iNIterations , -m_dInterpointStep ) ;
        return World ;
      }
      else if ( pSpot->m_SpotS )
      {
        cmplx World = pSpot->m_SpotS->m_WCoord
          + cmplx( m_dInterpointStep * iNIterations , m_dInterpointStep ) ;
        return World ;
      }
      pSpot = pSpot->m_SpotE ;
      iNIterations++ ;
    }
    return cmplx( DBL_MAX , DBL_MAX ) ;
  }
  inline bool InitFromSpotData( bool bForCalibration = true )
  {
    Sleep( 200 ) ; // for drawing on screen
    m_CenterSpot.Reset() ;
    m_dBeginTime = GetHRTickCount() ;

    Axis* AxisX = NULL;
    Axis* AxisY = NULL;
    Node1D* pCenterNode = NULL ;

    m_AddedSpots.clear() ;
    Spot* centerSpot = NULL ;
    try
    {
      centerSpot = getAxises( *m_Spots ,
        m_NBigSpots.cx , m_NBigSpots.cy , AxisX , AxisY ,
        m_dPosTol , &pCenterNode );
    }
    catch (CMemoryException* e)
    {
      TCHAR Msg[ 2000 ] ;
      if ( e->GetErrorMessage( Msg , 1999 ) )
      {
        FxSendLogMsg( 7 , "Grid Building" , 0 ,
          "Get Main Axes exception: %s" , Msg ) ;
      }
      centerSpot = NULL ;
    }

    if ( !centerSpot )
      return false;
#ifdef _DEBUG
    m_Spots->CheckConsistence() ;
#endif
    m_pAxisX = AxisX ;
    m_pAxisY = AxisY ;

    cmplx WorldStep( m_dInterpointStep , 0. ) ;
    cmplx WorldStepUp( 0. , m_dInterpointStep ) ;
    m_pAxisX->EnumerateAxis( centerSpot->m_imgCoord , cmplx( 0. , 0. ) ,
      WorldStep , CPoint( 0 , 0 ) ) ;
    m_pAxisY->EnumerateAxis( centerSpot->m_imgCoord , cmplx( 0. , 0. ) ,
      WorldStepUp , CPoint( 0 , 0 ) ) ;

    cmplx DefUp = m_pAxisY->GetMainVector()  ;
    cmplx DefDown = -DefUp ;

    m_pAxisX->ExpandHorAxis( m_Spots , m_dPosTol , m_iInsertMissed ,
      &m_AddedSpots , pCenterNode->m_Spot , DefUp ) ;
    //AxisY->ExpandAxis( m_Spots , m_dPosTol , AXIS_1 , m_iInsertMissed , &m_AddedSpots ) ;
    //m_pAxisX->FindNeighborsOnHLine( m_Spots->GetNodeList() , m_dPosTol , m_AddedSpots , 
    //  m_pAxisX->GetMainVector() , DefUp , DefDown ) ;
    m_CenterSpot = *centerSpot ;
#ifdef _DEBUG
    m_Spots->CheckConsistence() ;
#endif

    m_HorLines.push_back( m_pAxisX ) ;
 
    m_dFirstAxisTime = GetHRTickCount() ;

    //     Node1D * pNextOnY = pCentralNodeY->previous ; // go to minus direction on Y
    // 
    //     // In this moment full X and Y axes are built and we can do iteration for all
    //     // known nodes on X and Y
    //     // When X and Y are inclined to FOV, could be additional
    //     // axes which is not included in iterations below 
    // 
    //     // cycle for horizontal lines "before" central point (negative indexes)
    //     // Horizontal means all lines ~parallel to axis X in world coordinates
    //     // cycle goes to opposite to Y direction  
    bool bLineFound ;

    Node1D * pNode = m_pAxisX->GetHead() ;
    // cycle for spot searching into south (-) direction
    int iNPassesBack = 0;
    do
    {
      ASSERT( ++iNPassesBack < 1000 );
      bLineFound = false ;
      while ( !pNode->m_Spot->m_SpotS )
      {
        pNode = pNode->next ;
        if ( !pNode )
          break ;
      }
      if ( pNode && pNode->m_Spot->m_SpotS )
      {
        Spot * pSpotOnNewLine = pNode->m_Spot->m_SpotS ;
        pSpotOnNewLine->m_DirN = -pNode->m_Spot->m_DirS ;
        double dAbsLineDist = abs( pSpotOnNewLine->m_DirN ) ;
        pSpotOnNewLine->m_SpotN = pNode->m_Spot ;
        pSpotOnNewLine->m_WCoord = pNode->m_Spot->m_WCoord - WorldStepUp ;
        pSpotOnNewLine->m_Indexes = pNode->m_Spot->m_Indexes ;
        pSpotOnNewLine->m_Indexes.y-- ;
        // Create new axis
        Axis * pNewAxis = new Axis( pSpotOnNewLine ) ;
//         if ( m_HorLines.size() > 30 )
//         {
//           ASSERT( 0 ) ;
//         }
        pNewAxis->SetWMainVector( WorldStep ) ;
        pNewAxis->SetMainVector( pNode->m_Spot->m_DirE ) ;
        Node1D * pNextNode = pNode->next ;
        if ( !pNextNode )
          continue ;
        // find and enumerate spots on new axis, which is placed to 
        // the South direction from current line
        while ( pNextNode->m_Spot->m_SpotS && pNextNode->next )
        {
          //pNextNode = pNextNode->next ;
          if ( pNextNode && pNextNode->m_Spot && pNextNode->m_Spot->m_SpotS )
          {
            bLineFound = true;
            // if the same lengths
            if ( ( dAbsLineDist - abs( pNextNode->m_Spot->m_DirS ) ) / dAbsLineDist < 0.2 )
            {
              pNewAxis->insert( pNextNode->m_Spot->m_SpotS ) ;
              pNewAxis->ExpandHorAxis( m_Spots , m_dPosTol , m_iInsertMissed ,
                &m_AddedSpots , pSpotOnNewLine , -pSpotOnNewLine->m_DirS ) ;
              cmplx cNewFirstWorld = GetWorldForFirst( pNewAxis ) ;
              pNewAxis->EnumerateAxis( pNewAxis->GetHead()->m_Spot->m_imgCoord ,
                cNewFirstWorld ,
                WorldStep , pNewAxis->GetHead()->m_Spot->m_Indexes/* + CPoint( 0 , -1 )*/ ) ;
              m_HorLines.insert( m_HorLines.begin() , pNewAxis ) ;
              pNode = pNewAxis->GetHead() ;
              break ; // from spot enumeration
            }
            else
              continue ; // what to do with omitted spots??
          }
          else // axis is finished, only one spot on this axis
          {
            pSpotOnNewLine->m_DirS = -pSpotOnNewLine->m_DirN ;
            pSpotOnNewLine->m_DirE = pNode->m_Spot->m_DirE ;
            pSpotOnNewLine->m_DirW = pNode->m_Spot->m_DirW ;
            pSpotOnNewLine->m_SpotE = pNode->m_Spot->m_SpotW =
              pSpotOnNewLine->m_SpotS = NULL ;
            m_HorLines.insert( m_HorLines.begin() , pNewAxis ) ;
            bLineFound = false ; // no more lines (???)
            break ;
          }
        }
      }
    } while ( bLineFound ) ;

    pNode = m_pAxisX->GetHead() ;
    // cycle for spot searching into North (+) direction
    int iNPassesForw = 0;
    do
    {
      ASSERT( ++iNPassesForw < 1000 );
      bLineFound = false ;
      while ( !pNode->m_Spot->m_SpotN )
      {
        pNode = pNode->next ;
        if ( !pNode )
          break ;
      }
      if ( pNode && pNode->m_Spot->m_SpotN )
      {
        Spot * pSpotOnNewLine = pNode->m_Spot->m_SpotN ;
        pNode->m_Spot->m_DirN = pSpotOnNewLine->m_imgCoord
          - pNode->m_Spot->m_imgCoord ;
        pNode->m_Spot->m_DirS = pNode->m_Spot->m_SpotS ?
          pNode->m_Spot->m_SpotS->m_imgCoord - pNode->m_Spot->m_imgCoord
          : -pNode->m_Spot->m_DirN ;
        pSpotOnNewLine->m_DirS = -pNode->m_Spot->m_DirN ;
        pSpotOnNewLine->m_DirN = pNode->m_Spot->m_DirN ;
        double dAbsLineDist = abs( pSpotOnNewLine->m_DirS ) ;
        pSpotOnNewLine->m_SpotS = pNode->m_Spot ;
        pSpotOnNewLine->m_WCoord = pNode->m_Spot->m_WCoord + WorldStepUp ;
        pSpotOnNewLine->m_Indexes = pNode->m_Spot->m_Indexes ;
        pSpotOnNewLine->m_Indexes.y++ ;
//         if ( m_HorLines.size() > 30 )
//         {
//           ASSERT( 0 ) ;
//         }

        // Create new axis
        Axis * pNewAxis = new Axis( pSpotOnNewLine ) ;
        pNewAxis->SetWMainVector( WorldStep ) ;
        pNewAxis->SetMainVector( pNode->m_Spot->m_DirE ) ;
        Node1D * pNextNode = pNode->next ;
        // find and enumerate spots on new axis, which is placed to 
        // the South direction from current line
        int iNPassesOnHLine = 0;

        while ( pNextNode && pNextNode->m_Spot->m_SpotN )
        {
          ASSERT( ++iNPassesOnHLine < 200 );
          //           if ( pNextNode && pNextNode->m_Spot )
          //           {
          double dLineDistOnNext = abs( pNextNode->m_Spot->m_DirN ) ;
          double dDistDiff = abs( dAbsLineDist - dLineDistOnNext ) ;
          // if the same lengths
          if ( ( dDistDiff / dAbsLineDist ) < 0.2 )
          {
            bLineFound = true ;

            pNewAxis->insert( pNextNode->m_Spot->m_SpotN ) ;
            pNewAxis->ExpandHorAxis( m_Spots , m_dPosTol , m_iInsertMissed ,
              &m_AddedSpots , pSpotOnNewLine , pSpotOnNewLine->m_DirN ) ;
//             cmplx cNewFirstWorld =
//               pNewAxis->GetHead()->m_Spot->m_SpotS->m_WCoord + WorldStepUp ;
            //             pNewAxis->EnumerateAxis( pNewAxis->GetHead()->m_Spot->m_imgCoord ,
            //               cNewFirstWorld ,
            //               WorldStep , pNewAxis->GetHead()->m_Spot->m_Indexes + CPoint( 0 , 1 ) ) ;
            m_HorLines.push_back( pNewAxis ) ;
            pNode = pNewAxis->GetHead() ;
            break ; // from spot enumeration
          }
          else
            continue ; // what to do with omitted spots??
//           }
//           else // axis is finished, only one spot on this axis
//           {
//             pSpotOnNewLine->m_DirN = -pSpotOnNewLine->m_DirS ;
//             pSpotOnNewLine->m_DirE = pNode->m_Spot->m_DirE ;
//             pSpotOnNewLine->m_DirW = pNode->m_Spot->m_DirW ;
//             pSpotOnNewLine->m_SpotE = pNode->m_Spot->m_SpotW =
//               pSpotOnNewLine->m_SpotN = NULL ;
//             m_HorLines.push_back( pNewAxis ) ;
//             bLineFound = false ; // no more lines (???)
//             break ;
//           }
        }
      }
    } while ( bLineFound ) ;


    if ( m_HorLines.size() < 2 )
      return false ; // no calibration matrix
    m_Spots->CheckConsistence() ;

    //ASSERT( m_HorLines.size() < 12 ) ;

    // Now we will fill neighbors info for all found or created (as artificial) and used in lines spots
//     for ( int i = 0 ; i < m_HorLines.size() ; i++ )
//     {
//       Axis * pAxis = m_HorLines[i] ;
//       cmplx DefUp = pAxis->GetMainVector() * cmplx( 0. , -1. )  ;
//       cmplx DefDown = -DefUp ;
//       pAxis->FindNeighborsOnHLine( m_Spots->GetNodeList() , m_dPosTol , m_AddedSpots , 
//         pAxis->GetMainVector() , DefUp , DefDown ) ;
// 
//     }

    //ASSERT( m_HorLines.size() < 12 ) ;
    m_dAllAxisTime = GetHRTickCount() ;

    //     // now we will try to add horizontal lines, which are not crossing 
    //     // vertical line with index 0. It could be when "big" spots are shifted in FOV and 
    //     // matrix is inclined for big angle 
    // 
    //     // Looking for horizontal lines before first already found
    //     int iNBefore = FindAdditionalLines( false , m_dPosTol , m_dInterpointStep ) ;
    //     // Looking for horizontal lines after last already found
    //     int iNAfter = FindAdditionalLines( true , m_dPosTol , m_dInterpointStep ) ;
    //     //ASSERT( m_HorLines.size() < 12 ) ;
    // #ifdef _DEBUG
    //     m_Spots->CheckConsistence() ;
    // #endif
    m_dNeightboursTime = GetHRTickCount() ;
    int iXIndexMin = 10000 ;
    int iXIndexMax = -10000 ;
    int iYIndexMin = m_HorLines[ 0 ]->m_pHead->m_Spot->m_Indexes.y ;
    int iYIndexMax = m_HorLines.back()->m_pHead->m_Spot->m_Indexes.y ;
    for ( size_t iLine = 0 ; iLine < m_HorLines.size() ; iLine++ )
    {
      CPoint Index = m_HorLines[ iLine ]->m_pHead->m_Spot->m_Indexes ;
      if ( iXIndexMin > Index.x )
        iXIndexMin = Index.x ;
      Index = m_HorLines[ iLine ]->m_pTail->m_Spot->m_Indexes ;
      if ( iXIndexMax < Index.x )
        iXIndexMax = Index.x ;
    }
    ASSERT( iXIndexMin*iXIndexMax <= 0 && iYIndexMin*iYIndexMax <= 0 ) ;
    m_SpotArrSize = CSize( iXIndexMax - iXIndexMin + 1 , iYIndexMax - iYIndexMin + 1 ) ;
    if ( m_pSpotsAs2DArray )
      delete m_pSpotsAs2DArray ;
    int iArrLen = m_SpotArrSize.cx * m_SpotArrSize.cy ;
    ASSERT( iArrLen > 0 ) ;
    m_pSpotsAs2DArray = new Spot*[ iArrLen ] ;
    int iArrSize = sizeof( Spot* ) * iArrLen ;
    memset( m_pSpotsAs2DArray , 0 , iArrSize ) ;
    m_IndexOffset = CPoint( iXIndexMin , iYIndexMin ) ;
#ifdef _DEBUG
    m_Spots->CheckConsistence() ;
#endif

    pNode = m_Spots->GetNodeList()->m_pHead ;
    int iSettledCnt = 0 ;
    while ( pNode )
    {
      pNode->m_Spot->SetWorldSteps( WorldStep , WorldStepUp ) ;
      if ( ( pNode->m_Spot->m_DirE != 0. )
        && ( pNode->m_Spot->m_DirW != 0. )
        && ( pNode->m_Spot->m_DirN != 0. )
        && ( pNode->m_Spot->m_DirS != 0. ) )
      {
        if ( !SetSpotPtr( *( pNode->m_Spot ) ) )
        {
          CPoint pt = pNode->m_Spot->m_Indexes ;
        }
        iSettledCnt++ ;
      }
      pNode = pNode->next ;
    }
    //     int iNFilledByTriangles = BuildIndexTableByTriangles() ;
    //     int iNFilled = 0 ;
    //     for ( int i = 0 ; i < m_Nearest3Pts.GetCount() ; i++ )
    //     {
    //       ThreePts& Pt = m_Nearest3Pts.GetAt( i ) ;
    //       iNFilled += (Pt.m_Pt1 != NULL) || (Pt.m_Pt2 != NULL) || (Pt.m_Pt3 != NULL) ;
    //     }

//     m_iNearestNFilled = m_iNearestDoubled = 0 ;
//     int iNNearestPts = (m_ImageSize.cx / m_iBasicStep) * (m_ImageSize.cy / m_iBasicStep) ;
//     m_NearestSpots.SetSize( iNNearestPts ) ;
//     memset( m_NearestSpots.GetData() , 0 , sizeof( Spot* ) * iNNearestPts ) ;
    int iNCalibPts = m_SpotArrSize.cx * m_SpotArrSize.cy ;
//     for ( int i = 0 ; i < iNCalibPts ; i++ )
//     {
//       Spot * pS = *(m_pSpotsAs2DArray + i) ;
//       cmplx UL = pS->m_imgCoord + (pS->m_DirN + pS->m_DirW) * 0.55 ;
//       cmplx UR = pS->m_imgCoord + (pS->m_DirN + pS->m_DirE) * 0.55 ;
//       cmplx DR = pS->m_imgCoord + (pS->m_DirS + pS->m_DirE) * 0.55 ;
//       cmplx DL = pS->m_imgCoord + (pS->m_DirS + pS->m_DirW) * 0.55 ;
//       m_iNearestDoubled += FillTetraGone( UL , UR , DR , DL , pS ) ;
//     }
    m_bIsCalibrated = true ;
    if ( bForCalibration )
    {
      if ( BuildDistIndexTableForOnePoint() )
      {
        m_dTableFormTime = GetHRTickCount() ;
        return iNCalibPts ;
      }
      else
        return 0 ;
    }
    else
      return iNCalibPts ;
  }
  // following function returns index in spot array
  // Indexes is positions relatively to calibration matrix zero point (0,0)
  inline int GetLinearIndex( CPoint Indexes )
  {
    Indexes -= m_IndexOffset ;
    if ( Indexes.x >= 0 && Indexes.x < m_SpotArrSize.cx
      && Indexes.y >= 0 && Indexes.y < m_SpotArrSize.cy )
      return Indexes.x + ( Indexes.y * m_SpotArrSize.cx ) ;
    else
      return -1 ; // out of matrix
  }

  inline int GetIndexInMatrix( CPoint AbsIndexes )
  {
    if ( AbsIndexes.x >= 0 && AbsIndexes.x < m_SpotArrSize.cx
      && AbsIndexes.y >= 0 && AbsIndexes.y < m_SpotArrSize.cy )
      return AbsIndexes.x + ( AbsIndexes.y * m_SpotArrSize.cx ) ;
    else
      return -1 ;
  }

  inline Spot* GetSpotOnLeft( Spot& Current )
  {
    if ( Current.m_Indexes.x - m_IndexOffset.x > 0 )
    {
      int iLinearIndex = GetLinearIndex( Current.m_Indexes ) ;
      if ( iLinearIndex >= 1 )
        return  ( ( m_pSpotsAs2DArray[ 0 ] ) + iLinearIndex - 1 ) ;
    }
    return NULL ;
  }
  inline Spot* GetSpotOnRight( Spot& Current )
  {
    if ( Current.m_Indexes.x - m_IndexOffset.x < m_SpotArrSize.cx - 1 )
    {
      int iLinearIndex = GetLinearIndex( Current.m_Indexes ) ;
      if ( iLinearIndex >= 0 )
        return  ( ( m_pSpotsAs2DArray[ 0 ] ) + iLinearIndex + 1 ) ;
    }
    return NULL ;
  }
  inline Spot* GetSpotOnUp( Spot& Current )
  {
    if ( Current.m_Indexes.y - m_IndexOffset.y > 0 )
    {
      int iLinearIndex = GetLinearIndex( Current.m_Indexes ) ;
      if ( iLinearIndex >= 1 )
        return  ( ( m_pSpotsAs2DArray[ 0 ] ) + iLinearIndex - -m_SpotArrSize.cy ) ;
    }
    return NULL ;
  }
  inline Spot* GetSpotOnDown( Spot& Current )
  {
    if ( Current.m_Indexes.y - m_IndexOffset.y < m_SpotArrSize.cy - 1 )
    {
      int iLinearIndex = GetLinearIndex( Current.m_Indexes ) ;
      if ( iLinearIndex >= 0 )
        return  ( ( m_pSpotsAs2DArray[ 0 ] ) + iLinearIndex + m_SpotArrSize.cy ) ;
    }
    return NULL ;
  }

  inline int GetIndexInPtAndDirArray( int iX , int iY )
  {
    int iXIndex = iX / m_iBasicStep ;
    int iYIndex = iY / m_iBasicStep ;
    int iIndex = iXIndex + iYIndex * m_PtsAndDirsSize.cx ;
    return iIndex ;
  }

  inline int GetIndexInPtAndDirArray( cmplx& Pt )
  {
    int iXIndex = ROUND( Pt.real() / m_iBasicStep ) ;
    int iYIndex = ROUND( Pt.imag() / m_iBasicStep ) ;
    int iIndex = iXIndex + iYIndex * m_PtsAndDirsSize.cx ;
    return iIndex ;
  }

  inline bool BuildDistIndexTableForOnePoint()
  {
    if ( m_ImageSize.cx && m_ImageSize.cy && m_pSpotsAs2DArray )
    {
      m_SpotsAndDirs.RemoveAll() ;
      m_PtsAndDirsSize.cx = ( int ) ceil( ( DOUBLE ) m_ImageSize.cx / ( double ) m_iBasicStep ) ;
      m_PtsAndDirsSize.cy = ( int ) ceil( ( double ) m_ImageSize.cy / ( DOUBLE ) m_iBasicStep ) ;
      int iArrLen = m_SpotArrSize.cx * m_SpotArrSize.cy ;

      int iArrLenNearest = m_PtsAndDirsSize.cx * m_PtsAndDirsSize.cy ;
      m_SpotsAndDirs.SetSize( iArrLenNearest ) ;
      const Spot * pFirst = NULL , *pFollow = NULL ;
      double dHalfBasicStep = 0.5 * m_iBasicStep ;
      for ( int iY = 0 ; iY < m_ImageSize.cy ; iY += m_iBasicStep )
      {
        int iYBase = iY * m_PtsAndDirsSize.cx ;
        if ( pFirst == NULL ) // nothing known, find nearest point
        {
          ThreeDists NewPoint ;
          cmplx ImgCoord( dHalfBasicStep , iY + dHalfBasicStep ) ;
          double dMinDist = DBL_MAX ;
          Spot * pMinDistSpot = NULL ;
          for ( int i = 0 ; i < iArrLen ; i++ )
          {
            Spot * pSpot = m_pSpotsAs2DArray[ i ] ;
            if ( pSpot )
            {
              double dDist = pSpot->getDistance( ImgCoord ) ;

              if ( dDist < dMinDist )
              {
                dMinDist = dDist ;
                pMinDistSpot = pSpot ;
              }
            }
          }
          pFirst = pMinDistSpot ;
        }
        for ( int iX = 0 ; iX < m_ImageSize.cx ; iX += m_iBasicStep )
        {
          if ( pFollow == NULL )
            pFollow = pFirst ;
          cmplx ImgCoord( iX + dHalfBasicStep , iY + dHalfBasicStep ) ;
          const Spot * pNearest = getNearest( pFollow , ImgCoord ) ;
          SpotSector Sector = GetNearestDirection( pNearest , ImgCoord ) ;
          PtAndDir NewPt( pNearest , Sector ) ;

          int iIndex = GetIndexInPtAndDirArray( iX , iY ) ;
          m_SpotsAndDirs.SetAt( iIndex , NewPt ) ;
          pFollow = pNearest ;
        }
        pFollow = NULL ;
      }
      return true ;
    }
    return false ;
  }

  inline bool BuildDistIndexTableWith3Pts()
  {
    if ( m_ImageSize.cx && m_ImageSize.cy && m_pSpotsAs2DArray )
    {
      m_NearestCalibPts.RemoveAll() ;
      m_NearestSpots.clear() ;
      int iArrLen = m_SpotArrSize.cx * m_SpotArrSize.cy ;
      int iRowSTep = m_ImageSize.cx / m_iBasicStep ;
      for ( int iY = 0 ; iY < m_ImageSize.cy ; iY += m_iBasicStep )
      {
        int iYBase = iY * iRowSTep ;
        for ( int iX = 0 ; iX < m_ImageSize.cx ; iX += m_iBasicStep )
        {
          ThreeDists NewPoint ;
          cmplx ImgCoord( ( double ) iX , ( double ) iY ) ;
          double dMinDist = DBL_MAX ;
          Spot * pMinDistSpot = NULL ;
          for ( int i = 0 ; i < iArrLen ; i++ )
          {
            Spot * pSpot = m_pSpotsAs2DArray[ i ] ;
            if ( pSpot )
            {
              double dDist = pSpot->getDistance( ImgCoord ) ;

              if ( dDist < dMinDist )
              {
                dMinDist = dDist ;
                pMinDistSpot = pSpot ;
              }
            }
          }
          DistIndex NewDist( dMinDist , pMinDistSpot ) ;
          NewPoint.Insert( NewDist ) ;
          double dDistToRight = DBL_MAX , dDistToLeft = DBL_MAX ;
          double dDistToUp = DBL_MAX , dDistToDown = DBL_MAX ;
          Spot * pSpotOnRight = NULL , *pSpotOnLeft = NULL ,
            *pSpotToUp = NULL , *pSpotToDown = NULL ;
          CPoint Indexes = pMinDistSpot->m_Indexes ;
          int iLinearIndex = GetLinearIndex( Indexes ) ;
          ASSERT( iLinearIndex >= 0 ) ;

          if ( Indexes.x < m_SpotArrSize.cx - 1 )
          {
            pSpotOnRight = m_pSpotsAs2DArray[ iLinearIndex + 1 ] ;
            dDistToRight = abs( ImgCoord - pSpotOnRight->m_imgCoord ) ;
          }
          if ( Indexes.x > 0 )
          {
            pSpotOnLeft = m_pSpotsAs2DArray[ iLinearIndex - 1 ] ;
            dDistToLeft = abs( ImgCoord - pSpotOnLeft->m_imgCoord ) ;
          }
          if ( dDistToLeft < dDistToRight )
          {
            DistIndex LeftDist( dDistToLeft , pSpotOnLeft ) ;
            NewPoint.Insert( LeftDist ) ;
          }
          else
          {
            DistIndex RightDist( dDistToRight , pSpotOnRight ) ;
            NewPoint.Insert( RightDist ) ;
          }

          if ( Indexes.y < m_SpotArrSize.cy - 1 )
          {
            pSpotToDown = m_pSpotsAs2DArray[ iLinearIndex + m_SpotArrSize.cx ] ;
            dDistToDown = abs( ImgCoord - pSpotToDown->m_imgCoord ) ;
          }
          if ( Indexes.y > 0 )
          {
            pSpotToUp = m_pSpotsAs2DArray[ iLinearIndex - m_SpotArrSize.cx ] ;
            dDistToUp = abs( ImgCoord - pSpotToUp->m_imgCoord ) ;
          }
          if ( dDistToUp < dDistToDown )
          {
            DistIndex UpDist( dDistToUp , pSpotToUp ) ;
            NewPoint.Insert( UpDist ) ;
          }
          else
          {
            DistIndex DownDist( dDistToDown , pSpotToDown ) ;
            NewPoint.Insert( DownDist ) ;
          }
          m_NearestSpots.push_back( pMinDistSpot ) ;
          if ( NewPoint.IsValid() )
          {
            if ( NewPoint.CheckXY( NewPoint.D3.m_pSpot->m_Indexes ) )
            {
              NewPoint.D3.m_dDist = DBL_MAX ;
              NewPoint.D3.m_pSpot = NULL ;
              for ( int i = 0 ; i < iArrLen ; i++ )
              {
                Spot * pSpot = m_pSpotsAs2DArray[ i ] ;
                if ( pSpot )
                {
                  double dDist = pSpot->getDistance( ImgCoord ) ;
                  DistIndex NewDist( dDist , pSpot ) ;
                  NewPoint.Insert( NewDist ) ;
                }
              }
              if ( NewPoint.IsValid() )
              {
                if ( NewPoint.CheckXY( NewPoint.D3.m_pSpot->m_Indexes ) )
                  ASSERT( 0 ) ;
              }
            }
            m_NearestCalibPts.Add( NewPoint ) ;  // search key is ((y/m_iBasicStep)* sizex/m_iBasicStep) + x/m_iBasicStep
          }
        }
      }
      return true ;
    }
    return false ;
  }

  inline int BuildIndexTableByTriangles()
  {
    if ( !m_pSpotsAs2DArray )
      return 0 ;

    int iNCalibPts = ( m_ImageSize.cx / m_iBasicStep ) * ( m_ImageSize.cy / m_iBasicStep ) ;
    m_Nearest3Pts.SetSize( iNCalibPts ) ;
    memset( m_Nearest3Pts.GetData() , 0 , sizeof( ThreePts ) * iNCalibPts ) ;
    m_iNDoubled = m_iNFilled = 0 ;


    CPoint Indexes( 0 , 0 ) ;
    int iNFilledPts = 0 ;
    while ( Indexes.y < m_SpotArrSize.cy - 1 ) // last line will be processed
    {                                          // together with previous
      Indexes.x = 0 ;
      int iLinInd = GetIndexInMatrix( Indexes ) ;
      if ( iLinInd < 0 )
      {
        ASSERT( 0 ) ;
        break ;
      }
      Spot * CurrentSpot = *( m_pSpotsAs2DArray + iLinInd ) ;
      Spot * RightSpot = *( m_pSpotsAs2DArray + iLinInd + 1 ) ;
      Spot * LeftDownSpot = *( m_pSpotsAs2DArray + iLinInd + m_SpotArrSize.cx ) ;
      Spot * RightDownSpot = *( m_pSpotsAs2DArray + iLinInd + m_SpotArrSize.cx + 1 ) ;
      while ( Indexes.x < m_SpotArrSize.cx - 1 )
      {
        if ( Indexes.y == 0 ) // upper line
        {
          if ( Indexes.x == 0 ) // left upper corner
          {
            cmplx UL = ( 2. * CurrentSpot->m_imgCoord ) - RightDownSpot->m_imgCoord ;
            cmplx UR = ( 2. * CurrentSpot->m_imgCoord ) - LeftDownSpot->m_imgCoord ;
            cmplx DL = ( 2. * CurrentSpot->m_imgCoord ) - RightSpot->m_imgCoord ;
            ThreePts ForLeftCorner( CurrentSpot , RightSpot , LeftDownSpot ) ;
            iNFilledPts += FillTetraGone( UL , UR ,
              CurrentSpot->m_imgCoord , DL , ForLeftCorner ) ;
          }
          else if ( Indexes.x == m_SpotArrSize.cx - 1 ) // right upper corner
          {
            cmplx UL = ( 2. * RightSpot->m_imgCoord ) - RightDownSpot->m_imgCoord ;
            cmplx UR = ( 2. * RightSpot->m_imgCoord ) - LeftDownSpot->m_imgCoord ;
            cmplx DR = ( 2. * RightSpot->m_imgCoord ) - CurrentSpot->m_imgCoord ;
            ThreePts ForRightCorner( CurrentSpot , RightSpot , RightDownSpot ) ;
            iNFilledPts += FillTetraGone( UL , UR ,
              DR , RightSpot->m_imgCoord , ForRightCorner ) ;
          }
          // fill area above main calibration rectangle
          cmplx UL = ( 2. * CurrentSpot->m_imgCoord ) - LeftDownSpot->m_imgCoord ;
          cmplx UR = ( 2. * RightSpot->m_imgCoord ) - RightDownSpot->m_imgCoord ;
          ThreePts ForUpperArea( CurrentSpot , RightSpot , LeftDownSpot ) ;
          iNFilledPts += FillTetraGone( UL , UR ,
            RightSpot->m_imgCoord , CurrentSpot->m_imgCoord , ForUpperArea ) ;
        }
        if ( Indexes.x == 0 )
        {
          // fill left area out of main calibration rectangle
          cmplx UL = ( 2. * CurrentSpot->m_imgCoord ) - RightSpot->m_imgCoord ;
          cmplx DL = ( 2. * LeftDownSpot->m_imgCoord ) - RightDownSpot->m_imgCoord ;
          ThreePts ForLeftArea( CurrentSpot , RightSpot , LeftDownSpot ) ;
          iNFilledPts += FillTetraGone( UL , CurrentSpot->m_imgCoord ,
            RightSpot->m_imgCoord , DL , ForLeftArea ) ;
        }
        else if ( Indexes.x == m_SpotArrSize.cx - 1 )
        { // fill right area out of main calibration rectangle
          cmplx UR = ( 2. * RightSpot->m_imgCoord ) - CurrentSpot->m_imgCoord ;
          cmplx DR = ( 2. * RightDownSpot->m_imgCoord ) - LeftDownSpot->m_imgCoord ;
          ThreePts ForRightArea( RightDownSpot , RightSpot , CurrentSpot ) ;
          iNFilledPts += FillTetraGone( RightSpot->m_imgCoord , UR ,
            DR , RightDownSpot->m_imgCoord , ForRightArea ) ;
        }
        if ( Indexes.y == m_SpotArrSize.cy - 1 ) // lower line
        {
          if ( Indexes.x == 0 ) // left lower corner
          {
            cmplx DR = ( 2. * LeftDownSpot->m_imgCoord ) - RightSpot->m_imgCoord ;
            cmplx UL = ( 2. * LeftDownSpot->m_imgCoord ) - RightDownSpot->m_imgCoord ;
            cmplx DL = ( 2. * LeftDownSpot->m_imgCoord ) - CurrentSpot->m_imgCoord ;
            ThreePts ForLeftCorner( CurrentSpot , RightDownSpot , LeftDownSpot ) ;
            iNFilledPts += FillTetraGone( UL , LeftDownSpot->m_imgCoord ,
              DL , DR , ForLeftCorner ) ;
          }
          else if ( Indexes.x == m_SpotArrSize.cx - 1 ) // right lower corner
          {
            cmplx UL = ( 2. * RightSpot->m_imgCoord ) - RightDownSpot->m_imgCoord ;
            cmplx UR = ( 2. * RightSpot->m_imgCoord ) - LeftDownSpot->m_imgCoord ;
            cmplx DR = ( 2. * RightSpot->m_imgCoord ) - CurrentSpot->m_imgCoord ;
            ThreePts ForRightCorner( CurrentSpot , RightSpot , RightDownSpot ) ;
            iNFilledPts += FillTetraGone( UL , UR ,
              DR , RightSpot->m_imgCoord , ForRightCorner ) ;
          }
          // fill area below main calibration rectangle
          cmplx DL = ( 2. * LeftDownSpot->m_imgCoord ) - CurrentSpot->m_imgCoord ;
          cmplx DR = ( 2. * RightDownSpot->m_imgCoord ) - RightSpot->m_imgCoord ;
          ThreePts ForDownArea( CurrentSpot , RightDownSpot , LeftDownSpot ) ;
          iNFilledPts += FillTetraGone( LeftDownSpot->m_imgCoord , RightDownSpot->m_imgCoord ,
            DR , DL , ForDownArea ) ;
        }

        iNFilledPts += FillTetraGone( CurrentSpot , RightSpot , RightDownSpot , LeftDownSpot ) ;
        //         CurrentSpot = RightSpot ;
        //         LeftDownSpot = RightDownSpot ;
        //         RightSpot = (CurrentSpot + 1 ) ;
        //         RightDownSpot = (LeftDownSpot + 1) ;
        Indexes.x++ ;
        int iLinInd = GetIndexInMatrix( Indexes ) ;
        if ( iLinInd < 0 )
        {
          ASSERT( 0 ) ;
          break ;
        }
        Spot * CurrentSpot = *( m_pSpotsAs2DArray + iLinInd ) ;
        Spot * RightSpot = *( m_pSpotsAs2DArray + iLinInd + 1 ) ;
        Spot * LeftDownSpot = *( m_pSpotsAs2DArray + iLinInd + m_SpotArrSize.cx ) ;
        Spot * RightDownSpot = *( m_pSpotsAs2DArray + iLinInd + m_SpotArrSize.cx + 1 ) ;
      }
      Indexes.y++ ;
    }
    m_iNFilled = iNFilledPts ;
    return iNFilledPts ;
  }

  inline int GetNeighborIndex( cmplx& Pt )
  {
    int iIndex = ( ( ( int ) ( Pt.imag() ) / m_iBasicStep ) * ( m_ImageSize.cx / m_iBasicStep ) )
      + ( ( ( int ) Pt.real() ) / m_iBasicStep ) ;
    //     double dIndex = floor( Pt.real() / m_iBasicStep )
    //       + floor(Pt.imag() /  m_iBasicStep ) * (/(double)m_iBasicStep) ;
    return iIndex ;
  }
  // N steps in following function is defined by m_iBasicStep
  inline int FillOneLine( cmplx& cBegin , cmplx& cEnd , ThreePts& Neighbors )
  {
    cmplx Step = polar( ( m_iBasicStep - 0.1 ) , arg( cEnd - cBegin ) )  ;
    cmplx Iter = cBegin ;
    int iIndex = GetNeighborIndex( Iter ) ;
    int iNWritten = 0 ;
    int iNDoubled = 0 ;
    double dDist = abs( Iter - cEnd ) , dNewDist = dDist ;
    do
    {
      dDist = dNewDist ;
      iIndex = GetNeighborIndex( Iter ) ;
      ThreePts& Place = m_Nearest3Pts[ iIndex ] ;

      if ( ( Place.m_Pt1 && ( Place.m_Pt1->m_imgCoord != Neighbors.m_Pt1->m_imgCoord ) )
        || ( Place.m_Pt2 && ( Place.m_Pt2->m_imgCoord != Neighbors.m_Pt2->m_imgCoord ) )
        || ( Place.m_Pt3 && ( Place.m_Pt3->m_imgCoord != Neighbors.m_Pt3->m_imgCoord ) ) )
      {
        m_iNDoubled++ ;
        iNDoubled++ ;
      }
      m_Nearest3Pts[ iIndex ] = Neighbors ;
      iNWritten++ ;
      Iter += Step ;
      dNewDist = abs( Iter - cEnd ) ;
    } while ( dNewDist < dDist ) ;
    return iNWritten ;
  }

  inline int FillOneLine( cmplx& cBegin , cmplx& cEnd , Spot * pValForFill )
  {
    cmplx Step = polar( ( m_iBasicStep - 0.1 ) , arg( cEnd - cBegin ) )  ;
    cmplx Iter = cBegin ;
    int iIndex = GetNeighborIndex( Iter ) ;
    int iNWritten = 0 ;
    int iNDoubled = 0 ;
    double dDist = abs( Iter - cEnd ) , dNewDist = dDist ;
    do
    {
      dDist = dNewDist ;
      iIndex = GetNeighborIndex( Iter ) ;
      Spot * Place = m_NearestSpots[ iIndex ] ;

      if ( Place && Place != pValForFill )
      {
        m_iNearestDoubled++ ;
        iNDoubled++ ;
      }
      m_NearestSpots[ iIndex ] = pValForFill ;
      iNWritten++ ;
      Iter += Step ;
      dNewDist = abs( Iter - cEnd ) ;
    } while ( dNewDist < dDist ) ;
    return iNWritten ;
  }


  // following function fill elements of neighbor array
  // with pointers to nearest spots
  // Corners are in CW direction
  inline int FillTetraGone( Spot * pS1 , Spot * pS2 , Spot * pS3 , Spot * pS4 )
  {
    cmplx VectDownLeft = pS4->m_imgCoord - pS1->m_imgCoord ;
    cmplx VectDownRight = pS3->m_imgCoord - pS2->m_imgCoord ;
    int iNLeftSteps = ( int ) ( abs( VectDownLeft ) / ( m_iBasicStep - 0.1 ) );
    int iNRightSteps = ( int ) ( abs( VectDownRight ) / ( m_iBasicStep - 0.1 ) );
    int iNDownSteps = max( iNLeftSteps , iNRightSteps ) ;
    VectDownLeft /= ( double ) iNDownSteps ;
    VectDownRight /= ( double ) iNDownSteps ;
    cmplx DiagVect = pS4->m_imgCoord - pS2->m_imgCoord ;
    ThreePts LeftUp( pS1 , pS2 , pS4 ) ;
    ThreePts DownRight( pS2 , pS3 , pS4 ) ;
    int iNFilled = 0 ;
    for ( int iDown = 0 ; iDown <= iNDownSteps ; iDown++ )
    {
      if ( iDown == 0 )
        iNFilled += FillOneLine( pS1->m_imgCoord , pS2->m_imgCoord , LeftUp ) ;
      else if ( iDown == iNDownSteps )
        iNFilled += FillOneLine( pS4->m_imgCoord , pS3->m_imgCoord , DownRight ) ;
      else
      {
        cmplx LeftPt = pS1->m_imgCoord + ( double ) iDown * VectDownLeft ;
        cmplx RightPt = pS2->m_imgCoord + ( double ) iDown * VectDownRight ;
        cmplx Cross , Dummy ;
        bool bCross = intersect( LeftPt , RightPt , pS1->m_imgCoord , pS3->m_imgCoord , Cross , Dummy ) ;
        if ( bCross )
        {
          iNFilled += FillOneLine( LeftPt , Cross , LeftUp ) ;
          iNFilled += FillOneLine( Cross , RightPt , DownRight ) ;
        }
      }
    }
    return iNFilled ;
  }

  inline int FillTetraGone( cmplx& pS1 , cmplx& pS2 , cmplx& pS3 , cmplx& pS4 ,
    ThreePts Neightbors )
  {
    cmplx VectDownLeft = pS4 - pS1 ;
    cmplx VectDownRight = pS3 - pS2 ;
    int iNLeftSteps = ( int ) ( abs( VectDownLeft ) / ( m_iBasicStep - 0.1 ) );
    int iNRightSteps = ( int ) ( abs( VectDownRight ) / ( m_iBasicStep - 0.1 ) );
    int iNDownSteps = max( iNLeftSteps , iNRightSteps ) ;
    VectDownLeft /= ( double ) iNDownSteps ;
    VectDownRight /= ( double ) iNDownSteps ;
    cmplx OrigLeft = pS1 ;
    cmplx OrigRight = pS2 ;
    int iNFilled = 0 ;
    CRect ImageRect( 0 , 0 , m_ImageSize.cx - 1 , m_ImageSize.cy - 1 ) ;
    for ( int iDown = 0 ; iDown <= iNDownSteps ; iDown++ )
    {
      if ( IsPtInRect( ImageRect , OrigLeft ) && IsPtInRect( ImageRect , OrigRight ) )
      {
        iNFilled += FillOneLine( OrigLeft , OrigRight , Neightbors ) ;
      }
      OrigLeft += VectDownLeft ;
      OrigRight += VectDownRight ;
    }
    return iNFilled ;
  }

  inline int FillTetraGone( cmplx& pS1 , cmplx& pS2 , cmplx& pS3 , cmplx& pS4 ,
    Spot * pFillValue )
  {
    cmplx VectDownLeft = pS4 - pS1 ;
    cmplx VectDownRight = pS3 - pS2 ;
    int iNLeftSteps = ( int ) ( abs( VectDownLeft ) / ( m_iBasicStep - 0.01 ) );
    int iNRightSteps = ( int ) ( abs( VectDownRight ) / ( m_iBasicStep - 0.01 ) );
    int iNDownSteps = max( iNLeftSteps , iNRightSteps ) ;
    VectDownLeft /= ( double ) iNDownSteps ;
    VectDownRight /= ( double ) iNDownSteps ;
    cmplx OrigLeft = pS1 ;
    cmplx OrigRight = pS2 ;
    int iNFilled = 0 ;
    CRect ImageRect( 0 , 0 , m_ImageSize.cx - 1 , m_ImageSize.cy - 1 ) ;
    for ( int iDown = 0 ; iDown <= iNDownSteps ; iDown++ )
    {
      if ( IsPtInRect( ImageRect , OrigLeft ) && IsPtInRect( ImageRect , OrigRight ) )
      {
        iNFilled += FillOneLine( OrigLeft , OrigRight , pFillValue ) ;
      }
      OrigLeft += VectDownLeft ;
      OrigRight += VectDownRight ;
    }
    return iNFilled ;
  }

  inline bool FindNearest( cmplx& ImgPt , ThreeDists** Result )
  {
    int iIndex = GetNeighborIndex( ImgPt ) ;
    if ( 0 <= iIndex && iIndex < m_NearestCalibPts.GetCount() )
    {
      *Result = &m_NearestCalibPts.ElementAt( iIndex ) ;
      return true ;
    }
    return false ;
  }

  inline bool FindNearest( cmplx& ImgPt , ThreePts** Result )
  {
    int iIndex = GetNeighborIndex( ImgPt ) ;
    if ( 0 <= iIndex && iIndex < m_Nearest3Pts.GetCount() )
    {
      *Result = &m_Nearest3Pts.ElementAt( iIndex ) ;
      // return true, if all 3 pts are existing
      return ( ( *Result )->m_Pt1 && ( *Result )->m_Pt2 && ( *Result )->m_Pt3 ) ;
    }
    return false ;
  }

  inline bool FindNearest( cmplx& ImgPt , Spot** Result )
  {
    size_t iIndex = GetNeighborIndex( ImgPt ) ;
    if ( 0 <= iIndex && iIndex < m_NearestSpots.size() )
    {
      *Result = m_NearestSpots.at( iIndex ) ;
      return true ;
    }
    return false ;
  }

  inline Spot * GetSpot( int iX , int iY )
  {
    if ( m_pSpotsAs2DArray && m_SpotArrSize.cx && m_SpotArrSize.cy )
    {
      iX -= m_IndexOffset.x ;
      iY -= m_IndexOffset.y ;
      if ( iX >= 0 && iX < m_SpotArrSize.cx  && iY >= 0 && iY < m_SpotArrSize.cy )
        return m_pSpotsAs2DArray[ m_SpotArrSize.cx * iY + iX ] ;
    }
    return NULL ;
  }
  inline Spot *GetSpot( CPoint Indexes )
  {
    return GetSpot( Indexes.x , Indexes.y ) ;
  }
  inline bool SetSpotPtr( Spot& SpotInfo )
  {
    if ( m_pSpotsAs2DArray && m_SpotArrSize.cx && m_SpotArrSize.cy )
    {
      int iX = SpotInfo.m_Indexes.x ;
      int iY = SpotInfo.m_Indexes.y ;
      iX -= m_IndexOffset.x ;
      iY -= m_IndexOffset.y ;
      if ( iX >= 0 && iX < m_SpotArrSize.cx  && iY >= 0 && iY < m_SpotArrSize.cy )
      {
        if ( m_pSpotsAs2DArray[ m_SpotArrSize.cx * iY + iX ] == NULL )
        {
          m_pSpotsAs2DArray[ m_SpotArrSize.cx * iY + iX ] = &SpotInfo ;
          return true ;
        }
        ASSERT( 0 ) ;
      }
      ASSERT( 0 ) ;
    }
    return false ;
  }
  inline Spot * GetSpotAbs( CPoint Coord )  // this function doesn't check indexes!!!
  {
    return m_pSpotsAs2DArray[ m_SpotArrSize.cx * Coord.y + Coord.x ] ;
  }
  inline Spot * GetSpotAbs( int iX , int iY )  // this function doesn't check indexes!!!
  {
    return m_pSpotsAs2DArray[ m_SpotArrSize.cx * iY + iX ] ;
  }
  inline bool CheckInTable( CPoint Pt )
  {
    return ( ( 0 <= Pt.x ) && ( Pt.x < m_SpotArrSize.cx ) && ( 0 <= Pt.y ) && ( Pt.y < m_SpotArrSize.cy ) ) ;
  }

  inline bool Get3SPotsNotOnOneLine( CPoint& Indexes ,
    Spot* & pFound , Spot* & pFound2 , Spot* & pFound3 )
  {
    if ( m_pSpotsAs2DArray && m_SpotArrSize.cx && m_SpotArrSize.cy )
    {
      CPoint Curr( Indexes );
      Curr -= m_IndexOffset ;
      //       if ( Curr.x < 0  || Curr.x > m_SpotArrSize.cx - 1 
      //         || Curr.y < 0  || Curr.y > m_SpotArrSize.cy - 1 )
      //       {
      if ( Curr.x < 0 )
        Curr.x = 0 ;
      else if ( Curr.x >= m_SpotArrSize.cx )
        Curr.x = m_SpotArrSize.cx - 1 ;
      if ( Curr.y < 0 )
        Curr.y = 0 ;
      else if ( Curr.y >= m_SpotArrSize.cy )
        Curr.y = m_SpotArrSize.cy - 1 ;
      CPoint CentralPoint( m_SpotArrSize.cx / 2 , m_SpotArrSize.cy / 2 ) ;
      //CentralPoint += m_IndexOffset ;


      int iArrIndex = m_SpotArrSize.cx * Curr.y + Curr.x ;
      pFound = m_pSpotsAs2DArray[ iArrIndex ] ;
      pFound2 = NULL ;
      pFound3 = NULL ;
      CPoint Dir( 1 , 0 ) ;
      CPoint Pt( Curr ) ;
      int FoundMask = ( pFound != NULL ) ;
      int iStepCnt = 0 ;
      int iArrSize = ( int ) m_CCWSearchAroundPt.GetCount() ;
      do
      {
        if ( iStepCnt )
        {
          if ( iStepCnt > 17 )
            break ;
          if ( ( iStepCnt % 9 ) == 0 )
          {
            if ( Curr.x < CentralPoint.x - 1 )
              Pt.x += 2 ;
            else if ( Curr.x > CentralPoint.x + 1 )
              Pt.x -= 2 ;
            if ( Curr.y < CentralPoint.y - 1 )
              Pt.y += 2 ;
            else if ( Curr.y > CentralPoint.y + 1 )
              Pt.y -= 2 ;
          }
        }
        if ( !pFound )
        {
          CPoint Step = m_CCWSearchAroundPt[ iStepCnt % iArrSize ] ;
          Pt += Step ;
          iStepCnt++ ;
          if ( CheckInTable( Pt ) )
          {
            pFound = GetSpotAbs( Pt ) ;
            if ( pFound )
            {
              FoundMask = 1 ;
            }
            else
              continue ;
          }
          else
            continue ;
        }
        if ( !pFound2 )
        {
          CPoint Step = m_CCWSearchAroundPt[ iStepCnt % iArrSize ] ;
          Pt += Step ;
          iStepCnt++ ;
          if ( CheckInTable( Pt ) )
          {
            pFound2 = GetSpotAbs( Pt ) ;
            if ( pFound2 )
            {
              bool bTheSameX = ( pFound->m_Indexes.x == pFound2->m_Indexes.x ) ;
              bool bTheSameY = ( pFound->m_Indexes.y == pFound2->m_Indexes.y ) ;
              if ( !bTheSameX )
              {
                if ( !bTheSameY )
                  FoundMask |= 8 ; // diagonal
                else
                  FoundMask |= 4 ; // the same Y
              }
              else
              {
                ASSERT( !bTheSameY ) ;
                FoundMask |= 2 ; // the same X
              }
            }
            else
              continue ;
          }
          else
            continue ;
        }
        if ( !pFound3 )
        {
          CPoint Step = m_CCWSearchAroundPt[ iStepCnt % iArrSize ] ;
          Pt += Step ;
          iStepCnt++ ;
          if ( CheckInTable( Pt ) )
          {
            pFound3 = GetSpotAbs( Pt ) ;
            if ( pFound3 )
            {
              bool bTheSameX = ( pFound->m_Indexes.x == pFound3->m_Indexes.x ) ;
              bool bTheSameY = ( pFound->m_Indexes.y == pFound3->m_Indexes.y ) ;
              if ( !bTheSameX )
              {
                if ( !bTheSameY )
                  FoundMask |= 8 ; // diagonal
                else
                  FoundMask |= 4 ; // the same Y
              }
              else
              {
                ASSERT( !bTheSameY ) ;
                FoundMask |= 2 ; // the same X
              }
              if ( GetBitsCount( FoundMask ) < 3 )
              {
                pFound3 = NULL ;
                continue ;
              }
              return true ;
            }
            else
              continue ;
          }
          else
            continue ;
        }
      } while ( !pFound || !pFound2 || !pFound3 );
    }
    return false ;
  }

  inline Spot * GetAdjustedSpot( CPoint& Indexes , CPoint * pBase1 = NULL , CPoint * pBase2 = NULL )
  {
    if ( m_pSpotsAs2DArray && m_SpotArrSize.cx && m_SpotArrSize.cy )
    {
      CPoint Curr( Indexes );
      Curr -= m_IndexOffset ;
      if ( Curr.x < 0 )
        Curr.x = 0 ;
      else if ( Curr.x >= m_SpotArrSize.cx )
        Curr.x = m_SpotArrSize.cx - 1 ;
      if ( Curr.y < 0 )
        Curr.y = 0 ;
      else if ( Curr.y >= m_SpotArrSize.cy )
        Curr.y = m_SpotArrSize.cy - 1 ;
      int iArrIndex = m_SpotArrSize.cx * Curr.y + Curr.x ;
      Spot * pFound = NULL ;
      do
      {
        pFound = m_pSpotsAs2DArray[ iArrIndex ] ;
        if ( pFound )
        {
          Indexes = Curr + m_IndexOffset ;
          return pFound ;
        }
        do
        {
          if ( ( Curr.x == 0 ) || ( Curr.x == m_SpotArrSize.cx - 1 ) )
          {
            if ( Curr.y > 0 )
            {
              pFound = GetSpotAbs( Curr.x , Curr.y - 1 ) ;
              if ( pFound )
              {
                Curr.y-- ;
                Indexes = Curr + m_IndexOffset ;
                return pFound ;
              }
            }
            if ( Curr.y < m_SpotArrSize.cy - 1 )
            {
              pFound = GetSpotAbs( Curr.x , Curr.y + 1 ) ;
              if ( pFound )
              {
                Curr.y++ ;
                Indexes = Curr + m_IndexOffset ;
                return pFound ;
              }
            }
          }
          if ( ( Curr.y == 0 ) || ( Curr.y == m_SpotArrSize.cy - 1 ) )
          {
            if ( Curr.x > 0 )
            {
              pFound = GetSpotAbs( Curr.x - 1 , Curr.y ) ;
              if ( pFound )
              {
                Curr.x-- ;
                Indexes = Curr + m_IndexOffset ;
                return pFound ;
              }
            }
            if ( Curr.x < m_SpotArrSize.cx - 1 )
            {
              pFound = GetSpotAbs( Curr.x + 1 , Curr.y ) ;
              if ( pFound )
              {
                Curr.x++ ;
                Indexes = Curr + m_IndexOffset ;
                return pFound ;
              }
            }
          }
          if ( Indexes.x > 0 )
            Curr.x-- ;
          else if ( Indexes.x < 0 )
            Curr.x++ ;
          if ( Indexes.y > 0 )
            Curr.y-- ;
          else if ( Indexes.y < 0 )
            Curr.y++ ;
          if ( pBase1 && ( Curr == *pBase1 ) )
            continue ;
          if ( pBase2 && ( Curr == *pBase2 ) )
            continue ;
          if ( pBase1 && pBase2 )
          {
            if ( Curr.x == pBase1->x  &&  Curr.x == pBase2->x )
            {
              Curr.y = pBase1->y ;
              Curr.x += ( Indexes.x < 0 ) ? 1 : -1 ;
              if ( Curr.x == pBase1->x  &&  Curr.x == pBase2->x )
              {
                Curr.y = pBase1->y ;
                Curr.x += ( Indexes.x < 0 ) ? 1 : -1 ;
              }
            }
            if ( Curr.y == pBase1->y  &&  Curr.y == pBase2->y )
            {
              Curr.x = pBase1->x ;
              Curr.y += ( Indexes.y < 0 ) ? 1 : -1 ;
            }
          }
          iArrIndex = m_SpotArrSize.cx * Curr.y + Curr.x ;
          break ;
        } while ( 1 );
      } while ( !pFound );

    }
    return NULL ;
  }
  CPoint CheckIndexes( CPoint Indexes , bool bStepOut = true )
  {
    if ( Indexes.x < m_IndexOffset.x )
    {
      Indexes.x = m_IndexOffset.x ;
      if ( bStepOut )
        Indexes.x++ ;
    }
    if ( Indexes.x - m_IndexOffset.x >= m_SpotArrSize.cx )
    {
      Indexes.x = m_IndexOffset.x + m_SpotArrSize.cx - 1 ;
      if ( bStepOut )
        Indexes.x-- ;
    }
    if ( Indexes.y < m_IndexOffset.y )
    {
      Indexes.y = m_IndexOffset.y ;
      if ( bStepOut )
        Indexes.y++ ;
    }
    if ( Indexes.y - m_IndexOffset.y >= m_SpotArrSize.cy )
    {
      Indexes.y = m_IndexOffset.y + m_SpotArrSize.cy - 1 ;
      if ( bStepOut )
        Indexes.y-- ;
    }
    return Indexes ;
  }
  inline cmplx ImgToWorld( cmplx& Img )
  {
    cmplx World ;
    if ( ImgToWorld( Img , World ) )
      return World ;
    else
      return cmplx( 0. , 0. ) ;
  }

  inline bool ImgToWorld( cmplx& Img , cmplx& World )
  {
    //     ThreeDists * Nearest = NULL ;
    //     if ( FindNearest( Img , &Nearest) )
    //     {
    //       if ( Nearest->D1.m_pSpot && Nearest->D2.m_pSpot && Nearest->D3.m_pSpot )
    //       {
    //         CoordsCorresp First(  Nearest->D1.m_pSpot->m_imgCoord , Nearest->D1.m_pSpot->m_WCoord ) ;
    //         CoordsCorresp Second(  Nearest->D2.m_pSpot->m_imgCoord , Nearest->D2.m_pSpot->m_WCoord ) ;
    //         CoordsCorresp Third(  Nearest->D3.m_pSpot->m_imgCoord , Nearest->D3.m_pSpot->m_WCoord ) ;
    //         CoordsCorresp Measured( Img , Img ) ;
    //         if ( ConvertBy3Pts( First , Second, Third , Measured) )
    //         {
    //           World = Measured.World ;
    //           return true ;
    //         }
    //       }
    //     }
    bool bRes = ImgToWorldByNearestPt( Img , World ) ;
    return bRes ;
  }

  inline bool ImgToWorld( const CDPoint& Img , CDPoint& World )
  {
    cmplx TmpWorld ;
    cmplx TmpImg( Img.x , Img.y ) ;
    if ( ImgToWorld( TmpImg , TmpWorld ) )
    {
      if ( m_bUseCenter )
        TmpWorld -= m_CenterSpot.m_WCoord ;
      World.x = TmpWorld.real() ;
      World.y = TmpWorld.imag() ;
      return true ;
    }
    return false ;
  }

  inline bool ImgToWorldByTriangle( cmplx& Img , cmplx& World ,
    ThreePts * pKnown )
  {
    CoordsCorresp Pt1( pKnown->m_Pt1->m_imgCoord , pKnown->m_Pt1->m_WCoord ) ;
    CoordsCorresp Pt2( pKnown->m_Pt2->m_imgCoord , pKnown->m_Pt2->m_WCoord ) ;
    CoordsCorresp Pt3( pKnown->m_Pt3->m_imgCoord , pKnown->m_Pt3->m_WCoord ) ;
    return ConvertBy3Pts( Pt1 , Pt2 , Pt3 , Img , World ) ;
  }

  inline bool ImgToWorldByPtAndDir( cmplx& Img , cmplx& World )
  {
    int iIndexInTable = GetIndexInPtAndDirArray( Img ) ;
    PtAndDir& Nearest = m_SpotsAndDirs.GetAt( iIndexInTable ) ;

    CoordsCorresp PtNear( Nearest.m_pNearest->m_imgCoord , Nearest.m_pNearest->m_WCoord ) ;
    CoordsCorresp Pt2 ;
    CoordsCorresp Pt3 ;
    if ( GetNearestCorresps( Nearest.m_pNearest , Nearest.m_Dir , Pt2 , Pt3 ) )
    {
      return ConvertByNearestAndSegment( PtNear , Pt2 , Pt3 , Img , World ) ;
    }
    return false ;
  }

  inline cmplx WorldToImg( cmplx& World )
  {
    return cmplx( 0. , 0. ) ;
  }

  inline int FindAdditionalLines( bool bUpper , double dPosTol ,
    cmplx& cEastDir , cmplx& cNOrthDir )
  {
    bool bLineFound = true ;
    int iBefore = ( int ) m_HorLines.size() ;
    while ( bLineFound )
    {
      Axis * pEdgeLine = !bUpper ? m_HorLines.front() : m_HorLines.back() ;
      Node1D * pFirst = pEdgeLine->GetHead() ;
      Spot * pSpot = pFirst->m_Spot ;
      cmplx ToNextLine = ( bUpper ) ? pSpot->m_DirN : pSpot->m_DirS ;
      if ( abs( ToNextLine ) == 0. )
        ToNextLine = -( ( bUpper ) ? pSpot->m_DirS : pSpot->m_DirN ) ;
      double dStep = abs( ToNextLine ) ;
      ASSERT( dStep ) ;
      Axis * pNewHorLine = NULL ;
      Axis * pPrevFound = pEdgeLine ;
      Node1D * pOnLine0 = pFirst ;
      double dAbsTol = dPosTol * dStep ;
      int iNMissed = 0 ;
      Node1D * pLastFound = NULL ;
      cmplx DefUp ;
      while ( pOnLine0 )
      {
        cmplx OnNewLine = pOnLine0->m_Spot->m_imgCoord + ToNextLine /*
                  + ( (bUpper) ? pOnLine0->m_Spot->m_DirN  : pOnLine0->m_Spot->m_DirS )*/ ;
        if ( IsPtInImage( OnNewLine ) )
        {
          Node1D * pFound = m_Spots->find( OnNewLine , dAbsTol ) ;
          if ( pFound )
          {
            if ( !pNewHorLine )
              pNewHorLine = new Axis ;
            if ( iNMissed )
            {
              cmplx RealStepOnImage = ( pFound->m_Spot->m_imgCoord - pLastFound->m_Spot->m_imgCoord ) / ( double ) ( 1.0 + iNMissed ) ;
              for ( int i = 0 ; i < iNMissed ; i++ )
              {
                cmplx ForInsert( pLastFound->m_Spot->m_imgCoord + RealStepOnImage * ( double ) i ) ;
                Spot * AddedSpot = new Spot( -1 , ForInsert , 0. , 0. , AXIS_X ) ;
                AddedSpot->m_Indexes = pLastFound->m_Spot->m_Indexes + CPoint( i + 1 , 0 ) ;
                m_AddedSpots.push_back( AddedSpot ) ;
                pNewHorLine->AddToTail( AddedSpot ) ;
              }
            }

            pFound->m_Spot->m_Indexes = pOnLine0->m_Spot->m_Indexes + CPoint( 0 , bUpper ? 1 : -1 ) ;
            pFound->m_Spot->m_WCoord = pOnLine0->m_Spot->m_WCoord
              + ( bUpper ? cNOrthDir : -cNOrthDir ) ;
            ASSERT( !( pFound->m_Spot->m_OnLines & AXIS_X ) ) ;
            pFound->m_Spot->m_OnLines |= AXIS_X ;
            pNewHorLine->AddToTail( pFound->m_Spot ) ;
            iNMissed = 0 ;
            pLastFound = pFound ;
            bLineFound = true ;
            DefUp = pFound->m_Spot->m_imgCoord - pOnLine0->m_Spot->m_imgCoord ;
            if ( !bUpper )
              DefUp = -DefUp ;
          }
          else if ( pNewHorLine )
            iNMissed++ ;
        }
        pOnLine0 = pOnLine0->next ;
      }
      if ( pNewHorLine )
      {
        pNewHorLine->ExpandHorAxis( m_Spots , dPosTol ,
          m_iInsertMissed , &m_AddedSpots , pNewHorLine->GetHead()->m_Spot ,
          pNewHorLine->GetHead()->m_Spot->m_DirN ) ;
        if ( !pNewHorLine->CalcMainVectors() )
        {
          pNewHorLine->SetMainVector( pPrevFound->GetMainVector() ) ;
          pNewHorLine->SetWMainVector( pPrevFound->GetWMainVector() ) ;
        }
        pNewHorLine->FindNeighborsOnHLine( m_Spots->GetNodeList() , dPosTol , m_AddedSpots ,
          pNewHorLine->GetMainVector() , DefUp , -DefUp ) ;

        if ( bUpper )
          m_HorLines.push_back( pNewHorLine ) ;
        else
          m_HorLines.insert( m_HorLines.begin() , pNewHorLine ) ;
        pPrevFound = pNewHorLine ;
      }
      else
        bLineFound = false ;
    }
    return ( int ) m_HorLines.size() - iBefore ;
  }

  inline int FindUpDown( cmplx& Pt , cmplx& DirUp ,
    double dAbsTol , cmplx& PtUp , cmplx& PtDown )
  {
    int iRes = 0 ;
    cmplx PtUpPos = Pt + DirUp ;
    Spot * pUp = m_Spots->FindSpot( PtUpPos , dAbsTol ) ;
    if ( pUp || ( pUp = Find( m_AddedSpots , PtUpPos , dAbsTol ) ) )
    {
      iRes |= PT_UP_FOUND ;
      PtUp = pUp->m_imgCoord - Pt ;
    }
    else
      PtUp = cmplx( 0. , 0. ) ;

    cmplx PtDownPos = Pt - DirUp ;
    Spot * pDown = m_Spots->FindSpot( PtDownPos , dAbsTol ) ;
    if ( pDown || ( pDown = Find( m_AddedSpots , PtDownPos , dAbsTol ) ) )
    {
      iRes |= PT_DOWN_FOUND ;
      PtDown = pDown->m_imgCoord - Pt ;
    }
    else
      PtDown = cmplx( 0. , 0. ) ;
    return iRes ;
  }

  // 
  // Get Axis finds neighbor spots from pFirstPt to StepOnImage and opposite directions
  // normalized searching tolerance is dPosTol
  // pFirstPt has WorldOrigin world coordinates and step between points is  WorldStep
  // For every found point the World coordinates and indexes in calibration network will be filled
  // Omitted spots will be generated and saved in m_AddedSpots array
  inline Axis * GetAxis( Node1D * pFirstPt , const cmplx& StepOnImage ,
    double dPosTol , const cmplx& WorldOrigin ,
    const cmplx& WorldStep , CPoint Indexes )
  {
    cmplx NorthStep = WorldStep * cmplx( 0. , 1. ) ;
    double dAbsTol = abs( StepOnImage ) * dPosTol ;
    Axis * pNewAxis = new Axis() ;
    pFirstPt->m_Spot->m_OnLines |= AXIS_X ;
    CPoint CurrIndex = Indexes ;
    pFirstPt->m_Spot->m_Indexes = CurrIndex ;
    cmplx CurrWorld = WorldOrigin ;
    pFirstPt->m_Spot->m_WCoord = WorldOrigin ;
    pNewAxis->AddToHead( pFirstPt->m_Spot ) ;
    Node1D * pLastPt = pFirstPt ;
    //       int iCnt5 = m_Spots->GetNodeList()->GetRealLength() ;
    cmplx RealStepOnImage = StepOnImage ;
    Node1D * pLeft = NULL ;
    int iMissedCnt = 0 ;
    int iAddedToLeft = 0 ;
    while ( !pLeft && ( iMissedCnt <= m_iInsertMissed ) )
    {            // did not find, try to find next one
      cmplx OnLeft( pLastPt->m_Spot->m_imgCoord - ( RealStepOnImage * ( double ) ( iMissedCnt + 1 ) ) ) ; // look on left side (minus direction of X)
      if ( IsPtInImage( OnLeft ) )
      {
        pLeft = m_Spots->find( OnLeft , dAbsTol * ( 1.0 + iMissedCnt ) ) ;
        if ( !pLeft )
        {
          iMissedCnt++ ;
          continue ;
        }
        else
        {
          iAddedToLeft += iMissedCnt + 1 ;
          RealStepOnImage = -( pLeft->m_Spot->m_imgCoord - pLastPt->m_Spot->m_imgCoord ) / ( double ) ( 1.0 + iMissedCnt ) ;
          if ( iMissedCnt )
          {
            for ( int i = 1 ; i <= iMissedCnt ; i++ )
            {
              cmplx ForInsert( pLastPt->m_Spot->m_imgCoord - RealStepOnImage * ( double ) i ) ;
              Spot * AddedSpot = new Spot( -1 , ForInsert , 0. , 0. , AXIS_X ) ;
              CurrIndex.x-- ;
              AddedSpot->m_Indexes = CurrIndex ;
              CurrWorld -= WorldStep ;
              AddedSpot->m_WCoord = CurrWorld ;
              AddedSpot->m_DirE = -RealStepOnImage ;
              AddedSpot->m_DirW = RealStepOnImage ;
              FindUpDown( ForInsert , -RealStepOnImage * cmplx( 0. , -1. ) , dAbsTol ,
                AddedSpot->m_DirN , AddedSpot->m_DirS ) ;
              m_AddedSpots.push_back( AddedSpot ) ;
              pNewAxis->AddToHead( AddedSpot ) ;
            }
            iMissedCnt = 0 ;
          }
          pLastPt->m_Spot->m_DirW = -RealStepOnImage ;
          pLeft->m_Spot->m_DirE = RealStepOnImage ;
          pLeft->m_Spot->m_DirW = RealStepOnImage ;
          pLeft->m_Spot->m_OnLines |= AXIS_X ;
          CurrIndex.x-- ;
          FindUpDown( pLastPt->m_Spot->m_imgCoord ,
            -RealStepOnImage * cmplx( 0. , -1. ) , dAbsTol ,
            pLastPt->m_Spot->m_DirN , pLastPt->m_Spot->m_DirS ) ;
          pLeft->m_Spot->m_Indexes = CurrIndex ;
          CurrWorld -= WorldStep ;
          pLeft->m_Spot->m_WCoord = CurrWorld ;
          pNewAxis->AddToHead( pLeft->m_Spot ) ;
          pLastPt = pLeft ;
          pLeft = NULL ;
        }
      }
      else
        break ;
    }
    pLastPt = pFirstPt ;
    CurrIndex = Indexes ;
    CurrWorld = WorldOrigin ;
    RealStepOnImage = StepOnImage ;
    Node1D * pRight = NULL ;
    iMissedCnt = 0 ;
    int iAddedToRight = 0 ;
    while ( !pRight && ( iMissedCnt <= m_iInsertMissed ) )
    {
      cmplx OnRight( pLastPt->m_Spot->m_imgCoord + ( RealStepOnImage * ( double ) ( iMissedCnt + 1 ) ) ) ; // look on left side (minus direction of X)
      if ( IsPtInImage( OnRight ) )
      {
        pRight = m_Spots->find( OnRight , dAbsTol * ( iMissedCnt + 1 ) ) ;
        if ( !pRight )
        {
          iMissedCnt++ ;
          continue ;
        }
        else
        {
          iAddedToRight += iMissedCnt + 1 ;
          RealStepOnImage = ( pRight->m_Spot->m_imgCoord - pLastPt->m_Spot->m_imgCoord ) / ( double ) ( 1.0 + iMissedCnt ) ;
          if ( iMissedCnt )
          {
            for ( int i = 1 ; i <= iMissedCnt ; i++ )
            {
              cmplx ForInsert( pLastPt->m_Spot->m_imgCoord + RealStepOnImage * ( double ) i ) ;
              Spot * AddedSpot = new Spot( -1 , ForInsert , 0. , 0. , AXIS_X ) ;
              CurrIndex.x++ ;
              AddedSpot->m_Indexes = CurrIndex ;
              CurrWorld += WorldStep ;
              AddedSpot->m_WCoord = CurrWorld ;
              AddedSpot->m_DirE = -RealStepOnImage ;
              AddedSpot->m_DirW = RealStepOnImage ;
              FindUpDown( ForInsert , RealStepOnImage * cmplx( 0. , -1. ) , dAbsTol ,
                AddedSpot->m_DirN , AddedSpot->m_DirS ) ;
              m_AddedSpots.push_back( AddedSpot ) ;
              pNewAxis->AddToTail( AddedSpot ) ;
            }
            iMissedCnt = 0 ;
          }
          pLastPt->m_Spot->m_DirW = RealStepOnImage ;
          pRight->m_Spot->m_DirE = RealStepOnImage ;
          pRight->m_Spot->m_DirW = -RealStepOnImage ;
          FindUpDown( pRight->m_Spot->m_imgCoord ,
            -RealStepOnImage * cmplx( 0. , -1. ) , dAbsTol ,
            pRight->m_Spot->m_DirN , pRight->m_Spot->m_DirS ) ;
          pRight->m_Spot->m_OnLines |= AXIS_X ;
          CurrIndex.x++ ;
          pRight->m_Spot->m_Indexes = CurrIndex ;
          CurrWorld += WorldStep ;
          pRight->m_Spot->m_WCoord = CurrWorld ;
          pNewAxis->AddToTail( pRight->m_Spot ) ;
          pLastPt = pRight ;
          pRight = NULL ;
        }
      }
      else
        break ;
    }
    //int iAdded = ExpandAxis( pNewAxis , m_Spots , dPosTol , AXIS_X ) ;
    //     pNewAxis->EnumerateAxis( pFirstPt->m_Spot->m_imgCoord , // point on img 
    //       WorldOrigin , WorldStep , Indexes ) ; 
    pNewAxis->CalcMainVectors() ;
    return pNewAxis ;
  }

  // 	cmplx* getCoordinates(double x, double y)
  // 	{
  // 		cmplx point(x,y);
  // 
  // 		cmplx* res = getArbitaryCoordinates(point);
  // 
  // 		return res;
  // 	}
  bool SaveCalibData( FILE * fw )
  {
    int iArrayLen = m_SpotArrSize.cx * m_SpotArrSize.cy ;
    if ( m_pAxisX && m_pAxisY && iArrayLen )
    {
      fprintf_s( fw , _T( "ImageSize(%d,%d)\r\n" ) , m_ImageSize.cx , m_ImageSize.cy ) ;
      fprintf_s( fw , _T( "MatrixSize(%d,%d)\r\n" ) , m_SpotArrSize.cx , m_SpotArrSize.cy ) ;
      fprintf_s( fw , _T( "Offsets(%d,%d)\r\n" ) , m_IndexOffset.x , m_IndexOffset.y ) ;
      fprintf_s( fw , _T( "NBigSpots(%d,%d)\r\n" ) , m_NBigSpots.cx , m_NBigSpots.cy ) ;
      fprintf_s( fw , _T( "PosTolerance(%g)\r\n" ) , m_dPosTol ) ;
      fprintf_s( fw , _T( "InterpointStep(%g)\r\n" ) , m_dInterpointStep ) ;
      fprintf_s( fw , _T( "CentralSpot\r\n" ) ) ;
      m_CenterSpot.SaveCalibData( fw , _T( "\r\n" ) ) ;

      fprintf_s( fw , _T( "SpotData\r\n<\r\n" ) ) ;
      for ( int i = 0 ; i < iArrayLen ; i++ )
      {
        Spot * pSpot = m_pSpotsAs2DArray[ i ] ;
        if ( pSpot )
          pSpot->SaveCalibData( fw , _T( "\r\n" ) ) ;
        else
          fprintf_s( fw , _T( "No Spot %d\r\n" ) , i ) ;
      }
      fprintf_s( fw , _T( ">\r\n" ) ) ;
      m_pAxisX->SaveAxis( fw , _T( "AxisX\r\n <\r\n" ) , ">\r\n" ) ;
      m_pAxisY->SaveAxis( fw , _T( "AxisY\r\n <\r\n" ) , ">\r\n" ) ;

      return true ;
    }
    return false ;
  }
  bool SaveCalibData( FXString& Result )
  {
    int iArrayLen = m_SpotArrSize.cx * m_SpotArrSize.cy ;
    if ( m_pAxisX && m_pAxisY && iArrayLen )
    {
      TCHAR buf[ 200 ] ;
      _stprintf_s( buf , _T( "ImageSize(%d,%d)\r\n" ) , m_ImageSize.cx , m_ImageSize.cy ) ;
      Result += buf ;
      _stprintf_s( buf , _T( "MatrixSize(%d,%d)\r\n" ) , m_SpotArrSize.cx , m_SpotArrSize.cy ) ;
      Result += buf ;
      _stprintf_s( buf , _T( "Offsets(%d,%d)\r\n" ) , m_IndexOffset.x , m_IndexOffset.y ) ;
      Result += buf ;
      _stprintf_s( buf , _T( "NBigSpots(%d,%d)\r\n" ) , m_NBigSpots.cx , m_NBigSpots.cy ) ;
      Result += buf ;
      _stprintf_s( buf , _T( "PosTolerance(%g)\r\n" ) , m_dPosTol ) ;
      Result += buf ;
      _stprintf_s( buf , _T( "InterpointStep(%g)\r\n" ) , m_dInterpointStep ) ;
      Result += buf ;
      _stprintf_s( buf , _T( "CentralSpot\r\n" ) ) ;
      Result += buf ;
      m_CenterSpot.SaveCalibData( Result , _T( "\r\n" ) ) ;

      Result += _T( "SpotData\r\n<\r\n" ) ;
      for ( int i = 0 ; i < iArrayLen ; i++ )
      {
        Spot * pSpot = m_pSpotsAs2DArray[ i ] ;
        if ( pSpot )
          pSpot->SaveCalibData( Result , _T( "\r\n" ) ) ;
        else
        {
          _stprintf_s( buf , _T( "No Spot %d\r\n" ) , i ) ;
          Result += buf ;
        }
      }
      Result += _T( ">\r\n" ) ;
      m_pAxisX->SaveAxis( Result , _T( "AxisX\r\n <\r\n" ) , ">\r\n" ) ;
      m_pAxisY->SaveAxis( Result , _T( "AxisY\r\n <\r\n" ) , ">\r\n" ) ;

      return true ;
    }
    return false ;
  }

  bool RestoreCalibData( FILE * fr )
  {
    TCHAR ReadBuf[ 2000 ] ;
    CPoint Tmp ;
    double dTmp ;
    bool bSize = false ;
    bool bOffsets = false ;
    bool bCenter = false ;

    while ( _fgetts( ReadBuf , 1999 , fr ) )
    {
      FXParser2 Buf( ReadBuf ) ;
      Buf.Trim() ;
      if ( Buf.Find( _T( "//" ) ) == 0 )
        continue ;
      FXSIZE iPos = Buf.Find( _T( "ImageSize(" ) ) ;
      if ( iPos >= 0 )
      {
        if ( bSize = StrToIPt( ( LPCTSTR ) Buf + iPos + 10 , Tmp ) )
          memcpy( &m_ImageSize , &Tmp , sizeof( Tmp ) ) ;
      }
      else if ( ( iPos = Buf.Find( _T( "MatrixSize(" ) ) ) >= 0 )
      {
        if ( bSize = StrToIPt( ( LPCTSTR ) Buf + iPos + 11 , Tmp ) )
          memcpy( &m_SpotArrSize , &Tmp , sizeof( Tmp ) ) ;
      }
      else if ( ( iPos = Buf.Find( _T( "Offsets(" ) ) ) >= 0 )
      {
        if ( bOffsets = StrToIPt( ( LPCTSTR ) Buf + iPos + 8 , Tmp ) )
          m_IndexOffset = Tmp ;
      }
      else if ( ( iPos = Buf.Find( _T( "NBigSpots(" ) ) ) >= 0 )
      {
        if ( bOffsets = StrToIPt( ( LPCTSTR ) Buf + iPos + 10 , Tmp ) )
          m_NBigSpots = Tmp ;
      }
      else if ( ( iPos = Buf.Find( _T( "PosTolerance(" ) ) ) >= 0 )
      {
        if ( bOffsets = Buf.StrToDbl( ( LPCTSTR ) Buf + iPos + 13 , dTmp ) )
          m_dPosTol = dTmp ;
      }
      else if ( ( iPos = Buf.Find( _T( "InterpointStep(" ) ) ) >= 0 )
      {
        if ( bOffsets = Buf.StrToDbl( ( LPCTSTR ) Buf + iPos + 15 , dTmp ) )
          m_dInterpointStep = dTmp ;
      }
      else if ( ( iPos = Buf.Find( _T( "CentralSpot" ) ) ) >= 0 )
      {
        bCenter = m_CenterSpot.RestoreCalibData( fr ) > 0 ;
      }
      else if ( ( iPos = Buf.Find( _T( "SpotData" ) ) ) >= 0 )
      {
        int iCnt = 0 ;
        while ( !_tcschr( ReadBuf , _T( '<' ) )
          && _fgetts( ReadBuf , 1999 , fr ) && ++iCnt < 10 ) ;
        if ( iCnt < 10 && bSize )
        {
          if ( m_pSpotsAs2DArray )
          {
            delete m_pSpotsAs2DArray ;
            m_pSpotsAs2DArray = NULL ;
            m_Spots->delAll() ;
          }
          int iArrayLen = m_SpotArrSize.cx * m_SpotArrSize.cy ;
          m_pSpotsAs2DArray = new Spot*[ iArrayLen ] ;
          memset( m_pSpotsAs2DArray , 0 , iArrayLen * sizeof( Spot* ) ) ;
          for ( int i = 0 ; i < iArrayLen ; i++ )
          {
            Spot * pSpot = new Spot ;
            int iRes = pSpot->RestoreCalibData( fr ) ;
            switch ( iRes )
            {
              case 1:
              {
                m_Spots->insert( pSpot ) ;
                m_pSpotsAs2DArray[ i ] = pSpot ;
                pSpot->m_OnLines = 0 ;
              }
              break ;
              case 0:
                delete pSpot ;
                ASSERT( 0 ) ;
                break ;
              case -1:
                delete pSpot ;
                ASSERT( 0 ) ;
                break ;
              case -2:
                delete pSpot ;
                break ;
              default:
                delete pSpot ;
                ASSERT( 0 ) ;
                break ;
            }
          }
        }
      }
      else if ( ( iPos = Buf.Find( _T( "AxisX" ) ) ) >= 0 )
      {
        if ( m_pAxisX )
        {
          delete m_pAxisX ;
          m_pAxisX = NULL ;
        }
        //         m_pAxisX = new Axis ;
        //         m_pAxisX->RestoreAxis( fr , m_Spots ) ;
        //         delete m_pAxisX ;
        //         m_pAxisX = NULL ;
      }
      else if ( ( iPos = Buf.Find( _T( "AxisY" ) ) ) >= 0 )
      {
        if ( m_pAxisY )
        {
          delete m_pAxisY ;
          m_pAxisY = NULL ;
        }
        //         m_pAxisY = new Axis ;
        //         m_pAxisY->RestoreAxis( fr , m_Spots ) ;
        //         delete m_pAxisY ;
        //         m_pAxisY = NULL ;
      }
    }
    if ( InitFromSpotData() )
      return BuildDistIndexTableForOnePoint() ;
    //      return BuildDistIndexTableWith3Pts() ;   
    else
    {
      Clean() ;
      return false ;
    }
  }

  bool ImgToWorldByNearestPt( cmplx& Img , cmplx& Result )
  {
    // 1. Try to use 1 pt and direction table
    if ( m_SpotsAndDirs.GetCount() )
    {
      return ImgToWorldByPtAndDir( Img , Result ) ;
    }

    // 2. try to use table of triangles
    ThreePts * pTriangle = NULL ;
    if ( FindNearest( Img , &pTriangle ) )
    {
      return ImgToWorldByTriangle( Img , Result , pTriangle ) ;
    }

    Spot * pNearest ;
    if ( FindNearest( Img , &pNearest ) && pNearest )
    {
      cmplx X , Y ;
      cmplx Vect = Img - pNearest->m_imgCoord ;
      double signX = 1.0 , signY = 1.0 ;

      if ( abs( Vect ) < EPSILON )  // 1E-19
      {
        Result = pNearest->m_WCoord ;
        return true ;
      }
      double angToE = GetAngleBtwVects( Vect , pNearest->m_DirE ) ;
      double angToN = GetAngleBtwVects( Vect , pNearest->m_DirN ) ;
      double angToW = GetAngleBtwVects( Vect , pNearest->m_DirW ) ;
      double angToS = GetAngleBtwVects( Vect , pNearest->m_DirS ) ;
      if ( angToE <= 0. ) // East on right side
      {
        if ( angToN >= 0. )   // to North is on other side from East
        {
          X = pNearest->m_DirE ;
          Y = pNearest->m_DirN ;
        }
        else // North on the same side with East
        {
          if ( angToW > 0. ) // West on other side
          {
            X = pNearest->m_DirW ;
            Y = pNearest->m_DirN ;
            signX = -1.0 ;
          }
          else // W,N,E are on the same side: rare case of lens or matrix big distortion
          {
            X = pNearest->m_DirW ;
            Y = pNearest->m_DirS ;
            signX = signY = -1.0 ;
          }
        }
      }
      else // East on left side 
        if ( angToS <= 0. ) // South on right side 
        {
          X = pNearest->m_DirE ;
          Y = pNearest->m_DirS ;
          signY = -1.0 ;
        }
        else // South also on left side
        {
          if ( angToW < 0. )      // west on right side
          {
            X = pNearest->m_DirW ;
            Y = pNearest->m_DirS ;
            signX = signY = -1.0 ;
          }
          else  //W,S,E are on the same side: rare case of lens or matrix big distortion
          {
            X = pNearest->m_DirW ;
            Y = pNearest->m_DirN ;
            signX = -1.0 ;
          }
        }

      CLine2d LineX , LineY , ToX , ToY ;
      cmplx CrossX , CrossY , VectToX , VectToY ;

      LineX.ByPointAndDir( pNearest->m_imgCoord , X ) ;
      VectToX = -Y ;
      ToX.ByPointAndDir( Img , VectToX ) ;
      bool bX = LineX.intersect( ToX , CrossX ) ;
      ASSERT( bX ) ;
      cmplx SegmentX( CrossX - pNearest->m_imgCoord ) ;
      cmplx PartX = SegmentX / X ;
      double dX = pNearest->m_WCoord.real() + signX * m_dInterpointStep * abs( PartX ) ;

      LineY.ByPointAndDir( pNearest->m_imgCoord , Y ) ;
      VectToY = -X ;
      ToY.ByPointAndDir( Img , VectToY ) ;
      bool bY = LineY.intersect( ToY , CrossY ) ;
      ASSERT( bY ) ;
      cmplx SegmentY( CrossY - pNearest->m_imgCoord ) ;
      cmplx PartY = SegmentY / Y ;
      double dY = pNearest->m_WCoord.imag() + signY * m_dInterpointStep * abs( PartY ) ;
      //       SENDINFO( "Img(%g,%g) NearIm(%g,%g) Part(%g,%g) Signs(%d,%d)" , 
      //         Img.real() , Img.imag() , 
      //         m_pSNearest->m_imgCoord.real() , m_pSNearest->m_imgCoord.imag() , 
      //         abs(PartX) , abs(PartY) , (int)signX , (int)signY ) ;

      Result = cmplx( dX , dY ) ;
      if ( m_bUseCenter )
        Result -= m_CenterSpot.m_WCoord ;
      return true ;
    }
    return false ;
  }
  bool WorldToImgByNearestPt( cmplx& World , cmplx& Result )
  {

    return true ;
  }
  friend class CoordMapGadget;
  friend class SpotsDataProcessing;
  friend class SpotStatistics ;
};


#endif	//	__INC__Grid_H__
