#include "stdafx.h"
// #include <windows.h>
// #include <gdiplusinit.h>
#include "CMouseHook.h"
#include "SpotSymmetry.h"
#include "helpers/FramesHelper.h"
#include "fxfc/CSystemMonitorsEnumerator.h"
#include "fxfc/FXRegistry.h"
#define THIS_MODULENAME "SpotSymmetry"

USER_FILTER_RUNTIME_GADGET(SpotSymmetry, LINEAGE_FILEX);

SpotSymmetry::SpotSymmetry() 
{
  m_OutputMode = modeReplace;
  
  init();

  FXRegistry Reg( "TheFileX\\ZoomCollimator" );
  m_iLastExposure_us = Reg.GetRegiInt( "CameraParams" , "Exposure_us" , 10000 ) ;
  m_iLastGain_dBx10 = Reg.GetRegiInt( "CameraParams" , "Gain_dBx10" , 0 ) ;
  m_dMaxHorDistortion_perc = Reg.GetRegiDouble( "ProcessingParams" , 
    "MaxHDistortion_perc" , 3.3 ) ;
  m_dMaxVertDistortion_perc = Reg.GetRegiDouble( "ProcessingParams" , 
    "MaxVDistortion_perc" , 4.0 ) ;
  m_ImageSaveDirectory = _T( "./Images" ) ;
  m_ResultFileNamePrefix = _T( "Result" ) ;
  m_dTargetSpotAmplitude = 200. ;
  m_dMaxDeration_per_ms = Reg.GetRegiDouble( "ProcessingParams" ,
    "MaxDeration_per_ms" , 1.e-4 ) ;
};

void SpotSymmetry::ShutDown()
{
  FXRegistry Reg( "TheFileX\\ZoomCollimator" );
//   Reg.WriteRegiInt( "CameraParams" , "Exposure_us" , m_iLastExposure_us ) ;
//   Reg.WriteRegiInt( "CameraParams" , "Gain_dBx10" , m_iLastGain_dBx10 ) ;
//   FXString Tmp ;
//   Tmp.Format( "%d,%d,%d,%d" , m_rcLowPassArea.left , m_rcLowPassArea.top ,
//     m_rcLowPassArea.right - m_rcLowPassArea.left ,
//     m_rcLowPassArea.bottom - m_rcLowPassArea.top ) ;
//   Reg.WriteRegiString( "ProcessingParams" , "LowPassArea" , Tmp ) ;
//   Tmp.Format( "%d,%d,%d,%d" , m_rcSOS_Zone.left , m_rcSOS_Zone.top ,
//     m_rcSOS_Zone.right - m_rcSOS_Zone.left , 
//     m_rcSOS_Zone.bottom - m_rcSOS_Zone.top ) ;
//   Reg.WriteRegiString( "ProcessingParams" , "SOSZone" , Tmp ) ;
  Reg.WriteRegiDouble( "ProcessingParams" ,
    "MaxHDistortion_perc" , m_dMaxHorDistortion_perc ) ;
  Reg.WriteRegiDouble( "ProcessingParams" ,
    "MaxVDistortion_perc" , m_dMaxVertDistortion_perc ) ;
};

static const char * pProcessingMode = "Idle;View;ViewSOSSensor;";

void SpotSymmetry::PropertiesRegistration()
{
  addProperty(SProperty::COMBO, "ProcessingMode", &m_ProcessingMode, SProperty::Long, pProcessingMode);
  addProperty(SProperty::SPIN, "ProfileWidth_pix", &m_iProfileWidth, SProperty::Int, 1, 40);
  addProperty(SProperty::EDITBOX, "ImageSaveDir", &m_ImageSaveDirectory, SProperty::String);
  addProperty(SProperty::EDITBOX, "ImageSavePrefix", &m_ResultFileNamePrefix, SProperty::String);
  addProperty( SProperty::EDITBOX , "LowPassArea" , &m_LowPassArea , SProperty::String );
  addProperty( SProperty::EDITBOX , "TargetSpotAmpl" ,
    &m_dTargetSpotAmplitude , SProperty::Double );
  addProperty( SProperty::EDITBOX , "MaxHDistortion_perc" ,
    &m_dMaxHorDistortion_perc , SProperty::Double );
  addProperty( SProperty::EDITBOX , "MaxVDistortion_perc" ,
    &m_dMaxVertDistortion_perc , SProperty::Double );
  addProperty( SProperty::EDITBOX , "SOSZone" , &m_SOS_Zone , SProperty::String );
  addProperty( SProperty::SPIN , "AutoExposure" ,
    &m_bAutoExposeEnable , SProperty::Int , 0 , 1 );
  addProperty( SProperty::SPIN , "ViewMode" ,
    &m_iViewMode , SProperty::Int , 0 , 20 );
  addProperty( SProperty::SPIN , "Average" ,
    &m_iDiffAverager , SProperty::Int , 1 , 20 );
  addProperty( SProperty::SPIN , "AveragedIntens/R" ,
    &m_dAveragedIntensity , SProperty::Double );

}

void SpotSymmetry::ConnectorsRegistration()
{
  addInputConnector(transparent, "DataAfterTVObject");
  addOutputConnector(transparent, "OutImages");
  addOutputConnector( transparent , "OriginalSave" );
  addOutputConnector( transparent , "LowPassControl" );
  addOutputConnector( transparent , "CameraControl" ) ;
  addOutputConnector( transparent , "ViewControl" ) ;
  addDuplexConnector(text, text, "Control");
};

void SpotSymmetry::AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame)
{
  if (pParamFrame)
  {
    const CTextFrame * pCommand = pParamFrame->GetTextFrame();
    if (pCommand)
    {
      FXString Label = pCommand->GetLabel();
      if (Label == "SaveResult")
        m_bSaveResult = true;
      else if (Label == "SaveOrigin")
        m_bSaveOrigin = true;
      else
      {
        FXString Command = pCommand->GetString();
        if (Command.Find(_T("set") == 0))
        {
          bool Invalidate = false;
          ScanProperties(((LPCTSTR)Command) + 4, Invalidate);
        }
      }
    }
    pParamFrame->Release(pParamFrame);
  }
};

#define ORIGINALS_SIZE 10
/*
Licensing init procedure:
When gadget is started first time, it creates Registry string 
called "SystemID" with initial value "No ID" placed in
Computer\HKEY_CURRENT_USER\Software\TheFileX\ZoomCollimator\ProcessingParams

For create legal license, necessary to open regedit.exe application and to replace 
string in "SystemID" for CURRENT date in format YYYYMMDD (for example,
20231126).
After that necessary to restart graph running application (SHStudio or GraphDeploy 
or other application which does use SpotSymmetry gadget) and licensing update 
will be finished.
SystemID will hold hexadecimal string

*/

CDataFrame * SpotSymmetry::DoProcessing(const CDataFrame * pDataFrame)
{
  if (m_GadgetName.IsEmpty())
  {
    GetGadgetName( m_GadgetName );
    ProgramCamera( m_iLastExposure_us , m_iLastGain_dBx10 ) ;

    //m_bLicensed = Tvdb400_CheckSetLicense( "ZoomCollimator" , "ProcessingParams" ) ;
  }

//   if ( !m_bLicensed )
//   {
//     SEND_GADGET_ERR( "Gadget 'SpotSymmetry' is not licensed on this computer" ) ;
//     return NULL ;
//   }
  if ( m_LowPassArea != m_LowPassAreaUsed )
  {
    CRect rcLowPassArea ;
    if ( GetArray( m_LowPassArea , _T( 'i' ) , 4 , &rcLowPassArea ) == 4 )
    {
      FXString TVObjectROIPos ;
      TVObjectROIPos.Format( "name=get_cent;xoffset=%d;yoffset=%d;width=%d;height=%d;" ,
        rcLowPassArea.left , rcLowPassArea.top ,
        rcLowPassArea.right , rcLowPassArea.bottom );
      SetParametersToTVObject( TVObjectROIPos );

      rcLowPassArea.right += rcLowPassArea.left ;
      rcLowPassArea.bottom += rcLowPassArea.top ;
      if ( rcLowPassArea != m_rcLowPassArea ) // area is changed 
      {
        CTextFrame * pLowPassControl = CTextFrame::Create( _T( "ROI=" ) ) ;
        pLowPassControl->GetString() += m_LowPassArea + _T( ';' ) ;
        pLowPassControl->SetLabel( _T( "SetROI" ) ) ;
        PutFrame( GetOutputConnector( SSO_LPF_Control ) , pLowPassControl ) ;

        m_rcLowPassArea = rcLowPassArea ;
        FXRegistry Reg( "TheFileX\\ZoomCollimator" );
        Reg.WriteRegiString( "ProcessingParams" , "LowPassArea" , m_LowPassArea ) ;
      }
    }
    m_LowPassAreaUsed = m_LowPassArea ;
  }
  if ( m_SOS_Zone != m_SOS_ZoneUsed )
  {
    CRect rcSOSZone ;
    if ( GetArray( m_SOS_Zone , _T( 'i' ) , 4 , &rcSOSZone ) == 4 )
    {
      m_rcSOS_Zone = rcSOSZone ;
      m_rcSOS_Zone.right += m_rcSOS_Zone.left ;
      m_rcSOS_Zone.bottom += m_rcSOS_Zone.top ;
      FXRegistry Reg( "TheFileX\\ZoomCollimator" );
      Reg.WriteRegiString( "ProcessingParams" , "SOSZone" , m_SOS_Zone ) ;
      m_SOS_ZoneUsed = m_SOS_Zone ;
    }
  }

  if ( m_ProcessingMode != m_OldProcessingMode )
  {
    FXRegistry Reg( "TheFileX\\ZoomCollimator" );
    switch ( m_ProcessingMode )
    {
    case SSPM_Idle:
    case SSPM_Process:
      {
        int iExp = Reg.GetRegiInt( "CameraParams" , "ResetExposure_us" , 10000 ) ;
        int iGain = Reg.GetRegiInt( "CameraParams" , "ResetGain_dBx10" , 100 ) ;
        ProgramCamera( iExp , iGain ) ;
        SetTaskTVObject( m_ProcessingMode == SSPM_Process ? 2 : 10 ) ;
      }
      break ;
    case SSPM_ViewSOSSensor:
      {
        FXRegistry Reg( "TheFileX\\ZoomCollimator" );
        int iExp = Reg.GetRegiInt( "CameraParams" , "SOSExposure_us" , 30000 ) ;
        int iGain = Reg.GetRegiInt( "CameraParams" , "SOSGain_dBx10" , 100 ) ;
        ProgramCamera( iExp , iGain ) ;
        SetTaskTVObject( 10 ) ; //nothing to do
        break ;
      }
    }

    m_OldProcessingMode = m_ProcessingMode ;
  }

  if (!pDataFrame->IsContainer())
  {
    CVideoFrame * pVF = (CVideoFrame*)(pDataFrame->GetVideoFrame());
    if (pVF)
    {
      pVF->AddRef();
      if ( m_ProcessingMode == SSPM_Idle )
      {
        CContainerFrame * pOut = CContainerFrame::Create() ;
        pOut->CopyAttributes( pDataFrame , true ) ;
        pOut->AddFrame( pVF ) ;
        pOut->AddFrame( CreateRectFrame( m_rcLowPassArea , 0xff00ff ) );
        pOut->AddFrame( CreateFigureFrameEx( m_rcSOS_Zone, 0X000000FF ) ) ;
        return pOut ;
      }
      else
      {
        m_Originals.insert( m_Originals.begin() , pVF );
  // remove lasts in originals FIFO
        while ( m_Originals.size() > ORIGINALS_SIZE )
        {
          ( ( CDataFrame* ) ( m_Originals[ ORIGINALS_SIZE ] ) )->Release();
          m_Originals.erase( m_Originals.begin() + ORIGINALS_SIZE );
        }
      }
    }
    return NULL;
  }
  const CVideoFrame * pVF = pDataFrame->GetVideoFrame();
  if (!pVF)
    return NULL;

  m_cLastFrameSize._Val[ _RE ] = GetWidth( pVF ) ;
  m_cLastFrameSize._Val[ _IM ] = GetHeight( pVF ) ;

  switch (m_ProcessingMode)
  {
  case SSPM_Idle:
    {
      if (m_bSaveResult)
      {
        m_bSaveResult = false;
        CContainerFrame * pNewContainer = CContainerFrame::Create();
        pNewContainer->CopyAttributes( pDataFrame , true ) ;
        pNewContainer->AddFrame(pDataFrame);
        pNewContainer->AddFrame(CreateTextFrame("SaveImage",
          (LPCTSTR)m_ImageSaveDirectory));
        pNewContainer->AddFrame(CreateTextFrame("ImageSavePrefix",
          (LPCTSTR)m_ResultFileNamePrefix));
        return pNewContainer;
      }
      //((CDataFrame*)pDataFrame)->AddRef();
      //return (CDataFrame*)pDataFrame;
      return NULL ;
    }
    case SSPM_ViewSOSSensor:
    {
      if ( m_Originals.size() )
      {
         const CVideoFrame * pLastOrig = m_Originals.front();
         CContainerFrame * pOut = CContainerFrame::Create() ;
         pOut->CopyAttributes( pDataFrame , true ) ;
         pOut->AddFrame( pLastOrig ) ;
         pOut->AddFrame( CreateRectFrame( m_rcLowPassArea , 0xff00ff ) );
         pOut->AddFrame( CreateFigureFrameEx( m_rcSOS_Zone , 0X000000FF ) ) ;
         cmplx MarkPt1( m_rcSOS_Zone.left , m_rcSOS_Zone.top + (m_rcSOS_Zone.Height()* 0.5) ) ;
         cmplx MarkPt2 = MarkPt1 - 0.5 * m_rcSOS_Zone.Width() ; 
         cmplx MarkPt3( m_rcSOS_Zone.right , MarkPt1.imag() ) ;
         cmplx MarkPt4 = MarkPt3 + 0.5 * m_rcSOS_Zone.Width() ;

         pOut->AddFrame( CreateLineFrameEx( MarkPt1 , MarkPt2 , 0x000000ff ) ) ;
         pOut->AddFrame( CreateLineFrameEx( MarkPt3 , MarkPt4 , 0x000000ff ) ) ;
         return (CDataFrame*) pOut ;
      }

      return NULL ;
    }
  }

  CContainerFrame * pOut = CContainerFrame::Create();
  pOut->CopyAttributes( pDataFrame , true ) ;
  pOut->AddFrame(pDataFrame);
  m_LastSpots.RemoveAll();
  int iNewExposure = m_iLastExposure_us , iNewGain = m_iLastGain_dBx10 ;

  if (!ExtractDataAboutSpots(pDataFrame, m_LastSpots))
  {
    if ( CalcCamControlValues( pVF , 0 ,
      iNewExposure , iNewGain ) )
    {
      ProgramCamera( iNewExposure , iNewGain ) ;
    }

//     if ( (++m_iNframesNoSpots > 8) && (iMax < 50) )
//     {
//       if ( m_iNframesNoSpots <= 9 )
//       {
//         FXRegistry Reg( "TheFileX\\ZoomCollimator" );
//         m_iLastExposure_us = Reg.GetRegiInt( "CameraParams" , "ResetExposure_us" , 5000 ) ;
//         m_iLastGain_dBx10 = Reg.GetRegiInt( "CameraParams" , "ResetGain_dBx10" , 0 ) ;
//         ProgramCamera( m_iLastExposure_us , m_iLastGain_dBx10 ) ;
//       }
//       else if ( (m_iNframesNoSpots % 8) == 0 )
//       {
//         if ( m_iLastGain_dBx10 <= 240 )
//         {
//           m_iLastGain_dBx10 += 60 ;
//         }
//         else
//           m_iLastExposure_us *= 2 ;
//         ProgramCamera( m_iLastExposure_us , m_iLastGain_dBx10 ) ;
//       }
//       pOut->AddFrame( CreateTextFrameEx( ViewPt , 0x00ffff , 16 ,
//         "Exp=%dus Gain=%.1fdB\nMin=%d Max=%d" ,
//         m_iLastExposure_us , m_iLastGain_dBx10 * 0.1 ,
//         iMin , iMax ) ) ;
//     }
//     else if ( m_iNframesNoSpots > 5 )
//     {
//       cmplx MsgPt( m_rcLowPassArea.left + 10. , m_rcLowPassArea.top + 50. ) ;
//       pOut->AddFrame( CreateTextFrameEx( MsgPt , "Imaging Adjustment" , 0x00ffff , 20 ) ) ;
//     }
 //   else
    {
      pOut->Release() ;
      return NULL ;
    }

//     ((CDataFrame*)pDataFrame)->AddRef() ;
//     return ( ( CDataFrame* ) pDataFrame ); // no measured spot

    return pOut ; // no spots
  }

  CColorSpot * pSpotData = GetSpotData(m_LastSpots, "laser_spot");
  if (!pSpotData)
  {
    pOut->Release() ;
    return NULL;
  }

  m_iNframesNoSpots = 0 ;

  double dMaxDiff = pSpotData->m_iMaxPixel - m_dAveragedIntensity ;
  if ( dMaxDiff > 0 )
    m_dAveragedIntensity = pSpotData->m_iMaxPixel;

  if ( CalcCamControlValues( pVF , pSpotData->m_iMaxPixel ,
    iNewExposure , iNewGain ) )
  {
    ProgramCamera( iNewExposure , iNewGain ) ;
  }

  cmplx ExpViewPt( m_cLastFrameSize.real() * 0.1 , m_cLastFrameSize.imag() * 0.9 ) ;
  pOut->AddFrame( CreateTextFrameEx( ExpViewPt , 0x00ffff , 12 ,
    "Exp=%dus Gain=%.1fdB\nMax=%d Diff=%.1f" , 
    m_iLastExposure_us , m_iLastGain_dBx10 * 0.1 ,
    pSpotData->m_iMaxPixel , dMaxDiff ) ) ;


  CRect Intersection, LPFNorm( m_rcLowPassArea.TopLeft() , 
    CSize( m_rcLowPassArea.right , m_rcLowPassArea.bottom )) ;
  
  Intersection.IntersectRect( &m_rcLowPassArea , &(pSpotData->m_OuterFrame) ) ;
  if ( Intersection != pSpotData->m_OuterFrame )
  {
    cmplx ViewPt( m_rcLowPassArea.left + 10. , m_rcLowPassArea.top + 20. ) ;
    pOut->AddFrame( CreateTextFrameEx( ViewPt , "Spot is out of processing area" , 0x0000ff , 24 ) ) ;
    return pOut ;
  }
  Profile HProfile;
  Profile VProfile;
  Profile TmpProfile;

#define ProfileExtent (40)
  int iHorProfileLength = pSpotData->m_OuterFrame.Width() + ProfileExtent;
  int iVertProfileLength = pSpotData->m_OuterFrame.Height() + ProfileExtent;

  CRect rcHorProfile(ROUND(pSpotData->m_SimpleCenter.x) - iHorProfileLength / 2,
    ROUND(pSpotData->m_SimpleCenter.y) - m_iProfileWidth / 2,
    iHorProfileLength, m_iProfileWidth);
  if (rcHorProfile.left < 0)
  {
    rcHorProfile.left = 0;
    rcHorProfile.right = iHorProfileLength;
  }
  else if ( ( rcHorProfile.left + iHorProfileLength ) > (int)( GetWidth( pVF ) - 2 ) )
    rcHorProfile.right = GetWidth( pVF ) - rcHorProfile.left - 2;
  
  if ( rcHorProfile.top < 0 )
  {
    rcHorProfile.top = 0;
    rcHorProfile.bottom = m_iProfileWidth;
  }
  else if ( ( rcHorProfile.top + m_iProfileWidth ) > ( int )( GetHeight( pVF ) - 2 ) )
    rcHorProfile.bottom = ( GetHeight( pVF ) - rcHorProfile.top - 2 ) ;

  CRect rcVertProfile(ROUND(pSpotData->m_SimpleCenter.x) - m_iProfileWidth / 2,
    ROUND(pSpotData->m_SimpleCenter.y) - iVertProfileLength / 2,
    m_iProfileWidth, iVertProfileLength);
  if (rcVertProfile.left < 0)
  {
    rcVertProfile.left = 0;
    rcVertProfile.right = m_iProfileWidth;
  }
  else if ( ( rcVertProfile.left + m_iProfileWidth ) > ( int )( GetWidth( pVF ) - 2 ) )
    rcHorProfile.right = GetWidth( pVF ) - rcHorProfile.left - 2;
  if (rcVertProfile.top < 0)
  {
    rcVertProfile.top = 0;
    rcVertProfile.bottom = iVertProfileLength;
  }
  else if ( ( rcVertProfile.top + iVertProfileLength ) > ( int )( GetHeight( pVF ) - 2 ) )
    rcVertProfile.bottom = ( GetHeight( pVF ) - rcVertProfile.top - 2 ) ;

  calc_profiles(pVF, &HProfile, &TmpProfile, &rcHorProfile);
  calc_profiles(pVF, &TmpProfile, &VProfile, &rcVertProfile);


#define GraphShift (50.)
#define GraphAmpl_pix (200.)

  double dHBase = pSpotData->m_OuterFrame.top - GraphShift;
  double dHMax = dHBase - GraphAmpl_pix;
  //double dVBase = pSpotData->m_OuterFrame.right + GraphShift;
  double dVBase = pSpotData->m_OuterFrame.left - GraphShift;
  double dVMax = dVBase - GraphAmpl_pix;

  double dHScale = (HProfile.m_dMaxValue - HProfile.m_dMinValue) / GraphAmpl_pix;
  double dVScale = (VProfile.m_dMaxValue - VProfile.m_dMinValue) / GraphAmpl_pix;

  cmplx cHProfInitialPt(pSpotData->m_SimpleCenter.x - (iHorProfileLength / 2.), dHBase);
  CFigureFrame * pHProfileView = CreateFigureFrame(&cHProfInitialPt, 1, 0x00ff00, "HorProfile", pDataFrame->GetId());

  int iHProfEnd = ROUND(cHProfInitialPt.real()) + iHorProfileLength;
  double dLeftSum = 0., dRightSum = 0., dUpperSum = 0., dLowerSum = 0.;
  for (int i = ROUND(cHProfInitialPt.real()) + 1; i < iHProfEnd; i++)
  {
    double dAmpl = HProfile.m_pProfData[i] - HProfile.m_dMinValue;
    CDPoint cdpNextPt(i, dHBase - (dAmpl / dHScale));
    pHProfileView->Add(cdpNextPt);
    if (i < pSpotData->m_SimpleCenter.x)
      dLeftSum += dAmpl;
    else
      dRightSum += dAmpl;
  }
  CDPoint cHProfEndPointPt(iHProfEnd, dHBase);
  pHProfileView->Add(cHProfEndPointPt);
  pHProfileView->Add(CmplxToCDPoint(cHProfInitialPt));

  cmplx cVProfInitialPt(dVBase, pSpotData->m_SimpleCenter.y - (iVertProfileLength / 2.));
  CFigureFrame * pVProfileView = CreateFigureFrame(&cVProfInitialPt, 1, 0x00ff00, "VertProfile", pDataFrame->GetId());

  int iVProfEnd = ROUND(cVProfInitialPt.imag()) + iVertProfileLength;
  for (int i = ROUND(cVProfInitialPt.imag()) + 1; i < iVProfEnd; i++)
  {
    double dAmpl = VProfile.m_pProfData[i] - VProfile.m_dMinValue;
    CDPoint cdpNextPt(dVBase - (dAmpl / dVScale), i);
    pVProfileView->Add(cdpNextPt);
    if (i < pSpotData->m_SimpleCenter.y)
      dUpperSum += dAmpl;
    else
      dLowerSum += dAmpl;
  }
  CDPoint cVProfEndPointPt(dVBase, iVProfEnd);
  pVProfileView->Add(cVProfEndPointPt);
  pVProfileView->Add(CmplxToCDPoint(cVProfInitialPt));

  pOut->AddFrame(pHProfileView);
  pOut->AddFrame(pVProfileView);

  cmplx cLeftValView(pSpotData->m_SimpleCenter.x - 300., dHMax - 90.);
  cmplx cRightValView(pSpotData->m_SimpleCenter.x + 100., dHMax - 90.);
  cmplx cLeftRightDiffPt( pSpotData->m_SimpleCenter.x - 50. , dHMax - 50. );
  cmplx cUpperValView(dVMax - 150, pSpotData->m_SimpleCenter.y - (iVertProfileLength / 2.));
  cmplx cLowerValView( dVMax - 150 , pSpotData->m_SimpleCenter.y + ( iVertProfileLength / 3. ) );
  cmplx cUpperLowerDiffPt( dVMax - 150, pSpotData->m_SimpleCenter.y - 25);
  
  if ( m_iViewMode > 3 )
  {
    pOut->AddFrame( CreateTextFrame( cLeftValView , "0x4080ff" ,
      20 , "LeftVal" , pDataFrame->GetId() , "%.0f" , dLeftSum ) );
    pOut->AddFrame( CreateTextFrame( cRightValView , "0x4080ff" ,
      20 , "RightVal" , pDataFrame->GetId() , "%.0f" , dRightSum ) );
    pOut->AddFrame( CreateTextFrame( cUpperValView , "0x4080ff" ,
      20 , "UpperVal" , pDataFrame->GetId() , "%.0f" , dUpperSum ) );
    pOut->AddFrame( CreateTextFrame( cLowerValView , "0x4080ff" ,
      20 , "LowerVal" , pDataFrame->GetId() , "%.0f" , dLowerSum ) );
  }
 
  double dHDiff = dRightSum - dLeftSum ;
  double dVDiff = dUpperSum - dLowerSum ;
  double dHNormDiff = dHDiff * 100. * 2. / ( dRightSum + dLeftSum ) ;
  double dVNormDiff = dVDiff * 100. * 2. / ( dUpperSum + dLowerSum ) ;
  if ( m_iViewMode > 0 )
  {
    m_dHLastAveragedDiff =
      (m_dHLastAveragedDiff * (m_iDiffAverager - 1) + dHDiff) / m_iDiffAverager ;
    m_dVLastAveragedDiff =
      (m_dVLastAveragedDiff * (m_iDiffAverager - 1) + dVDiff) / m_iDiffAverager ;
    pOut->AddFrame( CreateTextFrameEx( cLeftRightDiffPt ,
      (fabs( dHNormDiff ) <= m_dMaxHorDistortion_perc) ? 0x0000ff00 : 0x000000ff ,
      24 , "%.0f%%" , /*m_dHLastAveragedDiff*/ dHNormDiff ) );
    pOut->AddFrame( CreateTextFrameEx( cUpperLowerDiffPt ,
      (fabs( dVNormDiff ) <= m_dMaxVertDistortion_perc) ? 0x0000ff00 : 0x000000ff ,
      24 , "%.0f%%" , /*m_dVLastAveragedDiff*/ dVNormDiff ) );
  }
  else
  {
    pOut->AddFrame( CreateTextFrameEx( cLeftRightDiffPt ,
      (fabs( dHNormDiff ) <= m_dMaxHorDistortion_perc) ? 0x0000ff00 : 0x000000ff ,
      24 , (fabs( dHNormDiff ) <= m_dMaxHorDistortion_perc) ? "PASS%c" : "FAIL%c" ,
      (dHDiff >= 0) ? '+' : '-' ) );
    pOut->AddFrame( CreateTextFrameEx( cUpperLowerDiffPt ,
      (fabs( dVNormDiff ) <= m_dMaxVertDistortion_perc) ? 0x0000ff00 : 0x000000ff ,
      24 , (fabs( dVNormDiff ) <= m_dMaxHorDistortion_perc) ? "PASS%c" : "FAIL%c" ,
      (dVDiff >= 0)? '+' : '-' ) );
  }

  // Form result as text for C#
  FXString ResultsAsText ;
  ResultsAsText.Format( "Left=%d HDiff=%d Right=%d Upper=%d VDiff=%d Lower=%d" ,
    ROUND( dLeftSum ) , ROUND( dHDiff ) , ROUND( dRightSum ) ,
    ROUND( dUpperSum ) , ROUND( dVDiff ) , ROUND( dLowerSum ) ) ;
  CTextFrame * pResultAsText = CTextFrame::Create( ResultsAsText ) ;
  pResultAsText->SetLabel( "ResultAsText" ) ;
  pResultAsText->ChangeId( pDataFrame->GetId() ) ;
  pOut->AddFrame( pResultAsText ) ;

  cmplx HorMarkLeft(0., pSpotData->m_SimpleCenter.y);
  cmplx HorMarkRight(GetWidth(pVF), pSpotData->m_SimpleCenter.y);
  pOut->AddFrame(CreateLineFrame(HorMarkLeft, HorMarkRight, 0xffff00, "HorMarker"));
  cmplx VertMarkUp(pSpotData->m_SimpleCenter.x, 0.);
  cmplx VertMarkDown(pSpotData->m_SimpleCenter.x, GetHeight(pVF));
  pOut->AddFrame(CreateLineFrame(VertMarkUp, VertMarkDown, 0xffff00, "VertMarker"));

//   const CTextFrame * pMoments = pDataFrame->GetTextFrame("WeightedMoments");
// 
//   CTextFrame * pMomentsView = CreateTextFrame(cmplx(10., GetHeight(pVF) - 100.),
//     "0x0000ff", 14, "Moments", pDataFrame->GetId(), "Moments: %s", (LPCTSTR)(pMoments->GetString()));
//   pOut->AddFrame(pMomentsView);
  if (m_bSaveResult)
  {
    m_bSaveResult = false;
    pOut->AddFrame(CreateTextFrame((LPCTSTR)m_ImageSaveDirectory, "SaveImage"));
    pOut->AddFrame(CreateTextFrame((LPCTSTR)m_ResultFileNamePrefix, "ImageSavePrefix"));
  }
  PutFrame(m_pOutput, pOut);

  if (m_bSaveOrigin)
  {
    for (size_t i = 0; i < m_Originals.size(); i++)
    {
      if (m_bSaveOrigin)
      {
        if (m_Originals[i]->GetId() == pDataFrame->GetId())
        {
          m_bSaveOrigin = false;
          PutFrame(GetOutputConnector(SSO_OriginalsOut), (CDataFrame*)(m_Originals[i]), 100);
          m_Originals.erase(m_Originals.begin() + i);
          i--;
        }
      }
      else
      {
        ((CDataFrame*)(m_Originals[i]))->Release();
        m_Originals.erase(m_Originals.begin() + i);
        i--;
      }
    }
  }

  return NULL;
}

int SpotSymmetry::ProgramCamera( int iExposure_us , int iGain_dBx10 )
{
  FXString Params ;
  if ( iGain_dBx10 >= 0 )
  {
    Params.Format( "set properties(Shutter_us=%d; Gain_dBx10=%d;)" ,
      m_iLastExposure_us = iExposure_us , m_iLastGain_dBx10 = iGain_dBx10 ) ;
  }
  else
  {
    Params.Format( "set properties(Shutter_us=%d; )" , m_iLastExposure_us = iExposure_us ) ;
  }
  CTextFrame * pCamCom = CreateTextFrame( Params , "CameraParamsSet" );

  return PutFrame( GetOutputConnector( SSO_CameraControl ) , pCamCom );
}

bool SpotSymmetry::SetParametersToTVObject( LPCTSTR pParams )
{
  CTextFrame * pCommand = CreateTextFrame( pParams ,
    "SetObjectProp" );
  return PutFrame( GetOutputConnector( SSO_CameraControl ) , pCommand );
}

bool SpotSymmetry::SetTaskTVObject( int iTask )
{
  CTextFrame * pCommand = CTextFrame::Create();
  pCommand->GetString().Format( "Task(%d);" , iTask ) ;
  pCommand->SetLabel( "SetTask" ) ;
  return PutFrame( GetOutputConnector( SSO_CameraControl ) , pCommand );
}



int SpotSymmetry::CalcCamControlValues( const CVideoFrame * pVF , 
  int iMaxValInSpot , int& iNewExposure_us , int& iNewGain_dBx10 )
{
  if ( !pVF )
    return 0 ;

  memcpy( &m_VFEmbedInfo , 
    GetData( pVF ) + GetImageSize( pVF) - sizeof( m_VFEmbedInfo ) ,
    sizeof( m_VFEmbedInfo ) ) ;
  m_VFEmbedInfo.Inverse() ;
  
  if ( m_iLastExposure_us != ROUND( m_VFEmbedInfo.m_dExposure_us )
    || m_iLastGain_dBx10 != ROUND( m_VFEmbedInfo.m_dGain_dB ) )
    return 0 ;

  int iTimeFromLastFrame = (int)(( m_ui64LastFrameTimeStamp_ns ) ?
    (m_VFEmbedInfo.m_CameraFrameTime - m_ui64LastFrameTimeStamp_ns) / 1000000L : 0) ;

  m_dLastMax *= pow( (1.- m_dMaxDeration_per_ms) , iTimeFromLastFrame ) ;

  if ( !iMaxValInSpot )
  {
    int iMin = INT_MAX, iMax = 0  ;
    _find_min_max( pVF , iMin , iMax , m_rcLowPassArea ) ;
    iMaxValInSpot = iMax ;
  }

  double dRelAmplError = 
    ( iMaxValInSpot - m_dTargetSpotAmplitude ) / m_dTargetSpotAmplitude ;
  if ( fabs( dRelAmplError ) > 0.05 )
  {
    if ( iMaxValInSpot >= 254 )
    {
      if ( m_iLastGain_dBx10 >= 60 )
      {
        m_iLastGain_dBx10 -= 60 ;
      }
      else
        m_iLastExposure_us *= 2 ;

      iNewExposure_us = m_iLastExposure_us / 2 ;
    }
    else
    {
      double dReldBx10 = 20. * log10( fabs(dRelAmplError) ) ;
      if ( dRelAmplError > 0 )
      {
        if ( dReldBx10 < m_iLastGain_dBx10 )
        {
          iNewGain_dBx10 = ROUND(( double ) m_iLastGain_dBx10 - dReldBx10) ;
        }
        else
          iNewExposure_us = ROUND( m_iLastExposure_us * ( 1 - dRelAmplError ) ) ;
      }
      else
      {
        double dNewExp_us = m_iLastExposure_us * ( 1 - dRelAmplError ) ;
        if ( dNewExp_us < 60000 )
          iNewExposure_us = ROUND( dNewExp_us ) ;
        else
        {
          if ( m_iLastGain_dBx10 <= 300 - dReldBx10 )
          {
            m_iLastGain_dBx10 += ROUND( dReldBx10 ) ;
          }
          else
            iNewExposure_us = ROUND( dNewExp_us ) ;
        }
      }
//       if ( m_iLastGain_dBx10 <= 240 )
//         {
//           m_iLastGain_dBx10 += 60 ;
//         }
//         else
//           m_iLastExposure_us *= 2 ;
//       iNewExposure_us = ROUND( m_iLastExposure_us * ( 1 - dRelAmplError ) ) ;
    }
  }

  return 1 ;
}


int SpotSymmetry::SetViewZone()
{
  FXRegistry Reg( "TheFileX\\ZoomCollimator" );

  switch ( m_ProcessingMode )
  {
    case SSPM_Idle:
    {
      int iViewShift = Reg.GetRegiInt( "ViewParams" , "IdleViewShift" , 0 ) ;
      int iProcessZoom = Reg.GetRegiInt( "ViewParams" , "IdleZoom" , 0 ) ;
      CPoint Center( ROUND( m_LastSpots[ 0 ].m_SimpleCenter.x ) ,
        ROUND( m_LastSpots[ 0 ].m_SimpleCenter.y + iViewShift ) ) ;
      FXString CenterInfo ;
      CenterInfo.Format( "Xc=%d;Yc=%d;Scale=%d" , Center.x , Center.y , iProcessZoom ) ;
      CTextFrame * pCenter = CreateTextFrame( CenterInfo , "SetCenter" ) ;
      PutFrame( GetOutputConnector( SSO_MainOutput ) , pCenter ) ;
    }
    break ;
    case SSPM_Process:
    {
      int iViewShift = Reg.GetRegiInt( "ViewParams" , "ProcessViewShift" , -250 ) ;
      int iProcessZoom = Reg.GetRegiInt( "ViewParams" , "ProcessZoom" , 1 ) ;
      CPoint Center( ROUND(m_LastSpots[0].m_SimpleCenter.x) ,
        ROUND( m_LastSpots[ 0 ].m_SimpleCenter.y + iViewShift ) ) ;
      FXString CenterInfo ;
      CenterInfo.Format( "Xc=%d;Yc=%d;Scale=%d" , Center.x , Center.y , iProcessZoom ) ;
      CTextFrame * pCenter = CreateTextFrame( CenterInfo , "SetCenter" ) ;
      PutFrame( GetOutputConnector( SSO_MainOutput ) , pCenter ) ;
    }
    break;
    case SSPM_ViewSOSSensor:
    {
      int iSosZoom = Reg.GetRegiInt( "ViewParams" , "SOSZoom" , 4 ) ;
      CPoint Center( m_rcSOS_Zone.CenterPoint() ) ;
      FXString CenterInfo ;
      CenterInfo.Format( "Xc=%d;Yc=%d;Scale=%d" , Center.x , Center.y , iSosZoom ) ;
      CTextFrame * pCenter = CreateTextFrame( CenterInfo , "SetCenter" ) ;
      PutFrame( GetOutputConnector( SSO_MainOutput ) , pCenter ) ;
    }
    break ;
  }
  return 0;
}
