#include "StdAfx.h"
#include "MergeLayers.h"
#include <Gadgets\VideoFrame.h>
#include <video\shvideo.h>
#include <imageproc/rotate.h>

IMPLEMENT_RUNTIME_GADGET_EX(MergeLayers, CGadget, "Video.conversion", TVDB400_PLUGIN_NAME);

MergeLayers::MergeLayers(void)
{
	m_pInput  = new CInputConnector(vframe);
	m_pOutput = new COutputConnector(vframe);
    m_MultyCoreAllowed=true;
	Resume();
}


void MergeLayers::ShutDown(void)
{
    CGadget::ShutDown();
    delete m_pInput;
	m_pInput = NULL;
	delete m_pOutput;
	m_pOutput = NULL;
	m_Metafiles.RemoveAll( );
}

CDataFrame* MergeLayers::DoProcessing(const CDataFrame* pDataFrame)
{
	const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame(DEFAULT_LABEL);
    CFramesIterator* mfIter = pDataFrame->CreateFramesIterator(metafile);
    if (mfIter)
    {
        m_Lock.Lock();
        m_Metafiles.RemoveAll( );
        CMetafileFrame* mfFrame = (CMetafileFrame*)mfIter->Next(DEFAULT_LABEL);
        while (mfFrame)
        {
            m_Metafiles.Add(*mfFrame);
            mfFrame = (CMetafileFrame*)mfIter->Next(DEFAULT_LABEL);
        }
        m_Lock.Unlock();
        delete mfIter; mfIter=NULL;
    }
    else
    {
        m_Lock.Lock();
        m_Metafiles.RemoveAll( );
        m_Lock.Unlock();
    }
    if (VideoFrame)
        return DoDraw(VideoFrame);
    else
        return NULL;
}

CVideoFrame* MergeLayers::DoDraw(const CVideoFrame* vf)
{
    CDC mdc; // memory DC
    mdc.CreateCompatibleDC(NULL);

    CBitmap cbm;
	
	LPBITMAPINFOHEADER pBMP;
    if (vf->lpBMIH->biCompression==BI_Y16)
    {
        pBMP=y16rgb24(vf->lpBMIH,vf->lpData);
        _fliph(pBMP);
    }
    else if (vf->lpBMIH->biCompression==BI_Y8)
    {
        pBMP=y8rgb24(vf->lpBMIH,vf->lpData);
        _fliph(pBMP);
    }
    else if (vf->lpBMIH->biCompression==BI_YUV12)
    {
        pBMP=yuv12rgb24(vf->lpBMIH,vf->lpData);
		_fliph(pBMP);
    }
    else if (vf->lpBMIH->biCompression==BI_YUV9)
    {
        pBMP=yuv9rgb24(vf->lpBMIH,vf->lpData);
        _fliph(pBMP);
    }
    m_Width=vf->lpBMIH->biWidth, m_Height=vf->lpBMIH->biHeight;
	LPBITMAPINFOHEADER pBMP32=rgb24rdb32(pBMP);
	free(pBMP);
    
	VERIFY(cbm.CreateBitmap(m_Width,m_Height,1,32,LPBYTE(pBMP32)+pBMP32->biSize)==TRUE);
		
    BITMAP bm;
    cbm.GetObject(sizeof(BITMAP),&bm);
	m_Width=bm.bmWidth;
	m_Height=bm.bmHeight;
    CBitmap* pOld = (CBitmap*)mdc.SelectObject(&cbm);
    
	DrawMetafiles(mdc.m_hDC);
	
	mdc.SelectObject(pOld);

    pTVFrame tvf= (pTVFrame)malloc(sizeof(TVFrame));
    int isize=sizeof(BITMAPINFOHEADER)+bm.bmWidthBytes*bm.bmHeight;
    tvf->lpBMIH=(LPBITMAPINFOHEADER)malloc(isize);
    tvf->lpData=NULL;
    memset(tvf->lpBMIH,0,isize);
    tvf->lpBMIH->biSize=sizeof(BITMAPINFOHEADER);
    tvf->lpBMIH->biWidth=bm.bmWidth;
    tvf->lpBMIH->biHeight=bm.bmHeight;
    tvf->lpBMIH->biPlanes=bm.bmPlanes;
    tvf->lpBMIH->biBitCount=bm.bmBitsPixel;
    LPBYTE data=(LPBYTE)tvf->lpBMIH+sizeof(BITMAPINFOHEADER);
	cbm.GetBitmapBits(bm.bmWidthBytes*bm.bmHeight,data);
    compress2yuv9(tvf);
	DeleteObject(cbm);
    _fliph(tvf);
	free(pBMP32);
    CVideoFrame* pFrame=CVideoFrame::Create(tvf);
    pFrame->CopyAttributes(vf);
	return pFrame;
}

void MergeLayers::DrawMetafiles(HDC hdc)
{
	bool MapModeChanged=false;
	int iMap = ::GetMapMode(hdc);
	if(iMap == MM_TEXT)
	{
		::SetMapMode(hdc,MM_HIMETRIC);
		MapModeChanged=true;
	}

    CPoint tl=0;
	DPtoLP(hdc,&tl,1);

    CPoint tstPoint(1000,1000);
    DPtoLP(hdc,&tstPoint,1);

    CRgn   rgn;
    VERIFY(rgn.CreateRectRgn(0,0,0+(int)(m_Width*1+0.5),0+(int)(m_Height*1+0.5)));
    ::SelectClipRgn(hdc,rgn);

    CPoint br(tl.x+(int)(m_Width+0.5),tl.y-(int)(m_Height+0.5));
    //CPoint br(m_ScrOffset.x+(int)(m_Width*m_ScrScale+0.5),m_ScrOffset.y+(int)(m_Height*m_ScrScale+0.5));
	//DPtoLP(hdc,&br,1);
    for(int i=0; i<m_Metafiles.GetSize(); i++)
    {
		CRect clRect (0,0,m_Width,m_Height);
		//GetClientRect(clRect);
		
        HENHMETAFILE hmf=m_Metafiles[i].GetMF();

		if (hmf)
			PlayEnhMetaFile(hdc,hmf,CRect(tl.x,tl.y,br.x,br.y));
        DeleteEnhMetaFile(hmf);
    }
	if (MapModeChanged)
		CDC::FromHandle(hdc)->SetMapMode(iMap);
}
