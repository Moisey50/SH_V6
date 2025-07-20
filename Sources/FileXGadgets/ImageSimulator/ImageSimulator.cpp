// UserExampleGadget.h.h : Implementation of the UserExampleGadget class


#include "StdAfx.h"
#include "ImageSimulator.h"

#include <iostream>
#include <iomanip>
#include <map>
#include <chrono>
#include <random>
#include <cmath>
#include "Math/PlaneGeometry.h"
#include <math/Line2d.h>
#include <files/imgfiles.h>
#include <fxfc/FXRegistry.h>
#include <imageproc/statistics.h>
#include <gadgets/VideoFrame.h>


USER_FILTER_RUNTIME_GADGET(ImageSimulator, "Video.capture" );	//	Mandatory

static const char * pWorkingMode = "Unknown;DispensSide;DispensFront;Pin;";
static const char * pAllSubStages =
  "Inactive;PartLoaded;FirstCentering;FirstCenteringFinished;"
  "SideGrinding;SidePrepareToFinish;SideExtSizeOK;"
  "SecondCentering;FrontGrinding;FrontGrindingFinished;FrontWhite;FrontBlack;" ;
static const char * pAutoProc = "AutoOff;AutoOn;";

FXString  ImageSimulator::m_sPartName ;

__int64 ImageSimulator::m_GenerationStage = DPS_Inactive;
AutoProcessing ImageSimulator::m_AutoProc = AP_Off ;
double ImageSimulator::m_dStateChangeTime_ms = 0. ;


double	ImageSimulator::m_dExtTargetRadius_um ;
double  ImageSimulator::m_dExtCurrentRadius_um ;
double	ImageSimulator::m_dIntRadius_um ;
double	ImageSimulator::m_dIntCurrentRadius_um ;
double  ImageSimulator::m_dSideAngle_deg ;
double  ImageSimulator::m_dInternalAngle_deg ;
cmplx   ImageSimulator::m_cExtEccentricity_um ;
cmplx   ImageSimulator::m_cIntEccentricity_um ;
cmplx   ImageSimulator::m_cInitialExtEccentricity_um ;
cmplx   ImageSimulator::m_cInitialIntEccentricity_um ;

ImageSimulator::ImageSimulator()
{
  m_ImageSize_pix.cx = 1280 ;
  m_ImageSize_pix.cy = 1024 ;
  m_sImageSize_pix = _T( "1280,1024" ) ;
  m_cROI_pix = cmplx( m_ImageSize_pix.cx , m_ImageSize_pix.cy ) ;
  if ( m_pFrontBackground )
  {
    delete[] m_pFrontBackground ;
    m_pFrontBackground = NULL ;
  }
  m_pFrontBackground = new BYTE[ m_ImageSize_pix.cx * m_ImageSize_pix.cy ] ;
  CreateFrontImageBackground( m_pFrontBackground , m_ImageSize_pix ) ;
  m_cCenter_pix = m_cROI_pix * 0.5 ;
  m_dDepth_um = 400. ;
  m_dExtTargetRadius_um = 230. ;
  m_dSideAngle_deg = 35. ;
  m_dSideAngle_rad = m_dSideAngle_deg * M_PI / 360. ; // 35 degrees default angle
  m_dInternalAngle_deg = 23 ;
  m_dInternalAngle_rad = m_dInternalAngle_deg * M_PI / 360. ; // 23 degrees default angle
  m_dExtBaseRadius_um = m_dExtTargetRadius_um + m_dDepth_um * tan( m_dSideAngle_rad ) ;
  m_dIntRadius_um = 65. ;
  m_dSideScale_um_per_pix = 3.6 ;
  m_dFrontScale_um_per_pix = 0.913 ;
  m_cExtEccentricity_um = m_cInitialExtEccentricity_um = 0. ;
  m_cIntEccentricity_um = m_cInitialIntEccentricity_um = 0. ;
  m_sExtEccentricity_um = m_sIntEccentricity_um = _T( "0.,0." ) ;
  
  m_dTurnAngle_rad = 0. ;
  iHRCounter = 0;

  unsigned seed = (unsigned) (chrono::system_clock::now().time_since_epoch().count());
  default_random_engine generator( seed );
  std::normal_distribution<double> EdgeDistribution( EDGE_MEAN_INTENS , EDGE_INTENS_STDDEV );

  for ( int i = 0 ; i < sizeof(m_EdgePixels) ; i++ )
  {
    int iVal = (int)EdgeDistribution( generator ) ;
    if ( iVal < FRONT_INTENS + 20 )
      iVal = FRONT_INTENS + 20 ;
    else if ( iVal > 255 )
      iVal = 255 ;
    m_EdgePixels[ i ] = ( BYTE ) iVal ;
  }

  std::normal_distribution<double> FrontDistribution( FRONT_INTENS , FRONT_INTENS_STD_DEV );
  for ( int i = 0 ; i < sizeof( m_FrontPixels ) ; i++ )
  {
    int iVal = ( int ) FrontDistribution( generator ) ;
    if ( iVal < FRONT_INTENS - 30 )
      iVal = FRONT_INTENS + 30 ;
    else if ( iVal > 255 )
      iVal = 255 ;
    m_FrontPixels[ i ] = ( BYTE ) iVal ;
  }

  init();
}
ImageSimulator::~ImageSimulator()
{
  if ( m_pFrontBackground )
  {
    delete[] m_pFrontBackground ;
    m_pFrontBackground = NULL ;
  }
}

void ImageSimulator::ConfigParamChange( LPCTSTR pName , void* pObject , bool& bInvalidate , bool& bInitRescan )
{
  ImageSimulator * pGadget = ( ImageSimulator* ) pObject;
  if ( pGadget )
  {
    if ( !_tcsicmp( pName , _T( "WorkingMode" ) ) )
    {
      bInvalidate = true;
    }
    else if ( !_tcsicmp( pName , _T( "PartSelect" ) ) )
    {
      FXRegistry Reg( "TheFileX\\ImageSimulation" );
      bInvalidate = true ;
    }
    else if ( !_tcsicmp( pName , _T( "PartName" ) ) 
      || !_tcsicmp( pName , _T( "LoadPart" ) ) 
      || (!_tcsicmp( pName , _T( "SideSubMode" ) ) && (pGadget->m_GenerationStage == DPS_PartLoaded )) )
    {
      pGadget->SelectPartByName( pGadget->m_sPartName , true );
      pGadget->m_iLoadPart = 0 ;
      pGadget->m_dSideSwipePhase_rad = 0. ;
      bInvalidate = true;
    }
    else if ( !_tcsicmp( pName , _T( "ImageSize" ) ) )
    {
      CSize Size ;
      if ( 2 == sscanf( pGadget->m_sImageSize_pix , "%d,%d" ,
        &Size.cx , &Size.cy ) && ( Size.cx > 0 ) && ( Size.cy > 0 ) )
      {
        pGadget->m_ImageSize_pix = Size ;
        if ( pGadget->m_pFrontBackground )
        {
          delete[] pGadget->m_pFrontBackground ;
          pGadget->m_pFrontBackground = NULL ;
        }
        pGadget->m_pFrontBackground = new BYTE[ Size.cx * Size.cy ] ;
        pGadget->CreateFrontImageBackground( pGadget->m_pFrontBackground , pGadget->m_ImageSize_pix ) ;
      }
      pGadget->m_cROI_pix = cmplx( Size.cx , Size.cy ) ;
      pGadget->m_cCenter_pix = pGadget->m_cROI_pix / 2.0 ;
    }
    else if ( !_tcsicmp( pName , _T( "SideAngle_deg" ) ) )
      pGadget->m_dSideAngle_rad = pGadget->m_dSideAngle_deg * M_PI / 180. ;
    else if ( !_tcsicmp( pName , _T( "InternalAngle_deg" ) ) )
      pGadget->m_dInternalAngle_rad = pGadget->m_dInternalAngle_deg * M_PI / 180. ;
    else if ( !_tcsicmp( pName , _T( "ExtEccentricity_um" ) ) )
    {
      cmplx cVal ;
      if ( 2 == sscanf( pGadget->m_sExtEccentricity_um , "%lf,%lf" , &cVal._Val[_RE] , &cVal._Val[ _IM ] ) )
        pGadget->m_cInitialExtEccentricity_um = pGadget->m_cExtEccentricity_um = cVal ;
    }
    else if ( !_tcsicmp( pName , _T( "IntEccentricity_um" ) ) )
    {
      cmplx cVal ;
      if ( 2 == sscanf( pGadget->m_sIntEccentricity_um , "%lf,%lf" , &cVal._Val[ _RE ] , &cVal._Val[ _IM ] ) )
        pGadget->m_cInitialIntEccentricity_um = pGadget->m_cIntEccentricity_um = cVal ;
    }
    else if ( !_tcsicmp( pName , _T( "SavePart" ) ) )
    {
      pGadget->SavePart() ;
      pGadget->m_iSavePart = 0 ;
      bInvalidate = true;
    }
  }
}

CDataFrame* ImageSimulator::DoProcessing(const CDataFrame* pDataFrame) 
{
  if ( ( pDataFrame->GetDataType() == text) 
    && !_tcsicmp( pDataFrame->GetTextFrame()->GetString() , "StartProcess" ) )
  {
    if ( m_WorkingMode == ISIMWM_DispensSide )
    {
      m_GenerationStage = DPS_ProcessStart ;
      m_dStateChangeTime_ms = GetHRTickCount() ;
    }
    return NULL ;
  }
  CVideoFrame * pFrame = NULL ;

  pTVFrame pTV = makeNewY8Frame( m_ImageSize_pix.cx , m_ImageSize_pix.cy ) ;
  if ( pTV )
    pFrame = CVideoFrame::Create( pTV ) ; // pixels already settled to zero (black image)
  if ( !pFrame )
    return NULL ;

  CContainerFrame * pOut = CContainerFrame::Create() ;
  pOut->AddFrame( pFrame ) ;
  m_cViewPt = cmplx ( 10. , m_cROI_pix.imag() - 50. ) ;
  m_cProcessViewPt = cmplx ( 100. , 10. ) ;
  switch ( m_WorkingMode )
  {
    case ISIMWM_DispensFront:
    {
      SimulateFront( pFrame , pOut ) ;

    }
    break ;
    case ISIMWM_DispensSide:
    case ISIMWM_Pin:
      memset( pFrame->lpBMIH + 1 , 255 , GetImageSize( pFrame ) ) ; // do white image
      SimulateSide( pFrame , pOut ) ;
      break ;
  }
  m_dTurnAngleStep_rad = M_2PI / m_iNframesPerRevol ;
  m_dTurnAngle_rad += m_dTurnAngleStep_rad ;
  if ( m_dTurnAngle_rad >= M_2PI )
    m_dTurnAngle_rad = 0. ;

  return pOut ;
}

void ImageSimulator::AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame) 
{
  if ( pParamFrame )
  {
    CTextFrame * ParamText = pParamFrame->GetTextFrame( DEFAULT_LABEL );
    if ( !ParamText )
    {
      if ( ParamText->GetString().MakeLower().Find( "heart" ) > -1 )
      {
//        m_bHeart = TRUE;
      }
    }

    pParamFrame->Release( pParamFrame );
  }
};

void ImageSimulator::PropertiesRegistration() 
{
  ClearProperties() ;
  addProperty( SProperty::COMBO , _T( "WorkingMode" ) ,
    ( int * ) &m_WorkingMode , SProperty::Int , pWorkingMode );
  addProperty( SProperty::EDITBOX , "ImageSize" , &m_sImageSize_pix , SProperty::String ) ;
  SetChangeNotification( _T( "ImageSize" ) , ConfigParamChange , this );
  SetChangeNotification( _T( "WorkingMode" ) , ConfigParamChange , this );
  if ( (m_WorkingMode == ISIMWM_DispensSide) || (m_WorkingMode == ISIMWM_DispensFront) )
  {
    addProperty( SProperty::COMBO , _T( "SubMode" ) ,
      ( int * ) &m_GenerationStage , SProperty::Int , pAllSubStages );
    SetChangeNotification( "Submode" , ConfigParamChange , this );

    addProperty( SProperty::COMBO , _T( "Auto" ) ,
      ( int * ) &m_AutoProc , SProperty::Int , pAutoProc );
    FXRegistry Reg( "TheFileX\\ImageSimulation" );
    m_sPartName = Reg.GetRegiString( "Common" , "SelectedPart" , m_sPartName ) ;
    addProperty( SProperty::EDITBOX , _T( "SelectedPart" ) , &m_sPartName , SProperty::String );
    SelectPartByName( m_sPartName , false ) ;
    addProperty( SProperty::SPIN , _T( "LoadPart" ) , &m_iLoadPart , SProperty::Int , 0 , 1 );
    SetChangeNotification( _T( "LoadPart" ) , ConfigParamChange , this );
  }

  switch( m_WorkingMode )
  {
    case ISIMWM_DispensSide: 
    case ISIMWM_DispensFront:
    {
      addProperty( SProperty::EDITBOX , _T( "ExtRadius_um" ) , &m_dExtTargetRadius_um , SProperty::Double );
      addProperty( SProperty::EDITBOX , _T( "IntRadius_um" ) , &m_dIntRadius_um , SProperty::Double );
      addProperty( SProperty::EDITBOX , _T( "Depth_um" ) , &m_dDepth_um , SProperty::Double );
      addProperty( SProperty::EDITBOX , _T( "SideAngle_deg" ) , &m_dSideAngle_deg , SProperty::Double );
      addProperty( SProperty::EDITBOX , _T( "InternalAngle_deg" ) , &m_dInternalAngle_deg , SProperty::Double );
      addProperty( SProperty::EDITBOX , _T( "SideScale_um/pix" ) , &m_dSideScale_um_per_pix , SProperty::Double );
      addProperty( SProperty::EDITBOX , _T( "FrontScale_um/pix" ) , &m_dFrontScale_um_per_pix , SProperty::Double );
      addProperty( SProperty::EDITBOX , _T( "ExtEccentricity_um" ) , &m_sExtEccentricity_um , SProperty::String );
      addProperty( SProperty::EDITBOX , _T( "IntEccentricity_um" ) , &m_sIntEccentricity_um , SProperty::String );
      addProperty( SProperty::EDITBOX , _T( "SideSwipeUp_um" ) , &m_dSideStoneSwipeUp_um , SProperty::Double );
      addProperty( SProperty::EDITBOX , _T( "SideSwipeDown_um" ) , &m_dSideStoneSwipeDown_um , SProperty::Double );
      addProperty( SProperty::EDITBOX , _T( "SideStonePos_um" ) , &m_dSideStonePos_um , SProperty::Double );
      addProperty( SProperty::EDITBOX , _T( "SideSwipeTime_rev" ) , &m_dSideSwipeTime_turns , SProperty::Double );
      addProperty( SProperty::EDITBOX , _T( "SideStoneSpeed_um/rev" ) , &m_dSideStoneSpeed_um_per_turn , SProperty::Double );
      addProperty( SProperty::EDITBOX , _T( "FrontSwipeTime_rev" ) , &m_dFrontSwipeTime_turns , SProperty::Double );
      addProperty( SProperty::EDITBOX , _T( "FrontStoneSpeed_um/cyc" ) , &m_dFrontStoneSpeed_um_per_cycle , SProperty::Double );

      SetChangeNotification( _T( "ExtEccentricity_um" ) , ConfigParamChange , this );
      SetChangeNotification( _T( "IntEccentricity_um" ) , ConfigParamChange , this );
      SetChangeNotification( _T( "SideAngle_deg" ) , ConfigParamChange , this );
    }
    break ;
    case ISIMWM_Pin:
    {
      addProperty( SProperty::EDITBOX , _T( "PinDia_um" ) , &m_dPinDia_um , SProperty::Double );
      addProperty( SProperty::EDITBOX , _T( "HeadRadius_um" ) , &m_dHeadRadius_um , SProperty::Double );
      addProperty( SProperty::EDITBOX , _T( "PinLength_um" ) , &m_dPinLength_um , SProperty::Double );
      addProperty( SProperty::SPIN , _T( "BasementAngle_deg" ) , &m_iBasementAngle , SProperty::Int , 0 , 60 );
      addProperty( SProperty::EDITBOX , _T( "SideScale_um/pix" ) , &m_dSideScale_um_per_pix , SProperty::Double );
    }
    break ;
  }
  addProperty( SProperty::SPIN , _T( "SavePart" ) , &m_iSavePart , SProperty::Int , 0 , 1 );
  SetChangeNotification( _T( "SavePart" ) , ConfigParamChange , this );
};

void ImageSimulator::ConnectorsRegistration() 
{
  addInputConnector( transparent, "Sync");
  addOutputConnector( vframe , "ImageOutput");
  addDuplexConnector( transparent, transparent, "Control");
};

int ImageSimulator::SelectPartByName( FXString& SelectedPartName , bool bReportToEngine )
{
  FXRegistry Reg( _T( "TheFileX\\ImageSimulation" ) );
  FXString PartName = SelectedPartName ;
  PartName.MakeUpper() ;

  switch( m_WorkingMode )
  {
    case ISIMWM_DispensSide:
    case ISIMWM_DispensFront:
    {
      if ( SelectedPartName.IsEmpty() )
        PartName = _T( "Dispenser" ) ;

      double dConeAngle_deg = Reg.GetRegiDouble( PartName , "ConeAngle_deg" , 35.0 );
      m_dSideAngle_rad = DegToRad( dConeAngle_deg ) / 2. ;
      m_dDepth_um = Reg.GetRegiDouble( PartName , "Depth_um" , m_dDepth_um );
      m_dExtTargetRadius_um = Reg.GetRegiDouble( PartName , "ExtRadius_um" , m_dExtTargetRadius_um );
      m_dIntRadius_um = Reg.GetRegiDouble( PartName , "IntRadius_um" , m_dIntRadius_um );
      m_dIntCurrentRadius_um = Reg.GetRegiDouble( PartName , "IntInitRadius_um" , m_dIntRadius_um + 15 );
      Reg.GetRegiCmplx( PartName , "IntEccentricity_um" , m_cIntEccentricity_um , cmplx( 25. , 5. ) ) ;
      Reg.GetRegiCmplx( PartName , "ExtEccentricity_um" , m_cExtEccentricity_um , cmplx( 10. , 5. ) ) ;
      m_dExtBaseRadius_um = m_dExtTargetRadius_um + m_dDepth_um * tan( m_dSideAngle_rad ) ;
      m_dSideScale_um_per_pix = Reg.GetRegiDouble( PartName , "SideScale_um/pix" , 3.6 );
      m_dFrontScale_um_per_pix = Reg.GetRegiDouble( PartName , "FrontScale_um/pix" , 0.913 );
      m_dSideStoneSwipeUp_um = Reg.GetRegiDouble( PartName , "SideSwipeUp_um" , -6000. ) ;
      m_dSideStoneSwipeDown_um = Reg.GetRegiDouble( PartName , "SideSwipeDown_um" , 1000. ) ;
      m_dSideStonePos_um = Reg.GetRegiDouble( PartName , "SideStonePos_um" , 3000. ) ;
      m_dSideStoneSpeed_um_per_turn = Reg.GetRegiDouble( PartName , "SideStoneSpeed_um/turn" , 20. ) ;
      m_dSideSwipeTime_turns = Reg.GetRegiDouble( PartName , "SideSwipeTime_turns" , 5.7 ) ;
      m_dFrontStoneSpeed_um_per_cycle = Reg.GetRegiDouble( PartName , "FrontStoneSpeed_um/cycle" , 1. ) ;
      m_dFrontSwipeTime_turns = Reg.GetRegiDouble( PartName , "FrontSwipeTime_turns" , 7.3 ) ;
      m_dExtCurrentRadius_um = m_dExtBaseRadius_um / 2. ;
      m_GenerationStage = DPS_PartLoaded ;
    }
    break ;
    case ISIMWM_Pin:
    {
      PartName = "Pin" ;
      m_dPinDia_um = Reg.GetRegiDouble( PartName , "PinDia_um" , 300 );
      m_dHeadRadius_um = Reg.GetRegiDouble( PartName , "HeadRadius_um" , 1000. );
      m_dPinLength_um = Reg.GetRegiDouble( PartName , "PinLength_um" , 1000. );
      m_iBasementAngle = Reg.GetRegiInt( PartName , "BasementAngle_deg" , 90 );
      m_dSideScale_um_per_pix = Reg.GetRegiDouble( PartName , "SideScale_um/pix" , 2.1 );

    }
    break ;
  }

  m_sPartName = SelectedPartName ;
  return 1 ;
}

int ImageSimulator::SavePart()
{
  FXString PartName = m_sPartName ;
  PartName.MakeUpper() ;
  FXRegistry Reg( _T( "TheFileX\\ImageSimulation" ) );
  switch ( m_WorkingMode )
  {
    case ISIMWM_DispensSide:
    case ISIMWM_DispensFront:
    {
      Reg.WriteRegiString( "Common" , "SelectedPart" , m_sPartName ) ;
      if ( PartName.IsEmpty() )
        PartName = _T( "Dispenser" ) ;

      Reg.WriteRegiDouble( PartName , "SideAngle_deg" , RadToDeg (m_dSideAngle_rad) * 2. );
      Reg.WriteRegiDouble( PartName , "Depth_um" , m_dDepth_um );
      Reg.WriteRegiDouble( PartName , "ExtRadius_um" , m_dExtTargetRadius_um );
      Reg.WriteRegiDouble( PartName , "IntRadius_um" , m_dIntRadius_um );
      Reg.WriteRegiDouble( PartName , "SideScale_um/pix" , 3.6 );
      Reg.WriteRegiDouble( PartName , "FrontScale_um/pix" , 0.913 );
//       Reg.WriteRegiCmplx( PartName , "ExtEccentricity_um" , m_cExtEccentricity_um );
//       Reg.WriteRegiCmplx( PartName , "IntEccentricity_um" , m_cIntEccentricity_um );
      Reg.WriteRegiDouble( PartName , "SideSwipeUp_um" , m_dSideStoneSwipeUp_um ) ;
      Reg.WriteRegiDouble( PartName , "SideSwipeDown_um" , m_dSideStoneSwipeDown_um ) ;
      //Reg.WriteRegiDouble( PartName , "SideStoneSpeed_um/turn" , m_dSideStoneSpeed_um_per_turn ) ;
      Reg.WriteRegiDouble( PartName , "SideSwipeTime_turns" , m_dSideSwipeTime_turns ) ;
      Reg.WriteRegiDouble( PartName , "FrontStoneSpeed_um/cycle" , m_dFrontStoneSpeed_um_per_cycle ) ;
      Reg.WriteRegiDouble( PartName , "FrontSwipeTime_turns" , m_dFrontSwipeTime_turns ) ;
    }
    return 1 ;
    case ISIMWM_Pin:
    {
     PartName = "Pin" ;
     Reg.WriteRegiDouble( PartName , "PinDia_um" , m_dPinDia_um );
     Reg.WriteRegiDouble( PartName , "HeadRadius_um" , m_dPinLength_um );
     Reg.WriteRegiDouble( PartName , "PinLength_um" , m_dPinLength_um );
     Reg.WriteRegiDouble( PartName , "SideScale_um/pix" , m_dSideScale_um_per_pix );

    }
    return 1 ;
  }
  return 0 ;
}

int ImageSimulator::SimulateFront( CVideoFrame * pFrame , CContainerFrame * pOut )
{
  switch ( m_WorkingMode )
  {
    case ISIMWM_DispensFront:
    case ISIMWM_DispensSide:
      return SimulateDispenserFront( pFrame , pOut ) ;
    case ISIMWM_Pin: 
      return 1 ;
  }
  return 0 ;
}

int ImageSimulator::SimulateDispenserFront( CVideoFrame * pFrame , CContainerFrame * pOut )
{
  switch ( m_GenerationStage )
  {
    case DPS_FrontBlackMeasurement:
    case DPS_FrontGrindingFinished:
      memcpy( GetData( pFrame ) , m_pFrontBackground , m_ImageSize_pix.cx * m_ImageSize_pix.cy ) ;
      break ;
    case DPS_FirstCentering :
    case DPS_SecondCentering :
      break ; // simple white circle on black background
    default: 
      return 0 ; // simple black image
  }

  cmplx cIntEccentrityAddition_um = m_cIntEccentricity_um * polar( 1. , m_dTurnAngle_rad ) ;
  cmplx cAddition_pix = cIntEccentrityAddition_um / m_dFrontScale_um_per_pix ;
  cmplx cCent_pix( m_cCenter_pix + cAddition_pix ) ;
  cmplx cExtEccentrityAddition_um = m_cExtEccentricity_um * polar( 1. , m_dTurnAngle_rad ) ;
  cmplx cExtAddition_pix = cExtEccentrityAddition_um / m_dFrontScale_um_per_pix ;
  cmplx cExtCent_pix( m_cCenter_pix + cExtAddition_pix ) ;

  switch( m_GenerationStage )
  {
    case DPS_PartLoaded:
      Sleep( 50 ) ;
    case DPS_FirstCentering:
    {
      int iNSeconds = ( int ) ( (GetHRTickCount() - m_dStateChangeTime_ms) / 1000. ) ;
      if ( abs( m_cIntEccentricity_um ) > 0.3 )
      {
        if ( ( iNSeconds > 3 ) )
        {
          cmplx cEccentricity_um = m_cIntEccentricity_um ;
          m_cIntEccentricity_um *= 0.95 ;
          cmplx cEccDiff_um = m_cIntEccentricity_um - cEccentricity_um ;
          m_cExtEccentricity_um += cEccDiff_um ;
        }
      }
      else
      {
        m_GenerationStage = DPS_FirstCenteringFinished ;
      }
      DrawCircle( pFrame , cExtCent_pix , m_dExtCurrentRadius_um / m_dFrontScale_um_per_pix ,
        70 , m_pCurrentPixel , m_pCurrentFrontPix ) ;
      DrawCircle( pFrame , cCent_pix , m_dIntCurrentRadius_um / m_dFrontScale_um_per_pix , 180 ) ;
    }
    break ;
    case DPS_FrontBlackMeasurement:
    {
      cmplx cExtEccentrityAddition_um = m_cExtEccentricity_um * polar( 1. , m_dTurnAngle_rad ) ;
      cmplx cExtAddition_pix = cExtEccentrityAddition_um / m_dFrontScale_um_per_pix ;
      cmplx cExtCent_pix( m_cCenter_pix + cExtAddition_pix ) ;
      DrawCircle( pFrame , cExtCent_pix , m_dExtCurrentRadius_um / m_dFrontScale_um_per_pix , 
        70 , m_pCurrentPixel , m_pCurrentFrontPix  ) ;
      DrawCircle( pFrame , cCent_pix , m_dIntCurrentRadius_um / m_dFrontScale_um_per_pix , 
        20 , m_pCurrentPixel ) ;

      if ( GetHRTickCount() - m_dStateChangeTime_ms > 2000 )
      {
        m_dExtCurrentRadius_um += m_dFrontStoneSpeed_um_per_cycle * tan( m_dSideAngle_rad ) ;
        m_dIntCurrentRadius_um -= m_dFrontStoneSpeed_um_per_cycle * tan( m_dInternalAngle_rad ) ;
        if ( m_dIntCurrentRadius_um - m_dIntRadius_um < 0. )
          m_GenerationStage = DPS_FrontGrindingFinished ;
        else
          m_GenerationStage = DPS_FrontGrinding ;
      }
    }
    break ;
    case DPS_FirstCenteringFinished:
    case DPS_FrontWhiteMeasurement:
    case DPS_SecondCentering:
    {
      DrawCircle( pFrame , cCent_pix , m_dIntCurrentRadius_um / m_dFrontScale_um_per_pix , 180 ) ;
    }
    break ;
    case DPS_FrontGrindingFinished:
    {
      cmplx cExtEccentrityAddition_um = m_cExtEccentricity_um * polar( 1. , m_dTurnAngle_rad ) ;
      cmplx cExtAddition_pix = cExtEccentrityAddition_um / m_dFrontScale_um_per_pix ;
      cmplx cExtCent_pix( m_cCenter_pix + cExtAddition_pix ) ;
      DrawCircle( pFrame , cExtCent_pix , m_dExtCurrentRadius_um / m_dFrontScale_um_per_pix ,
        70 , m_pCurrentPixel , m_pCurrentFrontPix ) ;
      DrawCircle( pFrame , cCent_pix , m_dIntCurrentRadius_um / m_dFrontScale_um_per_pix ,
        20 , m_pCurrentPixel ) ;
    }
    break ;
  }
  switch ( m_GenerationStage )
  {
    case DPS_ProcessStart:
    {
      CTextFrame * pTextOperation = CreateTextFrameEx( m_cProcessViewPt , 0x00ffff , 22 ,
        "Process Start" ) ;
      pTextOperation->Attributes()->WriteInt( "back" , 0 ) ;
      pOut->AddFrame( pTextOperation ) ;
    }
    break ;
    case DPS_PartLoaded:
    {
      CTextFrame * pTextOperation = CreateTextFrameEx( m_cViewPt , 0x00ffff , 18 ,
        "Part %s Loaded" , ( LPCTSTR ) m_sPartName ) ;
      pTextOperation->Attributes()->WriteInt( "back" , 0 ) ;
      pOut->AddFrame( pTextOperation ) ;
    }
    break ;
    case DPS_FirstCentering:
    case DPS_FirstCenteringFinished:
    {
      CTextFrame * pTextOperation = CreateTextFrameEx( m_cProcessViewPt , 0x00ffff , 22 ,
        "Centering %s" , ( LPCTSTR ) m_sPartName ,
        m_GenerationStage == DPS_FirstCenteringFinished ? " Finished" : " Process" ) ;
      pTextOperation->Attributes()->WriteInt( "back" , 0 ) ;
      pOut->AddFrame( pTextOperation ) ;
      CTextFrame * pTextResult = CreateTextFrameEx( m_cViewPt , 0x00ffff , 18 ,
        "Part %s    Eccentricity %.2fum" ,
        ( LPCTSTR ) m_sPartName , abs( m_cIntEccentricity_um ) ) ;
      pTextResult->Attributes()->WriteInt( "back" , 0 ) ;
      pOut->AddFrame( pTextResult ) ;
    }
    break ;
    case DPS_SideGrinding:
    case DPS_SidePrepareToFinish:
    case DPS_SideExtSizeOK:
    {
      CTextFrame * pTextOperation = CreateTextFrameEx( m_cProcessViewPt , 0x00ffff , 22 ,
        "Side grinding" ) ;
      pTextOperation->Attributes()->WriteInt( "back" , 0 ) ;
      pOut->AddFrame( pTextOperation ) ;
      CTextFrame * pTextResult = CreateTextFrameEx( m_cViewPt , 0x00ffff , 18 ,
        "Part %s   Camera is inactive" , ( LPCTSTR ) m_sPartName ) ;
      pTextResult->Attributes()->WriteInt( "back" , 0 ) ;
      pOut->AddFrame( pTextResult ) ;
    }
    break ;

    case DPS_FrontGrinding:
    {
      CTextFrame * pTextOperation = CreateTextFrameEx( m_cProcessViewPt , 0x00ffff , 22 ,
        "Front grinding" ) ;
      pTextOperation->Attributes()->WriteInt( "back" , 0 ) ;
      pOut->AddFrame( pTextOperation ) ;
      CTextFrame * pTextResult = CreateTextFrameEx( m_cViewPt , 0x00ffff , 18 ,
        "Part %s   Camera is inactive" , ( LPCTSTR ) m_sPartName ) ;
      pTextResult->Attributes()->WriteInt( "back" , 0 ) ;
      pOut->AddFrame( pTextResult ) ;
    }
    break ;
    case DPS_FrontGrindingFinished:
    {
      CTextFrame * pTextOperation = CreateTextFrameEx( m_cProcessViewPt , 0x00ff00 , 22 ,
        "Process Finished" ) ;
      pTextOperation->Attributes()->WriteInt( "back" , 0 ) ;
      pOut->AddFrame( pTextOperation ) ;
      CTextFrame * pTextResult = CreateTextFrameEx( m_cViewPt , 0x00ff00 , 18 ,
        "Part %s    Int Dia = %.2fum   Ext Dia = %.2fum " ,
        ( LPCTSTR ) m_sPartName ,
        m_dIntCurrentRadius_um * 2 + ( ( double ) rand() / RAND_MAX ) - 0.5 ,
        ( m_dExtCurrentRadius_um > GetWidth( pFrame ) ) ? 0. : // No ext circle
        m_dExtCurrentRadius_um * 2. + ( 3. * ( double ) rand() / RAND_MAX ) - 1.5 ) ;
      pTextResult->Attributes()->WriteInt( "back" , 0 ) ;
      pOut->AddFrame( pTextResult ) ;
    }
    break ;
    case DPS_FrontBlackMeasurement:
    {
      CTextFrame * pTextOperation = CreateTextFrameEx( m_cProcessViewPt , 0x00ffff , 22 ,
        "Tip Measurement" ) ;
      pTextOperation->Attributes()->WriteInt( "back" , 0 ) ;
      pOut->AddFrame( pTextOperation ) ;
      CTextFrame * pTextResult = CreateTextFrameEx( m_cViewPt , 0x00ffff , 18 ,
        "Part %s    Int. Dia = %.2fum   Ext. Dia = %.2fum " ,
        ( LPCTSTR ) m_sPartName ,
        m_dIntCurrentRadius_um * 2 + ( ( double ) rand() / RAND_MAX ) - 0.5 ,
        ( m_dExtCurrentRadius_um > GetWidth( pFrame ) ) ? 0. : // No ext circle
        m_dExtCurrentRadius_um * 2. + ( 3. * ( double ) rand() / RAND_MAX ) - 1.5 ) ;
      pTextResult->Attributes()->WriteInt( "back" , 0 ) ;
      pOut->AddFrame( pTextResult ) ;
    }

  }

  return 0;
}

int ImageSimulator::SimulateSide( CVideoFrame * pFrame , CContainerFrame * pOut )
{
  switch ( m_WorkingMode )
  {
    case ISIMWM_DispensFront:
    case ISIMWM_DispensSide:
      return SimulateDispenserSide( pFrame , pOut ) ;
    case ISIMWM_Pin:
      return SimulatePinSide( pFrame , pOut ) ;

  }
  return 0 ;

}

int ImageSimulator::SimulateDispenserSide( CVideoFrame * pFrame , CContainerFrame * pOut )
{
  cmplx cExtEccentrityAddition_um = m_cExtEccentricity_um * polar( 1. , m_dTurnAngle_rad ) ;
  cmplx cAddition_um = cExtEccentrityAddition_um + m_cConstantShift_um ;

  cmplx cConstantShift_pix = m_cConstantShift_um / m_dSideScale_um_per_pix ;

  cmplx cAddition_pix = cAddition_um / m_dSideScale_um_per_pix ;
  double dDepth_pix = m_dDepth_um / m_dSideScale_um_per_pix ;
  int iNFullPix = ( int ) dDepth_pix ;
  cmplx cOrigin_pix = cmplx( dDepth_pix , m_cCenter_pix.imag() + cAddition_pix.imag() )  ;
  
  cmplx cBasePt_pix( dDepth_pix + cConstantShift_pix.real() , m_cCenter_pix.imag() + cConstantShift_pix.imag() ) ;
  cmplx cOrthoToSideStone_pix = polar( m_dSideStonePos_um / m_dSideScale_um_per_pix , m_dSideAngle_rad - M_PI2 )  ; // Y is going down
  cmplx cSideStonePlanePt_pix = cBasePt_pix + cOrthoToSideStone_pix ;
  cmplx cNormOrthoToSide = GetNormalized( cOrthoToSideStone_pix ) ;
  cmplx cSideStonePlaneDir = GetOrthoRightOnVF( cNormOrthoToSide ) ;
  CLine2d SideStoneEdge ;
  SideStoneEdge.ByPointAndDir( cSideStonePlanePt_pix , cSideStonePlaneDir ) ;
  double dDepthCrossPt = SideStoneEdge.GetY( dDepth_pix ) ;
  double dSideSwipeAmpl_pix = m_dSideStoneSwipeDown_um - m_dSideStoneSwipeUp_um ;
  cmplx cSideStonePlaneOrigin_pix = cSideStonePlanePt_pix
    + ( cSideStonePlaneDir * m_dSideStoneSwipeUp_um / m_dSideScale_um_per_pix ) ; // upper stone pt
  double dCurrSideStoneSwipePos_um = dSideSwipeAmpl_pix * ( 1. - cos( m_dSideSwipePhase_rad ) ) / 2. ;
    // prepare to next cycle
  m_dSideSwipePhaseStep_rad = M_2PI / ( m_dSideSwipeTime_turns * m_iNframesPerRevol ) ;
  m_dSideSwipePhase_rad = fmod( m_dSideSwipePhase_rad + m_dSideSwipePhaseStep_rad , M_2PI ) ;
  cmplx cSideStoneLowerEdge_pix = cSideStonePlaneOrigin_pix + cSideStonePlaneDir * dCurrSideStoneSwipePos_um / m_dSideScale_um_per_pix ;

  if ( ( int ) m_GenerationStage > ( int ) DPS_SidePrepareToFinish )
    m_dSideSwipePhase_rad = 0. ;


  switch ( m_GenerationStage )
  {
    case DPS_PartLoaded:
      m_GenerationStage = DPS_FirstCentering ;
      m_dStateChangeTime_ms = GetHRTickCount() ;
    case DPS_FirstCentering:
      DrawFullBlank( pFrame ) ;
      m_dSideSwipePhase_rad = 0. ;
      break ;
    case DPS_FirstCenteringFinished:
    {
      DrawFullBlank( pFrame ) ;
      if ( cSideStoneLowerEdge_pix.imag() < 0. ) // necessary to draw stone (it is in FOV)
      {
        m_dSideStonePos_um -= m_dSideStoneSpeed_um_per_turn / m_iNframesPerRevol ;
        break ;
      }
      else
      {
        m_GenerationStage = DPS_SideGrinding ;
//         m_dExtCurrentRadius_um = (cOrigin_pix.imag() - cSideStoneLowerEdge_pix.imag()) * m_dSideScale_um_per_pix ;
      }
    }
    // No break, processing continues
    case DPS_SideGrinding:
    case DPS_SidePrepareToFinish:  
    {
      if ( (cSideStoneLowerEdge_pix.imag() < 0.) || (cSideStoneLowerEdge_pix.real() < -100.) )
      {
        if ( m_GenerationStage == DPS_SidePrepareToFinish )
        {
          m_GenerationStage = DPS_SideExtSizeOK ;
          m_dLastFrontGrindingBeginTime_ms = 0. ; // Sign about when side grinding finished
        }
      }
      else if ( cSideStoneLowerEdge_pix.real() >= 0. )
      {
        m_cExtEccentricity_um = cmplx( 1. , 0. ) ;
        int iX = ROUND( cSideStoneLowerEdge_pix.real() ) ;
        cmplx cCurrentLowPt_pix( cSideStoneLowerEdge_pix ) ;
        // Draw to the left from lower point
        double dSlope = fabs( tan(arg( cSideStonePlaneDir )) ) ;
        LPBYTE pEnd = GetData( pFrame ) ;
        do
        {
          LPBYTE p = GetData( pFrame ) + iX + ( ROUND( cCurrentLowPt_pix.imag() ) * GetWidth( pFrame ) ) ;
          while ( p > pEnd )
          {
            *p = 0 ;
            p -= GetWidth( pFrame ) ;
          }
        } while ( ( ( cCurrentLowPt_pix._Val[ _IM ] -= dSlope ) >= 0 ) && ( --iX >= 0 ) );

        // Left part of stone is drawn
        // Now draw right part - on the right side of lower point
        iX = ROUND( cSideStoneLowerEdge_pix.real() ) ;
        cCurrentLowPt_pix = cSideStoneLowerEdge_pix ;
        dSlope = fabs( tan( arg( cNormOrthoToSide) ) ) ;
        while ( ( ( cCurrentLowPt_pix._Val[ _IM ] -= dSlope ) >= 0 ) && ( ++iX < ( int ) GetWidth( pFrame ) ) )
        {
          LPBYTE p = GetData( pFrame ) + iX + ROUND( cCurrentLowPt_pix.imag() ) * GetWidth( pFrame );
          while ( p > pEnd ) // Draw black vertical line to up side
          {
            *p = 0 ;
            p -= GetWidth( pFrame ) ;
          }
        }
      }
      if ( (dDepthCrossPt > 0.) && (cSideStoneLowerEdge_pix.real() >= dDepthCrossPt) )
      {
        m_dExtCurrentRadius_um = ( cOrigin_pix.imag() - dDepthCrossPt ) * m_dSideScale_um_per_pix ;
        if ( m_dSideStoneSpeed_um_per_turn > 5 )
        {
          if ( ( m_dExtCurrentRadius_um - m_dExtTargetRadius_um ) < m_dSideStoneSpeed_um_per_turn * 3. )
            m_dSideStoneSpeed_um_per_turn /= 2. ;
        }
        if ( m_dExtCurrentRadius_um < m_dExtTargetRadius_um )
        {
          m_dSideStoneSpeed_um_per_turn = 0. ;
          m_GenerationStage = DPS_SidePrepareToFinish ;
        }
        else if ( m_dSideStoneSpeed_um_per_turn == 0. )
          m_dSideStoneSpeed_um_per_turn = 1. ;
      }
    }
    m_dSideStonePos_um -= m_dSideStoneSpeed_um_per_turn / m_iNframesPerRevol ;
    if ( m_GenerationStage != DPS_SideExtSizeOK )
    {
      if ( m_dExtCurrentRadius_um < cOrigin_pix.imag() * m_dSideScale_um_per_pix )
        DrawSideConus( pFrame , cOrigin_pix ) ;
      else
        DrawFullBlank( pFrame ) ;
      break ;
    }
    else
      m_dLastSwithToSideFinishedTime_ms = GetHRTickCount() ;
    // State is DPS_SideExtSizeOK - go directly to result view
    case DPS_SideExtSizeOK:
    {
      DrawSideConus( pFrame , cOrigin_pix ) ;
      if ( ( m_AutoProc == AP_On ) && ( GetHRTickCount() > m_dLastSwithToSideFinishedTime_ms + 3000. ) )
      {
        m_GenerationStage = DPS_FrontGrinding ;
        m_iNFrontGrindingCycles = 0 ;
        m_dFrontSwipePhase_rad = 0. ;
      }
    }
    break ;
    case DPS_FrontGrinding:
    {
      DrawSideConus( pFrame , cOrigin_pix ) ;
      m_dFrontSwipePhaseStep_rad = M_2PI / ( m_dFrontSwipeTime_turns * m_iNframesPerRevol ) ;
      double dFrontSwipeAmpl_pix = 2.5 * GetHeight( pFrame ) ;
      double dCurrFrontStoneSwipePos_um = dFrontSwipeAmpl_pix * ( 1. - cos( m_dFrontSwipePhase_rad ) ) / 2. ;
      m_dFrontSwipePhase_rad += m_dFrontSwipePhaseStep_rad ;
      if ( m_dFrontSwipePhase_rad >= M_2PI )
      {
        m_dFrontSwipePhase_rad = 0. ;
        m_dDepth_um -= m_dFrontStoneSpeed_um_per_cycle ;
        m_GenerationStage = DPS_FrontBlackMeasurement ;
        m_dStateChangeTime_ms = GetHRTickCount() ;
      }

      cmplx cFrontStoneUpperEdge_pix( dDepth_pix + 0.5 , GetHeight( pFrame ) * 2. - dCurrFrontStoneSwipePos_um ) ;

      if ( cFrontStoneUpperEdge_pix.imag() < GetHeight( pFrame ) )
      {
        int iUpperY = ( cFrontStoneUpperEdge_pix.imag() < 0. ) ?
          0 : ROUND( cFrontStoneUpperEdge_pix.imag() ) ;
        DWORD dwXBegin = ROUND( cFrontStoneUpperEdge_pix.real() ) ;
        LPBYTE p = GetData( pFrame ) + dwXBegin 
          + iUpperY * (int)GetWidth( pFrame ) ;
        LPBYTE pEnd = GetData( pFrame ) + GetHeight( pFrame ) * GetWidth( pFrame ) ;
        DWORD dwBlackWidth = GetWidth( pFrame ) - dwXBegin ;
        while ( p < pEnd )
        {
          memset( p , 0 , dwBlackWidth ) ;
          p += GetWidth( pFrame ) ;
        }
      }
    }
    break ;
    case DPS_FrontBlackMeasurement:
    case DPS_FrontGrindingFinished:
    {
      DrawSideConus( pFrame , cOrigin_pix ) ;
      m_dFrontSwipePhase_rad = 0. ;
    }
    break ;
  }
  switch ( m_GenerationStage )
  {
    case DPS_ProcessStart:
    {
      CTextFrame * pTextResult = CreateTextFrameEx( m_cProcessViewPt , 0x00ffff , 22 ,
        "Process Start" , ( LPCTSTR ) m_sPartName ) ;
      pTextResult->Attributes()->WriteInt( "back" , 0 ) ;
      pOut->AddFrame( pTextResult ) ;
      if ( ( GetHRTickCount() - m_dStateChangeTime_ms ) > 2000. )
      {
        SelectPartByName( m_sPartName , false ) ;
        m_GenerationStage = DPS_PartLoaded ;
        m_dStateChangeTime_ms = GetHRTickCount() ;
      }
    }
    break ;
    case DPS_PartLoaded:
    {
      CTextFrame * pTextOperation = CreateTextFrameEx( m_cProcessViewPt , 0x00ffff , 22 ,
        "Part Loaded" ) ;
      pTextOperation->Attributes()->WriteInt( "back" , 0 ) ;
      pOut->AddFrame( pTextOperation ) ;
      CTextFrame * pTextResult = CreateTextFrameEx( m_cViewPt , 0x00ffff , 18 ,
        "Part %s Loaded" , ( LPCTSTR ) m_sPartName ) ;
      pTextResult->Attributes()->WriteInt( "back" , 0 ) ;
      pOut->AddFrame( pTextResult ) ;
      if ( ( GetHRTickCount() - m_dStateChangeTime_ms ) > 2000. )
      {
        m_GenerationStage = DPS_FirstCentering ;
        m_dStateChangeTime_ms = GetHRTickCount() ;
      }
    }
    break ;
    case DPS_FirstCentering:
    case DPS_FirstCenteringFinished:
    {
      CTextFrame * pTextOperation = CreateTextFrameEx( m_cProcessViewPt , 0x00ffff , 22 ,
        "Centering" ,
        m_GenerationStage == DPS_FirstCenteringFinished ? " Finished" : " Process" ) ;
      pTextOperation->Attributes()->WriteInt( "back" , 0 ) ;
      pOut->AddFrame( pTextOperation ) ;
      CTextFrame * pTextResult = CreateTextFrameEx( m_cViewPt , 0x00ffff , 18 ,
        "Part %s " , ( LPCTSTR ) m_sPartName ) ;
      pTextResult->Attributes()->WriteInt( "back" , 0 ) ;
      pOut->AddFrame( pTextResult ) ;
    }
    break ;
    case DPS_SideGrinding:
    case DPS_SidePrepareToFinish:
    case DPS_SideExtSizeOK:
    {
      CTextFrame * pTextOperation = CreateTextFrameEx( m_cProcessViewPt , 0x00ffff , 22 ,
        "Side Grinding" ,
        m_GenerationStage == DPS_SideExtSizeOK ? " Finished" : " Process" ) ;
      pTextOperation->Attributes()->WriteInt( "back" , 0 ) ;
      pOut->AddFrame( pTextOperation ) ;
      CTextFrame * pTextResult = CreateTextFrameEx( m_cViewPt , 0x00ffff , 18 ,
        "Part %s  Ext Dia=%.2fum\n" ,
        ( LPCTSTR ) m_sPartName , m_dExtCurrentRadius_um * 2. ) ;
      pTextResult->Attributes()->WriteInt( "back" , 0 ) ;
      pOut->AddFrame( pTextResult ) ;
    }
    break ;

    case DPS_FrontGrinding:
    {
      CTextFrame * pTextOperation = CreateTextFrameEx( m_cProcessViewPt , 0x00ffff , 22 ,
        "Front Grinding" ) ;
      pTextOperation->Attributes()->WriteInt( "back" , 0 ) ;
      pOut->AddFrame( pTextOperation ) ;
      CTextFrame * pTextResult = CreateTextFrameEx( m_cViewPt , 0x00ffff , 18 ,
        "Part %s" , ( LPCTSTR ) m_sPartName ) ;
      pTextResult->Attributes()->WriteInt( "back" , 0 ) ;
      pOut->AddFrame( pTextResult ) ;
    }
    break ;
    case DPS_FrontGrindingFinished:
    {
      CTextFrame * pTextOperation = CreateTextFrameEx( m_cProcessViewPt , 0x00ff00 , 22 ,
        "Process Finished" ) ;
      pTextOperation->Attributes()->WriteInt( "back" , 0 ) ;
      pOut->AddFrame( pTextOperation ) ;
    }
    break ;
    case DPS_FrontBlackMeasurement:
    {
      CTextFrame * pTextOperation = CreateTextFrameEx( m_cProcessViewPt , 0x00ffff , 22 ,
        "Tip Measurement" ) ;
      pTextOperation->Attributes()->WriteInt( "back" , 0 ) ;
      pOut->AddFrame( pTextOperation ) ;
      CTextFrame * pTextResult = CreateTextFrameEx( m_cViewPt , 0x00ffff , 18 ,
        "Part %s   External Dia = %.2fum" ,
        ( LPCTSTR ) m_sPartName ,
        ( m_dExtCurrentRadius_um > GetWidth( pFrame ) ) ? 0. : // No ext circle
        m_dExtCurrentRadius_um * 2. + ( 3. * ( double ) rand() / RAND_MAX ) - 1.5 ) ;
      pTextResult->Attributes()->WriteInt( "back" , 0 ) ;
      pOut->AddFrame( pTextResult ) ;
    }
  }
  return iNFullPix ;
}


int ImageSimulator::DrawSideConus( CVideoFrame * pFrame , cmplx& cOrigin_pix )
{

  m_dExtBaseRadius_um = m_dExtCurrentRadius_um + m_dDepth_um * tan( m_dSideAngle_rad ) ;

  double dExtBaseRadius_pix = m_dExtBaseRadius_um / m_dSideScale_um_per_pix ;
  double dExtRadius_pix = m_dExtCurrentRadius_um / m_dSideScale_um_per_pix ;
  double dDepth_pix = cOrigin_pix.real() ;
  double dSlope_pixY_per_pixX = ( dExtBaseRadius_pix - dExtRadius_pix ) / dDepth_pix ;
  int iNFullPix = ( int ) dDepth_pix ;
  CPoint Pt( 0 , 0 );
  while ( Pt.x <= iNFullPix )
  {
    double dActiveRadius = dExtBaseRadius_pix - ( Pt.x * dSlope_pixY_per_pixX ) ;
    cmplx cUpperLeftPt( Pt.x , cOrigin_pix.imag() - dActiveRadius ) ;
    CPoint UpperPt( Pt.x , ( cUpperLeftPt.imag() < 0. ) ? 0 : ROUND( cUpperLeftPt.imag() ) ) ;
    cmplx cLowerPt( Pt.x , cOrigin_pix.imag() + dActiveRadius ) ;
    CPoint LowerPt( Pt.x , ( cLowerPt.imag() > m_cROI_pix.imag() ) ?
      m_ImageSize_pix.cy : ROUND( cLowerPt.imag() ) ) ;

    // Draw  vertical black line
    LPBYTE p = GetData( pFrame ) + Pt.x + UpperPt.y * GetWidth( pFrame ) ;
    LPBYTE pEnd = GetData( pFrame ) + Pt.x + LowerPt.y * GetWidth( pFrame ) ;
    while ( p < pEnd )
    {
      *p = 0 ;
      p += GetWidth( pFrame ) ;
    }
    Pt.x++ ;
  }
  return iNFullPix ;
}


int ImageSimulator::DrawFullBlank( CVideoFrame * pFrame )
{
  double dDepth_pix = m_dDepth_um / m_dSideScale_um_per_pix ;
  int iNFullPix = ( int ) dDepth_pix ;
  if ( m_dTurnAngle_rad < M_PI )
    iNFullPix-- ;

  DWORD dwY = 0 ;
  while ( dwY < GetHeight( pFrame ) )
  {
    // Draw vertical edge (black on left side, white on right side)
    LPBYTE p = GetData( pFrame ) + dwY * GetWidth( pFrame ) ;
    memset( p , 0 , iNFullPix ) ;
    dwY++ ;
    // End of part blank drawing
  }
  return iNFullPix;
}


int ImageSimulator::DrawCircle( CVideoFrame * pVF , cmplx& cCenter_pix , double dRadius_pix , 
  BYTE bIntens , BYTE * pEdgePixels , BYTE * pFrontPixels )
{
  int iNWritten = 0 ;
  double dXBegin = cCenter_pix.real() - dRadius_pix ;
  double dXEnd = cCenter_pix.real() + dRadius_pix ;
  double dYMin = cCenter_pix.imag() - dRadius_pix ;
  double dYMax = cCenter_pix.imag() + dRadius_pix ;
  if ( (dXEnd < 0) || (dXBegin > GetWidth( pVF ) )
    || ( dYMax < 0 ) || ( dYMin > GetWidth( pVF ) ) )
  {
    return 0 ;
  }
  else if ( (dXBegin < 0) && (dXEnd > GetWidth(pVF) ) )
  {

    LPBYTE p = GetData( pVF ) ;
    LPBYTE pEnd = p + GetImageSizeWH( pVF )  ;
    if ( pFrontPixels )
    {
      while ( p < pEnd )
      {
        if ( pFrontPixels > m_pFrontPixelsEnd )
          pFrontPixels = m_FrontPixels ;
        *( p++ ) = *( pFrontPixels++ ) ;
      }
      m_pCurrentFrontPix = pFrontPixels ;
    }
    else
      memset( p , bIntens , pEnd - p ) ;
  }
  double dXIter_pix = dXBegin + 0.5 ;
  double dR2 = dRadius_pix * dRadius_pix ;
  int iX = ROUND( dXIter_pix ) ;
  CRect ROI( 0 , 0 , GetWidth( pVF ) - 1 , GetHeight( pVF ) - 1 ) ;
  int iXEnd = ROUND( dXEnd ) ;
  if ( iXEnd > ROI.right )
    iXEnd = ROI.right ;

  // clipping left
  while ( iX < 0. )
    iX++ ;
  
  while ( iX < iXEnd )
  {
    double dXDistToCent_pix = cCenter_pix.real() - dXIter_pix ;
    double dY = sqrt( dR2 - ( dXDistToCent_pix * dXDistToCent_pix ) ) ;
    double dYUp = cCenter_pix.imag() - dY ;
    double dYDown = cCenter_pix.imag() + dY ;
    int iYDown = ROUND( dYDown ) ;
    if ( iYDown > ROI.bottom )
      iYDown = ROI.bottom ;

    int iY = ROUND( dYUp ) ;
    while ( iY < 0 ) // clipping upper side
      iY++ ;
    LPBYTE p = GetData( pVF ) + iX + iY * GetWidth( pVF ) ;
    LPBYTE pEnd = GetData( pVF ) + iX + iYDown * GetWidth( pVF ) ;
    if ( !pFrontPixels )
    {
      while ( p < pEnd )
      {
        *p = bIntens ;
        p += GetWidth( pVF ) ;
      }
    }
    else
    {
      while ( p < pEnd )
      {
        if ( pFrontPixels >= m_pFrontPixelsEnd )
          pFrontPixels = m_FrontPixels ;
        *p = *(pFrontPixels++) ;
        p += GetWidth( pVF ) ;
      }
    }

    iNWritten += (int)( pEnd - p ) / GetWidth( pVF ) ;
    iX++ ;
    dXIter_pix += 1. ;
  }
  if ( pFrontPixels )
    m_pCurrentFrontPix = pFrontPixels ;

  if ( pEdgePixels )
  {
    LPBYTE pVal = pEdgePixels ;
    LPBYTE pValEnd = m_EdgePixels + sizeof(m_EdgePixels) ;
    double dAngleStep_rad = 0.75 / dRadius_pix ;
    double dAngle_rad = 0. ;
    while ( dAngle_rad < M_2PI )
    {
      cmplx VectToPt = polar( dRadius_pix , dAngle_rad ) ;
      cmplx Pt = cCenter_pix + VectToPt ;
      if ( IsPtInFrame( Pt , pVF ) )
      {
        if ( pVal >= pValEnd )
          pVal = m_EdgePixels ;
        SetPixel( pVF , Pt , *( pVal++ ) ) ;
      }

      dAngle_rad += dAngleStep_rad ;
    }
    m_pCurrentPixel = pVal ;
  }

  return iNWritten ;
}

int ImageSimulator::FillConstFromCenterToEdge( LPBYTE pBuffer , CSize ROI , cmplx cEdgePt , BYTE Intens )
{
  cmplx cDir1( cEdgePt - m_cCenter_pix ) ;
  cmplx cNormDir = GetNormalized( cDir1 ) ;
  cmplx cPt( m_cCenter_pix ) ;
  cmplx cTarget( m_cCenter_pix + cDir1 ) ;
  while ( abs( cPt - cTarget ) > 1. )
  {
    SetPixel8( pBuffer , ROUND( cPt.real() ) , ROUND( cPt.imag() ) , ROI.cx , Intens ) ;
    cPt += cNormDir ;
  }
  return ROUND( abs(cDir1) ) ;
}

int ImageSimulator::CreateFrontImageBackground( LPBYTE pBuffer , CSize ROI )
{
#define NLEDS 6  
  for ( int iY = 0 ; iY < ROI.cy ; iY++ )
  {
    for ( int iX = 0 ; iX < ROI.cx ; iX++ )
    {
      cmplx Vect( iX - m_cCenter_pix.real() , iY - m_cCenter_pix.imag() ) ;
      double dAngle = arg( Vect ) * NLEDS ;
      BYTE bIntens = AVERAGE_CONUS 
        + ROUND( (ANGLE_NON_UNIFORMITY * cos( dAngle )) 
                 + ( RADIAL_NON_UNIFORMITY * ( 1. - abs( Vect ) / m_cCenter_pix.imag() ) ) ) ;
      SetPixel8( pBuffer , iX , iY , ROI.cx , bIntens ) ;
    }
  }
  return 1 ;
  for ( int iY = 0 ; iY < ROI.cy / 2 ; iY++ )
  {
    double dAngle = atan2( m_cCenter_pix.real() , m_cCenter_pix.imag() - iY ) ;
    dAngle *= NLEDS ;
    BYTE Intens = 100 + ROUND( 70. * cos( dAngle ) ) ;

    // Fill from center to up left
    FillConstFromCenterToEdge( pBuffer , ROI , cmplx( 0. , iY ) , Intens ) ;

    // Fill from center to down left
    FillConstFromCenterToEdge( pBuffer , ROI , cmplx( 0. , m_cROI_pix.imag() - iY ) , Intens ) ;

    // Fill from center to up right
    FillConstFromCenterToEdge( pBuffer , ROI , cmplx( ROI.cx , iY ) , Intens ) ;

    // Fill from center to down right
    FillConstFromCenterToEdge( pBuffer , ROI , cmplx( ROI.cx , m_cROI_pix.imag() - iY ) , Intens ) ;
  }

  for ( int iX = 0 ; iX < ROI.cx / 2 ; iX++ )
  {
    double dAngle = atan2( iX , m_cCenter_pix.imag() ) ;
    dAngle *= NLEDS ;
    BYTE Intens = 180 + ROUND( 30. * cos( dAngle ) ) ;

    // Fill from center to up left
    FillConstFromCenterToEdge( pBuffer , ROI , cmplx( iX , 0. ) , Intens ) ;

    // Fill from center to down left
    FillConstFromCenterToEdge( pBuffer , ROI , cmplx( iX , ROI.cy ) , Intens ) ;

    // Fill from center to up right
    FillConstFromCenterToEdge( pBuffer , ROI , cmplx( m_cROI_pix.real() - iX , 0. ) , Intens ) ;

    // Fill from center to down right
    FillConstFromCenterToEdge( pBuffer , ROI , cmplx( m_cROI_pix.real() - iX , ROI.cy ) , Intens ) ;
  }
  return 1;
}

int ImageSimulator::SimulatePinSide( CVideoFrame * pVF , CContainerFrame * pOut )
{
  UINT uiWidth = GetWidth( pVF ) ;
  UINT uiHeight = GetHeight( pVF ) ;

  cmplx cCenter( uiWidth / 2. , uiHeight / 2. ) ;
  cCenter._Val[ _IM ] += cos( fmod(M_PI_4 * m_iNFrames++ , M_2PI ) ) ;
  cmplx cROI( cCenter * 2. ) ;
  cmplx cPinBegin_pix( cROI.real() * m_dRelLeftSide , cCenter.imag() ) ;

  double dPinRadius_pix = m_dPinDia_um * 0.5 / m_dSideScale_um_per_pix ;
  cmplx cUpperLeftPt_pix( cPinBegin_pix - cmplx( 0. , dPinRadius_pix ) ) ;
  cmplx cLowerLeftPt_pix = ( cPinBegin_pix + cmplx( 0. , dPinRadius_pix ) ) ;

  double dPinLength_pix = m_dPinLength_um / m_dSideScale_um_per_pix ;
  cmplx cUpperRightPt_pix = cUpperLeftPt_pix + dPinLength_pix ;
  cmplx cLowerRightPt_pix = cLowerLeftPt_pix + dPinLength_pix ;

  double dHeadRadius_pix = m_dHeadRadius_um / m_dSideScale_um_per_pix ;

  // Draw round tip
  cmplx cTipCircleCenter ;
  if ( GetCircleCentOnVF( dHeadRadius_pix , cUpperRightPt_pix ,
    cLowerRightPt_pix , false , cTipCircleCenter ) ) // take center on right side
  {
    cmplx cTipPt = cTipCircleCenter + cmplx( dHeadRadius_pix , 0. ) ;
    for ( double dX= cUpperRightPt_pix.real();
              dX <= cTipPt.real(); dX += 1. )
    {
      double dY1 , dY2 ;
      if ( GetYforXOnCircle( dX , cTipCircleCenter , dHeadRadius_pix , dY1 , dY2 ) )
      {
        int iY1 = ROUND( dY1 ) , iY2 = ROUND( dY2 ) , iX = ROUND( dX ) ;
        LPBYTE p = GetData( pVF ) + iX + iY1 * uiWidth ;
        LPBYTE pEnd = p + (iY2 - iY1) * uiWidth ;
        *p = GetNextEdgePixel() ;
        p += uiWidth ;
        while ( p < pEnd )
        {
          *p = 0 ;
          p += uiWidth ;
        }
        *p = GetNextEdgePixel() ;
      }
    }
  };

  // Draw pin main body
  int iXLeft = ROUND( cUpperLeftPt_pix.real() ) ;
  int iXRight = ROUND( cUpperRightPt_pix.real() ) ;
  int iYUpper = ROUND( cUpperLeftPt_pix.imag() ) ;
  int iYLower = ROUND( cLowerLeftPt_pix.imag() ) ;
  LPBYTE pBegin = GetData( pVF ) + iXLeft + iYUpper * uiWidth ;
  int iLen = iXRight - iXLeft + 1 ;
  if ( m_pCurrentPixel + iLen >= m_pEdgePixelsEnd )
  {
    memcpy( pBegin , m_pCurrentPixel , m_pEdgePixelsEnd - m_pCurrentPixel ) ;
    memcpy( pBegin + ( m_pEdgePixelsEnd - m_pCurrentPixel ) , m_EdgePixels , 
      iLen - ( m_pEdgePixelsEnd - m_pCurrentPixel ) ) ;
    m_pCurrentPixel = m_EdgePixels + iLen - ( m_pEdgePixelsEnd - m_pCurrentPixel ) ;
  }
  else
  {
    memcpy( pBegin , m_pCurrentPixel , iLen ) ;
    m_pCurrentPixel += iLen ;
  }
  for ( int iY = iYUpper + 1 ; iY <= iYLower ; iY++ )
    memset( (pBegin += uiWidth) , 0 , iXRight - iXLeft + 1 ) ;
  if ( m_pCurrentPixel + iLen >= m_pEdgePixelsEnd )
  {
    memcpy( pBegin , m_pCurrentPixel , m_pEdgePixelsEnd - m_pCurrentPixel ) ;
    memcpy( pBegin + ( m_pEdgePixelsEnd - m_pCurrentPixel ) , m_EdgePixels , 
      iLen - ( m_pEdgePixelsEnd - m_pCurrentPixel ) ) ;
    m_pCurrentPixel = m_EdgePixels + iLen - ( m_pEdgePixelsEnd - m_pCurrentPixel ) ;
  }
  else
  {
    memcpy( pBegin , m_pCurrentPixel , iLen ) ;
    m_pCurrentPixel += iLen ;
  }

  // Draw basement
  if ( m_iBasementAngle == 0 )
  {
    for ( UINT uiY = 0 ; uiY < uiHeight ; uiY++ )
      memset( GetData( pVF ) + uiY * uiWidth , 0 , iXLeft ) ;
  }
  else
  {
    double dAngle = M_PI_2 + ( m_iBasementAngle * M_2PI / 360. ) ;
    cmplx UpperDir = polar( 1. , -dAngle ) ; // Y is going to down
    CLine2d UpperBaseLine ;
    UpperBaseLine.ByPointAndDir( cUpperLeftPt_pix , UpperDir ) ;

    cmplx LowerDir = polar( 1. , dAngle ) ; // Y is going to down
    CLine2d LowerBaseLine ;
    LowerBaseLine.ByPointAndDir( cLowerLeftPt_pix , LowerDir ) ;

    for ( int iX = iXLeft - 1 ; iX >= 0 ; iX-- )
    {
      double dUpperY = UpperBaseLine.GetY( iX ) ;
      double dLowerY = LowerBaseLine.GetY( iX ) ;
      int iUpperY = ( dUpperY < 0. ) ? 0 : ROUND( dUpperY ) ;
      int iLowerY = ( dLowerY > uiHeight ) ? uiHeight - 1 : ROUND( dLowerY ) ;

      LPBYTE p = GetData( pVF ) + iX + iUpperY * uiWidth ;
      LPBYTE pEnd = p + ( iLowerY - iUpperY - 1 ) * uiWidth ;
      *p = GetNextEdgePixel() ;
      p += uiWidth ;
      while ( p < pEnd )
      {
        *p = 0 ;
        p += uiWidth ;
      }
      *p = GetNextEdgePixel() ;
    }
  }
  return 1 ;
}