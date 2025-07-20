// VectorFieldFrame.cpp: implementation of the CVectorFieldFrame class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "VectorFields.h"
#include "VectorFieldFrame.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

__forceinline int vectorSize(const CVectorField& vf)
{
    return sizeof(vfvector)*vf.sizeX*vf.sizeY;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CVectorFieldFrame::CVectorFieldFrame():
        CUserDataFrame(VECTROFILELDNAME)
{

}

CVectorFieldFrame::CVectorFieldFrame(CVectorField& vf):
        CUserDataFrame(VECTROFILELDNAME)
{
    sizeX=vf.sizeX;
    sizeY=vf.sizeY;
    stepX=vf.stepX;
    stepY=vf.stepY;
    vfvectors=(pvfvector)malloc(vectorSize(vf));
    memcpy(vfvectors,vf.vfvectors,vectorSize(vf));
}

CVectorFieldFrame::~CVectorFieldFrame()
{

}

BOOL CVectorFieldFrame::Serialize(LPBYTE* ppData, UINT* cbData) const
{
	*cbData = (int) m_UserDataName.GetLength() + 1 + sizeof(CVectorField)+vectorSize(*this);
	*ppData = (LPBYTE)malloc(*cbData);
	::ZeroMemory(*ppData, *cbData);
	memcpy(*ppData, m_UserDataName, m_UserDataName.GetLength());
	LPBYTE dst = (*ppData) + m_UserDataName.GetLength() + 1;
	memcpy(dst, (CVectorField*)this, sizeof(CVectorField));
    dst+=sizeof(CVectorField);
    memcpy(dst, this->vfvectors, vectorSize(*this));
	return TRUE;
}

BOOL CVectorFieldFrame::Restore(LPBYTE lpData, UINT cbData)
{
	if (strcmp((char*)lpData, m_UserDataName))
		return FALSE;
	UINT offset = (UINT)strlen((char*)lpData) + 1;
	if (cbData - offset < sizeof(CVectorField))
		return FALSE;
    memcpy((CVectorField*)this, lpData+offset, sizeof(CVectorField));
    offset+=sizeof(CVectorField);
    unsigned dsize=vectorSize(*this);
    if (cbData-offset!=dsize)
        return FALSE;
    this->vfvectors=(pvfvector)malloc(dsize);
	memcpy(this->vfvectors, lpData + offset, dsize);
	return TRUE;
}
