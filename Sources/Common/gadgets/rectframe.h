// RectFrame.h: interface for the CRectFrame class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RECTFRAME_H__8FE7AEDD_CF35_4E2C_BD67_B2A4010130FF__INCLUDED_)
#define AFX_RECTFRAME_H__8FE7AEDD_CF35_4E2C_BD67_B2A4010130FF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include <gadgets\gadbase.h>

class FX_EXT_GADGET CRectFrame : public CDataFrame, public RECT
{
protected:
	CRectFrame(LPCRECT Rect);
	CRectFrame(const CRectFrame* RectFrame);
	virtual ~CRectFrame();
public:
	operator LPRECT();
  CDataFrame* Copy() const { return new CRectFrame(this); }
	CRectFrame* GetRectFrame(LPCTSTR label = DEFAULT_LABEL);
  const CRectFrame* GetRectFrame(LPCTSTR label = DEFAULT_LABEL) const;
	static CRectFrame* Create(LPRECT Rect = NULL);
	BOOL IsNullFrame() const;
	BOOL Serialize(LPBYTE* ppData, FXSIZE* cbData) const;
  BOOL Serialize( LPBYTE pBufferOrigin , FXSIZE& CurrentWriteIndex , FXSIZE BufLen ) const ;
  virtual FXSIZE GetSerializeLength( FXSIZE& uiLabelLen ,
    FXSIZE * pAttribLen = NULL ) const ;
  BOOL Restore( LPBYTE lpData , FXSIZE cbData );
  virtual void ToLogString(FXString& Output);
};

FX_EXT_GADGET CRectFrame * CreateRectFrameEx( CRect& Rect ,
  DWORD dwColor = 0xc0ffc0 , int iThickness = 0 ); // 0 means "do default"

FX_EXT_GADGET CRectFrame * CreateRectFrameEx( CRect& Rect ,
  const char * pAttributes ) ;

#endif // !defined(AFX_RECTFRAME_H__8FE7AEDD_CF35_4E2C_BD67_B2A4010130FF__INCLUDED_)
