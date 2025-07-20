//  $File : Clusters.h: interface for the CClusters class.
//  (C) Copyright The File X Ltd 2002. 
//
//
//
//  Revision History (excluding minor changes for specific compilers)
//   13 May 02 Firts release version, all followed changes must be listed below  (Andrey Chernushich)

#if !defined(AFX_CLUSTERS_H__140F6061_E7D5_11D5_B0F2_000001360793__INCLUDED_)
#define AFX_CLUSTERS_H__140F6061_E7D5_11D5_B0F2_000001360793__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <video\TVFrame.h>

#define LINE_CLUSTERS_GROWTH 20

typedef struct tagCluster
{
    POINT   offset;
    SIZE    size;
    int     area;
    int     colormin;
	int		colormax;
    int     color;
    int     gsize;
    int     rec_error;
    char    rec_as;
	bool	deleted;
    bool    datagood;
    int     fgI,fgU,fgV, int1, int2, int3;
    LPBYTE  clstBM, clstU, clstV;
    LPBYTE  userdata1,userdata2,userdata3;
    DWORD   userparam;
}Cluster, *pCluster;

typedef struct tagClustersInfo
{
    pCluster m_Clusters;
    int      m_ClustersNmb;
}ClustersInfo, *pClustersInfo;

inline double _clXYRatio(Cluster& pcl) { return (((double)pcl.size.cx)/pcl.size.cy);}

class CClusters  
{
protected:
    pCluster m_Clusters;
    int      m_ClustersNmb;
    const pTVFrame m_OrgFrame;
    LPBYTE   m_WrkData;
    DWORD    m_WrkDataSize;
    int      m_Width;
    int      m_Height;
    int      m_ClustersAllocated;
    bool     m_ConnectDiagonal;
	unsigned char m_MinColor;
	unsigned char m_MaxColor;
private:
// private methods, do not touch!
    virtual bool    SeekClusters();
    virtual bool    SeekIntervalClusters();
    virtual void    Add(LPVOID data);
public:
				CClusters();
	virtual		~CClusters();
	void		Reset();
	bool		ParseFrame(const pTVFrame frame);
    pCluster    GetAt(int n) { return &m_Clusters[n]; };
	void        InsertAt(pCluster pCl, int n);
	bool        ParseRawData(LPBYTE data, int w, int h);
	void        Alter(int x, int y);
	void        Rescale(int Ratio);
//  inlines
    void		SetClusterColor(int Color) { SetClusterColors(Color,Color); };
	void		SetClusterColors(unsigned char min, unsigned char max) { m_MinColor = min; m_MaxColor = max; };
	void		GetClusterColors(unsigned char* min, unsigned char* max) { if (min) *min = m_MinColor; if (max) *max = m_MaxColor; };
    int			GetClustersNmb()	{ return m_ClustersNmb;  };
	pCluster	GetCluster(int nmb) { return &(m_Clusters[nmb]); }; 
    pClustersInfo GetClusterInfo()  { return (pClustersInfo)(&m_Clusters);};    
    CRect       GetClusterRect(int nmb) { return(CRect(m_Clusters[nmb].offset.x,m_Clusters[nmb].offset.y,m_Clusters[nmb].offset.x+m_Clusters[nmb].size.cx,m_Clusters[nmb].offset.y+m_Clusters[nmb].size.cy));}
    const pTVFrame GetOrgFrame()       {return m_OrgFrame; };
    int         GetImgWidth()       { return m_Width; };
    int         GetImgHeight()      { return m_Height; };
    void        DiagonalConections(bool allow=true) { m_ConnectDiagonal=allow; };
	bool		IsClusterColor(unsigned char color)
    {
	    return ((color >= m_MinColor) && (color <= m_MaxColor));
    }
};


#include <ImageProc\Colors.h>

#endif // !defined(AFX_CLUSTERS_H__140F6061_E7D5_11D5_B0F2_000001360793__INCLUDED_)
