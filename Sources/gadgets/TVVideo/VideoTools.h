// VideoTools.h: interface for the CVideoTools class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VIDEOTOOLS_H__54FBF75A_7DD0_4CF9_84ED_1A41FF9BC2A7__INCLUDED_)
#define AFX_VIDEOTOOLS_H__54FBF75A_7DD0_4CF9_84ED_1A41FF9BC2A7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Gadgets\gadbase.h>
#include <imageproc\averager.h>
#include <helpers/FramesHelper.h>
enum ComplementOutput
{
  CO_AverageValue = 0 ,
  CO_FrameCounter 
};

class VideoAverage : public CGadget
{
	CInputConnector*  m_pInput;
	COutputConnector* m_pVideoOut;
	COutputConnector* m_pValueOut;
	CAverager         m_Averager;
  int               m_iFrameCounter ;
  ComplementOutput  m_ComplementOutput ;
private:
	VideoAverage();
public:
	virtual void ShutDown();
	virtual int GetInputsCount();
	virtual int GetOutputsCount();
	CInputConnector* GetInputConnector(int n);
	COutputConnector* GetOutputConnector(int n);
	virtual bool PrintProperties(FXString& text);
	virtual bool ScanProperties(LPCTSTR text, bool& Invalidate);
	bool         ScanSettings(FXString& text);
private:
	virtual int DoJob();
	friend class VideoAverageDialog;
	DECLARE_RUNTIME_GADGET(VideoAverage);
};

#endif // !defined(AFX_VIDEOTOOLS_H__54FBF75A_7DD0_4CF9_84ED_1A41FF9BC2A7__INCLUDED_)			   
