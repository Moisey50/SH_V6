#pragma once
#include <video\dibviewbase.h>
#include "math/Intf_sup.h"

class FDIBViewBase :
  public CDIBViewBase
{
public:
  FDIBViewBase(void);
  virtual ~FDIBViewBase(void);
  DECLARE_MESSAGE_MAP()
  afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
  void Pic2Scr( CPoint &point );
  void SubPix2Scr( CPoint &point );
  void Scr2Pic( CPoint &point );
  cmplx Scr2PicCmplx( CPoint& point ) ;

  int  Pic2Scr( double len );
  int  Scr2Pic( double len );

protected:
  virtual void* DoPrepareData(pTVFrame frame, LPVOID lpBuf=NULL);

};

