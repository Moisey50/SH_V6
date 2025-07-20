// StVideoRenderer.cpp: implementation of the CStVideoRenderer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "tvvideo.h"
#include "StVideoRenderer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_RUNTIME_GADGET_EX(CStVideoRenderer, CRenderGadget, "Video.Stereo",TVDB400_PLUGIN_NAME);

CStVideoRenderer::CStVideoRenderer()
{

}

CStVideoRenderer::~CStVideoRenderer()
{

}

void CStVideoRenderer::Render(CDataFrame* pDataFrame)
{

}

void CStVideoRenderer::Create(CWnd* pWnd)
{
}
