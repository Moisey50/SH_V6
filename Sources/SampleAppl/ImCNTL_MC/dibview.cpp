// dibview.cpp : implementation file
//

#include "stdafx.h"
#include "IMCNTL.h"
#include "dibview.h"
#include "conio.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//#define  TOOLBAR_SIZE    24 
//#define  STATUSBAR_SIZE  42
//#define  Y_SCAN_OFFSET   16

                          
void CDibView::ReSize(int x, int y, int bits)
{
    long ImageSize;

    ImageSize = (long)x * bits;
    ImageSize = (ImageSize + 31) / 32 ;       // align to DWORD 
    m_nStride = ImageSize * 4;
    ImageSize = m_nStride * y ;
    m_lpBMPinfo->biWidth = x;
    m_lpBMPinfo->biHeight = y;
    m_lpBMPinfo->biBitCount = bits;
    m_lpBMPinfo->biSizeImage = ImageSize;
}

CDibView::CDibView(int x, int y, int bits, WORD *buffer, int imode) :
    m_lpImageBuffer(NULL),
    m_lpBMPinfo(NULL),
    m_nColorTableSize(0),
    m_hPalette(NULL)
{
  // int x -- width
  // int y -- height
  // int bits -- number of bits per pixel
  m_imode = imode;

  long ImageSize;
  RGBQUAD *pRgb;
  m_nColorTableSize = 256;
  m_bTo8bits = FALSE;
	if (bits == 10 || bits == 12 || bits == 14 || bits == 16)	
	{
		// for 10, 12, 14, 16 bits, show them as 8 bits
		m_nBits = bits;
		bits = 8;
		m_bTo8bits = TRUE;
	}

  ImageSize = (long)x * bits;
  ImageSize = (ImageSize + 31) / 32 ;       // align to DWORD 
  m_nStride = ImageSize * 4;
  ImageSize = m_nStride * y ;
  m_lpBuffer_disp = NULL;
	// for 10, 12, 14, 16 bits
	if (m_bTo8bits) 
	{
		m_nSize = x * y;
		m_lpBuffer_disp = (LPBYTE) new char[m_nSize];
		m_lpImageBuffer = m_lpBuffer_disp;
		m_lpBuffer_grab = buffer;
	}
	else
		m_lpImageBuffer = (LPBYTE)buffer;

  m_lpBMPinfo = (LPBITMAPINFOHEADER) 
      new char[sizeof(BITMAPINFOHEADER) + 
      sizeof(RGBQUAD) * m_nColorTableSize];

  m_lpBMPinfo->biSize = sizeof(BITMAPINFOHEADER);
  m_lpBMPinfo->biWidth = x;
  m_lpBMPinfo->biHeight = y;
  m_lpBMPinfo->biPlanes = 1;
  m_lpBMPinfo->biBitCount = bits;
  m_lpBMPinfo->biCompression = BI_RGB;
  m_lpBMPinfo->biSizeImage = ImageSize;
  m_lpBMPinfo->biXPelsPerMeter = 0;
  m_lpBMPinfo->biYPelsPerMeter = 0;
  m_lpBMPinfo->biClrUsed = m_nColorTableSize;        
  m_lpBMPinfo->biClrImportant = m_lpBMPinfo->biClrUsed ;

  pRgb = (RGBQUAD *)((LPSTR)m_lpBMPinfo + (WORD)m_lpBMPinfo->biSize);
  for  (int i=0; i < 256; i++)
  {
      pRgb[i].rgbRed      = i;
      pRgb[i].rgbGreen    = i;
      pRgb[i].rgbBlue     = i;
      pRgb[i].rgbReserved = 0;
  }
}

CDibView::~CDibView()
{
  if (m_lpImageBuffer != NULL)
  {
		m_lpImageBuffer = NULL;
		m_lpImageUpsideDown = NULL;
		if (m_lpBuffer_disp != NULL)
		{
			delete [] m_lpBuffer_disp;
			m_lpBuffer_disp = NULL;
		}
  }
  if (m_lpBMPinfo!= NULL)
  {
      delete [] m_lpBMPinfo;
      m_lpBMPinfo = NULL;
  }
}

void CDibView::ChangeDIBPalette()
{

}

void CDibView::ShowDIB(CDC *pDC)
{
  HPALETTE    hOldPal;

  if (m_lpImageBuffer == NULL || m_lpBMPinfo == NULL)
      return;

	// for 10 ~ 16 bits 
	if (m_bTo8bits)
	{
		BYTE *dd = m_lpImageBuffer;
		WORD *ss = (WORD*)m_lpBuffer_grab;
    int iShift = m_nBits - 8 ;
		for ( int i = 0; i < m_nSize; i ++)
			*(dd++) = (BYTE)(*(ss++) >> iShift);
	}
  HDC hDC = pDC->GetSafeHdc();
  if (m_hPalette != NULL)
  {
      ::SetSystemPaletteUse(hDC, SYSPAL_STATIC);
      hOldPal = ::SelectPalette(hDC, m_hPalette, FALSE);
      ::RealizePalette(hDC);
  }

#if 0       
    // now move image data line by line to make it upsidedown for
    // DIB display
    src = (LPBYTE) GetDIBdataBufferPtr();
    dest = m_lpImageUpsideDown;
    stride = GetDIBdataStride();
    height = GetDIBdataHeight();

    dest = dest + (height-1) * stride;

    for (int i=0; i<height; i++)
    {
        memcpy(dest, src, stride);
        src += stride;
        dest -= stride;
    }
#endif
    // try either of the following display method to see if one works
    // better on your system than the other. Maybe. Maybe not.
#if 1
	CWnd * DcWin = pDC->GetWindow();
	RECT Rc ;
	DcWin->GetClientRect(&Rc);
	int iWidth = Rc.right;
	int iHeight = Rc.bottom;
	if(iWidth > m_lpBMPinfo->biWidth)
			iWidth = m_lpBMPinfo->biWidth;
	if(iHeight > m_lpBMPinfo->biHeight)
			iHeight = m_lpBMPinfo->biHeight;
// 	if((double)iWidth/(double)iHeight != (double)m_lpBMPinfo->biWidth/(double)m_lpBMPinfo->biHeight)
// 	{
// 		if(iWidth > iHeight)
// 			iWidth = iHeight;
// 		else
// 		  iHeight = iWidth;																												
// 	}                                     
  int iImgHeight = m_lpBMPinfo->biHeight ;
  int iSourseY = 0 ;  
  if ((m_lpBMPinfo->biHeight + TOOLBAR_SIZE + STATUSBAR_SIZE) > Rc.bottom) 
  {
    iImgHeight = (Rc.bottom - STATUSBAR_SIZE - TOOLBAR_SIZE ) ;
	  iSourseY = m_lpBMPinfo->biHeight - (Rc.bottom - STATUSBAR_SIZE - TOOLBAR_SIZE);
  }
	//int iYScanOffset = (m_lpBMPinfo->biHeight > 512) ? Y_SCAN_OFFSET : 
	//    Y_SCAN_OFFSET / 2;

  if(m_imode == 1 )
    ::SetDIBitsToDevice(hDC, 
      0, TOOLBAR_SIZE  , iWidth/*m_lpBMPinfo->biWidth*/ , iImgHeight , // destination 
      0, iSourseY,/*iYScanOffset*/0, m_lpBMPinfo->biHeight ,      // source
      m_lpImageBuffer,
      (LPBITMAPINFO) m_lpBMPinfo,
      DIB_RGB_COLORS                 // DIB_PAL_COLORS
//        SRCCOPY
      );
  else
  	::StretchDIBits(hDC, 
		  0, TOOLBAR_SIZE, iWidth , iHeight - STATUSBAR_SIZE ,      // destination 
		  0, 0, m_lpBMPinfo->biWidth, m_lpBMPinfo->biHeight,      // source
      m_lpImageBuffer,
		  (LPBITMAPINFO) m_lpBMPinfo,
		  DIB_RGB_COLORS,                 // DIB_PAL_COLORS
			SRCCOPY
		  );
#else
    ::SetDIBitsToDevice(hDC, 
        0, 0, m_lpBMPinfo->biWidth, m_lpBMPinfo->biHeight,      // destination 
        0, 0, 0, 
        m_lpBMPinfo->biHeight,      // source
		m_lpImageBuffer,
        (LPBITMAPINFO) m_lpBMPinfo,
        DIB_PAL_COLORS
        );
#endif
  if (m_hPalette != NULL)
  {
      ::SelectPalette(hDC, hOldPal, FALSE);
      ::RealizePalette(hDC);
  }
}



void * CDibView::GetDIBdataBufferPtr()
{
    return m_lpImageBuffer;
}

long CDibView::GetDIBdataBufferSize()
{
    return m_lpBMPinfo->biSizeImage;
}

int CDibView::GetDIBdataWidth()
{
    return m_lpBMPinfo->biWidth;
}

int CDibView::GetDIBdataHeight()
{
    return m_lpBMPinfo->biHeight;
}

int CDibView::GetDIBdataStride()
{
    return m_nStride;
}


/////////////////////////////////////////////////////////////////////////////
// dibview message handlers










































