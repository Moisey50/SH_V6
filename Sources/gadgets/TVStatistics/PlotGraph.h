#pragma once
#include <gadgets\gadbase.h>
#include <userinterface\PlotGraphView.h>

class PlotGraph :
  public CRenderGadget
{
protected:
  FXLockObject     m_Lock;
  CPlotGraphView*  m_wndOutput;
  COutputConnector* m_pOutput;
  CXYMinMaxes m_MinMaxes;
  BOOL   m_bFit;
  BOOL   m_bViewRanges ;
  int    m_iNSamples ;
  BOOL   m_bViewNames ;
  int    m_iViewNet ;
  double m_dSamplePeriod_ms ;
  CXYMinMaxes m_Labels ;
  CWnd * m_pOwnWnd ;
  HWND   m_hExternalWnd ;
  CWnd * m_pAttachedWnd ;
  int    m_iNAttachments ;
  FXString        m_GadgetInfo ;
public:
  void ShutDown();
  void Attach(CWnd* pWnd);
  void Detach();
  bool PrintProperties(FXString& text);
  bool ScanProperties(LPCTSTR text, bool& Invalidate);
  bool ScanSettings(FXString& text);

private:
  PlotGraph();
	void Render(const CDataFrame* pDataFrame);
  int  GetOutputsCount()                { return 1; }
  COutputConnector* GetOutputConnector(int n)   { return (n==0)?m_pOutput:NULL; }
  virtual bool ReceiveEOS(const CDataFrame* pDataFrame);

  CWnd*GetRenderWnd() { return m_wndOutput; }
  void GetDefaultWndSize (RECT& rc) { rc.left=rc.top=0; rc.right=GADGET_VIDEO_WIDTH; rc.bottom=GADGET_VIDEO_HEIGHT; }
  LPCTSTR GetGadgetInfo() { return (LPCTSTR) m_GadgetInfo ; }
  DECLARE_RUNTIME_GADGET(PlotGraph);
};
