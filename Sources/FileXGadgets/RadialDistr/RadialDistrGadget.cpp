// RadialDistrGadget.h : Implementation of the RadialDistr class


#include "StdAfx.h"
#include "RadialDistrGadget.h"
#include <imageproc\clusters\segmentation.h>
#include <imageproc\utilities.h>
#include <gadgets\vftempl.h>


IMPLEMENT_RUNTIME_GADGET_EX(RadialDistr, CFilterGadget, "Radial", TVDB400_PLUGIN_NAME);

RadialDistr::RadialDistr(void)
{
    m_pInput = new CInputConnector(transparent);
    m_pOutput = new COutputConnector(transparent);
    m_LastFormat = 0xffffffff ;
    m_FormatErrorProcessed = false ;
    m_OutputMode = modeAppend ;
    Resume();
}

void RadialDistr::ShutDown()
{
    //TODO: Add all destruction code here
    CFilterGadget::ShutDown();
	delete m_pInput;
	m_pInput = NULL;
	delete m_pOutput;
	m_pOutput = NULL;
}

CDataFrame* RadialDistr::DoProcessing(const CDataFrame* pDataFrame)
{
  const CVideoFrame* pVideoFrame = pDataFrame->GetVideoFrame(DEFAULT_LABEL);
  PASSTHROUGH_NULLFRAME(pVideoFrame, pDataFrame);
  if (m_LastFormat != pVideoFrame->lpBMIH->biCompression)
  {
    m_LastFormat = pVideoFrame->lpBMIH->biCompression;
    m_FormatErrorProcessed=false;
  }
  if ((m_LastFormat!=BI_YUV9) && (m_LastFormat!=BI_YUV12) 
    && (m_LastFormat!=BI_Y8) && ( m_LastFormat != BI_Y800 ) && (m_LastFormat != BI_Y16) )
  {
    if (!m_FormatErrorProcessed)
      SENDERR_0("TVObjects can only accept formats Y16,YUV9 and Y8");
    m_FormatErrorProcessed=true;
    return NULL;
  }
  CContainerFrame * pContainer = NULL ;
  CTextFrame * pTextFrame = NULL ;
  CFramesIterator* Iterator = pDataFrame->CreateFramesIterator( text ) ;
  if ( Iterator )
  {
    int iNBlobs = 0 ;
    int iMaxIntens = 0 ;
    int iMinIntens = 0 ;
    SpotArray SpotResults ;
    while ( pTextFrame = (CTextFrame *) Iterator->Next(NULL) )
    {
      FXString Label = pTextFrame->GetLabel() ;
      if ( Label.Find("Data_Spot:") < 0 )
        continue ;
      FXString Data =pTextFrame->GetString() ;
      int iPos = (int)Data.Find( '\n' ) ;
      if ( iPos > 0   )
      {
        FXString Statistics = Data.Left( iPos ) ;
        int iNSpotPos = (int) Statistics.Find( "Spots=" ) ;
        int iMaxPos = (int) Statistics.Find( "Max=" ) ;
        int iMinPos = (int) Statistics.Find( "Min=" ) ;
        CHAR * p = Statistics.GetBuffer() ;
        iNBlobs = ( iNSpotPos >= 0 ) ? 
          atoi( p + iNSpotPos + 6 ) : 0 ;
        iMaxIntens = (iMaxPos >= 0) ?
          ROUND(atof( p + iMaxPos + 4 )) : 0 ;
        iMinIntens = ( iMinPos >= 0 ) ?
          ROUND( atof( p + iMinPos + 4 ) ) : 0  ;

        if ( (iNSpotPos >= 0)  || (iMaxPos >= 0)  || (iMinPos >= 0) )
          Data = Data.Mid( iPos + 1 ) ; // remove statistics string
      }
      do 
      {
        if ( iNBlobs == 0 )
          break ;
        CColorSpot Spot ;
        int iIndex ;
        int iWidthFOV = 0 , iHeightFOV = 0 ;
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
        if ( iNItems >= 8 )
        {
//           Spot.m_SimpleCenter = Spot.m_SimpleCenter + CDPoint( 160. , 8 ) ;
//           if ( (iNItems > 20)  &&  (Spot.m_dCentralIntegral != 0.) 
//             && (Spot.m_dCentralIntegral > Spot.m_dLDiffraction) 
//             && (Spot.m_dCentralIntegral > Spot.m_dRDiffraction)
//             && (Spot.m_dCentralIntegral > Spot.m_dUDiffraction)
//             && (Spot.m_dCentralIntegral > Spot.m_dDDiffraction) )
//             Spot.m_iDetailed |= MEASURE_DIFFRACT ;
          SpotResults.Add( Spot ) ;
        }
        int iPos = (int) Data.Find( '\n' ) ;
        if ( iPos > 0  &&  ((int) Data.GetLength() > (iPos + 10)) )
          Data = Data.Mid( iPos + 1 ) ;
        else
          break ;
      } while( Data.GetLength() > 10 ) ;
    }
    for ( int i = 0 ; i < SpotResults.GetCount() ; i++ )
    {
      CColorSpot& Spot = SpotResults.GetAt( i ) ;
      cmplx Cent ( Spot.m_SimpleCenter.x , Spot.m_SimpleCenter.y ) ;
      double Signal[1000] ;
      CFigureFrame * pBorder = NULL ;
      for ( double dAngle = 0. ; dAngle < 360. ; dAngle += 2.0 ) // every 2 degrees
      {
        cmplx OneVect = polar( 1. , DegToRad( dAngle) ) ;
        int iLen = GetRadiusPixels( Cent , DegToRad(dAngle) , m_iMinRadius , m_iMaxRadius ,
          Signal , pVideoFrame ) ;
        double dSum = 0. ;
        for ( int iValCnt = 0 ; iValCnt < iLen ; iValCnt++ )
          dSum += Signal[iValCnt] ;
        cmplx Pt = Cent + OneVect * ( m_iMinRadius + dSum * m_dKampl / 1000. ) ;
        if ( !pBorder )
          pBorder = CreatePtFrame( Pt , GetHRTickCount() , "0xff80ff" ) ;
        else
          pBorder->AddPoint( CDPoint(Pt.real() , Pt.imag()) ) ;
      }
      if ( pBorder->GetCount() > 2 )
        pBorder->AddPoint( pBorder->ElementAt(0) ) ;
      if ( !pContainer )
        pContainer = CContainerFrame::Create() ;
      pContainer->AddFrame( pBorder ) ;
    }
    delete Iterator ;
  }

  return pContainer ;
}

bool RadialDistr::ScanProperties(LPCTSTR text, bool& Invalidate)
{
  FXPropertyKit pk(text);
  pk.GetInt( "MinRadius_pix" , m_iMinRadius ) ;
  pk.GetInt( "MaxRadius_pix" , m_iMaxRadius ) ;
  pk.GetDouble( "Kampl" , m_dKampl ) ;
	return true;
}

bool RadialDistr::PrintProperties(FXString& text)
{
  FXPropertyKit pk;
  CFilterGadget::PrintProperties(text);
  pk.WriteInt( "MinRadius_pix" , m_iMinRadius ) ;
  pk.WriteInt( "MaxRadius_pix" , m_iMaxRadius ) ;
  pk.WriteDouble( "Kampl" , m_dKampl ) ;
  text+=pk;
  return true;
}

bool RadialDistr::ScanSettings(FXString& text)
{
  text = "template("
    "Spin(MinRadius_pix,10,50)"
    ",Spin(MaxRadius_pix,15,150)"
    ",EditBox(Kampl)"
    ")";
    return true;
}

