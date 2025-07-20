#include "StdAfx.h"
#include "AverageIntense.h"
#include <gadgets\gadbase.h>
#include "gadgets\FigureFrame.h"
#include "gadgets\VideoFrame.h"
#include <gadgets\vftempl.h>
#include "video\stdcodec.h"
#include <math\intf_sup.h>
#include <float.h>
#include "imageproc\simpleip.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define THIS_MODULENAME "AverageIntensity"

bool CCell::PtInRegion(POINT point)
{
	/* The points creating the polygon. */
	double x[4];
	double y[4];
	double x1,x2;

	/* The coordinates of the point */
	double px = point.x, py = point.y;

	/* How many times the ray crosses a line-segment */
	int crossings = 0;

	/* Coordinates of the points */
	for (int i=0; i<4; i++)
	{
		x[i] = points[i].x;
		y[i] = points[i].y;
	}

	/* Iterate through each line */
	for ( int i = 0; i < 4; i++ ){

		/* This is done to ensure that we get the same result when
		the line goes from left to right and right to left */

		if (x[i]==px && y[i]==py)
			return true;

		if ( x[i] < x[ (i+1)%4 ] ){
			x1 = x[i];
			x2 = x[(i+1)%4];
		} else {
			x1 = x[(i+1)%4];
			x2 = x[i];
		}

		/* First check if the ray is possible to cross the line */
		if ( px >= x1 && px <= x2 && ( py <= y[i] || py <= y[(i+1)%4] ) ){
			static const double eps = 0.000001;

			/* Calculate the equation of the line */
			double dx = x[(i+1)%4] - x[i];
			double dy = y[(i+1)%4] - y[i];
			double k;

			if ( fabs(dx) < eps ){
				if (px==x[i])
					crossings++;
			} else {
				k = dy/dx;
				double m = y[i] - k * x[i];

				/* Find if the ray crosses the line */
				double y2 = k * px + m;
				if ( py <= y2 ){
					crossings++;
				}
			}

		}
	}

	if ( crossings % 2 == 1 ){
		return true;
	}
	return false;
}

void CCell::GetRgnBox(LPRECT rect)
{
	int minX = INT_MAX, maxX=INT_MIN, minY = INT_MAX, maxY = INT_MIN;
	for (int i=0; i<4; i++)
	{
		if (points[i].x<minX)
			minX = points[i].x;
		if (points[i].x>maxX)
			maxX = points[i].x;
		if (points[i].y<minY)
			minY = points[i].y;
		if (points[i].y>maxY)
			maxY = points[i].y;
	}
	rect->left = minX; rect->right = maxX;
	rect->top = minY; rect->bottom = maxY;
}

IMPLEMENT_RUNTIME_GADGET_EX(AverageIntensity, CFilterGadget, LINEAGE_FILEX, TVDB400_PLUGIN_NAME);

void CALLBACK onGetNet(CDataFrame* lpData, void* lpParam, CConnector* lpInput)
{
	((AverageIntensity*)lpParam)->onText(lpData);
}

AverageIntensity::AverageIntensity()
{
	m_pInputs[0] = new CInputConnector(vframe);
	m_pInputs[1] = new CInputConnector(transparent, ::onGetNet, this);
	m_pOutput = new COutputConnector(vframe);	
	m_Cells = NULL;
	m_CellsSize = 0;
	m_sVideoLabel = "";
	xDim = 0;
	yDim = 0;
  m_OutputMode = modeReplace ;
	Resume();
}

void AverageIntensity::onText(CDataFrame* lpData)
{
	int ntime=0;
	LARGE_INTEGER ntime1,ntime2;
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&ntime1);
	CFigureFrame *pFf = NULL;
	CFramesIterator* Iterator = lpData->CreateFramesIterator(figure);
	if (Iterator!=NULL)
	{
		m_Lock.Lock();
		RemoveCells();
		pFf = (CFigureFrame*)Iterator->Next(DEFAULT_LABEL);
		int counter = 0;
		while (pFf)
		{
			if (m_sVideoLabel.Compare(pFf->GetLabel())!=0)
			{
				pFf = (CFigureFrame*)Iterator->Next(DEFAULT_LABEL);
				continue;
			}
			int col=-1, row=-1;
			pFf->Attributes()->GetInt("totalCol", xDim);
			pFf->Attributes()->GetInt("totalRow", yDim);
			if (m_Cells == NULL)
			{
				m_Cells = new CCell[xDim*yDim];
				for (int i=0; i<xDim; i++)
				{
					for (int j=0; j<yDim; j++)
					{
						m_Cells[i+j*xDim].i = i;
						m_Cells[i+j*xDim].j = j;
					}
				}
				m_CellsSize = xDim*yDim;
			}

			//CAUTION!!!
			//points were added starting from 0,0 to dX,dY. Top to bottom, left to right!
			CDPoint point;
			POINT tP;
			for (int i=0; i<pFf->GetNumberVertex(); i++)
			{
				point = pFf->GetAt(i);
				tP.x = (int)(floor(point.x+0.5)); //hell's round func =)
				tP.y = (int)(floor(point.y+0.5));
				col = i/(yDim+1);
				row = i%(yDim+1);
				if (col<xDim && row<yDim)
					m_Cells[col+row*xDim].points[0] = tP; //in i,j cell it top-left corner
				if (col>0 && row<yDim)
					m_Cells[col-1+row*xDim].points[3] = tP; //in i-1,j cell it's top-right corner
				if (row>0 && col<xDim)
					m_Cells[col+(row-1)*xDim].points[1] = tP; //in i,j-1 cell it's bottom-left corner
				if (col>0 && row>0)
					m_Cells[col-1+(row-1)*xDim].points[2] = tP; //in i-1,j-1 cell it's bottom right corner
			}
			pFf = (CFigureFrame*)Iterator->Next(DEFAULT_LABEL);
		}
		m_Lock.Unlock();
		delete Iterator; 
    Iterator = NULL;
	}
	QueryPerformanceCounter(&ntime2);
	ntime = (int)((ntime2.QuadPart-ntime1.QuadPart)/(freq.QuadPart/1000));
	TRACE("PARSING NET: %d %d: %ld milliseconds\n", xDim, yDim, ntime);
	lpData->RELEASE(lpData);
}

int AverageIntensity::DoJob()
{
	CDataFrame* pDataFrame = NULL;
	while (m_pInputs[0]->Get(pDataFrame))
	{
		ASSERT(pDataFrame);
		if ( Tvdb400_IsEOS (pDataFrame ) )
		{
			if (!m_pOutput->Put(pDataFrame))
				pDataFrame->RELEASE(pDataFrame);
			return WR_CONTINUE;
		}
        switch (m_Mode)
		{
			case mode_reject:
				pDataFrame->RELEASE(pDataFrame);
				break;
			case mode_transmit:
				if (!m_pOutput->Put(pDataFrame))
					pDataFrame->RELEASE(pDataFrame);
				break;
			case mode_process:
                {
                    CDataFrame* Container = pDataFrame->CopyContainer();
		            if (Container)
		            {
			            pDataFrame->RELEASE(pDataFrame);
			            pDataFrame = Container;
		            }
		            CDataFrame* pResultFrame = DoProcessing(pDataFrame);

		            if ((pResultFrame) && (!m_pOutput->Put(pResultFrame)))
			            pResultFrame->RELEASE(pResultFrame);
                }
        }
	}
	return WR_CONTINUE;
}

CDataFrame* AverageIntensity::DoProcessing(const CDataFrame* pDataFrame)
{
	const CVideoFrame *vf1 = pDataFrame->GetVideoFrame(DEFAULT_LABEL);
	PASSTHROUGH_NULLFRAME(vf1, pDataFrame);
  if (m_CellsSize<=0)
    return (CDataFrame*) vf1;

	m_Lock.Lock();
	m_sVideoLabel = vf1->GetLabel();

	const pTVFrame frame2 = vf1;

	LPBITMAPINFOHEADER hdr = (LPBITMAPINFOHEADER)malloc(sizeof(BITMAPINFOHEADER));
	memset(hdr, 0, sizeof(BITMAPINFOHEADER));
	hdr->biHeight = yDim;
	hdr->biWidth = xDim;
	hdr->biSize = sizeof(BITMAPINFOHEADER);
	hdr->biPlanes =1;
	hdr->biBitCount = 0;
	hdr->biCompression = BI_Y8;
	hdr->biSizeImage = xDim * yDim;
	pTVFrame frame3 = new TVFrame;
	frame3->lpBMIH=hdr;
	frame3->lpData=(LPBYTE)malloc(sizeof(BYTE)*xDim*yDim);
	memset(frame3->lpData, 0, sizeof(BYTE)*xDim*yDim);
	CVideoFrame *vf0 = CVideoFrame::Create(frame3);

	long xSize = frame2->lpBMIH->biWidth;
	long ySize = frame2->lpBMIH->biHeight;

	LPBYTE bm0 = GetData(vf0);
	LPBYTE bm1 = GetData(frame2);
	for (int i=0; i<m_CellsSize; i++)
	{
		double averIntensity = 0;
		CCell cell = m_Cells[i];
		CRect boundRect;
		POINT pt;
		LONG pointsCounter = 0;
		double sumIntensity = 0.0;
		cell.GetRgnBox(&boundRect);
		for (int l1=boundRect.left; l1<boundRect.right+1; l1++)
		{
			for (int l2=boundRect.top; l2<boundRect.bottom+1; l2++)
			{
				pt.x = l1;
				pt.y = l2;
				if (cell.PtInRegion(pt))
				{
					sumIntensity+=bm1[pt.y*xSize+pt.x];
					pointsCounter++;
				}
			}
		}
		if (pointsCounter>0)
			averIntensity = (BYTE)(sumIntensity/pointsCounter);
		else
			averIntensity = bm1[boundRect.top*xSize+boundRect.left];

		bm0[cell.j*xDim+cell.i] = (int)averIntensity;
	}

	/*
	long xSize = frame1->lpBMIH->biWidth;
	long ySize = frame1->lpBMIH->biHeight;
	double pixXSize = (double)xSize/xDim;
	double pixYSize = (double)ySize/yDim;

	LPBYTE bm = GetData(frame1);


	for (int i=0; i<m_CellsSize; i++)
	{
	double averIntensity = 0;
	CCell cell = m_Cells[i];
	CRect boundRect;
	POINT pt;
	LONG pointsCounter = 0;
	double sumIntensity = 0.0;
	cell.GetRgnBox(&boundRect);
	for (int l1=boundRect.left; l1<boundRect.right+1; l1++)
	{
	for (int l2=boundRect.top; l2<boundRect.bottom+1; l2++)
	{
	pt.x = l1;
	pt.y = l2;
	if (cell.PtInRegion(pt))
	{
	sumIntensity+=bm1[pt.y*xSize+pt.x];
	pointsCounter++;
	}
	}
	}
	if (pointsCounter>0)
	averIntensity = (BYTE)(sumIntensity/pointsCounter);
	else
	averIntensity = bm1[boundRect.top*xSize+boundRect.left];

	for (int k1=0; k1<pixXSize; k1++)
	{
	for (int k2=0; k2<pixYSize; k2++)
	{
	bm[(int(cell.j*pixYSize)+k2)*xSize+((int)(cell.i*pixXSize)+k1)] = (BYTE)averIntensity;
	}
	}
	}     */
	m_Lock.Unlock();

	//CVideoFrame *retVal = CVideoFrame::Create(frame);
	//CVideoFrame *vf = CVideoFrame::Create(frame3);
	vf0->SetTime(pDataFrame->GetTime());
	vf0->ChangeId(pDataFrame->GetId());
	return vf0;
}

void AverageIntensity::ShutDown()
{
	CGadget::ShutDown();

	CDataFrame * pFr ;
	for (int i=0; i<2; i++)
	{
		while ( m_pInputs[i]->Get( pFr ) )
			pFr->Release( pFr )  ;
	}

	delete m_pInputs[0];
	m_pInputs[0] = NULL;
	delete m_pInputs[1];
	m_pInputs[1] = NULL;
	delete m_pOutput;
	m_pOutput = NULL;

	RemoveCells();
}

void AverageIntensity::RemoveCells()
{
	delete []m_Cells;
	m_Cells = NULL;
	m_CellsSize = 0;
}

bool AverageIntensity::PrintProperties(FXString& text)
{
	FXPropertyKit pc;
	pc.WriteString("VideoLabel",m_sVideoLabel);
	text=pc;
	return true;
}

bool AverageIntensity::ScanProperties(LPCTSTR text, bool& Invalidate)
{
	FXPropertyKit pc(text);
	pc.GetString("VideoLabel",m_sVideoLabel);
	return true;
}


bool AverageIntensity::ScanSettings(FXString& text)
{
	text.Format("template(EditBox(VideoLabel))",TRUE,FALSE);
	return true;
}
