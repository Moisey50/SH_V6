#include "stdafx.h"
#include <math.h>
#include "LineDetector.h"
#include <imageproc\fstfilter.h>
#include <imageproc\edgefilters.h>
#include <imageproc\featuredetector.h>

#define LNDETECTOR_CLUSTERS_GROWTH   500
#define LNDETECTOR_DEF_SCANSTEP 1
#define LNDETECTOR_DEF_MAGNITUDE_THRESOLD 10
#define LNDETECTOR_DEF_MIN_LINE_LENGTH    10

#define CTG_PI_1_8 2.4142135623730950488016887242097
// good approximation is 2378/985 - |CTG_PI_1_8-2378/985|<1e-6
#define CTG_PI_3_8 0.4142135623730950488016887242097
// good approximation is 408/985 - |CTG_PI_3_8-408/985|<1e-6
#define CTG_PI_5_8 -0.4142135623730950488016887242097
#define CTG_PI_7_8 -2.4142135623730950488016887242097

__forceinline char dir2ch(int dir)
{
    switch (dir)
    {
    case BoundaryDir01_30:
        return '\\';
    case BoundaryDir03_00:
        return '|';
    case BoundaryDir04_30:
        return '/';
    case BoundaryDir00_00:
        return '-';
    default:
        return '?';
    }
}

#define sign(a) ((a<0)?-1:1)

CLineDetector::CLineDetector():
    m_ScanStep(LNDETECTOR_DEF_SCANSTEP),
    m_MagnitudeThreshold(LNDETECTOR_DEF_MAGNITUDE_THRESOLD),
    m_pMaxMap(NULL),
    m_pMagnitude(NULL),
    m_pDirections(NULL),
    m_BorderOffset(3)
{
}

CLineDetector::~CLineDetector()
{
    if (m_pMagnitude)
    {
        free(m_pMagnitude);
        m_pMagnitude=NULL;
        m_pMaxMap=NULL;
        m_pDirections=NULL;
    }
}


__forceinline bool __markMaximums3x3(long width, long depth, idata* src, idata* dst)
{
    //memset(dst,0,width*depth*sizeof(idata));
    idata* ss=src+width+1;
    idata* sd=dst+width+1;
    idata* se=src+width*(depth-1)-1;
    while (ss<se)
    {
		if ((*(ss-1)>*ss) || (*(ss+1)>*ss) || 
			(*(ss-width)>*ss) || (*(ss-width-1)>*ss) || (*(ss-width+1)>*ss) || 
			(*(ss+width)>*ss) || (*(ss+width-1)>*ss) || (*(ss+width+1)>*ss))
		{
		        *sd=1;
		}
		else
		{
			*sd=*ss;
		}
        sd++; ss++;
    }
	return true;
}

__forceinline int hypot(int _X, int _Y)
{
	//return sqrt(x*x+y*y);
	return abs(_X)+abs(_Y);
}

void CLineDetector::EstimateBoundaries()
{
    LPBYTE src, eod;                  
    long i;
	idata v00, v10, v20, v01, v11, v21, v02, v12, v22;
	idata i3_00, i0_00;

	idata i0_00m, i3_00m1, i3_00m2;

	DWORD t1;
	//TRACE("EstimateBoundaries: Start %d\n", t1=GetTickCount());

    if (m_pMagnitude) free(m_pMagnitude);

    m_pMagnitude=(int*)malloc(3*m_WrkDataSize*sizeof(idata));
	ASSERT(m_pMagnitude!=NULL);
    memset(m_pMagnitude,0,3*m_WrkDataSize*sizeof(idata));
    m_pMaxMap=m_pMagnitude+m_WrkDataSize;
	m_pDirections=m_pMagnitude+2*m_WrkDataSize;
    i=m_Width+1; 

	//TRACE("EstimateBoundaries: After Init %d\n", GetTickCount()-t1);

    src=m_WrkData+m_Width+1;
    eod=m_WrkData+(m_Height-1)*m_Width-1;

    // Weight pixel value by appropriate coefficient in convolution matrix 

	v01 = *(src - m_Width - 1); v02 = *(src - m_Width);
	v11 = *(src - 1);           v12=*src;
	v21 = *(src + m_Width - 1); v22 = *(src + m_Width);
    while (src<eod)
    {
		v00=v01; v01=v02; v02 = *(src - m_Width + 1);
		v10=v11; v11=v12; v12 = *(src + 1);
		v20=v21; v21=v22; v22 = *(src + m_Width + 1);


        i3_00 =2*(v12-v10)+v02+v22-v00-v20;
        i0_00 =2*(v01-v21)+v00+v02-v20-v22;

		*(m_pMagnitude+i)=abs(i3_00)+abs(i0_00);

		if (*(m_pMagnitude+i)>=m_MagnitudeThreshold)
		{
		
			*(m_pDirections+i)=BoundaryDir00_00;

			//due to approximation of CTG(M_PI/8)=sqrt(2)+1 and CTG(3*M_PI/8)=sqrt(2)-1
			i0_00m=985*i0_00;
			i3_00m1=2378*i3_00;
			i3_00m2=408*i3_00;
			if (i3_00>=0)
			{
				if (i0_00m>=i3_00m1)
				{
					*(m_pDirections+i)=BoundaryDir00_00;
				}
				else if (i0_00m>=i3_00m2)
				{
					*(m_pDirections+i)=BoundaryDir01_30;
				}
				else if (i0_00m>=-i3_00m2)
				{
					*(m_pDirections+i)=BoundaryDir03_00;
				}
				else if (i0_00m>=-i3_00m1)
				{
					*(m_pDirections+i)=BoundaryDir04_30;
				}
			}
			else //i3_00<0
			{
				if (i0_00m<=i3_00m1)
				{
					*(m_pDirections+i)=BoundaryDir00_00;
				}
				else if (i0_00m<=i3_00m2)
				{
					*(m_pDirections+i)=BoundaryDir01_30;
				}
				else if (i0_00m<=-i3_00m2)
				{
					*(m_pDirections+i)=BoundaryDir03_00;
				}
				else if (i0_00m<=-i3_00m1)
				{
					*(m_pDirections+i)=BoundaryDir04_30;
				}
			}
		}
        src++;i++;
    }

	//TRACE("EstimateBoundaries: After Cycle %d\n", GetTickCount()-t1);

    __markMaximums3x3(m_Width, m_Height, m_pMagnitude, m_pMaxMap); //3ms
	//memcpy(m_pMaxMap, m_pMagnitude, m_WrkDataSize*sizeof(idata));
	//TRACE("EstimateBoundaries: After MarkMax %d\n", GetTickCount()-t1);
}

void CLineDetector::TrailEdge(BoundaryEdge& be, int forward)
{
    int dx,dy;
    int x=be.x,y=be.y;
//    if (m_pMaxMap[y*m_Width+x]<2) return;
    EdgeDesc* pCurEdge;

    if(forward<0)
    {
        pCurEdge =&m_EdgeL;
    }
    else
    {
        pCurEdge =&m_EdgeR;
    }

    int sdir=be.scanDir;
    pCurEdge->minX=x; pCurEdge->minY=y; pCurEdge->maxX=x; pCurEdge->maxY=y; // bound rectangle
	pCurEdge->pntsNumber=0;
	pCurEdge->aintensity=0;

    BoundaryEdge beTmp=be;
     
	switch (sdir)
	{
	case BoundaryDir00_00: //BoundaryDir00_00
		dx=forward;
		dy=0;
		break;
	case BoundaryDir04_30: //BoundaryDir04_30
		dx=-forward;
		dy=forward;
		break;
	case BoundaryDir03_00: //BoundaryDir03_00
		dx=0;
		dy=forward;
		break;
	case BoundaryDir01_30: //BoundaryDir01_30
		dx=forward;
		dy=forward;
		break;
	default:
		return;
	}

    while(beTmp.isEdge /*&& (x>m_BorderOffset-1) && (x<m_Width-m_BorderOffset) && (y>m_BorderOffset-1) && (y<m_Height-m_BorderOffset)*/
				&& pCurEdge->pntsNumber<m_maxLength ) 
    {
          x=beTmp.x;
          y=beTmp.y;

          pCurEdge->pCurData[pCurEdge->pntsNumber].x=x;
          pCurEdge->pCurData[pCurEdge->pntsNumber].y=y;
          pCurEdge->pntsNumber++;

          pCurEdge->aintensity+=beTmp.magnitude;
          if (pCurEdge->minX>x) pCurEdge->minX=x;
          if (pCurEdge->maxX<x) pCurEdge->maxX=x;
          if (pCurEdge->minY>y) pCurEdge->minY=y;
          if (pCurEdge->maxY<y) pCurEdge->maxY=y;
          
          if (!SeekNextStep(beTmp, dx,dy)) 
              break;
    }
    if (pCurEdge->pntsNumber < 1)
      return;
    pCurEdge->aintensity/=pCurEdge->pntsNumber;
    //ASSERT(pCurEdge->aintensity<256);
    for(int i=1; i<pCurEdge->pntsNumber; i++)
    {
        m_pMaxMap[pCurEdge->pCurData[i].y*m_Width+pCurEdge->pCurData[i].x]=0;
    }

}

bool CLineDetector::SeekNextStep(BoundaryEdge& be, int dx, int dy)
{
    BoundaryEdge beNext, beNextL, beNextR;
    int sdir=be.scanDir;
    int lastmag=be.magnitude;
    beNext=be;
    beNext.x+=dx;
    beNext.y+=dy;
    beNextL=beNextR=be;

	switch (sdir)
	{
	case BoundaryDir00_00: //BoundaryDir00_00
		beNextL.y-=1;
		beNextR.y+=1;
		beNextL.x+=dx;
		beNextR.x+=dx;
		dy=0;
		break;
	case BoundaryDir04_30: //BoundaryDir04_30
		beNextL.x-=dx;
		beNextR.y+=dy;
		break;
	case BoundaryDir03_00: //BoundaryDir03_00
		beNextL.x-=1;
		beNextR.x+=1;
		beNextL.y+=dy;
		beNextR.y+=dy;
		break;
	case BoundaryDir01_30: //BoundaryDir01_30
		beNextL.x-=dy;
		beNextR.y+=dx;
		break;
	default:
		be.isEdge=false;
		return false;
	}

    GetMagnitude(beNext); GetMagnitude(beNextL); GetMagnitude(beNextR);

    if  (beNext.edgeDir!=sdir) beNext.magnitude=0;
    if  (beNextL.edgeDir!=sdir) beNextL.magnitude=0;
	if  (beNextR.edgeDir!=sdir) beNextR.magnitude=0;

    if (beNext.magnitude<beNextL.magnitude)
    {
        if (beNextL.magnitude<beNextR.magnitude)
            be=beNextR;
        else
            be=beNextL;
    }
    else if (beNext.magnitude<beNextR.magnitude)
    {
        if (beNextL.magnitude<beNextR.magnitude)
            be=beNextR;
        else
            be=beNextL;
    }
    else
    {
        be=beNext;
    }
    if ((be.magnitude > m_MagnitudeThreshold) && (3*lastmag/4<be.magnitude))
    {
        be.isEdge=true;
    }
    else 
        be.isEdge=false;
    return be.isEdge;
}

void CLineDetector::SeekClusterAt(int x, int y)
{
    BoundaryEdge Edge(x,y);
    GetMagnitude(Edge); 
    Edge.isEdge=true;
    Edge.scanDir=Edge.edgeDir;

    TrailEdge(Edge,-1);
    TrailEdge(Edge,1);
    Add();
}

#include <stdio.h>

bool CLineDetector::SeekClusters()
{
   int i, x, y;

   DWORD t1;

   //TRACE("qqq Start: %d\n", t1=GetTickCount());
   m_maxLength=2*(abs(m_Width)+abs(m_Height));

   m_EdgeL.pCurData=(PPOINT)malloc(m_maxLength*sizeof(POINT));
   m_EdgeL.pntsNumber=0;
   m_EdgeL.minX=m_EdgeL.maxX=m_EdgeL.minY=m_EdgeL.maxY=0;
   m_EdgeL.aintensity=0;

   m_EdgeR.pCurData=(PPOINT)malloc(m_maxLength*sizeof(POINT));
   m_EdgeR.pntsNumber=0;
   m_EdgeR.minX=m_EdgeL.maxX=m_EdgeL.minY=m_EdgeL.maxY=0;
   m_EdgeR.aintensity=0;

   //TRACE("qqq After init: %d\n", GetTickCount()-t1);

   EstimateBoundaries();

   //TRACE("qqq After EsimateBoundaries: %d\n", GetTickCount()-t1);

   BoundaryEdge Edge;
   for(y=m_BorderOffset; y<m_Height-m_BorderOffset-1; y+=m_ScanStep)
      for (x=m_BorderOffset; x<m_Width-m_BorderOffset-1; x+=m_ScanStep)
	  {
		  if (m_pMaxMap[y*m_Width+x]>=2*LNDETECTOR_DEF_MAGNITUDE_THRESOLD) 
		  {
			  //SeekClusterAt(x,y) function replaced for performance
			  Edge.edgeDir=Edge.scanDir=100;
			  Edge.x=x; Edge.y=y;
			  Edge.xFrac=Edge.yFrac=0.0;
			  GetMagnitude(Edge); 
				
			  if (Edge.edgeDir==0)
				  continue;

			  Edge.isEdge=true;
			  Edge.scanDir=Edge.edgeDir;

			  TrailEdge(Edge,-1);
			  TrailEdge(Edge,1);
			  Add();
		  }
			
	  }

   //TRACE("qqq After SeekClusterAt: %d\n", GetTickCount()-t1);

   free(m_EdgeR.pCurData); m_EdgeR.pCurData=NULL;
   free(m_EdgeL.pCurData); m_EdgeL.pCurData=NULL;

   //TRACE("qqq After free: %d\n", GetTickCount()-t1);

   LPBYTE ptr1=GetData(m_OrgFrame);
   idata *ptr2=m_pMagnitude;
   for (i=0; (unsigned)i<m_WrkDataSize; i++)
   {
	   *(ptr1+i)=*(ptr2+i)/5;
   }

   //TRACE("qqq After evaluate: %d\n", GetTickCount()-t1);

//   SeekDuplicateLines(GetClusterInfo());
   LineParams(GetClusterInfo());

   //TRACE("qqq After LineParams: %d\n", GetTickCount()-t1);

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

__forceinline int calc_distance(Cluster *a, int minX, int minY, int SizeX, int SizeY)
{
	return 2*(abs(a->offset.x-minX)+abs(a->offset.y-minY))+abs(a->size.cx-SizeX)+abs(a->size.cy-SizeY);
}

void CLineDetector::Add()
{
    if ((m_EdgeL.pntsNumber +m_EdgeR.pntsNumber) < LNDETECTOR_DEF_MIN_LINE_LENGTH)
      return;
    
    int pntsNumber=m_EdgeL.pntsNumber +m_EdgeR.pntsNumber-1;
    int aintensity=(m_EdgeL.aintensity+m_EdgeR.aintensity)/2;
    
    ASSERT(aintensity>=LNDETECTOR_DEF_MAGNITUDE_THRESOLD);

	int sizeX, sizeY;
	long datasize, i;
    int minX=min(m_EdgeL.minX,m_EdgeR.minX);
    int minY=min(m_EdgeL.minY,m_EdgeR.minY);
    int maxX=max(m_EdgeL.maxX,m_EdgeR.maxX);
    int maxY=max(m_EdgeL.maxY,m_EdgeR.maxY);
	sizeX=maxX-minX+1;
	sizeY=maxY-minY+1;

	//check for duplicates!!!
	for (i=0; i<m_ClustersNmb; i++)
	{
		if (calc_distance(&(m_Clusters[i]), minX, minY, sizeX, sizeY)<8)
		{
			return;
		}
	}

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
        memset(&(m_Clusters[m_ClustersNmb]),0,sizeof(Cluster));
        m_Clusters[m_ClustersNmb].offset.x=minX;
        m_Clusters[m_ClustersNmb].offset.y=minY;
        m_Clusters[m_ClustersNmb].size.cx=sizeX;
        m_Clusters[m_ClustersNmb].size.cy=sizeY;
        m_Clusters[m_ClustersNmb].color=aintensity;
        m_Clusters[m_ClustersNmb].colormin=0;
        m_Clusters[m_ClustersNmb].colormax=0; 
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
        memrcpyPOINTS((PPOINT)m_Clusters[m_ClustersNmb].userdata1, m_EdgeL.pCurData, m_EdgeL.pntsNumber); 
        memcpy(m_Clusters[m_ClustersNmb].userdata1+m_EdgeL.pntsNumber*sizeof(POINT), m_EdgeR.pCurData+1, (m_EdgeR.pntsNumber-1)*sizeof(POINT)); 
        // Draw sample cluster
        for(i=0; i<pntsNumber; i++)
            DrawPixel( ((PPOINT)(m_Clusters[m_ClustersNmb].userdata1)+i)->x - minX, ((PPOINT)(m_Clusters[m_ClustersNmb].userdata1)+i)->y - minY, m_Clusters[m_ClustersNmb].clstBM, sizeX );
    }
    m_ClustersNmb++;
}

void CLineDetector::DrawPixel(int x, int y, LPBYTE Data, int width)
{
   ASSERT(x>=0&&y>=0&&x<width);
   //ASSERT(*(Data+x+width*y)!=255);
   /*if (*(Data+x+width*y)==0)
       *(Data+x+width*y)=255;
   else
       *(Data+x+width*y)=128; */
   *(Data+x+width*y)=128;
}

void LineParams(pClustersInfo pCI)
{
    int i;
    int strokesallocated=1024;
    pstroke strokes = (pstroke)malloc(strokesallocated*sizeof(stroke));
    for (i=0; i<pCI->m_ClustersNmb; i++)
    {
        int strokesnmb=-1;
        int dx,dy;
        int lastdir=StrokeDirNoDir,dir=StrokeDirNoDir;
        PPOINT points=(PPOINT)pCI->m_Clusters[i].userdata1; 
        for (int j=1; j<pCI->m_Clusters[i].area; j++)
        {
            dx=points[j].x-points[j-1].x;
            dy=points[j].y-points[j-1].y;
            ASSERT(dx<2); ASSERT(dy<2);ASSERT(dx>-2); ASSERT(dy>-2);
            dir = (dx==0)?StrokeDir00_00:(dy==0)?StrokeDir03_00:(sign(dx)==sign(dy))?StrokeDir04_30:StrokeDir01_30;
            if (dir!=lastdir) 
            {
                strokesnmb++;
                strokes[strokesnmb].type=dir;
                strokes[strokesnmb].length=0;
                strokes[strokesnmb].start=points[j-1];
                lastdir=dir;
            }
            strokes[strokesnmb].length++;
        }
        pCI->m_Clusters[i].userdata3=(LPBYTE)malloc((strokesnmb+1)*sizeof(stroke)); 
        memcpy(pCI->m_Clusters[i].userdata3,strokes,(strokesnmb+1)*sizeof(stroke));
        pCI->m_Clusters[i].int3=strokesnmb+1;
    }
    free(strokes);

//	   DWORD tS=GetTickCount();
//     for (i=0; i<pCI->m_ClustersNmb; i++)
//     {
//         if (!pCI->m_Clusters[i].userdata2) pCI->m_Clusters[i].userdata2=(LPBYTE)malloc(sizeof(lineparam));
//         plineparam plp=(plineparam)pCI->m_Clusters[i].userdata2;
//         plp->meanX=plp->meanY=0; 
//         plp->meandx= plp->meandy=plp->meanddx=plp->meanddy=0;
//         plp->isline=false;
//         plp->vertical=(pCI->m_Clusters[i].size.cy>pCI->m_Clusters[i].size.cx);
//         PPOINT points=(PPOINT)pCI->m_Clusters[i].userdata1; 
//         
//         TRACE("+++ +++ +++\n\nCluster %d\n",i);
//         if (plp->vertical)
//         {
//             for (int j=0; j<pCI->m_Clusters[i].area; j++)
//             {
//                 TRACE("\n");
//                 plp->meanX+=points[j].y;
//                 plp->meanY+=points[j].x;
//                 TRACE("%d\t%d\t%d\t",j,points[j].x,points[j].y);
//                 if (j==0) continue;
//                 plp->meandx+=points[j].x-points[j-1].x;
//                 plp->meandy+=points[j].y-points[j-1].y;
//                 TRACE("%d\t%d\t",points[j].x-points[j-1].x,points[j].y-points[j-1].y);
//                 if (j>1)
//                 {
//                     plp->meanddx+=2*points[j-1].x-points[j-2].x-points[j].x;
//                     plp->meanddy+=2*points[j-1].y-points[j-2].y-points[j].y;
//                     TRACE("%d\t%d",2*points[j-1].x-points[j-2].x-points[j].x,2*points[j-1].y-points[j-2].y-points[j].y);
//                 }
//             }
//         }
//         else
//         {
//             for (int j=0; j<pCI->m_Clusters[i].area; j++)
//             {
//                 TRACE("\n");
//                 plp->meanX+=points[j].x;
//                 plp->meanY+=points[j].y;
//                 TRACE("%d\t%d\t%d\t",j,points[j].x,points[j].y);
//                 if (j==0) continue;
//                 plp->meandx+=points[j].x-points[j-1].x;
//                 plp->meandy+=points[j].y-points[j-1].y;
//                 TRACE("%d\t%d\t",points[j].x-points[j-1].x,points[j].y-points[j-1].y);
//                 if (j>1)
//                 {
//                     plp->meanddx+=2*points[j-1].x-points[j-2].x-points[j].x;
//                     plp->meanddy+=2*points[j-1].y-points[j-2].y-points[j].y;
//                     TRACE("%d\t%d",2*points[j-1].x-points[j-2].x-points[j].x,2*points[j-1].y-points[j-2].y-points[j].y);
//                 }
//             }
//         }
//         
//         TRACE("\n+++ +++ +++\n");
//         
//         plp->meanX/=pCI->m_Clusters[i].area;
//         plp->meanY/=pCI->m_Clusters[i].area;
//         plp->meandx/=pCI->m_Clusters[i].area-1;
//         plp->meandy/=pCI->m_Clusters[i].area-1;
//         plp->meanddx/=pCI->m_Clusters[i].area-2;
//         plp->meanddy/=pCI->m_Clusters[i].area-2;
// 
//         double na,nb;
//         if (plp->vertical)
//         {
//             na=plp->meandx/plp->meandy;
//             nb=plp->meanY - na*plp->meanX;
//         }
//         else
//         {
//             na=plp->meandy/plp->meandx;
//             nb=plp->meanY - na*plp->meanX;
//         }
//         plp->isline=((plp->meanddx==0) && (plp->meanddy==0)); ///(plp->db<10);
//         plp->a=na;
//         plp->b=nb;
//         TRACE("+++ Cluster %d a= %f, b= %f\n",i, plp->a, plp->b); 
//     }
//     TRACE("+++ LineParams takes %d\n",GetTickCount()-tS);
}

__forceinline int calc_distance(Cluster *a, Cluster *b)
{
	return 2*(abs(a->offset.x-b->offset.x)+abs(a->offset.y-b->offset.y))+abs(a->size.cx-b->size.cx)+abs(a->size.cy-b->size.cy);
}

void SeekDuplicateLines(pClustersInfo pCI)
{
    int i,j;
    DWORD tS=GetTickCount();
    for (i=0; i<pCI->m_ClustersNmb; i++)
    {
        if (pCI->m_Clusters[i].deleted) continue;
        for (j=i+1; j<pCI->m_ClustersNmb; j++)
        {
            if (pCI->m_Clusters[j].deleted) continue;
            if (calc_distance(&(pCI->m_Clusters[j]), &(pCI->m_Clusters[i]))<8)
            {
                if (pCI->m_Clusters[j].color>pCI->m_Clusters[i].color)
                {
                    pCI->m_Clusters[i].deleted=true;
                    break;
                }
                else
                    pCI->m_Clusters[j].deleted=true;
            }
        }
    }
    //TRACE("+++ %d\n",GetTickCount()-tS);
}
