// Pseudocolor2.cpp : Implementation of the Pseudocolor2 class


#include "StdAfx.h"
#include "Pseudocolor.h"
#include "gadgets\VideoFrame.h"
#include "palettes.h"

IMPLEMENT_RUNTIME_GADGET_EX(Pseudocolor2, CFilterGadget, "Video.color&brightness", TVDB400_PLUGIN_NAME);

Pseudocolor2::Pseudocolor2(void):
    m_Palette(0)
{
    m_MultyCoreAllowed=true;
    m_OutputMode=modeReplace;
    m_pInput = new CInputConnector(vframe);
    m_pOutput = new COutputConnector(vframe);
    Resume();
}

void Pseudocolor2::ShutDown()
{
  //TODO: Add all destruction code here
    CFilterGadget::ShutDown();
	delete m_pInput;
	m_pInput = NULL;
	delete m_pOutput;
	m_pOutput = NULL;
}

CDataFrame* Pseudocolor2::DoProcessing(const CDataFrame* pDataFrame)
{
    const CVideoFrame* vf=pDataFrame->GetVideoFrame(DEFAULT_LABEL);
    if (vf)
    {
        int extra=0;
        if ((vf->lpBMIH->biWidth*3)%4!=0)
        {
            extra=4-((vf->lpBMIH->biWidth*3)%4);
        }
        int bmSize=(vf->lpBMIH->biWidth+extra)*vf->lpBMIH->biHeight*3;

        pTVFrame retV=(pTVFrame)malloc(sizeof(TVFrame));
        retV->lpBMIH=(LPBITMAPINFOHEADER)malloc(sizeof(BITMAPINFOHEADER)+bmSize);
        memset(retV->lpBMIH,0,sizeof(BITMAPINFOHEADER)+bmSize);
        retV->lpData=NULL;
        retV->lpBMIH->biSize=sizeof(BITMAPINFOHEADER);
        retV->lpBMIH->biWidth  = vf->lpBMIH->biWidth;
        retV->lpBMIH->biHeight = vf->lpBMIH->biHeight;
        retV->lpBMIH->biPlanes = 1;
        retV->lpBMIH->biBitCount = 24;
        retV->lpBMIH->biCompression=BI_RGB;
        retV->lpBMIH->biSizeImage = 0;
        retV->lpBMIH->biXPelsPerMeter = 0;
        retV->lpBMIH->biYPelsPerMeter = 0;
        retV->lpBMIH->biClrUsed = 0;
        retV->lpBMIH->biClrImportant = 0;
        LPBYTE dst0=GetData(retV);
        LPBYTE src=GetData(vf);
        LPWORD srcw = GetData16(vf);
        switch (vf->lpBMIH->biCompression)
        {
        case BI_Y8:
        case BI_YUV9:
            {
                for (int y=0; y<retV->lpBMIH->biHeight; y++)
                {
                    LPBYTE dst=dst0+(retV->lpBMIH->biHeight-y-1)*(vf->lpBMIH->biWidth*3+extra);
                    for (int x=0; x<retV->lpBMIH->biWidth; x++)
                    {
                        *dst=Palettes[m_Palette][*src].rgbRed; dst++;
                        *dst=Palettes[m_Palette][*src].rgbGreen; dst++;
                        *dst=Palettes[m_Palette][*src].rgbBlue; dst++;
                        src++;
                    }
                    dst+=extra;
                } 
                return CVideoFrame::Create(retV);
            }
        case BI_Y16:
            {
                for (int y=0; y<retV->lpBMIH->biHeight; y++)
                {
                    LPBYTE dst=dst0+(retV->lpBMIH->biHeight-y-1)*(vf->lpBMIH->biWidth*3+extra);
                    for (int x=0; x<retV->lpBMIH->biWidth; x++)
                    {
                        *dst=Palettes[m_Palette][(*srcw)/255].rgbRed; dst++;
                        *dst=Palettes[m_Palette][(*srcw)/255].rgbGreen; dst++;
                        *dst=Palettes[m_Palette][(*srcw)/255].rgbBlue; dst++;
                        srcw++;
                    }
                    dst+=extra;
                } 
                return CVideoFrame::Create(retV);
            }
        case BI_RGB:
            {
                switch (vf->lpBMIH->biBitCount)
                {
                case 24:
                    {
                        for (int y=0; y<retV->lpBMIH->biHeight; y++)
                        {
                            for (int x=0; x<retV->lpBMIH->biWidth; x++)
                            {
                                int col=(int)(0.299 * *src + 0.587 * *(src+1)+ 0.114 * *(src+2)); src+=3;
                                *dst0=Palettes[m_Palette][col].rgbRed; dst0++;
                                *dst0=Palettes[m_Palette][col].rgbGreen; dst0++;
                                *dst0=Palettes[m_Palette][col].rgbBlue; dst0++;
                            }
                            dst0+=extra;
                            src+=extra;
                        } 
                        return CVideoFrame::Create(retV);
                    }
                }
            }
        }
    }
    return NULL;
}

bool Pseudocolor2::ScanProperties(LPCTSTR text, bool& Invalidate)
{
    CFilterGadget::ScanProperties(text,Invalidate);
    FXPropertyKit pk(text);
    pk.GetInt( "Palette" , m_Palette) ;
    return true;
}

bool Pseudocolor2::PrintProperties(FXString& text)
{
     FXPropertyKit pk;
     CFilterGadget::PrintProperties(text);
     pk.WriteInt("Palette" , m_Palette) ;
     text+=pk;
     return true;
}

bool Pseudocolor2::ScanSettings(FXString& text)
{
    int pNmb=sizeof(PalettesNames)/sizeof(FXString);
    FXString param="template(ComboBox(Palette(";
    for (int i=0; i<pNmb; i++)
    {
        FXString opt;
        opt.Format("%s(%d),",PalettesNames[i],i);
        param+=opt;
    }
    param+=")))";
    text=param;
    return true;

    // Here should be commented samples for all types of  
    // dialog controls with short explanation


//     text = "template(EditBox(Sigma_Pix),"
//       "Spin(HalfWidth,3,40),"
//       "Spin(MinRadius_pix,10,50),"
//       "Spin(MaxRadius_pix,15,150),"
//       "Spin(SZone,5,50),"
//       "EditBox(Angle_Deg),"
//       "Spin(AngleStep,1,10),"
//       "EditBox(2ndThres),"
//       "EditBox(Scale_nm),"
//       "EditBox(MinSize_um),"
//       "EditBox(MaxSize_um)," 
//       "EditBox(SigSigma2),"
//       "Spin(ViewMode,0,16))";
//     return true;
}


