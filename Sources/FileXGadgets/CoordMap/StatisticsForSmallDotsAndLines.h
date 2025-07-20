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
    return _T( "//#  ,Angle_deg,  Area ,Area_Std,dArea_75Min,dArea_75Max,Area_95Min,Area_95%Max,"
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
    "    Dot%, Dot%_Std,Dot%_75%Min,Dot%_75%Max,Dot%_95%Min,Dot%_95%Max"
    ) ;
  }
  FXString ToString( int iIndex , double dAngle_deg )
  {
    FXString Out ;
    Out.Format( _T("%5d,%9.3f,"
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
      "%7.1f,%10.1f,%11.1f,%11.1f,%11.1f,%11.1f,"
      "\n"),
    iIndex , dAngle_deg ,
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