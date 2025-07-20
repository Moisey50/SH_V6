#pragma once

// #include <gadgets/BaseGadgets.h>
#include <Gadgets\gadbase.h>
class CGraphCameraControl
{
public:
  CGraphCameraControl(void)  ;
  ~CGraphCameraControl(void);

  DWORD m_dwSerialNumber ;
  CGadget * m_pGadget ;
};
