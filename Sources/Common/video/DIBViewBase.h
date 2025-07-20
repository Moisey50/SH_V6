#if !defined(AFX_DIBVIEWBASE_H__1937B095_D574_4FFE_97AD_4D237A6ECB07__INCLUDED_)
#define AFX_DIBVIEWBASE_H__1937B095_D574_4FFE_97AD_4D237A6ECB07__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DIBViewBase.h : header file
//
#include <fxfc\fxfc.h>
#include <video\TVFrame.h>
#include <helpers\AutoLockMutex.h>

class CDIBViewBase;

#define SEL_NOTHING 0
#define SEL_LINE    1
#define SEL_RECT    2

#define DIBVE_NOTHINGEVENT      0
#define DIBVE_MOUSEMOVEEVENT    1
#define DIBVE_LINESELECTEVENT   2
#define DIBVE_RECTSELECTEVENT   4
#define DIBVE_ACTIVWNDCHEVENT   8
#define DIBVE_LBUTTONDOWN      16
#define DIBVE_LBUTTONUP        32
#define DIBVE_RBUTTONDOWN      64
#define DIBVE_RBUTTONUP       128
#define DIBVE_MBUTTONDOWN     256
#define DIBVE_MBUTTONUP       512
#define DIBVE_LBUTTONDBL     1024
#define DIBVE_DO_MAXIMIZE    2048
#define DIBVE_OUT_FRAME 0x8000000
#define DIBVE_OUT_VFRAME 0x4000000

#define DIBVE_MOUSEMOVE (DIBVE_MOUSEMOVEEVENT | DIBVE_RBUTTONDOWN)

typedef struct tagSelection
{
  CPoint p1,p2;
  CRect  Object;  
  CPen*  Pen;
  bool   LineEnable, RectEnable;
  bool   LPressed,RPressed;
  bool   DrawLine,DrawRect;

}Selection,*pSelection;

typedef void DipPPEvent(int Event, void *Data, void *pParam, CDIBViewBase* wParam);
typedef bool (*DrawExFunc)(HDC hdc,RECT& rc,CDIBViewBase* view, LPVOID lParam);

/////////////////////////////////////////////////////////////////////////////
// CDIBViewBase window
class FX_EXT_SHVIDEO CDIBViewBase : public CWnd
{
public:
  CObservedMutex m_LockOMutex;
protected:
  BITMAPINFOHEADER * m_pConvertedBMIH;         // This type free pointer, usualy I hold in it decompressed image
  CString     m_Name;
  int         m_Monochrome;
protected:
  double      m_Scale;
  double      m_ScrScale;
  CPoint      m_ScrOffset;
  CPoint      m_IntBmOffset;  //Offset of the object inside of bitmap
  pSelection  m_Selection;
  pTVFrame    m_Frame;
  DipPPEvent *m_CallBack;
  void*	      m_CallBackParam;
  DrawExFunc  m_DrawEx;
  LPVOID      m_DrawExParam;
  int         m_Width, m_Height;
  bool        m_SizeChanged;
  bool        m_DataMonochrome;
public:
  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CDIBViewBase)
public:
  virtual BOOL Create(CWnd* pParentWnd, DWORD dwAddStyle=0,
    UINT nID=0, LPCTSTR szWindowName = _T("DIBView"));
  //}}AFX_VIRTUAL
public:
  CDIBViewBase(LPCTSTR name=_T(""));
  virtual   ~CDIBViewBase();
  void      Reset();
  void      SetMonochrome( int bw = 1 ) { m_Monochrome = bw ; }
  int       IsMonochrome()              { return m_Monochrome ; };
  virtual   bool Draw(HDC hdc,RECT& rc);
  virtual 	bool LoadFrame(const pTVFrame frame, /*const void * pExtData = NULL , */bool bForceInvalidate = true);
  virtual 	bool LoadDIB(BITMAPINFOHEADER* bmih, bool bForceInvalidate = true);
  virtual   void LoadFrameWait(pTVFrame frame /*, const void * pExtData = NULL*/ );
  virtual   void ResetData() { return; }
  void SetActive();
  void InvalidateRect(CRect rc, BOOL bErase = TRUE);
  // Image-screen coordinates conversions
  bool InFrame(CPoint pnt);
  void Pic2Scr(CPoint &point);
  void SubPix2Scr(CPoint &point);
  void Scr2Pic(CPoint &point);
  int  Pic2Scr(double len);
  int  Scr2Pic(double len);
  // Selection
  void InitSelBlock(bool SelLine, bool SelRect);
  void FreeSelBlock();
  void GetSelRect(RECT& rc);
  void GetSelLine(RECT& rc);
  int  GetSelStyle();
  bool IsSelected();
  void SelectRectangle(RECT rc);
  void SelectLine(RECT rc);
  void SelectAll();
  void ResetSelection();
  // clipboard suuport
  HGLOBAL CopyHandle(); // return global pointer to copy of selected bitmap
  // inlines
  pTVFrame GetFramePntr() { return m_Frame; }
  bool     GetImageSize(LONG &width, LONG &height) 
  {
    if ((!m_Frame) || (!m_Frame->lpBMIH)) { width=0; height=0; return false;}
    width=m_Width; height=m_Height;
    return true;
  }
  double   GetScale() { return m_Scale;}
  void	 SetCallback(DipPPEvent* callback, void *pParam) 
  {
    m_CallBackParam=pParam; 
    m_CallBack=callback;
  }
  void     SetDrawExFunc(DrawExFunc DrawEx, LPVOID lParam)
  { 
    m_DrawEx=DrawEx; 
    m_DrawExParam=lParam; 
  }
  DrawExFunc GetDrawExFunc()  {return m_DrawEx;}
  LPVOID     GetDrawExParam() {return m_DrawExParam;}
  virtual LPBITMAPINFOHEADER GetDrawData() { return m_pConvertedBMIH; }
  /// This function is not implemented in base class
  virtual     void    SetScale(double s)   { }
  virtual     bool    DoScrolling(int& x, int& y, RECT& rc) { return false; }
#ifdef _DEBUG
  void Invalidate(BOOL bErase = TRUE);
#endif
protected:
  virtual bool PrepareData(const pTVFrame frame, /*const void * pExtData = NULL ,*/ DWORD dwTimeOut = INFINITE);
  virtual BITMAPINFOHEADER * PrepareImage(pTVFrame frame, LPVOID lpBuf=NULL);
  virtual void SetExtData( const void * pDataFrameWithGraphics ) {}
  //{{AFX_MSG(CDIBViewBase)
  afx_msg void OnPaint();
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
  afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
  afx_msg void OnMouseMove(UINT nFlags, CPoint point);
  afx_msg BOOL OnNcActivate(BOOL bActive);
  afx_msg void OnDestroy();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIBVIEWBASE_H__1937B095_D574_4FFE_97AD_4D237A6ECB07__INCLUDED_)
