// Pearson.cpp : Implementation of the Pearson class


#include "StdAfx.h"
#include "Pearson.h"
#include <Gadgets\VideoFrame.h>
#include <Gadgets\vftempl.h>
#include <math.h>

#define THIS_MODULENAME "BMPCapture"

__forceinline double _pearson(PearsonData& x, PearsonData& y)
{
    double retV=0;
    double cov=0;
    double sgm1=0;
    double sgm2=0;
    if (x.m_datasize==y.m_datasize)
    {
        for (int i=0; i<x.m_datasize; i++)
        {
            cov+=(x.m_data[i]-x.m_mean)*(y.m_data[i]-y.m_mean);
        }
        retV=cov/sqrt(x.m_sgm*y.m_sgm);
    }
    ASSERT(retV<1.1);
    ASSERT(retV>-1.1);
    return retV;
}

///////////////////////////////////////////////////////////////////////////////
void    PearsonData::Copy(LPBYTE data, int size)
{
    if (size!=m_datasize)
    {
        if (m_data) 
            m_data=(LPBYTE)realloc(m_data,size);
        else
            m_data=(LPBYTE)malloc(size);
        m_datasize=size;
    }
    memcpy(m_data,data,m_datasize);
    m_mean=0;
    for (int i=0; i<m_datasize; i++)
        m_mean+=m_data[i];
    m_mean/=m_datasize;
    m_sgm=0;
    for (int i=0; i<m_datasize; i++)
        m_sgm+=pow((m_data[i]-m_mean),2);
}

IMPLEMENT_RUNTIME_GADGET_EX(Pearson, CFilterGadget, LINEAGE_FILEX , TVDB400_PLUGIN_NAME);

Pearson::Pearson(void):
     m_FrameCntr(0)
    ,m_Size(242)
    ,m_pData(NULL)
{
    m_OutputMode=modeReplace;
    m_pInput = new CInputConnector(vframe);
    m_pOutput = new COutputConnector(vframe);
    m_pDuplexConnector = new CDuplexConnector( this , transparent, transparent);
    Resume();
}

void Pearson::ShutDown()
{
    CFilterGadget::ShutDown();
	delete m_pInput;
	m_pInput = NULL;
	delete m_pOutput;
	m_pOutput = NULL;
	delete m_pDuplexConnector ;
	m_pDuplexConnector = NULL ;
    FreeData();
}

void Pearson::FreeData()
{
    if (m_pData)
    {
        for (int i=0; i<m_Size; i++)
        {
            if (m_pData[i].m_data) free(m_pData[i].m_data); m_pData[i].m_data=NULL;
        }
        free(m_pData); m_pData=NULL;
    }
}

CDataFrame* Pearson::DoProcessing(const CDataFrame* pDataFrame)
{
    const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame(DEFAULT_LABEL);
    PASSTHROUGH_NULLFRAME(VideoFrame, pDataFrame);
    if ((VideoFrame) && ((VideoFrame->lpBMIH->biCompression==BI_YUV9) 
      || (VideoFrame->lpBMIH->biCompression==BI_Y8) || (VideoFrame->lpBMIH->biCompression==BI_Y800)))
    {
        m_pData[m_FrameCntr].Copy(GetData(VideoFrame),VideoFrame->lpBMIH->biWidth*VideoFrame->lpBMIH->biHeight);
        m_FrameCntr++;
    }
    if (m_Size==m_FrameCntr) // filled
    {
        CVideoFrame* retFrame=CVideoFrame::Create(CalcPearson());
        m_FrameCntr=0;
        return retFrame;
    }
    return NULL;
}

pTVFrame Pearson::CalcPearson()
{
    int extra=0;
    if ((m_Size*3)%4!=0)
    {
        extra=4-((m_Size*3)%4);
    }
    int bmSize=(m_Size+extra)*m_Size*3;

    pTVFrame retV=(pTVFrame)malloc(sizeof(TVFrame));
    retV->lpBMIH=(LPBITMAPINFOHEADER)malloc(sizeof(BITMAPINFOHEADER)+bmSize);
    memset(retV->lpBMIH,0,sizeof(BITMAPINFOHEADER)+bmSize);
    retV->lpData=NULL;
    retV->lpBMIH->biSize=sizeof(BITMAPINFOHEADER);
    retV->lpBMIH->biWidth  = m_Size;
    retV->lpBMIH->biHeight = m_Size;
    retV->lpBMIH->biPlanes = 1;
    retV->lpBMIH->biBitCount = 24;
    retV->lpBMIH->biCompression=BI_RGB;
    retV->lpBMIH->biSizeImage = 0;
    retV->lpBMIH->biXPelsPerMeter = 0;
    retV->lpBMIH->biYPelsPerMeter = 0;
    retV->lpBMIH->biClrUsed = 0;
    retV->lpBMIH->biClrImportant = 0;
    //LPBYTE ptr=GetData(retV);
    for (int y=0; y<retV->lpBMIH->biHeight; y++)
    {
        LPBYTE ptr=GetData(retV)+(retV->lpBMIH->biHeight-y-1)*(retV->lpBMIH->biWidth*3+extra);
        //LPBYTE ptr=GetData(retV)+y*(retV->lpBMIH->biWidth*3+extra);
        for (int x=0; x<retV->lpBMIH->biWidth; x++)
        {
            if (x>y) break;
            //BYTE b=(x%10==0)?255:(y%10==0)?255:0;
            BYTE b=(BYTE)(127*_pearson(m_pData[x],m_pData[y])+127);
            *ptr=b; ptr++;
            *ptr=b; ptr++;
            *ptr=b; ptr++;
            LPBYTE ptr2=GetData(retV)+(retV->lpBMIH->biHeight-x-1)*(retV->lpBMIH->biWidth*3+extra)+y*3;
            *ptr2=b; ptr2++;
            *ptr2=b; ptr2++;
            *ptr2=b;

        }
        //ptr+=extra;
        SENDINFO("Row %d is calculated",y);
    } 
    return retV;
}

bool Pearson::ScanProperties(LPCTSTR text, bool& Invalidate)
{
    FXPropertyKit pk(text);
    FreeData();
    pk.GetInt( "Size" , m_Size ) ;
    m_pData=(PearsonData*)malloc(m_Size*sizeof(PearsonData));
    memset(m_pData,0,m_Size*sizeof(PearsonData));
    return true;
}

bool Pearson::PrintProperties(FXString& text)
{
     FXPropertyKit pk;
     CFilterGadget::PrintProperties(text);
     pk.WriteInt( "Size" , m_Size ) ;
     text+=pk;
     return true;
}

bool Pearson::ScanSettings(FXString& text)
{
     text = "template(Spin(Size,1,1000))";
     return true;
}


int Pearson::GetDuplexCount()
{
    return (m_pOutput)?1:0;
}

CDuplexConnector* Pearson::GetDuplexConnector(int n)
{
    return ((!n)?m_pDuplexConnector:NULL);
}

void Pearson::AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame)
{
  // Do implement reaction on data frame receiving by duplex connector
    pParamFrame->Release();
}
