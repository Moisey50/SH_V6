// VideoFrameNew.cpp: implementation of the CVideoFrame class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <gadgets\VideoFrame.h>
#include <gadgets\FLWArchive.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CVideoFrame::CVideoFrame(const pTVFrame Frame)
{
	m_DataType = vframe;
	if (!Frame)
		memset((pTVFrame)this, 0, sizeof(TVFrame));
	else
	{
		memcpy((pTVFrame)this, Frame, sizeof(TVFrame));
        //ASSERT(FALSE);
		free((void*)Frame); //TODO - remove this line
	}
}

CVideoFrame::CVideoFrame(const CVideoFrame* VideoFrame)
{
    ASSERT(VideoFrame->m_DataType==vframe);
	m_DataType = vframe;
    lpBMIH=NULL; lpData=NULL;
    if (VideoFrame)
    {
        CopyAttributes(VideoFrame);
        lpBMIH=(LPBITMAPINFOHEADER)malloc(getsize4BMIH(VideoFrame));
        copy2BMIH(lpBMIH,VideoFrame);
    }
}

CVideoFrame::~CVideoFrame()
{
	if ((pTVFrame)this->lpData)
		free(this->lpData);
	else
		free(this->lpBMIH);
	memset((pTVFrame)this, 0, sizeof(TVFrame));
}

CVideoFrame* CVideoFrame::GetVideoFrame(LPCTSTR label)
{
	if (!label || m_Label == label)
		return this;
	return NULL;
}

const CVideoFrame* CVideoFrame::GetVideoFrame(LPCTSTR label) const
{
	if (!label || m_Label == label)
		return this;
	return NULL;
}

CVideoFrame* CVideoFrame::Create(pTVFrame Frame)
{
	return new CVideoFrame(Frame);
}

BOOL CVideoFrame::IsNullFrame() const
{
	return (this->lpBMIH == NULL);
}

BOOL CVideoFrame::Serialize(LPBYTE* ppData, FXSIZE* cbData) const
{
	ASSERT(ppData);
	FXSIZE cb;
	if (!CDataFrame::Serialize(ppData, &cb))
		return FALSE;
	*cbData = cb + getsize4BMIH(this);
	*ppData = (LPBYTE)realloc(*ppData, *cbData);
	LPBYTE ptr = *ppData + cb;
	if (copy2BMIH((LPBITMAPINFOHEADER)ptr, this))
		return TRUE;
	free(*ppData);
	return FALSE;
}

BOOL CVideoFrame::Serialize( LPBYTE pBufferOrigin , FXSIZE& CurrentWriteIndex , FXSIZE BufLen ) const
{
  FXSIZE uiLabelLen , uiInitialIndex = CurrentWriteIndex , uiAttribLen ;
  FXSIZE AdditionLen = GetSerializeLength( uiLabelLen , &uiAttribLen ) ;
  if ( ( CurrentWriteIndex + AdditionLen ) >= BufLen )
    return FALSE ;

  if ( !CDataFrame::Serialize( pBufferOrigin , CurrentWriteIndex , BufLen ) )
    return FALSE;
  LPBYTE ptr = pBufferOrigin + CurrentWriteIndex ;
  if ( !copy2BMIH( ( LPBITMAPINFOHEADER ) ptr , this ) )
    return FALSE;
  CurrentWriteIndex = uiInitialIndex + AdditionLen ;
  return TRUE;
}

FXSIZE CVideoFrame::GetSerializeLength( FXSIZE& uiLabelLen ,
  FXSIZE * pAttribLen ) const
{
  FXSIZE Len = CDataFrame::GetSerializeLength( uiLabelLen , pAttribLen ) ;
  Len += getsize4BMIH( this ) ;
  return Len ;
};

BOOL CVideoFrame::Restore(LPBYTE lpData, FXSIZE cbData)
{
	FXSIZE cb;
	if (!CDataFrame::Serialize(NULL, &cb) || !CDataFrame::Restore(lpData, cbData))
		return FALSE;
	LPBYTE ptr = lpData + cb;
	pTVFrame frame = makeTVFrame((LPBITMAPINFOHEADER)ptr);
	if (this->lpBMIH)
		free(this->lpBMIH);
	if (this->lpData)
		free(this->lpData);
	memcpy((pTVFrame)this, frame, sizeof(TVFrame));
	free(frame);
	return TRUE;
}
void CVideoFrame::ToLogString(FXString& Output)
{
  CDataFrame::ToLogString(Output);
  FXString TmpS;
  TmpS.Format("%s[%dx%d]", GetVideoFormatName(lpBMIH->biCompression),
    lpBMIH->biWidth, lpBMIH->biHeight);
  Output += TmpS ;
}

