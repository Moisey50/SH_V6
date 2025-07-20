//  $File : Clusters.cpp: implementation of the CClusters class.
//  (C) Copyright The File X Ltd 2002. 
//
//
//
//  Revision History (excluding minor changes for specific compilers)
//   13 May 02 Firts release version, all followed changes must be listed below  (Andrey Chernushich)

#include "stdafx.h"
#include <imageproc\Clusters\Clusters.h>
#include <imageproc\DIBUtils.h>
#include <imageproc\simpleip.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define MAXREC_LEVEL 10000
#define MIN_CLUSTER_BOXAREA 4
// The data will be reallocated with quant value of clusters_growth
#define clusters_growth 20

///////////////////////////////////////////////////////////////////////////////
// Private static functions

// Just replace the color which will be used as inactive color with another color
inline void _replace_inactivecolor(LPBYTE data, DWORD size)
{
    LPBYTE end=data+size;
    while (data<end)
    {
        if ((*(data))==INACTIVE_COLOR) *(data)=INACTIVE_REPLACE_COLOR;
        data++;
    }
}

inline void _clusters_reset(pCluster c, int& length)
{
    if (!length) return;
    for (int i=0; i<length; i++)
    {
        if (c[i].clstBM) free(c[i].clstBM); c[i].clstBM=NULL;
        if (c[i].clstU) free(c[i].clstU); c[i].clstU=NULL;
        if (c[i].clstV) free(c[i].clstV); c[i].clstV=NULL;
        if (c[i].userdata1) free(c[i].userdata1); c[i].userdata1=NULL;
        if (c[i].userdata2) free(c[i].userdata2); c[i].userdata2=NULL;
        if (c[i].userdata3) free(c[i].userdata3); c[i].userdata3=NULL;
    }
    length=0;
}

///////////////////////////////////////////////////////////////////////////////
// All below is internal CClusters class _hlines. It stores information about
// cluster as a set of hlines found at scaning process..

typedef struct tag_hline
{
    int xl,xr,y;
}_hline;

// array of hlines will grow on this value everytime it will be required
#define _hlines_grows 10

class _hlines
{
private:
    _hline *m_phlines;
    int     nmb_allocated;
    int     nmb_stored;
public:
// construction and destruction
    _hlines():nmb_allocated(0)
    { 
        m_phlines=(_hline*)malloc(sizeof(_hline)*_hlines_grows); 
        if (m_phlines) nmb_allocated=_hlines_grows; 
        nmb_stored=0; 
    };
    ~_hlines() { if (m_phlines) free(m_phlines); }
// methods
    void reset()    { nmb_stored=0; };
    int get_hlinesnmb() { return nmb_stored; };
    _hline * get_at(int pos) { return &m_phlines[pos]; };
    void add(int xl,int xr,int y)
    {
        if (nmb_stored==nmb_allocated)
        {
            m_phlines=(_hline*)realloc(m_phlines,sizeof(_hline)*(nmb_allocated+_hlines_grows)); 
            ASSERT(m_phlines!=NULL);
            if (m_phlines)
            {
                nmb_allocated+=_hlines_grows;
            }
        }
        if (m_phlines)
        {
            m_phlines[nmb_stored].xl=xl;
            m_phlines[nmb_stored].xr=xr;
            m_phlines[nmb_stored].y=y;
            nmb_stored++;
        }
    }
};

// structure used at time of image parsing
//
typedef struct _tagstroke
{
    int xl,xr, iy;
}stroke,*pstroke;

class strokestack
{
    int         isizeallocated;
    int         isize;
    pstroke     pdata;
public:
    strokestack(int size=MAXREC_LEVEL)
    {
        pdata=(pstroke)malloc(sizeof(stroke)*size);
        isize=0;
        isizeallocated=size;
    }
    ~strokestack()
    {
        if (pdata) free(pdata); pdata=0;
        isize=0;
        isizeallocated=0;
    }
    bool push(int xl,int xr,int iy)
    {
        if (isize<isizeallocated)
        {
            pdata[isize].xl=xl;
            pdata[isize].xr=xr;
            pdata[isize].iy=iy;
            isize++;
            return true;
        }
        TRACE("!!!! StrockStack overflow !!!!!\n");
        return false;
    }
    bool pop(int& xl, int& xr, int& iy)
    {
        if (isize>0)
        {
            isize--;
            xl=pdata[isize].xl;
            xr=pdata[isize].xr;
            iy=pdata[isize].iy;
            return true;
        }
        return false;
    }
    int getsize() { return isize; }
};

typedef struct tagClusterWrkData
{
    LPBYTE   ibits;      // pointer to data
    int      width;      // width of the image
    int      height;     // height of the image
    unsigned color;
    int      colormin;   // cluster minimum color greylevel to seek
    int      colormax;   // cluster maximum color greylevel to seek
    RECT     rc;         // rectangle, bounded cluster
    _hlines  hlines;
    strokestack stack;
}ClusterWrkData;

// someinlines

__forceinline bool _match_color(int color, int min, int max)
{
    if (min!=max)
        return ((color >= min) && (color <= max));
    else
        return (color==max);
}

__forceinline int _scanleft(int & x, LPBYTE data, int ColorMin, int ColorMax, int ColorInactive)
{
    LPBYTE pntr=data+x;
    int avg=0;
    do
    {
        avg+=*(pntr);
        *(pntr)=ColorInactive;
        pntr--;
    }while ((pntr>=data) && _match_color(*(pntr),ColorMin,ColorMax));
    x=pntr-data+1;
    return avg;
}

__forceinline int _scanright(int & x, LPBYTE data, int ColorMin, int ColorMax, int ColorInactive, int maxX)
{
    LPBYTE pntr=data+x;
    LPBYTE pntrE=data+maxX;
    int avg=0;
    avg-=*(pntr);
    do
    {
        avg+=*(pntr);
        *(pntr)=ColorInactive;
        pntr++;
    }while ((pntr<pntrE) && _match_color(*(pntr),ColorMin,ColorMax));
    x=pntr-data-1;
    return avg;
}

__forceinline bool _check_stroke(ClusterWrkData* data, int ColorInactive)
{
    int xl, xr, iy;

    data->stack.pop(xl, xr, iy);
    
    if (iy<0) return false;
    if (iy>=data->height) return false;

    if (xl<0) xl=0; if (xr>=data->width) xr=data->width-1;
    int off=iy*data->width;
    while (xl<=xr)
    {
        if (_match_color(*(data->ibits+off+xl),data->colormin,data->colormax))
        {
            int xxr=xl, xxl=xl;
            data->color+=_scanleft(xxl, data->ibits + off, data->colormin,data->colormax,ColorInactive);
            data->color+=_scanright(xxr, data->ibits + off, data->colormin,data->colormax,ColorInactive,data->width);
            data->hlines.add(xxl,xxr,iy);
            data->stack.push(xxl,xxr,iy+1);
            data->stack.push(xxl,xxr,iy-1);
            if (iy>data->rc.bottom) data->rc.bottom=iy;
            if (iy<data->rc.top) data->rc.top=iy;
            if (xxl<data->rc.left) data->rc.left=xxl;
            if (xxr>data->rc.right) data->rc.right=xxr;
            xl=xxr;
        }
        xl++;
    }
    return true;
}

__forceinline void _check_cluster(ClusterWrkData* data, int ix, int iy, int d)
{
    int xl=ix, xr=ix;
    int off=iy*data->width;
    if (
       (ix<0)  || (ix>=data->width) || (iy<0) || (iy>=data->height) || 
       !_match_color(*(data->ibits+off+ix),data->colormin,data->colormax)
       ) 
       return;

    // update data rectngle
    if (iy>data->rc.bottom) data->rc.bottom=iy;
    if (iy<data->rc.top) data->rc.top=iy;

    int ColorInactive = (data->colormin==data->colormax)?INACTIVE_COLOR:((data->colormin > 0) ? data->colormin-1:data->colormax+1);
    data->color+=_scanleft(xl, data->ibits + off, data->colormin,data->colormax,ColorInactive);
    data->color+=_scanright(xr, data->ibits + off, data->colormin,data->colormax,ColorInactive,data->width);
    data->hlines.add(xl,xr,iy);
    data->stack.push(xl,xr,iy+1);
    data->stack.push(xl,xr,iy-1);
    if (xl<data->rc.left) data->rc.left=xl;
    if (xr>data->rc.right) data->rc.right=xr;
    
    while (data->stack.getsize())
        _check_stroke(data, ColorInactive);
}

__forceinline bool _check_stroked(ClusterWrkData* data, int ColorInactive)
{
    int xl, xr, iy;

    data->stack.pop(xl, xr, iy);
    
    if (iy<0) return false;
    if (iy>=data->height) return false;

    if (xl<0) xl=0; if (xr>=data->width) xr=data->width-1;
    int off=iy*data->width;
    while (xl<=xr)
    {
        if (_match_color(*(data->ibits+off+xl),data->colormin,data->colormax))
        {
            int xxr=xl, xxl=xl;
            data->color+=_scanleft(xxl, data->ibits + off, data->colormin,data->colormax,ColorInactive);
            data->color+=_scanright(xxr, data->ibits + off, data->colormin,data->colormax,ColorInactive,data->width);
            data->hlines.add(xxl,xxr,iy);
            data->stack.push(xxl-1,xxr+1,iy+1);
            data->stack.push(xxl-1,xxr+1,iy-1);
            if (iy>data->rc.bottom) data->rc.bottom=iy;
            if (iy<data->rc.top) data->rc.top=iy;
            if (xxl<data->rc.left) data->rc.left=xxl;
            if (xxr>data->rc.right) data->rc.right=xxr;
            xl=xxr;
        }
        xl++;
    }
    return true;
}

__forceinline void _check_cluster_d(ClusterWrkData* data, int ix, int iy, int d)
{
    int xl=ix, xr=ix;
    int off=iy*data->width;
    if (
       (ix<0)  || (ix>=data->width) || (iy<0) || (iy>=data->height) || 
       !_match_color(*(data->ibits+off+ix),data->colormin,data->colormax)
       ) 
       return;

    // update data rectngle
    if (iy>data->rc.bottom) data->rc.bottom=iy;
    if (iy<data->rc.top) data->rc.top=iy;

    int ColorInactive = (data->colormin==data->colormax)?INACTIVE_COLOR:((data->colormin > 0) ? data->colormin-1:data->colormax+1);
    data->color+=_scanleft(xl, data->ibits + off, data->colormin,data->colormax,ColorInactive);
    data->color+=_scanright(xr, data->ibits + off, data->colormin,data->colormax,ColorInactive,data->width);
    data->hlines.add(xl,xr,iy);
    data->stack.push(xl-1,xr+1,iy+1);
    data->stack.push(xl-1,xr+1,iy-1);
    if (xl<data->rc.left) data->rc.left=xl;
    if (xr>data->rc.right) data->rc.right=xr;
    
    while (data->stack.getsize())
        _check_stroked(data, ColorInactive);
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CClusters::CClusters():
        m_WrkData(NULL),m_WrkDataSize(0), m_OrgFrame(NULL),m_ClustersNmb(0),
        m_Clusters(0),m_ClustersAllocated(0),m_ConnectDiagonal(false),
        m_Width(0),m_Height(0), m_MinColor(255), m_MaxColor(255)
{
}

CClusters::~CClusters()
{
    Reset();
}

void CClusters::Reset()
{
    if (m_WrkData) free(m_WrkData); m_WrkData=NULL;
    _clusters_reset(m_Clusters, m_ClustersNmb);
    if (m_Clusters) free(m_Clusters);
	m_Clusters=NULL;
    m_ClustersNmb=0;
    m_ClustersAllocated=0;
}


bool CClusters::ParseFrame(pTVFrame frame)
{
    Reset();
    if ((!frame) || (!frame->lpBMIH)) return false;
    m_OrgFrame=frame;
    m_Width = frame->lpBMIH->biWidth;
    m_Height= frame->lpBMIH->biHeight;
    
    m_WrkDataSize=m_Width * m_Height;

    if (!m_WrkDataSize) return false;
    m_WrkData=(LPBYTE)malloc(m_WrkDataSize);
    ASSERT(m_WrkData!=NULL);
    memcpy(m_WrkData,GetData(frame),m_WrkDataSize);
    if (m_MaxColor == m_MinColor )
        return SeekClusters();
    else
        return SeekIntervalClusters();
    return true;
}

bool CClusters::ParseRawData(LPBYTE data, int w, int h)
{
    Reset();
    if (!data) return false;
    m_OrgFrame=NULL;
    m_Width = w;
    m_Height= h;
    
    m_WrkDataSize=m_Width * m_Height;

    if (!m_WrkDataSize) return false;
    m_WrkData=(LPBYTE)malloc(m_WrkDataSize);
    ASSERT(m_WrkData!=NULL);
    memcpy(m_WrkData,data,m_WrkDataSize);
    if (m_MaxColor == m_MinColor)
        return SeekClusters();
    else
        return SeekIntervalClusters();
    return true;
}

bool CClusters::SeekClusters()
{
    ClusterWrkData cd;
    cd.ibits=m_WrkData;
    cd.width=m_Width;
    cd.height=m_Height;

    _replace_inactivecolor(m_WrkData,m_WrkDataSize);

    cd.rc.left=cd.width; cd.rc.top=cd.height;
    cd.rc.right=0; cd.rc.bottom=0;
    LPBYTE end_of_data=m_WrkData+m_WrkDataSize;
    LPBYTE scanner=m_WrkData;
    while(scanner<end_of_data)
    {
        if (*scanner!=INACTIVE_COLOR) 
        {
            int offset=scanner-m_WrkData;
            cd.colormin=cd.colormax=(*scanner);
            cd.color=0;
            if (!m_ConnectDiagonal)
                _check_cluster(&cd, offset%cd.width, offset/cd.width, 1);
            else
                _check_cluster_d(&cd, offset%cd.width, offset/cd.width, 1);
            if (cd.hlines.get_hlinesnmb())
            {
                Add(&cd);
                cd.rc.left=cd.width; cd.rc.top=cd.height;
                cd.rc.right=0; cd.rc.bottom=0;
            }
            cd.hlines.reset();
        }
        scanner++;
    }
    return true;
}

bool CClusters::SeekIntervalClusters()
{
    ClusterWrkData cd;
    cd.ibits=m_WrkData;
    cd.width=m_Width;
    cd.height=m_Height;

    cd.rc.left=cd.width; cd.rc.top=cd.height;
    cd.rc.right=0; cd.rc.bottom=0;

    LPBYTE end_of_data=m_WrkData+m_WrkDataSize;
    LPBYTE scanner=m_WrkData;
    while(scanner<end_of_data)
    {
        if (IsClusterColor(*scanner)) 
        {
            int offset=scanner-m_WrkData;
            cd.color=0;
            cd.colormin=m_MinColor;
			cd.colormax=m_MaxColor;
            if (!m_ConnectDiagonal)
                _check_cluster(&cd, offset%cd.width, offset/cd.width, 1);
            else
                _check_cluster_d(&cd, offset%cd.width, offset/cd.width, 1);
            if (cd.hlines.get_hlinesnmb())
            {
                Add(&cd);
                cd.rc.left=cd.width; cd.rc.top=cd.height;
                cd.rc.right=0; cd.rc.bottom=0;
            }
            cd.hlines.reset();
        }
        scanner++;
    }
    return true;
}

void CClusters::InsertAt(pCluster pCl, int n)
{
    if (m_ClustersNmb+1>=m_ClustersAllocated)
    {
        m_Clusters =(pCluster)realloc(m_Clusters,sizeof(Cluster)*(m_ClustersAllocated+clusters_growth)); 
        if (m_Clusters)
        {
            m_ClustersAllocated+=clusters_growth;
        }
    }
    if ((m_ClustersNmb-n+1)!=0)
        memmove(&(m_Clusters [n+1]),&(m_Clusters [n]), sizeof(Cluster)*(m_ClustersNmb-n+1));
    memcpy(&(m_Clusters [n]),pCl,(sizeof(Cluster)));
    memset(pCl,0,(sizeof(Cluster)));
    m_ClustersNmb++;
}

void CClusters::Add(LPVOID data)
{
    ClusterWrkData *cd=(ClusterWrkData*)data;
    if (m_ClustersNmb==m_ClustersAllocated)
    {
        m_Clusters =(pCluster)realloc(m_Clusters,sizeof(Cluster)*(m_ClustersAllocated+clusters_growth)); 
        if (m_Clusters)
        {
            m_ClustersAllocated+=clusters_growth;
        }
    }
    if ((m_Clusters) && (((cd->rc.right - cd->rc.left+1)*(cd->rc.bottom - cd->rc.top+1))>MIN_CLUSTER_BOXAREA))
    {
        memset(&(m_Clusters[m_ClustersNmb]),0,sizeof(Cluster));
        m_Clusters[m_ClustersNmb].offset.x=cd->rc.left;
        m_Clusters[m_ClustersNmb].offset.y=cd->rc.top;
        m_Clusters[m_ClustersNmb].size.cx=cd->rc.right - cd->rc.left+1;
        m_Clusters[m_ClustersNmb].size.cy=cd->rc.bottom - cd->rc.top+1;
        m_Clusters[m_ClustersNmb].area=0;
        m_Clusters[m_ClustersNmb].color=cd->colormin;
        m_Clusters[m_ClustersNmb].colormin=cd->colormin;
        m_Clusters[m_ClustersNmb].colormax=cd->colormax;
        m_Clusters[m_ClustersNmb].userdata1=NULL;
        m_Clusters[m_ClustersNmb].userdata2=NULL;
        m_Clusters[m_ClustersNmb].userdata3=NULL;
        m_Clusters[m_ClustersNmb].userparam=NULL;
        m_Clusters[m_ClustersNmb].clstU=NULL;
        m_Clusters[m_ClustersNmb].clstV=NULL;
        int datasize=m_Clusters[m_ClustersNmb].size.cx*m_Clusters[m_ClustersNmb].size.cy;
        m_Clusters[m_ClustersNmb].clstBM=(LPBYTE)malloc(datasize);
        ASSERT(m_Clusters[m_ClustersNmb].clstBM);
        memset(m_Clusters[m_ClustersNmb].clstBM,0,datasize); 
        for (int i=0; i<cd->hlines.get_hlinesnmb(); i++)
        {
            _hline * ph=cd->hlines.get_at(i);
            
            m_Clusters[m_ClustersNmb].area+=ph->xr-ph->xl+1; // calculate area occupired

            int y=ph->y - m_Clusters[m_ClustersNmb].offset.y;
            int xl=ph->xl - m_Clusters[m_ClustersNmb].offset.x;
            int offset=y*m_Clusters[m_ClustersNmb].size.cx + xl;
			ASSERT(offset>=0);
			ASSERT(offset<=datasize);
			ASSERT((offset+(ph->xr-ph->xl))<=datasize);
            memset((m_Clusters[m_ClustersNmb].clstBM)+offset,255,(ph->xr-ph->xl+1));
        }
        m_Clusters[m_ClustersNmb].color=cd->color/m_Clusters[m_ClustersNmb].area;
        m_ClustersNmb++;
    }
}


void CClusters::Rescale(int Ratio)
{
    for (int i=0; i<m_ClustersNmb; i++)
    {
        m_Clusters[i].offset.x = (m_Clusters[i].offset.x*Ratio);
        m_Clusters[i].offset.y = (m_Clusters[i].offset.y*Ratio);
        m_Clusters[i].size.cx  = (m_Clusters[i].size.cx*Ratio);
        m_Clusters[i].size.cy  = (m_Clusters[i].size.cy*Ratio);
        int datasize=m_Clusters[i].size.cx*m_Clusters[i].size.cy;
        m_Clusters[i].clstBM=(LPBYTE)realloc(m_Clusters[i].clstBM,datasize);
        if (Ratio==2)
        {
            redouble(m_Clusters[i].clstBM,m_Clusters[i].size.cx/2,m_Clusters[i].size.cy/2);
        }
        else
        {
            TRACE("!!! Warrning at rescaling all data loss!\n");
            memset(m_Clusters[i].clstBM,0,datasize);
        }
    }
    m_Width=(int)(Ratio*m_Width);
    m_Height=(int)(Ratio*m_Height);
}


void CClusters::Alter(int x, int y)
{
    for (int i=0; i<m_ClustersNmb; i++)
    {
        m_Clusters[i].offset.x -= x; if ((int)(m_Clusters[i].offset.x)<0) m_Clusters[i].offset.x=0;
        m_Clusters[i].offset.y -= y; if ((int)(m_Clusters[i].offset.y)<0) m_Clusters[i].offset.y=0;

        m_Clusters[i].size.cx  += 2*x; if (m_Clusters[i].size.cx>=m_Width-m_Clusters[i].offset.x) m_Clusters[i].size.cx=m_Width-m_Clusters[i].offset.x-1;
        m_Clusters[i].size.cy  += 2*y; if (m_Clusters[i].size.cy>=m_Height-m_Clusters[i].offset.y) m_Clusters[i].size.cy=m_Height-m_Clusters[i].offset.y-1;
        
        int datasize=m_Clusters[i].size.cx*m_Clusters[i].size.cy;
        m_Clusters[i].clstBM=(LPBYTE)realloc(m_Clusters[i].clstBM,datasize);
        memset(m_Clusters[i].clstBM,0,datasize);
    }
}
