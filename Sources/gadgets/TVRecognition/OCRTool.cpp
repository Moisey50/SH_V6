// OCRTool.cpp: implementation of the COCRTool class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TVRecognition.h"
#include "OCRTool.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#include <imageproc\ocr\crcgn.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COCRTool::COCRTool()
{
    OCR_Init(GetModuleHandle("TVRecognition.dll"),MAKEINTRESOURCE(IDR_OCRLIB),"OCRLIB");
}

COCRTool::~COCRTool()
{
    OCR_Free();
}

bool COCRTool::Recognize(const pTVFrame frame, FXString &Result, int& validity)
{
    if ((!frame) || (!frame->lpBMIH)) return false;
    m_Inscriber.Inscribe(frame);
    FXString tmpS;
    Pattern ptrn;
    for (int i=0; i<m_Inscriber.GetLinesNumber(); i++)
    {
        for (int j=0; j<m_Inscriber.GetObjectsNmb(i); j++)
        {
            OCR_RecognizeChar(m_Inscriber.GetCluster(i,j),ALPHA_ANY,ptrn);
            tmpS+=(TCHAR)ptrn.ch;
        }
        tmpS+="\r\n";
    }
    Result=tmpS;
    validity=100;   
    return true;
}

bool    COCRTool::LoadOcrLib(LPCTSTR f_Name)
{
    bool retV;
    OCR_Free();
    if ((f_Name==NULL) || (_tcslen(f_Name)==0))
        retV=(OCR_Init(GetModuleHandle("TVRecognition.dll"),MAKEINTRESOURCE(IDR_OCRLIB),"OCRLIB")!=FALSE);
    else
        retV=(OCR_Load(f_Name)!=FALSE);
    return retV;
}