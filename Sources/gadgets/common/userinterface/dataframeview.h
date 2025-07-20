#if !defined(AFX_DATAFRAMEVIEW_H__)
#define AFX_DATAFRAMEVIEW_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxmt.h>
#include <afxtempl.h>
#include <gadgets\gadbase.h>
#include <gadgets\WaveFrame.h>

/////////////////////////////////////////////////////////////////////////////
// CDataFrameView window

class CDataFrameView : public CWnd
{
// Construction
private:   
    FXLockObject m_Busy;
	 datatype    m_Mode;

    LPWAVEHDR   m_pWaveData;
	 WORD        m_nChannels;
	 WORD        m_wBitsPerSample;

    CPen			 m_Pen;
    CPen			 m_RedPen;
    RECT			 m_RC;
    double		 m_scalex,m_scaley,m_min,m_max;
    CPoint		 m_MousePnt;
    bool			 m_ScaleAuto;
public:
    CDataFrameView();
	~CDataFrameView();
    void    SetDataFrame(CDataFrame *pDataFrame);
    void    SetMinMaxData(double min, double max) 
				{ 
					m_ScaleAuto=false;
					m_min=min;
					m_max=max;
				};
    void    Reset()           
				{ 
			      m_Busy.Lock();
					m_min=0; 
					m_max=255; 
					if (m_pWaveData)
					{
						free(m_pWaveData->lpData);
						free(m_pWaveData);
						m_pWaveData=NULL;
					}
			      m_Busy.Unlock();
				};
    int GetSize()         
				{ 
			 	   int size=0;
					if ((Tvdb400_TypesCompatible(m_Mode, wave))&&(m_pWaveData))
					{
						if (m_wBitsPerSample == 8)
							size=m_pWaveData->dwBufferLength;
						else if (m_wBitsPerSample == 16)
							size=m_pWaveData->dwBufferLength/2;
						else
							ASSERT(FALSE);
					}
					return size;
				};
    CPoint  GetMousePnt();
    int     GetAt(int pozition)      
				{ 
					if ((Tvdb400_TypesCompatible(m_Mode, wave))&&(m_pWaveData))
					{
						if (!GetSize()) 
							return 0; 
						if (m_wBitsPerSample == 8)
							return ((signed char*)m_pWaveData->lpData)[pozition];
						else if (m_wBitsPerSample == 16)
							return ((short*)m_pWaveData->lpData)[pozition];
						else
							ASSERT(FALSE);
					}
					return 0;
				};
    void    SetScaleAuto(bool val=true) 
				{
					m_ScaleAuto=val; 
				};
// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDataFrameView)
	public:
	virtual BOOL Create(CWnd* pParentWnd);
	//}}AFX_VIRTUAL

// Implementation
public:
	// Generated message map functions
protected:
	//{{AFX_MSG(CDataFrameView)
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DATAVIEW_H__CBD40C04_BC89_11D4_9462_000001360793__INCLUDED_)
