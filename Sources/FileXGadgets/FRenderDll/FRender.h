// FRender.h : Declaration of the FRender class

#pragma once
#include <gadgets\gadbase.h>
#include <Gadgets\VideoFrame.h>
#include "dibfrender.h"
#include "SpyWnd.h"
#include <helpers\SharedMemBoxes.h>
//#include <gadgets\IPCamera.h>
//#include <map>

#define DEFAULT_MONOCHROME 0
#define DEFAULT_SCALE       (-1)

#define KB_CTRL    0x01
#define KB_SHIFT   0x02
#define KB_LCNTRL  0x04
#define KB_RCNTRL  0x08
#define KB_LSHIFT  0x10
#define KB_RSHIFT  0x20



typedef enum
{
  ObjSel_Disabled = 0 ,
  ObjSel_Enabled = 1
} ObjectsSelection ;


//#define MAX_IP_CAMERAS 64
class FRender : public CRenderGadget //,public CIPCameraMapper
{
protected:
  FXLockObject     m_Lock;
  CDIBFRender    * m_wndOutput ;
//   CWnd           * m_pDisplayView = NULL ; // for full screen view
  CWnd           * m_pNormalView = NULL ;
  bool            m_bInitialized = false ;

  CWnd            m_ScreenWnd ;
  bool            m_bMaximized = false ;
  CRect           m_NormalRect ;
  COutputConnector * m_pOutput; // output of info about mouse activities
  COutputConnector * m_pImageOutput ; // Formed images output
  CDuplexConnector * m_pControl ;
  int             m_Scale;         // Zoom switching
  CPoint          m_ViewPos ;
  int             m_Monochrome;    // 0 - color, 1-mono, 2 - convert RAW to mono, 3 - RAW to color
  LengthViewMode  m_LengthViewMode = LVM_ViewBoth ; // allows line selection and view in pixels, 
                                                   // units and/or Tenth if scale is in microns
  bool            m_RectSelection; // allows rectangle selection
  FXString        m_SaveImagesDir ;
  bool            m_bViewRGB ;
  ObjectsSelection  m_ObjectSelection ;
  int             m_bShowLabel ;
  CPoint          m_PointOfInterest;
  bool            m_bSomeSelected ;
  bool            m_bCntrlWasPressed ;
  bool            m_bLButtonWasPressed ;
  CWnd *          m_pOwnWnd ;
  HWND            m_hExternalWnd = NULL ;
  HWND            m_hNormalWnd = NULL ; // for h saving when zoom for full screen activated
  CWnd *          m_pAttachedWnd ;
  FXString        m_sUnits ;
  double          m_dScale ; // Scale for conversion from pixels to units 
  cmplx           m_cScale ;
  bool            m_bcScaleCongugate ;
  double          m_dScaleTenthPerUnit ;
  //SpyWnd *        m_pSelectorWnd ;
  long			      m_nPlaydecHandle;
  int             m_nIndex;
  int             m_iNAttachments ;
  int             m_iRenderId ;
  bool            m_bAddedToList ;
  bool            m_bShouldBeAddedToList ;
  //static HANDLE   m_CallBackLock ;
  //static FXArray<FRender*> m_AllRenderPtrs ;
  //static WNDPROC  m_MainWindowDefFunc ;
  //static DWORD    m_dwMessageId ;
  //static DWORD    m_dwProcessId ;
  //static HWND     m_hTopWnd ;
  //static bool     m_IPIndexes[MAX_IP_CAMERAS] ;
  bool			      m_bWasCreatedbyMe;
  FXString        m_GadgetInfo ;
  //IPCamInfo       m_IPCamBusInfo;
  //CSharedMemBoxes m_IPCamShared;
  

public:
  void        ShutDown();
  void        Attach(CWnd* pWnd);
  void        Detach();
  int         GetOutputsCount()                { return 2; }
  COutputConnector* GetOutputConnector(int n)   
  { return (!n)? m_pOutput: (n==1)? m_pImageOutput : NULL ; }
  bool        PrintProperties(FXString& text);
  bool        ScanProperties(LPCTSTR text, bool& Invalidate);
  bool        ScanSettings(FXString& text);
  int         DibEvent(int Event, void *Data);
  int         OnSpyMessage( int Event , WPARAM wParam ) ;
  int         GetDuplexCount() { return 1 ; };
  LPCTSTR GetGadgetInfo() { return (LPCTSTR)m_GadgetInfo ; }
  CDuplexConnector* GetDuplexConnector(int n) { return (n==0) ? m_pControl : NULL ; } ;
  int GetNextAvIndex(int index, int playhandle);
  void OpenStream();
  virtual void      AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);

  void SetHandle(HWND hWnd);
  CPoint      GetScrollPos() 
  {
    if ( m_wndOutput )
    {
      CPoint ScrollPos( m_wndOutput->GetScrollPos( SB_HORZ ) , 
        m_wndOutput->GetScrollPos( SB_VERT )) ;
      m_ViewPos = ScrollPos ;
    }
    return m_ViewPos ; 
  }
  void        SetScrollPos( CPoint ScrollPos ) 
  { 
    m_ViewPos = ScrollPos ; 
    if ( m_wndOutput ) 
    {
      m_wndOutput->SetScrollPos( SB_HORZ , m_ViewPos.x  ) ;
      m_wndOutput->SetScrollPos( SB_VERT , m_ViewPos.y  ) ;
    }
  }
private:
  FRender(void);
  void        Render(const CDataFrame* pDataFrame);
  CWnd*       GetRenderWnd() { return m_wndOutput; }
  void        GetDefaultWndSize (RECT& rc) { 
    rc.left=rc.top=0; rc.right=GADGET_VIDEO_WIDTH; rc.bottom=GADGET_VIDEO_HEIGHT; }
  static LRESULT CALLBACK FRenderWindowProc( HWND hwnd,  UINT uMsg, WPARAM wParam, LPARAM lParam ) ;
  static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam ) ;

  DECLARE_RUNTIME_GADGET(FRender);
public:
  // For DIB events processing
  bool FormOutputText( CPoint& OnImagePt , DWORD dwBitMask , FXString * pOutString = NULL );

  cmplx SetScaleAndUnits( FXString& AsText );
};
