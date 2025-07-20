// GridCalbrator.cpp: implementation of the CGridCalbrator class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GridCalbrator.h"
#include <gadgets\textframe.h>
#include <gadgets\VideoFrame.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_RUNTIME_GADGET_EX(GridCalibrator, CFilterGadget, LINEAGE_FILEX, TVDB400_PLUGIN_NAME);


GridCalibrator::GridCalibrator()
{
	m_pInput = new CInputConnector(vframe);
	m_pOutput = new COutputConnector(text);
  m_OutputMode = modeReplace ;
	Resume();
}

void GridCalibrator::ShutDown()
{
  CFilterGadget::ShutDown();
	delete m_pInput;
	m_pInput = NULL;
	delete m_pOutput;
	m_pOutput = NULL;
}

CDataFrame* GridCalibrator::DoProcessing(const CDataFrame* pDataFrame)
{
  CTextFrame* ResultFrame = NULL;
	if( !Tvdb400_IsEOS(pDataFrame) )
  {
    const CVideoFrame* pVideoFrame = pDataFrame->GetVideoFrame(DEFAULT_LABEL);
    FXString str;
    if (pVideoFrame && m_Grid.CalcMetric((const pTVFrame)pVideoFrame) 
      && m_Grid.VerboseMetricPK(str))
    {
      ResultFrame = CTextFrame::Create(str);
      ResultFrame->ChangeId(pDataFrame->GetId());
      ResultFrame->SetTime(pDataFrame->GetTime());
    }
  }
	return ResultFrame;
}
