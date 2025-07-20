// OCRGadget.cpp: implementation of the OCR class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TVRecognition.h"
#include "OCRGadget.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define THIS_MODULENAME "OCRGadget.cpp"

#define PASSTHROUGH_NULLFRAME(vfr, fr)			\
{												\
	if (!(vfr) || ((vfr)->IsNullFrame()))		\
    {	                                        \
		return NULL;			                \
    }                                           \
}

IMPLEMENT_RUNTIME_GADGET_EX(OCR, CFilterGadget, "Video.recognition", TVDB400_PLUGIN_NAME);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

OCR::OCR():
    m_LibFileName("")
{
	m_pInput   =    new CInputConnector(vframe);
	m_pOutput  =    new COutputConnector(text);
    m_FormatErrorProcessed=false;
    m_LastFormat=0;
	Resume();
}

void OCR::ShutDown()
{
    CFilterGadget::ShutDown();
	delete m_pInput;
	m_pInput = NULL;
	delete m_pOutput;
	m_pOutput = NULL;
}

CDataFrame* OCR::DoProcessing(const CDataFrame* pDataFrame)
{
	const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame(DEFAULT_LABEL);
    PASSTHROUGH_NULLFRAME(VideoFrame, pDataFrame);
    if (m_LastFormat!=VideoFrame->lpBMIH->biCompression)
    {
        m_LastFormat=VideoFrame->lpBMIH->biCompression;
        m_FormatErrorProcessed=false;
    }
    if ((m_LastFormat!=BI_YUV9) && (m_LastFormat!=BI_Y8)) 
    {
        if (!m_FormatErrorProcessed)
            SENDERR_0("OCR can only accept formats YUV9 and Y8");
        m_FormatErrorProcessed=true;
        return NULL;
    }
    FXString retS;
    int validity;
    if (Recognize(VideoFrame,retS,validity))
    {
        if (retS.GetLength()==0) return NULL;
        
        CTextFrame* retVal=CTextFrame::Create(retS);
            retVal->CopyAttributes(pDataFrame);;
        return retVal;
    }
    return NULL;
}

bool OCR::ScanSettings(FXString& text)
{
    text.Format("template(EditBox(LibFileName))",TRUE,FALSE);
    return true;
}

bool OCR::PrintProperties(FXString& text)
{
    CFilterGadget::PrintProperties(text);
    FXPropertyKit pc;
    pc.WriteString("LibFileName",m_LibFileName);
    text+=pc;
    return true;
}

bool OCR::ScanProperties(LPCTSTR text, bool& Invalidate)
{
    CString tmpS;
    CFilterGadget::ScanProperties(text,Invalidate);
    FXPropertyKit pc(text);
    pc.GetString("LibFileName",m_LibFileName);
    if (!LoadOcrLib(m_LibFileName))
    {
        SENDERR_1("Can't load OCR library file \"%s\"",m_LibFileName);
        m_LibFileName.Empty();
    }
    return true;
}

