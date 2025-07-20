 /********************************************************************
	created:	2005/11/09
	created:	9:11:2005   14:25
	filename: 	c:\msc_proj\newcomponents\imcntlmv_2\dibview.h
	file path:	c:\msc_proj\newcomponents\imcntlmv_2
	file base:	dibview
	file ext:	h
	author:	Michael Son	e
	
	purpose:	Creation and output	of image 
*********************************************************************/


#if !defined(AFX_DIBVIEW_H__5FC4CDB5_E518_4C00_BAA1_2C61B1DB9237__INCLUDED_)
#define AFX_DIBVIEW_H__5FC4CDB5_E518_4C00_BAA1_2C61B1DB9237__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// dibview.h : header file
//

#include "stdafx.h"

#define  TOOLBAR_SIZE     26
#define  STATUSBAR_SIZE   24
#define  Y_SCAN_OFFSET    16


class CDibView
{
protected:
    LPBYTE m_lpImageBuffer;
    LPBYTE m_lpImageUpsideDown;
    LPBYTE m_lpBuffer_disp;	//xd
	LPWORD m_lpBuffer_grab;	//XD
    LPBITMAPINFOHEADER m_lpBMPinfo;
		LPBITMAPINFO m_lpBMPinfoColor;

    int m_nStride;
    int m_nColorTableSize;       // for now
    HPALETTE m_hPalette ;        // for now

	// XD
	int m_nBits;
	int m_nSize;
	int m_imode;
	
public:
		BOOL m_bTo8bits;
    CDibView(int x, int y, int bits, WORD * buffer, int mode);
    ~CDibView();
	void ReSize(int x, int y, int bits);

    void ChangeDIBPalette();
    void ShowDIB(CDC *pDC);
    void *GetDIBdataBufferPtr();
    long GetDIBdataBufferSize();
    int GetDIBdataWidth();
    int GetDIBdataHeight();
    int GetDIBdataStride();

};


/*----------------------------- end of file -------------------------*/


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIBVIEW_H__5FC4CDB5_E518_4C00_BAA1_2C61B1DB9237__INCLUDED_)
