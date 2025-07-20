#include "stdafx.h"
#include "videofilters.h"
#include <gadgets\videoframe.h>
#include "VideoLogic.h"

#ifdef THIS_MODULENAME 
    #undef THIS_MODULENAME 
#endif
#define THIS_MODULENAME "VideoLogic.CVideoLogicGadget"

IMPLEMENT_RUNTIME_GADGET_EX(Video_XOR, CTwoPinCollector, "Video.logic", TVDB400_PLUGIN_NAME);

CDataFrame* Video_XOR::DoProcessing(const CDataFrame* pDataFrame1, const CDataFrame* pDataFrame2)
{
	const CVideoFrame* VideoFrame1 = pDataFrame1->GetVideoFrame(DEFAULT_LABEL);
	const CVideoFrame* VideoFrame2 = pDataFrame2->GetVideoFrame(DEFAULT_LABEL);
	if (!VideoFrame1 || !VideoFrame2 || !VideoFrame1->lpBMIH || !VideoFrame2->lpBMIH)
		return NULL;
	pTVFrame frame1 = makecopyTVFrame(VideoFrame1);
    const pTVFrame frame2 = VideoFrame2;
    if ((Width(frame1)!=Width(frame2)) || (Height(frame1)!=Height(frame2)) || (frame1->lpBMIH->biCompression!=frame2->lpBMIH->biCompression))
        return NULL;
    switch (frame1->lpBMIH->biCompression)
    {
    case BI_Y8:
    case BI_YUV9:
    case BI_YUV12:
        _videoXOR(GetData(frame1),GetData(frame2), Width(frame1)*Height(frame1));
        break;
    case BI_Y16:
        _videoXOR16((LPWORD)GetData(frame1),(LPWORD)GetData(frame2), Width(frame1)*Height(frame1));
       break;
    }
    CVideoFrame* retV=CVideoFrame::Create(frame1);
    retV->CopyAttributes(pDataFrame1);
	return retV;
}

IMPLEMENT_RUNTIME_GADGET_EX(Video_XORB, CTwoPinCollector, "Video.logic", TVDB400_PLUGIN_NAME);

CDataFrame* Video_XORB::DoProcessing(const CDataFrame* pDataFrame1, const CDataFrame* pDataFrame2)
{
	const CVideoFrame* VideoFrame1 = pDataFrame1->GetVideoFrame(DEFAULT_LABEL);
	const CVideoFrame* VideoFrame2 = pDataFrame2->GetVideoFrame(DEFAULT_LABEL);
    if (!VideoFrame1 || !VideoFrame2 || !VideoFrame1->lpBMIH || !VideoFrame2->lpBMIH)
		return NULL;
	pTVFrame frame1 = makecopyTVFrame(VideoFrame1);
    const pTVFrame frame2 = VideoFrame2;
    if ((Width(frame1)!=Width(frame2)) || (Height(frame1)!=Height(frame2)) || (frame1->lpBMIH->biCompression!=frame2->lpBMIH->biCompression))
        return NULL;
    switch (frame1->lpBMIH->biCompression)
    {
    case BI_Y8:
    case BI_YUV9:
    case BI_YUV12:
        _videoXORB(GetData(frame1),GetData(frame2), Width(frame1)*Height(frame1));
        break;
    case BI_Y16:
        _videoXORB16((LPWORD)GetData(frame1),(LPWORD)GetData(frame2), Width(frame1)*Height(frame1));
       break;
    }
    CVideoFrame* retV=CVideoFrame::Create(frame1);
    retV->CopyAttributes(pDataFrame1);
	return retV;
}

IMPLEMENT_RUNTIME_GADGET_EX(Video_AND, CTwoPinCollector, "Video.logic", TVDB400_PLUGIN_NAME);

CDataFrame* Video_AND::DoProcessing(const CDataFrame* pDataFrame1, const CDataFrame* pDataFrame2)
{
	const CVideoFrame* VideoFrame1 = pDataFrame1->GetVideoFrame(DEFAULT_LABEL);
	const CVideoFrame* VideoFrame2 = pDataFrame2->GetVideoFrame(DEFAULT_LABEL);
	if (!VideoFrame1 || !VideoFrame2 || !VideoFrame1->lpBMIH || !VideoFrame2->lpBMIH)
		return NULL;
	pTVFrame frame1 = makecopyTVFrame(VideoFrame1);
    const pTVFrame frame2 = VideoFrame2;
    if ((Width(frame1)!=Width(frame2)) || (Height(frame1)!=Height(frame2)) || (frame1->lpBMIH->biCompression!=frame2->lpBMIH->biCompression))
        return NULL;
    switch (frame1->lpBMIH->biCompression)
    {
    case BI_Y8:
    case BI_YUV9:
    case BI_YUV12:
        _videoAND(GetData(frame1),GetData(frame2), Width(frame1)*Height(frame1));
        break;
    case BI_Y16:
        _videoAND16((LPWORD)GetData(frame1),(LPWORD)GetData(frame2), Width(frame1)*Height(frame1));
       break;
    }
    CVideoFrame* retV=CVideoFrame::Create(frame1);
    retV->CopyAttributes(pDataFrame1);
	return retV;
}

IMPLEMENT_RUNTIME_GADGET_EX(Video_Mask, CTwoPinCollector, "Video.logic", TVDB400_PLUGIN_NAME);

CDataFrame* Video_Mask::DoProcessing(const CDataFrame* pDataFrame1, const CDataFrame* pDataFrame2)
{
  const CVideoFrame* VideoFrame1 = pDataFrame1->GetVideoFrame(DEFAULT_LABEL);
  const CVideoFrame* VideoFrame2 = pDataFrame2->GetVideoFrame(DEFAULT_LABEL);
  if (!VideoFrame1 || !VideoFrame2 || !VideoFrame1->lpBMIH || !VideoFrame2->lpBMIH)
    return NULL;
  pTVFrame frame1 = makecopyTVFrame(VideoFrame1);
  const pTVFrame frame2 = VideoFrame2;
  if ((Width(frame1)!=Width(frame2)) || (Height(frame1)!=Height(frame2)) || (frame1->lpBMIH->biCompression!=frame2->lpBMIH->biCompression))
    return NULL;
  switch (frame1->lpBMIH->biCompression)
  {
  case BI_Y8:
  case BI_YUV9:
  case BI_YUV12:
    _videoMask(GetData(frame1),GetData(frame2), Width(frame1)*Height(frame1));
    break;
  case BI_Y16:
    _videoMask16((LPWORD)GetData(frame1),(LPWORD)GetData(frame2), Width(frame1)*Height(frame1));
    break;
  }
  CVideoFrame* retV=CVideoFrame::Create(frame1);
  retV->CopyAttributes(pDataFrame1);
  return retV;
}

IMPLEMENT_RUNTIME_GADGET_EX(Video_ANDB, CTwoPinCollector, "Video.logic", TVDB400_PLUGIN_NAME);

CDataFrame* Video_ANDB::DoProcessing(const CDataFrame* pDataFrame1, const CDataFrame* pDataFrame2)
{
	const CVideoFrame* VideoFrame1 = pDataFrame1->GetVideoFrame(DEFAULT_LABEL);
	const CVideoFrame* VideoFrame2 = pDataFrame2->GetVideoFrame(DEFAULT_LABEL);
	if (!VideoFrame1 || !VideoFrame2 || !VideoFrame1->lpBMIH || !VideoFrame2->lpBMIH)
		return NULL;
	pTVFrame frame1 = makecopyTVFrame(VideoFrame1);
    const pTVFrame frame2 = VideoFrame2;
    if ((Width(frame1)!=Width(frame2)) || (Height(frame1)!=Height(frame2)) || (frame1->lpBMIH->biCompression!=frame2->lpBMIH->biCompression))
        return NULL;
    switch (frame1->lpBMIH->biCompression)
    {
    case BI_Y8:
    case BI_YUV9:
    case BI_YUV12:
        _videoANDB(GetData(frame1),GetData(frame2), Width(frame1)*Height(frame1));
        break;
    case BI_Y16:
        _videoANDB16((LPWORD)GetData(frame1),(LPWORD)GetData(frame2), Width(frame1)*Height(frame1));
       break;
    }
    CVideoFrame* retV=CVideoFrame::Create(frame1);
    retV->CopyAttributes(pDataFrame1);
	return retV;
}

IMPLEMENT_RUNTIME_GADGET_EX(Video_OR, CTwoPinCollector, "Video.logic", TVDB400_PLUGIN_NAME);

CDataFrame* Video_OR::DoProcessing(const CDataFrame* pDataFrame1, const CDataFrame* pDataFrame2)
{
	const CVideoFrame* VideoFrame1 = pDataFrame1->GetVideoFrame(DEFAULT_LABEL);
	const CVideoFrame* VideoFrame2 = pDataFrame2->GetVideoFrame(DEFAULT_LABEL);
	if (!VideoFrame1 || !VideoFrame2 || !VideoFrame1->lpBMIH || !VideoFrame2->lpBMIH)
		return NULL;
	pTVFrame frame1 = makecopyTVFrame(VideoFrame1);
    const pTVFrame frame2 = VideoFrame2;
    if ((Width(frame1)!=Width(frame2)) || (Height(frame1)!=Height(frame2)) || (frame1->lpBMIH->biCompression!=frame2->lpBMIH->biCompression))
        return NULL;
    switch (frame1->lpBMIH->biCompression)
    {
    case BI_Y8:
    case BI_YUV9:
    case BI_YUV12:
        _videoOR(GetData(frame1),GetData(frame2), Width(frame1)*Height(frame1));
        break;
    case BI_Y16:
        _videoOR16((LPWORD)GetData(frame1),(LPWORD)GetData(frame2), Width(frame1)*Height(frame1));
       break;
    }
    CVideoFrame* retV=CVideoFrame::Create(frame1);
    retV->CopyAttributes(pDataFrame1);
	return retV;
}

IMPLEMENT_RUNTIME_GADGET_EX(Video_ORB, CTwoPinCollector, "Video.logic", TVDB400_PLUGIN_NAME);

CDataFrame* Video_ORB::DoProcessing(const CDataFrame* pDataFrame1, const CDataFrame* pDataFrame2)
{
	const CVideoFrame* VideoFrame1 = pDataFrame1->GetVideoFrame(DEFAULT_LABEL);
	const CVideoFrame* VideoFrame2 = pDataFrame2->GetVideoFrame(DEFAULT_LABEL);
	if (!VideoFrame1 || !VideoFrame2 || !VideoFrame1->lpBMIH || !VideoFrame2->lpBMIH)
		return NULL;
	pTVFrame frame1 = makecopyTVFrame(VideoFrame1);
    const pTVFrame frame2 = VideoFrame2;
    if ((Width(frame1)!=Width(frame2)) || (Height(frame1)!=Height(frame2)) || (frame1->lpBMIH->biCompression!=frame2->lpBMIH->biCompression))
        return NULL;
    switch (frame1->lpBMIH->biCompression)
    {
    case BI_Y8:
    case BI_YUV9:
    case BI_YUV12:
        _videoORB(GetData(frame1),GetData(frame2), Width(frame1)*Height(frame1));
        break;
    case BI_Y16:
        _videoORB16((LPWORD)GetData(frame1),(LPWORD)GetData(frame2), Width(frame1)*Height(frame1));
       break;
    }
    CVideoFrame* retV=CVideoFrame::Create(frame1);
    retV->CopyAttributes(pDataFrame1);
	return retV;
}

IMPLEMENT_RUNTIME_GADGET_EX(Video_SUB, CTwoPinCollector, "Video.math", TVDB400_PLUGIN_NAME);

CDataFrame* Video_SUB::DoProcessing(const CDataFrame* pDataFrame1, const CDataFrame* pDataFrame2)
{
	const CVideoFrame* VideoFrame1 = pDataFrame1->GetVideoFrame(DEFAULT_LABEL);
	const CVideoFrame* VideoFrame2 = pDataFrame2->GetVideoFrame(DEFAULT_LABEL);
	if (!VideoFrame1 || !VideoFrame2 || !VideoFrame1->lpBMIH || !VideoFrame2->lpBMIH)
		return NULL;
	pTVFrame frame1 = makecopyTVFrame(VideoFrame1);
    const pTVFrame frame2 = VideoFrame2;
    if ((Width(frame1)!=Width(frame2)) || (Height(frame1)!=Height(frame2)) || (frame1->lpBMIH->biCompression!=frame2->lpBMIH->biCompression))
        return NULL;
    switch (frame1->lpBMIH->biCompression)
    {
    case BI_Y8:
        _videoSUB(GetData(frame1),GetData(frame2), Width(frame1)*Height(frame1));
        break;
    case BI_YUV9:
        _videoSUB(GetData(frame1),GetData(frame2), 9*Width(frame1)*Height(frame1)/8);
        break;
    case BI_YUV12:
        _videoSUB(GetData(frame1),GetData(frame2), 12*Width(frame1)*Height(frame1)/8);
        break;
    case BI_Y16:
        _videoSUB16((LPWORD)GetData(frame1),(LPWORD)GetData(frame2), Width(frame1)*Height(frame1));
       break;
    }
    CVideoFrame* retV=CVideoFrame::Create(frame1);
    retV->CopyAttributes(pDataFrame1);
	return retV;
}

bool Video_SUB::PrintProperties( FXString& text )
{
  FXPropertyKit pc;
  pc.WriteInt( "Offset" , m_iOffset );
  return true;
}

bool Video_SUB::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  FXPropertyKit pc( text );
  pc.GetInt( "Offset" , m_iOffset ) ;
  return true;
}

bool Video_SUB::ScanSettings( FXString& text )
{
  text = "template(Spin(Offset,-10000,10000))";
  return true;
}


IMPLEMENT_RUNTIME_GADGET_EX(Video_EQ, CTwoPinCollector, "Video.math", TVDB400_PLUGIN_NAME);

CDataFrame* Video_EQ::DoProcessing(const CDataFrame* pDataFrame1, const CDataFrame* pDataFrame2)
{
	const CVideoFrame* VideoFrame1 = pDataFrame1->GetVideoFrame(DEFAULT_LABEL);
	const CVideoFrame* VideoFrame2 = pDataFrame2->GetVideoFrame(DEFAULT_LABEL);
	if (!VideoFrame1 || !VideoFrame2 || !VideoFrame1->lpBMIH || !VideoFrame2->lpBMIH)
		return NULL;
	pTVFrame frame1 = makecopyTVFrame(VideoFrame1);
    const pTVFrame frame2 = VideoFrame2;
    if ((Width(frame1)!=Width(frame2)) || (Height(frame1)!=Height(frame2)) || (frame1->lpBMIH->biCompression!=frame2->lpBMIH->biCompression))
        return NULL;
    switch (frame1->lpBMIH->biCompression)
    {
    case BI_Y8:
    case BI_YUV9:
    case BI_YUV12:
        _videoEQ(GetData(frame1),GetData(frame2), Width(frame1)*Height(frame1));
        break;
    case BI_Y16:
        _videoEQ16((LPWORD)GetData(frame1),(LPWORD)GetData(frame2), Width(frame1)*Height(frame1));
       break;
    }
    CVideoFrame* retV=CVideoFrame::Create(frame1);
    retV->CopyAttributes(pDataFrame1);
	return retV;
}
