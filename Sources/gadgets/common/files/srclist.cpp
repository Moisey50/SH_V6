// SrcList.cpp: implementation of the CSrcList class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SrcList.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSrcList::CSrcList():
    m_Position(0),
    m_TimeID(-1)
{
    m_Comment="No time marker in a file";
}

CSrcList::~CSrcList()
{

}
