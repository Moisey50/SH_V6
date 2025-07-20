// FigureFrame.cpp: implementation of the CFigureFrame class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "math/hbmath.h"
#include <gadgets\TextFrame.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

#ifndef ROUND
#define ROUND(a) ((int)round(a))
#endif

CTextFrame * CreateTextFrameEx( cmplx& Pt , LPCTSTR Text ,
  DWORD Color , int iSize )
{
  CTextFrame * pText = CTextFrame::Create() ;
  if ( pText )
  {
    pText->Attributes()->WriteInt( "x" , ROUND( Pt.real() ) ) ;
    pText->Attributes()->WriteInt( "y" , ROUND( Pt.imag() ) ) ;
    pText->Attributes()->WriteInt( "Sz" , iSize ) ;
    TCHAR Buf[ 20 ] ;
    sprintf_s( Buf , "0x%06x" , Color ) ;
    pText->Attributes()->WriteString( "color" , Buf ) ;
    pText->GetString() = Text ;
  }
  return pText ;
}

CTextFrame * CreateTextFrameEx( cmplx& Pt ,
  DWORD Color , int iSize , LPCTSTR pContentFormat , ... )
{
  FXString Content ;
  va_list argList;
  va_start( argList , pContentFormat );
  Content.FormatV( pContentFormat , argList );

  CTextFrame * pText = CTextFrame::Create( Content ) ;
  if ( pText )
  {
    pText->Attributes()->WriteInt( "x" , ROUND( Pt.real() ) ) ;
    pText->Attributes()->WriteInt( "y" , ROUND( Pt.imag() ) ) ;
    pText->Attributes()->WriteInt( "Sz" , iSize ) ;
    TCHAR Buf[ 20 ] ;
    sprintf_s( Buf , "0x%06x" , Color ) ;
    pText->Attributes()->WriteString( "color" , Buf ) ;
  }
  return pText ;
}

CTextFrame * CreateTextFrameEx( 
  LPCTSTR pLabel , LPCTSTR pContentFormat , ... )
{
  FXString Content ;
  va_list argList;
  va_start( argList , pContentFormat );
  Content.FormatV( pContentFormat , argList );

  CTextFrame * pText = CTextFrame::Create( Content ) ;
  if ( pText && pLabel )
    pText->SetLabel( pLabel ) ;

  return pText ;
}

BOOL CTextFrame::Serialize( LPBYTE pBufferOrigin , FXSIZE& CurrentWriteIndex , FXSIZE BufLen ) const
{
  FXSIZE uiLabelLen , uiInitialIndex = CurrentWriteIndex , uiAttribLen ;
  FXSIZE AdditionLen = GetSerializeLength( uiLabelLen , &uiAttribLen ) ;
  if ( ( CurrentWriteIndex + AdditionLen ) >= BufLen )
    return FALSE ;
  if ( !CDataFrame::Serialize( pBufferOrigin , CurrentWriteIndex , BufLen ) )
    return FALSE;

  LPBYTE ptr = pBufferOrigin + CurrentWriteIndex ;
  memcpy( ptr , ( LPCTSTR ) m_TextData , AdditionLen );
  CurrentWriteIndex = uiInitialIndex + AdditionLen ;
  return TRUE;
}

FXSIZE CTextFrame::GetSerializeLength( FXSIZE& uiLabelLen ,
  FXSIZE * pAttribLen ) const
{
  FXSIZE Len = CDataFrame::GetSerializeLength( uiLabelLen , pAttribLen ) ;
  Len += m_TextData.GetLength() + 1 ; // trailing zero is included
  return Len ;
};
