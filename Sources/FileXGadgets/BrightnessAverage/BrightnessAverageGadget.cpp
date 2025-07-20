// BrightnessAverageGadget.h.h : Implementation of the BrightnessAverageGadget class


#include "StdAfx.h"
#include <helpers\FramesHelper.h>
#include <imageproc\statistics.h>
#include "BrightnessAverageGadget.h"

USER_FILTER_RUNTIME_GADGET(BrightnessAverageGadget,"Video.statistics");

BrightnessAverageGadget::BrightnessAverageGadget()
{
	init();
}

CDataFrame* BrightnessAverageGadget::DoProcessing(const CDataFrame* pDataFrame) 
{
	if ((pDataFrame) && (!Tvdb400_IsEOS(pDataFrame)))
	{
		//	Get Picture Data from Input Package
		const CVideoFrame* pInputPictureData = pDataFrame->GetVideoFrame();

		//	Get Picture Description
		LPBITMAPINFOHEADER pInputInfoHeader = pInputPictureData->lpBMIH;
		DWORD dwCompression = pInputInfoHeader->biCompression ;
		//	Get Picture Width
		LONG imgWidth	= pInputInfoHeader->biWidth;
		//	Get Picture Height
		LONG imgHeight	= pInputInfoHeader->biHeight;
		LPBYTE pImg = GetData(pInputPictureData) ;
		LONG imgSize = (imgHeight - 2) * imgWidth ; // doesn't process first and last row
		
		unsigned long long brightnessCnt = 0;

		switch( dwCompression )
		{
		case BI_Y8:
		case BI_Y800:
		case BI_YUV9:
		case BI_YUV12:
			{
				pImg += imgWidth ; // Doesn't include first row

				for(LONG uiCnt=0; uiCnt<imgSize; uiCnt++)
				{
					brightnessCnt += pImg[uiCnt];
				}
			}
			break ;
		case BI_Y16:
			{
				pImg += imgWidth * 2; // omit first row, 2 bytes per pixel

				LPWORD p16 = (LPWORD)pImg ;
				for(LONG uiCnt=0; uiCnt<imgSize; uiCnt++)
				{
					brightnessCnt += p16[uiCnt];
				}
			}
      break ;
		default: 
			return NULL ;
		}

		double dAverageBrightness = brightnessCnt/(double)imgSize;

		// Create Output Frame
		CQuantityFrame* pOutputFrame = CQuantityFrame::Create(dAverageBrightness);
		if ( pOutputFrame )
		{
			pOutputFrame->CopyAttributes( pDataFrame ) ;
			pOutputFrame->SetLabel( _T("AverageBrightness") ) ;
		}

		//	Return with casting to general class (CDataFrame*) for match framework
		return  pOutputFrame;
	}
	return NULL ;
}

void BrightnessAverageGadget::AsyncTransaction(
  CDuplexConnector* pConnector, CDataFrame* pParamFrame) 
{
  pParamFrame->Release( pParamFrame );
};

void BrightnessAverageGadget::PropertiesRegistration() 
{
};

void BrightnessAverageGadget::ConnectorsRegistration() 
{
	addInputConnector( vframe, "ImgInput");
	addOutputConnector(	transparent , "AverageBrightness");
};



USER_FILTER_RUNTIME_GADGET( CalcDiffSum , "Video.statistics" );

CalcDiffSum::CalcDiffSum()
{
  m_Area = CRect( -1 , -1 , -1 , -1 ) ;
  m_FrameCntr = 0 ;
  m_WorkingMode = DS_AverageDiff ;
  init();
}


CDataFrame* CalcDiffSum::DoProcessing( const CDataFrame* pDataFrame )
{
  const CTextFrame * pt = pDataFrame->GetTextFrame() ;
  if ( pt )
  {
    if ( !_tcscmp( pt->GetLabel() , _T( "ROI" ) ) )
    {
      FXString Control = pt->GetString() ;
      bool bInv = false ;
      UserGadgetBase::ScanProperties( Control , bInv ) ;
    }
  }
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  if ( !VideoFrame )
    return NULL ;

  CRect RealArea( m_Area ) ;
  //RealArea.right += RealArea.left ;
  //RealArea.bottom += RealArea.top ;
  //if ( RealArea.Width() <= 0  ||  RealArea.Height() <= 0
  //  || RealArea.left < 0 || RealArea.top < 0 )
  //{
  //  RealArea.left = RealArea.top = 0 ;
  //  RealArea.bottom = GetHeight( VideoFrame ) - 1 ;
  //  RealArea.right = GetWidth( VideoFrame ) - 1 ;
  //}
  //else
  //{
    RealArea.right += RealArea.left ;
    RealArea.bottom += RealArea.top ;
  //}
  double dDiff = 0. ;
  switch ( m_WorkingMode )
  {
    case DS_AverageDiff: 
      dDiff = _calc_diff_sum( VideoFrame , RealArea );
      break ;
    case DS_AbsMAxDiff:
      dDiff = _find_max_diff( VideoFrame , RealArea );
      break ;
    case DS_Laplace:
      dDiff = _calc_laplace( VideoFrame , RealArea );
      break ;
  }

  if ( !m_Format.IsEmpty() )
  {
    CTextFrame* retVal = CTextFrame::Create();
    retVal->GetString().Format( _T( "dDiff=%8.1f" ) , dDiff ) ;

    retVal->SetTime( pDataFrame->GetTime() );
    retVal->ChangeId( m_FrameCntr++ );
    return retVal;
  }
  else
  {
    CQuantityFrame * pRetVal = CreateQuantityFrame( dDiff , NULL , m_FrameCntr++ ) ;
    return pRetVal;
  }

}

void CalcDiffSum::AsyncTransaction( CDuplexConnector* pConnector , CDataFrame* pParamFrame )
{
  if ( !pParamFrame )
    return ;
  pParamFrame->Release( pParamFrame );
};

static LPCTSTR pList = _T( "AverageDiff;MaxDiff;Laplace;" ) ;

void CalcDiffSum::PropertiesRegistration()
{
  addProperty( SProperty::SPIN , _T( "XOff" ) ,
    &m_Area.left , SProperty::Int , 0 , 100000 );
  addProperty( SProperty::SPIN , _T( "YOff" ) ,
    &m_Area.top , SProperty::Int , 0 , 100000 );
  addProperty( SProperty::SPIN , _T( "Width" ) ,
    &m_Area.right , SProperty::Int , 1 , 100000 );
  addProperty( SProperty::SPIN , _T( "Height" ) ,
    &m_Area.bottom , SProperty::Int , 1 , 100000 );
  addProperty( SProperty::EDITBOX , _T( "Format" ) ,
    &m_Format , SProperty::String );
  addProperty( SProperty::COMBO , _T( "WorkingMode" ) , &m_WorkingMode ,
    SProperty::Int , pList ) ;
};

void CalcDiffSum::ConnectorsRegistration()
{
  addInputConnector( transparent , "ImgInput" );
  addOutputConnector( quantity , "AverageDiff" );
  addDuplexConnector( text , text , "Params" ) ;
};


