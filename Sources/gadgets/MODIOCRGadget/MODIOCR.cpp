// MODIOCR.h : Implementation of the MODIOCR class


#include "StdAfx.h"
#include "MODIOCR.h"
#include <files\imgfiles.h>
#pragma message( "Compiling " __FILE__ )
#include <gadgets\vftempl.h>
#include <gadgets\TextFrame.h>
#include <gadgets\VideoFrame.h>
#include <video\shvideo.h>

#import "MDIVWCTL.dll"

#define THIS_MODULENAME "MODIOCR"

IMPLEMENT_RUNTIME_GADGET_EX(MODIOCR, CFilterGadget, LINEAGE_VIDEO".recognition", TVDB400_PLUGIN_NAME);

MODIOCR::MODIOCR(void):
            m_OverwriteDPI(TRUE),
            m_X_DPI(300), m_Y_DPI(300)
{
    m_pInput = new CInputConnector(vframe);
    m_pOutput = new COutputConnector(transparent);
    Resume();
}

void MODIOCR::ShutDown()
{
    //TODO: Add all destruction code here
    CFilterGadget::ShutDown();
	delete m_pInput;
	m_pInput = NULL;
	delete m_pOutput;
	m_pOutput = NULL;
}

CDataFrame* MODIOCR::DoProcessing(const CDataFrame* pDataFrame)
{
    CTextFrame* retVal=NULL;
	const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame(DEFAULT_LABEL);
	PASSTHROUGH_NULLFRAME(VideoFrame, pDataFrame);
    FXString fName;
    FxGetTempFileName("MODIOCR","bmp",fName);
    LPBITMAPINFOHEADER lpBMIH=_decompress2rgb(VideoFrame);
    if (m_OverwriteDPI)
    {
        lpBMIH->biXPelsPerMeter=(int)(m_X_DPI*39.37+0.5);
        lpBMIH->biYPelsPerMeter=(int)(m_Y_DPI*39.37+0.5);
    }
    if (lpBMIH)
    {
	    if (!saveDIB(fName, lpBMIH))
            SENDERR_1("Can't write file \"%s\"",fName);
        free(lpBMIH);
    }
    else
        SENDERR_0("Can't decompress image");
    bstr_t wfName(fName);
    bstr_t ocrres;
    bool res=true;
    CString errmes;
    try
    {
        errmes="MODI is not installed";
        MODI::IDocumentPtr doc(__uuidof(MODI::Document));
        errmes="Can't read source file";
        doc->Create(wfName);
        errmes="OCR Error";
        doc->OCR(MODI::miLANG_ENGLISH, VARIANT_TRUE, VARIANT_TRUE);
        MODI::IImagePtr img = doc->Images->Item[0];
        MODI::ILayoutPtr layout = img->Layout;
        doc->Close(VARIANT_FALSE);
        ocrres=layout->Text;
        errmes="";
    }
    catch(_com_error& ex)
    {
            ocrres=ex.Description();
            res=false;
    }
    DeleteFile(fName);
    CString tmpS;
    tmpS=((char*)ocrres);
    if (res)
    {
        retVal=CTextFrame::Create(tmpS);
            retVal->CopyAttributes(pDataFrame);;
    }
    else
    {
        if (tmpS.GetLength()==0)
            tmpS=errmes;
        SENDERR_1("OCR Error: %s",tmpS);
    }
	return retVal;
}

bool MODIOCR::ScanProperties(LPCTSTR text, bool& Invalidate)
{
    CFilterGadget::ScanProperties(text, Invalidate);
    FXPropertyKit pk(text);
    BOOL OverwriteDPIold=m_OverwriteDPI;
    pk.GetInt("OverwriteDPI",m_OverwriteDPI);
    Invalidate=(m_OverwriteDPI!=OverwriteDPIold);
    if (m_OverwriteDPI)
    {
        pk.GetInt("X_DPI",m_X_DPI);
        pk.GetInt("Y_DPI",m_Y_DPI);
    }
	return true;
}

bool MODIOCR::PrintProperties(FXString& text)
{
    CFilterGadget::PrintProperties(text);
    FXPropertyKit pk;

    pk.WriteInt("OverwriteDPI",m_OverwriteDPI);
    if (m_OverwriteDPI)
    {
        pk.WriteInt("X_DPI",m_X_DPI);
        pk.WriteInt("Y_DPI",m_Y_DPI);
    }
    text+=pk;
    return true;
}

bool MODIOCR::ScanSettings(FXString& text)
{
    if (m_OverwriteDPI)
        text.Format("template(ComboBox(OverwriteDPI(True(%d),False(%d))),Spin(X_DPI,50,1000),Spin(Y_DPI,50,1000))",TRUE,FALSE);
    else
        text.Format("template(ComboBox(OverwriteDPI(True(%d),False(%d))))",TRUE,FALSE);    
    return true;
}

