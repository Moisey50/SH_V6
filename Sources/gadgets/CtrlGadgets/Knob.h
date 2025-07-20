// Knob.h: interface for the Knob class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_KNOB_H__A5BDE736_C482_4BEC_8715_78D77654E893__INCLUDED_)
#define AFX_KNOB_H__A5BDE736_C482_4BEC_8715_78D77654E893__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <gadgets\gadbase.h>
#include <helpers\PrxyWnd.h>

enum KnobOutType
{
  KOT_Text = 0 ,
  KOT_Int  ,
  KOT_Double ,
  KOT_Bool
};
class Knob : public CCtrlGadget
{
protected:
  CPrxyWnd          m_Proxy;
  CButton           m_Button;
  FXString          m_ButtonName;
  KnobOutType       m_Type ;
  BOOL              m_bVal ;
  FXString          m_Value ;
  Knob();
public:
  void    ShutDown();
  void    Attach( CWnd* pWnd );
  void    Detach();
  CWnd*   GetRenderWnd() { return &m_Proxy; }
  void    GetDefaultWndSize( RECT& rc ) { rc.left = rc.top = 0; 
  rc.right = 4 * DEFAULT_GADGET_WIDTH / 3; 
  rc.bottom = 22 /*DEFAULT_GADGET_HEIGHT / 2*/; }

  void    OnStart();
  void    OnStop();

  bool    ScanSettings( FXString& text );
  bool    ScanProperties( LPCTSTR text , bool& Invalidate );
  bool    PrintProperties( FXString& text );

  void    OnCommand( UINT nID , int nCode );

  DECLARE_RUNTIME_GADGET( Knob );
};

#endif // !defined(AFX_KNOB_H__A5BDE736_C482_4BEC_8715_78D77654E893__INCLUDED_)
