// RectFrame.cpp: implementation of the CRectFrame class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <Gadgets\RectFrame.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRectFrame::CRectFrame(LPCRECT Rect)
{
	m_DataType = rectangle;
	if (!Rect)
		memset(LPRECT(this), 0, sizeof(RECT));
	else
		memcpy(LPRECT(this), Rect, sizeof(RECT));
}

CRectFrame::CRectFrame(const CRectFrame* RectFrame)
{
	m_DataType = rectangle;
	if (RectFrame)
		memcpy(LPRECT(this), LPRECT(RectFrame), sizeof(RECT));
	else
		memset(LPRECT(this), 0, sizeof(RECT));
    CopyAttributes(RectFrame);
}

CRectFrame::~CRectFrame()
{
}

CRectFrame::operator LPRECT()
{
	return (LPRECT)this;
}

CRectFrame* CRectFrame::GetRectFrame(LPCTSTR label)
{
	if (!label || m_Label==label) 
		return this;
	return NULL;
}

const CRectFrame* CRectFrame::GetRectFrame(LPCTSTR label) const
{
	if (!label || m_Label==label) 
		return this;
	return NULL;
}

CRectFrame* CRectFrame::Create(LPRECT Rect)
{
	return new CRectFrame(Rect);
}

BOOL CRectFrame::IsNullFrame() const
{
	return (LPRECT(this) == NULL);
}

BOOL CRectFrame::Serialize(LPBYTE* ppData, FXSIZE* cbData) const
{
	ASSERT(ppData);
	FXSIZE cb;
	if (!CDataFrame::Serialize(ppData, &cb))
		return FALSE;
	*cbData = cb + sizeof(RECT);
	*ppData = (LPBYTE)realloc(*ppData, *cbData);
	LPBYTE ptr = *ppData + cb;
	memcpy(ptr, LPRECT(this), sizeof(RECT));
	return TRUE;
}

BOOL CRectFrame::Serialize( LPBYTE pBufferOrigin , FXSIZE& CurrentWriteIndex , FXSIZE BufLen ) const
{
  FXSIZE uiLabelLen , uiAttribLen ;
  FXSIZE AdditionLen = GetSerializeLength( uiLabelLen , &uiAttribLen ) ;
  if ( ( CurrentWriteIndex + AdditionLen ) >= BufLen )
    return FALSE ;

  if ( !CDataFrame::Serialize( pBufferOrigin , CurrentWriteIndex , BufLen ) )
    return FALSE;
  LPBYTE ptr = pBufferOrigin + CurrentWriteIndex ;
  memcpy( ptr , LPRECT( this ) , sizeof( RECT ) );
  CurrentWriteIndex += sizeof( RECT ) ;
  return TRUE;
}

FXSIZE CRectFrame::GetSerializeLength( FXSIZE& uiLabelLen ,
  FXSIZE * pAttribLen ) const
{
  FXSIZE Len = CDataFrame::GetSerializeLength( uiLabelLen , pAttribLen ) ;
  Len += sizeof( RECT ) ; // trailing zero is included
  return Len ;
};

BOOL CRectFrame::Restore(LPBYTE lpData, FXSIZE cbData)
{
	FXSIZE cb;
	if (!CDataFrame::Serialize(NULL, &cb) || !CDataFrame::Restore(lpData, cbData))
		return FALSE;
	memcpy(LPRECT(this), lpData + cb, sizeof(RECT));
	return TRUE;
}

void CRectFrame::ToLogString(FXString& Output)
{
  CDataFrame::ToLogString(Output);
  FXString tmpS;
  tmpS.Format("(%d,%d,%d,%d) ", left , top , right , bottom );
  Output += tmpS;
}

CRectFrame * CreateRectFrameEx( CRect& Rect ,
  DWORD dwColor , int iThickness ) 
{
  CRectFrame * pRect = CRectFrame::Create( &Rect ) ;
  if ( pRect )
  {
    pRect->Attributes()->WriteLong( "color" , ( long ) dwColor ) ;
    if ( iThickness )
      pRect->Attributes()->WriteLong( "thickness" , ( long ) iThickness ) ;
  }
  return pRect ;
}

CRectFrame * CreateRectFrameEx( CRect& Rect ,
  const char * pAttributes )
{
  CRectFrame * pRect = CRectFrame::Create( &Rect ) ;
  if ( pRect )
  {
    *( pRect->Attributes() ) = pAttributes ;
  }
  return pRect ;
}

