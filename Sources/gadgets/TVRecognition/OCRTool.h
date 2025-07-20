// OCRTool.h: interface for the COCRTool class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OCRTOOL_H__8ED3F9E7_C07C_4DB2_9F61_09EC82F79DD5__INCLUDED_)
#define AFX_OCRTOOL_H__8ED3F9E7_C07C_4DB2_9F61_09EC82F79DD5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <video\TVFrame.h>
#include "OCRInscribing.h"
#include <fxfc\fxfc.h>

class COCRTool  
{
    COCRInscribing m_Inscriber;
public:
	        COCRTool();
	virtual ~COCRTool();
	bool    Recognize(const pTVFrame frame, FXString& Result, int& validity);
    bool    LoadOcrLib(LPCTSTR f_Name);
};

#endif // !defined(AFX_OCRTOOL_H__8ED3F9E7_C07C_4DB2_9F61_09EC82F79DD5__INCLUDED_)
