/*
*
* Copyright 2022 by File X Ltd., Ness Ziona, Israel
*
* All Rights Reserved
*
* Permission to use, copy, modify and distribute this
* software and its documentation for any purpose is
* restricted according to the software end user license
* attached to this software.
*
* Any use of this software is subject to the limitations
* of warranty and liability contained in the end user
* license.  Without derogating from the abovesaid,
* File X Ltd. disclaims all warranty with regard to
* this software, including all implied warranties of
* merchantability and fitness.  In no event shall
* File X Ltd. be held liable for any special, indirect
* or consequential damages or any damages whatsoever
* resulting from loss of use, data or profits, whether in
* an action of contract, negligence or other tortious
* action, arising out of or in connection with the use or
* performance of this software.
*
*/


// SpotStatistics.h : Implementation of the SpotStatistics class


#include "StdAfx.h"
#include "SpotStatistics.h"
#include "helpers\FramesHelper.h"
#include "imageproc\ExtractVObjectResult.h"
#include <vector>
#include "Math\FRegression.h"
#include "afxcoll.h"
#include <fxfc/FXRegistry.h>

USER_FILTER_RUNTIME_GADGET( SpotStatistics , "Statistics" );

SpotStatistics::SpotStatistics()
{
  m_dDistBetweenSquares_um = 8000.0 ;
  m_LineAreaIDPrefix = _T( "lines" ) ;
  init();
}

CDataFrame* SpotStatistics::DoProcessing( const CDataFrame* pDataFrame )
{
  if ( ( pDataFrame ) && ( !Tvdb400_IsEOS( pDataFrame ) ) )
  {
    CContainerFrame * pMarking = NULL ;
    if ( m_cLastScaleViewPoint.real() == 0. )
      m_cLastScaleViewPoint = cmplx( 1000. , 1000. ) ;

    const CTextFrame * pAreaDescriptor = pDataFrame->GetTextFrame( ( LPCTSTR ) m_SpotDataProcessing.m_TextObjectName ) ;
    if ( pAreaDescriptor )
    {
      if ( !pMarking )
        pMarking = CContainerFrame::Create();
      CalculateScales( pDataFrame , pMarking ) ;
      
      if ( !m_HLineIndexes.size() )
        RestoreIndexes() ; // first time initialization

      if ( m_iAlgorithm > NoControlAndProcessing )
      {
        LPCTSTR pTaskAsText = NULL ;
        CTextFrame * pTaskFrame = NULL ;
        m_LastTypeString = pAreaDescriptor->GetString();
        if ( m_LastTypeString[ m_LastTypeString.GetLength() - 1 ] == _T( '\n' ) )
          m_LastTypeString.Delete( m_LastTypeString.GetLength() - 1 );
        m_LastTypeString.Remove( ' ' );
       // m_LastTypeString.Replace( '0' , 'O' ) ;
        if ( m_LastTypeString.Find( _T("YW") ) > 0 )
        {
          pTaskAsText = _T( "Task(-1);" ) ; // no processing
          m_iAlgorithm = NoProcessing;
        }
        else
        {
          int iPos = (int)m_LastTypeString.ReverseFind(_T('_'));
          int iIndex = -1;
          if (iPos > 0)
          {
            FXString Replaced = m_LastTypeString.Mid(iPos + 1);
            Replaced.Replace('O', '0');
            Replaced.Replace('S', '5');
            iIndex = atoi(((LPCTSTR)Replaced));
            m_LastTypeString = m_LastTypeString.Left( iPos + 1 ) ;
            m_LastTypeString += Replaced;
          }
          if ( m_bUseIndexes )
          {
            if ( iIndex > 0 )
            {
              if ( std::find( m_HLineIndexes.begin() , m_HLineIndexes.end() , iIndex  ) != m_HLineIndexes.end() )
              {
                pTaskAsText = _T( "Task(2);" ) ; // find horizontal lines
                m_iAlgorithm = HLinesAsSpots;
              }
              else
              {
                if ( std::find( m_VLineIndexes.begin() , m_VLineIndexes.end() , iIndex ) != m_VLineIndexes.end() )
                {
                  pTaskAsText = _T( "Task(4);" ) ; // find Vertical lines
                  m_iAlgorithm = VLinesAsSpots;
                }
                else
                {
                  if ( std::find( m_DotIndexes.begin() , m_DotIndexes.end() , iIndex ) != m_DotIndexes.end() )
                  {
                    pTaskAsText = _T( "Task(1);" ) ; // find spots
                    m_iAlgorithm = SmallDots;
                  }
                  else
                  {
                    pTaskAsText = _T( "Task(-1);" ) ; // no processing
                    m_iAlgorithm = NoProcessing;
                  }
                }
              }
            }
            else
            {
              pTaskAsText = _T("Task(-1);"); // no processing, unknown index
              m_iAlgorithm = NoProcessing;
            }
          }
          else
          {
            if ( ( m_LastTypeString.Find( "DOT" ) == 0 )
              || (m_LastTypeString.Find("D0T") == 0)
              || ( m_LastTypeString.Find( "API" ) == 0 ) ) // dots should be measured
            {
              pTaskAsText = _T( "Task(1);" ) ; // find spots
              m_iAlgorithm = SmallDots;
            }
            else if ( m_LastTypeString.Find( "LINV" ) == 0 ) // vertical lines should be measured
            {
              pTaskAsText = _T( "Task(4);" ) ; // find Vertical lines
              m_iAlgorithm = VLinesAsSpots;
            }
            else if ( m_LastTypeString.Find( "LINH" ) == 0 ) // lines should be measured
            {
              pTaskAsText = _T( "Task(2);" ) ; // find horizontal lines
              m_iAlgorithm = HLinesAsSpots;
            }
            else
              m_iAlgorithm = NoProcessing;
          }
        }

        if ( !pTaskAsText )
          pTaskFrame = CreateTextFrame( _T( "Dummy;" ) , _T( "DummyForAggregator" ) , pDataFrame->GetId() );
        else
          pTaskFrame = CreateTextFrame( pTaskAsText , _T( "NewTask" ) , pDataFrame->GetId() );

        if ( pTaskFrame )
          PutFrame( GetOutputConnector( SSON_TaskSelector ) , pTaskFrame );

      }
      else
      {
        RestoreIndexes() ;
      }
      pMarking->AddFrame( pDataFrame ) ;
      PutFrame( GetOutputConnector( SSON_ScalingView ) , pMarking );

      return NULL ;
    }
    const CVideoFrame * pVF = pDataFrame->GetVideoFrame() ;
    int iWidth = GetWidth( pVF ) ;
    int iHeight = GetHeight( pVF ) ;
    switch ( m_iAlgorithm )
    {
    case NoProcessing:
      ( ( CDataFrame* ) pDataFrame )->AddRef();

      return ( CDataFrame* ) pDataFrame;
    case HLinesAsSpots:
    case VLinesAsSpots:
      {
        if ( !pMarking )
          pMarking = CContainerFrame::Create();
        try
        {
          FXString ResultAsString = StatisticsForLinesAsSpots( pDataFrame , pMarking );
          if ( ResultAsString != "" )
          {
            if ( m_iNMeasurements++ == 0 )
            {
              PutFrame( GetOutputConnector( SSON_Statistics ) ,
                CreateTextFrame( m_SpotDataProcessing.m_SpotsStatistics.GetCaption() ,
                  "Caption" , pDataFrame->GetId() ) );
              Sleep( 100 ) ;
            }
            ResultAsString.Insert( 0 , _T( ",   " ) ) ;
            ResultAsString.Insert( 1 , m_LastTypeString + ',' ) ; // not first statistics string
            PutFrame( GetOutputConnector( SSON_Statistics ) , 
              CreateTextFrame( ResultAsString , "LinesAsSpotsResult" , pDataFrame->GetId() ) );
            ResultAsString += _T( '\n' ) ;
            PutFrame( GetOutputConnector( SSON_ResultView ) ,
              CreateTextFrame( ResultAsString , "LinesAsSpotsResult" , pDataFrame->GetId() ) );

          }
        }
        catch (CException* e)
        {
          FXString Msg ;
          TCHAR ErrDescription[2000] ;
          e->GetErrorMessage( ErrDescription , 1999 ) ;
          Msg.Format( "Frame %s - Processing Error: %s" , (LPCTSTR)m_LastTypeString , ErrDescription ) ;
          cmplx cViewPt( iWidth / 4. , iHeight / 2. ) ;
          pMarking->AddFrame( CreateTextFrame( cViewPt , 
            ( LPCTSTR ) Msg , 0x0000ff , 24 , "ExceptionMsg" , pDataFrame->GetId() ) ) ;
        }

        pMarking->AddFrame( pDataFrame );
        return pMarking;
      }
      break ;
    case SmallDots:
      {
        if ( m_SpotDataProcessing.DoCalibrationProcess( pDataFrame , true ) )
        {
          FXString ResultAsString ;
          bool bRes = m_SpotDataProcessing.FormStatistics( pDataFrame , ResultAsString ) ;
          if ( bRes )
          {
            if ( m_iNMeasurements++ == 0 )
            {
              PutFrame( GetOutputConnector( SSON_Statistics ) ,
                CreateTextFrame( m_SpotDataProcessing.m_SpotsStatistics.GetCaption() ,
                  "Caption" , pDataFrame->GetId() ) );
              Sleep( 100 ) ;
            }
            int iCRPos = ( int ) ResultAsString.Find( _T( '\n' ) ) ;
            if ( iCRPos < 0 )
            {
              ResultAsString.Insert( 0 , _T( ',' ) ) ;
              ResultAsString.Insert( 1 , m_LastTypeString + ',' ) ; // not first statistics string
            }
            else
            {
              ResultAsString.Insert( iCRPos + 1 , _T( ',' ) ) ;
              ResultAsString.Insert( iCRPos + 2 , m_LastTypeString + ',' ) ; // first statistics string
            }

            PutFrame( GetOutputConnector( SSON_Statistics ) , CreateTextFrame(
              ResultAsString , "SpotResult" , pDataFrame->GetId() ) ) ;
            ResultAsString += _T( '\n' ) ;
            PutFrame( GetOutputConnector( SSON_ResultView ) ,
              CreateTextFrame( ResultAsString , "LinesAsSpotsResult" , pDataFrame->GetId() ) );
          }
        }
        if ( !pMarking )
          pMarking = CContainerFrame::Create();

        pMarking->AddFrame( pDataFrame ) ;
        return pMarking ;
      }
      break ;
    }
  }
  return NULL ;
}

void SpotStatistics::AsyncTransaction( CDuplexConnector* pConnector , CDataFrame* pParamFrame )
{

};

void SpotStatistics::ConfigParamChange( LPCTSTR pName , void* pObject , bool& bInvalidate , bool& bInitRescan )
{
  SpotStatistics * pGadget = ( SpotStatistics* ) pObject;
  if ( pGadget )
  {
    bInvalidate = true;
  }
}


void SpotStatistics::PropertiesRegistration()
{
  addProperty( SProperty::COMBO , _T( "Algorithm" ) ,
    &m_iAlgorithm , SProperty::Long , _T( "NoControlAndProcessing;NoProcessing;HLines;VLines;SmallDots;" ) ) ;
  SetChangeNotification( _T( "Algorithm" ) , ConfigParamChange , this );
  addProperty( SProperty::EDITBOX , _T( "Scale_um/pix" ) ,
    &m_SpotDataProcessing.m_dScale_um_per_pixel , SProperty::Double ) ;
  addProperty( SProperty::EDITBOX , _T( "DistBetweenSquares_um" ) ,
    &m_dDistBetweenSquares_um , SProperty::Double ) ;
  addProperty( SProperty::EDITBOX , _T( "TextObjectID" ) ,
    &m_SpotDataProcessing.m_TextObjectName , SProperty::String ) ;
  addProperty( SProperty::EDITBOX , _T( "SpotName" ) ,
    &m_SpotDataProcessing.m_TextObjectName , SProperty::String ) ;
  addProperty( SProperty::EDITBOX , _T( "LineName" ) ,
    &m_LineAreaIDPrefix , SProperty::String ) ;

  switch ( m_iAlgorithm )
  {
  case SmallDots:
    {
      addProperty( SProperty::EDITBOX , _T( "CalibObjectName" ) ,
        &m_SpotDataProcessing.m_CalibDataName , SProperty::String ) ;
      addProperty( SProperty::SPIN , _T( "#OmittedObjects" ) ,
        &m_SpotDataProcessing.m_CalibGrid.m_iInsertMissed , SProperty::Int , 0 , 5 ) ;
    }
  }
};

void SpotStatistics::ConnectorsRegistration()
{
  addInputConnector( transparent , "ObjectsDataInput" );
  addOutputConnector( transparent , "ImagesOut" );
  addOutputConnector( transparent , "StatisticsOut" ); // for writing to file
  addOutputConnector( transparent , "TaskSelector" );
  addOutputConnector( transparent , "ScalingView" );
  addOutputConnector( transparent , "ResultView" ); // for viewing on screen
};

FXString&	SpotStatistics::StatisticsForLinesAsSpots(
  const CDataFrame * pSrc , CContainerFrame * pMarking )
{

FXRegistry Reg("TheFileX\\SHStudio");
  int iStatisticsOnlyByProfile = Reg.GetRegiInt(
    "SpotStatistics", "Statistics Only By Profile", 1);
  m_Result.Empty() ;

  m_SeparatedResults.clear();
  m_Profiles.clear();
  m_ROIs.clear() ;
  m_LinesAsSpotsProcessing.Reset() ;

  int iNSpots = ExtractDataAboutSpots( pSrc , m_SeparatedResults , m_Profiles , &m_ROIs );

  if ( iNSpots )
  {
    if ( m_iAlgorithm == VLinesAsSpots )  // exchange X and Y in profiles and spots info
    {
      for ( auto ZoneIter = m_SeparatedResults.begin() ; ZoneIter != m_SeparatedResults.end() ; ZoneIter++ )
      {
        for ( auto It = ZoneIter->begin() ; It != ZoneIter->end() ; It++ )
        {
          swap( It->m_SimpleCenter.x , It->m_SimpleCenter.y );
          swap( It->m_dBlobWidth , It->m_dBlobHeigth ) ;
        }
      }
      for ( auto ProfileIter = m_Profiles.begin() ; ProfileIter != m_Profiles.end() ; ProfileIter++ )
      {
        for ( auto PointIter = ProfileIter->m_Data.begin() ; PointIter != ProfileIter->m_Data.end() ; PointIter++ )
        {
          swap( PointIter->_Val[_RE] , PointIter->_Val[ _IM ] );
        }
      }
      for ( auto ROIIter = m_ROIs.begin() ; ROIIter != m_ROIs.end() ; ROIIter++ )
      {
        swap( ROIIter->m_Rect.left , ROIIter->m_Rect.top ) ;
        swap( ROIIter->m_Rect.right , ROIIter->m_Rect.bottom ) ;
      }
      m_LinesAsSpotsProcessing.SetVertical(true);
    }
    else
      m_LinesAsSpotsProcessing.SetVertical(false);

     // m_Zones[0] holds rectangle for anchor rect
    for ( size_t i = 0 ; i < m_ROIs.size() ; i++ )
      m_LinesAsSpotsProcessing.m_Zones.push_back( m_ROIs[ i ].m_Rect ) ;

    // Main processing

    double dTimeBefore = GetHRTickCount();
    pMarking->CopyAttributes( pSrc ) ;

    m_LinesAsSpotsProcessing.FindProfileCenters( 
      m_ROIs , m_Profiles , pMarking );

    if (iStatisticsOnlyByProfile == 1)
    {
     // m_LinesAsSpotsProcessing.WriteToFileProfileRawData(m_Profiles);
      m_Result = m_LinesAsSpotsProcessing.GetStatResultsForLines() ;
    }
    else if (iStatisticsOnlyByProfile == 0)
    {
      if ( m_LinesAsSpotsProcessing.m_ProfileCenters.size())
      {
        m_LinesAsSpotsProcessing.m_WithMissingSpots.clear();
        m_LinesAsSpotsProcessing.FindMissingSpotsByProfile(
          m_LinesAsSpotsProcessing.m_WithMissingSpots , m_SeparatedResults, pMarking);
        m_Result = m_LinesAsSpotsProcessing.RegressionForLinesAsSpots(
          m_LinesAsSpotsProcessing.m_WithMissingSpots );
      }
    }
    //pMarking->AddFrame(pSrc);
    //PutFrame(m_pOutput, pMarking);

    double dTimeAfter = GetHRTickCount();
    double dDeltaTime = dTimeBefore - dTimeAfter;
  }
  return m_Result ;
}

bool SpotStatistics::CalculateScales( const CDataFrame * pDataFrame , CContainerFrame * pMarking )
{
  const CFigureFrame * pLeftSquareCent = pDataFrame->GetFigureFrame( "square_0_cent" ) ;
  const CFigureFrame * pRightSquareCent = pDataFrame->GetFigureFrame( "right_square_0_cent" ) ;
  if ( pLeftSquareCent && pRightSquareCent 
    && pLeftSquareCent->GetCount() && pRightSquareCent->GetCount() )
  {
    cmplx cLeftSqCenter = CDPointToCmplx(pLeftSquareCent->GetAt(0)) ;
    cmplx cRightSqCenter = CDPointToCmplx( pRightSquareCent->GetAt( 0 ) ) ;
    cmplx cDist = cRightSqCenter - cLeftSqCenter ;
    double dDist_pix = abs( cRightSqCenter - cLeftSqCenter ) ;
    m_LinesAsSpotsProcessing.m_dScale_unit_per_pixel = 
      m_SpotDataProcessing.m_dScale_um_per_pixel = m_dDistBetweenSquares_um / dDist_pix ;
    cmplx cCentBetweenSquares = 0.5 * ( cRightSqCenter + cLeftSqCenter ) ;
    m_cLastScaleViewPoint = cmplx( cCentBetweenSquares + cmplx( -1000. , 500 ) ) ;
    CTextFrame * pScaleInfo = CreateTextFrame( m_cLastScaleViewPoint , "0x00c000" , 20 , "ScaleingResult" ,
      pDataFrame->GetId() , "Scale=%.4f um/pix Vect(%.2f,%.2f)" ,
      m_SpotDataProcessing.m_dScale_um_per_pixel , cDist ) ;
    pScaleInfo->Attributes()->WriteInt("back" , 0 ) ;
    pMarking->AddFrame( pScaleInfo ) ; 
    return true ;
  }
  else
  {
    pMarking->AddFrame( CreateTextFrame( m_cLastScaleViewPoint , "0x0000ff" , 20 , "ScaleingResult" ,
      pDataFrame->GetId() , "Scaling FAILED"  ) ) ;
  }
  return false;
}

int GetIndexesFromRegistry( FXRegistry& Reg , IntVector& Dest ,
  LPCTSTR pTreeLeafName , LPCTSTR pIndexesName , LPCTSTR pDefault )
{
  int TmpArray[ 5000 ] ;

  Dest.clear() ;
  FXString Existent = Reg.GetRegiString( pTreeLeafName , pIndexesName , pDefault ) ;
  int iNIndexes = Reg.GetRegiIntSerie( pTreeLeafName , pIndexesName , TmpArray , 5000 ) ;
  for ( int i = 0 ; i < iNIndexes ; i++ )
    Dest.push_back( TmpArray[ i ] ) ;
  return iNIndexes ;
}

bool SpotStatistics::RestoreIndexes()
{
  FXRegistry Reg( "TheFileX\\HP_IndigoPQAnalysis" );
  m_bUseIndexes = Reg.GetRegiInt( "AlgorithmSelection" , "UseIndexes" , TRUE ) != 0 ;

  if ( m_bUseIndexes )
  {
    GetIndexesFromRegistry(Reg, m_HLineIndexes,
      "AlgorithmSelection", "HLineIndexes",
      "1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16");

    GetIndexesFromRegistry(Reg, m_VLineIndexes,
      "AlgorithmSelection", "VLineIndexes",
      "17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32");

    GetIndexesFromRegistry(Reg, m_DotIndexes,
      "AlgorithmSelection", "DotIndexes",
      "33,34,35,36,38,43,44,45");

    GetIndexesFromRegistry(Reg, m_OtherIndexes,
      "AlgorithmSelection", "OtherIndexes",
      "70,72");
  }
  return m_bUseIndexes ;
}
