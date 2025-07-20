// MatrixReader.h: interface for the CMatrixReader class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MATRIXREADER_H__D1303686_DC63_48D1_A377_DD52D1D03BB5__INCLUDED_)
#define AFX_MATRIXREADER_H__D1303686_DC63_48D1_A377_DD52D1D03BB5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <video\TVFrame.h>
#include <imageproc\recognition\mtrxrd.h>
#include <fxfc\fxfc.h>

class CMatrixReader  
{
private:
    pMtrxRdBlck m_mxBlck;
public:
	CMatrixReader();
	virtual ~CMatrixReader();
	bool Read(const pTVFrame ptv, FXString& Result);
    tetragon* GetMxTetragon() { if (m_mxBlck) return &(m_mxBlck->mtetragon); return NULL;};
};

#endif // !defined(AFX_MATRIXREADER_H__D1303686_DC63_48D1_A377_DD52D1D03BB5__INCLUDED_)
