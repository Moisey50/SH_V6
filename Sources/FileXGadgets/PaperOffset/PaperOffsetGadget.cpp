#include "StdAfx.h"
#include "PaperOffsetGadget.h"
#include "classes\dpoint.h"
#include "gadgets\FigureFrame.h"
#include "gadgets\VideoFrame.h"
#include "gadgets\containerframe.h"
#include <math\intf_sup.h>
#include <complex>
#include <gadgets\vftempl.h>

IMPLEMENT_RUNTIME_GADGET_EX(PaperOffset, CFilterGadget, LINEAGE_FILEX, TVDB400_PLUGIN_NAME);

PaperOffset::PaperOffset()
{
	m_FirstPoint = CPoint(0,0);
	m_SecondPoint = CPoint(0,0);
	m_PartsNum = 1;
	m_IntensOffset = 80;
	m_GradOffset = 60;
	m_AveragePointCount = 3;

	m_pInput = new CInputConnector(vframe);
	m_pOutput = new COutputConnector(transparent);
  m_OutputMode = modeReplace ;

	Resume();
}

PaperOffset::~PaperOffset()
{
	delete m_pInput;
	m_pInput = NULL;
	delete m_pOutput;
	m_pOutput = NULL;
}

CDPoint PaperOffset::GetWhitePointUp(LPBYTE data, int xSize, int ySize, int x, int y, bool firstPoint)
{
	int gradient = 0;
	int y_copy = y;
	double a = double(m_TopSecondPoint.y-m_TopFirstPoint.y)/(m_TopSecondPoint.x-m_TopFirstPoint.x);
	double b = (double)(m_TopFirstPoint.y*m_TopSecondPoint.x-m_TopFirstPoint.x*m_TopSecondPoint.y)/(m_TopSecondPoint.x-m_TopFirstPoint.x);
	int y_top = (int)(a*x+b);
		
	while (y_copy>0 && y_copy>y_top-1)
	{
		double bottom = GetIntensityAt(data, xSize, ySize, x, y_copy);
		double top = GetIntensityAt(data, xSize, ySize, x, y_copy-1);
		gradient = (int)(top - bottom);
		if (firstPoint && (-gradient>m_GradOffset) && bottom>m_IntensOffset)
		{
			return CDPoint(x,y_copy);
		}
		if ((!firstPoint) && (gradient>m_GradOffset) && top>m_IntensOffset)
			return CDPoint(x,y_copy-1);
		y_copy--;
	}
	return CDPoint(x,y);
}

double PaperOffset::GetSquare(CDPoint *bottom, CDPoint *top, double xStep)
{
	double sum = 0;
	for (int i=0; i<m_PartsNum; i++)
	{
		sum += ( (bottom[i].y+bottom[i+1].y)/2.0 - (top[i].y+top[i+1].y)/2.0 )* xStep;
	}
	return sum;
}

double PaperOffset::GetIntensityAt(LPBYTE data, int xSize, int ySize, int x, int y)
{
	double result = 0.0;
	if (m_FirstPoint.x == m_SecondPoint.x)
		return data[y*xSize+x];

	//line equation
	double a = double(m_SecondPoint.y-m_FirstPoint.y)/(m_SecondPoint.x-m_FirstPoint.x);
	double b = double(y-a*x);

	for (int i=-m_AveragePointCount+1; i<m_AveragePointCount; i++)
	{
		int x_c = (int)(x+i);
		int y_c = (int)(a*x_c + b);
		if (x_c>=0 && y_c>=0 && x_c<xSize && y_c<ySize)
			result += data[y_c*xSize+x_c];
	}
	return result/(2*m_AveragePointCount-1);
}

CDataFrame* PaperOffset::DoProcessing(const CDataFrame* pDataFrame)
{
	const CVideoFrame *vf = pDataFrame->GetVideoFrame(DEFAULT_LABEL);
	PASSTHROUGH_NULLFRAME(vf, pDataFrame);

	LPBYTE data = GetData(vf);
	int xDim = vf->lpBMIH->biWidth;
	int yDim = vf->lpBMIH->biHeight;

	CSize dist = m_SecondPoint - m_FirstPoint;
	CDPoint step = CDPoint( (double)dist.cx/m_PartsNum, (double)dist.cy/m_PartsNum);

	CFigureFrame *fig1 = CFigureFrame::Create();
	CFigureFrame *fig2 = CFigureFrame::Create();
	fig1->Attributes()->WriteString("color", "0x0000FF");
	fig2->Attributes()->WriteString("color", "0x0000FF");
	fig1->ChangeId(pDataFrame->GetId());
	fig2->ChangeId(pDataFrame->GetId());
	fig1->SetTime(pDataFrame->GetTime());
	fig2->SetTime(pDataFrame->GetTime());

	CDPoint *bottom = new CDPoint[m_PartsNum+1];
	CDPoint *top = new CDPoint[m_PartsNum+1];

	CPoint currPoint = m_FirstPoint;
	for (int i=0; i<m_PartsNum+1; i++)
	{
		currPoint.x = (int)(m_FirstPoint.x+i*step.x);
		currPoint.y = (int)(m_FirstPoint.y+i*step.y);
		CDPoint tempPoint = GetWhitePointUp(data, xDim, yDim, currPoint.x, currPoint.y, true);
		bottom[i] = tempPoint;
		fig1->AddPoint(tempPoint);
		if (i>0 && i<m_PartsNum)
			tempPoint = GetWhitePointUp(data,xDim,yDim,(int)tempPoint.x,(int)tempPoint.y, false);
		top[i] = tempPoint;
		fig2->AddPoint(tempPoint);
	}
	
	CQuantityFrame *square = CQuantityFrame::Create(GetSquare(bottom, top, fabs(step.x)));
	square->ChangeId(pDataFrame->GetId());
	square->SetTime(pDataFrame->GetTime());

	delete []bottom;
	delete []top;

	CContainerFrame *cont = CContainerFrame::Create();
	cont->ChangeId(pDataFrame->GetId());
	cont->SetTime(pDataFrame->GetTime());
	cont->AddFrame(vf);
  CDataFrame *pdf = (CDataFrame*)vf ;
	pdf->AddRef();
	cont->AddFrame(fig1);
	cont->AddFrame(fig2);
	cont->AddFrame(square);
	return cont;
}

bool PaperOffset::PrintProperties(FXString& text)
{
	FXPropertyKit pc;
	pc.WriteLong("BottomFirstPointX", m_FirstPoint.x);
	pc.WriteLong("BottomFirstPointY", m_FirstPoint.y);
	pc.WriteLong("BottomSecondPointX", m_SecondPoint.x);
	pc.WriteLong("BottomSecondPointY", m_SecondPoint.y);
	pc.WriteLong("TopFirstPointX", m_TopFirstPoint.x);
	pc.WriteLong("TopFirstPointY", m_TopFirstPoint.y);
	pc.WriteLong("TopSecondPointX", m_TopSecondPoint.x);
	pc.WriteLong("TopSecondPointY", m_TopSecondPoint.y);
	pc.WriteLong("ColonNumber", m_PartsNum);
	pc.WriteInt("GradientOffset", m_GradOffset);
	pc.WriteInt("AveragePointCount", m_AveragePointCount);
	pc.WriteInt("IntensityOffset", m_IntensOffset);
	text=pc;
	return true;
}

bool PaperOffset::ScanProperties(LPCTSTR text, bool& Invalidate)
{
	FXPropertyKit pc(text);
	pc.GetLong("BottomFirstPointX", m_FirstPoint.x);
	pc.GetLong("BottomFirstPointY", m_FirstPoint.y);
	pc.GetLong("BottomSecondPointX", m_SecondPoint.x);
	pc.GetLong("BottomSecondPointY", m_SecondPoint.y);
	pc.GetLong("TopFirstPointX", m_TopFirstPoint.x);
	pc.GetLong("TopFirstPointY", m_TopFirstPoint.y);
	pc.GetLong("TopSecondPointX", m_TopSecondPoint.x);
	pc.GetLong("TopSecondPointY", m_TopSecondPoint.y);
	pc.GetInt("ColonNumber", m_PartsNum);
	pc.GetInt("GradientOffset", m_GradOffset);
	pc.GetInt("AveragePointCount", m_AveragePointCount);
	pc.GetInt("IntensityOffset", m_IntensOffset);
	return true;
}


bool PaperOffset::ScanSettings(FXString& text)
{
	text.Format("template(Spin(BottomFirstPointX,0,640),\
				Spin(BottomFirstPointY,0,480),\
				Spin(BottomSecondPointX,0,640),\
				Spin(BottomSecondPointY,0,480),\
				Spin(TopFirstPointX,0,640),\
				Spin(TopFirstPointY,0,480),\
				Spin(TopSecondPointX,0,640),\
				Spin(TopSecondPointY,0,480),\
				Spin(ColonNumber,0,100),\
				Spin(GradientOffset,0,255),\
				Spin(IntensityOffset,0,255),\
				Spin(AveragePointCount,1,10))",
				TRUE,FALSE);
	return true;
}