#include "StdAfx.h"
#include "NeedleIndicator.h"
#include "Locator.h"
#include "LocatorDialog.h"
#include <math\intf_sup.h>
#include <math\hbmath.h>
#include <gadgets\VideoFrame.h>
#include <gadgets\vftempl.h>

IMPLEMENT_RUNTIME_GADGET_EX(Locator, CGadget, "Video.conversion", TVDB400_PLUGIN_NAME);

#define THIS_MODULENAME "Locator"

Locator::Locator(void):
		m_pnt(205,458),
        m_Angle(ANGLE_180),
        m_Radius(-1)
{
	m_pInput  = new CInputConnector(vframe);
	m_pOutput = new COutputConnector(vframe);
    m_SetupObject = new LocatorDialog(this, NULL);
	Resume();
}

void Locator::ShutDown()
{
	CGadget::ShutDown();
	if(m_pInput) delete m_pInput;
	m_pInput = NULL;
	if(m_pOutput) delete m_pOutput;
	m_pOutput = NULL;
}

__forceinline pTVFrame _radar(pTVFrame src, CPoint pnt)
{
	pTVFrame retV=(pTVFrame)malloc(sizeof(TVFrame));
	int width=1000;
	//int height=(int)sqrt((double)src->lpBMIH->biWidth*src->lpBMIH->biWidth+src->lpBMIH->biHeight*src->lpBMIH->biHeight);
	int height=src->lpBMIH->biHeight;
	int dSize=width*height;

	retV->lpData=NULL;
	retV->lpBMIH=(LPBITMAPINFOHEADER)malloc(sizeof(BITMAPINFOHEADER)+dSize);
	memset(retV->lpBMIH,0,sizeof(BITMAPINFOHEADER)+dSize);
	retV->lpBMIH->biSize=sizeof(BITMAPINFOHEADER);
	retV->lpBMIH->biHeight=height;
	retV->lpBMIH->biWidth=width;
	retV->lpBMIH->biCompression=BI_Y8;
	LPBYTE data=((LPBYTE)retV->lpBMIH)+retV->lpBMIH->biSize;
	LPBYTE sdata=GetData(src);
	double x2a=M_PI/width;
	for (int y=0; y<height; y++)
	{
		for (int x=0; x<width; x++)
		{
			double a=x2a*(x-width/2);
			double grad=180.0*a/M_PI;
			int ax=pnt.x+(int)((double)y*sin(a)+0.5);
			int ay=pnt.y-(int)((double)y*cos(a)+0.5);
			if ((ax>0) && (ax<src->lpBMIH->biWidth) && (ay>0) && (ay<src->lpBMIH->biHeight))
				data[x+(height-y-1)*width]=sdata[ax+ay*src->lpBMIH->biWidth];
		}
	}
	return retV;
}

__forceinline pTVFrame _radar2(const pTVFrame src, CPoint pnt, int radius, int angle)
{
	pTVFrame retV=(pTVFrame)malloc(sizeof(TVFrame));
	int width=1000;
	int height;

    if (radius<=0)
        height=src->lpBMIH->biHeight;
    else
        height=radius;

	int dSize=width*height;

	retV->lpData=NULL;
	retV->lpBMIH=(LPBITMAPINFOHEADER)malloc(sizeof(BITMAPINFOHEADER)+dSize);
	memset(retV->lpBMIH,0,sizeof(BITMAPINFOHEADER)+dSize);
	retV->lpBMIH->biSize=sizeof(BITMAPINFOHEADER);
	retV->lpBMIH->biHeight=height;
	retV->lpBMIH->biWidth=width;
	retV->lpBMIH->biCompression=BI_Y8;
	LPBYTE data=((LPBYTE)retV->lpBMIH)+retV->lpBMIH->biSize;
	LPBYTE sdata=GetData(src);
	double x2a;
    if (angle==0)
        x2a=M_PI/width;
    else
        x2a=2*M_PI/width;
	for (int y=0; y<height; y++)
	{
		for (int x=0; x<width; x++)
		{
			double a=x2a*(x-width/2);
			double grad=180.0*a/M_PI;
			int ax=pnt.x+(int)((double)y*sin(a)+0.5);
			int ay=pnt.y-(int)((double)y*cos(a)+0.5);
			if ((ax>0) && (ax<src->lpBMIH->biWidth) && (ay>0) && (ay<src->lpBMIH->biHeight))
				data[x+(height-y-1)*width]=sdata[ax+ay*src->lpBMIH->biWidth];
		}
	}
	return retV;
}


CDataFrame* Locator::DoProcessing(const CDataFrame* pDataFrame)
{
	const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame(DEFAULT_LABEL);
	PASSTHROUGH_NULLFRAME(VideoFrame, pDataFrame);

	if (IsSetupOn())
		((LocatorDialog*)m_SetupObject)->LoadFrame(VideoFrame);

	//pTVFrame frame = _radar(VideoFrame,m_pnt);
    pTVFrame frame = _radar2(VideoFrame,m_pnt,m_Radius, m_Angle);
	CVideoFrame* retVal=CVideoFrame::Create(frame);
        retVal->CopyAttributes(pDataFrame);;
	return retVal;
}

bool Locator::PrintProperties(FXString& text)
{
    FXPropertyKit pc;
    pc.WriteLong("X",m_pnt.x);
    pc.WriteLong("Y",m_pnt.y); 
    pc.WriteInt("Angle",m_Angle);
    pc.WriteInt("Radius",m_Radius);
    text=pc;
	return true;
}

bool Locator::ScanProperties(LPCTSTR text, bool& Invalidate)
{
    FXPropertyKit pc(text);
    pc.GetLong("X",m_pnt.x);
    pc.GetLong("Y",m_pnt.y);
    pc.GetInt("Angle",m_Angle);
    pc.GetInt("Radius",m_Radius);
    return true;
}

bool Locator::ScanSettings(FXString& text)
{
    text="calldialog(true)";
    return true;
}

