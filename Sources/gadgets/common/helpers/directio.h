// DirectIO.h: interface for the CDirectIO class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DIRECTIO_H__10BA69A3_B03D_46B3_B7F5_12BFD0798A50__INCLUDED_)
#define AFX_DIRECTIO_H__10BA69A3_B03D_46B3_B7F5_12BFD0798A50__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDirectIO  
{
private:
    int     m_Sysver;
    HANDLE  m_hdriver;
public:
	CDirectIO();
	virtual ~CDirectIO();
    void Out32(short PortAddress, short data);
    short Inp32(short PortAddress);
    bool  IsValid() { return ((m_Sysver==1) || (m_hdriver!=NULL));}
};

#endif // !defined(AFX_DIRECTIO_H__10BA69A3_B03D_46B3_B7F5_12BFD0798A50__INCLUDED_)
