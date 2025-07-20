// UserExampleGadget.h.h : Implementation of the UserExampleGadget class


#include "StdAfx.h"
#include <opencv2/imgproc/types_c.h>
#include "math\intf_sup.h"
#include "TrackerGadget.h"

#define THIS_MODULENAME ("Tracker")

//
//using namespace cv;
IMPLEMENT_RUNTIME_GADGET_EX(TrackerGadget, CFilterGadget, "Matchers", TVDB400_PLUGIN_NAME);
//USER_FILTER_RUNTIME_GADGET(ObjectMatcherGadget,"ObjectMatcher");	//	Mandatory
const char* TrackerGadget::pList = "CV_TM_SQDIFF;CV_TM_SQDIFF_NORMED;CV_TM_CCORR;CV_TM_CCORR_NORMED;CV_TM_CCOEFF;CV_TM_CCOEFF_NORMED;";

TrackerGadget::TrackerGadget()
{
	m_pContainer = NULL;

	FXPropertyKit pk;
	FXString text;
	bool Invalidate = false;
	m_bselect_ROI = false;
	m_dRadius = 12;
	m_dTemplateToROICoeff = 1.5;

	m_dFirstMatchCoeff = -1;

	m_prevCenter.x = m_prevCenter.y = -1;

	m_cdTrackPt.x = m_cdTrackPt.y = -1;
	m_dMaxAllowedPattern = 0.99;
	m_dMaxAllowedView = 0.5;
  m_sPO = "PO-Unknown";
	m_sMatchMethod = "CV_TM_CCORR";

	init();
  FXRegistry Reg( "TheFileX\\FindObjects" );
//   Reg.GetRegiCmplx( "Parameters" , "SaveFragmentSize" , m_SaveFragmentSize ,
//     cmplx( 300. , 300. ) );
  m_sDataDir = Reg.GetRegiString( _T("DataLocation") , _T( "MainDir" ) , _T( "e:/FindObjects" ) ) ;
  if ( FxVerifyCreateDirectory( m_sDataDir ) )
    SENDERR( "Can't create directory %s" , ( LPCTSTR ) m_sDataDir ) ;
  m_sPatternsDir = Reg.GetRegiString( _T( "DataLocation" ) , _T( "PatternsDir" ) , _T( "/Patterns" ) );
  m_sPatternsDir = m_sDataDir + m_sPatternsDir + _T('/') ;
  if ( FxVerifyCreateDirectory( m_sPatternsDir ) )
    SENDERR( "Can't create directory %s" , ( LPCTSTR ) m_sPatternsDir ) ;
  m_sPartID = Reg.GetRegiString( _T( "LastMeasurement" ), _T( "LastPart" ) , _T( "" ) );
  m_sPO = Reg.GetRegiString( _T( "LastMeasurement" ) , _T( "LastOrder" ) , m_sPO );
  m_iMeasurementExposure_us = Reg.GetRegiInt( _T( "LastMeasurement" ) , _T( "MeasExp_us" ) ,
    m_iMeasurementExposure_us );

}


void TrackerGadget::ConfigParamChange( 
  LPCTSTR pName , void* pObject , bool& bInvalidate , bool& bInitRescan )
{
  TrackerGadget* pGadget = ( TrackerGadget* ) pObject;
  if ( pGadget )
  {
    if ( !_tcsicmp( pName , _T( "ActivePatterns" ) ) )
    {
      pGadget->m_DataUpdateLock.Lock( INFINITE , _T("ConfigParamChange") ) ;
      pGadget->m_NewActiveIndexes.clear() ;
      FXSIZE iPos = 0 ;
      FXString Token = pGadget->m_sActivePatternsAsText.Tokenize( " ,;\t" , iPos ) ;
      while( !Token.IsEmpty() )
      {
        int iNextIndex = atoi( ( LPCTSTR ) Token ) ;
        if ( iNextIndex > 0 )
          pGadget->m_NewActiveIndexes.push_back( iNextIndex ) ;

        Token = pGadget->m_sActivePatternsAsText.Tokenize( " ,;\t" , iPos ) ;
      }
      pGadget->m_DataUpdateLock.Unlock() ;
    }
  }
}

cv::Mat TrackerGadget::ConvertVideoFrameToCvMat(const CVideoFrame* pVideoFrame)
{
	//pVideoFrame->lpBMIH.biWidth;
	//biHeight;


	cv::Mat Img(pVideoFrame->lpBMIH->biHeight,pVideoFrame->lpBMIH->biWidth,  CV_16S);//(480, 640, CV_16S);
	size_t memsize = pVideoFrame->lpBMIH->biWidth * pVideoFrame->lpBMIH->biHeight/*3 **/;// 480 * 640;
	Img.flags = 1124024320;////124024336;
	Img.dims = 2;
	Img.step = pVideoFrame->lpBMIH->biWidth;// 640/*1920*/;
	Img.step.buf[1] = 1;
	Img.data = (uchar*)malloc(memsize);
	//memset(Img.data, 0, memsize);
	memcpy(Img.data, pVideoFrame->lpBMIH + 1, memsize);

	//cv::imwrite("e:\\RAWImage.jpg", Img);
	return Img;
}

CVideoFrame* TrackerGadget::ConvertCvMatToVideoFrame( const cv::Mat cvImage )
{
  if ( ( cvImage.dims != 2 ) || ( cvImage.rows == 0 ) || ( cvImage.cols == 0 ) )
    return nullptr ;

  int iWidth = ( cvImage.cols & 3 ) ? ( cvImage.cols & ~3 ) + 4 : cvImage.cols ;
  int iHeight = ( cvImage.rows & 3 ) ? ( cvImage.rows & ~3 ) + 4 : cvImage.rows ;
  pTVFrame pTVF = makeNewY8Frame( iWidth , iHeight ,
    ( ( cvImage.cols & 3 ) || ( cvImage.rows & 3 ) ) ? 0 : cvImage.data ) ;
  if ( pTVF )
  {
    int iY = 0 , iRest ;
    if ( ( cvImage.cols & 3 ) || ( cvImage.rows & 3 ) ) // not *4, needs to copy data by strings
    {
      LPBYTE pSrc = cvImage.data , pDest = GetData( pTVF ) ;
      for ( ; iY < cvImage.rows ; iY++ )
      {
        memcpy( pDest , pSrc , cvImage.cols ) ;
        pSrc += cvImage.step1() ;
        pDest += cvImage.cols ;
        iRest = cvImage.cols & 3 ;
        while ( iRest++ & 3 )
        {
          *( pDest++ ) = *( pSrc - 1 ) ;
        }
      }
      iRest = cvImage.rows & 3 ;
      LPBYTE pLastDest = pDest - iWidth ;
      while ( iRest++ & 3 )
      {
        memcpy( pDest , pLastDest , iWidth ) ;
        pDest += iWidth ;
      }
    }
    CVideoFrame* pVF = CVideoFrame::Create( pTVF );
    if ( pVF )
      return pVF ;
    else
      freeTVFrame( pTVF ) ;
  }
  return nullptr ;
}

CDataFrame* TrackerGadget::DoProcessing(const CDataFrame* pDataFrame)
{

	using namespace std;
	const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame(DEFAULT_LABEL);

	if (!VideoFrame || !VideoFrame->lpBMIH)
		return NULL;

	cv::Mat cvImg = ConvertVideoFrameToCvMat(VideoFrame);
	if (m_bselect_ROI)
	{
		m_bselect_ROI = false;
    UpdateTemplateRect( cvImg , m_FutureROI_Rect ) ;
	}

	double dMatchFactor = 0;	
	CDPoint cdCenterPoint;
  CDataFrame* pDataFrameOut = NULL ;
  if ( m_template.dims && m_template.rows && m_template.cols )
	{
    cv::Mat result;
    int match_method = cv::TM_CCOEFF_NORMED;

    cv::matchTemplate( cvImg , m_template , result , match_method );

	CContainerFrame* resVal;
	resVal = CContainerFrame::Create();

	resVal->ChangeId(pDataFrame->GetId());
	resVal->SetTime(pDataFrame->GetTime());
	resVal->SetLabel("Results");

    std::vector<CPoint> cpFiltered ;
    DoubleVector MaxCorrValues ;
    CSize RectSize( m_template.cols , m_template.rows ) ;
    CSize OffFromCent( RectSize.cx / 2 , RectSize.cy / 2 ) ;
    for ( int y = 0; y < result.rows; ++y )
	{
      for ( int x = 0; x < result.cols; ++x )
	{
        CPoint Pt( x , y ) ;
        double dCorrValue = result.at<float>( y , x ) ;
        if ( dCorrValue >= m_dThreshold )
		{
          size_t iExisted = 0 ;
          for (  ; iExisted < cpFiltered.size() ; ++iExisted )
          {
            CPoint cpDist = Pt - cpFiltered[ iExisted ] ;
            if ( (abs( cpDist.x ) < RectSize.cx) && ( abs( cpDist.y ) < RectSize.cy )  )
            {
              if ( MaxCorrValues[ iExisted ] < dCorrValue )
              {       // New point has bigger correlation
                MaxCorrValues[ iExisted ] = dCorrValue ;
                cpFiltered[ iExisted ] = Pt ;
              } // else new point has lower correlation
              break ;
		}
          }
          if ( iExisted >= cpFiltered.size() )
		{
            MaxCorrValues.push_back( dCorrValue ) ;
            cpFiltered.push_back( Pt ) ;
		}
	}
      }
    }
    for ( size_t Index = 0; Index < cpFiltered.size(); ++Index )
    {
      CRect ViewRect( cpFiltered[ Index ] , RectSize );
      CFigureFrame* ff = CreateFigureFrameEx( ViewRect , 0x0000ff , 3 ) ;
      resVal->AddFrame( ff ) ;
      cmplx cPt( cpFiltered[ Index ].x + OffFromCent.cx , 
        cpFiltered[ Index ].y + OffFromCent.cy ) ;
      CTextFrame* pViewText = CreateTextFrameEx( cPt , 0x0000ff , 
        10 , "%.3f" , MaxCorrValues[ Index ] );
      resVal->AddFrame( pViewText ) ;
    }
		 
  // 	m_cdTrackPt = cdCenterPoint;
	//FXString CoordView;
  // 	CoordView.Format("Match Factor : %6.5f", dMatchFactor);
  // 	CTextFrame * ViewText = CTextFrame::Create(CoordView);
  // 	ViewText->ChangeId(pDataFrame->GetId());
  // 	ViewText->SetTime(pDataFrame->GetTime());
  // 	resVal->AddFrame(ViewText);
  // 
  // 	//FXString CoordView;
  // 	CoordView.Format("New Coordinate : x=%6.2f,y=%6.2f", cdCenterPoint.x, cdCenterPoint.y);
  // 	ViewText = CTextFrame::Create(CoordView);
  // 	ViewText->ChangeId(pDataFrame->GetId());
  // 	ViewText->SetTime(pDataFrame->GetTime());
  // 	resVal->AddFrame(ViewText);

	resVal->AddFrame(VideoFrame);
    pDataFrameOut = resVal ;
    cv::normalize( result , result , 0 , 255 , cv::NORM_MINMAX , CV_8U );
    CVideoFrame * pVF = ConvertCvMatToVideoFrame( result ) ;

    PutFrame( GetOutputConnector( CORRELATION_OUT_PIN ) , pVF , 100 ) ;
  }
  else // no template
  {
    ((CDataFrame*)pDataFrame)->AddRef() ;
    pDataFrameOut = ( ( CDataFrame* ) pDataFrame ) ;
  }

  free( cvImg.data ) ;
  cvImg.release();

  return pDataFrameOut ;
}
cv::Rect TrackerGadget::GetRect(CDPoint center, double rad)
{
	cv::Rect tmpRect;
	tmpRect.x = (int)(center.x - rad);
	tmpRect.y = (int)(center.y - rad);
	tmpRect.height = (int)(2 * rad);
	tmpRect.width = (int)(2 * rad);

	if (tmpRect.x < 0) tmpRect.x = 0;
	if (tmpRect.y < 0) tmpRect.y = 0;

	return tmpRect;
}
void TrackerGadget::AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame)
{
  if (!pParamFrame)
    return;

  CTextFrame * ParamText = pParamFrame->GetTextFrame(DEFAULT_LABEL);
  if (ParamText)
  {

    TRACE( "%s" , _T( ParamText->GetString() ) );

    if ( ParamText->GetString().Find( "Rect" ) >= 0 )
    {
      FXPropKit2 Param = ParamText->GetString() ;
      FXString RectPts ;
      Param.GetString( "Rect" , RectPts ) ;
      CRect ROI ;
      int iNPts = GetArray( RectPts , _T( 'i' ) , 4 , &ROI ) ;
      if ( iNPts == 4 )
      {
        m_FutureROI_Rect = ROI ;
        m_bselect_ROI = true;
        m_dFirstMatchCoeff = -1;
        m_centersArr.RemoveAll();
        m_prevCenter.x = m_prevCenter.y = -1;
        m_cdTrackPt = CDPoint( ROI.CenterPoint() );
      }
    }
  }
  pParamFrame->Release( pParamFrame );
};

bool TrackerGadget::UpdateTemplateRect(cv::Mat Img, CRect& rect )
{
	try
	{
    cv::Mat newTemplate = CropROI( Img , rect );
		newTemplate.copyTo(m_template);
    return true ;
	}
  
  catch ( const char * str )
	{
    SENDERR( "TrackerGadget::UpdateTemplateRect - Exception in :%s" , str ) ;
		int y = *str ;
	}
  return false ;
}

bool TrackerGadget::CreateAndSaveTemplate( cv::Mat Img , CRect& rect )
{	
  if ( UpdateTemplateRect( Img , rect ) )
  {
    int iFoundIndex = 1 ;
    for ( ; iFoundIndex <= m_iMaxIndex ; iFoundIndex++ )
    {
      size_t i = 0 ;
      for ( ; i < m_BusyIndexes.size() ; i++ )
      {
        if ( m_BusyIndexes[ i ] == iFoundIndex )
          break ;
      }
      if ( i >= m_BusyIndexes.size() )
        break ;
    }
    Template Templ( m_template , m_sPatternName , iFoundIndex ) ;
    if ( Templ.Save( m_sPatternsDir ) )
    {
      if ( m_iMaxIndex < iFoundIndex )
        m_iMaxIndex ;

    }
  }
  return false ;
}

void TrackerGadget::UpdateTemplateROI( cv::Mat Img , CDPoint center )
{
  try
  {
    cv::Mat newTemplate = CropROI( Img , m_cdTrackPt , m_dRadius , 1 );
    newTemplate.copyTo( m_template );
  }

  catch ( const char* str )
  {
    TRACE( "\n exception in TrackerGadget::UpdateTemplateROI:%s" , str ) ;
    int y = *str ;
  }
  //cv::imwrite("e:\\NewTemplate.jpg", m_template);	
}

cv::Mat TrackerGadget::CropROI(const cv::Mat &img ,CDPoint center, double rad ,double sizeCoeff)
{
	cv::Rect newRect = GetRect(center, rad * sizeCoeff);

  if ( newRect.x < 4 )
    newRect.x = 4;
  if ( newRect.y < 4 )
    newRect.y = 4;
  if ( newRect.y + newRect.height > img.rows - 4 )
    newRect.height = img.rows - newRect.y - 4;
  if ( newRect.x + newRect.width > img.cols - 4 )
    newRect.width = img.cols - newRect.x - 4;
    cv::Mat newImg;
	if (newRect.height > 4 && newRect.width > 4)
	{
		newImg = img(newRect);
		m_ROI_Origin.x = center.x;
		m_ROI_Origin.y = center.y;
	}

	return newImg;
}

cv::Mat TrackerGadget::CropROI( const cv::Mat& img , CRect rect )
{
  cv::Rect newRect( rect.left , rect.top , rect.Width() , rect.Height() );

  if ( newRect.x < 4 )
    newRect.x = 4;
  if ( newRect.y < 4 )
    newRect.y = 4;
  if ( newRect.y + newRect.height > img.rows - 4 )
    newRect.height = img.rows - newRect.y - 4;
  if ( newRect.x + newRect.width > img.cols - 4 )
    newRect.width = img.cols - newRect.x - 4;
  cv::Mat newImg;
  if ( newRect.height > 4 && newRect.width > 4 )
  {
    newImg = img( newRect );
    m_ROI_Origin = CDPoint( rect.CenterPoint() ) ;
    m_TemplateRect = newRect ;
  }

  return newImg;
}


void TrackerGadget::PropertiesRegistration()
{	
  addProperty( SProperty::EDITBOX , _T( "PartID" ) , &m_sPartID ,
    SProperty::String );
  addProperty( SProperty::EDITBOX , _T( "MaxAllowedForView" ) , &m_dMaxAllowedView ,
    SProperty::Double, 0.001, 1000000000);
	addProperty(SProperty::EDITBOX, _T("MaxAllowedForPattern"), &m_dMaxAllowedPattern, 
    SProperty::Double, 0.001, 100000000);
  addProperty( SProperty::EDITBOX , _T( "Template_pix" ) , &m_dRadius ,
    SProperty::Double , 5 , 400 );
  addProperty( SProperty::EDITBOX , _T( "Threshold" ) , &m_dThreshold , 
    SProperty::Double , 0. , 1. );
  addProperty( SProperty::EDITBOX , _T( "ROI_xToTempl" ) , &m_dTemplateToROICoeff , 
    SProperty::Double , 1 , 100 );
  //addProperty(SProperty::COMBO, _T("MatchMethod"), &m_sMatchMethod, SProperty::String, pList);
  addProperty(SProperty::EDITBOX, _T("ActivePatterns"), &m_sActivePatternsAsText ,
    SProperty::String );
  SetChangeNotification( _T( "ActivePatterns" ) , ConfigParamChange , this );

  addProperty( SProperty::EDITBOX , _T( "SizeRange_perc" ) , &m_dSizeRangePercent ,
    SProperty::Double , 0. , 25. );
  addProperty( SProperty::EDITBOX , _T( "AngleRange_deg" ) , &m_dAngleRange_deg ,
    SProperty::Double , 0. , 10. );
  addProperty( SProperty::SPIN_BOOL , _T( "CurrentPattern#" ) , &m_iCurrentPattern ,
    SProperty::Int , 0 , m_iLastPattern + 1 );
  addProperty( SProperty::EDITBOX , _T( "NewPatternName" ) , &m_sPatternName ,
    SProperty::String );

};

void TrackerGadget::ConnectorsRegistration()
{
	addInputConnector( transparent, "Image");
  addOutputConnector( createComplexDataType( 3 , rectangle , text , vframe ) , "Results" );
  addOutputConnector( createComplexDataType( 3 , rectangle , text , vframe ) , "Correlation" );
  addDuplexConnector( transparent , transparent , "Control" );
};
CDPoint  TrackerGadget::TrackTemplateMatching(cv::Mat &img, const cv::Mat &templ, double &dMatch, bool bSearchAllImage)
{
	if (m_template.data == NULL)
		return CDPoint(-1, -1);

	cv::Mat cropedImg;
	if ((m_cdTrackPt.x != -1 && m_cdTrackPt.y != -1) && !bSearchAllImage)
	{
		cropedImg = CropROI(img, m_cdTrackPt, m_dRadius, m_dTemplateToROICoeff);	
	}
	else
	{
		img.copyTo(cropedImg);
	}

	//cv::imwrite("e:\\templ.jpg", templ);
	//cv::imwrite("e:\\cropedImg.jpg", cropedImg);

	//cv::imwrite("e:\\templEdges.jpg", GetEdges(templ));
	//cv::imwrite("e:\\cropedImgEdges.jpg", GetEdges(cropedImg));

	int match_method = CV_TM_CCOEFF_NORMED;

	//switch (atoi(m_sMatchMethod))
	//{
	//	case 0:
	//	{
	//		match_method = CV_TM_SQDIFF;
	//	}
	//	break;
	//	case 1:
	//	{
	//		match_method = CV_TM_SQDIFF_NORMED;
	//	}
	//	break;
	//	case 2:
	//	{
	//		match_method = CV_TM_CCORR;
	//	}
	//	break;
	//	case 3:
	//	{
	//		match_method = CV_TM_CCORR_NORMED;
	//	}
	//	break;
	//	case 4:
	//	{
	//		match_method = CV_TM_CCOEFF;
	//	}
	//	break;
	//	case 5:
	//	{
	//		match_method = CV_TM_CCOEFF_NORMED;
	//	}
	//	break;
	//default:
	//	break;
	//}

	cv::Mat result;	
	matchTemplate(cropedImg, templ, result, match_method);
	//matchTemplate(GetEdges(cropedImg), GetEdges(templ), result, match_method);


	//normalize(result, result, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());
	//normalize(result, result, 0, 1, cv::NORM_HAMMING, -1, cv::Mat());

	double minVal; double maxVal; cv::Point minLoc; cv::Point maxLoc;
	cv::Point matchLoc0;
	minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat());

	if (match_method == CV_TM_SQDIFF || match_method == CV_TM_SQDIFF_NORMED)
	{
		matchLoc0 = minLoc;
		dMatch = minVal;
	}
	else
	{
		matchLoc0 = maxLoc;
		dMatch = maxVal;
	}

	if (bSearchAllImage)
	{
		double newX = matchLoc0.x + templ.cols / 2;
		double newY = matchLoc0.y + templ.rows / 2;

		m_cdTrackPt.x = newX;
		m_cdTrackPt.y = newY;
	}
	else
	{
		double newX = m_ROI_Origin.x - cropedImg.cols / 2 + (matchLoc0.x + templ.cols / 2);
		double newY = m_ROI_Origin.y - cropedImg.rows / 2 + (matchLoc0.y + templ.rows / 2);

		if (abs(newX - m_cdTrackPt.x) > 2 || abs(newY - m_cdTrackPt.y) > 2)
		{
			m_cdTrackPt.x = newX;
			m_cdTrackPt.y = newY;
		}
	}


    cropedImg.release();
	return m_cdTrackPt;
}
CDPoint  TrackerGadget::GetNextCenter(CDPoint pt, double time)
{
	return pt;
	if (m_prevCenter.x == -1 && m_prevCenter.y == -1)
	{
		m_prevCenter = pt;
		return pt;
	}
	else
	{
      CDPoint newCenter(pt.x + pt.x - m_prevCenter.x, pt.y + pt.y - m_prevCenter.y);
	  m_prevCenter = pt;
	  return newCenter;
	}


}
cv::Mat TrackerGadget::GetEdges(const cv::Mat &img)
{
	/// Canny detector
	cv::Mat detected_edges;
	int edgeThresh = 1;
	int lowThreshold = 5;
	int ratio = 3;
	int kernel_size = 3;
	cv::Canny(img, detected_edges, lowThreshold, lowThreshold*ratio, kernel_size);
	return detected_edges;
}

size_t TrackerGadget::LoadTemplates()
{
  namespace fs = std::filesystem;

  fs::path dirPath = ( LPCTSTR ) m_sPatternsDir ;
  Template NewTemplate ;
  m_Templates.clear() ;
  m_iMaxIndex = 1 ;
  try
	{
    for ( const auto& entry : fs::directory_iterator( dirPath ) )
	{
      if ( entry.is_regular_file() )
	{

        if ( NewTemplate.Load( entry.path().string().c_str() ) )
	{
          m_Templates.push_back( NewTemplate ) ;
          if ( m_iMaxIndex < NewTemplate.m_iIndex )
            m_iMaxIndex = NewTemplate.m_iIndex ;
	}
        else
	{
          SENDERR( "Can't load template %s" , entry.path().c_str() ) ;
	}
	}
	}
    return m_Templates.size() ;
  }
  catch ( const std::filesystem::filesystem_error& e )
  {
    SENDERR( "Tracker::TemplatesLoad : Exception %s" , e.what() ) ;
}
  return 0;
};
