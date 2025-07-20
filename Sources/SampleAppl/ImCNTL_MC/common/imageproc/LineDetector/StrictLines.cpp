#include "stdafx.h"
#include <math.h>
#include <imageproc\LineDetector\StrictLines.h>
#include <imageproc\fstfilter.h>
#include <imageproc\edgefilters.h>
#include <imageproc\featuredetector.h>
#include <imageproc\clusters\clusterop.h>
#include <helpers\ctimer.h>
#include <afxtempl.h>

#define LNDETECTOR_CLUSTERS_GROWTH   500
#define LNDETECTOR_DEF_MAGNITUDE_THRESOLD 20
#define LNDETECTOR_DEF_MIN_LINE_LENGTH    10


#define sign(a) ((a<0)?-1:1)

CStrictLines::CStrictLines():
    m_pIntensity(NULL),
    m_pMagnitude(NULL),
    m_MagnitudeThreshold(LNDETECTOR_DEF_MAGNITUDE_THRESOLD),
    m_AOI(NULL),
    m_ROI(0,0,0,0)
{
    //m_ROI=CRect(345,60,542,331);
}

CStrictLines::~CStrictLines()
{
    if (m_pIntensity)
    {
        free(m_pIntensity);
        m_pIntensity=NULL;
        m_pMagnitude=NULL;
    }
    if (m_AOI) freeTVFrame(m_AOI); m_AOI=NULL;
}


__forceinline unsigned char maxdif_s( 
                  unsigned char a00, unsigned char a01, unsigned char a02, 
                  unsigned char a10, unsigned char a11, unsigned char a12, 
                  unsigned char a20, unsigned char a21, unsigned char a22)
{
	return imax(gso_v(a00,a01,a02,a10,a11,a12,a20,a21,a22), gso_h(a00,a01,a02,a10,a11,a12,a20,a21,a22));
}


__forceinline void _edge_detector_s(pTVFrame frame)
{
    LPBYTE Data=GetData(frame);
    DWORD width=frame->lpBMIH->biWidth;
    DWORD height=frame->lpBMIH->biHeight;

	DWORD size =width * height;
	LPBYTE Copy = (LPBYTE)malloc(size);
	memcpy(Copy, Data, size);
    memset(Data,0,size);
	LPBYTE in = Copy + width + 1, end = Copy + width * (height - 1) - 1;
	LPBYTE out = Data + width + 1;
	while (in < end)
	{
        *out=maxdif_s(
                    *(in-width-1),
                    *(in-width),
                    *(in-width+1),
                    *(in-1),
					*in,
                    *(in+1),
                    *(in+width-1),
                    *(in+width),
                    *(in+width+1)
					);
        in++;
		out++;
    }
    _clear_frames(frame,0,2);
    free(Copy);
}

__forceinline void _edge_detector_s(pTVFrame frame,RECT* rc)
{
    LPBYTE Data=GetData(frame);
    DWORD width=frame->lpBMIH->biWidth;
    DWORD height=frame->lpBMIH->biHeight;

	DWORD size =width * height;
	LPBYTE Copy = (LPBYTE)malloc(size);
	memcpy(Copy, Data, size);
    memset(Data,0,size);
    
    Data+=rc->left+rc->top*width;
    LPBYTE inS = Copy + rc->left+rc->top*width;

    for (int i=0; i<rc->bottom-rc->top; i++)
    {
        LPBYTE out=Data;
        LPBYTE in=inS;
        LPBYTE eod=out+rc->right-rc->left;
        while (out<eod)
        {
            *(out)=maxdif_s(
                    *(in-width-1),
                    *(in-width),
                    *(in-width+1),
                    *(in-1),
					*in,
                    *(in+1),
                    *(in+width-1),
                    *(in+width),
                    *(in+width+1)
					);
            in++;
            out++;
        }
        Data+=width;
        inS+=width;
    }
    free(Copy);
}

__forceinline pTVFrame _estimateAOI(pTVFrame frame, RECT* rc)
{
    CRect roi=*rc;
    
    roi.left/=4;
    roi.right/=4;
    roi.top/=4;
    roi.bottom/=4;

    pTVFrame retV=newTVFrame(_cdiminish(frame));
    _diminish(retV);
    if (rc)
        _edge_detector_s(retV,roi);
    else
        _edge_detector_s(retV);
    _simplebinarize(retV,40);

//	_erode(retV);
//	_erode(retV);
    return retV;
}

__forceinline int _estimate_hstep(LPBYTE src)
{
	return (*(src+1)-*(src-1));
}

__forceinline void _estimate_boundaries(LPBYTE src, int w, idata* hdata, idata*vdata)
{
	*hdata=(*(src+1)-*(src-1));
	*vdata=(*(src+w)-*(src-w));
}

typedef struct tagsegoi
{
	int max, brtns;
}segoi;

class soi
{
	segoi* m_segs;
	int	   m_size;
	int    m_length;
public:
	soi(int size)
	{
		m_size=2*size;
		m_segs=(segoi*)malloc(sizeof(segoi)*m_size);
		m_length=0;
	}
	~soi()
	{
		free(m_segs);
	}
	void	reset() { m_length=0; };
	void	add(int max, int brightness) 
	{ 
		m_segs[m_length].max=max; 
		m_segs[m_length].brtns=brightness;
		m_length++; 
		ASSERT(m_length<m_size);
	}
	segoi&	get(int i) { ASSERT(i<m_size); return m_segs[i]; }
	int		length()   { return m_length; };
};

void CStrictLines::EstimateBoundaries()
{
    LPBYTE src, eod;                  
    long i;

    if (m_pIntensity) free(m_pIntensity);
    if (m_AOI) freeTVFrame(m_AOI); m_AOI=NULL;
	
	DWORD ts=GetHRTickCount();
    
    if (m_ROI.IsRectEmpty())
        m_AOI=_estimateAOI(m_OrgFrame,NULL);
    else
        m_AOI=_estimateAOI(m_OrgFrame,m_ROI);
	ts=GetHRTickCount()-ts;
	TRACE("+++ estmateAOI takes %d ms\n",ts);

	ts=GetHRTickCount();

    m_pIntensity=(idata*)malloc(2*m_WrkDataSize*sizeof(idata));
    memset(m_pIntensity,0,2*m_WrkDataSize*sizeof(idata));
    ASSERT(m_pIntensity!=NULL);
    m_pMagnitude=   m_pIntensity +m_WrkDataSize;

    i=m_Width+1; 
    src=m_WrkData+m_Width+1;
    eod=m_WrkData+(m_Height-1)*m_Width-1;
    /* Weight pixel value by appropriate coefficient in convolution matrix */
	
	//// New algorithm
	LPBYTE aoiS=GetData(m_AOI);
	LPBYTE aoi=aoiS+m_AOI->lpBMIH->biWidth;
	LPBYTE aoiE=aoi+m_AOI->lpBMIH->biWidth*(m_AOI->lpBMIH->biHeight-4);
	while (aoi<aoiE)
	{
		if (*aoi)
		{
			int y=aoi-aoiS;
			int x=y%m_AOI->lpBMIH->biWidth; y/=m_AOI->lpBMIH->biWidth;
			x*=4;
			y*=4;
			idata* h=m_pIntensity+x+y*m_Width;
			idata* v=m_pMagnitude+x+y*m_Width;
			src=m_WrkData+x+y*m_Width;
			_estimate_boundaries(src,m_Width,h,v); _estimate_boundaries(src+1,m_Width,h+1,v+1); _estimate_boundaries(src+2,m_Width,h+2,v+2); _estimate_boundaries(src+3,m_Width,h+3,v+3); h+=m_Width; v+=m_Width; src+=m_Width;
			_estimate_boundaries(src,m_Width,h,v); _estimate_boundaries(src+1,m_Width,h+1,v+1); _estimate_boundaries(src+2,m_Width,h+2,v+2); _estimate_boundaries(src+3,m_Width,h+3,v+3); h+=m_Width; v+=m_Width; src+=m_Width;
			_estimate_boundaries(src,m_Width,h,v); _estimate_boundaries(src+1,m_Width,h+1,v+1); _estimate_boundaries(src+2,m_Width,h+2,v+2); _estimate_boundaries(src+3,m_Width,h+3,v+3); h+=m_Width; v+=m_Width; src+=m_Width;
			_estimate_boundaries(src,m_Width,h,v); _estimate_boundaries(src+1,m_Width,h+1,v+1); _estimate_boundaries(src+2,m_Width,h+2,v+2); _estimate_boundaries(src+3,m_Width,h+3,v+3); 
		}
		aoi++;
	}

	{
		idata* h=m_pIntensity;
		idata* v=m_pMagnitude;
		idata* eod=h+m_WrkDataSize;
		while(h<eod)
		{
			if ((*h) || (*v))
			{
				*v=(int)sqrt(*h**h+*v**v);
				if (*v>LNDETECTOR_DEF_MAGNITUDE_THRESOLD)
				{
                    int s;
					*h=255*(*h/ *v); 
                    s=sign(h);
                    *h=(abs(*h)<128)?128:255;
				}
				else *h=0;
			}
			h++; v++;
		}
	}

	{
		idata* h=m_pIntensity;
		idata* v=m_pMagnitude;
//		idata* re=m_pDirections;
		idata* eod=h+m_WrkDataSize;
		soi _soi(m_Width);

		while(h<eod)
		{
			int offset;
			idata* max;
			int	   maxV;
			if (*h==128) // almost horizontal lines
			{
				idata *hs=h, *vs=v;
				_soi.reset();
				while (*hs==128)
				{
					offset=0;
					maxV=0; max=NULL;
					int l=hs-m_pIntensity;
					int r=l;
					while (((hs+offset)<eod) && (*(hs+offset)==128))
					{
						if (maxV<*(vs+offset))
						{
							maxV=*(vs+offset);
							max=   vs+offset;
						}
						*(hs+offset)=0;
						offset+=m_Width;
					}
					r+=offset;
					if (max) 
					{
						_soi.add(max-m_pMagnitude,maxV);
					}
					hs++; vs++; 
					while ((*hs==128) && (hs>m_pIntensity))	    { hs-=m_Width; vs-=m_Width; }
					while ((*hs!=128) && (hs-m_pIntensity<r)) { hs+=m_Width; vs+=m_Width; }
				}
				Add(_soi); 
			}
			else if (*h==255) // almost vertical
			{
				idata *hs=h, *vs=v;
				_soi.reset();
				while (*hs==255)
				{
					offset=0;
					maxV=0; max=NULL;
					int l=hs-m_pIntensity;
					int r=l;
					while (((hs+offset)<eod) && (*(hs+offset)==255))
					{
						if (maxV<*(vs+offset))
						{
							maxV=*(vs+offset);
							max=   vs+offset;
						}
						*(hs+offset)=0;
						offset++;
					}
					r+=offset;
					if (max) 
					{
						_soi.add(max-m_pMagnitude,maxV);
					}
					hs+=m_Width; vs+=m_Width; 
					while ((*hs==255) && (hs>m_pIntensity))	    { hs--; vs--; }
					while ((*hs!=255) && (hs-m_pIntensity<+r+m_Width)) { hs++; vs++; }
				}
				Add(_soi); 
			} 
			h++; v++; 
		}
	}

	ts=GetHRTickCount()-ts;
	TRACE("+++ Making image map takes %d ms\n",ts);
	return;
}

bool CStrictLines::SeekClusters()
{
   bool gl=true;

   m_maxLength=10*(long)(4*sqrt(m_Width*m_Width+m_Height*m_Height));

   EstimateBoundaries();
   Finish();
   // display
   //for (int i=0; (unsigned)i<m_WrkDataSize; i++)
        //*(GetData(m_OrgFrame)+i)=*(m_WrkData+i)/2;   
        //*(GetData(m_OrgFrame)+i)=*(m_pMagnitude+i)/2;   
		//*(GetData(m_OrgFrame)+i)=(*(m_pIntensity+i));   
		//*(GetData(m_OrgFrame)+i)=(*(m_pDirections+i));   
   return true;
}

__forceinline void memrcpyPOINTS(PPOINT d, PPOINT s, size_t i)
{
    if (i==0) return;
    PPOINT  ss=s+i-1;
    while (ss!=s)
    {
        *d=*ss;
        d++; ss--;
    }
    *d=*ss;
}


void CStrictLines::Add(soi& _soi)
{
    if (_soi.length() < LNDETECTOR_DEF_MIN_LINE_LENGTH)
      return;
    
    int pntsNumber=_soi.length();

    int minX=m_Width;
    int minY=m_Height;
    int maxX=0;
    int maxY=0;
	int brightness=0;
	double avgX=0, avgY=0;
	for (int i=0; i<_soi.length(); i++)
	{
		int x=_soi.get(i).max%m_Width,y=_soi.get(i).max/m_Width;
		if (minX>x) minX=x;
		if (maxX<x) maxX=x;
		if (minY>y) minY=y;
		if (maxY<y) maxY=y;
		avgX+=x;
		avgY+=y;
		brightness+=_soi.get(i).brtns;
	}
    if ((maxX-minX)>pntsNumber) return; //cluser overlap edge of the frame
	brightness/=pntsNumber;
	avgX/=pntsNumber;
	avgY/=pntsNumber;

    if (m_ClustersNmb==m_ClustersAllocated)
    {
        m_Clusters =(pCluster)realloc(m_Clusters,sizeof(Cluster)*(m_ClustersAllocated+LNDETECTOR_CLUSTERS_GROWTH)); 
        if (m_Clusters)
        {
            m_ClustersAllocated+=LNDETECTOR_CLUSTERS_GROWTH;
        }
    }
    if (m_Clusters)
    {
        int sizeX, sizeY;
        long datasize, i;

        sizeX=maxX-minX+1;
        sizeY=maxY-minY+1;
        memset(&(m_Clusters[m_ClustersNmb]),0,sizeof(Cluster));
        m_Clusters[m_ClustersNmb].offset.x=minX;
        m_Clusters[m_ClustersNmb].offset.y=minY;
        m_Clusters[m_ClustersNmb].size.cx=sizeX;
        m_Clusters[m_ClustersNmb].size.cy=sizeY;
        m_Clusters[m_ClustersNmb].color=brightness; 
        m_Clusters[m_ClustersNmb].userdata1=NULL;
        m_Clusters[m_ClustersNmb].userdata2=NULL;
        m_Clusters[m_ClustersNmb].userdata3=NULL;
        m_Clusters[m_ClustersNmb].userparam=0;
        m_Clusters[m_ClustersNmb].clstU=NULL;
        m_Clusters[m_ClustersNmb].clstV=NULL;
        m_Clusters[m_ClustersNmb].area=pntsNumber; 
        datasize=m_Clusters[m_ClustersNmb].size.cx*m_Clusters[m_ClustersNmb].size.cy;
        m_Clusters[m_ClustersNmb].clstBM=(LPBYTE)malloc(datasize);
        ASSERT(m_Clusters[m_ClustersNmb].clstBM);
        memset(m_Clusters[m_ClustersNmb].clstBM,0,datasize); 
        
        m_Clusters[m_ClustersNmb].userdata1=(LPBYTE)malloc(pntsNumber*sizeof(POINT));
        //memrcpyPOINTS((PPOINT)m_Clusters[m_ClustersNmb].userdata1, m_Edge.pCurData, m_Edge.pntsNumber); 
		double avgA=0, avgB=0; int cnt=0;
		for (i=0; i<_soi.length(); i++)
		{
			int x=_soi.get(i).max%m_Width,y=_soi.get(i).max/m_Width;
			CPoint pt(x,y);
			((POINT*)m_Clusters[m_ClustersNmb].userdata1)[i]=pt;
			double a=x-avgX; double b=y-avgY; a=a*a; b=b*b;
			avgA+=a;
			avgB+=b;
		}	
		avgA/=_soi.length(); 
		avgB/=_soi.length();
		m_Clusters[m_ClustersNmb].userdata2=(LPBYTE)malloc(sizeof(linefactors));
		memset(m_Clusters[m_ClustersNmb].userdata2,0,sizeof(linefactors));
		((plinefactors)m_Clusters[m_ClustersNmb].userdata2)->coi.x=avgX;
		((plinefactors)m_Clusters[m_ClustersNmb].userdata2)->coi.y=avgY;
		((plinefactors)m_Clusters[m_ClustersNmb].userdata2)->averagesq.x=avgA;
		((plinefactors)m_Clusters[m_ClustersNmb].userdata2)->averagesq.y=avgB;
		
		        // Draw sample cluster
        for(i=0; i<pntsNumber; i++)
            DrawPixel(((PPOINT)(m_Clusters[m_ClustersNmb].userdata1))[i].x-minX, ((PPOINT)(m_Clusters[m_ClustersNmb].userdata1))[i].y-minY, m_Clusters[m_ClustersNmb].clstBM, sizeX);
    }
    m_ClustersNmb++;
}


void CStrictLines::DrawPixel(int x, int y, LPBYTE Data, int width)
{
   ASSERT(x>=0&&y>=0&&x<width);
   //ASSERT(*(Data+x+width*y)!=255);
   /*if (*(Data+x+width*y)==0)
       *(Data+x+width*y)=255;
   else
       *(Data+x+width*y)=128; */
   *(Data+x+width*y)=128;
}

__forceinline plinefactors _getfactors(pCluster cl, int i)
{
    return (plinefactors)cl[i].userdata2;
}

__forceinline void _calc_res(DPOINT& dA, DPOINT& dB, DPOINT& a, DPOINT& b, DPOINT& dR)
{
    double d[4];

    d[0]=distance(dA,a);
    d[1]=distance(dA,b);
    d[2]=distance(dB,a);
    d[3]=distance(dB,b);
    double min=d[0]+d[1]+d[2]+d[3];
    int minI=-1;
    for (int i=0; i<4; i++)
    {
        if (d[i]<min)
        {
            min=d[i];
            minI=i;
        }
    }
    switch (minI)
    {
    case 0:
        dR.x=(dA.x+a.x)/2;
        dR.y=(dA.y+a.y)/2;
        break;
    case 1:
        dR.x=(dA.x+b.x)/2;
        dR.y=(dA.y+b.y)/2;
        break;
    case 2:
        dR.x=(dB.x+a.x)/2;
        dR.y=(dB.y+a.y)/2;
        break;
    case 3:
        dR.x=(dB.x+b.x)/2;
        dR.y=(dB.y+b.y)/2;
        break;
    }
}

void CStrictLines::Finish()
{
    DWORD ts=GetHRTickCount();
    int i;

	for(i=0; i<m_ClustersNmb; i++)
	{
		pCluster cl=&m_Clusters[i];
		plinefactors plf=(plinefactors)cl->userdata2;
		if (plf->averagesq.x>plf->averagesq.y) // more horizontal lines
		{
			double dX, dY;
			double a=0, b=0;
			int used=0;
			int j;
			POINT* pnts=(POINT*)cl->userdata1;
			for (j=0; j<cl->area; j++)
			{
				dX=plf->coi.x-pnts[j].x;
				dY=plf->coi.y-pnts[j].y;
				if (abs((int)dX)>1.0)
				{
					a+=dY/dX;
					used++;
				}
			}
			if (used)
			{
				a/=used;
				b=plf->coi.y-a*plf->coi.x;
				plf->a.x=cl->offset.x; plf->a.y=a*plf->a.x+b;
				plf->b.x=cl->offset.x+cl->size.cx; plf->b.y=a*plf->b.x+b;
                plf->type=TYPE_HSEGMENT;

			}
			double sigma=0;
			for (j=0; j<cl->area; j++)
			{
				sigma+=pow(a*pnts[j].x+b-pnts[j].y,2);
			}
			plf->sigma=sigma/cl->area;
		}
		else // more vertical
		{
			double dX, dY;
			double a=0, b=0;
			int used=0;
			int j;
			POINT* pnts=(POINT*)cl->userdata1;
			for (j=0; j<cl->area; j++)
			{
				dX=plf->coi.x-pnts[j].x;
				dY=plf->coi.y-pnts[j].y;
				if (abs((int)dY)>1)
				{
					a+=dX/dY;
					used++;
				}
			}
			if (used)
			{
				a/=used;
				b=plf->coi.x-a*plf->coi.y;
				plf->a.y=cl->offset.y; plf->a.x=a*plf->a.y+b;
				plf->b.y=cl->offset.y+cl->size.cy; plf->b.x=a*plf->b.y+b;
                plf->type=TYPE_VSEGMENT;

			}
			double sigma=0;
			for (j=0; j<cl->area; j++)
			{
				sigma+=pow(a*pnts[j].y+b-pnts[j].x,2);
			}
			plf->sigma=sigma/cl->area;
		}
		if (plf->sigma>1.5)
		{
			cl->deleted=true;
		}
		TRACE("+++ Cluster %d sigma=%f\n",i, plf->sigma);
	}
    
    goto finish;
    {
        CArray<CRect,CRect> pairs; 
	    for(i=0; i<m_ClustersNmb; i++)
	    {
            if (m_Clusters[i].deleted) continue;

            plinefactors plfa=(plinefactors)m_Clusters[i].userdata2;
	        for(int j=i+1; j<m_ClustersNmb; j++)
	        {
                if (m_Clusters[j].deleted) continue;
                plinefactors plfb=(plinefactors)m_Clusters[j].userdata2;
                if (distance(plfa,plfb)<5)
                {
                    pairs.Add(CRect(i,plfa->type,j,plfb->type));
                }
            }
        }
        TRACE("=====\n");
        int vertex[4];

        for (i=0; i<pairs.GetUpperBound()+1; i++)
        {
            /*TRACE("cluster %d (%c) is connected to %d (%c)\n",pairs[i].left
                                                             ,(pairs[i].top==TYPE_HSEGMENT)?'-':'|'
                                                             ,pairs[i].right
                                                             ,(pairs[i].bottom==TYPE_HSEGMENT)?'-':'|'); */
            int step=0;
            bool found=false;
            vertex[step]=pairs[i].left; 
            step++; 
            vertex[step]=pairs[i].right;        

            for (int j=i+1; j<pairs.GetUpperBound()+1; j++)
            {
                if ((pairs[j].left==vertex[step]) && (pairs[j].right!=vertex[step-1]))
                {
                    step++;
                    if (step<4)
                    {
                        vertex[step]=pairs[j].right;
                        j=i;
                        continue;
                    }
                    else
                    {
                        found=(vertex[0]==pairs[j].right);
                        //double t=
                        break;
                    }
                }
                else if ((pairs[j].right==vertex[step]) && (pairs[j].left!=vertex[step-1]))
                {
                    step++;
                    if (step<4)
                    {
                        vertex[step]=pairs[j].left;
    //                    _chooseandcoorect(dvertex[step-2],dvertex[step-1],dvertex[step],
    //                        _getfactors(m_Clusters,vertex[step])->a,_getfactors(m_Clusters,vertex[step])->b);                    //TRACE("%d ",     pairs[j].right);
                        j=i;
                        continue;
                    }
                    else
                    {
                        found=(vertex[0]==pairs[j].left);
                        break;
                    }
                }
            }
        
            if (found)
            {
                DPOINT dvertex[4];

                _calc_res(_getfactors(m_Clusters,vertex[0])->a ,_getfactors(m_Clusters,vertex[0])->b,
                          _getfactors(m_Clusters,vertex[1])->a, _getfactors(m_Clusters,vertex[1])->b,dvertex[0]);
                _calc_res(_getfactors(m_Clusters,vertex[1])->a ,_getfactors(m_Clusters,vertex[1])->b,
                          _getfactors(m_Clusters,vertex[2])->a, _getfactors(m_Clusters,vertex[2])->b,dvertex[1]);
                _calc_res(_getfactors(m_Clusters,vertex[2])->a ,_getfactors(m_Clusters,vertex[2])->b,
                          _getfactors(m_Clusters,vertex[3])->a, _getfactors(m_Clusters,vertex[3])->b,dvertex[2]);
                _calc_res(_getfactors(m_Clusters,vertex[3])->a ,_getfactors(m_Clusters,vertex[3])->b,
                          _getfactors(m_Clusters,vertex[0])->a, _getfactors(m_Clusters,vertex[0])->b,dvertex[3]);

                plinefactors plfa=(plinefactors)m_Clusters[vertex[0]].userdata2;
                plfa->type=TYPE_RECTANGLE;
                plfa->a.x=dvertex[0].x;
                plfa->a.y=dvertex[0].y;
                plfa->b.x=dvertex[1].x;
                plfa->b.y=dvertex[1].y;
                plfa->c.x=dvertex[2].x;
                plfa->c.y=dvertex[2].y;
                plfa->d.x=dvertex[3].x;
                plfa->d.y=dvertex[3].y;
                m_Clusters[vertex[1]].deleted=true;
                m_Clusters[vertex[2]].deleted=true;
                m_Clusters[vertex[3]].deleted=true;
                //TRACE("Found rectangle %d %d %d %d\n",vertex[0],vertex[1],vertex[2],vertex[3]);
                /*TRACE("Found rectangle (%g,%g) (%g,%g) (%g,%g) (%g,%g)\n",
                    dvertex[0].x,dvertex[0].y,
                    dvertex[1].x,dvertex[1].y,
                    dvertex[2].x,dvertex[2].y,
                    dvertex[3].x,dvertex[3].y
                    );*/
            }
            else
            {
                //TRACE("Not found rectangle %d %d %d %d\n",vertex[0],vertex[1],vertex[2],vertex[3]);
            }

        }
    }
    ClustersRemoveDeleted(m_Clusters,m_ClustersNmb);
finish:
	ts=GetHRTickCount()-ts;
	TRACE("+++ Finishing takes %d ms\n",ts);
}

plinefactors  CStrictLines::GetFigure(int i)
{
    if (i<m_ClustersNmb)
        return _getfactors(m_Clusters,i);
    return NULL;
}