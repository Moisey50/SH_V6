// BlackAreaBin.h.h : Implementation of the BlackAreaBin class


#include "StdAfx.h"
#include "BlackAreaBin.h"

USER_FILTER_RUNTIME_GADGET(BlackAreaBin,"Statistics");

BlackAreaBin::BlackAreaBin()
{

	init();
}

double BlackAreaBin::countSpotsDensity(UINT8* pProcessingPicture, unsigned int uiProcessingPictureSize)
{
	unsigned int uiNonBlackPixelsCnt = 0;

	for(unsigned int uiCnt=0; uiCnt<uiProcessingPictureSize; uiCnt++)
	{
		if( pProcessingPicture[uiCnt] > 0 )
			uiNonBlackPixelsCnt++;
	}

	unsigned int uiBlackPixelsCnt = uiProcessingPictureSize - uiNonBlackPixelsCnt;

	double dDensity_percents = 100.0 * ((double)uiBlackPixelsCnt / uiProcessingPictureSize);
	return dDensity_percents;
}

CDataFrame* BlackAreaBin::DoProcessing(const CDataFrame* pDataFrame) 
{
	//	Get Picture Data from Input Package
	const CVideoFrame* pInputPictureData = pDataFrame->GetVideoFrame();	

	//	Get Picture Description
	LPBITMAPINFOHEADER pInputInfoHeader = pInputPictureData->lpBMIH;

	//	Get Picture Width
	int iWidth	= pInputInfoHeader->biWidth;
	//	Get Picture Height
	int iHeight	= pInputInfoHeader->biHeight;

	//	Get Picture pointer
	LPBYTE pPicture = GetData(pInputPictureData);

	//	Drop First and Last rows from picture
	UINT8* pProcessingPicture = &((UINT8*)pPicture)[iWidth];
	unsigned int uiProcessingPictureSize = (iWidth*iHeight) - (2*iWidth);

	//	Calculate Spots Density
	double dDensity_percents = countSpotsDensity(pProcessingPicture, uiProcessingPictureSize);

	// Create Output Package
	CQuantityFrame* pOutputPackage = CQuantityFrame::Create(dDensity_percents);
	pOutputPackage->CopyAttributes(pDataFrame);

	//	Return with casting to general class (CDataFrame*) for match framework
	return (CDataFrame*) pOutputPackage;
}

void BlackAreaBin::AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame) 
{
  pParamFrame->Release( pParamFrame );
};

void BlackAreaBin::PropertiesRegistration() 
{

};

void BlackAreaBin::ConnectorsRegistration() 
{
	addInputConnector( vframe, "PicInput");
	addOutputConnector(	quantity,	"AreaQuantity");
};




