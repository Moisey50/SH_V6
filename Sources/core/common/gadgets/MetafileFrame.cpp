// MetafileFrame.cpp: implementation of the CMetafileFrame class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <gadgets\MetafileFrame.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// CMFHelper
//////////////////////////////////////////////////////////////////////
CMFHelper::CMFHelper()
{
    m_WMFData=NULL;
    m_WMFSize=0;
    m_MFHDC=NULL;
}

CMFHelper::CMFHelper(CMFHelper* wmHelper)
{
    m_MFHDC=NULL;
    if (!wmHelper)
    {
        m_WMFData=NULL;
        m_WMFSize=0;
    }
    else
    {
        Copy(*wmHelper);
    }
}

CMFHelper::~CMFHelper()
{
    if (m_MFHDC)
    {
        EndDraw();
    }
    if (m_WMFData)
        free(m_WMFData);
    m_WMFData=NULL;
    m_WMFSize=0;
}

void CMFHelper::Copy(const CMFHelper& mfSrc ) 
{ 
    //m_MFDC=NULL;
    m_MFHDC=NULL;
    m_WMFSize=mfSrc.m_WMFSize;
    if (m_WMFSize)
    {
        m_WMFData=malloc(m_WMFSize);
        memcpy(m_WMFData,mfSrc.m_WMFData,m_WMFSize);
    }
    else
    {
		m_WMFData=NULL;
    }
}

HDC CMFHelper::StartDraw(LPRECT rc)
{
    ASSERT(m_MFHDC==NULL);
    //HWND dwnd=::GetDesktopWindow();
    //HDC hdcRef = ::GetDC(dwnd); 
    m_MFHDC=::CreateEnhMetaFile(NULL,NULL,rc,NULL);
    //ReleaseDC(dwnd, hdcRef);
    return m_MFHDC;
}

void CMFHelper::EndDraw()
{
    if (m_MFHDC)
    {
        HENHMETAFILE hmf = ::CloseEnhMetaFile(m_MFHDC);
        if (hmf)
        {
            m_WMFSize=::GetEnhMetaFileBits(hmf,0,NULL);
            if (m_WMFSize)
            {
                m_WMFData=malloc(m_WMFSize);
                if (::GetEnhMetaFileBits(hmf,m_WMFSize,(LPBYTE)m_WMFData)==0)
                {
                    free(m_WMFData);
                    m_WMFData=NULL;
                    m_WMFSize=0;
                }
            }
            ::DeleteEnhMetaFile(hmf);
        }
        m_MFHDC=NULL;
    }
}

//////////////////////////////////////////////////////////////////////
// CMetafileFrame
//////////////////////////////////////////////////////////////////////

CMetafileFrame::CMetafileFrame()
{
    m_DataType = metafile;
}

CMetafileFrame::CMetafileFrame(const CMetafileFrame* WMFFrame):
        CMFHelper((CMFHelper*)WMFFrame)
{
    ASSERT(WMFFrame->m_DataType==metafile);
	m_DataType = metafile;
	if (WMFFrame)
        CopyAttributes(WMFFrame);
}

CMetafileFrame::~CMetafileFrame()
{
}

CMetafileFrame* CMetafileFrame::GetMetafileFrame(LPCTSTR label)
{
	if (!label || m_Label == label)
		return this;
	return NULL;
}

const CMetafileFrame* CMetafileFrame::GetMetafileFrame(LPCTSTR label) const
{
	if (!label || m_Label == label)
		return this;
	return NULL;
}

CMetafileFrame* CMetafileFrame::Create()
{
    return new CMetafileFrame();
}

BOOL CMetafileFrame::IsNullFrame() const
{
    return (m_WMFData==NULL);
}

