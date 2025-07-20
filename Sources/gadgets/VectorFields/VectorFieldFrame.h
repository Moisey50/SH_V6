// VectorFieldFrame.h: interface for the CVectorFieldFrame class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VECTORFIELDFRAME_H__01E3782A_AA67_4DF5_AFE0_51B1012EA0D0__INCLUDED_)
#define AFX_VECTORFIELDFRAME_H__01E3782A_AA67_4DF5_AFE0_51B1012EA0D0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <gadgets\gadbase.h>

#define VECTROFILELDNAME "VectorField"

typedef struct _tagvfvector
{
    double x,y;
} vfvector,*pvfvector;

class CVectorField
{
public:
    int     sizeX, sizeY;
    double  stepX, stepY;
    vfvector* vfvectors;
public:
    CVectorField()
    {
        sizeX=sizeY=0;
        stepX=stepY=0.0;
        vfvectors=NULL;
    };
    ~CVectorField()
    {
        sizeX=sizeY=0;
        stepX=stepY=0.0;
        if (vfvectors)
        {
            free(vfvectors); vfvectors=NULL;
        }
    }
};

class CVectorFieldFrame : public CUserDataFrame,
                          public CVectorField
{
protected:
	CVectorFieldFrame();
    CVectorFieldFrame(CVectorField& vf);
	virtual ~CVectorFieldFrame();
public:
    virtual CUserDataFrame* CreateEmpty() const { return new CVectorFieldFrame(); }
    static  CVectorFieldFrame * Create() { return new CVectorFieldFrame(); };
	static  CVectorFieldFrame * Create(CVectorField& c) { return new CVectorFieldFrame(c); };
	virtual BOOL Serialize(LPBYTE* ppData, UINT* cbData) const;
	virtual BOOL Restore(LPBYTE lpData, UINT cbData);
};

#endif // !defined(AFX_VECTORFIELDFRAME_H__01E3782A_AA67_4DF5_AFE0_51B1012EA0D0__INCLUDED_)
