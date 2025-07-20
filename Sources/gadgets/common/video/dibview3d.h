#pragma once

class CDIBView3D : public CDIBView
{
	int m_alpha, m_beta, m_rotZ;
	int m_grid;
  int m_iLow , m_iHigh ;
	bool m_bGray;
public:
	CDIBView3D(LPCTSTR name = "");
	~CDIBView3D();
	void ShiftPos(int x, int y) { };
	void SetParams(int* alpha = NULL, int* beta = NULL, 
    int* grid = NULL, bool* bGray = NULL, int* rotZ = NULL ,
    int * piLow = NULL , int * piHigh = NULL )
	{
		if (alpha) 
      m_alpha = *alpha;
		if (beta) 
      m_beta = *beta;
		if (grid) 
      m_grid = *grid;
		if (bGray) 
      m_bGray = *bGray;
		if (rotZ) 
      m_rotZ = *rotZ;
    if ( piLow )
      m_iLow = *piLow ;
    if ( piHigh )
      m_iHigh = *piHigh ;
	}
protected:
	bool PrepareData(const pTVFrame frame, DWORD dwTimeout );
};
