// OCRInscribing.cpp: implementation of the COCRInscribing class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "OCRInscribing.h"
#include <imageproc\Clusters\ClusterOp.h>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COCRInscribing::COCRInscribing()
{
//    m_Clusters.SetBWImage();
    m_Clusters.SetClusterColors(BLACK_COLOR, BLACK_COLOR);
}

COCRInscribing::~COCRInscribing()
{
    m_Clusters.Reset();
}

bool COCRInscribing::Inscribe(const pTVFrame frame)
{
    m_Clusters.Reset();
    m_Clusters.ParseFrame(frame);
    Cluster_SimpleFilters(m_Clusters.GetClusterInfo());
    //Cluster_SortX(m_Clusters.GetClusterInfo()->m_Clusters,m_Clusters.GetClusterInfo()->m_ClustersNmb);
    Cluster_SortLinepos(m_Clusters.GetClusterInfo(), m_Clusters.GetImgWidth());
    return true;
}

int COCRInscribing::GetLinesNumber()
{
    return 1;
}


int COCRInscribing::GetObjectsNmb(int line)
{
    if (line==0)
        return m_Clusters.GetClustersNmb();
    return 0;
}

static char TestMsg[11]="This a abc";

pCluster COCRInscribing::GetCluster(int line, int nmb)
{
    if (line==0)
        return m_Clusters.GetCluster(nmb);
    return NULL;
}
