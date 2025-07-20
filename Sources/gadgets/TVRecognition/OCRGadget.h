// OCRGadget.h: interface for the OCR class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OCRGADGET_H__FDD23573_63DB_4D69_B892_DACEAA67C73E__INCLUDED_)
#define AFX_OCRGADGET_H__FDD23573_63DB_4D69_B892_DACEAA67C73E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <Gadgets\gadbase.h>
#include <Gadgets\VideoFrame.h>
#include <Gadgets\TextFrame.h>
#include "OCRTool.h"

class OCR : public CFilterGadget, COCRTool
{
    bool         m_FormatErrorProcessed;
    DWORD        m_LastFormat;
    FXString     m_LibFileName;
public:
	OCR();
	virtual void ShutDown();
    virtual CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
    virtual bool ScanSettings(FXString& text);
    virtual bool PrintProperties(FXString& text);
    virtual bool ScanProperties(LPCTSTR text, bool& Invalidate);
    DECLARE_RUNTIME_GADGET(OCR);
};

#endif // !defined(AFX_OCRGADGET_H__FDD23573_63DB_4D69_B892_DACEAA67C73E__INCLUDED_)
