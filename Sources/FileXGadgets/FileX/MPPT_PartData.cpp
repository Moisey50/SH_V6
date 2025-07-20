#include "stdafx.h"
#include "MPPT.h"
#include <fxfc/FXRegistry.h>

FXString CavityParams::ToString()
{
  FXString AsString ;
  AsString.Format( "CavityExp_us=%d;LaserExp_us=%d;"
    "DefocusExp_us=%d;CavityEdge=%d;"
    "DefocThres=%.3f;DefocLongStep=%.1f;DefocShortStep=%.1f;"
    "RingDefocExp_us=%d;StraightDefocExp_us=%d;"
    "FocusLightMask=%d;XYLightMask=%d;"
    "TargetForDefocus=%.3f;TargetForCavity=%.3f;CentZone_pix=%d;"
    "Width_um=%.1f;Height_um=%.1f;Area_um2=%.1f;"
    "m_dDistBtwAreas_um=%.1f;"
    "YCorrectionWidth_um=%.1f;RelHeightForX=%.3f;"
    "FindBlackCorners=%d;CavitySizeTolerance_perc=%.1f;"
    "AngleFiltration=%d;CheckYDiffer=%d;AllowedYDiffer_um=%.2f;"
    "EnableProfileUsing=%d;CompareConturAndProfileXResults=%d;"
    "MaxDiffBetweenConturAndProfile_um=%.2f",
    m_iCavityExp_us , m_iLaserExp_us , m_iDefocusExp_us ,
    (int) m_CavityEdge , m_dDefocusingThreshold , m_dDefocusingLongStep ,
    m_dDefocusingShortStep , m_iRingDefocusExp_us , m_iStraightDefocusExp_us ,
    m_bCavFocusRingLightOn | (m_bCavFocusStraightLightOn << 1) ,
    m_bCavXYRingLightOn | (m_bCavXYStraightLightOn << 1) ,
    m_dTargetForFocusExpAdjust , m_dNormBrightnessForCavity ,
    m_iCentralZoneWidth_pix ,
    m_dPlaneWidth_um , m_dPlaneHeight_um , m_dPlaneArea_um2 ,
    m_dYCorrectionWidth_um, m_dRelativeHeightForXSampling ,
    m_bFindBlackCorners , m_dCavitySizeTolerance_perc ,
    m_bAngleFiltrationOn , m_bCavCheckYdiffer , m_dAllowedYdiffer_um ,
    m_bCavEnableProfileUsing ,  m_bCavCompareContAndProfResults , 
    m_dMaxDiffBetweenXsOnConturAndProfile_um ) ;
  return AsString ;
}

bool CavityParams::FromString( LPCTSTR AsString )
{
  FXPropertyKit pk( AsString ) ;
  bool bOK = pk.GetInt( "CavityExp_us" , m_iCavityExp_us ) ;
  bOK |= pk.GetInt( "LaserExp_us" , m_iLaserExp_us ) ;
  bOK |= pk.GetInt( "DefocusExp_us" , m_iDefocusExp_us ) ;
  bOK |= pk.GetInt( "CavityEdge" , (int&) m_CavityEdge ) ;
  bOK |= pk.GetDouble( "DefocThres" , m_dDefocusingThreshold ) ;
  bOK |= pk.GetDouble( "DefocLongStep" , m_dDefocusingLongStep ) ;
  bOK |= pk.GetDouble( "DefocShortStep" , m_dDefocusingShortStep ) ;
  bOK |= pk.GetInt( "RingDefocExp_us" , m_iRingDefocusExp_us ) ;
  bOK |= pk.GetInt( "StraightDefocExp_us" , m_iStraightDefocusExp_us ) ;
  bOK |= pk.GetInt( "FocusLightMask" , m_bCavFocusRingLightOn ) ;
  m_bCavFocusStraightLightOn = (m_bCavFocusRingLightOn & 2) != 0;
  m_bCavFocusRingLightOn &= 1 ;
  bOK |= pk.GetInt( "XYLightMask" , m_bCavXYRingLightOn ) ;
  m_bCavXYStraightLightOn = (m_bCavXYRingLightOn & 2) != 0;
  m_bCavXYRingLightOn &= 1 ;
  bOK |= pk.GetDouble( "TargetForDefocus" , m_dTargetForFocusExpAdjust ) ;
  bOK |= pk.GetDouble( "TargetForCavity" , m_dNormBrightnessForCavity ) ;
  bOK |= pk.GetInt( "CentZone_pix" , m_iCentralZoneWidth_pix ) ;
  bOK |= pk.GetDouble( "Width_um" , m_dPlaneWidth_um ) ;
  bOK |= pk.GetDouble( "Height_um" , m_dPlaneHeight_um ) ;
  bOK |= pk.GetDouble( "Area_um2" , m_dPlaneArea_um2 ) ;
  bOK |= pk.GetDouble( "m_dDistBtwAreas_um" , m_dDistBetweenAreas_um ) ;
  bOK |= pk.GetInt( "FindBlackCorners" , m_bFindBlackCorners ) ;
  bOK |= pk.GetDouble( "CavitySizeTolerance_perc" , m_dCavitySizeTolerance_perc ) ;
  bOK |= pk.GetDouble( "YCorrectionWidth_um" , m_dYCorrectionWidth_um ) ;
  bOK |= pk.GetDouble( "RelHeightForX" , m_dRelativeHeightForXSampling ) ;
  bOK |= pk.GetInt( "AngleFiltration" , m_bAngleFiltrationOn ) ;
  bOK |= pk.GetInt( "CheckYDiffer" , m_bCavCheckYdiffer ) ;
  bOK |= pk.GetInt( "EnableProfileUsing" , m_bCavEnableProfileUsing ) ;
  bOK |= pk.GetInt( "CompareConturAndProfileXResults" , m_bCavCompareContAndProfResults ) ;
  bOK |= pk.GetDouble( "AllowedYDiffer_um" , m_dAllowedYdiffer_um ) ;
  bOK |= pk.GetDouble("MaxDiffBetweenConturAndProfile_um",
    m_dMaxDiffBetweenXsOnConturAndProfile_um);
  bOK |= pk.GetDouble("MaxAngleDiffBetweenInternalEdges_deg",
    m_dMaxAngleDiffBetweenInternalEdges_deg);
  return bOK ;
}

int CavityParams::RestoreCavityDataFromRegistry( LPCTSTR pPartFolder , bool bSetDefault )
{
  string CavityFolder( pPartFolder ) ;
  FXRegistry Reg( CavityFolder.c_str() );

  string Descr = Reg.GetRegiString( "Cavity" , "CavDescription" , "No Description" );
  int  iResult = (Descr != "No Description"); // No Description (should be filled in registry after first saving)
  m_Description = Descr;
  m_iCavityExp_us = Reg.GetRegiInt( "Cavity" , "CavityExposure_us" , bSetDefault ? 500 : m_iCavityExp_us );
  m_iDefocusExp_us = Reg.GetRegiInt( "Cavity" , "FocusExposure_us" , bSetDefault ? 400 : m_iDefocusExp_us );
  int iLightMask = Reg.GetRegiInt( "Cavity" , "FocusLight(1-Ring,2-Straight,3-Both)" , 
    bSetDefault ? 1 : (m_bCavFocusRingLightOn | (m_bCavFocusStraightLightOn<<1)));
  m_bCavFocusRingLightOn = (iLightMask & 1) != 0;
  m_bCavFocusStraightLightOn = (iLightMask & 2) != 0;
  iLightMask = Reg.GetRegiInt( "Cavity" , "XYLight(1-Ring,2-Straight,3-Both)" , 
    bSetDefault ? 2 : (m_bCavXYRingLightOn | (m_bCavXYStraightLightOn<<1)));
  m_bCavXYRingLightOn = (iLightMask & 1) != 0;
  m_bCavXYStraightLightOn = (iLightMask & 2) != 0;
  m_dTargetForFocusExpAdjust = Reg.GetRegiDouble( "Cavity" , "TargetForFocusExpAdjust" , 
    bSetDefault ? 0.88 : m_dTargetForFocusExpAdjust );
  m_dNormBrightnessForCavity = Reg.GetRegiDouble( "Cavity" , "TargetForCavityExpAdjust" , 
    bSetDefault ? 0.95 : m_dNormBrightnessForCavity );
  m_iCentralZoneWidth_pix = Reg.GetRegiInt( "Cavity" , "CentralZoneWidth_pix" , 
    bSetDefault ? 200 : m_iCentralZoneWidth_pix );
  m_dPlaneWidth_um = Reg.GetRegiDouble( "Cavity" , "CavityPlaneWidth_um" , 
    bSetDefault ? 250. : m_dPlaneWidth_um );
  m_dPlaneHeight_um = Reg.GetRegiDouble( "Cavity" , "CavityPlaneHeight_um" , 
    bSetDefault ? 900. : m_dPlaneHeight_um );
  m_dPlaneArea_um2 = Reg.GetRegiDouble( "Cavity" , "CavityPlaneArea_um2" , 
    bSetDefault ? m_dPlaneWidth_um * m_dPlaneHeight_um * 0.94 : m_dPlaneArea_um2);
  m_dDistBetweenAreas_um = Reg.GetRegiDouble( "Cavity" , "CavityDistBetweenAreas_um" , 
    bSetDefault ? 400. : m_dDistBetweenAreas_um );
  m_dCavitySizeTolerance_perc = Reg.GetRegiDouble( "Cavity" , "CavitySizeTolerance_perc" , 
    bSetDefault ? 10. : m_dCavitySizeTolerance_perc );
  m_bFindBlackCorners = Reg.GetRegiInt( "Cavity" , "FindBlackCorners" , 
    bSetDefault ? 0 : m_bFindBlackCorners );
  m_CavityEdge = (CAVITY_EDGE) Reg.GetRegiInt( "Cavity" , "CavityEdge(1-low,2-up)" , 
    bSetDefault ? 2 : m_CavityEdge );
  m_dYCorrectionWidth_um = Reg.GetRegiDouble( "Cavity" , "YCorrectionWidth_um" ,
    bSetDefault ? 0. : m_dYCorrectionWidth_um );
  m_dRelativeHeightForXSampling = Reg.GetRegiDouble( "Cavity" , "RelHeightForX(-0.5,0.5)" ,
    bSetDefault ? 0. : m_dRelativeHeightForXSampling );
  m_dDefocusingThreshold = Reg.GetRegiDouble( "Cavity" , "DefocusingThreshold" ,
    bSetDefault ? 0.7 : m_dDefocusingThreshold );
  m_dDefocusingLongStep = Reg.GetRegiDouble( "Cavity" , "DefocusingLongStep_um" ,
    bSetDefault ? 50. : m_dDefocusingLongStep );
  m_dDefocusingShortStep = Reg.GetRegiDouble( "Cavity" , "DefocusingShortStep_um" ,
    bSetDefault ? 10. : m_dDefocusingShortStep );

  m_bAngleFiltrationOn  = Reg.GetRegiInt( "Cavity" , 
    "AngleFiltration" , bSetDefault ? 0 : m_bAngleFiltrationOn ) ;
  m_bCavCheckYdiffer  = Reg.GetRegiInt( "Cavity" , 
    "CheckYDiffer" , bSetDefault ? 1 : m_bCavCheckYdiffer ) ;
  m_bCavEnableProfileUsing  = Reg.GetRegiInt( "Cavity" , 
    "EnableProfileUsing" , bSetDefault ? 0 : m_bCavEnableProfileUsing ) ;
  m_bCavCompareContAndProfResults  = Reg.GetRegiInt( "Cavity" , 
    "CompareConturAndProfileXResults" , bSetDefault ? 0 : m_bCavCompareContAndProfResults ) ;
  m_dAllowedYdiffer_um  = Reg.GetRegiDouble( "Cavity" , 
    "AllowedYDiffer_um" , bSetDefault ? 10. : m_dAllowedYdiffer_um ) ;
  m_dMaxDiffBetweenXsOnConturAndProfile_um = Reg.GetRegiDouble("Cavity",
    "MaxDiffBetweenConturAndProfile_um",
    bSetDefault ? 1.5 : m_dMaxDiffBetweenXsOnConturAndProfile_um);
  m_dMaxAngleDiffBetweenInternalEdges_deg = Reg.GetRegiDouble("Cavity",
    "MaxAngleDiffBetweenInternalEdges_deg",
    bSetDefault ? 1.5 : m_dMaxAngleDiffBetweenInternalEdges_deg);

  return iResult;
}

void CavityParams::SaveCavityDataToRegistry( LPCTSTR pPartsFolder , LPCTSTR PartName )
{
  string Folder( pPartsFolder ) ;
  Folder += "\\" ;
  Folder += PartName ;
  FXRegistry Reg( Folder.c_str() );

  Reg.WriteRegiString( "Cavity" , "CavDescription" , m_Description.c_str() );
  Reg.WriteRegiInt( "Cavity" , "CavityExposure_us" , m_iCavityExp_us );
  Reg.WriteRegiInt( "Cavity" , "FocusExposure_us" , m_iDefocusExp_us );
  Reg.WriteRegiInt( "Cavity" , "FocusLight(1-Ring,2-Straight,3-Both)" ,
    (m_bCavFocusRingLightOn | (m_bCavFocusStraightLightOn << 1)) );
  Reg.WriteRegiInt( "Cavity" , "XYLight(1-Ring,2-Straight,3-Both)" ,
    (m_bCavXYRingLightOn | (m_bCavXYStraightLightOn << 1)) );
  Reg.WriteRegiDouble( "Cavity" , "TargetForFocusExpAdjust" ,
    m_dTargetForFocusExpAdjust );
  Reg.WriteRegiDouble( "Cavity" , "TargetForCavityExpAdjust" ,
    m_dNormBrightnessForCavity );
  Reg.WriteRegiInt( "Cavity" , "CentralZoneWidth_pix" ,
    m_iCentralZoneWidth_pix );
  Reg.WriteRegiDouble( "Cavity" , "CavityPlaneWidth_um" ,
    m_dPlaneWidth_um );
  Reg.WriteRegiDouble( "Cavity" , "CavityPlaneHeight_um" ,
    m_dPlaneHeight_um );
  Reg.WriteRegiDouble( "Cavity" , "CavityPlaneArea_um2" ,
    m_dPlaneArea_um2 );
  Reg.WriteRegiDouble( "Cavity" , "CavityDistBetweenAreas_um" ,
    m_dDistBetweenAreas_um );
  Reg.WriteRegiDouble( "Cavity" , "CavitySizeTolerance_perc" ,
    m_dCavitySizeTolerance_perc );
  Reg.WriteRegiInt( "Cavity" , "FindBlackCorners" ,
    m_bFindBlackCorners );
  Reg.WriteRegiInt( "Cavity" , "CavityEdge(1-low,2-up)" ,
    m_CavityEdge );
  Reg.WriteRegiDouble( "Cavity" , "YCorrectionWidth_um" ,
    m_dYCorrectionWidth_um );
  Reg.WriteRegiDouble( "Cavity" , "RelHeightForX(-0.5,0.5)" ,
    m_dRelativeHeightForXSampling );
  Reg.WriteRegiDouble( "Cavity" , "DefocusingThreshold" ,
    m_dDefocusingThreshold );
  Reg.WriteRegiDouble( "Cavity" , "DefocusingLongStep_um" ,
    m_dDefocusingLongStep );
  Reg.WriteRegiDouble( "Cavity" , "DefocusingShortStep_um" ,
    m_dDefocusingShortStep );

  Reg.WriteRegiInt( "Cavity" , "AngleFiltration" , m_bAngleFiltrationOn ) ;
  Reg.WriteRegiInt( "Cavity" , "CheckYDiffer" , m_bCavCheckYdiffer ) ;
  Reg.WriteRegiInt( "Cavity" , "EnableProfileUsing" , m_bCavEnableProfileUsing ) ;
  Reg.WriteRegiInt( "Cavity" , "CompareConturAndProfileXResults" ,
    m_bCavCompareContAndProfResults ) ;
  Reg.WriteRegiDouble( "Cavity" , "AllowedYDiffer_um" , m_dAllowedYdiffer_um ) ;
  Reg.WriteRegiDouble( "Cavity" , "MaxDiffBetweenConturAndProfile_um" ,
    m_dMaxDiffBetweenXsOnConturAndProfile_um ) ;
  Reg.WriteRegiDouble( "Cavity" , "MaxAngleDiffBetweenInternalEdges_deg" ,
    m_dMaxAngleDiffBetweenInternalEdges_deg) ;
}



int BlankParams::RestoreBlankDataFromRegistry(
  LPCTSTR pPartFolder , bool bGauge , bool bSetDefault )
{
  string BlankFolder( pPartFolder ) ;
  FXRegistry Reg( BlankFolder.c_str() );
  m_bIsGauge = bGauge ;
  LPCTSTR pSubfolderName = bGauge ? "Gauge" : "Blank" ;

  string Descr = Reg.GetRegiString( pSubfolderName , 
    bGauge ? "GaugeDescription" : "BlankDescription" , "No Description" );
  int  iResult = (Descr != "No Description"); // No Description (should be filled in registry after first saving)
  m_Description = Descr;
  int iLightMask = Reg.GetRegiInt( pSubfolderName , "BlankLight(1-Ring,2-Straight,3-Both)" , 
    bSetDefault ? 2 : (m_bBlankXYRingLightOn | (m_bBlankXYStraightLightOn << 1)));
  m_bBlankXYRingLightOn = ((iLightMask & 1) != 0);
  m_bBlankXYStraightLightOn = ((iLightMask & 2) != 0);
  m_iBlankExp_us = Reg.GetRegiInt( pSubfolderName , "BlankExposure_us" , 
    bSetDefault? 800 : m_iBlankExp_us );
  m_dBlankWidth_um = Reg.GetRegiDouble( pSubfolderName , "BlankWidth_um" , 
    bSetDefault ? 800. : m_dBlankWidth_um );
  m_dBlankHeight_um = Reg.GetRegiDouble( pSubfolderName , "BlankHeight_um" , 
    bSetDefault ? 900. : m_dBlankHeight_um );
  m_dSizeTolerance_perc = Reg.GetRegiDouble( pSubfolderName , "SizeTolerance_perc" , 
    bSetDefault? 3. : m_dSizeTolerance_perc );
  m_UsedBlankEdge = (SQ_EDGE_AND_CORNERS) Reg.GetRegiInt( pSubfolderName , "BlankEdge(LURL-1234)" , 
    bSetDefault ? 2 : m_UsedBlankEdge );
  m_dYShiftForBlank_um = Reg.GetRegiDouble( pSubfolderName , "YShiftForBlank_um" , 
    bSetDefault ? 0. : m_dYShiftForBlank_um );
  m_dParallelismTol_deg = Reg.GetRegiDouble( pSubfolderName , "ParallelismTol_deg" ,
    bSetDefault ? 2. : m_dParallelismTol_deg );

  return iResult;

}

FXString BlankParams::ToString()
{
  FXString AsString ;
  AsString.Format( m_bIsGauge ?
    "GaugeExp_us=%d;"
    "GaugeEdge=%d;GaugeLightMask=%d;"
    "GaugeWidth_um=%.1f;GaugeHeight_um=%.1f;"
    "GaugeSizeTolerance_perc=%.1f;"
    "YShiftorGauge=%.1f;"
    : "BlankExp_us=%d;"
    "BlankEdge=%d;BlankLightMask=%d;"
    "BlankWidth_um=%.1f;BlankHeight_um=%.1f;"
    "BlankSizeTolerance_perc=%.1f;"
    "BlankYShift=%.1f;",
    m_iBlankExp_us , m_UsedBlankEdge , 
    m_bBlankXYRingLightOn | (m_bBlankXYStraightLightOn << 1) ,
    m_dBlankWidth_um , m_dBlankHeight_um , m_dSizeTolerance_perc , m_dYShiftForBlank_um ) ;
  return AsString ;
}

bool BlankParams::FromString( LPCTSTR AsString )
{
  FXPropertyKit pk( AsString ) ;
  FXString BlankOrGauge( m_bIsGauge ? "Gauge" : "Blank" ) ;
  bool bOK = pk.GetInt( BlankOrGauge + "Exp_us" , m_iBlankExp_us ) ;
  bOK |= pk.GetInt( BlankOrGauge + "Edge" , (int&) m_UsedBlankEdge ) ;
  bOK |= pk.GetInt( BlankOrGauge + "LightMask" , m_bBlankXYRingLightOn ) ;
  m_bBlankXYStraightLightOn = (m_bBlankXYRingLightOn & 2) != 0;
  m_bBlankXYRingLightOn &= 1 ;
  bOK |= pk.GetDouble( BlankOrGauge + "Width_um" , m_dBlankWidth_um ) ;
  bOK |= pk.GetDouble( BlankOrGauge + "Height_um" , m_dBlankHeight_um ) ;
  bOK |= pk.GetDouble( BlankOrGauge + "SizeTolerance_perc" , m_dSizeTolerance_perc ) ;
  bOK |= pk.GetDouble( BlankOrGauge + "YShift" , m_dYShiftForBlank_um ) ;
  return bOK ;
}

int PartParams::RestorePartDataFromRegistry(
  LPCTSTR pFolderForParts , LPCTSTR pPartName , bool bSetDefault )
{
  if ( !pFolderForParts )
    pFolderForParts = "TheFileX\\Micropoint\\PartsData" ;
  string PartFolder( pFolderForParts ) ;
  PartFolder += '\\' ;
  FXRegistry Reg( PartFolder.c_str() );

  m_Name = Reg.GetRegiString( pPartName , "Name" , pPartName ) ;
  m_Description = Reg.GetRegiString( pPartName , "Description" , "No Description" ) ;
  PartFolder += pPartName ;
  m_Cavity.RestoreCavityDataFromRegistry( PartFolder.c_str() , bSetDefault ) ;
  m_Blank.RestoreBlankDataFromRegistry( PartFolder.c_str() , false , bSetDefault ) ;
  m_Gauge.RestoreBlankDataFromRegistry( PartFolder.c_str() , true , bSetDefault ) ;

  return 1 ;
}


FXString PartParams::ToString()
{
  FXString AsString ;
  AsString.Format( "Name=%s;Description=%s;" , m_Name.c_str() , m_Description.c_str() ) ;
  AsString += m_Cavity.ToString() ;
  AsString += m_Blank.ToString() ;
  AsString += m_Gauge.ToString() ;
  return AsString ;
}

bool PartParams::FromString( LPCTSTR AsString )
{
  FXPropertyKit pk( AsString ) ;
  FXString Tmp ;
  bool bOK = pk.GetString( "Name" , Tmp ) ;
  if ( bOK )
    m_Name = (LPCTSTR)Tmp ;
  if ( pk.GetString( "Description" , Tmp ) )
  {
    bOK = true ;
    m_Description = (LPCTSTR) Tmp ;
  }
  bOK |= m_Cavity.FromString( AsString ) ;
  bOK |= m_Cavity.FromString( AsString ) ;
  bOK |= m_Cavity.FromString( AsString ) ;


  return bOK ;
}


int MPPT::SelectPartByName( FXString& SelectedPartName , bool bReportToEngine )
{
  FXString PartName = SelectedPartName.MakeUpper() ;
  if ( !SelectCurrentPart( PartName ) )
  {
    SEND_GADGET_ERR( "Unknown part name %s" , (LPCTSTR) PartName );
    FXString Msg ;
    Msg.Format( _T( "Part %s doesn't exists. Create with default parameters?" ) , (LPCTSTR) PartName ) ;
    if ( AfxMessageBox( Msg , MB_YESNO ) == IDYES )
    {
      // No such part in list, new part created
      m_CurrentPart.RestorePartDataFromRegistry( NULL , "Default" ) ;
      m_CurrentPart.m_Name = PartName ;
      m_CurrentPart.m_Description = (LPCTSTR) GetTimeAsString( "Created from default at " ) ;
      SavePartDataToRegistryEx( m_CurrentPart ) ;
      SEND_GADGET_INFO( "New part %s created" , (LPCTSTR) PartName );
      m_PartName = PartName ;
      RestoreKnownParts() ;
    }
    else if ( bReportToEngine )
    {
      SendMessageToEngine( "Error; No such part" , (LPCTSTR) (m_GadgetName + _T( "_AnswerForWedge" )) );
      return 0 ;
    }
  }
  else if ( bReportToEngine )
    SendMessageToEngine( "OK" , (LPCTSTR) (m_GadgetName + _T( "_AnswerForWedge" )) );

//   SelectCurrentPart( PartName ) ;
  m_PartName = PartName ;
  return 1 ;
}


bool MPPT::SetParametersToTVObject( LPCTSTR pParams )
{
  CTextFrame * pCommand = CreateTextFrame( pParams ,
    "SetObjectProp" );
  return PutFrame( GetOutputConnector( 3 ) , pCommand );
}

bool MPPT::SetMeasureConturParameters()
{
  FXString PartSizes;
  switch ( m_WorkingMode )
  {
  case MPPTWM_Down:
    {
      double dTol = m_CurrentPart.m_Cavity.m_dCavitySizeTolerance_perc / 100.;
      dTol *= 2.;
      cmplx cRadius_um(m_CurrentPart.m_Cavity.m_dPlaneWidth_um * 3 + m_CurrentPart.m_Cavity.m_dDistBetweenAreas_um / 2. ,
        m_CurrentPart.m_Cavity.m_dPlaneHeight_um * 2);
      cmplx cRadius_pix = cRadius_um / m_dScale_um_per_pix;
      CSize Radius(ROUND(cRadius_pix.real()) , ROUND(cRadius_pix.imag()));
      int iXOffset = m_LastROICenter.x - Radius.cx;
      if (iXOffset < 460)
        iXOffset = 460;
      int iYOffset = m_LastROICenter.y - Radius.cy;
      if (iYOffset < 50)
        iYOffset = 50;
      CSize AOISize_pix( Radius.cx * 2 , Radius.cy * 2 );
      if (iXOffset + AOISize_pix.cx > m_LastROICenter.x + 460)
        AOISize_pix.cx = m_LastROICenter.x + 460 ;
      if (iYOffset + AOISize_pix.cy > m_LastROICenter.y + 550)
        AOISize_pix.cy = m_LastROICenter.y + 550 ;

      PartSizes.Format( "name=cavity_bottom;"
        "xoffset=%d;yoffset=%d;width=%d;height=%d;"
        "width_min=%d;height_min=%d;"
        "width_max=%d;height_max=%d;area_min=%d;area_max=%d;" ,
        iXOffset , iYOffset , AOISize_pix.cx , AOISize_pix.cy ,        
        ROUND( m_CurrentPart.m_Cavity.m_dPlaneWidth_um * (1. - dTol) / m_dScale_um_per_pix ) ,
        ROUND( m_CurrentPart.m_Cavity.m_dPlaneHeight_um * (1. - dTol) / m_dScale_um_per_pix ) ,
        ROUND( m_CurrentPart.m_Cavity.m_dPlaneWidth_um * (1. + dTol) / m_dScale_um_per_pix ) ,
        ROUND( m_CurrentPart.m_Cavity.m_dPlaneHeight_um * (1. + dTol) / m_dScale_um_per_pix ) ,
        ROUND( m_CurrentPart.m_Cavity.m_dPlaneArea_um2 * (1. - dTol * 2.)
        / (m_dScale_um_per_pix * m_dScale_um_per_pix) ) ,
        ROUND( m_CurrentPart.m_Cavity.m_dPlaneArea_um2 * (1. + dTol * 2)
        / (m_dScale_um_per_pix * m_dScale_um_per_pix) )
      );
    }
    break;
  case MPPTWM_UpFront:
    {
#define X_MIN 400
#define Y_MIN  48
      BlankParams& Blank = m_iNProcessedBlanks > 0 ? m_CurrentPart.m_Blank : m_CurrentPart.m_Gauge;
      double dTol = Blank.m_dSizeTolerance_perc * 4./ 100.;
      double dArea_um = Blank.m_dBlankWidth_um * Blank.m_dBlankHeight_um ;
      cmplx cRadius_um(Blank.m_dBlankWidth_um * 2 , Blank.m_dBlankHeight_um * 2);
      cmplx cRadius_pix = cRadius_um / m_dScale_um_per_pix;
      CSize Radius(ROUND(cRadius_pix.real()) , ROUND(cRadius_pix.imag()));
      int iXOffset = m_LastROICenter.x - Radius.cx;
      if (iXOffset < X_MIN)
        iXOffset = X_MIN;
      int iYOffset = m_LastROICenter.y - Radius.cy;
      if (iYOffset < Y_MIN)
        iYOffset = Y_MIN;
      CSize AOISize_pix( Radius.cx * 2 , Radius.cy * 2 );
      if (iXOffset + AOISize_pix.cx > m_LastROI.right - X_MIN)
        AOISize_pix.cx = m_LastROI.Width() - 2 * X_MIN;
      if (iYOffset + AOISize_pix.cy > m_LastROI.bottom - Y_MIN)
        AOISize_pix.cy = m_LastROI.Height() - 2 * Y_MIN ;
      PartSizes.Format( "name=part_top;"
        "xoffset=%d;yoffset=%d;width=%d;height=%d;"
        "width_min=%d;height_min=%d;"
        "width_max=%d;height_max=%d;area_min=%d;area_max=%d;" ,
        iXOffset , iYOffset , AOISize_pix.cx , AOISize_pix.cy ,
        ROUND( Blank.m_dBlankWidth_um * (1. - dTol) / m_dScale_um_per_pix ) ,
        ROUND( Blank.m_dBlankHeight_um * (1. - dTol) / m_dScale_um_per_pix ) ,
        ROUND( Blank.m_dBlankWidth_um * (1. + dTol) / m_dScale_um_per_pix ) ,
        ROUND( Blank.m_dBlankHeight_um * (1. + dTol) / m_dScale_um_per_pix ) ,
        ROUND( dArea_um * (1. - dTol * 2.) / (m_dScale_um_per_pix * m_dScale_um_per_pix) ) ,
        ROUND( dArea_um * (1. + dTol * 2.) / (m_dScale_um_per_pix * m_dScale_um_per_pix) )
      );
    }
    break;
  }

  if ( !PartSizes.IsEmpty() )
    return SetParametersToTVObject( PartSizes );

  return false;
}

