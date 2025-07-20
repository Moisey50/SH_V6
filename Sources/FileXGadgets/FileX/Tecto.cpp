#include "stdafx.h"
#include "Tecto.h"
#include "fxfc/FXRegistry.h"
#include "imageproc/statistics.h"
#include "Math/PlaneGeometry.h"
#include <psapi.h>
#include <process.h>
#include <algorithm>
#include <iterator>

// #define __CRTDBG_MAP_ALLOC
// #include <crtdbg.h>
// #ifdef DEBUG_NEW
// #undef DEBUG_NEW
// #endif
// #define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
// #define new DEBUG_NEW

USER_FILTER_RUNTIME_GADGET( Tecto , "Video.recognition" );

using namespace nsTecto;

Tecto * g_LogGadgetPtr = NULL ;
void Tecto::GlobalLogger( LPCTSTR pLogString )
{
  if ( g_LogGadgetPtr )
    g_LogGadgetPtr->Logger( pLogString ) ;
}

LPCTSTR Tecto::GetTectoGadgetModeName()
{
  switch ( m_GadgetMode )
  {
    case TGM_SIDE: return "SideView" ;
    case TGM_FRONT: return "FrontView" ;
    default: ASSERT( 0 ) ;
  }
  return NULL ;
}
void Tecto::ConfigParamChange( LPCTSTR pName , void* pObject ,
  bool& bInvalidate , bool& bInitRescan )
{
  Tecto * pGadget = ( Tecto* ) pObject;
  if ( pGadget )
  {
    if ( _tcscmp( pName , "FlowerName" ) == 0 )
      pGadget->RestoreFlower() ;
    else if ( pGadget->m_uiNOriginalFrames && (_tcscmp( pName , "SaveToReg" ) == 0) )
    {
      pGadget->SaveToRegistry() ;
      pGadget->m_bSaveToRegistry = FALSE ;
      bInvalidate = true ;
    }
  }
}

Tecto::Tecto()
{
  m_OutputMode = modeReplace ;
  m_GadgetMode = TGM_SIDE ;
  m_ProcessingMode = TPM_SIDE_Processing ;
  m_dScale_mm_per_pix = 1. ;
  m_iViewMode = TIVM_Original ;

  m_dCalibLength_mm = 680. ;

  m_dMinimalLength_mm = 400. ;
  m_dMaximalLength_mm = 1100. ;
  m_dMinimalWidth_mm = 200. ;
  m_dMaximalWidth_mm = 600. ;
  m_dStalkDiaMin_mm = 1. ;
  m_dStalkDiaMax_mm = 7. ;
  m_bLengthMeasurement = TRUE ;
  m_bXRightOut = TRUE ;
  m_bWidthMeasurement = TRUE ;
  m_bAreaMeasurement = TRUE ;
  m_bMeasureStalk = TRUE ;
  m_bMeasureCentralStem = TRUE ;
  m_dStalkAndStemCorrection_pix = 1.5 ;
  m_dStemWidthStart_perc = 45. ;
  m_dStemWidthEnd_perc = 55. ;
  m_dStemDiaMin_mm = 3. ;
  m_dStemDiaMax_mm = 3. ;
  m_dMinimalFlowerArea_cm2 = 300. ;
  m_dMaximalFlowerArea_cm2 = 1000. ; 
  m_cRectOnLineSize_mm = cmplx( 39.0 , 8.0 ) ;
  m_cRectOnLineTol_mm = cmplx( 3.0 , 3.0 ) ;

  m_LastROI = CRect( 0 , 0 , 640 , 480 ) ;
  m_pLastOriginalVideoFrame = NULL ;
  m_dLastProcessingTime_ms = 0. ;

  g_LogGadgetPtr = this ;

  init() ;
}


Tecto::~Tecto()
{
  if ( m_pLastOriginalVideoFrame )
  {
    ( ( CVideoFrame* ) m_pLastOriginalVideoFrame )->Release() ;
    m_pLastOriginalVideoFrame = NULL ;
  }
}

static const char * pViewMode = "Unknown;Free;Lock;Manual;" ;
static const char * pGadgetMode = "Side;Front;" ;

void Tecto::PropertiesRegistration()
{
  //   addProperty( SProperty::SPIN , _T( "MinLength_pix" ) ,
  //     &m_iMinStraightSegment , SProperty::Long , 3 , 3000 ) ;
  //   addProperty( SProperty::EDITBOX , _T( "Tolerance_mrad" ) ,
  //     &m_dTolerance , SProperty::Double );
  //   addProperty( SProperty::EDITBOX , _T( "DiffFromLine_pix" ) ,
  //     &m_dDiffFromLine_pix , SProperty::Double );
  //   addProperty( SProperty::SPIN , _T( "NMaxDeviated" ) ,
  //     &m_iNMaxDeviated , SProperty::Long , 1 , 1000 ) ;
  // 

  addProperty( SProperty::COMBO , _T( "GadgetMode" ) , ( int * ) &m_GadgetMode ,
    SProperty::Int , pGadgetMode ) ;
  SetInvalidateOnChange( _T( "GadgetMode" ) , true ) ;
  addProperty( SProperty::EDITBOX , "FlowerName" , &m_FlowerName , SProperty::String ) ;
  SetChangeNotificationForLast( Tecto::ConfigParamChange , this ) ;
  addProperty( SProperty::EDITBOX , "Scale_mm/pix" , &m_dScale_mm_per_pix , SProperty::Double ) ;
  addProperty( SProperty::COMBO , _T( "ViewMode" ) , ( int * ) &m_iViewMode ,
    SProperty::Int , _T( "Original;Processed;Binaryzed;" ) ) ;


  switch ( m_GadgetMode )
  {
    case TGM_SIDE:
      // Measurement flags
      addProperty( SProperty::CHECK_BOX , "Meas_Length" , &m_bLengthMeasurement , SProperty::Int ) ;
      addProperty( SProperty::CHECK_BOX , "XRight_out" , &m_bXRightOut , SProperty::Int ) ;
      addProperty( SProperty::CHECK_BOX , "Meas_Width" , &m_bWidthMeasurement , SProperty::Int ) ;
      addProperty( SProperty::CHECK_BOX , "Meas_Area" , &m_bAreaMeasurement , SProperty::Int ) ;
      addProperty( SProperty::CHECK_BOX , "Meas_Stalk" , &m_bMeasureStalk , SProperty::Int ) ;
      addProperty( SProperty::CHECK_BOX , "Meas_Stem" , &m_bMeasureCentralStem , SProperty::Int ) ;
      SetInvalidateOnChange( _T( "Meas_Stem" ) , true ) ;

      addProperty( SProperty::EDITBOX , "MinimalLength_mm" ,
        &m_dMinimalLength_mm , SProperty::Double ) ;
      addProperty( SProperty::EDITBOX , "MaximalLength_mm" ,
        &m_dMaximalLength_mm , SProperty::Double ) ;
      addProperty( SProperty::EDITBOX , "MinimalWidth_mm" ,
        &m_dMinimalWidth_mm , SProperty::Double ) ;
      addProperty( SProperty::EDITBOX , "MaximalWidth_mm" ,
        &m_dMaximalWidth_mm , SProperty::Double ) ;
      addProperty( SProperty::EDITBOX , "StalkDiaMin_mm" ,
        &m_dStalkDiaMin_mm , SProperty::Double ) ;
      addProperty( SProperty::EDITBOX , "StalkDiaMax_mm" ,
        &m_dStalkDiaMax_mm , SProperty::Double ) ;
      addProperty( SProperty::EDITBOX , "StalkShiftRight_mm" ,
        &m_dStalkShiftRight_mm , SProperty::Double ) ;

      addProperty( SProperty::EDITBOX , "MinimalFlowerArea_cm2" ,
        &m_dMinimalFlowerArea_cm2 , SProperty::Double ) ;
      addProperty( SProperty::EDITBOX , "MaximalFlowerArea_cm2" ,
        &m_dMaximalFlowerArea_cm2 , SProperty::Double ) ;

      addProperty( SProperty::EDITBOX , "StemWidthStart_perc" ,
        &m_dStemWidthStart_perc , SProperty::Double ) ;
      addProperty( SProperty::EDITBOX , "StemWidthEnd_perc" ,
        &m_dStemWidthEnd_perc , SProperty::Double ) ;
      addProperty( SProperty::EDITBOX , "StemDiaMin_mm" ,
        &m_dStemDiaMin_mm , SProperty::Double ) ;
      addProperty( SProperty::EDITBOX , "StemDiaMax_mm" ,
        &m_dStemDiaMax_mm , SProperty::Double ) ;
      addProperty( SProperty::EDITBOX , "CalibDistance_mm" ,
        &m_dCalibLength_mm , SProperty::Double ) ;
      addProperty( SProperty::EDITBOX , "OutBinaryThres_norm" ,
        &m_dOutBinaryThres , SProperty::Double ) ;
      break ;
    case TGM_FRONT:
      addProperty( SProperty::EDITBOX , "MinimalFlowerArea_cm2" ,
        &m_dMinimalFlowerArea_cm2 , SProperty::Double ) ;
      addProperty( SProperty::EDITBOX , "MaximalFlowerArea_cm2" ,
        &m_dMaximalFlowerArea_cm2 , SProperty::Double ) ;
      break ;
    default:
      ASSERT( 0 ) ;
      break ;
  }

  addProperty( SProperty::CHECK_BOX , "SaveToReg" , &m_bSaveToRegistry , SProperty::Int ) ;
  SetChangeNotificationForLast( Tecto::ConfigParamChange , this ) ;
  addProperty( SProperty::SPIN , "DebugViewMode" , &m_iDebugView , SProperty::Int , 1 , 50 ) ;
};

void Tecto::ConnectorsRegistration()
{
  addInputConnector( transparent , "DataInput" );
  addOutputConnector( transparent , "OutputView" );
  addOutputConnector( text , "Diagnostics" ) ;

  GetInputConnector( 0 )->SetQueueSize( 12 ) ;

};

bool Tecto::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  FXPropertyKit pk( text ) ;
  pk.GetDouble( "ThickCorrect_pix" , m_dStalkAndStemCorrection_pix ) ;
  return UserGadgetBase::ScanProperties( text , Invalidate ) ;
}

CDataFrame* Tecto::DoProcessing( const CDataFrame* pDataFrame )
{
//   DWORD PID = GetCurrentProcessId() ;
//   PROCESS_MEMORY_COUNTERS Counters ;
//   HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
//     PROCESS_VM_READ ,
//     FALSE , PID );
// 
//   if ( GetProcessMemoryInfo( hProcess , &Counters , sizeof( Counters ) ) )
//     m_dwMemOnEntry = Counters.WorkingSetSize ;
//   else
//     m_dwMemOnEntry = 0 ;
//   CloseHandle( hProcess ) ;
  m_dLastStartTime_ms = GetHRTickCount() ;

  if ( !pDataFrame->IsContainer() )
  {
    switch ( pDataFrame->GetDataType() )
    {
      case vframe:
      {
        if ( m_pLastOriginalVideoFrame )
        {
          ( ( CVideoFrame* ) m_pLastOriginalVideoFrame )->Release() ;
          m_pLastOriginalVideoFrame = NULL ;
        }
        if ( !m_bProcessDisable )
        {
          m_pLastOriginalVideoFrame = ( CVideoFrame* ) pDataFrame ;
          ( ( CVideoFrame* ) m_pLastOriginalVideoFrame )->AddRef() ;
        }
        m_uiNOriginalFrames++ ;
      }
      case text:
      {
        if ( _tcsicmp( pDataFrame->GetLabel() , "mode" ) == 0 )
        {
          if ( _tcsicmp( ( LPCTSTR ) ( pDataFrame->GetTextFrame()->GetString() ) , "Task(1);" ) == 0 )
            m_ProcessingMode = TPM_SIDE_Processing ;
          if ( _tcsicmp( ( LPCTSTR ) ( pDataFrame->GetTextFrame()->GetString() ) , "Task(2);" ) == 0 )
            m_ProcessingMode = TPM_SIDE_Calibration ;
        }
        else if ( _tcsicmp( pDataFrame->GetLabel() , "process" ) == 0 )
        {
          FXString Content = pDataFrame->GetTextFrame()->GetString() ;
          Content.MakeLower() ;
          m_bProcessDisable = ( Content == "enable" ) ? false : true ;
        }
      }
    }
    return NULL ;
  }

  if ( m_bProcessDisable )
    return NULL ;

  CContainerFrame * pOut = CContainerFrame::Create() ;
  CopyIdAndTime( pOut , pDataFrame ) ;

  m_dLastLength_mm = m_dLastWidth_mm = m_dLastWidth_mm
    = m_dLastFlowerArea_cm2 = m_dAltFlowerArea_cm2 = 0. ;

  FXDblArray Times ;
  //#ifdef _DEBUG
  FXString DiagInfo ;
  //#endif


  const CVideoFrame * pImage = pDataFrame->GetVideoFrame() ;
  if ( pImage )
  {

    m_cLastFOV_pix._Val[ _RE ] = GetWidth( pImage ) ;
    m_cLastFOV_pix._Val[ _IM ] = GetHeight( pImage ) ;

    m_cLastFOVCenter_pix = m_cLastFOV_pix / 2. ;

    if ( m_ProcessingMode == TPM_SIDE_Calibration )
    {
      m_LastROIs.clear() ;
      ExtractDataAboutROIs( pDataFrame , m_LastROIs ) ;
      m_LastSpots.clear() ;
      if ( ExtractDataAboutSpots( pDataFrame , m_LastSpots ) )
        DoCalibration( pOut ) ;
      pOut->AddFrame( pDataFrame ) ;
      return pOut ;
    }

    switch ( m_iViewMode )
    {
      case TIVM_Processed:pOut->AddFrame( pImage ) ; break ;
      case TIVM_Original:
      {
//         if ( m_pLastOriginalVideoFrame )
//                 pOut->AddFrame( m_pLastOriginalVideoFrame ) ;
      }
      break ;
    }
  }

  double dMaxY = -DBL_MAX ;
  m_bTrayIsEmpty = false ;
  m_AdditionalInfo.Empty() ;


  const CRectFrame * pFlowerROI = pDataFrame->GetRectFrame( "ROI:flower" ) ;
  if ( !pFlowerROI )
  {
    m_bTrayIsEmpty = true ;
    if ( m_pLastOriginalVideoFrame )
      pOut->AddFrame( m_pLastOriginalVideoFrame->Copy() ) ;
    m_LastROI.SetRectEmpty() ;
  }
  else
  {
    pOut->AddFrame( pFlowerROI ) ;
    m_LastROI = ( CRect ) ( *pFlowerROI ) ;
    if ( m_pLastOriginalVideoFrame )
    {
      m_iFoundCutLevel = GetCutLevelByHisto8(
        m_pLastOriginalVideoFrame , m_dHistoCutLevelForEmptyTray , m_LastHistoGram , &m_LastROI ) ;
      ProcessHistogram() ;
      m_AdditionalInfo.Format( "Cut=%d %%=%4.1f Mh=%d(%d)" ,
        m_iFoundCutLevel , m_dHistoCutLevelForEmptyTray * 100. , m_iHistoMaxIndex , m_iMaxThreshold ) ;
    }
    else
      m_iFoundCutLevel = 0 ;
  }

  CRect EdgeROI( pFlowerROI ? ( *( RECT* ) ( pFlowerROI ) ) : CRect( 100 , 500 , 1100 , 1800 ) ) ;
  EdgeROI.DeflateRect( 5 , 5 ) ;
  FXSIZE iIndexOfMaxY = -1 ;
  std::vector< std::vector< nsTecto::R2 > > figures;
  cmplx cMaxSizes ;
  vector<Extremes_s> AllExtremes ;
  vector<ExtrIndexes_s> AllIndexes ;
  vector<const CFigureFrame *> AllFigures ;

  CFramesIterator* Iterator = pDataFrame->CreateFramesIterator( figure );
  if ( Iterator != NULL )
  {
    const CFigureFrame * pFigure = NULL ;
    cmplx cRectOnLineMin_pix = ( m_cRectOnLineSize_mm - m_cRectOnLineTol_mm )
      / m_dScale_mm_per_pix ;
    cmplx cRectOnLineMax_pix = ( m_cRectOnLineSize_mm + m_cRectOnLineTol_mm )
      / m_dScale_mm_per_pix ;
    while ( pFigure = ( const CFigureFrame * ) Iterator->Next() )
    {
      FXString Label = pFigure->GetLabel() ;
      if ( Label.Find( "Contur[flower" ) == 0 )
      {
        int iContNumPos = ( int ) Label.Find( '_' ) ;
        if ( iContNumPos > 0 )
        {
          Extremes_s Extrems ;
          ExtrIndexes_s Indexes ;
          cmplx cSize ;
          FindExtrems( ( const cmplx* ) ( pFigure->GetData() ) ,
            pFigure->Size() ,
            Extrems , &Indexes , &cSize ) ;
          cmplx cSizeDiff = cSize - cMaxSizes ;
          if ( cSizeDiff.real() > 0 )
            cMaxSizes._Val[ _RE ] = cSize.real() ;
          if ( cSizeDiff.imag() > 0 )
            cMaxSizes._Val[ _IM ] = cSize.imag() ;

          if ( !IsInRange( cSize , cRectOnLineMin_pix , cRectOnLineMax_pix ) )
          {
            int iContNum = atoi( ( LPCTSTR ) Label + iContNumPos + 1 ) ;
            std::vector< nsTecto::R2 > NewContur ;
            figures.push_back( NewContur ) ;

            figures.back().resize( pFigure->GetCount() ) ;
            memcpy( figures.back().data() ,
              pFigure->GetData() , sizeof( cmplx ) * pFigure->GetCount() ) ;

            AllExtremes.push_back( Extrems ) ;
            AllIndexes.push_back( Indexes ) ;
            AllFigures.push_back( pFigure ) ;
          }
        }
      }
    }
    delete Iterator ;
  }
  else
  {
    Logger( "No Iterator  " ) ;
    m_bTrayIsEmpty = true ;
    if ( m_pLastOriginalVideoFrame )
      pOut->AddFrame( m_pLastOriginalVideoFrame->Copy() ) ;
  }

  if ( figures.empty() )
  {
    m_bTrayIsEmpty = true ;
    if ( m_pLastOriginalVideoFrame )
      pOut->AddFrame( m_pLastOriginalVideoFrame->Copy() ) ;
  }
  else
  {
    cMaxSizes *= m_dScale_mm_per_pix ;
    bool bIsLongContur = ( cMaxSizes.real() > ( m_dMinimalLength_mm  * 0.9 ) ) ;
    bool bSmallFlowerDetected = ( ( m_dMinimalLength_mm > 0. )
      && !m_bAreaMeasurement && bIsLongContur ) ;

    if ( m_pLastOriginalVideoFrame )
    {
      if ( !bIsLongContur )
      {
  //       m_iFoundCutLevel = GetCutLevelByHisto8(
  //         m_pLastOriginalVideoFrame , m_dHistoCutLevelForEmptyTray , m_LastHistoGram , &m_LastROI ) ;
        if ( ( !bSmallFlowerDetected && ( m_iFoundCutLevel < 50 ) ) || ( m_iHistoMaxIndex < m_iMaxThreshold ) )
        {
          m_bTrayIsEmpty = true ;
          pOut->AddFrame( m_pLastOriginalVideoFrame->Copy() ) ;
        }
      }
      else if ( bSmallFlowerDetected  )
      {
        if ( m_iHistoMaxIndex < m_iMaxThreshold )
        {
          m_bTrayIsEmpty = true ;
          pOut->AddFrame( m_pLastOriginalVideoFrame->Copy() ) ;
        }
      }
      else if ( /*(m_iHistoMaxIndex < m_iMaxThreshold) || */( m_iFoundCutLevel < 50 ) )
      {
        m_bTrayIsEmpty = true ;
        pOut->AddFrame( m_pLastOriginalVideoFrame->Copy() ) ;
      }
    }
  }

  m_dMinimalLength_pix = m_dMinimalLength_mm / m_dScale_mm_per_pix ;
  if ( !m_bTrayIsEmpty )
  {
    if ( figures.size() )
    {
      switch ( m_GadgetMode )
      {
        case TGM_SIDE:
        {
          m_dLastStalkThickness_mm = 0. ;
          m_dLastStemDia_mm = 0. ;
          ProcessPlantImage ConturProcessing ;

          ProcessPlantImageParameters p ;
          memset( &p , 0 , sizeof( p ) ) ;

          p.MashingSize        = (int)m_dMachingSize  ;
          p.XAxeCompressCoeff = m_dXAxeCompressCoeff ;
          p.RelationDistance = m_dRelationDistance ;
          p.SlicedPartForStalk = m_dSlicedPartForStalk ; 

          ConturProcessing.SetParameters( p );

          ProcessPlantImage::eErrorCode ProcError =
            ConturProcessing.process( figures /*, GlobalLogger*/ ) ;

          /*const*/ ProcessPlantImageResults& results = ( ProcessPlantImageResults&)ConturProcessing.Results();

//             FXString Msg ;
//             Msg.Format( "EProcess=%d, NConturs=%d" , ( int ) ProcError , results.includedConturs.size() ) ;
//            Logger( Msg ) ;
          const std::vector<nsTecto::R2>& refContour = figures[ results.ReferenceConturIndex ];
          nsTecto::Rectangle boundary = results.boundary ;
          cmplx BoundLT( boundary.s.x , boundary.s.y ) , BoundRD( boundary.e.x , boundary.e.y ) ;
          CmplxRect crBound( BoundLT , BoundRD ) ;
          Extremes_s MainExtremes = AllExtremes[ results.ReferenceConturIndex ] ;
          double dMainLeft = MainExtremes.m_cLeft.real() ;
          double dMainLength = MainExtremes.GetSize().real() ;
          double dLengthThreshold = dMainLength * 0.07 ;
          double dCurrentLeft = dMainLeft , dTmpLeft ;
          size_t iLeftConturIndex = results.ReferenceConturIndex ;
          int iMinX = INT_MAX ;
          bool bStalkIsMeasured = !m_bMeasureStalk ;
          bool bStemIsMeasured = !m_bMeasureCentralStem ;

          CVideoFrame * pVResult = m_pLastOriginalVideoFrame ?
            ( CVideoFrame* ) m_pLastOriginalVideoFrame->Copy() : NULL ;

          cmplx cSelectedMaxSize ;
          cmplx cRefSize = MainExtremes.GetSize() ;
          for ( int i = 0; i < ( int ) results.includedConturs.size(); ++i )
          {
            int iConturIndex = results.includedConturs[ i ] ;
            cmplx cSize = AllExtremes[ iConturIndex ].GetSize() ;
            cmplx cDiffToMain = cRefSize - cSize ;
            if ( (iConturIndex != results.ReferenceConturIndex ) && (abs( cDiffToMain ) < 2.) )
            {
              if ( abs(AllExtremes[ iConturIndex ].m_cLeft - MainExtremes.m_cLeft ) < 1.5 )
              {
                results.includedConturs.erase( results.includedConturs.begin() + i ) ;
                i-- ;
              }
            }
            if ( cSize.real() > cSelectedMaxSize.real() )
              cSelectedMaxSize._Val[ _RE ] = cSize.real() ;
            if ( cSize.imag() > cSelectedMaxSize.imag() )
              cSelectedMaxSize._Val[ _IM ] = cSize.imag() ;
          }
          
          double dMaxDia_pix = ( m_bMeasureStalk ) ? m_dStalkDiaMax_mm / m_dScale_mm_per_pix : 0. ;
          if ( m_bMeasureCentralStem && ( dMaxDia_pix < m_dStemDiaMax_mm / m_dScale_mm_per_pix ) )
            dMaxDia_pix = m_dStemDiaMax_mm / m_dScale_mm_per_pix ;

          double dMaxArea_pix = crBound.GetWidth() * dMaxDia_pix * 3 ;
          bool bBadMainConturArea = false ;
//           if ( m_bMeasureStalk || m_bMeasureCentralStem )
//           {
//             double dMainArea_pix = GetFigureArea( AllFigures[ results.ReferenceConturIndex ] ) ;
//             bBadMainConturArea = ( dMainArea_pix > dMaxArea_pix ) ;
//           }

          if ( (( crBound.GetWidth() * m_dScale_mm_per_pix ) < m_dMinimalLength_mm * 0.9)
            || ((cSelectedMaxSize.real() * m_dScale_mm_per_pix) < m_dMinimalLength_mm * 0.9) 
            || bBadMainConturArea )
          {
            m_bTrayIsEmpty = true ;
            if ( pVResult )
              pOut->AddFrame( pVResult ) ;
          }
          else
          {
            if ( dMainLeft - BoundLT.real() > 0.1 )
            {
              for ( size_t i = 0 ; i < results.includedConturs.size() ; i++ )
//               for ( size_t i = 0 ; i < AllExtremes.size() ; i++ )
              {
                int iIncluded = results.includedConturs[ i ] ;
                if ( iIncluded != results.ReferenceConturIndex )
                {
                  Extremes_s& Extr = AllExtremes[ iIncluded ] ;
                  dTmpLeft = Extr.m_cLeft.real() ;
                  double dLength = Extr.GetSize().real() ;
                  if ( ( dTmpLeft < dCurrentLeft ) 
                    && ( dLength > m_dMinimalLength_pix * 0.6 ) )
                  {
                    dCurrentLeft = dTmpLeft ;
                    iLeftConturIndex = iIncluded ;
                  }
                }
              }
            }
            for ( int i = 0; i < ( int ) results.includedConturs.size(); ++i )
            {
              int iConturIndex = results.includedConturs[ i ] ;

              CFigureFrame * pContur = CreateFigureFrame(
                ( cmplx* ) figures[ iConturIndex ].data() ,
                ( int ) figures[ iConturIndex ].size() , ( DWORD ) 0x0000ff ) ;
              if ( m_bMeasureStalk && ( iConturIndex == iLeftConturIndex ) && pVResult )
              {
                m_dLastStalkThickness_mm = m_dScale_mm_per_pix *
                  GetStalkDia( crBound , pContur , pVResult , pOut , iMinX )  ;
                if ( boundary.s.x != crBound.m_cLT.real() )
                  boundary.s.x = crBound.m_cLT.real() ;
                bStalkIsMeasured = true ;
              }
              if ( m_bMeasureCentralStem && ( iConturIndex == results.ReferenceConturIndex ) && pVResult )
              {
                m_dLastStemDia_mm = GetStemDia( crBound , pContur ,
                  pVResult , pOut ) ;
                if ( m_dLastStemDia_mm == -1. )
                {
                  m_bTrayIsEmpty = true ;
                  pOut->AddFrame( pVResult ) ;
                  break ;
                }
                bStemIsMeasured = true ;
              }
              double dArea = GetFigureArea( pContur ) ;
              if ( m_iViewMode != TIVM_Binaryzed )
                m_dLastFlowerArea_cm2 += dArea ;
              else
                m_dAltFlowerArea_cm2 += dArea ;
              pOut->AddFrame( pContur ) ;
            }
          }
          
          if ( !m_bTrayIsEmpty && (!bStalkIsMeasured || !bStemIsMeasured) )
          {
            m_bTrayIsEmpty = true ;
            pOut->AddFrame( pVResult ) ;
          }
          if ( !m_bTrayIsEmpty )
          {
            if ( iMinX < boundary.s.x )
              boundary.s.x = iMinX ;

            int iXbeg = ROUND( boundary.s.x ) ;
            int iXend = ROUND( boundary.e.x ) ;
            int iYbeg = ROUND( boundary.s.y ) ;
            int iYend = ROUND( boundary.e.y ) ;

            if ( m_pLastOriginalVideoFrame )
            {
              int iYStep = GetWidth( m_pLastOriginalVideoFrame ) ;

              BYTE Thres = m_iHistoMinIndex + ROUND( ( m_iHistoMaxIndex - m_iHistoMinIndex ) * m_dOutBinaryThres ) ;

              // Calculate # of "white" pixels inside updated boundary
              for ( int iY = iYbeg ; iY < iYend ; iY++ )
              {
                LPBYTE pSrc = GetData( m_pLastOriginalVideoFrame )
                  + iY * iYStep + iXbeg ;
                LPBYTE pDst = GetData( pVResult )
                  + iY * iYStep + iXbeg ;
                LPBYTE pXEnd = pSrc + iXend - iXbeg ;
                if ( m_iViewMode == TIVM_Binaryzed )
                {
                  while ( pSrc < pXEnd )
                  {
                    if ( *( pSrc++ ) > Thres )
                    {
                      *pDst = 255 ;
                      m_dLastFlowerArea_cm2++;
                    }
                    pDst++ ;
                  }
                }
                else
                {
                  while ( pSrc < pXEnd )
                  {
                    if ( *( pSrc++ ) > Thres )
                      m_dAltFlowerArea_cm2++;
                    pDst++ ;
                  }
                }
              }
              pOut->AddFrame( pVResult ) ;
            }

            cmplx bound[ 5 ];
            bound[ 0 ] = cmplx( boundary.s.x , boundary.s.y );
            bound[ 1 ] = cmplx( boundary.s.x , boundary.e.y );
            bound[ 2 ] = cmplx( boundary.e.x , boundary.e.y );
            bound[ 3 ] = cmplx( boundary.e.x , boundary.s.y );
            bound[ 4 ] = bound[ 0 ] ;
            pOut->AddFrame( CreateFigureFrame( bound , 5 , ( DWORD ) 0x00ffff ) ) ;

//             if (results.stalkContur.size())
//             {
//               CFigureFrame * pResult = CreateFigureFrame( ( cmplx* ) results.stalkContur.data() ,
//                 ( int ) results.stalkContur.size() , ( DWORD ) 0xff00ff ) ;
//               pResult->Attributes()->WriteInt( "thickness" , 3 ) ;
//               pOut->AddFrame( pResult ) ;
//               m_dLastStalkThickness_mm = results.stalkWidth * m_dScale_mm_per_pix ;
//             }

            m_dLastLength_mm = ( boundary.e.x - boundary.s.x ) * m_dScale_mm_per_pix ;
            m_dLastXRight_mm = boundary.e.x * m_dScale_mm_per_pix ;
            m_dLastWidth_mm = ( boundary.e.y - boundary.s.y ) * m_dScale_mm_per_pix ;
          }
        }
        break ;
        case TGM_FRONT:
        {
          DetectFlowerComponents t;
          DetectFlowerComponentsParameters p;
          memset( &p , 0 , sizeof( p ) ) ;

          p.MashingSize = 10;
          p.RelationDistance = 5.0;
          p.SmallAreaMaxSize = M_PI * 6 * 6; // pi*10*10
          p.BigAreaMinSize = p.SmallAreaMaxSize * 3;

          t.SetParameters( p );
          t.process( figures );

          const ProcessPlantImageResults& results = t.Results();

          for ( int i = 0; i < ( int ) results.includedConturs.size(); ++i )
          {
            CFigureFrame * pContur = CreateFigureFrame(
              ( cmplx* ) figures[ results.includedConturs[ i ] ].data() ,
              ( int ) figures[ results.includedConturs[ i ] ].size() , ( DWORD ) 0x0000ff ) ;
//             m_dLastFlowerArea_cm2 += GetFigureArea( pContur ) - ( pContur->GetConturLength() * 2 ) ;
            pOut->AddFrame( pContur ) ;
          }
        }
        break ;
      }
    }
    m_dLastFlowerArea_cm2 *= m_dScale_mm_per_pix * m_dScale_mm_per_pix / 100. ;
    m_dAltFlowerArea_cm2 *= m_dScale_mm_per_pix * m_dScale_mm_per_pix / 100. ;
    cmplx ViewHistPt( 10. , m_cLastFOV_pix.imag() * 0.9 ) ;
    pOut->AddFrame( CreateTextFrameEx( ViewHistPt ,
      0x00ffff , 12 , "Histo[%i,%i], Area=%.2f cm2, AltArea=%.2f cm2,   Frame[%d,%d],  ROI[%d,%d]mm " ,
      m_iHistoMinIndex , m_iHistoMaxIndex , m_dLastFlowerArea_cm2 , m_dAltFlowerArea_cm2 ,
      GetWidth( pImage ) , GetHeight( pImage ) , 
      ROUND(m_LastROI.Width() * m_dScale_mm_per_pix) , 
      ROUND( m_LastROI.Height() * m_dScale_mm_per_pix ) ) ) ;
  }

  m_dLastProcessingTime_ms = GetHRTickCount() - m_dLastStartTime_ms ;
  FormResult( pOut ) ;

//   FXString Msg ;
//   Msg.Format( "Output Frame Cnt=%d" , pOut->GetFramesCount() ) ;
//   Logger( Msg ) ;
 // _CrtDumpMemoryLeaks() ;
  return ( CDataFrame* ) pOut  ;
}

bool Tecto::ViewHistogram( CContainerFrame * pMarking ) 
{
  if ( m_LastROI.IsRectEmpty() )
    return false ;
#define BASE_X 10.
#define BASE_Y (m_LastROI.bottom + 110.)
#define TECTOHISTO_LEN 512.
#define TECTOHISTO_HEIGHT 100.

  cmplx cBasePt( BASE_X , BASE_Y ) ;
  cmplx cUpPt( BASE_X , BASE_Y - TECTOHISTO_HEIGHT ) ;
  cmplx cRightPt( BASE_X + TECTOHISTO_LEN , BASE_Y ) ;

  pMarking->AddFrame( CreateLineFrame( cBasePt , cUpPt , 0x0000ff ) ) ;
  pMarking->AddFrame( CreateLineFrame( cBasePt , cRightPt , 0x0000ff ) ) ;
  int iMaxVal = 0 , iLastVal = 0 ;
  for ( int i = 0 ; i < 256 ; i++ )
  {
    int iVal = m_LastHistoGram[ i ] ;
    if ( iVal )
    {
      iLastVal = i ;
      if ( iMaxVal < iVal )
        iMaxVal = iVal ;
    }
  }
  CFigureFrame * pHisto = CreatePtFrameEx( cBasePt , 0x0000ff ) ;
  for ( int i = 0 ; i < 256 ; i++ )
  {
    CDPoint PtOnGraph( BASE_X + i * TECTOHISTO_LEN / 256. , BASE_Y - m_LastHistoGram[ i ] * TECTOHISTO_HEIGHT / (double)iMaxVal ) ;
    pHisto->AddPoint( PtOnGraph ) ;
  }
  pMarking->AddFrame( pHisto ) ;
  cmplx cMarkUp( BASE_X + iLastVal * TECTOHISTO_LEN / 256. , BASE_Y - 10. ) ;
  cmplx cMarkDown( BASE_X + iLastVal * TECTOHISTO_LEN / 256. , BASE_Y ) ;
  pMarking->AddFrame( CreateLineFrame( cMarkUp , cMarkDown , 0x0000ff ) ) ;
  
  cmplx cCutMarkUp( BASE_X + m_iFoundCutLevel * TECTOHISTO_LEN / 256. , BASE_Y - 20. ) ;
  cmplx cCutMarkDown( BASE_X + m_iFoundCutLevel * TECTOHISTO_LEN / 256. , BASE_Y ) ;
  pMarking->AddFrame( CreateLineFrame( cCutMarkUp , cCutMarkDown , 0x00ffff ) ) ;
  return true ;
}


bool Tecto::FormResult( CContainerFrame * pMarking )
{
  FXString Result , TimeCRs ;
  cmplx cViewPt( 400. , 50. ) ;
  switch ( m_GadgetMode )
  {
    case TGM_SIDE:
    {
      ViewHistogram( pMarking ) ;
      if ( m_bTrayIsEmpty )
      {
        pMarking->AddFrame( CreateTextFrame( cViewPt ,
          "0x0000ff" , m_iResultOnScreenFontSize , "WholeResult" , pMarking->GetId() ,
          "FAILED: Tray is empty" ) ) ;

        FXString Result ;
        Result.Format( "FAILED: TRAY IS EMPTY. FileName=%s;" , m_pLastOriginalVideoFrame->GetLabel() ) ;
        CTextFrame * pResult = CTextFrame::Create( Result ) ;
        PutFrame( GetOutputConnector( 1 ) , pResult ) ;
      }
      else
      {
        FXString CRs( "\n" ) ;
        bool bWholeGood = true ;
        FXString LengthResult , XRightResult , AllResultsAsText , Tmp ;
        if ( m_bLengthMeasurement )
        {
          bool bGoodLength = IsInRange( m_dLastLength_mm , m_dMinimalLength_mm , m_dMaximalLength_mm ) ;
          LengthResult.Format( "Length is '%s' %.1f[%.1f,%.1f] mm\n" ,
            bGoodLength ? "OK" : "NOK" , m_dLastLength_mm , m_dMinimalLength_mm , m_dMaximalLength_mm ) ;
          Tmp.Format( "%.1f" , m_dLastLength_mm ) ;
          AllResultsAsText += Tmp ;
          pMarking->AddFrame( CreateTextFrame( cViewPt ,
            bGoodLength ? "0x00ff00" : "0x0000ff" ,
            m_iResultOnScreenFontSize , "LengthResult" , pMarking->GetId() ,
            _T( "%s%s" ) , ( LPCTSTR ) CRs , (LPCTSTR)LengthResult ) ) ;
          CRs += '\n' ;
          if ( !bGoodLength )
            bWholeGood = false ;
          if ( m_bXRightOut )
          {
            XRightResult.Format( "XRight=%.1f mm\n" , m_dLastXRight_mm ) ;
            Tmp.Format( "%.1f" , m_dLastXRight_mm ) ;
            if ( !AllResultsAsText.IsEmpty() )
              AllResultsAsText += ',' ;
            AllResultsAsText += Tmp ;

            pMarking->AddFrame( CreateTextFrame( cViewPt , "0x00ff00" ,
              m_iResultOnScreenFontSize , "XRightResult" , pMarking->GetId() ,
              _T( "%s%s" ) , ( LPCTSTR ) CRs , ( LPCTSTR ) XRightResult ) ) ;
            CRs += '\n' ;
          }
        }

        FXString WidthResult ;
        if ( m_bWidthMeasurement )
        {
          bool bGoodWidth = IsInRange( m_dLastWidth_mm , m_dMinimalWidth_mm , m_dMaximalWidth_mm ) ;
          WidthResult.Format( "Width is '%s' %.1f[%.1f,%.1f] mm\n" ,
            bGoodWidth ? "OK" : "NOK" , m_dLastWidth_mm , m_dMinimalWidth_mm , m_dMaximalWidth_mm ) ;
          Tmp.Format( "%.1f" , m_dLastWidth_mm ) ;
          if ( !AllResultsAsText.IsEmpty() )
            AllResultsAsText += ',' ;
          AllResultsAsText += Tmp ;
          pMarking->AddFrame( CreateTextFrame( cViewPt ,
            bGoodWidth ? "0x00ff00" : "0x0000ff" ,
            m_iResultOnScreenFontSize , "WidthResult" , pMarking->GetId() ,
            _T( "%s%s" ) , ( LPCTSTR ) CRs , ( LPCTSTR ) WidthResult ) ) ;
            CRs += '\n' ;
            if ( !bGoodWidth )
              bWholeGood = false ;
        }

        FXString AreaResult ;
        if ( m_bAreaMeasurement )
        {
          bool bGoodArea = IsInRange( m_dLastFlowerArea_cm2 , m_dMinimalFlowerArea_cm2 , m_dMaximalFlowerArea_cm2 ) ;
          AreaResult.Format( "Area is '%s' %.1f[%.1f,%.1f] cm2\n" ,
            bGoodArea ? "OK" : "NOK" , m_dLastFlowerArea_cm2 , m_dMinimalFlowerArea_cm2 , m_dMaximalFlowerArea_cm2 ) ;
          Tmp.Format( "%.1f" , m_dLastFlowerArea_cm2 ) ;
          if ( !AllResultsAsText.IsEmpty() )
            AllResultsAsText += ',' ;
          AllResultsAsText += Tmp ;
          pMarking->AddFrame( CreateTextFrame( cViewPt ,
            bGoodArea ? "0x00ff00" : "0x0000ff" ,
            m_iResultOnScreenFontSize , "WidthResult" , pMarking->GetId() ,
            _T( "%s%s" ) , ( LPCTSTR ) CRs , ( LPCTSTR ) AreaResult ) ) ;
          CRs += '\n' ;
          if ( !bGoodArea )
            bWholeGood = false ;
        }

        FXString StalkResult ;
        if ( m_bMeasureStalk )
        {
          bool bGoodStalk = IsInRange( m_dLastStalkThickness_mm , m_dStalkDiaMin_mm , m_dStalkDiaMax_mm ) ;
          StalkResult.Format( "Stalk Thickness is '%s' %.1f[%.1f,%.1f] mm\n" ,
            bGoodStalk ? "OK" : "NOK" , m_dLastStalkThickness_mm , m_dStalkDiaMin_mm , m_dStalkDiaMax_mm ) ;
          Tmp.Format( "%.1f" , m_dLastStalkThickness_mm ) ;
          if ( !AllResultsAsText.IsEmpty() )
            AllResultsAsText += ',' ;
          AllResultsAsText += Tmp ;
          pMarking->AddFrame( CreateTextFrame( cViewPt ,
            bGoodStalk ? "0x00ff00" : "0x0000ff" ,
            m_iResultOnScreenFontSize , "StalkResult" , pMarking->GetId() ,
            _T( "%s%s" ) , ( LPCTSTR ) CRs , ( LPCTSTR ) StalkResult ) ) ;
          CRs += '\n' ;
          if ( !bGoodStalk )
            bWholeGood = false ;
        }

        FXString StemResult ;
        if ( m_bMeasureCentralStem )
        {
          bool bGoodStem = IsInRange( m_dLastStemDia_mm , m_dStemDiaMin_mm , m_dStemDiaMax_mm ) ;
          StemResult.Format( "Stem Thickness is '%s' %.1f[%.1f,%.1f] mm\n" ,
            bGoodStem ? "OK" : "NOK" , m_dLastStemDia_mm , m_dStemDiaMin_mm , m_dStemDiaMax_mm ) ;
          Tmp.Format( "%.1f" , m_dLastStemDia_mm ) ;
          if ( !AllResultsAsText.IsEmpty() )
            AllResultsAsText += ',' ;
          AllResultsAsText += Tmp ;
          pMarking->AddFrame( CreateTextFrame( cViewPt ,
            bGoodStem ? "0x00ff00" : "0x0000ff" ,
            m_iResultOnScreenFontSize , "StalkResult" , pMarking->GetId() ,
            _T( "%s%s" ) , ( LPCTSTR ) CRs , ( LPCTSTR ) StemResult ) ) ;
          CRs += '\n' ;
          if ( !bGoodStem )
            bWholeGood = false ;
        }

        FXString WholeResult ;
        WholeResult.Format( "\nAnalyze result: %s%s%s%s%s%s%s Numerical=%s;\nFileName=%s;\n" ,
          bWholeGood ? "PASS\n" : "FAILED\n" ,
          ( LPCTSTR ) LengthResult , ( LPCTSTR ) XRightResult , ( LPCTSTR ) WidthResult ,
          ( LPCTSTR ) AreaResult , ( LPCTSTR ) StalkResult , ( LPCTSTR ) StemResult , 
          (LPCTSTR) AllResultsAsText , m_pLastOriginalVideoFrame->GetLabel() ) ;

        CTextFrame * pResult = CTextFrame::Create( WholeResult ) ;
        PutFrame( GetOutputConnector( 1 ) , pResult ) ;

        pMarking->AddFrame( CreateTextFrame( cViewPt ,
          bWholeGood ? "0x00ff00" : "0x0000ff" ,
          m_iResultOnScreenFontSize , "WholeResult" , pMarking->GetId() ,
          bWholeGood ? "PASS" : "FAILED" ) ) ;
      }
    }
    break ;
    case TGM_FRONT:
    {
      bool bGoodBurgeons = IsInRange( m_dLastFlowerArea_cm2 ,
         m_dMinimalFlowerArea_cm2 , m_dMaximalFlowerArea_cm2 ) ;

      pMarking->AddFrame( CreateTextFrame( cViewPt ,
        bGoodBurgeons ? "0x00ff00" : "0x0000ff" ,
        18 , "WholeResult" , pMarking->GetId() ,
        bGoodBurgeons ? "PASS" : "FAILED" ) ) ;

      pMarking->AddFrame( CreateTextFrame( cViewPt ,
        bGoodBurgeons ? "0x00ff00" : "0x0000ff" ,
        18 , "BurgeonResult" , pMarking->GetId() ,
        _T( "\nFlower Area = %.1f[%.1f,%.1f] cm^2;\n" ) ,
        m_dLastFlowerArea_cm2 , m_dMinimalFlowerArea_cm2 , m_dMaximalFlowerArea_cm2 ) );

      TimeCRs = _T( "\n\n" ) ;
    }
    break ;
    default: Result = "Unknown" ;
  }
  double dGraphTime = (GetGraphTime() * 1.e-3);
  double dHRTime = GetHRTickCount() ;
  if ( m_pLastOriginalVideoFrame )
    m_dLastProcessingTime_ms = dHRTime - m_pLastOriginalVideoFrame->GetAbsTime() ;
  m_dLastGadgetProcessingTime_ms = dHRTime - m_dLastStartTime_ms ;
  if ( m_pLastOriginalVideoFrame )
  {
    ( ( CVideoFrame* ) m_pLastOriginalVideoFrame )->Release() ;
    m_pLastOriginalVideoFrame = NULL ;
  }
//   DWORD PID = GetCurrentProcessId() ;
//   PROCESS_MEMORY_COUNTERS Counters ;
//   HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
//     PROCESS_VM_READ ,
//     FALSE , PID );
// 
//   if ( GetProcessMemoryInfo( hProcess , &Counters , sizeof( Counters ) ) )
//     m_dwMemOnExit = Counters.WorkingSetSize ;
//   else
//     m_dwMemOnExit = 0 ;
//   CloseHandle( hProcess ) ;

  cmplx ViewHistPt( 10. , m_cLastFOV_pix.imag() * 0.9 ) ;
  pMarking->AddFrame( CreateTextFrame( ViewHistPt ,
    m_dLastProcessingTime_ms < 400. ? "0x00ff00" : "0x0000ff" ,
    10 , "TimeAndScale" , pMarking->GetId() ,
    _T( "\nTpath = %.2f ms, Tgadget=%.2f Scale=%.4fmm/pix %s" ) ,
    m_dLastProcessingTime_ms , m_dLastGadgetProcessingTime_ms , 
    /*m_dwMemOnExit - m_dwMemOnEntry , */m_dScale_mm_per_pix , ( LPCTSTR ) m_AdditionalInfo ) ) ;
  return Result.IsEmpty() ;
} ;

void Tecto::Logger( LPCTSTR pLogString )
{
  PutFrame( GetOutputConnector( 1 ) , CreateTextFrame( pLogString , "LogMsg" ) ) ;
}

bool Tecto::DoCalibration( CContainerFrame * pMarking )
{
  cmplx cLeft( DBL_MAX , 0. ) , cRight( -DBL_MAX , 0. ) ;
  for ( size_t i = 0 ; i < m_LastSpots.size() - 1 ; i++ )
  {
    if ( cLeft.real() > m_LastSpots[ i ].m_SimpleCenter.x )
      cLeft = CDPointToCmplx( m_LastSpots[ i ].m_SimpleCenter ) ;
    if ( cRight.real() < m_LastSpots[ i ].m_SimpleCenter.x )
      cRight = CDPointToCmplx( m_LastSpots[ i ].m_SimpleCenter ) ;
  }
  if ( ( cLeft.real() != DBL_MAX ) && ( cRight.real() != -DBL_MAX ) )
  {
    cmplx cViewTextRes( cLeft );
    if ( m_LastROIs.size() )
      cViewTextRes._Val[ _IM ] = m_LastROIs[ 0 ].m_Rect.bottom + 10. ;
 
    double dDist_pix = cRight.real() - cLeft.real() ;
    m_dScale_mm_per_pix = m_dCalibLength_mm / dDist_pix ;
    pMarking->AddFrame( CreateLineFrame( cLeft , cRight , 0x00ff00 ) ) ;
    pMarking->AddFrame( CreateTextFrame( cViewTextRes , "0x0000ff" , 20 , "Dist&Scale" , 0 ,
      "Dist=%.2f Scale=%.6f mm/pix" , m_dCalibLength_mm , m_dScale_mm_per_pix ) ) ;

    CTextFrame * pAnswer = CreateTextFrameEx( "CalibResult" ,
      "Dist=%.2f Scale=%.6f mm/pix" , m_dCalibLength_mm , m_dScale_mm_per_pix ) ;

    FXString RegFolder( "TheFileX\\UI_NViews\\Tecto\\" ) ;
    RegFolder += GetTectoGadgetModeName() ;
    FXRegistry Reg( RegFolder ) ;

    Reg.WriteRegiDouble( m_FlowerName , "Scale_mm_per_pixel" , m_dScale_mm_per_pix ) ;
    Reg.WriteRegiDouble( m_FlowerName , "CalibLength_mm" , m_dCalibLength_mm ) ;
    PutFrame( GetOutputConnector( 1 ) , pAnswer ) ;
    return true ;
  }

  CTextFrame * pAnswer = CreateTextFrameEx( "CalibResult" , "Calibration Failed" ) ;
  PutFrame( GetOutputConnector( 1 ) , pAnswer ) ;

  pMarking->AddFrame( CreateTextFrame( m_cLastFOVCenter_pix , "0x0000ff" ,
    30 , "Bad scaling" , 0 , "Bad Scaling" ) ) ;
  return false ;
}

void Tecto::ProcessHistogram( CRect * pRC )
{
  CRect ROI = ( pRC ) ? *pRC : m_LastROI ;
  if ( ROI.left || ROI.top )
  {
//     GetHistogram( m_pLastOriginalVideoFrame ,
//       ROI , m_LastHistoGram ) ;
    double dMinAreaInPix = m_dMinimalFlowerArea_cm2 * 100. / ( m_dScale_mm_per_pix * m_dScale_mm_per_pix ) ;
    double dThres = m_bAreaMeasurement ? dMinAreaInPix * 0.5 
      : ROI.Width() * ROI.Height() * m_dHistoPerc * 0.01 ;
    m_iHistoMaxIndex = 0 ;
    int iSum = 0 ;
    for ( int i = 255 ; i >= 0 ; i-- )
    {
      if ( ( iSum += m_LastHistoGram[ i ] ) > dThres )
      {
        m_iHistoMaxIndex = i ;
        break ;
      }
    }
    dThres *= 10. ;
    iSum = 0 ;
    for ( int i = 0 ; i < 256 ; i++ )
    {
      if ( ( iSum += m_LastHistoGram[ i ] ) > dThres )
      {
        m_iHistoMinIndex = i ;
        break ;
      }
    }
  }
}

int Tecto::RestoreFlower( LPCTSTR pFlowerName )
{
  if ( !pFlowerName )
    pFlowerName = ( LPCTSTR ) m_FlowerName ;

  FXString RegFolder( "TheFileX\\UI_NViews\\Tecto\\" ) ;
  RegFolder += GetTectoGadgetModeName() ;
  FXRegistry Reg( RegFolder ) ;
  m_iResultOnScreenFontSize = Reg.GetRegiInt(
    "Presentation" , "FontSize" , 14 ) ;
  Reg.GetRegiCmplx( "Common" , "RectOnLineSize_mm" , m_cRectOnLineSize_mm , m_cRectOnLineSize_mm ) ;
  Reg.GetRegiCmplx( "Common" , "RectOnLineTol_mm" , m_cRectOnLineTol_mm , m_cRectOnLineTol_mm ) ;
  if ( m_GadgetMode == TGM_SIDE )
  {
    m_bLengthMeasurement = Reg.GetRegiInt(
      pFlowerName , "DoLengthMeasurement" , m_bLengthMeasurement ) ;
    m_bXRightOut = Reg.GetRegiInt(
      pFlowerName , "DoXRightOut" , m_bXRightOut ) ;
    m_bWidthMeasurement = Reg.GetRegiInt(
      pFlowerName , "DoWidthMeasurement" , m_bWidthMeasurement ) ;
    m_bAreaMeasurement = Reg.GetRegiInt(
      pFlowerName , "DoAreaMeasurement" , m_bAreaMeasurement ) ;
    m_bMeasureStalk = Reg.GetRegiInt(
      pFlowerName , "DoStalkMeasurement" , m_bMeasureStalk ) ;
    m_bMeasureCentralStem = Reg.GetRegiInt(
      pFlowerName , "DoMeasureCentralStem" , m_bMeasureCentralStem ) ;

    m_dMinimalLength_mm = Reg.GetRegiDouble(
      pFlowerName , "MinimalLength_mm" , m_dMinimalLength_mm ) ;
    m_dMaximalLength_mm = Reg.GetRegiDouble(
      pFlowerName , "MaximalLength_mm" , m_dMaximalLength_mm ) ;
    m_dMinimalWidth_mm = Reg.GetRegiDouble(
      pFlowerName , "MinimalWidth_mm" , m_dMinimalWidth_mm ) ;
    m_dMaximalWidth_mm = Reg.GetRegiDouble(
      pFlowerName , "MaximalWidth_mm" , m_dMaximalWidth_mm ) ;
    m_dStalkAndStemCorrection_pix = Reg.GetRegiDouble(
      pFlowerName , "ThicknessCorrection_pix" , m_dStalkAndStemCorrection_pix ) ;
    m_dStalkDiaMin_mm = Reg.GetRegiDouble(
      pFlowerName , "StalkDiaMin_mm" , m_dStalkDiaMin_mm ) ;
    m_dStalkDiaMax_mm = Reg.GetRegiDouble(
      pFlowerName , "StalkDiaMax_mm" , m_dStalkDiaMax_mm ) ;
    m_dStalkThreshold = Reg.GetRegiDouble(
      pFlowerName , "StalkThreshold" , m_dStalkThreshold ) ;
    m_dStalkShiftRight_mm = Reg.GetRegiDouble(
      pFlowerName , "StalkMeasShiftToRight" , m_dStalkShiftRight_mm ) ;

    m_dStemWidthStart_perc = Reg.GetRegiDouble(
      pFlowerName , "StemWidthStart_perc" , m_dStemWidthStart_perc ) ;
    m_dStemWidthEnd_perc = Reg.GetRegiDouble(
      pFlowerName , "StemWidthEnd_perc" , m_dStemWidthEnd_perc ) ;
    m_dStemDiaMin_mm = Reg.GetRegiDouble(
      pFlowerName , "StemMinDia_mm" , m_dStemDiaMin_mm ) ;
    m_dStemDiaMax_mm = Reg.GetRegiDouble(
      pFlowerName , "StemMaxDia_mm" , m_dStemDiaMax_mm ) ;
    m_dMinimalFlowerArea_cm2 = Reg.GetRegiDouble(
      pFlowerName , "MinimalFlowerArea_cm2" , m_dMinimalFlowerArea_cm2 ) ;
    m_dMaximalFlowerArea_cm2 = Reg.GetRegiDouble(
      pFlowerName , "MaximalFlowerArea_cm2" , m_dMaximalFlowerArea_cm2 ) ;

    m_dMachingSize = Reg.GetRegiDouble(
      pFlowerName , "MachingSize" , m_dMachingSize ) ;
    m_dXAxeCompressCoeff = Reg.GetRegiDouble(
      pFlowerName , "XAxeCompressCoeff" , m_dXAxeCompressCoeff ) ;
    m_dRelationDistance = Reg.GetRegiDouble(
      pFlowerName , "RelationDistance" , m_dRelationDistance ) ;
    m_dSlicedPartForStalk = Reg.GetRegiDouble(
      pFlowerName , "SlicedPartForStalk" , m_dSlicedPartForStalk ) ;
    m_iViewMode = (TectoImageViewMode)Reg.GetRegiInt(
      pFlowerName , "ViewMode(0-bin,1-Proc,2-Orig" , (int)m_iViewMode ) ;
    m_iMaxThreshold = ( TectoImageViewMode ) Reg.GetRegiInt(
      pFlowerName , "MaxForEmpty" , 75 ) ;

    m_dScale_mm_per_pix = Reg.GetRegiDouble( 
      pFlowerName , "Scale_mm_per_pixel" , m_dScale_mm_per_pix ) ;
    m_dCalibLength_mm = Reg.GetRegiDouble( 
      pFlowerName , "CalibLength_mm" , m_dCalibLength_mm ) ;
    m_dOutBinaryThres = Reg.GetRegiDouble(
      pFlowerName , "OutBinaryThreshold" , 0.38 ) ;
    m_dHistoCutLevelForEmptyTray = Reg.GetRegiDouble( 
      pFlowerName , "HistoCutLevelForEmptyTray" , 0.005 ) ;
  }
  else if ( m_GadgetMode == TGM_FRONT )
  {
  }
  return 0;
}


int Tecto::SaveToRegistry( LPCTSTR pFlowerName )
{
  if ( !pFlowerName )
    pFlowerName = ( LPCTSTR ) m_FlowerName ;

  FXString RegFolder( "TheFileX\\UI_NViews\\Tecto\\" ) ;
  RegFolder += GetTectoGadgetModeName() ;
  FXRegistry Reg( RegFolder ) ;

  if ( m_GadgetMode == TGM_SIDE )
  {
    Reg.WriteRegiInt(
      pFlowerName , "DoLengthMeasurement" , m_bLengthMeasurement ) ;
    Reg.WriteRegiInt(
      pFlowerName , "DoXRightOut" , m_bXRightOut ) ;
    Reg.WriteRegiInt(
      pFlowerName , "DoWidthMeasurement" , m_bWidthMeasurement ) ;
    Reg.WriteRegiInt(
      pFlowerName , "DoAreaMeasurement" , m_bAreaMeasurement ) ;
    Reg.WriteRegiInt(
      pFlowerName , "DoStalkMeasurement" , m_bMeasureStalk ) ;
    Reg.WriteRegiInt(
      pFlowerName , "DoMeasureCentralStem" , m_bMeasureCentralStem ) ;

    Reg.WriteRegiDouble(
      pFlowerName , "MinimalLength_mm" , m_dMinimalLength_mm ) ;
    Reg.WriteRegiDouble(
      pFlowerName , "MaximalLength_mm" , m_dMaximalLength_mm ) ;
    Reg.WriteRegiDouble(
      pFlowerName , "MinimalWidth_mm" , m_dMinimalWidth_mm ) ;
    Reg.WriteRegiDouble(
      pFlowerName , "MaximalWidth_mm" , m_dMaximalWidth_mm ) ;

    Reg.WriteRegiDouble(
      pFlowerName , "ThicknessCorrection_pix" , m_dStalkAndStemCorrection_pix ) ;
    Reg.WriteRegiDouble(
      pFlowerName , "StalkDiaMin_mm" , m_dStalkDiaMin_mm ) ;
    Reg.WriteRegiDouble(
      pFlowerName , "StalkDiaMax_mm" , m_dStalkDiaMax_mm ) ;
    Reg.WriteRegiDouble(
      pFlowerName , "StalkThreshold" , m_dStalkThreshold ) ;
    Reg.WriteRegiDouble(
      pFlowerName , "StalkMeasShiftToRight" , m_dStalkShiftRight_mm ) ;

    Reg.WriteRegiDouble(
      pFlowerName , "StemWidthStart_perc" , m_dStemWidthStart_perc ) ;
    Reg.WriteRegiDouble(
      pFlowerName , "StemWidthEnd_perc" , m_dStemWidthEnd_perc ) ;
    Reg.WriteRegiDouble(
      pFlowerName , "StemMinDia_mm" , m_dStemDiaMin_mm ) ;
    Reg.WriteRegiDouble(
      pFlowerName , "StemMaxDia_mm" , m_dStemDiaMax_mm ) ;
    Reg.WriteRegiDouble(
      pFlowerName , "MinimalFlowerArea_cm2" , m_dMinimalFlowerArea_cm2 ) ;
    Reg.WriteRegiDouble(
      pFlowerName , "MaximalFlowerArea_cm2" , m_dMaximalFlowerArea_cm2 ) ;

    Reg.WriteRegiDouble(
      pFlowerName , "MachingSize" , m_dMachingSize ) ;
    Reg.WriteRegiDouble(
      pFlowerName , "XAxeCompressCoeff" , m_dXAxeCompressCoeff ) ;
    Reg.WriteRegiDouble(
      pFlowerName , "RelationDistance" , m_dRelationDistance ) ;
    Reg.WriteRegiDouble(
      pFlowerName , "SlicedPartForStalk" , m_dSlicedPartForStalk ) ;
    
    Reg.WriteRegiInt(
      pFlowerName , "ViewMode(0-bin,1-Proc,2-Orig" , m_iViewMode ) ;

    Reg.WriteRegiDouble( pFlowerName , "Scale_mm_per_pixel" , m_dScale_mm_per_pix ) ;
    Reg.WriteRegiDouble( pFlowerName , "CalibLength_mm" , m_dCalibLength_mm ) ;
    Reg.WriteRegiDouble( pFlowerName , "OutBinaryThreshold" , m_dOutBinaryThres ) ;
    Reg.WriteRegiDouble( pFlowerName , "HistoCutLevelForEmptyTray" , m_dHistoCutLevelForEmptyTray ) ;

    SEND_GADGET_INFO( "Side data for '%s' is saved" , pFlowerName ) ;
  }
  else if ( m_GadgetMode == TGM_FRONT )
  {
    SEND_GADGET_INFO( "Front data for '%s' is saved" , pFlowerName ) ;
  }

  return 0;
}


double Tecto::GetStalkDia( CmplxRect& Boundary , CFigureFrame * pMainContur ,
   CVideoFrame * pImageForResult , CContainerFrame * pOut , int& iMinX )
{
  Extremes_s Extremes ;
  ExtrIndexes_s Indexes ;
  cmplx MainConturSize_pix ;
  double dAvgThickness_pix = 0 ;

  int iImageWidth = GetWidth( pImageForResult ) ;
  const cmplx * pContur = ( const cmplx* ) pMainContur->GetData() ;
  cmplx cCentByExtremes = FindExtrems( pContur , pMainContur->Size() ,
    Extremes , &Indexes , &MainConturSize_pix );
  cmplx cLeftPt = Extremes.m_cLeft ;
  int iLeftPtIndex = Indexes.m_iLeft ;
  double dInitialDia_pix , dAverageDia_pix ;
    ;
  CmplxVector cPlusV , cMinusV , Centers ;

  if ( m_dStalkShiftRight_mm > 0. )
  {
//     if ( Boundary.m_cLT.real() < cLeftPt.real() )
//       Boundary.m_cLT._Val[ _RE ] = cLeftPt.real() ;
    double dXBeg_pix= cLeftPt.real() + ( m_dStalkShiftRight_mm / m_dScale_mm_per_pix ) ;
    double dScanWidth_pix = ( 15.0 / m_dScale_mm_per_pix ) ;
    double dXLim_pix = dXBeg_pix + dScanWidth_pix ;
    int iScanWidth_pix = ROUND( dScanWidth_pix ) ;
    cmplx cInitPt = FindFirstPtForMeasurement( pImageForResult , pMainContur ,
        iLeftPtIndex , dXBeg_pix , dXLim_pix , dInitialDia_pix , pOut ) ;


    if ( TrackHContrastLine(
      cInitPt , ROUND( dInitialDia_pix * 2. ) ,
      Centers , m_dStalkThreshold , 30 , pImageForResult ,
      dAverageDia_pix ,pOut , iScanWidth_pix /*= 0*/ ) )
    {
      cmplx cLeftEdge = TrackLeft( pImageForResult , cInitPt , dAverageDia_pix , pOut ) ;
      if ( (cLeftEdge != 0.) && ( cLeftEdge.real() < Boundary.m_cLT.real() ) )
        Boundary.m_cLT._Val[ _RE ] = cLeftEdge.real() ;

      return dAverageDia_pix ;
    }
  }
  else // measure stalk near left edge
  {
    while ( fabs( cLeftPt.real()
      - pContur[ pMainContur->IncrementIndex( iLeftPtIndex ) ].real() ) < 1. ) ;
    cLeftPt = pContur[ iLeftPtIndex ] ;
    int iToPlusIndex = iLeftPtIndex ;
    int iToMinusIndex = iLeftPtIndex ;
    pMainContur->IncrementIndex( iToPlusIndex ) ;
    pMainContur->DecrementIndex( iToMinusIndex ) ;
    double dDiff = 0. , dPrevDiff ;
    int iCompareCount = 0 ;
    do
    {
      dPrevDiff = dDiff ;
      dDiff = abs( pContur[ pMainContur->IncrementIndex( iToPlusIndex ) ]
        - pContur[ pMainContur->DecrementIndex( iToMinusIndex ) ] ) ;
      if ( dDiff < ( dPrevDiff + 0.3 ) )
      {
        if ( ++iCompareCount > 3 )
          break ;
      }
      else
        iCompareCount = 0 ;
    } while ( ( iToPlusIndex - iLeftPtIndex ) < 50 );

    cmplx pBeginPlus = pContur[ iToPlusIndex ] ;
    cmplx pBeginMinus = pContur[ iToMinusIndex ] ;
    if ( m_iDebugView > 10 )
    {
      pOut->AddFrame( CreatePtFrame( cLeftPt , "color=0x00ffff; Sz=4;thickness=1;" ) ) ;
      pOut->AddFrame( CreatePtFrame( pBeginPlus , "color=0xffff00; Sz=3;thickness=1;" ) ) ;
      pOut->AddFrame( CreatePtFrame( pBeginMinus , "color=0xff00ff; Sz=3;thickness=1;" ) ) ;
    }

    if ( iToPlusIndex - iLeftPtIndex >= 50 )
    {
      double dAvStalkDia_pix ;
      cmplx cInitCent = cLeftPt + cmplx( 8. , 0. ) ;
      int iNSlices = (int) TrackHContrastLine(
        cInitCent , ROUND( m_dStalkDiaMax_mm * 1.5 / m_dScale_mm_per_pix ) ,
        Centers , 0.5 , 20 , pImageForResult , dAvStalkDia_pix , pOut ) ;

      if ( iNSlices )
        dAvgThickness_pix = dAvStalkDia_pix * m_dScale_mm_per_pix ;
    }
    else
    {
    // X alignment
      double dXDiff = pBeginPlus.real() - pBeginMinus.real() ;
      if ( dXDiff > 1. )
      {
        while ( pBeginPlus.real() > pContur[ pMainContur->DecrementIndex( iToMinusIndex ) ].real() )
        {
          if ( ( iLeftPtIndex - iToMinusIndex ) >= 80 ) // too far, no stalk
            return 0. ;
        }
        pBeginMinus = pContur[ iToMinusIndex ] ;
      }
      else if ( dXDiff < 1. )
      {
        while ( pBeginMinus.real() > pContur[ pMainContur->IncrementIndex( iToPlusIndex ) ].real() )
        {
          if ( ( iToPlusIndex - iLeftPtIndex ) >= 80 ) // too far, no stalk
            return 0. ;
        }
        pBeginPlus = pContur[ iToPlusIndex ] ;
      }
      int iToPlusOrig = iToPlusIndex , iToMinusOrig = iToMinusIndex ;

      double dMinVal = DBL_MAX ;
      int iMinIndex = -1 ;
      CFRegression RegrCent ;
      CFStatistics DiaStat ;
      for ( size_t i = 0 ; i < ( size_t ) m_iStalkMeasDist_pix ; i++ )
      {
        cmplx cPlus = pContur[ pMainContur->IncrementIndex( iToPlusIndex ) ] ;
        cmplx cMinus = pContur[ pMainContur->DecrementIndex( iToMinusIndex ) ];
        cmplx cMinusToPlus = cPlus - cMinus ;
        cPlus -= m_dStalkAndStemCorrection_pix * GetNormalized( cMinusToPlus ) ;
        cMinus += m_dStalkAndStemCorrection_pix * GetNormalized( cMinusToPlus ) ;
        cMinusToPlus = cPlus - cMinus ;
    //     cPlusV.push_back( cPlus ) ;
    //    RegrPlus.Add( cPlus ) ;
    //     cMinusV.push_back( cMinus ) ;
    //    RegrMinus.Add( cMinus ) ;
        double dDia = abs( cMinusToPlus ) /*- m_dStalkAndStemCorrection_pix * 2. */; // Erosion compensation
        if ( dDia < dMinVal )
        {
          iMinIndex = ( int ) i ;
          dMinVal = dDia ;
        }
        dAvgThickness_pix += dDia ;
        DiaStat.Add( dDia ) ;
        cmplx cCent = ( cPlus + cMinus ) * 0.5 ;
        RegrCent.Add( cCent ) ;
        Centers.push_back( cCent ) ;
      }
      RegrCent.Calculate() ;
    //   RegrPlus.Calculate() ;
    //   RegrMinus.Calculate() ;
      dAvgThickness_pix /= m_iStalkMeasDist_pix ;
      double dAngle_rad = RegrCent.GetAngle() ;

      if ( pImageForResult )
      {
        double dLineWidth ;
        cmplx cInitCent = Centers.back() ;
        cmplx cUp = cInitCent + cmplx( 0. , -dAvgThickness_pix * 2. ) ;
        cmplx cDown = cInitCent + cmplx( 0. , dAvgThickness_pix * 2. ) ;

        CPoint PtLT = GetCPoint( cUp ) ;
        CSize RectSz( 3 , ROUND( dAvgThickness_pix * 4. ) ) ;

        CPoint PtCent = GetCPoint( cInitCent ) ;
        double dAvgWidth_pix = 0. ;
        LPBYTE pCent = GetData( pImageForResult ) + PtCent.x + PtCent.y * iImageWidth  ;
        double dCentY = find_line_pos_ud(
          pCent , RectSz.cy , 0.5 , 20 , dLineWidth , iImageWidth ) ;
        cmplx cCent( PtCent.x , PtCent.y + dCentY ) ;
        cmplx cUpper = cCent + cmplx( 0. , -dLineWidth * 0.5 ) ;
        cmplx cLower = cCent + cmplx( 0. , dLineWidth * 0.5 ) ;
        pOut->AddFrame( CreatePtFrame( cCent , "color=0xffffff ; Sz=2;thickness=1;" ) ) ;
        CFRegression RegrAmplCent , RegrUpper , RegrLower ;

        CmplxVector LeftScanCenters ;
        DoubleVector Widths ;
        DiaStat.Reset() ;
        double dAvgDiaCont_pix = 0. ;
        double dDiaStdCont_pix = 0. ;
        int iNPtsCont = 0 ;
        if ( dCentY < ( RectSz.cy / 4. ) )
        {
          LeftScanCenters.push_back( cCent ) ;
          Widths.push_back( dLineWidth ) ;
          DiaStat.Add( dLineWidth ) ;
          dAvgWidth_pix += dLineWidth ;
          RegrAmplCent.Add( cCent ) ;
          RegrUpper.Add( cUpper ) ;
          RegrLower.Add( cLower ) ;
        }
        int iOmitCounter = 0 ;
        while ( ( --PtCent.x > 0 ) && ( iOmitCounter < 6 ) )
        {
          PtCent.y = ROUND( cCent.imag() ) ;
          pCent = GetData( pImageForResult ) + PtCent.x + PtCent.y * iImageWidth  ;
          double dCentY = find_line_pos_ud(
            pCent , RectSz.cy , 0.5 , 20 , dLineWidth , iImageWidth ) ;
          if ( dCentY < ( RectSz.cy / 4. ) )
          {
            cCent = cmplx( PtCent.x , PtCent.y + dCentY ) ;
            RegrAmplCent.Add( cCent ) ;
            LeftScanCenters.push_back( cCent ) ;
            Widths.push_back( dLineWidth ) ;
            DiaStat.Add( dLineWidth ) ;
            dAvgWidth_pix += dLineWidth ;
            iMinX = ROUND( cCent.real() ) ;
            cUpper = cCent + cmplx( 0. , -dLineWidth * 0.5 ) ;
            cLower = cCent + cmplx( 0. , dLineWidth * 0.5 ) ;
            cMinusV.push_back( cUpper ) ;
            cPlusV.push_back( cLower ) ;
            RegrUpper.Add( cUpper ) ;
            RegrLower.Add( cLower ) ;
            if ( ( ( PtCent.x - cLeftPt.real() ) < 3 )
              && ( dAvgDiaCont_pix == 0. )
              && ( LeftScanCenters.size() > 5 ) )
            {
              dAvgDiaCont_pix = DiaStat.Calculate() ;
              dDiaStdCont_pix = DiaStat.GetStd() ;

              RegrAmplCent.Calculate() ;
              dAngle_rad = RegrAmplCent.GetAngle() ;
              dAvgDiaCont_pix *= fabs( cos( dAngle_rad ) ) ;
              iNPtsCont = ( int ) LeftScanCenters.size() ;
            }
  //           if ( m_iViewMode == TIVM_Binaryzed )
  //           {
  //             CPoint UpPt = GetCPoint( cMinus ) ; // this is upper point on image (Y goes down)
  //             LPBYTE pPt = GetData( pImageForResult ) + UpPt.x + UpPt.y * iImageWidth  ;
  //             LPBYTE pLowerPt = pPt + iImageWidth * ROUND( dLineWidth ) ;
  //             while ( pPt <= pLowerPt )
  //             {
  //               *pPt = 255 ;
  //               pPt += iImageWidth ;
  //             }
  //           }
          }
          else
            iOmitCounter++ ;
        }
        if ( LeftScanCenters.size() > 5 )
        {
          RegrAmplCent.Calculate() ;
          RegrUpper.Calculate() ;
          RegrLower.Calculate() ;
          dAvgThickness_pix = DiaStat.Calculate() ;
          double cDiaStd_pix = DiaStat.GetStd() ;

          if ( cDiaStd_pix > dAvgDiaCont_pix * 0.3 )
          {
            iMinX = ( int ) cLeftPt.real() ;
            dAvgThickness_pix = dAvgDiaCont_pix ;
            LeftScanCenters.resize( iNPtsCont ) ;
            cMinusV.resize( iNPtsCont ) ;
            cPlusV.resize( iNPtsCont ) ;
          }
          else
          {
            dAngle_rad = RegrAmplCent.GetAngle() ;
            dAvgThickness_pix *= fabs( cos( dAngle_rad ) ) ;
          }
        }
        pOut->AddFrame( CreateFigureFrameEx( LeftScanCenters ,
          ( m_iViewMode == TIVM_Binaryzed ) ? 0xff0000 : 0xffffff , 3 ) ) ;
        if ( m_iDebugView > 5 )
        {
          pOut->AddFrame( CreatePtFrame( cInitCent , "color=0x00ffff; Sz=2;thickness=1;" ) ) ;
          pOut->AddFrame( CreatePtFrame( cUp , "color=0xffff00; Sz=2;thickness=1;" ) ) ;
          pOut->AddFrame( CreatePtFrame( cDown , "color=0xff00ff; Sz=2;thickness=1;" ) ) ;
        }
      }

      pOut->AddFrame( CreateFigureFrameEx( cMinusV.data() ,
        ( int ) cMinusV.size() , "color=0xff00ff;thickness=1;" ) ) ;
      pOut->AddFrame( CreateFigureFrameEx( cPlusV.data() ,
        ( int ) cPlusV.size() , "color=0xffff00;thickness=1;" ) ) ;
    }
  }
  return dAvgThickness_pix ;
}

double Tecto::GetStemDia( CmplxRect& Boundary , CFigureFrame * pMainContur , 
  CVideoFrame * pImageForResult ,  CContainerFrame * pOut )
{
  if ( !pImageForResult || !pMainContur )
    return 0. ;

  Extremes_s Extremes ;
  ExtrIndexes_s Indexes ;
  cmplx MainConturSize_pix ;
  cmplx cCentByExtremes = FindExtrems( 
    ( const cmplx* ) pMainContur->GetData() , pMainContur->Size() ,
    Extremes , &Indexes , &MainConturSize_pix );
  int iLeftPtIndex = Indexes.m_iLeft ;

  bool bNearButton = ( m_dStemWidthStart_perc > 99. ) && ( m_dStemWidthEnd_perc > 99. ) ;
  double dStartPerc = bNearButton ? 50. : m_dStemWidthStart_perc ;
  double dEndPerc =  bNearButton ? 60. : m_dStemWidthEnd_perc ;

  double dXBegin = Boundary.m_cLT.real() 
    + (Boundary.GetWidth() * dStartPerc / 100.) ;
  if ( dXBegin < Extremes.m_cLeft.real() || dXBegin > Extremes.m_cRight.real() )
    return -1. ;
  double dXEnd = Boundary.m_cLT.real()
    + ( Boundary.GetWidth() * dEndPerc / 100. ) ;

  // iLeftPointIndex on exit holds index in m_UpperByContur, 
  // m_LowerByContur and m_Diameters arrays
  cmplx cInitCenter = CheckAndScanStem(
    pImageForResult , pMainContur , iLeftPtIndex , dXBegin ,
    bNearButton ? Extremes.m_cRight.real() : dXEnd , bNearButton , pOut ) ;
  if ( cInitCenter.real() == -1 )
    return -1. ;
  if ( cInitCenter == 0. )
    return 0. ;
  if ( bNearButton )
    dXEnd = cInitCenter.real() + SliceStep * 3.0 ;

  // find slice to plus and to minus from left point on contour
//   cmplx cPlusPt = FindNearestToX( pMainContur , iIndexToPlus , dXBegin , true ) ;
//   cmplx cMinusPt = FindNearestToX( pMainContur , iIndexToMinus , dXBegin , false ) ;

  CFStatistics WidthStatistics ;

  int iImageWidth = GetWidth( pImageForResult ) ;
  double dLineWidth ;
  double dAvgWidth_pix = 0. ;
  int iRange_pix = ROUND(m_dStemDiaMax_mm * 2.5) ;

  double dCentY = 0. ;
  CPoint PtCent ;
  LPBYTE pCent ;
  do 
  {
    PtCent = GetCPoint( cInitCenter ) ;
    pCent = GetData( pImageForResult ) + PtCent.x + PtCent.y * iImageWidth  ;
    dCentY = find_line_pos_ud(
      pCent , iRange_pix , 0.3 , 20 , dLineWidth , iImageWidth ) ;
    if ( (dCentY < iRange_pix) || !bNearButton )
      break ;
    if ( (iLeftPtIndex >= 3) )
    {
      iLeftPtIndex -= 3 ;
      cInitCenter = 0.5 * 
        ( m_UpperByContur[ iLeftPtIndex ] + m_LowerByContur[ iLeftPtIndex ] ) ;
    }
  } while ( iLeftPtIndex >= 0 );

  if ( dCentY < iRange_pix )
  {
    cmplx cCent( PtCent.x , PtCent.y + dCentY ) ;
    pOut->AddFrame( CreatePtFrame( cCent , "color=0xffffff ; Sz=2;thickness=1;" ) ) ;

    CmplxVector UpperEdge , LowerEdge ;
    DoubleVector Widths ;

    cmplx cUpperPt( cCent.real() , cCent.imag() - dLineWidth * 0.5 ) ;
    cmplx cLowerPt( cCent.real() , cCent.imag() + dLineWidth * 0.5 ) ;
    UpperEdge.push_back( cUpperPt ) ;
    LowerEdge.push_back( cLowerPt ) ;
    Widths.push_back( dLineWidth ) ;
    WidthStatistics.Add( dLineWidth ) ;

    dAvgWidth_pix += dLineWidth ;

    int iOmitCounter = 0 ;
    while ( (++PtCent.x < dXEnd) && (iOmitCounter++ < 5) )
    {
      iOmitCounter = 0 ;
      cCent._Val[ _RE ] = PtCent.x ;
      PtCent.y = ROUND( cCent.imag() ) ;
      pCent = GetData( pImageForResult ) + PtCent.x + PtCent.y * iImageWidth  ;
      double dCentY = find_line_pos_ud(
        pCent , iRange_pix , 0.3 , 20 , dLineWidth , iImageWidth ) ;
      if ( dCentY < iRange_pix )
      {
        cCent = cmplx( PtCent.x , PtCent.y + dCentY ) ;
        cmplx cPlus = cCent + cmplx( 0. , dLineWidth * 0.5 ) ;
        cmplx cMinus = cCent + cmplx( 0. , -dLineWidth * 0.5 ) ;

        UpperEdge.push_back( cMinus ) ;
        LowerEdge.push_back( cPlus ) ;
        Widths.push_back( dLineWidth ) ;
        WidthStatistics.Add( dLineWidth ) ;
        dAvgWidth_pix += dLineWidth ;

        if ( m_iViewMode == TIVM_Binaryzed )
        {
          CPoint UpPt = GetCPoint( cMinus ) ; // this is upper point on image (Y goes down)
          LPBYTE pPt = GetData( pImageForResult ) + UpPt.x + UpPt.y * iImageWidth  ;
          LPBYTE pLowerPt = pPt + iImageWidth * ROUND( dLineWidth ) ;
          while ( pPt <= pLowerPt )
          {
            *pPt = 255 ;
            pPt += iImageWidth ;
          }
        }
      }
      else
        iOmitCounter++ ;
    }
    if ( UpperEdge.size() > 5 )
    {
      if ( m_iDebugView > 5 )
      {
        pOut->AddFrame( CreateFigureFrameEx( UpperEdge , 0xff00ff , 1 ) ) ;
        pOut->AddFrame( CreateFigureFrameEx( LowerEdge , 0xffff00 , 1 ) ) ;
      }
      CFRegression RUpperEdge , RLowerEdge ;
      RUpperEdge.AddPtsToRegression( UpperEdge ) ;
      RLowerEdge.AddPtsToRegression( LowerEdge ) ;

      RUpperEdge.Calculate() ;
      RLowerEdge.Calculate() ;
      double dStatAvgWidth_pix = WidthStatistics.Calculate() ;
      double dWidthStd = WidthStatistics.GetStd() ;
      double dUpperLimit = dStatAvgWidth_pix + dWidthStd * 0.5 ;

      CLine2d UpperLine = RUpperEdge.GetCLine2d() ;
      double dUpperStd = sqrt( RUpperEdge.GetRSquared() ) ;
      CLine2d LowerLine = RLowerEdge.GetCLine2d() ;
      double dLowerStd = sqrt( RLowerEdge.GetRSquared() ) ;

      CFStatistics FilteredStat ;
      int iNRemoved = 0 ;
      for ( int i = 0 ; i < (int)Widths.size() ; i++ )
      {
        int iRealIndex = ( i - iNRemoved ) ;
        double dUpperYAvg = UpperLine.dist( UpperEdge[ iRealIndex ] ) ;
        if ( dUpperYAvg > dWidthStd )
        {
          UpperEdge.erase( UpperEdge.begin() + iRealIndex ) ;
          LowerEdge.erase( LowerEdge.begin() + iRealIndex ) ;
          iNRemoved++ ;
        }
        else
        {
          double dLowerYAvg = LowerLine.dist( LowerEdge[ iRealIndex ] ) ;
          if ( -dLowerYAvg > dWidthStd )
          {
            UpperEdge.erase( UpperEdge.begin() + iRealIndex ) ;
            LowerEdge.erase( LowerEdge.begin() + iRealIndex ) ;
            iNRemoved++ ;
          }
          else
            FilteredStat.Add( Widths[ i ] ) ;
        }
      }
      if ( m_iDebugView > 5 )
      {
        pOut->AddFrame( CreateFigureFrameEx( UpperEdge , 0x00ffff , 3 ) ) ;
        pOut->AddFrame( CreateFigureFrameEx( LowerEdge , 0xffffff , 3 ) ) ;
      }


      dAvgWidth_pix = FilteredStat.Calculate() ;
      double dUpperAng = RUpperEdge.GetAngle() ;
      dUpperAng = NormRad( dUpperAng ) ;
      double dLowerAng = RLowerEdge.GetAngle() ;
      dLowerAng = NormRad( dLowerAng ) ;
      double dMainAngle = ( dUpperAng + dLowerAng ) * 0.5 ;
      return dAvgWidth_pix * m_dScale_mm_per_pix * fabs( cos( dMainAngle ) ) ;
  // 
  //           dAvgThickness_mm = dAvgWidth_pix ;
  //           dAngle_rad = RegrAmplCent.GetAngle() ;
    }

  }
  return 0. ;
}

size_t Tecto::TrackHContrastLine( 
  cmplx cInitCent , int iStripRange , 
  CmplxVector& ScanCenters , double dNormThres , int iMinAmpl ,
  const CVideoFrame * pVF , double& dAvLineWidth_pix ,
  CContainerFrame* pMarking , int iScanWidth_pix /*= 0*/ ) // 0 - no restriction
{
  double dLineWidth ;
  int iImageWidth = GetWidth( pVF ) ;
  cmplx cUp = cInitCent + cmplx( 0. , -iStripRange ) ;
  cmplx cDown = cInitCent + cmplx( 0. , iStripRange ) ;

  CPoint PtLT = GetCPoint( cUp ) ;
  if ( (PtLT.y <= 0) || (PtLT.y >= (int) (GetHeight( pVF ) - iStripRange * 2 )))
  {
    return 0 ;
  }

  BYTE bMin = 255 , bMax = 0 ;

  LPBYTE pOrigin = GetData( pVF ) + PtLT.x + PtLT.y * iImageWidth ;
  LPBYTE pMaxSlicePos = GetMaxPos( pOrigin ,
    iStripRange * 2 , bMin , bMax , iImageWidth ) ;
  CPoint PtCent = GetCPoint( cInitCent ) ;
  double dAvgWidth_pix = 0. ;
  int iNRows = (int)(( pMaxSlicePos - pOrigin ) / iImageWidth );
  PtCent.y = PtLT.y + iNRows ;

  LPBYTE pCent = pMaxSlicePos ; // GetData( pVF ) + PtCent.x + PtCent.y * iImageWidth  ;
  double dCentY = find_line_pos_ud(
    pCent , iStripRange * 2 , 0.5 , 20 , dLineWidth , iImageWidth ) ;
  cmplx cCent( PtCent.x , PtCent.y + dCentY ) ;
  if ( fabs( dCentY ) > ( iStripRange * 0.9 ) )
    return 0 ;

  if ( pMarking )
    pMarking->AddFrame( CreatePtFrame(
      cCent , "color=0x4040ff ; Sz=2;thickness=1;" ) ) ;

  CmplxVector cPlusV , cMinusV ;
  DoubleVector Widths ;
  CFRegression RegrCent ;

  ScanCenters.push_back( cCent ) ;
  Widths.push_back( dLineWidth ) ;
  RegrCent.Add( cCent ) ;

  int iOmitCounter = 0 ;
  int iScannedWidth_pix = (iScanWidth_pix == 0) ? (int)GetWidth( pVF ) : iScanWidth_pix ;
  while ( ( ++PtCent.x < PtLT.x + 40 ) && ( iOmitCounter < 6 ) && (iScanWidth_pix-- > 0) )
  {
    PtCent.y = ROUND( cCent.imag() ) ;
    pCent = GetData( pVF ) + PtCent.x + PtCent.y * iImageWidth  ;
    double dCentY = find_line_pos_ud(
      pCent , iStripRange * 2 , 0.5 , 20 , dLineWidth , iImageWidth ) ;
    if ( dCentY < ( iStripRange * 0.5 ) )
    {
      cCent = cmplx( PtCent.x , PtCent.y + dCentY ) ;
      ScanCenters.push_back( cCent ) ;
      Widths.push_back( dLineWidth ) ;
      dAvgWidth_pix += dLineWidth ;
      cmplx cPlus = cCent + cmplx( 0. , dLineWidth * 0.5 ) ;
      cmplx cMinus = cCent + cmplx( 0. , -dLineWidth * 0.5 ) ;
      cPlusV.push_back( cPlus ) ;
      cMinusV.push_back( cMinus ) ;
      if ( m_iViewMode == TIVM_Binaryzed )
      {
        CPoint UpPt = GetCPoint( cMinus ) ; // this is upper point on image (Y goes down)
        LPBYTE pPt = GetData( pVF ) + UpPt.x + UpPt.y * iImageWidth  ;
        LPBYTE pLowerPt = pPt + iImageWidth * ROUND( dLineWidth ) ;
        while ( pPt <= pLowerPt )
        {
          *pPt = 255 ;
          pPt += iImageWidth ;
        }
      }
    }
    else
      iOmitCounter++ ;
  }
  if ( ScanCenters.size() < 5 )
    return 0 ;

  RegrCent.Calculate() ;
  dAvgWidth_pix /= Widths.size() ;
  double dAngle_rad = RegrCent.GetAngle() ;
  dAvLineWidth_pix = dAvgWidth_pix * fabs( cos( dAngle_rad ) ) ;

  if ( pMarking )
  {
    pMarking->AddFrame( CreateFigureFrameEx( ScanCenters ,
      ( m_iViewMode == TIVM_Binaryzed ) ? 0xff0000 : 0xffffff , 1 ) ) ;
    if ( m_iDebugView > 5 )
    {
      pMarking->AddFrame( CreateFigureFrameEx( cPlusV , 0xffff00 ) ) ;
      pMarking->AddFrame( CreateFigureFrameEx( cMinusV , 0xff00ff ) ) ;
//       pMarking->AddFrame( CreatePtFrame( cInitCent , "color=0x00ffff; Sz=2;thickness=1;" ) ) ;
//       pMarking->AddFrame( CreatePtFrame( cUp , "color=0xffff00; Sz=2;thickness=1;" ) ) ;
//       pMarking->AddFrame( CreatePtFrame( cDown , "color=0xff00ff; Sz=2;thickness=1;" ) ) ;
    }
  }
  return ScanCenters.size() ;
}

cmplx Tecto::CheckAndScanStem( 
  CVideoFrame * pImageForResult , CFigureFrame * pMainContur , 
  int& iLeftPtIndex , double dXBegin , double dXLim ,
  bool bScanToTheRight , CContainerFrame * pOut )
{
  if ( !pMainContur || !pImageForResult )
    return cmplx( 0. , 0. ) ;
// find slices to plus and to minus from left point on contour
  m_UpperByContur.clear() ;
  m_LowerByContur.clear() ;
  m_Diameters.clear() ;
  int iIndexToPlus = iLeftPtIndex , iIndexToMinus = iLeftPtIndex ;
  cmplx cPlusPt , cPlusPtByContur ;
  cmplx cMinusPt , cMinusPtByContur ;
  double dInitialDiaByContur_mm = 0 ;
  double dMinDiaByContur = DBL_MAX ;
  DWORD dwMinIndexByContur = -1 ;
  int iNAttepts = 0 ;
  do
  {
    cPlusPtByContur = FindNearestToX( pMainContur , iIndexToPlus , dXBegin , true ) ;
    cMinusPtByContur = FindNearestToX( pMainContur , iIndexToMinus , dXBegin , false ) ;
    if ( ( cPlusPtByContur.real() > 0. ) && ( cMinusPtByContur.real() > 0. ) )
    {
      if ( cPlusPt.real() == 0 )
      {
        cPlusPt = cPlusPtByContur ;
        cMinusPt = cMinusPtByContur ;
        dInitialDiaByContur_mm = abs( cMinusPt - cPlusPt ) * m_dScale_mm_per_pix ;
        if ( !bScanToTheRight )
          break ;
      }
      m_UpperByContur.push_back( cPlusPtByContur ) ;
      m_LowerByContur.push_back( cMinusPtByContur ) ;
      double dDiaByContur = abs( cMinusPtByContur - cPlusPtByContur ) ;
      m_Diameters.push_back( dDiaByContur ) ;
      if ( dDiaByContur < dMinDiaByContur  )
      {
        dMinDiaByContur = dDiaByContur ;
        dwMinIndexByContur = (DWORD)m_UpperByContur.size() - 1 ;
      }
      if ( m_iDebugView > 7 )
      {
        pOut->AddFrame( CreatePtFrameEx( cPlusPtByContur , ( DWORD ) 0xffffff ) ) ;
        pOut->AddFrame( CreatePtFrameEx( cMinusPtByContur , ( DWORD ) 0xffffff ) ) ;
      }
      iNAttepts = 0 ;
    }
    dXBegin += 10. ;
    if ( ++iNAttepts > 10 )
      return 0. ;
  } while ( dXBegin < dXLim - 30. ) ;// edges not found

  if ( bScanToTheRight )
  {
    while ( dwMinIndexByContur >= 0 )
    {
      if ( dMinDiaByContur * m_dScale_mm_per_pix > 1.5 * m_dStemDiaMax_mm )
        return -1 ; // tray is empty
      DWORD dwToPlus = dwMinIndexByContur + 1 ;
      while ( (dwToPlus < m_Diameters.size()) 
        && ( m_Diameters[ dwToPlus ] < dMinDiaByContur * 1.1) )
      {
        dwToPlus++ ;
      }
      if ( pOut && ( m_iDebugView > 8 ) )
        pOut->AddFrame( CreateLineFrameEx( m_UpperByContur[ dwToPlus ] , m_LowerByContur[ dwToPlus ] , 0xffff00 ) ) ;

      dwToPlus-- ;
      if ( ( dwToPlus - dwMinIndexByContur ) > 2 )
      {
        iLeftPtIndex = dwToPlus - 3 ;
        cPlusPtByContur = m_UpperByContur[ iLeftPtIndex ] ;
        cMinusPtByContur = m_LowerByContur[ iLeftPtIndex ] ;
      }
      else
      {
        DWORD dwToMinus = dwMinIndexByContur - 1 ;
        if ( dwToMinus > 100000 )
          break ;
        while ( dwToMinus && ( m_Diameters[ dwToMinus ] < dMinDiaByContur * 1.1 ) )
          dwToMinus-- ;
       
        if ( pOut && ( m_iDebugView > 8 ) )
          pOut->AddFrame( CreateLineFrameEx( m_UpperByContur[ dwToMinus ] , m_LowerByContur[ dwToMinus ] , 0xffff00 ) ) ;
        dwToMinus++ ;
        if ( ( dwMinIndexByContur - dwToMinus ) > 2 )
        {
          iLeftPtIndex = dwMinIndexByContur - 2 ;
          cPlusPtByContur = m_UpperByContur[ iLeftPtIndex ] ;
          cMinusPtByContur = m_LowerByContur[ iLeftPtIndex ] ;
        }
        else if ( dwToPlus - dwToMinus >= 3 )
        {
          iLeftPtIndex = dwToMinus - 2 ;
          cPlusPtByContur = m_UpperByContur[ iLeftPtIndex ] ;
          cMinusPtByContur = m_LowerByContur[ iLeftPtIndex ] ;
        }
        else // we have to find another minimum
        {
          m_UpperByContur.erase( m_UpperByContur.begin() + dwMinIndexByContur ) ;
          m_LowerByContur.erase( m_LowerByContur.begin() + dwMinIndexByContur ) ;
          m_Diameters.erase( m_Diameters.begin() + dwMinIndexByContur ) ;
          if ( m_Diameters.size() < 3 )
            break ;
          auto pdMinDia = std::min_element( m_Diameters.begin() , m_Diameters.end() ) ;
          dwMinIndexByContur = ( DWORD ) ( pdMinDia - m_Diameters.begin() ) ;
          continue ;
        }
      }
      return ( cPlusPtByContur + cMinusPtByContur ) * 0.5 ;
    }
  }

  int iImageWidth = GetWidth( pImageForResult ) ;
  cmplx cInitCent = ( cPlusPt + cMinusPt ) * 0.5 ;
  if ( pOut && ( m_iDebugView > 5 ) )
    pOut->AddFrame( CreateLineFrameEx( cPlusPt , cMinusPt , 0xff00ff ) ) ;
  int iRange_pix = ROUND( abs( cMinusPt - cPlusPt ) * 2. ) ;
  if ( iRange_pix < 10 )
    iRange_pix = 20 ;

  double dStemDia ;
  double dAvgWidth_pix = 0. ;

  CPoint PtCent = GetCPoint( cInitCent ) ;
  LPBYTE pCent = GetData( pImageForResult ) + PtCent.x + PtCent.y * iImageWidth  ;
  double dCentY = find_line_pos_ud(
    pCent , iRange_pix , 0.3 , 20 , dStemDia , iImageWidth ) ;

  if ( dCentY > iRange_pix )
    return 0.0;
  cmplx cCent( PtCent.x , PtCent.y + dCentY ) ;
  if ( !bScanToTheRight ) // if not necessary to scan to right side
    return cCent ;     //simple return dstart point
  cPlusPt = cCent + cmplx( 0. , -dStemDia * 0.5 ) ;
  cMinusPt = cCent + cmplx( 0. , dStemDia * 0.5 ) ;
  if ( pOut && ( m_iDebugView > 5 ) )
    pOut->AddFrame( CreateLineFrameEx( cPlusPt , cMinusPt , 0xff00ff ) ) ;

  double dNewStemDia = dStemDia ;
  int iNOmitted = 0 , iNWide = 0 ;
  PtCent.y += ROUND( dCentY ) ;
  CmplxVector Centers ;
  cmplx cLastCenter = cCent ;
  Centers.push_back( cLastCenter ) ;
  while ( dNewStemDia < dStemDia * 2 )
  {
    iRange_pix = ROUND( dStemDia * 3. );
    PtCent.x += 10 ; // ~7mm
    cCent._Val[ _RE ] = PtCent.x ;
    if ( PtCent.x >= dXLim )
      return cLastCenter ;
    LPBYTE pCent = GetData( pImageForResult ) + PtCent.x + PtCent.y * iImageWidth  ;
    dCentY = find_line_pos_ud(
      pCent , iRange_pix , 0.3 , 20 , dNewStemDia , iImageWidth ) ;
//     double dDiaByContur_mm =  * m_dScale_mm_per_pix ;
//     bool bByConturOK = ( dDiaByContur_mm < dInitialDiaByContur_mm * 2 ) ;

    if ( dCentY < iRange_pix ) // OK, we found center
    {
      if ( dNewStemDia < dStemDia * 1.1 )
      {
        dStemDia = dNewStemDia ;
        cCent._Val[_IM] = PtCent.y + dCentY ;
        PtCent.y = ROUND( cCent.imag() ) ;
        cLastCenter = cCent ;
        Centers.push_back( cLastCenter ) ;
        cPlusPt = cCent + cmplx( 0. , -dStemDia * 0.5 ) ;
        cMinusPt = cCent + cmplx( 0. , dStemDia * 0.5 ) ;
        if ( pOut && ( m_iDebugView > 7 ) )
        {
          pOut->AddFrame( CreateLineFrameEx( cPlusPt , cMinusPt , 0xff00ff ) ) ;
          pOut->AddFrame( CreatePtFrameEx( cCent , (DWORD) 0 ) ) ;
        }
        iNWide = iNOmitted = 0 ;
        continue ; // go to check on the right
      }
    }
    else if ( ++iNWide >= 2 ) 
      break ;
    dCentY = 0. ;
    if ( ++iNOmitted > 10 )
      break ;
  } ;
  if ( Centers.size() )
  {
    double dLastX = Centers.back().real() ;
    CmplxVector::reverse_iterator Last = Centers.rbegin() ;
    for ( CmplxVector::reverse_iterator rit = Centers.rbegin() + 1 ; 
      rit != Centers.rend() ; rit++ ) 
    {
      int iNSlices = (int)(rit - Last) ;
      double dDistX = dLastX - rit->real() ;
      if ( ( ROUND( dDistX ) / SliceStep ) == iNSlices )
      {
        if ( dDistX >= SliceStep * m_iNBackCheckedOnRightScan )
        {
          cmplx rValue = *rit ;
          return rValue + cmplx( SliceStep * (m_iNBackCheckedOnRightScan - 3) , 0. ) ;
        }
      }
      else
      {
        dLastX = rit->real() ;
        Last = rit ;
      }
    }
  }
  return 0.0 ;
}

cmplx Tecto::FindFirstPtForMeasurement(
  CVideoFrame * pImageForResult , CFigureFrame * pMainContur , 
  int iLeftPtIndex , double dXBegin , double dXLim , double& dInitialDia_pix ,
  CContainerFrame * pOut )
{
  if ( !pMainContur || !pImageForResult )
    return cmplx( 0. , 0. ) ;
// find slices to plus and to minus from left point on contour
  cmplx cPlusPt , cMinusPt ;
  int iIndexToPlus = iLeftPtIndex , iIndexToMinus = iLeftPtIndex ;
  double dInitialDiaByContur_mm = 0 ;
  double dMinDiaByContur = DBL_MAX ;
  DWORD dwMinIndexByContur = -1 ;
  int iNAttepts = 0 ;
  do
  {
    cPlusPt = FindNearestToX( pMainContur , iIndexToPlus , dXBegin , true ) ;
    cMinusPt = FindNearestToX( pMainContur , iIndexToMinus , dXBegin , false ) ;
    if ( ( cPlusPt.real() > 0. ) && ( cMinusPt.real() > 0. ) )
    {
      dInitialDia_pix = abs( cPlusPt - cMinusPt ) ;
      dInitialDiaByContur_mm = dInitialDia_pix * m_dScale_mm_per_pix ;
      if ( (m_dStalkDiaMin_mm <= dInitialDiaByContur_mm)
        && ( dInitialDiaByContur_mm <= m_dStalkDiaMax_mm * 1.3 ) )
      {
        return ( cPlusPt + cMinusPt ) * 0.5 ;
      }
    }
    dXBegin += 1. ;
  } while ( dXBegin < dXLim ) ;// edges not found
  return 0. ;
}

cmplx Tecto::TrackLeft(
  CVideoFrame * pImageForResult , cmplx cInitPoint , 
  double dInitialDia_pix , CContainerFrame * pOut )
{
  if ( !pImageForResult )
    return 0. ;
  cmplx cPt( cInitPoint ) ;
  CPoint PtCent = GetCPoint( cPt ) ;
  int iImageWidth = GetWidth( pImageForResult ) ;
  int iImageHeight = GetHeight( pImageForResult ) ;
  int iRange_pix = ROUND( dInitialDia_pix * 2. ) ;
  double dNewDia_pix ;
  LPBYTE pCent = GetData( pImageForResult ) + PtCent.x + PtCent.y * iImageWidth  ;
  double dCentY = find_line_pos_ud(
    pCent , iRange_pix , 0.3 , 30 , dNewDia_pix , iImageWidth ) ;

  while ( dCentY < iRange_pix )
  {
    cPt += cmplx( -1. , dCentY ) ;
    PtCent = GetCPoint( cPt ) ;

    LPBYTE pCent = GetData( pImageForResult ) + PtCent.x + PtCent.y * iImageWidth  ;
    if ( ((PtCent.y - iRange_pix/2 - 1) < 0)
      || ( iImageHeight <= ( PtCent.y + iRange_pix + 1 ) ) 
      || (PtCent.x <= 0) )
    {
      break ;
    }
    dCentY = find_line_pos_ud(
      pCent , iRange_pix , 0.3 , 30 , dNewDia_pix , iImageWidth ) ;

  } ;
  return (cPt == cInitPoint) ? 0. : cPt ;
}
