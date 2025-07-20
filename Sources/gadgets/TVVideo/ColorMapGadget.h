#pragma once

#include <video\tvframe.h>

//__forceinline bool _decompress_any(pTVFrame frame);
//__forceinline void _makeRGB15(LPBYTE pntr, DWORD& R, DWORD& G, DWORD& B);
//__forceinline void _makeRGB(LPBYTE pntr, DWORD& R, DWORD& G, DWORD& B);
//__forceinline void _makeRGB8(LPBYTE pntr, RGBQUAD * Palette, DWORD& R, DWORD& G, DWORD& B);

class ColorMap : CFilterGadget
{
public:
		virtual void ShutDown();
		int m_iOffsetPure;
		int m_iOffsetMixed;
		ColorMap(void);

		virtual bool PrintProperties(FXString& text);
    virtual bool ScanProperties(LPCTSTR text, bool& Invalidate);
    virtual bool ScanSettings(FXString& text);
	virtual CDataFrame* DoProcessing(const CDataFrame* pDataFrame);   
	DECLARE_RUNTIME_GADGET(ColorMap);
};

