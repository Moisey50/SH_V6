#pragma once

class SmallSpotsStatistics
{
public:
  double m_dArea , m_dArea_Std , m_dArea_75percMin , m_dArea_75percMax , m_dArea_95percMin , m_dArea_95percMax ;
  double m_dDia , m_dDia_Std , m_dDia_75percMin , m_dDia_75percMax , m_dDia_95percMin , m_dDia_95percMax ;
  double m_dPerimeter , m_dPerimeter_Std , m_dPerimeter_75percMin , 
    m_dPerimeter_75percMax , m_dPerimeter_95percMin , m_dPerimeter_95percMax ;
  double m_dBoxRatio , m_dBoxRatio_Std , m_dBoxRatio_75percMin ,
    m_dBoxRatio_75percMax , m_dBoxRatio_95percMin , m_dBoxRatio_95percMax ;
  double m_dXCentroid , m_dXCentroid_Std , m_dXCentroid_75percMin ,
    m_dXCentroid_75percMax , m_dXCentroid_95percMin , m_dXCentroid_95percMax ;
  double m_dYCentroid , m_dYCentroid_Std , m_dYCentroid_75percMin ,
    m_dYCentroid_75percMax , m_dYCentroid_95percMin , m_dYCentroid_95percMax ;
  double m_dXMid , m_dXMid_Std , m_dXMid_75percMin ,
    m_dXMid_75percMax , m_dXMid_95percMin , m_dXMid_95percMax ;
  double m_dYMid , m_dYMid_Std , m_dYMid_75percMin ,
    m_dYMid_75percMax , m_dYMid_95percMin , m_dYMid_95percMax ;
  double m_dBoundLeft , m_dBoundLeft_Std , m_dBoundLeft_75percMin ,
    m_dBoundLeft_75percMax , m_dBoundLeft_95percMin , m_dBoundLeft_95percMax ;
  double m_dBoundTop , m_dBoundTop_Std , m_dBoundTop_75percMin ,
    m_dBoundTop_75percMax , m_dBoundTop_95percMin , m_dBoundTop_95percMax ;
  double m_dBoundRight , m_dBoundRight_Std , m_dBoundRight_75percMin ,
    m_dBoundRight_75percMax , m_dBoundRight_95percMin , m_dBoundRight_95percMax ;
  double m_dBoundBottom , m_dBoundBottom_Std , m_dBoundBottom_75percMin ,
    m_dBoundBottom_75percMax , m_dBoundBottom_95percMin , m_dBoundBottom_95percMax ;
  double m_dBoundWidth , m_dBoundWidth_Std , m_dBoundWidth_75percMin ,
    m_dBoundWidth_75percMax , m_dBoundWidth_95percMin , m_dBoundWidth_95percMax ;
  double m_dBoundHeight , m_dBoundHeight_Std , m_dBoundHeight_75percMin ,
    m_dBoundHeight_75percMax , m_dBoundHeight_95percMin , m_dBoundHeight_95percMax ;
  double m_dSpacing , m_dSpacing_Std , m_dSpacing_75percMin ,
    m_dSpacing_75percMax , m_dSpacing_95percMin , m_dSpacing_95percMax ;
  double m_dRuling_lpcm , m_dRuling_lpcm_Std , m_dRuling_lpcm_75percMin ,
    m_dRuling_lpcm_75percMax , m_dRuling_lpcm_95percMin , m_dRuling_lpcm_95percMax ;
  double m_dRuling_lpi , m_dRuling_lpi_Std , m_dRuling_lpi_75percMin ,
    m_dRuling_lpi_75percMax , m_dRuling_lpi_95percMin , m_dRuling_lpi_95percMax ;
  double m_dDotPercent , m_dDotPercent_Std , m_dDotPercent_75percMin ,
    m_dDotPercent_75percMax , m_dDotPercent_95percMin , m_dDotPercent_95percMax ;

  SmallSpotsStatistics() { Reset(); }
  void Reset() { memset( this , 0 , sizeof( *this ) ) ; }

  LPCTSTR GetCaption()
  {
    return _T( "//Data placement for For Dots\n"
      "// Date & Time and #,    Type   ,Angle_deg,  Area ,Area_Std,dArea_75Min,dArea_75Max,Area_95Min,Area_95%Max,"
      "  Dia ,Dia_Std,Dia_75%Min,Dia_75%Max,Dia_95%Min,Dia_95%Max,"
      "   Per ,Per_Std,Per_75%Min,Per_75%Max,Per_95%Min,Per_95%Max,"
      " BoxRat,BoxRat_Std,BoxRat_75%Min,BoxRat_75%Max,BoxRat_95%Min,BoxRat_95%Max,"
      " XCentr,XCentr_Std,XCentr_75%Min,XCentr_75%Max,XCentr_95%Min,XCentr_95%Max,"
      " YCentr,YCentr_Std,YCentr_75%Min,YCentr_75%Max,YCentr_95%Min,YCentr_95%Max,"
      "   XMid,  XMid_Std,XMid_75%Min,XMid_75%Max,XMid_95%Min,XMid_95%Max,"
      "   YMid,  YMid_Std,YMid_75%Min,YMid_75%Max,YMid_95%Min,YMid_95%Max,"
      "   BndL,  BndL_Std,BndL_75%Min,BndL_75%Max,BndL_95%Min,BndL_95%Max,"
      "   BndT,  BndT_Std,BndT_75%Min,BndT_75%Max,BndT_95%Min,BndT_95%Max,"
      "   BndR,  BndR_Std,BndR_75%Min,BndR_75%Max,BndR_95%Min,BndR_95%Max,"
      "   BndB,  BndB_Std,BndB_75%Min,BndB_75%Max,BndB_95%Min,BndB_95%Max,"
      "   BndW,  BndW_Std,BndW_75%Min,BndW_75%Max,BndW_95%Min,BndW_95%Max,"
      "   BndH,  BndH_Std,BndH_75%Min,BndH_75%Max,BndH_95%Min,BndH_95%Max,"
      "Spacing, Space_Std, Spc_75%Min, Spc_75%Max, Spc_95%Min, Spc_95%Max,"
      "   LPCM,  LPCM_Std,LPCM_75%Min,LPCM_75%Max,LPCM_95%Min,LPCM_95%Max,"
      "    DPI,   DPI_Std, DPI_75%Min, DPI_75%Max, DPI_95%Min, DPI_95%Max,"
      "    Dot%, Dot%_Std,Dot%_75%Min,Dot%_75%Max,Dot%_95%Min,Dot%_95%Max\n"
      "//Data placement for For Lines\n"
      "// Date & Time and #,    Type   ,"
      "   LeadRag,LeadRag_Std,LeadRag_75%Min,LeadRag_75%Max,LeadRag_95%Min,LeadRag_95%Max,"
      "   TrailRag,TrailRag_Std,TrailRag_75%Min,TrailRag_75%Max,TrailRag_95%Min,TrailRag_95%Max,"
      "   LeadAng,LeadAng_Std,LeadAng_75%Min,LeadAng_75%Max,LeadAng_95%Min,LeadAng_95%Max,"
      "   TrailAng,TrailAng_Std,TrailAng_75%Min,TrailAng_75%Max,TrailAng_95%Min,TrailAng_95%Max,"
      "   Width,Width_Std,Width_75%Min,Width_75%Max,Width_95%Min,Width_95%Max,"
      "   BndL,  BndL_Std,BndL_75%Min,BndL_75%Max,BndL_95%Min,BndL_95%Max,"
      "   BndT,  BndT_Std,BndT_75%Min,BndT_75%Max,BndT_95%Min,BndT_95%Max,"
      "   BndR,  BndR_Std,BndR_75%Min,BndR_75%Max,BndR_95%Min,BndR_95%Max,"
      "   BndB,  BndB_Std,BndB_75%Min,BndB_75%Max,BndB_95%Min,BndB_95%Max,"
      "   BndH,  BndH_Std,BndH_75%Min,BndH_75%Max,BndH_95%Min,BndH_95%Max,"
      "   Xmid,Xmid_Std,Xmid_75%Min,Xmid_75%Max,Xmid_95%Min,Xmid_95%Max,"
      "   Ymid,Ymid_Std,Ymid_75%Min,Ymid_75%Max,Ymid_95%Min,Ymid_95%Max,"
      "   Spacing,Space_Std, Spc_75%Min, Spc_75%Max, Spc_95%Min, Spc_95%Max\n"
    ) ;
  }
  FXString ToString( int iIndex , double dAngle_deg )
  {
    FXString Out ;
//    Out.Format( _T( ",%5d,%9.3f,"
    Out.Format( _T( "%9.3f,"
        "%7.1f,%8.1f,%11.1f,%11.1f,%10.1f,%11.1f," // Area
      "%6.2f,%7.2f,%10.1f,%10.1f,%10.1f,%10.1f," //Dia
      "%7.1f,%7.1f,%10.1f,%10.1f,%10.1f,%10.1f," // Perimeter
      "%7.3f,%10.4f,%13.1f,%13.1f,%13.1f,%13.1f," // Box Ratio
      "%7.1f,%10.1f,%13.1f,%13.1f,%13.1f,%13.1f," // XCentroid
      "%7.1f,%10.1f,%13.1f,%13.1f,%13.1f,%13.1f," // Y Centroid
      "%7.1f,%10.1f,%11.1f,%11.1f,%11.1f,%11.1f," // XMid
      "%7.1f,%10.1f,%11.1f,%11.1f,%11.1f,%11.1f," // YMid
      "%7.1f,%10.1f,%11.1f,%11.1f,%11.1f,%11.1f,"
      "%7.1f,%10.1f,%11.1f,%11.1f,%11.1f,%11.1f,"
      "%7.1f,%10.1f,%11.1f,%11.1f,%11.1f,%11.1f,"
      "%7.1f,%10.1f,%11.1f,%11.1f,%11.1f,%11.1f,"
      "%7.1f,%10.1f,%11.1f,%11.1f,%11.1f,%11.1f,"
      "%7.1f,%10.1f,%11.1f,%11.1f,%11.1f,%11.1f,"
      "%7.1f,%10.1f,%11.1f,%11.1f,%11.1f,%11.1f,"
      "%7.1f,%10.1f,%11.1f,%11.1f,%11.1f,%11.1f,"
      "%7.1f,%10.1f,%11.1f,%11.1f,%11.1f,%11.1f,"
      "%7.1f,%10.1f,%11.1f,%11.1f,%11.1f,%11.1f"),
    /*iIndex ,*/ dAngle_deg ,
    m_dArea , m_dArea_Std , m_dArea_75percMin , m_dArea_75percMax , m_dArea_95percMin , m_dArea_95percMax ,
    m_dDia , m_dDia_Std , m_dDia_75percMin , m_dDia_75percMax , m_dDia_95percMin , m_dDia_95percMax ,
    m_dPerimeter , m_dPerimeter_Std , m_dPerimeter_75percMin ,
    m_dPerimeter_75percMax , m_dPerimeter_95percMin , m_dPerimeter_95percMax ,
    m_dBoxRatio , m_dBoxRatio_Std , m_dBoxRatio_75percMin ,
    m_dBoxRatio_75percMax , m_dBoxRatio_95percMin , m_dBoxRatio_95percMax ,
    m_dXCentroid , m_dXCentroid_Std , m_dXCentroid_75percMin ,
    m_dXCentroid_75percMax , m_dXCentroid_95percMin , m_dXCentroid_95percMax ,
    m_dYCentroid , m_dYCentroid_Std , m_dYCentroid_75percMin ,
    m_dYCentroid_75percMax , m_dYCentroid_95percMin , m_dYCentroid_95percMax ,
    m_dXMid , m_dXMid_Std , m_dXMid_75percMin ,
    m_dXMid_75percMax , m_dXMid_95percMin , m_dXMid_95percMax ,
    m_dYMid , m_dYMid_Std , m_dYMid_75percMin ,
    m_dYMid_75percMax , m_dYMid_95percMin , m_dYMid_95percMax ,
    m_dBoundLeft , m_dBoundLeft_Std , m_dBoundLeft_75percMin ,
    m_dBoundLeft_75percMax , m_dBoundLeft_95percMin , m_dBoundLeft_95percMax ,
    m_dBoundTop , m_dBoundTop_Std , m_dBoundTop_75percMin ,
    m_dBoundTop_75percMax , m_dBoundTop_95percMin , m_dBoundTop_95percMax ,
    m_dBoundRight , m_dBoundRight_Std , m_dBoundRight_75percMin ,
    m_dBoundRight_75percMax , m_dBoundRight_95percMin , m_dBoundRight_95percMax ,
    m_dBoundBottom , m_dBoundBottom_Std , m_dBoundBottom_75percMin ,
    m_dBoundBottom_75percMax , m_dBoundBottom_95percMin , m_dBoundBottom_95percMax ,
    m_dBoundWidth , m_dBoundWidth_Std , m_dBoundWidth_75percMin ,
    m_dBoundWidth_75percMax , m_dBoundWidth_95percMin , m_dBoundWidth_95percMax ,
    m_dBoundHeight , m_dBoundHeight_Std , m_dBoundHeight_75percMin ,
    m_dBoundHeight_75percMax , m_dBoundHeight_95percMin , m_dBoundHeight_95percMax ,
    m_dSpacing , m_dSpacing_Std , m_dSpacing_75percMin ,
    m_dSpacing_75percMax , m_dSpacing_95percMin , m_dSpacing_95percMax ,
    m_dRuling_lpcm , m_dRuling_lpcm_Std , m_dRuling_lpcm_75percMin ,
    m_dRuling_lpcm_75percMax , m_dRuling_lpcm_95percMin , m_dRuling_lpcm_95percMax ,
    m_dRuling_lpi , m_dRuling_lpi_Std , m_dRuling_lpi_75percMin ,
    m_dRuling_lpi_75percMax , m_dRuling_lpi_95percMin , m_dRuling_lpi_95percMax ,
    m_dDotPercent , m_dDotPercent_Std , m_dDotPercent_75percMin ,
    m_dDotPercent_75percMax , m_dDotPercent_95percMin , m_dDotPercent_95percMax 
    );

    return Out ;
  }
};

class LinesAsSpotsStatistics
{
public:
  double m_dDist_um , m_dDist_um_Std , m_dDist_um_75percMin , 
    m_dDist_um_75percMax , m_dDist_um_95percMin , m_dDist_um_95percMax ;
  double m_dLeadRaggedness , m_dLeadRaggedness_Std , m_dLeadRaggedness_75percMin , 
    m_dLeadRaggedness_75percMax , m_dLeadRaggedness_95percMin , m_dLeadRaggedness_95percMax ;
  double m_dTrailRaggedness , m_dTrailRaggedness_Std , m_dTrailRaggedness_75percMin , 
    m_dTrailRaggedness_75percMax , m_dTrailRaggedness_95percMin , m_dTrailRaggedness_95percMax ;
  double m_dWidth_um , m_dWidth_Std_um , m_dWidth_um_75percMin ,
    m_dWidth_um_75percMax , m_dWidth_um_95percMin , m_dWidth_um_95percMax ;
  double m_dMinWidth_um , m_dMaxWidth_um ;
  double m_dStdWidth_um , m_dStdWidth_um_Std , m_dStdWidth_um_75percMin ,
    m_dStdWidth_um_75percMax , m_dStdWidth_um_95percMin , m_dStdWidth_um_95percMax ;
  double m_dLeadAngle_deg , m_dLeadAngleStd_deg , m_dTrailAngle_deg , m_dTrailAngleStd_deg ;
  double m_dBoundLeft_um , m_dBoundLeft_um_Std , m_dBoundLeft_um_75percMin ,
    m_dBoundLeft_um_75percMax , m_dBoundLeft_um_95percMin , m_dBoundLeft_um_95percMax ;
  double m_dBoundRight_um , m_dBoundRight_um_Std , m_dBoundRight_um_75percMin , 
    m_dBoundRight_um_75percMax , m_dBoundRight_um_95percMin , m_dBoundRight_um_95percMax ;
  double m_dBoundTop_um , m_dBoundTop_um_Std , m_dBoundTop_um_75percMin , 
    m_dBoundTop_um_75percMax , m_dBoundTop_um_95percMin , m_dBoundTop_um_95percMax ;
  double m_dBoundBottom_um , m_dBoundBottom_um_Std , m_dBoundBottom_um_75percMin , 
    m_dBoundBottom_um_75percMax , m_dBoundBottom_um_95percMin , m_dBoundBottom_um_95percMax ;
  double m_dBoundHeight_um , m_dBoundHeight_um_Std , m_dBoundHeight_um_75percMin ,
    m_dBoundHeight_um_75percMax , m_dBoundHeight_um_95percMin , m_dBoundHeight_um_95percMax ;
  double m_dXmid_um , m_dXmid_um_Std , m_dXmid_um_75percMin ,
    m_dXmid_um_75percMax , m_dXmid_um_95percMin , m_dXmid_um_95percMax ;
  double m_dYmid_um , m_dYmid_um_Std , m_dYmid_um_75percMin ,
    m_dYmid_um_75percMax , m_dYmid_um_95percMin , m_dYmid_um_95percMax ;

  LinesAsSpotsStatistics() { Reset(); }
  void Reset() { memset(this, 0, sizeof(*this)); }

  //rowNum, upperAvr, LowerAvr, CenterAngle

  LPCTSTR GetCaption()
  {
    return _T("\n//#  ,UpperConturAvrAngle, LowerConturAvrAngle, CentersAngle \n"
   
    );
  }
  FXString ToString(int iIndex, double dUpperConturAngle, double dLowerConturAngle, double dCentersAngle)
  {
    FXString Out;
    Out.Format(_T("%5d,%9.5f,%9.5f,%9.5f\n"),
      iIndex, dUpperConturAngle, dLowerConturAngle,
      dCentersAngle
    );

    return Out;
  }
};