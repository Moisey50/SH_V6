//  $File : ClusterOp.h - Cluster manipulation primetives
//  (C) Copyright The File X Ltd 2002. 
//
//
//
//  Revision History (excluding minor changes for specific compilers)
//   13 May 02 Firts release version, all followed changes must be listed below  (Andrey Chernushich)


#ifndef CLUSTER_OPERATIONS
#define CLUSTER_OPERATIONS

#include <imageproc\clusters\Clusters.h>
//#include <SimpleIP.h>
#include <imageproc\rotate.h>
#include <video\DIBViewBase.h>

typedef struct LINE_PARAMS_STRUCT
{
	double slope, interlace;
	double std, rxy, dev;	// standrad dev., correlation, linear deviation
	double x0, y0;
}LINEPARAMS, *LPLINEPARAMS;

__forceinline void Cluster_Exchange(pCluster clusters, int& length, int pos1, int pos2)
{
    ASSERT(pos1<length);
    ASSERT(pos2<length);
    Cluster tmpP;
    memcpy(&tmpP,               &(clusters[pos1]),  sizeof(Cluster));
    memcpy(&(clusters[pos1]),   &(clusters[pos2]),  sizeof(Cluster));
    memcpy(&(clusters[pos2]),   &tmpP,              sizeof(Cluster));
}

__forceinline void Cluster_SortX(pCluster clusters, int& length)
{
    bool sorted=true;
    do
    {
        sorted=true;
        for (int i=0; i<length-1; i++)
        {
            if ((clusters[i].offset.x)>(clusters[i+1].offset.x))
            {
                Cluster_Exchange(clusters, length, i, i+1);
                sorted=false;
            }
        }
    }while (!sorted);
}

__forceinline void Cluster_SortArea(pCluster clusters, int& length)
{
    bool sorted=true;
    do
    {
        sorted=true;
        for (int i=0; i<length-1; i++)
        {
            if ((clusters[i].area)>(clusters[i+1].area))
            {
                Cluster_Exchange(clusters, length, i, i+1);
                sorted=false;
            }
        }
    }while (!sorted);
}

__forceinline void Cluster_SortArea(pClustersInfo pCI)
{
    Cluster_SortArea(pCI->m_Clusters, pCI->m_ClustersNmb);
}

__forceinline int Cluster_GetOverlappedX(pCluster a, pCluster b)
{
    int l=a->offset.x,r=a->offset.x+a->size.cx;
    if (l<(b->offset.x)) l=b->offset.x;
    if (r>(b->offset.x+b->size.cx)) r=b->offset.x+b->size.cx;
    if (r<=l) return 0;
    return (r-l);
}

__forceinline int Cluster_GetOverlappedY(pCluster a, pCluster b)
{
    int l=a->offset.y,r=a->offset.y+a->size.cy;
    if (l<(b->offset.y)) l=b->offset.y;
    if (r>(b->offset.y+b->size.cy)) r=b->offset.y+b->size.cy;
    if (r<=l) return 0;
    return (r-l);
}

__forceinline int Cluster_GetXYPos(Cluster& a, int width)
{
    int y=(2*a.offset.y)/(3*a.size.cy);
    return (a.offset.x+y*width);
}

__forceinline void Cluster_SortLinepos(pClustersInfo pCI, int fwidth)
{
    bool sorted=true;
    do
    {
        sorted=true;
        for (int i=0; i<pCI->m_ClustersNmb-1; i++)
        {
            if (Cluster_GetXYPos(pCI->m_Clusters[i],fwidth)>Cluster_GetXYPos(pCI->m_Clusters[i+1],fwidth))
            {
                Cluster_Exchange(pCI->m_Clusters, pCI->m_ClustersNmb, i, i+1);
                sorted=false;
            }
        }
    }while (!sorted);

/*    int clstrnmb=pCI->m_ClustersNmb;
    pCluster clusters=pCI->m_Clusters;
    Cluster_SortX(clusters, clstrnmb);
    for (int i=0; i<clstrnmb-1; i++)
    {
        Cluster_GetOverlappedX(&clusters[i],&clusters[i+1]);
    } */
}


__forceinline void Cluster_RemoveAt(pCluster clusters, int& length, int pos)
{
    ASSERT(pos<length);
    if (clusters[pos].clstBM) free(clusters[pos].clstBM);
    if (clusters[pos].clstU) free(clusters[pos].clstU);
    if (clusters[pos].clstV) free(clusters[pos].clstV);
    if (clusters[pos].userdata1) free(clusters[pos].userdata1);
    if (clusters[pos].userdata2) free(clusters[pos].userdata2);
    if (clusters[pos].userdata3) free(clusters[pos].userdata3);
    if (pos==length-1) { length--;  return; }
    length--;
    memmove(&(clusters[pos]),&(clusters[pos+1]), sizeof(Cluster)*(length-pos));
}

__forceinline void Cluster_LeaveLargest(pCluster clusters, int& length)
{
    Cluster_SortArea(clusters, length);
    while (length>1) Cluster_RemoveAt(clusters, length, 0);
}

__forceinline void ClustersRemoveDeleted(pCluster clusters, int& length)
{
	for(int i=0; i<length; i++)
    {
		if (clusters[i].deleted)
		{
			Cluster_RemoveAt(clusters,length,i);
            i--;
		}
	}
}

__forceinline void ClustersRemoveDeleted(pClustersInfo pCI)
{
    ClustersRemoveDeleted(pCI->m_Clusters,pCI->m_ClustersNmb);
}

__forceinline void ClustersRemoveAll(pCluster clusters, int& length)
{
	for(int i=0; i<length; i++)
    {
			Cluster_RemoveAt(clusters,length,i);
            i--;
	}
}

__forceinline void ClustersRemoveAll(pClustersInfo pCI)
{
    ClustersRemoveAll(pCI->m_Clusters,pCI->m_ClustersNmb);
}


__forceinline void Cluster_MinMaxAreaFilter(pCluster clusters, int& length, int min, int max, bool remove=true)
{
    for (int i=0; i<length; i++)
    {
        if (max>0) 
        {
            if (clusters[i].area>max) clusters[i].deleted=true;
        }
        if (min>0) 
        {
            if (clusters[i].area<min) clusters[i].deleted=true;
        }
    }
    if (remove) { ClustersRemoveDeleted(clusters,length); }
}

__forceinline void Cluster_XYRatioFilter(pCluster clusters, int& length, double min, double max, bool remove=true)
{
    for (int i=0; i<length; i++)
    {
        ASSERT(clusters[i].size.cy!=0);
        double ratio=((double)clusters[i].size.cx)/clusters[i].size.cy;
        if (max>0) 
        {
            if (ratio>max) { Cluster_RemoveAt(clusters,length,i); i--; continue; }
        }
        if (min>0) 
        {
            if (ratio<min) { Cluster_RemoveAt(clusters,length,i); i--;  }
        }
    }
    if (remove) { ClustersRemoveDeleted(clusters,length); }
}

__forceinline void Cluster_HeightFilter(pCluster clusters, int& length, int min, int max, bool remove=true)
{
    for (int i=0; i<length; i++)
    {
        if (max>0) 
        {
            if (clusters[i].size.cy>max) { Cluster_RemoveAt(clusters,length,i); i--; continue; }
        }
        if (min>0) 
        {
            if (clusters[i].size.cy<min) { Cluster_RemoveAt(clusters,length,i); i--;  }
        }
    }
    if (remove) { ClustersRemoveDeleted(clusters,length); }
}

__forceinline void Cluster_WidthFilter(pCluster clusters, int& length, int min, int max, bool remove=true)
{
    for (int i=0; i<length; i++)
    {
        if (max>0) 
        {
            if (clusters[i].size.cx>max) { Cluster_RemoveAt(clusters,length,i); i--; continue; }
        }
        if (min>0) 
        {
            if (clusters[i].size.cx<min) { Cluster_RemoveAt(clusters,length,i); i--;  }
        }
    }
    if (remove) { ClustersRemoveDeleted(clusters,length); }
}


__forceinline void Cluster_DensityFilter(pCluster clusters, int& length, double min, double max)
{
    for (int i=0; i<length; i++)
    {
        ASSERT((clusters[i].size.cy * clusters[i].size.cx)!=0);

        double density=((double)clusters[i].area)/(clusters[i].size.cy * clusters[i].size.cx);
        if (max>0) 
        {
            if (density>max) { Cluster_RemoveAt(clusters,length,i); i--; continue; }
        }
        if (min>0) 
        {
            if (density<min) { Cluster_RemoveAt(clusters,length,i); i--;  }
        }
    }
}

__forceinline void Cluster_ColorFilter(pCluster clusters, int& length, int Color)
{
    for (int i=0; i<length; i++)
    {
        if (clusters[i].colormax!=clusters[i].colormin)
        {
		    if (Color>=clusters[i].colormin && Color<=clusters[i].colormax)
                { Cluster_RemoveAt(clusters,length,i); i--;}
        }
        else
        {
            if (clusters[i].color!=Color) 
                { Cluster_RemoveAt(clusters,length,i); i--;}
        }
    }
}


__forceinline void Cluster_ShowCluster(pCluster cluster, LPBYTE data, int width, int height, int color)
{
    for (int y=0; y<cluster->size.cy; y++)
    {
        for (int x=0; x<cluster->size.cx; x++)
        {
            if (*(cluster->clstBM+x+y*cluster->size.cx))
            {
                *(data+width*(y+cluster->offset.y)+x+cluster->offset.x)=color;
            }
        }
    }
}

__forceinline void Cluster_ShowClusters(pCluster clusters, int length, pTVFrame frame, int bgColor=128)
{
    if (!frame) return;
    LPBYTE framedata = GetData(frame);
    int    width  = frame->lpBMIH->biWidth;
    int    height = frame->lpBMIH->biHeight;
    int    size   = width*height;

    memset(framedata,bgColor,size);
    for (int i=0; i<length; i++)
    {   
        Cluster_ShowCluster(&(clusters[i]), framedata, width, height, clusters[i].color);
    }
}

const BITMAPINFOHEADER bmTmpl={sizeof(BITMAPINFOHEADER),0,0,1,9,BI_Y8,0,0,0,0,0};

__forceinline pTVFrame Cluster_GetFrame(pCluster cluster, RECT* rc=NULL)
{
    DWORD isize;
    int cx,cy;
    
    if (!rc)
        { cx = cluster->size.cx; cy=cluster->size.cy; }   
    else
        { cx = rc->right-rc->left; cy=rc->bottom-rc->top; }

    if (cx%4!=0)
        cx+=4-(cx%4);
    if (cy%4!=0)
        cy+=4-(cy%4);

    isize=cx*cy;

    LPBITMAPINFOHEADER lpBMIH=(LPBITMAPINFOHEADER)malloc(sizeof(BITMAPINFOHEADER)+isize);
    
    memcpy(lpBMIH,&bmTmpl,sizeof(bmTmpl));
    
    lpBMIH->biSizeImage=isize;
    lpBMIH->biWidth=cx;
    lpBMIH->biHeight=cy;
    
    LPBYTE data=((LPBYTE)lpBMIH)+lpBMIH->biSize;
    if (!rc)
    {
        memset(data,0,isize);
        for (int i=0; i<cluster->size.cy; i++)
        memcpy(data+i*cx,cluster->clstBM+i*cluster->size.cx,cluster->size.cx);
        //memcpy(data,cluster->clstBM,isize);
    }
    else
    {
        memset(data,0,isize);
        int offXsrc=(rc->left>cluster->offset.x)?(rc->left - cluster->offset.x):0;
        int offXdst=(rc->left<cluster->offset.x)?(cluster->offset.x - rc->left):0;
        int offYsrc=(rc->top>cluster->offset.y)?(rc->top - cluster->offset.y):0;
        int offYdst=(rc->top<cluster->offset.y)?(cluster->offset.y - rc->top):0;
        LPBYTE src=cluster->clstBM,dst=data;
        for (int i=offYdst; i<((cy>cluster->size.cy)?cluster->size.cy:cy); i++)
        {
            memcpy(dst+offXdst,src+offXsrc,(cx>cluster->size.cx)?cluster->size.cx:cx);
            src+=cluster->size.cx;
            dst+=cx;
        }
    }
    pTVFrame retVal=newTVFrame(lpBMIH,NULL);
    return retVal;
}

__forceinline void Cluster_DrawOverFrame(pCluster cluster, pTVFrame frame)
{
	LPBYTE Src = cluster->clstBM;
	LPBYTE SrcEnd = Src + cluster->size.cx * cluster->size.cy;
	LPBYTE Dst = GetData(frame) + cluster->offset.y * frame->lpBMIH->biWidth + cluster->offset.x;
	LPBYTE DstEnd = Dst + frame->lpBMIH->biWidth * frame->lpBMIH->biHeight - cluster->size.cx;
	while (Src < SrcEnd && Dst < DstEnd)
	{
		int i;
		for (i = 0; i < cluster->size.cx; i++)
			if (/*Src[i] != BLACK_COLOR*/(Src[i] >= cluster->colormin) && (Src[i] <= cluster->colormax))
				Dst[i] = Src[i];
//		memcpy(Dst, Src, cluster->size.cx);
		Src += cluster->size.cx;
		Dst += frame->lpBMIH->biWidth;
	}
}

__forceinline pTVFrame Cluster_KeepUndelClustersOnFrame(pCluster cluster, int length, pTVFrame orgFrame, BYTE bgColor = BLACK_COLOR)
{
	pTVFrame retFrame = makecopyTVFrame(orgFrame);
	memset(GetData(retFrame), bgColor, retFrame->lpBMIH->biWidth * retFrame->lpBMIH->biHeight);
	int i;
	for (i = 0; i < length; i++)
	{
		if (!cluster[i].deleted)
			Cluster_DrawOverFrame(&cluster[i], retFrame);
	}
	return retFrame;
}

__forceinline void Cluster_SimpleFilters(pClustersInfo pCI)
{
    for (int i=0; i<pCI->m_ClustersNmb; i++)
    {

        if (pCI->m_Clusters[i].area>CHAR_MAX_AREA) { Cluster_RemoveAt(pCI->m_Clusters,pCI->m_ClustersNmb,i); i--; continue; }
        if (pCI->m_Clusters[i].area<CHAR_MIN_AREA) { Cluster_RemoveAt(pCI->m_Clusters,pCI->m_ClustersNmb,i); i--; continue; }

		if (pCI->m_Clusters[i].size.cy>CHAR_MAX_HEIGHT) { Cluster_RemoveAt(pCI->m_Clusters,pCI->m_ClustersNmb,i); i--; continue; }
        if (pCI->m_Clusters[i].size.cy<CHAR_MIN_HEIGHT) { Cluster_RemoveAt(pCI->m_Clusters,pCI->m_ClustersNmb,i); i--; continue; }


        ASSERT(pCI->m_Clusters[i].size.cy!=0);
        double ratio=((double)pCI->m_Clusters[i].size.cx)/pCI->m_Clusters[i].size.cy;

	    if (ratio>CHAR_MAX_XYRATIO)				   { Cluster_RemoveAt(pCI->m_Clusters,pCI->m_ClustersNmb,i); i--; continue; }
        if (ratio<CHAR_MIN_XYRATIO)				   { Cluster_RemoveAt(pCI->m_Clusters,pCI->m_ClustersNmb,i); i--; continue; }
       
		ASSERT((pCI->m_Clusters[i].size.cy * pCI->m_Clusters[i].size.cx)!=0);
        double density=((double)(pCI->m_Clusters[i].area))/(pCI->m_Clusters[i].size.cy * pCI->m_Clusters[i].size.cx);

        if (density>CHAR_MAX_DENSITY)			   { Cluster_RemoveAt(pCI->m_Clusters,pCI->m_ClustersNmb,i); i--; continue; }
        if (density<CHAR_MIN_DENSITY)              { Cluster_RemoveAt(pCI->m_Clusters,pCI->m_ClustersNmb,i); i--;  }
	}
}

__forceinline void Cluster_CalcLineParam(pCluster cluster, LPLINEPARAMS lineparam)
{
  	if (cluster->area == 1)
	{
		lineparam->slope = lineparam->interlace = 0;
		lineparam->dev = lineparam->std = lineparam->rxy = 0;
		lineparam->x0 = (double) cluster->offset.x;
		lineparam->y0 = (double) cluster->offset.y;
        return;
	}

	DWORD size = cluster->size.cx * cluster->size.cy, i = 0;
	double x0 = 0, y0 = 0, Sxx = 0, Sxy = 0, Syy = 0;
	while (i < size)
	{
		if (cluster->clstBM[i] != ACTIVE_COLOR)
		{
			i++; continue;
		}
		double x = (double)(i % cluster->size.cx);
		double y = (double)(i / cluster->size.cx);
		x0 += x;
		y0 += y;
		Sxx += x * x;
		Sxy += x * y;
		Syy += y * y;
		i++;
	}
	x0 /= (double)cluster->area;
	y0 /= (double)cluster->area;
	Sxx -= (x0 * x0 * (double)cluster->area);
	Sxy -= (x0 * y0 * (double)cluster->area);
	Syy -= (y0 * y0 * (double)cluster->area);
	if ((Sxx == 0) || (Syy == 0))
	{
		lineparam->rxy = 1;
		if (Sxx == 0)
		{
			lineparam->slope = PI / 2.0;
			lineparam->interlace = x0+(double)cluster->offset.x;
			lineparam->std = 0;
			lineparam->dev = 0;
		}
		else
		{
			lineparam->slope = 0;
			lineparam->interlace = y0+(double)cluster->offset.y;
			lineparam->std = 0;
			lineparam->dev = 0;
		}
	}
	else
	{
		lineparam->rxy = Sxy / sqrt(Sxx * Syy);
		double kx = Sxy / Sxx, ky = Syy / Sxy, slopeX = atan(kx), slopeY = atan(ky);
		if (slopeX < 0)
			slopeX += PI;
		if (slopeY < 0)
			slopeY += PI;
		lineparam->slope = (slopeX * Sxx + slopeY * Syy) / (Sxx + Syy)/*2.*/;
		if (lineparam->slope == PI / 2.)
		{
			lineparam->interlace = x0;
			lineparam->std = 0;
			lineparam->dev = 0;
			double ny = 0, nx = 1;
			DWORD j = 0;
			while (j < size)
			{
				if (cluster->clstBM[j] != ACTIVE_COLOR)
				{
					j++; continue;
				}
				double dx = nx * (x0 - (double)(j % cluster->size.cx));
				double dy = ny * (y0 - (double)(j / cluster->size.cx));
				double d = (dx * dx + dy * dy);
				lineparam->std += d;
				lineparam->dev += sqrt(d);
				j++;
			}
			lineparam->std /= (double)(cluster->area - 1);
			lineparam->std = sqrt(lineparam->std);
			lineparam->dev /= (double)cluster->area;
		}
		else
		{
			double k = tan(lineparam->slope);
			lineparam->interlace = y0 + (double)cluster->offset.y
											-k * (x0 + (double)cluster->offset.x);
			lineparam->std = 0;
			lineparam->dev = 0;
			double ny = 1 / (1 + k * k), nx = -k * ny;
			DWORD j = 0;
			while (j < size)
			{
				if (cluster->clstBM[j] != ACTIVE_COLOR)
				{
					j++;
					continue;
				}
				double dx = nx * (x0 - (double)(j % cluster->size.cx));
				double dy = ny * (y0 - (double)(j / cluster->size.cx));
				double d = (dx * dx + dy * dy);
				lineparam->std += d;
				lineparam->dev += sqrt(d);
				j++;
			}
			lineparam->std /= (double)(cluster->area - 1);
			lineparam->std = sqrt(lineparam->std);
			lineparam->dev /= (double)cluster->area;
		}
	}
	lineparam->x0 = x0 + (double)cluster->offset.x;
	lineparam->y0 = y0 + (double)cluster->offset.y;
}

__forceinline void Cluster_DimsFilter(pCluster cluster, int nClusters, int minD, int maxD)
{
	int i;
	for (i = 0; i < nClusters; i++)
	{
		BOOL suits = (	(cluster[i].size.cx > minD && cluster[i].size.cx < maxD) ||
						(cluster[i].size.cy > minD && cluster[i].size.cy < maxD) );
		cluster[i].deleted = !suits;
	}
}

__forceinline void Cluster_Skeletize(pCluster c)
{
    _skeletize(c->clstBM, c->size.cx, c->size.cy);
}

__forceinline void Cluster_DelHoles(pCluster c)
{
    _delholes(c->clstBM, c->size.cx, c->size.cy);
}

typedef struct tagCurveParam
{
    POINT cm; // center mass
    POINT cc; // center of the cicle
    double radius;
    POINT* pnts; // all points of the cluster
    int    ce1,ce2,ce3; // pointers to significant points
}CURVEPARAM,*LPCURVEPARAM;

__forceinline LPCURVEPARAM newCurveParam()
{
    LPCURVEPARAM lpCP=(LPCURVEPARAM)malloc(sizeof(CURVEPARAM));
    memset(lpCP,0,sizeof(CURVEPARAM));
    lpCP->ce1=-1;
    lpCP->ce2=-1;
    lpCP->ce3=-1;
    return lpCP;
}

__forceinline void freeCurveParam(LPCURVEPARAM& lpCP)
{
    if (lpCP)
    {
        if (lpCP->pnts) free(lpCP->pnts);
        free(lpCP); lpCP=NULL;
    }
}

bool Cluster_DrawCurveParam(HDC hdc,RECT& rc, CDIBViewBase* view, LPVOID lParam);

__forceinline int Cluster_CountNeighbours(pCluster cluster,int x,int y)
{
    int cntr=0;
    int width =cluster->size.cx;
    int height=cluster->size.cy;
    LPBYTE d=cluster->clstBM+x+y*width;
    if ((x!=0) && (y!=0) && (*(d-1-width)==ACTIVE_COLOR))            cntr++;
    if ((x!=0) && (*(d-1)==ACTIVE_COLOR))                            cntr++;
    if ((y!=0) && (*(d-width)==ACTIVE_COLOR))                        cntr++;
    if ((x+1<width) && (y+1<height) && (*(d+1+width)==ACTIVE_COLOR)) cntr++;
    if ((x+1<width) && (*(d+1)==ACTIVE_COLOR))                       cntr++;
    if ((y+1<height) && (*(d+width)==ACTIVE_COLOR))                  cntr++;
    if ((x!=0) && (y+1<height) && (*(d-1+width)==ACTIVE_COLOR))      cntr++;
    if ((x+1<width) && (y!=0) && (*(d+1-width)==ACTIVE_COLOR))       cntr++;
    return cntr;
}



__forceinline double cos_angle(CPoint O, CPoint a, CPoint b)
{
    a-=O;
    b-=O;
    double sp=a.x*b.x+a.y*b.y;
    double la=sqrt(a.x*a.x+a.y*a.y);
    double lb=sqrt(b.x*b.x+b.y*b.y);
    if ((la!=0) && (lb!=0)) return (sp/(la*lb));
    return 2;
}

__forceinline int pnt_dist(CPoint a, CPoint b)
{
    a-=b;
    return((int)(sqrt(a.x*a.x+a.y*a.y)+0.5));
}

__forceinline bool vec_above(CPoint O, CPoint a, CPoint b)
{
    a-=O;
    b-=O;
    int resInv=false;
    if (a.x<0)
    {
        a.x=-a.x;
        a.y=-a.y;
    }
    if (b.x<0)
    {
        resInv=true;
        b.x=-b.x;
        b.y=-b.y;
    }
    double sa=(double)a.y/a.x;
    double sb=(double)b.y/b.x;
    if (resInv)
        return (sb<sa);
    else
        return (sb>sa);
}

__forceinline bool calc_ortparam(CPoint O,CPoint A, double &a, double &b)
{
    CPoint v=A-O;
    if (!v.y)
    {
        a=A.x; // Vertical line
        return false;
    }
    else
        a=(double)-v.x/v.y;
    b=A.y-a*A.x;
    return true;
}

__forceinline bool calc_circ3(CPoint O, CPoint A, CPoint B, POINT &cc, double &radius)
{
    double a1,a2,b1,b2;
    bool av,bv;
    av=calc_ortparam(O,A, a1, b1);
    bv=calc_ortparam(O,B, a2, b2);

    if (a1-a2==0) return false; // parallel lines
    
    CPoint oD;
    if (av && bv)
    {
        oD.x=(long)((b2-b1)/(a1-a2));
        oD.y=(long)(a1*oD.x+b1);
    }
    else if (av)
    {
        oD.x=(long)a1;
        oD.y=(long)(a2*oD.x+b2);
    }
    else
    {
        oD.x=(long)a2;
        oD.y=(long)(a1*oD.x+b1);
    }
    oD-=O;
    oD.x/=2;
    oD.y/=2;
    radius=sqrt(oD.x*oD.x+oD.y*oD.y);
    oD+=O;
    cc.x=oD.x;
    cc.y=oD.y;
    return true;
}


__forceinline void Cluster_SeekEdges(pCluster cluster, LPCURVEPARAM cp)
{
    if (cp->pnts) return; // data already collected

    int width=cluster->size.cx;
    int height=cluster->size.cy;
    int size=width*height;
    int i;

    cp->pnts=(POINT*)malloc(sizeof(POINT)*cluster->area);

    LPBYTE sc=cluster->clstBM;
    int cntr=0;
    int x=0,y=0;

    while (sc<cluster->clstBM+size)
    {
        if (*sc==ACTIVE_COLOR)
        {
            int off = (int)(sc - cluster->clstBM);
            
            ASSERT(cntr<cluster->area);

            cp->pnts[cntr].x=off%width;
            cp->pnts[cntr].y=off/width;
            x+=cp->pnts[cntr].x;
            y+=cp->pnts[cntr].y;
            int nb = Cluster_CountNeighbours(cluster,cp->pnts[cntr].x,cp->pnts[cntr].y);
            cntr++;
        }
        sc++;
    }

    if (cluster->area==1)
    {
        cp->cm.x=0;
        cp->cm.y=0;
        cp->ce1=0;
        cp->ce2=0;
        cp->ce3=0;
        return;
    }

    cp->cm.x=(int)((double)x/cntr+0.5);
    cp->cm.y=(int)((double)y/cntr+0.5);
    int mindist=10000;
    for (i=0; i<cluster->area; i++)
    {
        int dist=pnt_dist(cp->cm,cp->pnts[i]);
        if (dist<mindist)
        {
            cp->ce2=i;
            mindist=dist;
        }
    }
    int maxdist=0;
    for (i=0; i<cluster->area; i++)
    {
        int dist=pnt_dist(cp->pnts[cp->ce2],cp->pnts[i]);
        if (dist>maxdist)
        {
            cp->ce1=i;
            maxdist=dist;
        }
    }
    maxdist=0;
    for (i=0; i<cluster->area; i++)
    {
        int dist=pnt_dist(cp->pnts[cp->ce1],cp->pnts[i]);
        if (dist>maxdist)
        {
            cp->ce3=i;
            maxdist=dist;
        }
    }
}

__forceinline void Cluster_CalcCurveParam(pCluster cluster, LPCURVEPARAM cp)
{
    if (!cp->pnts) Cluster_SeekEdges(cluster, cp);

    ASSERT((cp->ce3>=0) && (cp->ce2>=0) && (cp->ce1>=0));

    calc_circ3(cp->pnts[cp->ce2], cp->pnts[cp->ce1], cp->pnts[cp->ce3], cp->cc, cp->radius);

/*    x=0, y=0,cntr=0;
    double radius=0;
    for (i=0; i<cluster->area; i++)
    {
        POINT tmp;
        double rtmp;
        if ((i==cp->ce1) || (i==cp->ce3)) continue;
        if (calc_circ3(cp->pnts[i], cp->pnts[cp->ce1], cp->pnts[cp->ce3], tmp, rtmp))
        {
            x+=tmp.x;
            y+=tmp.y;
            radius+=rtmp;
            cntr++;
        }
    }
    cp->cc.x=x/cntr;
    cp->cc.y=y/cntr;
    cp->radius=radius/cntr;
*/    
}

#define _iabs(a) (((a)<0)?-(a):(a))

__forceinline int _min4(int a1, int a2, int b1, int b2)
{
    if ((b1<=a1) && (a1<=b2)) return 0;
    if ((b1<=a2) && (a2<=b2)) return 0;
    if ((a1<=b1) && (b1<=a2)) return 0;
    if ((a1<=b2) && (b2<=a2)) return 0;

    int d11=_iabs(a1-b1), d12=_iabs(a1-b2), d21=_iabs(a2-b1), d22=_iabs(a2-b2);
    int min1=(d11<d12)?d11:d12;
    int min2=(d21<d22)?d21:d22;
    return ((min1<min2)?min1:min2);
}

__forceinline bool _calc_overlap_reg(int a1, int a2, int b1, int b2, long& c1, long& c2)
{
    c1=-1; c2=-1;

    if ((b1<=a1) && (a1<=b2)) 
    {
        if ((b1<=a2) && (a2<=b2)) 
        {
            c1=a1; c2=a2; return true;
        }
        c1=a1; c2=b2; return true;
    }
    if ((b1<=a2) && (a2<=b2)) 
    {
        c1=b1; c2=a2; return true;
    }

    if ((a1<=b1) && (b1<=a2))
    {
        if ((a1<=b2) && (b2<=a2)) 
        {
            c1=b1; c2=b2; return true;
        }
        c1=b1; c2=a2; return true;
    }
    if ((a1<=b2) && (b2<=a2)) 
    {
        c1=a1; c2=b2; return true;
    }
    return false;
}

__forceinline int Cluster_CalcClustersDistance(pCluster a, pCluster b, int maxdist)
{
    int bcX=_min4(a->offset.x,a->offset.x+a->size.cx,b->offset.x,b->offset.x+b->size.cx),
        bcY=_min4(a->offset.y,a->offset.y+a->size.cy,b->offset.y,b->offset.y+b->size.cy);
    if (bcX<0) bcX=0; if (bcY<0) bcY=0;
    int dist=(int)sqrt(bcX*bcX+bcY*bcY);
    if (dist>maxdist) return dist;
    
    RECT ovrRC;

    _calc_overlap_reg(a->offset.x,a->offset.x+a->size.cx,
                      b->offset.x,b->offset.x+b->size.cx,
                      ovrRC.left,ovrRC.right);

    _calc_overlap_reg(a->offset.y,a->offset.y+a->size.cy,
                      b->offset.y,b->offset.y+b->size.cy,
                      ovrRC.top,ovrRC.bottom);

    return dist;
}

__forceinline void Cluster_GlueClusters(pClustersInfo ci,int a, int b)
{
    Cluster ca,cb;
    memcpy(&ca,&ci->m_Clusters[a],sizeof(ca));
    memcpy(&cb,&ci->m_Clusters[b],sizeof(cb));
    
    memset(&ci->m_Clusters[a],0,sizeof(Cluster));
    memset(&ci->m_Clusters[b],0,sizeof(Cluster));

    ci->m_Clusters[b].deleted=true;

    int xl,xr,yb,yu;

    xl=(ca.offset.x<cb.offset.x)?ca.offset.x:cb.offset.x;
    yu=(ca.offset.y<cb.offset.y)?ca.offset.y:cb.offset.y;
    xr=(ca.offset.x+ca.size.cx>cb.offset.x+cb.size.cx)?ca.offset.x+ca.size.cx:cb.offset.x+cb.size.cx;
    yb=(ca.offset.y+ca.size.cy>cb.offset.y+cb.size.cy)?ca.offset.y+ca.size.cy:cb.offset.y+cb.size.cy;
    ci->m_Clusters[a].offset.x=xl;
    ci->m_Clusters[a].offset.y=yu;
    ci->m_Clusters[a].size.cx=xr-xl;
    ci->m_Clusters[a].size.cy=yb-yu;
    ci->m_Clusters[a].area=ca.area+cb.area;
    ci->m_Clusters[a].colormin=ci->m_Clusters[a].colormax=255;
    ci->m_Clusters[a].clstBM=(LPBYTE)malloc(ci->m_Clusters[a].size.cx*ci->m_Clusters[a].size.cy);
    memset(ci->m_Clusters[a].clstBM,0,ci->m_Clusters[a].size.cx*ci->m_Clusters[a].size.cy);

    int y;
    int offX=ca.offset.x - ci->m_Clusters[a].offset.x;
    int offY=ca.offset.y - ci->m_Clusters[a].offset.y;
    LPBYTE offD=ci->m_Clusters[a].clstBM+offY*ci->m_Clusters[a].size.cx+offX;
    LPBYTE sc=ca.clstBM;
    for (y=0; y<ca.size.cy; y++)
    {
        memcpy(offD,sc,ca.size.cx);
        offD+=ci->m_Clusters[a].size.cx;
        sc+=ca.size.cx;
    }

    offX=cb.offset.x - ci->m_Clusters[a].offset.x;
    offY=cb.offset.y - ci->m_Clusters[a].offset.y;
    offD=ci->m_Clusters[a].clstBM+offY*ci->m_Clusters[a].size.cx+offX;
    sc=cb.clstBM;
    for (y=0; y<cb.size.cy; y++)
    {
        memcpy(offD,sc,cb.size.cx);
        offD+=ci->m_Clusters[a].size.cx;
        sc+=cb.size.cx;
    }

    ClustersRemoveDeleted(ci);
    if (ca.clstBM) free(ca.clstBM); ca.clstBM=NULL;
    if (cb.clstBM) free(cb.clstBM); cb.clstBM=NULL;
}

__forceinline int _imin(int a, int b)
{
    return (a<b)?a:b;
}

__forceinline void Cluster_GlueNeighbourClusters(pClustersInfo ci, int maxdist=2)
{
    if (ci->m_ClustersNmb<2) return;
    int i;

    LPCURVEPARAM* cd=(LPCURVEPARAM*)malloc(ci->m_ClustersNmb*sizeof(LPCURVEPARAM));

    for(i=0; i<ci->m_ClustersNmb; i++)
    {
        cd[i] = newCurveParam();  
        Cluster_SeekEdges(&(ci->m_Clusters[i]), cd[i]);
    }

    for(i=0; i<ci->m_ClustersNmb-1; i++)
    {
        for (int j=i+1; j<ci->m_ClustersNmb; j++)
        {
            if (i==j) continue;
            CPoint a1=cd[i]->pnts[cd[i]->ce1]; a1+=ci->m_Clusters[i].offset;
            CPoint b1=cd[i]->pnts[cd[i]->ce3]; b1+=ci->m_Clusters[i].offset;
            CPoint a2=cd[j]->pnts[cd[j]->ce1]; a2+=ci->m_Clusters[j].offset;
            CPoint b2=cd[j]->pnts[cd[j]->ce3]; b2+=ci->m_Clusters[j].offset;

            int d1=pnt_dist(a1,a2), d2=pnt_dist(a1,b2), d3=pnt_dist(b1,a2), d4=pnt_dist(b1,b2);

            int dist=_imin(_imin(pnt_dist(a1,a2),pnt_dist(a1,b2)),_imin(pnt_dist(b1,a2),pnt_dist(b1,b2)));
            /*
            int dist=Cluster_CalcClustersDistance(&(ci->m_Clusters[i]), &(ci->m_Clusters[j]),maxdist); */
            if (dist<=maxdist+1)
            {
                TRACE("Clasters %d and %d have to be glued.Distance = %d\n",i,j,dist); 
                freeCurveParam(cd[i]); freeCurveParam(cd[j]);    
                memcpy(&cd[j],&cd[j+1],sizeof(LPCURVEPARAM)*(ci->m_ClustersNmb-j-1));
                Cluster_GlueClusters(ci,i,j); j=i; 
                cd[i] = newCurveParam();
                Cluster_SeekEdges(&(ci->m_Clusters[i]), cd[i]);
                continue;
            }
        }
    }
    for(i=0; i<ci->m_ClustersNmb; i++)
    {
        freeCurveParam(cd[i]);  
    }
    free(cd);
}

__forceinline void Cluster_FitCluster(pCluster pcl)
{
    LPBYTE scn=pcl->clstBM;
    CRect rc(0,0,
             pcl->size.cx,
             pcl->size.cy);
    int x,y,xMin=pcl->size.cx,xMax=0,yMin=pcl->size.cy, yMax=0;
    int na=0;

    for (y=rc.top; y<rc.bottom;y++)
    {
        for (x=rc.left; x<rc.right; x++)
        {
            if (*scn)
            {
                if (x>xMax) xMax=x;
                if (x<xMin) xMin=x;
                if (y>yMax) yMax=y;
                if (y<yMin) yMin=y;
                na++;
            }
            scn++;
        }
    }
    int nW=xMax-xMin+1,nH=yMax-yMin+1;
    pcl->area=na;
    if (na==0)
    {
        pcl->deleted=true;
        return;
    }

    if ((nW==pcl->size.cx) && (nH==pcl->size.cy)) return; // no re-fitting required
    int dX=xMin;
    int dY=yMin;
    LPBYTE src=pcl->clstBM+dX+dY*rc.right;
    LPBYTE dst=pcl->clstBM;
    LPBYTE src1=pcl->userdata1+dX+dY*rc.right;
    LPBYTE dst1=pcl->userdata1;
    LPBYTE src2=pcl->userdata2+dX+dY*rc.right;
    LPBYTE dst2=pcl->userdata2;

    for (y=0; y<nH; y++)
    {
        memmove(dst,src,nW);
        src+=rc.right;
        dst+=nW;
 
        memmove(dst1,src1,nW);
        src1+=rc.right;
        dst1+=nW;

        memmove(dst2,src2,nW);
        src2+=rc.right;
        dst2+=nW;
    }
    pcl->offset.x+=dX;
    pcl->offset.y+=dY;
    pcl->size.cx=nW;
    pcl->size.cy=nH;
}

__forceinline void Cluster_GetOrgImg(pTVFrame Frame,pCluster pcl)
{
    ASSERT(pcl->offset.x>=0);
    ASSERT(pcl->offset.y>=0);

    if ((pcl->size.cx%4) || (pcl->size.cy%4))
    {
        int ow=pcl->size.cx;
        int oh=pcl->size.cy;
        int offx=0,offy=0;
        pcl->size.cx+=(4-(pcl->size.cx%4));
        pcl->size.cy+=(4-(pcl->size.cy%4));
        if (pcl->size.cx+pcl->offset.x>=Frame->lpBMIH->biWidth)
        {
            pcl->offset.x-=Frame->lpBMIH->biWidth-pcl->size.cx-1;
            offx-=Frame->lpBMIH->biWidth-pcl->size.cx-1;
        }
        if (pcl->size.cy+pcl->offset.y>=Frame->lpBMIH->biHeight)
        {
            pcl->offset.y-=Frame->lpBMIH->biHeight-pcl->size.cy-1;
            offy-=Frame->lpBMIH->biHeight-pcl->size.cy-1;
        }
        LPBYTE newBM=(LPBYTE)malloc(pcl->size.cy*pcl->size.cx);
        memset(newBM,0,pcl->size.cy*pcl->size.cx);
        for (int y=0; y<oh; y++)
        {
            memcpy(newBM+pcl->size.cx*y,pcl->clstBM+y*ow,ow);
        }
        free(pcl->clstBM); pcl->clstBM=newBM;
    }
}


__forceinline void Cluster_GetOrgUV(pTVFrame Frame, pCluster pcl)
{
    CRect rc(pcl->offset.x,pcl->offset.y,
             pcl->offset.x+pcl->size.cx,
             pcl->offset.y+pcl->size.cy);
    int uvW=pcl->size.cx/4;
    int uvH=pcl->size.cy/4;

    if(pcl->clstU!=NULL) free(pcl->clstU);
    if(pcl->clstV!=NULL) free(pcl->clstV);

    LPBYTE U=(LPBYTE)malloc(uvW*uvH);
    LPBYTE V=(LPBYTE)malloc(uvW*uvH);
    pcl->clstU=U;
    pcl->clstV=V;

    LPBYTE sU=U;
    LPBYTE sV=V;
    LPBYTE scn=pcl->clstBM;
    int uvOff=(rc.top/4)*(Frame->lpBMIH->biWidth/4)+rc.left/4;
    //while (uvOff%4) { uvOff++; }
    LPBYTE oData=GetData(Frame)+rc.top*Frame->lpBMIH->biWidth+rc.left;
    LPBYTE uvData=GetData(Frame)+Frame->lpBMIH->biHeight*Frame->lpBMIH->biWidth;
    LPBYTE uData=uvData+uvOff;
    LPBYTE vData=uData
            +(Frame->lpBMIH->biWidth*Frame->lpBMIH->biHeight)/16;
    for (int y=rc.top; y<rc.bottom;y++)
    {
        memcpy(scn,oData,pcl->size.cx);
        oData+=Frame->lpBMIH->biWidth;
        if ((y%4)==0)
        {
           memcpy(sU,uData,uvW); 
           memcpy(sV,vData,uvW); 

           sU+=uvW; 
           uData+=(Frame->lpBMIH->biWidth/4);
           sV+=uvW; 
           vData+=(Frame->lpBMIH->biWidth/4);
        }
        scn+=pcl->size.cx;
    }
}

__forceinline void Cluster_GetSubPixelMassCenter(pCluster cluster, int colormin, int colormax, double& xc, double& yc)
{
	xc = yc = 0;
	double weight = 0;
	for (int y = 0; y < cluster->size.cy; y++)
	{
		for (int x = 0; x < cluster->size.cx; x++)
		{
			int color = (int)(cluster->clstBM[y * cluster->size.cx + x]);
			if (color < colormin || color > colormax)
				continue;
			int w = 1;
			if (colormin <= 0)
				w = 255 - color;
			else if (colormax >= 255)
				w = color;
			xc += (double)(w * x);
			yc += (double)(w * y);
			weight += (double)w;
		}
	}
	if (weight > 0)
	{
		xc /= weight;
		yc /= weight;
	}
    xc += (double)cluster->offset.x;
    yc += (double)cluster->offset.y;
}


#endif