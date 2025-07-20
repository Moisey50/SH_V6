#pragma once
#include <ddraw.h>
#include <video\dibview.h>

class FX_EXT_SHVIDEO  CDXView :
    public CDIBView
{
protected:
	LPDIRECTDRAW			m_DD;         // As is..
	LPDIRECTDRAWSURFACE     m_SurfPri;	  // As is..
	LPDIRECTDRAWSURFACE     m_SurfSec;	  // As is..
    LPDIRECTDRAWCLIPPER     m_Clipper;      // clipper for primary
	HBITMAP					m_hBM;
    HBRUSH                  m_BkBrush;
    DEVMODE                 m_DevMode;
    LPBITMAPINFOHEADER      m_BMIH;
    int                     m_ViewWidth, m_ViewHeight;
    FXLockObject            m_LockSurf;
private:
    BOOL UpdateFrame();
    BOOL ReInit();
    BOOL RestoreAll();
    BOOL UpdateBitmap(int w, int h);
    LPBITMAPINFOHEADER GetDrawData() { return m_BMIH; }
    virtual BOOL DrawOverBitmap();
public:
    CDXView(LPCTSTR name=_T(""));
    ~CDXView(void);
    virtual BOOL Create(CWnd* pParentWnd, DWORD dwAddStyle=0,UINT nID=0);
    virtual bool LoadFrame(const pTVFrame frame, bool bForceInvalidate = true);
    DECLARE_MESSAGE_MAP()
    afx_msg void OnDestroy();
    afx_msg void OnPaint();
    afx_msg void OnSize(UINT nType, int cx, int cy);
};
