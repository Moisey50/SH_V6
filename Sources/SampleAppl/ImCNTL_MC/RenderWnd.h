#pragma once
#include "afxwin.h"
#include <habitat.h>
// #include <gadgets/BaseGadgets.h>
#include <Gadgets\gadbase.h>

class CRenderWnd :
  public CWnd
{
public:
  CRenderWnd(void);
  ~CRenderWnd(void);

  CRenderGadget * m_pRenderGadget ;
};
