#pragma once

#include <video\dxview.h>
#include <gadgets\MetafileFrame.h>

class FX_EXT_SHBASE CDXRender :
    public CDXView
{
    CRect	    m_Rect;
    FXLockObject m_Lock;
    CMFHelpers  m_Metafiles;
    virtual BOOL DrawOverBitmap();
    void    DrawMetafiles(HDC hdc);
public:
    CDXRender(LPCTSTR name="");
    ~CDXRender(void);
    void    Render(const CDataFrame* pDataFrame);
};
