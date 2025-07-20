// OCRInscribing.h: interface for the COCRInscribing class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OCRINSCRIBING_H__2BDA36E7_FB7A_48BC_979E_AA4263B5D3D8__INCLUDED_)
#define AFX_OCRINSCRIBING_H__2BDA36E7_FB7A_48BC_979E_AA4263B5D3D8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <video\TVFrame.h>
#include <imageproc\Clusters\Clusters.h>

class COCRInscribing  
{
    CClusters  m_Clusters;
public:
	            COCRInscribing();
	virtual     ~COCRInscribing();
	pCluster    GetCluster(int line, int nmb);
	int         GetObjectsNmb(int line);
	int         GetLinesNumber();
	bool        Inscribe(const pTVFrame frame);
};

#endif // !defined(AFX_OCRINSCRIBING_H__2BDA36E7_FB7A_48BC_979E_AA4263B5D3D8__INCLUDED_)
