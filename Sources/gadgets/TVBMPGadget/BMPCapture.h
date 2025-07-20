// BMPCapture.h: interface for the BMPCapture class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BMPCAPTURE_H__49E5D762_4105_409D_8321_BADA20887367__INCLUDED_)
#define AFX_BMPCAPTURE_H__49E5D762_4105_409D_8321_BADA20887367__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <gadgets\gadbase.h>
#include <fxfc\fxfc.h>
#include <list>

using namespace std;

static LPCTSTR Properties[] = {
  "Directory" ,
  "FileName" ,
  "FrameRate" ,
  "WatchNew" ,
  "LabelFullPath" ,
  "IterationMode" ,
  "Output" ,
  "Grab" ,
  NULL
} ;

static LPCTSTR MainSettings = 
"EditBox(Directory)"
",EditBox(FileName)"
",Spin(FrameRate,1,25)"
",ComboBox(WatchNew(true(1),false(0)))"
",ComboBox(LabelFullPath(true(1),false(0)))"
",ComboBox(IterationMode(Native(0),ByTime(1),ASCIINumber(2)))"
",ComboBox(Output(Y8(0),Color(1)))" ;

static LPCTSTR SubPattern1 = ",ComboBox(DeleteRead(true(1),false(0)))" ;
static LPCTSTR SubPattern2 = ",ComboBox(Loop(true(1),false(0)))"
                             ",ComboBox(Remove#(No(0),Yes(1)))";

class BMPCapture : public CCaptureGadget
{
private:
  FXString 		  m_PathName;
  FXString 		  m_FileName;
  FXString 		  m_Directory;
  FXFileFindEx	m_FF;
  BOOL					m_ffWorking;
  BOOL          m_bNameChanged ;
  int						m_FrameRate;
  BOOL					m_Loop;
  BOOL          m_LabelFullPath;
  BOOL          m_WatchNew;
  BOOL          m_DeleteRead;
  int           m_RemoveNumberInlabel;
  FFIterationMode m_IterationMode;
  int           m_ColorOrBW ; // 0 - BW , 1 - Color, if possible
  // sync
  CInputConnector*    m_pInputTrigger;
  CDuplexConnector * m_pDuplex;
  HANDLE			    m_evSWTriggerPulse;
public:
  BMPCapture();
  virtual void ShutDown();
  //
  virtual void OnStart();
  virtual void OnStop();

  virtual int GetDuplexCount() { return (m_pDuplex) ? 1 : 0; }
  virtual CDuplexConnector* GetDuplexConnector(int n) { return (n == 0) ? m_pDuplex : NULL; }

  LPCTSTR GetKey(const FXString& cmd) const
  {
    LPCTSTR pRes = NULL;

    LPCTSTR * pPropIter = Properties ;

    while ( *pPropIter != NULL )
    {
      if ( cmd.Find( *pPropIter ) == 0 ) // match
      {
        pRes = *pPropIter ;
        break ;
      }
      pPropIter++ ;
    }

    return pRes;
  }

  bool    ScanSettings( FXString& text );
  bool    ScanProperties( LPCTSTR text , bool& Invalidate );
  bool    PrintProperties( FXString& text );
  void	  CameraTriggerPulse( CDataFrame* pDataFrame );

  void AsyncTransaction(CDuplexConnector * pConnector, CDataFrame * pParamFrame);

  virtual int GetInputsCount() { return ( m_pInputTrigger ) ? 1 : 0; }
  virtual CInputConnector* GetInputConnector( int n ) { return ( n == 0 ) ? m_pInputTrigger : NULL; }

  virtual CDataFrame* GetNextFrame( double* StartTime );
  bool IsTriggerByInputPin() { return ( ( m_pInputTrigger ) && ( m_pInputTrigger->IsConnected() ) ); }
  static friend void CALLBACK BMPCapture_capture_trigger( CDataFrame* pDataFrame , void* pGadget , CConnector* lpInput )
  {
    ( ( BMPCapture* ) pGadget )->CameraTriggerPulse( pDataFrame );
  }
  DECLARE_RUNTIME_GADGET( BMPCapture );
};

#endif // !defined(AFX_BMPCAPTURE_H__49E5D762_4105_409D_8321_BADA20887367__INCLUDED_)
