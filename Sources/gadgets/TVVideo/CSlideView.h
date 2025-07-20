#pragma once

class CSlideView :
    public CDIBView
{
protected:
    bool    m_Rescale;
    bool    m_Shift;
    int     m_TargetWidth, m_TargetHeight;
    int     m_FramesLen;
    int     m_FramesInRow;
    int     m_Rows;
    int     m_Cntr;
    int     m_SelectedItem;
    int     m_OrgWidth, m_OrgHeight;
    bool    m_bResetData ;
public:
                    CSlideView(LPCTSTR name = "");
                   ~CSlideView(void);
    int             GetTargetWidth()        { return m_TargetWidth; }
    int             GetSlidesLen()          { return m_FramesLen; }
    int             GetFramesInRow()        { return m_FramesInRow; }
    void            SetResetData() { m_bResetData = true ; }
    virtual void    ResetData()
    {
      if ( m_bResetData )
      {
        CAutoLockMutex al( m_LockOMutex , 1000
        #ifdef _DEBUG
          , "CSlideView::ResetData"
        #endif
        ) ;
        OnBufferReset() ;
        m_bResetData = false ;
      }
    }
protected:
    bool            SetCicle(LPBITMAPINFOHEADER dst, LPBITMAPINFOHEADER src);
    bool            SetShift(LPBITMAPINFOHEADER dst, LPBITMAPINFOHEADER src);
    bool            CopyFrame(int from, int to, LPBYTE data);
    void            DipPPEvent(int Event, void *Data);
    bool            DrawExFunc(HDC hdc, RECT& rc);
    bool            PrepareData(const pTVFrame frame, DWORD dwTimeOut = INFINITE);
    virtual void    OnBufferReset();
    virtual void    OnItemSelected();
    friend void CSlideView_DipPPEvent(int Event, void *Data, void *pParam, CDIBViewBase* wParam);
    friend bool CSlideView_DrawExFunc(HDC hdc, RECT& rc, CDIBViewBase* view, LPVOID lParam);
};
