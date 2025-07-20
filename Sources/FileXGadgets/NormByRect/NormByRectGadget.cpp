// UserExampleGadget.h.h : Implementation of the UserExampleGadget class


#include "StdAfx.h"
#include "NormByRectGadget.h"

USER_FILTER_RUNTIME_GADGET(NormByRect, "Video.color&brightness");	//	Mandatory

NormByRect::NormByRect()
{
	m_iX = 1, m_iY =1, m_iHeight = 480, m_iWidth = 640;
  m_dCoefficient=500;
  m_bSelectByLabel = FALSE ;
  m_iAveraging = 10 ;
  m_WorkingMode = NormByMult ;
	init();
}

int NormByRect::ExtractWaveLength( FXString& Label )
{
  //update
  if ( !Label.IsEmpty() )
  {
    int iWaveLength = atoi( Label ) ;
    if ( iWaveLength == 0 )
      SEND_GADGET_ERR( "Bad wave length label: %s" , ( LPCTSTR ) Label ) ;
    return iWaveLength ;
  }
  return 0 ;
}

CDataFrame* NormByRect::DoProcessing(const CDataFrame* pDataFrame)
{
  if ( m_GadgetInfo.IsEmpty() )
    GetGadgetName( m_GadgetInfo ) ;

	if ((pDataFrame) && (!Tvdb400_IsEOS(pDataFrame)))
	{
	  const CVideoFrame* pInputPictureData = pDataFrame->GetVideoFrame();
    if ( !pInputPictureData )
      return NULL ;
		//	Get Picture Description
		LPBITMAPINFOHEADER pInputInfoHeader = pInputPictureData->lpBMIH;
		DWORD dwCompression = pInputInfoHeader->biCompression;
		//	Get Picture Width
		LONG imgWidth = pInputInfoHeader->biWidth;
		//	Get Picture Height
		LONG imgHeight = pInputInfoHeader->biHeight;
		LPBYTE pImg = GetData(pInputPictureData);
		LONG imgSize = (imgHeight - 2) * imgWidth; // doesn't process first and last row

		unsigned long long brightnessCnt = 0;
    if ( m_iX < 0 || m_iY < 0 || m_iX >= imgWidth || m_iY > imgHeight )
      return NULL ;

		if ( m_iX + m_iWidth >= imgWidth )
      m_iWidth = imgWidth - m_iX ;
    if ( m_iY + m_iHeight >= imgHeight )
      m_iHeight = imgHeight - m_iY ;

    if ( m_iHeight <= 0 || m_iWidth <= 0 )
      return NULL ;

		switch (dwCompression)
		{
		case BI_Y8:
		case BI_Y800:
		case BI_YUV9:
		case BI_YUV12:
		{		
			pImg += m_iY * imgWidth + m_iX;
			for (LONG y = 0; y < m_iHeight; y++)
			{	
        LPBYTE pIter = pImg ;
        LPBYTE pEnd = pImg + m_iWidth ;
        while ( pIter < pEnd )
          brightnessCnt += *(pIter++) ;
				pImg += imgWidth;
			}
		}
		break;
		case BI_Y16:
		{
      LPWORD pImg16 = ( LPWORD )pImg ;
      pImg16 += m_iY * imgWidth + m_iX;
			for (LONG y = 0; y < m_iHeight; y++)
			{
				LPWORD p16 = pImg16;
        LPWORD pEnd = pImg16 + m_iWidth ;
        while ( p16 < pEnd )
          brightnessCnt += *(p16++);
        pImg16 += imgWidth ;
      }
		}
		break;
		default:
			return NULL;
		}
		double dAverageBrightness = brightnessCnt / (double)(m_iHeight * m_iWidth);

    FXString Label = pInputPictureData->GetLabel() ;
    IntDataMap::iterator it ;
    LabeledAverageData * pAveData = NULL ;
    if ( m_WorkingMode == NormAdd )
    {
      int iWaveLength = ( !Label.IsEmpty() && m_bSelectByLabel ) ? 
        ExtractWaveLength( Label ) : 0 ;
      it = m_WaveLengthMap.find( iWaveLength ) ;
      if ( it != m_WaveLengthMap.end() )
      {
        LabeledAverageData& AverData = it->second ;
        if ( ++AverData.m_iAveraged == 0 )
          AverData.m_dAveragedValue = AverData.m_dCurrentValue = dAverageBrightness ;
        else 
        {
          double dKOld = ( double ) ( AverData.m_iAveraged - 1 ) /
            ( double ) ( AverData.m_iAveraged ) ;
          AverData.m_dAveragedValue *= dKOld ;
          AverData.m_dAveragedValue += dAverageBrightness * ( 1. - dKOld ) ;
          AverData.m_dCurrentValue = dAverageBrightness ;
        }
        if ( AverData.m_iAveraged >= AverData.m_iAveraging )
          AverData.m_iAveraged = AverData.m_iAveraging - 1 ;
        pAveData = &AverData ;
      }
      else
      {
        LabeledAverageData NewWaveLengthData( m_iAveraging ) ;
        NewWaveLengthData.m_dAveragedValue = NewWaveLengthData.m_dCurrentValue = dAverageBrightness ;
        NewWaveLengthData.m_iAveraged = 1 ;
        m_WaveLengthMap[ iWaveLength ] = NewWaveLengthData ;
        pAveData = &(m_WaveLengthMap.find( iWaveLength )->second) ;
      }
    }
    //	Get Picture Data from Input Package

		CContainerFrame* resVal  = CContainerFrame::Create();
    CVideoFrame* pvf = (CVideoFrame*)pInputPictureData->Copy() ;
    if ( pvf )
    {
      pvf->SetTime( GetHRTickCount() );

      switch ( m_WorkingMode )
      {
        case NormByMult: 
        {
          double dMult = m_dCoefficient / dAverageBrightness  ;
          CorrectByMultiplication( pvf , dMult ) ;
	        pvf->SetLabel("MultOutput");
        }
        break ;
        case NormAdd:
        {
          if ( pAveData )
          {
            CorrectAdditiveNoise( pvf , *pAveData ) ;
            pvf->SetLabel( "AdditiveOutput" );
          }
          else
            pvf->SetLabel( _T( "NoAverageData" ) ) ;
        }
      }
		  pvf->CopyAttributes(pDataFrame);
		  resVal->AddFrame(pvf);
		  if (m_bShowROI == 1)
		  {
			  CFigureFrame* ff = CFigureFrame::Create();
			  ff->Attributes()->WriteString("color", "0x00ff00");
			  ff->AddPoint(CDPoint(m_iX, m_iY));
			  ff->AddPoint(CDPoint(m_iX + m_iWidth, m_iY));
			  ff->AddPoint(CDPoint(m_iX + m_iWidth, m_iY + m_iHeight));
			  ff->AddPoint(CDPoint(m_iX, m_iY + m_iHeight));
			  ff->AddPoint(CDPoint(m_iX, m_iY));
			  ff->ChangeId(pDataFrame->GetId());
			  resVal->AddFrame(ff);
		  }
    }
			
    resVal->CopyAttributes( pDataFrame ) ;
		return resVal;
		//return pvf;
	}
	return NULL;
}
void NormByRect::AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame)
{
  pParamFrame->Release( pParamFrame );
};
void NormByRect::PropertiesRegistration()
{
  addProperty( SProperty::SPIN , _T( "X" ) , &m_iX , SProperty::Int , 0 , 10000 );
  addProperty( SProperty::SPIN , _T( "Y" ) , &m_iY , SProperty::Int , 0 , 10000 );
  addProperty( SProperty::SPIN , _T( "Width" ) , &m_iWidth , SProperty::Int , 1 , 10000 );
  addProperty( SProperty::SPIN , _T( "Height" ) , &m_iHeight , SProperty::Int , 1 , 10000 );
	addProperty(SProperty::EDITBOX, _T("Coefficient"), &m_dCoefficient, SProperty::Double);
	addProperty(SProperty::SPIN, _T("ShowROI"), &m_bShowROI, SProperty::Int,0,1);
  addProperty( SProperty::COMBO , _T( "WorkingMode" ) , &m_WorkingMode ,
    SProperty::Int , _T( "NoProcess;Multiplicative;Additive;" ) );
  addProperty( SProperty::SPIN , _T( "UseLabels" ) , &m_bSelectByLabel , SProperty::Int , 0 , 1 );
  addProperty( SProperty::SPIN , _T( "Averaging" ) , &m_iAveraging , SProperty::Int , 0 , 1000 ) ;

};
void NormByRect::ConnectorsRegistration()
{
	addOutputConnector(vframe, "Result");	
	addInputConnector(vframe, "InputName1");
}

CVideoFrame * NormByRect::CorrectByMultiplication( 
  CVideoFrame * pFrame , double dMult )
{
  LPBYTE pImg = GetData( pFrame ) ;
  int imgSize = GetImageSize( pFrame ) ;
  DWORD dwCompr = pFrame->lpBMIH->biCompression ;
  switch ( dwCompr )
  {
    case BI_Y8:
    case BI_Y800:
    case BI_YUV9:
    case BI_YUV12:
    {
      LPBYTE pEnd = pImg + imgSize ;
      while ( pImg < pEnd )
      {
        int iVal = ( int ) ( *( pImg ) * dMult ) ;
        *( pImg++ ) = ( iVal > 255 ) ? 255 : ( BYTE ) iVal ;
      }
    }
    break;
    case BI_Y16:
    {
      LPWORD p16 = ( LPWORD ) pImg;
      LPWORD pEnd = p16 + imgSize/sizeof(WORD) ;
      while ( p16 < pEnd )
      {
        int iVal = ( int ) ( *( p16 ) * dMult ) ;
        *( p16++ ) = ( iVal > 65535 ) ? 65535 : ( WORD ) iVal ;
      }
    }
    break;
    default:
      TRACE( "\nUnsupported compression %u(0x%8x)  " , dwCompr , dwCompr ) ;
      break ;
  }
  return pFrame ;
}
CVideoFrame * NormByRect::CorrectAdditiveNoise(
  CVideoFrame * pFrame , LabeledAverageData& Params )
{
  LPBYTE pImg = GetData( pFrame ) ;
  int imgSize = GetImageSize( pFrame ) ;
  DWORD dwCompr = pFrame->lpBMIH->biCompression ;
  int iCorrection = ROUND(Params.m_dCurrentValue - Params.m_dAveragedValue) ;
  switch ( dwCompr )
  {
    case BI_Y8:
    case BI_Y800:
    case BI_YUV9:
    case BI_YUV12:
    {
      LPBYTE pEnd = pImg + imgSize ;
      while ( pImg < pEnd )
      {
        int iVal = ( int ) ( *( pImg )) - iCorrection ;
        BOUND( iVal , 0 , 255 ) ;
        *( pImg++ ) = iVal ;
      }
    }
    break;
    case BI_Y16:
    {
      LPWORD p16 = ( LPWORD ) pImg;
      LPWORD pEnd = p16 + imgSize / sizeof( WORD ) ;
      while ( p16 < pEnd )
      {
        int iVal = ( int ) ( *( p16 )) - iCorrection ;
        BOUND( iVal , 0 , 65535 ) ;
        *( p16++ ) = iVal ;
      }
    }
    break;
    default:
      TRACE( "\nUnsupported compression %u(0x%8x)  " , dwCompr , dwCompr ) ;
      break ;
  }
  return pFrame ;
}