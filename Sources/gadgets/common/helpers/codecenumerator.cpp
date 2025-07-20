// CodecEnumerator.cpp: implementation of the CCodecEnumerator class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CodecEnumerator.h"
#include <vfw.h>
#pragma comment( lib, "Vfw32.lib" )

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCodecEnumerator::CCodecEnumerator()
{
    codecInfo ci;
    m_Codecs.RemoveAll();

    ci.fccHandler=0;
    ci.cName="Uncompressed YUV9";
    ci.Description="Uncompressed YUV9";
    
    m_Codecs.Add(ci);

    ICINFO ii;
    memset(&ii,0,sizeof(ICINFO));
    ii.dwSize=sizeof(ICINFO);
    ii.fccType=ICTYPE_VIDEO;

    for(DWORD i = 0; ICInfo(ICTYPE_VIDEO, i, &ii); i++)
    {


        HIC h = ICOpen(ICTYPE_VIDEO, ii.fccHandler, ICMODE_COMPRESS);
        if (h) 
        {
            if (ICGetInfo(h, &ii, sizeof(ii)))
            {
                ci.fccHandler=ii.fccHandler;
                ci.cName=ii.szName;
                ci.Description=ii.szDescription;
                if (ci.fccHandler!=0)
                    m_Codecs.Add(ci);
            }
            ICClose(h);
        }
    }
    TRACE("%d codecs installed.\n", GetCodecCount());
}

CCodecEnumerator::~CCodecEnumerator()
{
    m_Codecs.RemoveAll();
}
