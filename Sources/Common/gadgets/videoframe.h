// VideoFrame.h: interface for the CVideoFrame class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VIDEOFRAME_H__5DEE9078_90D9_445E_9822_36C0E52B034B__INCLUDED_)
#define AFX_VIDEOFRAME_H__5DEE9078_90D9_445E_9822_36C0E52B034B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <gadgets\gadbase.h>
#include <video\TVFrame.h>

class FX_EXT_GADGET CVideoFrame : public CDataFrame, public TVFrame
{
protected:
	CVideoFrame(const pTVFrame Frame);
  CVideoFrame(const CVideoFrame* VideoFrame);
	virtual ~CVideoFrame();
public:
    CDataFrame* Copy() const { return new CVideoFrame(this); }
	virtual CVideoFrame* GetVideoFrame(LPCTSTR label = DEFAULT_LABEL);
    virtual const CVideoFrame* GetVideoFrame(LPCTSTR label = DEFAULT_LABEL) const;
	static CVideoFrame* Create(pTVFrame Frame = NULL);
	BOOL IsNullFrame() const;
  virtual BOOL Serialize( LPBYTE* ppData , FXSIZE* cbData ) const;
  virtual BOOL Serialize( LPBYTE pBufferOrigin , FXSIZE& CurrentWriteIndex , FXSIZE BufLen ) const;
  virtual FXSIZE GetSerializeLength( FXSIZE& uiLabelLen ,
    FXSIZE * pAttribLen = NULL ) const ;
    virtual BOOL Restore( LPBYTE lpData , FXSIZE cbData );
  virtual void ToLogString(FXString& Output);
};

#endif // !defined(AFX_VIDEOFRAME_H__5DEE9078_90D9_445E_9822_36C0E52B034B__INCLUDED_)
