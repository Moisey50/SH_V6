#include "stdafx.h"
#include "DataMarixReader.h"
#include <Gadgets\TextFrame.h>
#include <Gadgets\VideoFrame.h>

#define THIS_MODULENAME "DataMatrix.cpp"


#define PASSTHROUGH_NULLFRAME(vfr, fr)			\
{												\
	if (!(vfr) || ((vfr)->IsNullFrame()))		\
    {	                                        \
		return NULL;			                \
    }                                           \
}


IMPLEMENT_RUNTIME_GADGET_EX(DataMatrixReader, CFilterGadget, "Video.recognition", TVDB400_PLUGIN_NAME);


DataMatrixReader::DataMatrixReader(void)
{
	m_pInput   =    new CInputConnector(vframe);
	m_pOutput  =    new COutputConnector(text);
	Resume();
}

void DataMatrixReader::ShutDown()
{
    CFilterGadget::ShutDown();
	delete m_pInput;
	m_pInput = NULL;
	delete m_pOutput;
	m_pOutput = NULL;
}

CDataFrame* DataMatrixReader::DoProcessing(const CDataFrame* pDataFrame)
{
	const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame(DEFAULT_LABEL);
	PASSTHROUGH_NULLFRAME(VideoFrame, pDataFrame);			//data pointers (incase there is data)
	FXString retS;											//result string

	double scan39time=GetHRTickCount();
	if (m_Reader.Read(VideoFrame,retS))
	{
		scan39time=GetHRTickCount()-scan39time;
		FXString retV;
		retV.Format("%f. ",scan39time);
		retV="\nScan time:  "+retV+"\nDataMatrix: "+retS ;	
		CTextFrame* retVal=CTextFrame::Create(retV);
            retVal->CopyAttributes(pDataFrame);;
		return retVal;
	}
	
/*	else	//Uncomment if wanted to see scan time for failed scans.
	{
		scan39time=get_current_time()-scan39time;
		CString retV;
		retV.Format("%f",scan39time);
		retV="\nScan time:  "+retV+"  Failed to find " ;				//writing the answer
		CTextFrame* retVal=CTextFrame::Create(&retV);
            retVal->CopyAttributes(pDataFrame);;
		pDataFrame->Release();
		return retVal;
	}
*/	
	return NULL;
}
