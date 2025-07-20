// ExtremumMeas.h.h : Implementation of the CExtremumMeas class


#include "StdAfx.h"
#include "ExtremumMeas.h"
#include "math/hbmath.h"
#include "helpers/FramesHelper.h"
#include <math\PlaneGeometry.h>
#include "imageproc/Utilities.h"
#include <imageproc/seekspots.h>

#define THIS_MODULENAME "ExtremumMeas"

extern char TVDB400_PLUGIN_NAME[APP_NAME_MAXLENGTH] ; 

// ExtremumMeas.h.h : Implementation of the CExtremumMeas class
IMPLEMENT_RUNTIME_GADGET_EX(CExtremumMeas, CFilterGadget, "Video.recognition" , TVDB400_PLUGIN_NAME);

CExtremumMeas::CExtremumMeas(void)
{
    m_pInput = new CInputConnector(vframe);
    m_pOutput = new COutputConnector(vframe);
    m_pDuplexConnector = new CDuplexConnector( this , transparent, transparent) ;
    m_pObjects = m_pNewObjects = m_pOldObjects = NULL ;
    m_iAverage_pix = 7 ;
    m_iFromEdge_perc = 10 ;
    m_OutputMode = modeAppend ;
    Resume();
}

void CExtremumMeas::ShutDown()
{
  //TODO: Add all destruction code here
  CFilterGadget::ShutDown();
	delete m_pInput;
	m_pInput = NULL;
	delete m_pOutput;
	m_pOutput = NULL;
  delete m_pDuplexConnector ;
  m_pDuplexConnector = NULL ;
  m_Lock.Lock() ;
  if ( m_pObjects )
  {
    delete m_pObjects ;
    m_pObjects = NULL ;
  }
  if ( m_pNewObjects )
  {
    delete m_pNewObjects ;
    m_pNewObjects = NULL ;
  }
  m_Lock.Unlock() ;
}

CDataFrame* CExtremumMeas::DoProcessing(const CDataFrame* pDataFrame)
{
  m_Lock.Lock() ;

  if ( m_pNewObjects )
  {
    delete m_pObjects ;
    m_pObjects = m_pNewObjects ;
    m_pNewObjects = NULL ;
  }

  m_Lock.Unlock() ;
  if ( !m_pObjects || !m_pObjects->GetCount() )
    return (CDataFrame*)pDataFrame ;

  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame(DEFAULT_LABEL);
  PASSTHROUGH_NULLFRAME(VideoFrame, pDataFrame);

  if (m_LastFormat!=VideoFrame->lpBMIH->biCompression)
  {
    m_LastFormat=VideoFrame->lpBMIH->biCompression;
    m_FormatErrorProcessed=false;
  }
  if ((m_LastFormat!=BI_YUV9) && (m_LastFormat!=BI_Y8) && (m_LastFormat!=BI_Y800) 
    && (m_LastFormat != BI_Y16) && (m_LastFormat != BI_YUV12)) 
  {
    if (!m_FormatErrorProcessed)
      SENDERR_0("ExtremumMeas can only accept formats Y16,YUV9 and Y8");
    m_FormatErrorProcessed=true;
    return NULL;
  }
  
  CContainerFrame * OutputContainer = NULL ;
  CFramesIterator* Iterator = pDataFrame->CreateFramesIterator(text) ;
  if ( Iterator )  // there are texts
  {
    CRect FrameRect( 0, 0, VideoFrame->lpBMIH->biWidth - 1 ,
      VideoFrame->lpBMIH->biHeight - 1 ) ;
    CTextFrame * tf = NULL ;
    while ( tf = (CTextFrame*) Iterator->Next( NULL ) )
    {
      bool bSpotDataExtracted = false ;
      FXString Label = tf->GetLabel() ;
      int iSpotResultSignPos = (int) Label.Find( _T("Data_Spot:") ) ;
      if ( iSpotResultSignPos >= 0 )
      {
        FXString SpotName = Label.Mid( iSpotResultSignPos + 10 ) ;
        SpotName =SpotName.Trim( _T(" \t\n;") ) ;
        CColorSpot Spot ;
        int iIndex ;
        int iWidthFOV = 0 , iHeightFOV = 0 ;

        for ( int i = 0 ; i < m_pObjects->GetCount() ; i++ )
        {
          if ( SpotName == m_pObjects->GetAt(i).m_Name )
          {
            if ( !bSpotDataExtracted )
            {
              FXString Data = tf->GetString() ;
              int iMinusPos = (int) Data.Find( _T('-')) ;
              Data = Data.Mid(iMinusPos + 1) ;
              Data = Data.Trim(_T(" \t;\n\r") ) ;
              int iNItems = sscanf( (LPCTSTR)Data ,
                "%d %lf %lf %lf %lf %d %d %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf \
                %d %d %d %d %d %d" ,
                &iIndex ,
                &Spot.m_SimpleCenter.x ,
                &Spot.m_SimpleCenter.y ,
                &Spot.m_dBlobWidth ,
                &Spot.m_dBlobHeigth ,
                &Spot.m_Area ,
                &Spot.m_iMaxPixel,
                &Spot.m_dCentral5x5Sum ,
                &Spot.m_dSumOverThreshold ,
                &Spot.m_dAngle   ,
                &Spot.m_dLongDiametr,
                &Spot.m_dShortDiametr ,
                &Spot.m_dRDiffraction ,
                &Spot.m_dLDiffraction ,
                &Spot.m_dDDiffraction ,
                &Spot.m_dUDiffraction ,
                &Spot.m_dCentralIntegral ,
                &Spot.m_OuterFrame.left ,
                &Spot.m_OuterFrame.top ,
                &Spot.m_OuterFrame.right ,
                &Spot.m_OuterFrame.bottom ,
                &iWidthFOV , &iHeightFOV
                ) ;  
              bSpotDataExtracted = ( iNItems >= 23 ) ;
            }
            if ( bSpotDataExtracted )
            {
              cmplx Cent( Spot.m_SimpleCenter.x , Spot.m_SimpleCenter.y ) ;
              double dAngle_rad = DegToRad(Spot.m_dAngle) ;
               // norm target angle to +-PI
              double dNormKnownAngle_rad = NormRad(m_pObjects->GetAt(i).m_dDirection_rad) ;
              double dDiffLimit = DegToRad( 25. ) ;
              if ( -M_PI2 <= dNormKnownAngle_rad  
                &&  dNormKnownAngle_rad <= M_PI_2 )
              {
                double dAngleDiff = fabs( dAngle_rad - dNormKnownAngle_rad ) ;
                if ( dAngleDiff > dDiffLimit )
                  continue ; // too big difference
              }
              else
              {
                double dOppositeAngle = dAngle_rad + (dNormKnownAngle_rad > 0) ? -M_PI : +M_PI ;
                // Angle is opposite 
                double dOppositeDiff = fabs( dOppositeAngle - dAngle_rad ) ;
                if ( dOppositeDiff <= dDiffLimit )
                  dAngle_rad = dOppositeAngle ;
                else
                  continue ; // too big difference
              }
              dAngle_rad = -dAngle_rad ; // video coordinate system accounting
              cmplx StepToExtremum = polar( 1.0 , dAngle_rad ) ;
              double dOffsetOnLongDia = 
                (Spot.m_dLongDiametr/2.) * ( (100 - m_iFromEdge_perc) * 0.01) ;
              cmplx BeginExtremumSearch = Cent 
                + StepToExtremum * dOffsetOnLongDia ;
              cmplx StepToLeft = polar( 1.0 , dAngle_rad + M_PI_2 ) ;
              cmplx StepToRight = -StepToLeft ;
              
              // we will overscan once after expected edge
              double dScanLength = Spot.m_dLongDiametr * m_iFromEdge_perc * 0.01 ; 
              int iNSteps = ROUND( dScanLength ) ;
              double dHalfWidth = Spot.m_dShortDiametr / 2.;
              int iRotatedWidth = (ROUND(dHalfWidth * 4) + 2) ;
              double * pdRotatedImage = new double[iNSteps * iRotatedWidth] ;
              int i = 0 ;
              for (  ; i < iNSteps ; i++ )
              {
                cmplx ShiftToExtremum = BeginExtremumSearch 
                  + (StepToExtremum * (double)i) ;
                cmplx LeftPoint = ShiftToExtremum + (StepToLeft * dHalfWidth * 2.) ;
                cmplx RightPoint = ShiftToExtremum + (StepToRight * dHalfWidth * 2.) ;
                
                if ( IsPtInFrame( LeftPoint , VideoFrame )
                  && IsPtInFrame( RightPoint , VideoFrame) )
                {
                int iNPixels = GetPixelsOnLine( LeftPoint , RightPoint , 
                  &pdRotatedImage[iRotatedWidth * i] , iRotatedWidth , VideoFrame ) ;
                ASSERT ( iNPixels < iRotatedWidth - 1 ) ;

              }
                else
                  break ;
              }
              if ( i < iNSteps ) // measurement area is out of frame
                continue ;
              Profile VProfile(iNSteps + 5) ;
              int iVProfileBegin = ROUND( Spot.m_dShortDiametr * 0.6 ) ;
              int iVProfileEnd = ROUND( Spot.m_dShortDiametr * 1.4 ) ;
              double dMaxVDiff = 0. ;
              int iMaxVDiffPos = -1 ;
              for ( int j = 0 ; j < iNSteps ; j++ )
              {
                double dSum = 0. ;
                double * pPix = &pdRotatedImage[ iRotatedWidth * j + iVProfileBegin ] ;
                for ( int k = iVProfileBegin ; k < iVProfileEnd ; k++ )
                  dSum += *(pPix++) ;
                if ( dSum < VProfile.m_dMinValue )
                  VProfile.m_dMinValue = dSum ;
                if ( dSum > VProfile.m_dMaxValue )
                  VProfile.m_dMaxValue = dSum ;
                VProfile.m_pProfData[j] = dSum ;
              }
              if ( m_dwViewMode & VIEW_PROFILES )
              {
                OutputContainer = CheckAndCreateContainer( OutputContainer ,
                  pDataFrame->GetId() , _T("ExtremumMeas") ) ;
                CFigureFrame * pVProf = CreateGraphic( 
                  VProfile.m_pProfData , iNSteps ,
                  BeginExtremumSearch + (StepToRight * dHalfWidth * 2.) ,
                  StepToExtremum , StepToLeft * (dHalfWidth * 4.) ,
                  VProfile.m_dMinValue , VProfile.m_dMaxValue ,  _T("0xff00ff") ) ;
                OutputContainer->AddFrame( pVProf ) ;
              }
              double dThresExtremum = VProfile.m_dMinValue 
                + (VProfile.m_dMaxValue - VProfile.m_dMinValue) 
                * 0.01 * m_iThres_perc ;
              int iExtrEdge = -1 ;
              for ( int j = 0 ; j < iNSteps ; j++ )
              {
                if ( VProfile.m_pProfData[j] > dThresExtremum )
                {
                  iExtrEdge = j ;
                  break ;
                }
              }

              if ( iExtrEdge >= 0  && ( m_dwViewMode & VIEW_CROSSES ) )
              {
                OutputContainer = CheckAndCreateContainer( OutputContainer ,
                  pDataFrame->GetId() , _T("ExtremumMeas") ) ;
                cmplx ExtremPt = BeginExtremumSearch 
                  + (StepToExtremum * (double)iExtrEdge ) ;
                CFigureFrame *pUpperBorder = CreatePtFrame( ExtremPt , 
                  GetHRTickCount() , _T("0x00ffff") , _T("UpperEdge") ) ;
                OutputContainer->AddFrame( pUpperBorder ) ;
              }
              
              double * pdHProfPlus = new double[ (iRotatedWidth/2) + 5 ] ;
              double * pdHProfMinus = new double[ (iRotatedWidth/2) + 5 ] ;
              double dMinPlus = 1e30 ;
              double dMaxPlus = -1e30 ;
              double dMinMinus = 1e30 ;
              double dMaxMinus = -1e30 ;
              int iNStepsToSide = (iRotatedWidth/2) - 1 ;
              for ( int j = 0 ; j < (iRotatedWidth/2) - 1 ; j++ )
              {
                int iPlusPos = iRotatedWidth/2 + j ;
                int iMinusPos = iRotatedWidth/2 - j ;
                double * pPixPlus = &pdRotatedImage[ iPlusPos ] ;
                double * pPixMinus = &pdRotatedImage[ iMinusPos ] ;
                double dSumPlus = 0. , dSumMinus = 0. ;
                for ( int k = 0 ; k < iExtrEdge ; k++ )
                {
                  dSumPlus += *(pPixPlus) ;
                  pPixPlus += iRotatedWidth ;
                  dSumMinus += *(pPixMinus) ;
                  pPixMinus += iRotatedWidth ;

                }
                if ( dSumMinus < dMinMinus )
                  dMinMinus = dSumMinus ;
                if ( dSumMinus > dMaxMinus )
                  dMaxMinus = dSumMinus ;
                if ( dSumPlus < dMinPlus )
                  dMinPlus = dSumPlus ;
                if ( dSumPlus > dMaxPlus )
                  dMaxPlus = dSumPlus ;

                pdHProfPlus[j] = dSumPlus ;
                pdHProfMinus[j] = dSumMinus ;
              }
              if ( m_dwViewMode & VIEW_PROFILES )
              {
                OutputContainer = CheckAndCreateContainer( OutputContainer ,
                  pDataFrame->GetId() , _T("ExtremumMeas") ) ;
                CFigureFrame * pHProfPlusGraph = CreateGraphic( pdHProfPlus, 
                  (iRotatedWidth/2) - 1 ,
                  BeginExtremumSearch - (50. * StepToExtremum) , StepToLeft ,
                  StepToExtremum * 50. ,
                  dMinPlus , dMaxPlus ,  _T("0xff0000") ) ;
                OutputContainer->AddFrame( pHProfPlusGraph ) ;
                CFigureFrame * pHProfMinusGraph = CreateGraphic( pdHProfMinus, 
                  (iRotatedWidth/2) - 1 ,
                  BeginExtremumSearch - (50. * StepToExtremum) , StepToRight ,
                  StepToExtremum * 50. ,
                  dMinMinus , dMaxMinus ,  _T("0xff0000") ) ;
                OutputContainer->AddFrame( pHProfMinusGraph ) ;
              }
              int iPlusEdge = 0 ;
              int iMinusEdge = 0 ;
              double dThresPlus = dMinPlus 
                + (dMaxPlus - dMinPlus) * 0.01 * m_iThres_perc ;
              double dThresMinus = dMinMinus 
                + (dMaxMinus - dMinMinus) * 0.01 * m_iThres_perc ;
              for ( int j = 0 ; j < iNStepsToSide ; j++ )
              {
                if ( pdHProfPlus[j] > dThresPlus )
                {
                  iPlusEdge = j ;
                  break ;
                }
              }
              for ( int j = 0 ; j < iNStepsToSide ; j++ )
              {
                if ( pdHProfMinus[j] > dThresMinus )
                {
                  iMinusEdge = j ;
                  break ;
                }
              }
              if ( m_dwViewMode & VIEW_CROSSES )
              {              
                if ( iPlusEdge )
                {

                  cmplx ExtremPt = BeginExtremumSearch 
                    + (StepToLeft * (double)iPlusEdge ) ;
                  CFigureFrame *pSidePlusBorder = CreatePtFrame( ExtremPt , 
                    GetHRTickCount() , _T("0x00ffff") , _T("SidePlusEdge") ) ;
                  OutputContainer = CheckAndCreateContainer( OutputContainer ,
                    pDataFrame->GetId() , _T("ExtremumMeas") ) ;
                  OutputContainer->AddFrame( pSidePlusBorder ) ;
                }
                if ( iMinusEdge )
                {

                  cmplx ExtremPt = BeginExtremumSearch 
                    + (StepToRight * (double)iMinusEdge ) ;
                  CFigureFrame *pSideMinusBorder = CreatePtFrame( ExtremPt , 
                    GetHRTickCount() , _T("0xffff00") , _T("SideMinusEdge") ) ;
                  OutputContainer = CheckAndCreateContainer( OutputContainer ,
                    pDataFrame->GetId() , _T("ExtremumMeas") ) ;
                  OutputContainer->AddFrame( pSideMinusBorder ) ;
                }
              }
              delete[] pdHProfPlus ;
              delete[] pdHProfMinus ;
              delete[] pdRotatedImage ;
            }
          }
        }

      }
    }
    delete Iterator ;
  }
  return OutputContainer;
}

bool CExtremumMeas::ScanProperties(LPCTSTR text, bool& Invalidate)
{
  FXPropertyKit pk(text);
  pk.GetString( _T("Obj_Angles") , m_ObjectsAndDirections ) ;
  pk.GetInt( "Average_pix" , m_iAverage_pix ) ;
  pk.GetInt( "FromEdge_perc" , m_iFromEdge_perc ) ;
  pk.GetInt( "Thres_percent" , m_iThres_perc ) ;
  pk.GetInt( "ViewMode" , m_dwViewMode ) ;
    
  FXString ForParsing( m_ObjectsAndDirections ) ;
  
  m_Lock.Lock() ;
  if ( m_pNewObjects != NULL )
  {
    delete m_pNewObjects ;
    m_pNewObjects = NULL ;
  }
  m_Lock.Unlock() ;
  
  ObjArray * pNewObjects = new ObjArray ;
  while ( ForParsing.GetLength() > 2 )
  {
    int iSemicolonPos = (int) ForParsing.Find( _T(';') ) ;
    FXString NextObj = ( iSemicolonPos < 0 ) ?
      ForParsing : ForParsing.Left( iSemicolonPos ) ;
    NextObj = NextObj.Trim( _T(" \t\n\r") ) ;
    int iNameEndPos = (int) NextObj.FindOneOf( _T(" \t\r\n") ) ;
    if ( iNameEndPos <= 0 )
    {
      SENDERR_1( "Can't parse ""%s""" , m_ObjectsAndDirections ) ;
      break ;
    }
    Extremum NewExtremum ;
    NewExtremum.m_Name = NextObj.Left( iNameEndPos ) ;
    NewExtremum.m_dDirection_rad = (iNameEndPos < NextObj.GetLength() - 1 ) ?
      _tstof( (LPCTSTR)NextObj + iNameEndPos + 1 ) : 0.0 ;
    NewExtremum.m_dDirection_rad = DegToRad(NewExtremum.m_dDirection_rad) ;
    pNewObjects->Add( NewExtremum ) ;
    ForParsing.Delete( 0 , NextObj.GetLength() ) ;
  }
  if ( pNewObjects->GetCount() )
    m_pNewObjects = pNewObjects ;
  else
  {
    delete pNewObjects ;
  }
  // Here should be commented samples for all types of variables and 
// dialog controls reading

//   pk.GetDouble("Sigma_Pix" , m_dGaussSigma_pix ) ;
//   pk.GetInt( "MinRadius_pix" , m_iMinRadius ) ;
//   pk.GetInt( "MaxRadius_pix" , m_iMaxRadius ) ;
//   pk.GetInt( "SZone" , m_iSecondaryMeasZone ) ;
//   double m_dInitialAngle_deg ;
//   pk.GetDouble("Angle_Deg" , m_dInitialAngle_deg ) ;
//   pk.GetInt( "AngleStep" , m_iMeasStep_deg ) ;
//   pk.GetDouble("2ndThres" , m_dDiscoverThres ) ;
//   pk.GetDouble("Scale_nm" , m_dScale_nm ) ;
//   pk.GetDouble("MinSize_um" , m_dMinSize_um ) ;
//   pk.GetDouble("MaxSize_um" , m_dMaxSize_um ) ;
//   pk.GetDouble("SigSigma2" , m_dSignThres ) ;
//   pk.GetInt( "ViewMode" , m_iViewMode ) ;
// 
//   m_dInitialAngle_deg = m_iMeasStep_deg * ROUND( m_dInitialAngle_deg / m_iMeasStep_deg ) ;
//   m_dInitialAngle_rad = DegToRad( m_dInitialAngle_deg ) ;
//   m_dInitialAngle_rad = fmod( m_dInitialAngle_rad , M_2PI ) ;
//   m_iPatternLen = CreateGaussPattern( 
//   m_dGaussSigma_pix , m_iGaussHalfWidth_pix , m_Pattern ) ;
   return true;
}

bool CExtremumMeas::PrintProperties(FXString& text)
{


     FXPropertyKit pk;
     CFilterGadget::PrintProperties(text);
     FXString AsString , DirVal ;
     ObjArray * pObjArray = (m_pNewObjects) ? m_pNewObjects : m_pObjects ;
     for ( int i = 0 ; pObjArray && i < pObjArray->GetCount() ; i++  )
     {
       DirVal.Format( _T(" %6.1f;") , 
         RadToDeg(pObjArray->GetAt(i).m_dDirection_rad) ) ;
       AsString += pObjArray->GetAt(i).m_Name + DirVal ;
     }
     pk.WriteString( _T("Obj_Angles") , AsString ) ;
     pk.WriteInt( "Average_pix" , m_iAverage_pix ) ;
     pk.WriteInt( "FromEdge_perc" , m_iFromEdge_perc ) ;
     pk.WriteInt( "Thres_percent" , m_iThres_perc ) ;
     pk.WriteInt( "ViewMode" , m_dwViewMode ) ;
    // Here should be commented samples for all types of variables and 
    // dialog controls writing
//     pk.WriteDouble("Sigma_Pix" , m_dGaussSigma_pix ) ;
//     pk.WriteInt( "HalfWidth" , m_iGaussHalfWidth_pix ) ;
//     pk.WriteInt( "MinRadius_pix" , m_iMinRadius ) ;
//     pk.WriteInt( "MaxRadius_pix" , m_iMaxRadius ) ;
//     pk.WriteInt( "SZone" , m_iSecondaryMeasZone ) ;
//     pk.WriteDouble("Angle_Deg" , RadToDeg( m_dInitialAngle_rad ) ) ;
//     pk.WriteInt( "AngleStep" , m_iMeasStep_deg ) ;
//     pk.WriteDouble("2ndThres" , m_dDiscoverThres ) ;
//     pk.WriteDouble("Scale_nm" , m_dScale_nm ) ;
//     pk.WriteDouble("MinSize_um" , m_dMinSize_um ) ;
//     pk.WriteDouble("MaxSize_um" , m_dMaxSize_um ) ;
//     pk.WriteDouble("SigSigma2" , m_dSignThres ) ;
//     pk.WriteInt( "ViewMode" , m_iViewMode ) ;
     text+=pk;
     return true;

}

bool CExtremumMeas::ScanSettings(FXString& text)
{
    // Here should be commented samples for all types of  
    // dialog controls with short explanation


    text = _T("template(EditBox(Obj_Angles)")
       ",Spin(Average_pix,1,40)"
       ",Spin(FromEdge_perc,5,50)"
       ",Spin(Thres_percent,1,99)"
       ",Spin(ViewMode,0,7)"
//       ",Spin(MaxRadius_pix,15,150)"
//       ",Spin(SZone,5,50)"
//       ",EditBox(Angle_Deg)"
//       ",Spin(AngleStep,1,10)"
//       ",EditBox(2ndThres)"
//       ",EditBox(Scale_nm)"
//       ",EditBox(MinSize_um)"
//       ",EditBox(MaxSize_um)" 
//       ",EditBox(SigSigma2)"
//       ",Spin(ViewMode,0,16)"
      _T(")");
    return true;

}

int CExtremumMeas::GetInputsCount()
{  
  return 1;
}

CInputConnector* CExtremumMeas::GetInputConnector(int n)
{
  return ( n == 0 ) ? m_pInput : NULL ;
}

int CExtremumMeas::GetOutputsCount()
{
  return 1;
}

COutputConnector* CExtremumMeas::GetOutputConnector(int n)
{
  return ( n == 0 ) ? m_pOutput : NULL ;
}

int CExtremumMeas::GetDuplexCount()
{
  return 1 ;
}

CDuplexConnector* CExtremumMeas::GetDuplexConnector(int n)
{
  return ( n == 0 ) ? m_pDuplexConnector : NULL  ;
}

void CExtremumMeas::AsyncTransaction(
  CDuplexConnector* pConnector, CDataFrame* pParamFrame)
{
  pParamFrame->Release( pParamFrame );
}

