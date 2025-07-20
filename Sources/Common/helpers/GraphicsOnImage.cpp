#pragma once

#include "stdafx.h"
#include <gadgets\gadbase.h>
#include <gadgets\containerframe.h>
#include <gadgets\videoframe.h>
#include <gadgets\textframe.h>
#include <gadgets\figureframe.h>
#include <gadgets\quantityframe.h>
#include <gadgets\arrayframe.h>
#include <gadgets\rectframe.h>
#include <gadgets\waveframe.h>
#include <helpers\GraphicsOnImage.h>
#include <iostream>
#include <fstream>

int FXGFigure::SaveToCSV(LPCTSTR pFileName , LPCTSTR pNumFormat)
{
  if ( GetCount() && pFileName && *pFileName )
  {
    ofstream myfile(pFileName , ios_base::app);
    if (myfile.is_open())
    {
      FXString Caption( _T( "    #    Value   \n" ) ) ;
      FXString Formatter ;
      if (m_bSaveTiming)
        Caption.Insert( 7,  _T("    Time Stamp"));
      Formatter.Format( "%9d,%s%s\n" ,
        (m_bSaveTiming ? "s," : "") , pNumFormat ? pNumFormat : "%g" ) ;

      myfile.write(( LPCTSTR )Caption , Caption.GetLength());

      for ( int i = 0 ; i < GetCount() ; i++ )
      {
        FXString Out , SampleTime ;
        if ( m_bSaveTiming )
        {
          FILETIME TimePt = m_Timing[i];
          SampleTime = FormatFILETIMEasString_ms( TimePt ) ;
        }
        Out.Format( Formatter , i , (LPCTSTR) SampleTime , GetAt( i ).y ) ;
        myfile.write( (LPCTSTR) Out , Out.GetLength() );
      }
      myfile.close() ;
    }

  }
  return 0;
}



// the next does checking about point position near horizontal or vertical segment
int PointNearAlignedSegment( CPoint& Pt ,  CPoint& Seg1 , CPoint& Seg2 , 
  int iTolerance , int * pDist )
{
  int iDeltaX1 = Pt.x - Seg1.x ;
  int iAbsDeltaX1 = abs( iDeltaX1 ) ;
  int iDeltaX2 = Pt.x - Seg2.x ;
  int iAbsDeltaX2 = abs( iDeltaX2 ) ;
  int iDeltaY1 = Pt.y - Seg1.y ;
  int iAbsDeltaY1 = abs( iDeltaY1 ) ;
  int iDeltaY2 = Pt.y - Seg2.y ;
  int iAbsDeltaY2 = abs( iDeltaY2 ) ;
  if ( Seg1.x == Seg2.x ) // Vertical segment ?
  {
    if ( iAbsDeltaX1 <= iTolerance )
    {
      if ( iAbsDeltaY1 <= iTolerance )
      {
        if ( pDist )
          *pDist = min( iAbsDeltaX1 , iAbsDeltaY1 ) ;
        return IS_NEAR_FIRST_EDGE ;
      }
      if ( iAbsDeltaY2 <= iTolerance )
      {
        if ( pDist )
          *pDist = min( iAbsDeltaX1 , iAbsDeltaY2 ) ;
        return IS_NEAR_SECOND_EDGE ;
      }
      if ( (iDeltaY1 > 0) ^ (iDeltaY2 > 0) )
      {
        if ( pDist )
          *pDist = iAbsDeltaX1 ;
        return IS_NEAR_SEGMENT ;
      }
    }
  }
  // Horizontal segment
  else if ( iAbsDeltaY1 <= iTolerance )
  {
    if ( iAbsDeltaX1 <= iTolerance )
    {
      if ( pDist )
        *pDist = min( iAbsDeltaX1 , iAbsDeltaY1 ) ;
      return IS_NEAR_FIRST_EDGE ;
    }
    if ( iAbsDeltaX2 <= iTolerance )
    {
      if ( pDist )
        *pDist = min( iAbsDeltaX2 , iAbsDeltaY1 ) ;
      return IS_NEAR_SECOND_EDGE ;
    }
    if ( (iDeltaX1 > 0) ^ (iDeltaX2 > 0) )
    {
      if ( pDist )
        *pDist = iAbsDeltaY1 ;
      return IS_NEAR_SEGMENT ;
    }
  }
  return NO_NEIGHBORHOOD ;
}

bool GraphicsData::LoadGraphics( const CDataFrame * df )
{
  if ( !IsFilled() && df )
  {
    double dSTartTime = GetHRTickCount() ;
    CFramesIterator* rectIter = df->CreateFramesIterator( rectangle );
    if ( rectIter )
    {
      FXString atribS;
      CRectFrame* rectFrame = (CRectFrame*) rectIter->Next( DEFAULT_LABEL );
      int iCurSelect = -1 ;
      LPCSTR hCursor = NULL ;
      for ( int i = 0 ; i < m_Rects.GetCount() ; i++ )
        m_Rects[ i ].SetMatched( false ) ;
      while ( rectFrame )
      {
        if ( !rectFrame->Attributes()->IsEmpty() )
        {
          int i = 0 ;
          for ( ; i < m_Rects.GetCount() ; i++ )
          {
            FXRectangle& Rect = m_Rects.GetAt( i ) ;
            if ( Rect.m_ObjectName == rectFrame->GetLabel() )
            {
              Rect.SetMatched( true ) ;
              Rect.m_Color = 0xff ;
              Rect.m_dwLineWidth = 1 ;
              ExtractAttributes( rectFrame->Attributes() ,
                &Rect.m_Color , (int*)&Rect.m_dwLineWidth , NULL ,
                &Rect.m_Style , &Rect.m_Back ) ;

              if ( m_iSelectedIndex == i
                && m_iSelectedType == SELECTED_RECT )
              {
                FXString SelectedName = Rect.m_ObjectName ;
              }
              else
              {
                int bSelectable = false ;
                if ( rectFrame->Attributes()->GetInt( "Selectable" , bSelectable ) )
                  Rect.SetSelectable( bSelectable != 0 ) ;
                Rect = *(RECT*) rectFrame ;  // only coordinates will be copied
              }
              break ;
            }
            //             }
          }
          if ( i >= m_Rects.GetCount() ) // the same object is not found
          {
            FXRectangle NewRect ;
            ExtractAttributes( rectFrame->Attributes() ,
              &(NewRect.m_Color) , ( int* ) &NewRect.m_dwLineWidth , NULL ,
              &NewRect.m_Style , &NewRect.m_Back ) ;
            CRect * pRC = (CRect*) rectFrame ;
            NewRect = CRect( rectFrame ) ;
            int bSelectable = false ;
            if ( rectFrame->Attributes()->GetInt( "Selectable" , bSelectable ) )
              NewRect.SetSelectable( bSelectable != 0 ) ;
            NewRect.SetObjectName( rectFrame->GetLabel() ) ;

            m_Rects.Add( NewRect ) ;
          }
        }
        rectFrame = (CRectFrame*) rectIter->Next( DEFAULT_LABEL );
      }
      delete rectIter;
    }
    for ( int i = 0 ; i < m_Rects.GetCount() ; i++ )
    {
      FXRectangle& Rect = m_Rects.GetAt( i ) ;
      if ( !Rect.GetMatched() )
      {
        m_Rects.RemoveAt( i ) ;
        i-- ;
      }
    }
    CFramesIterator* figIter = df->CreateFramesIterator( figure );
    const CFigureFrame* pFigFrame = NULL ;
    if ( (figIter != NULL) || ((pFigFrame = df->GetFigureFrame()) != NULL) )
    {
      FXString atribS;
      CPen* cstPen = NULL;
      if ( figIter )
        pFigFrame = (CFigureFrame*) figIter->Next();
      while ( pFigFrame )
      {
        if ( !pFigFrame->Attributes()->IsEmpty() )
        {
          int iNPoints = (int) pFigFrame->GetCount() ;
          if ( iNPoints )
          {
            if ( iNPoints > 1 )
            {
              FXGFigure NewFigure ;
              ExtractAttributes( pFigFrame->Attributes() ,
                &( NewFigure.m_Color ) , ( int* ) &NewFigure.m_dwLineWidth , NULL ,
                &NewFigure.m_Style , NULL ) ;

              int bSelectable = false ;
              if ( pFigFrame->Attributes()->GetInt( "Selectable" , bSelectable ) )
                NewFigure.SetSelectable( bSelectable != 0 ) ;
              NewFigure.Copy( *((CDPointArray*) pFigFrame) ) ;
              NewFigure.SetObjectName( pFigFrame->GetLabel() ) ;
              m_Figures.Add( NewFigure ) ;
            }
            else   // separate point case
            {
              CGPoint NewPoint( pFigFrame->GetAt( 0 ) ) ;
              ExtractAttributes( pFigFrame->Attributes() ,
                &( NewPoint.m_Color ) , ( int* ) &NewPoint.m_dwLineWidth ,
                &NewPoint.m_iSizeMult ,
                NULL , NULL ) ;
              int bSelectable = false ;
              if ( pFigFrame->Attributes()->GetInt( "Selectable" , bSelectable ) )
                NewPoint.SetSelectable( bSelectable != 0 ) ;
              NewPoint.SetObjectName( pFigFrame->GetLabel() ) ;
              m_Points.Add( NewPoint ) ;
            }
          }

        }
        if ( figIter )
          pFigFrame = (CFigureFrame*) figIter->Next();
        else
          break ;
      }
      if ( figIter )
        delete figIter;
    }
    CFramesIterator* TextIter = df->CreateFramesIterator( text );
    const CTextFrame* TextFrame = NULL ;
    if ( (TextIter != NULL) || ((TextFrame = df->GetTextFrame()) != NULL) )
    {
      if ( TextIter )
        TextFrame = (CTextFrame*) TextIter->Next();
      while ( TextFrame )
      {
        const FXPropertyKit * pProp = TextFrame->Attributes();
        if ( !pProp->IsEmpty() )
        {
          FXString Text = TextFrame->GetString();
          int color = 0x00ff00 , x , y , iSz , back = 1 ; // 1 means, that there is no background
          int iSize = 12;
          if ( pProp->GetInt( "x" , x ) && pProp->GetInt( "y" , y ) )
          {
            FXString t;
            if ( pProp->GetString( "color" , t ) )
              color = strtoul( t , NULL , 16 );
            else
              color = 0x000000ff ;
            if ( pProp->GetInt( "Sz" , iSz ) )
              iSize = iSz ;
            if ( pProp->GetString( "back" , t ) )
              back = strtoul( t , NULL , 16 );
            if ( !Text.IsEmpty() || pProp->GetString( "message" , Text ) )
            {
              FXGText NewText( Text , CPoint( x , y ) , iSize , color , back ) ;
              NewText.SetObjectName( TextFrame->GetLabel() ) ;
              m_Texts.Add( NewText ) ;
            }
          }
        }
        if ( !TextIter )
          break ;
        TextFrame = (CTextFrame*) TextIter->Next();
      }
      if ( TextIter )
        delete TextIter;
    }
    CFramesIterator* qIter = df->CreateFramesIterator( quantity );
    const CQuantityFrame* qFrame = NULL ;
    if ( (qIter != NULL) || ((qFrame = df->GetQuantityFrame()) != NULL) )
    {
      if ( qIter )
        qFrame = (CQuantityFrame*) qIter->Next() ;
      while ( qFrame )
      {
        const FXPropertyKit * pProp = qFrame->Attributes();
        if ( !pProp->IsEmpty() )
        {
          int color = 0x00ff00 , x , y , iSz;
          int iSize = 12;
          if ( pProp->GetInt( "x" , x ) && pProp->GetInt( "y" , y ) )
          {
            FXString Text( qFrame->ToString() ) ;
            FXString t;
            if ( pProp->GetString( "color" , t ) )
              color = strtoul( t , NULL , 16 );
            else
              color = 0x000000ff ;
            if ( pProp->GetInt( "Sz" , iSz ) )
              iSize = iSz ;
            FXGText NewText( qFrame->ToString() ,
              CPoint( x , y ) , iSize , color ) ;
            NewText.SetObjectName( qFrame->GetLabel() ) ;
            m_Texts.Add( NewText ) ;
          }
        }
        if ( !qIter )
          break ;
        qFrame = (CQuantityFrame*) qIter->Next() ;
      }  ;
      if ( qIter )
        delete qIter;
    }
    SetFilled( true ) ;

    static double dLoadGraphicsTime = GetHRTickCount() - dSTartTime ;
    return true ;
  }
  return false ;
}

bool GraphicsData::DrawRectangles( CDC*  dc , bool bInverseBW )
{
  if ( m_Rects.GetCount() )
  {
    CPen* oP = NULL;
    for ( int i = 0 ; i < m_Rects.GetCount() ; i++ )
    {
      FXRectangle& Rect = m_Rects.ElementAt( i ) ;

      DWORD dwColor = Rect.m_Color ;
      if ( bInverseBW )
      {
        if ( dwColor == 0xffffff )
          dwColor = 0 ;
        else if ( dwColor == 0 )
          dwColor = 0xffffff ;
      }

      CPen Pen( Rect.m_Style , Rect.m_dwLineWidth * ((Rect.m_bSelected) ? 2 : 1) ,
        dwColor );
      if ( oP = NULL )
        oP = dc->SelectObject( &Pen );
      else
        dc->SelectObject( &Pen );
      CPoint tl( Rect.TopLeft() );
      CPoint br( Rect.BottomRight() );
      Pic2Scr( tl );
      Pic2Scr( br );
      CPointArray Contur ;
      Contur.Add( tl ) ;
      Contur.Add( CPoint( tl.x , br.y ) ) ;
      Contur.Add( br ) ;
      Contur.Add( CPoint( br.x , tl.y ) ) ;
      Contur.Add( tl ) ;
      dc->Polyline( Contur.GetData() , (int) Contur.GetCount() ) ;
      dc->SelectObject( oP );
    }
    return true;
  }
  return false ;
}

bool GraphicsData::DrawFigures( CDC*  dc , bool bInverseBW )
{
  CPen * pOldPen = NULL ;
  CPoint pnt;
  bool bSomethingDrawn = false ;
  if ( m_Figures.GetCount() )
  {
    bSomethingDrawn = true ;
    for ( int i = 0 ; i < m_Figures.GetCount() ; i++ )
    {
      FXGFigure& Fig = m_Figures.ElementAt( i ) ;

      DWORD dwColor = Fig.m_Color ;
      if ( bInverseBW )
      {
        if ( dwColor == 0xffffff )
          dwColor = 0 ;
        else if ( dwColor == 0 )
          dwColor = 0xffffff ;
      }

      CPen NewPen( Fig.m_Style , Fig.m_dwLineWidth , dwColor );
      if ( pOldPen == NULL )
        pOldPen = dc->SelectObject( &NewPen );
      else
        dc->SelectObject( &NewPen );
      switch ( Fig.GetSize() )
      {
      case 1:
        {
          Pic2Scr( Fig.GetAt( 0 ) , pnt );
          int iShift = 2 * Fig.m_iSizeMult ;
          dc->MoveTo( pnt.x - iShift , pnt.y - iShift );
          dc->LineTo( pnt.x + iShift , pnt.y + iShift );
          dc->MoveTo( pnt.x + iShift , pnt.y - iShift );
          dc->LineTo( pnt.x - iShift , pnt.y + iShift );
          break;
        }
      case 2:
        {
          Pic2Scr( Fig.GetAt( 0 ) , pnt );
          dc->MoveTo( pnt.x , pnt.y );
          Pic2Scr( Fig.GetAt( 1 ) , pnt );
          dc->LineTo( pnt.x , pnt.y );
          break;
        }
      default:
        {
          CPointArray ScreenFig ;
          ScreenFig.SetSize( Fig.GetSize() ) ;
          for ( int i = 0 ; i < Fig.GetSize() ; i++ )
          {
            Pic2Scr( Fig.GetAt( i ) , pnt );
            ScreenFig.SetAt( i , pnt );
          }
          dc->Polyline( ScreenFig.GetData() , (int) ScreenFig.GetCount() ) ;
        }
        break ;
      }
    }
  }
  if ( m_Points.GetCount() )
  {
    bSomethingDrawn = true ;
    for ( int i = 0 ; i < m_Points.GetCount() ; i++ )
    {
      CGPoint& Pt = m_Points.ElementAt( i ) ;

      CPen NewPen( Pt.m_Style ,
        Pt.m_dwLineWidth * (Pt.IsSelected()) ? 2 : 1 , Pt.m_Color );
      if ( pOldPen == NULL )
        pOldPen = dc->SelectObject( &NewPen );
      else
        dc->SelectObject( &NewPen );
      Pic2Scr( (CDPoint) Pt , pnt );
      int iShift = 2 * Pt.m_iSizeMult ;
      dc->MoveTo( pnt.x - iShift , pnt.y - iShift );
      dc->LineTo( pnt.x + iShift , pnt.y + iShift );
      dc->MoveTo( pnt.x + iShift , pnt.y - iShift );
      dc->LineTo( pnt.x - iShift , pnt.y + iShift );
    }
  }

  if ( pOldPen )
    dc->SelectObject( pOldPen ) ;
  return bSomethingDrawn ;
}

bool GraphicsData::DrawTexts( CDC*  dc , bool bInverseBW )
{
  if ( m_Texts.GetCount() )
  {
    CPoint pnt;
    CFont* pOldFont = NULL;
    COLORREF OldColor = dc->GetTextColor() ;
    COLORREF OldBack = dc->GetBkColor() ;
    int OldBackMode = dc->GetBkMode() ;
    for ( int i = 0 ; i < m_Texts.GetCount() ; i++ )
    {
      FXGText& Text = m_Texts.GetAt( i ) ;
      if ( !pOldFont )
        pOldFont = dc->SelectObject( GetFont( Text.m_iSz ) );
      else
        dc->SelectObject( GetFont( Text.m_iSz ) );
      {
        DWORD dwColor = Text.m_Color ;
        if ( bInverseBW )
        {
          if ( dwColor == 0xffffff )
            dwColor = 0 ;
          else if ( dwColor == 0 )
            dwColor = 0xffffff ;
        }
        dc->SetTextColor( dwColor );
        if ( Text.m_bUseBackGround || (Text.m_Back != 1) )
        {
          dc->SetBkMode( OPAQUE ) ;
          dc->SetBkColor( Text.m_Back ) ;
        }
        else
          dc->SetBkMode( TRANSPARENT ) ;

        CPoint toff( Text.m_Coord );
        Pic2Scr( toff );
        CRect rc( toff , CSize( Pic2Scr( 50 ) , Pic2Scr( 30 ) ) );
        dc->DrawText( Text , (int) Text.GetLength() , &rc , DT_LEFT | DT_NOCLIP );
      }
    }

    dc->SetTextColor( OldColor );
    dc->SetBkColor( OldBack ) ;
    dc->SetBkMode( OldBackMode ) ;
    if ( pOldFont )
      dc->SelectObject( pOldFont );
  }
  return true;
}

int  GraphicsData::Pic2Scr( double len )
{
  return (int) ((len + 0.5)*m_ScrScale + 0.5);
}

int  GraphicsData::Scr2Pic( double len )
{
  return (int) (len / m_ScrScale/*+0.5*/);
}

void GraphicsData::Scr2Pic( CPoint &point )
{
  if ( m_ScrScale < 0.01 ) { point = CPoint( 0 , 0 ); return; }
  point.x -= m_ScrOffset.x;
  point.y -= m_ScrOffset.y;
  point.x = (int) (point.x / m_ScrScale/*+0.5*/);
  point.y = (int) (point.y / m_ScrScale/*+0.5*/);

  point -= m_IntBmOffset;
}

void GraphicsData::SubPix2Scr( CPoint &point )
{
  point += m_IntBmOffset;
  if ( m_ScrScale < 0.01 ) { point = CPoint( 0 , 0 ); return; }
  point.x = (int) ((point.x + 0.5)*m_ScrScale + 0.5);
  point.y = (int) ((point.y + 0.5)*m_ScrScale + 0.5);
  point.x += m_ScrOffset.x;
  point.y += m_ScrOffset.y;
}

void GraphicsData::Pic2Scr( CPoint &point )
{
  point += m_IntBmOffset;
  if ( m_ScrScale < 0.01 ) { point = CPoint( 0 , 0 ); return; }
  point.x = (int) (point.x*m_ScrScale + 0.5);
  point.y = (int) (point.y*m_ScrScale + 0.5);
  point.x += m_ScrOffset.x;
  point.y += m_ScrOffset.y;
}

bool GraphicsData::Pic2Scr( DPOINT& pt , CPoint& res )
{
  if ( m_ScrScale < 0.01 )
    res = CPoint( 0 , 0 );
  else
  {
    CDPoint point( pt ) ;
    point.x += m_IntBmOffset.x + 0.5 ;
    point.y += m_IntBmOffset.y + 0.5 ;
    //     point.x *= m_ScrScale ;
    //     point.y *= m_ScrScale ;
    point = point * m_ScrScale ;
    point.x += m_ScrOffset.x ;
    point.y += m_ScrOffset.y ;
    //     if ( point.y < 0. )
    //       point.y = 0. ;
    res = CPoint( ROUND( point.x ) , ROUND( point.y ) ) ;
  }
  return true;
}

bool GraphicsData::CreateFontWithSize( UINT iSize )
{
  CFont * pFont = new CFont;
  if ( pFont->CreateFont(
    (int) (iSize * 1.5 + 0.5) ,              // nHeight
    0 ,                         // nWidth
    0 ,                         // nEscapement
    0 ,                         // nOrientation
    FW_NORMAL ,                 // nWeight
    FALSE ,                     // bItalic
    FALSE ,                     // bUnderline
    0 ,                         // cStrikeOut
    ANSI_CHARSET ,              // nCharSet
    OUT_DEFAULT_PRECIS ,        // nOutPrecision
    CLIP_DEFAULT_PRECIS ,       // nClipPrecision
    DEFAULT_QUALITY ,           // nQuality
    DEFAULT_PITCH | FF_SWISS ,  // nPitchAndFamily
    _T( "Arial" ) ) )              // lpszFacename 
  {
    m_Fonts.Add( pFont );
    m_Sizes.Add( iSize );
    return true;
  }
  return false;
}

CFont * GraphicsData::GetFont( UINT uiSize )
{
  for ( int i = 0 ; i <= m_Fonts.GetUpperBound(); i++ )
  {
    if ( m_Sizes[ i ] == uiSize ) return m_Fonts[ i ];
  }
  if ( CreateFontWithSize( uiSize ) )
    return m_Fonts[ m_Fonts.GetUpperBound() ];
  else
    return NULL ;
}

bool GraphicsData::DrawTexts( HDC hdc , bool bInverseBW )
{
  CDC DC ;
  DC.Attach( hdc ) ;
  bool bRes = DrawTexts( &DC , bInverseBW ) ;
  DC.Detach() ;
  return bRes ;
}
bool GraphicsData::DrawFigures( HDC hdc , bool bInverseBW )
{
  CDC DC ;
  DC.Attach( hdc ) ;
  bool bRes = DrawFigures( &DC , bInverseBW ) ;
  DC.Detach() ;
  return bRes ;
}

bool GraphicsData::DrawRectangles( HDC hdc , bool bInverseBW )
{
  CDC DC ;
  DC.Attach( hdc ) ;
  bool bRes = DrawRectangles( &DC , bInverseBW ) ;
  DC.Detach() ;
  return bRes ;
}

bool GraphicsData::CorrectRect( CRect& SelectedRect ,
  CPoint SrcPt , CPoint CursorPnt , SelectionByCursor Side )
{
//   SubPix2Scr( CursorPnt ) ;
//   SubPix2Scr( SrcPt ) ;
  CSize Offset = CursorPnt - SrcPt ;
  switch (Side)
  {
    case CURSOR_ON_WHOLE_SELECTION:
      SelectedRect.OffsetRect( Offset ) ;
      break ;
    case CURSOR_ON_LTCORNER:
      SelectedRect.left += Offset.cx ;
      SelectedRect.top += Offset.cy ;
      break ;
    case CURSOR_ON_TOP_BORDER:
      SelectedRect.top += Offset.cy ;
      break ;
    case CURSOR_ON_TRCORNER:
      SelectedRect.right += Offset.cx ;
      SelectedRect.top += Offset.cy ;
      break ;
    case CURSOR_ON_RIGHT_BORDER:
      SelectedRect.right += Offset.cx ;
      break ;
    case CURSOR_ON_RBCORNER:
      SelectedRect.right += Offset.cx ;
      SelectedRect.bottom += Offset.cy ;
      break ;
    case CURSOR_ON_BOTTOM_BORDER:
      SelectedRect.bottom += Offset.cy ;
      break ;
    case CURSOR_ON_BLCORNER:
      SelectedRect.left += Offset.cx ;
      SelectedRect.bottom += Offset.cy ;
      break ;
    case CURSOR_ON_LEFT_BORDER:
      SelectedRect.left += Offset.cx ;
      break ;
    default:
      return false ;
  }
  return true ;
}

DWORD ExtractAttributes( const FXPropertyKit * pAttributes ,
  DWORD * pColor , int * piThickness , int * piSz , 
  int * piStyle , DWORD * pBack ) 
{
  DWORD dwResult = 0 ;
  FXString Tmp ;

  if ( pColor )
  {
    if ( pAttributes->GetString( "color" , Tmp ) )
    {
      FXSIZE fxscolor ;
      ConvToBinary( Tmp , fxscolor ) ;
      *pColor = ( DWORD ) fxscolor ;
      dwResult |= AE_Color ;
    }
  }
  if ( piThickness )
  {
    if ( pAttributes->GetInt( "thickness" , *piThickness ) )
      dwResult |= AE_Thickness ;
  }
  if ( piSz )
  {
    if ( pAttributes->GetInt( "Sz" , *piSz ) )
      dwResult |= AE_Sz ;
  }
  if ( piStyle )
  {
    if ( pAttributes->GetInt( "style" , *piStyle ) )
      dwResult |= AE_Style ;
  }
  if ( pBack )
  {
    if ( pAttributes->GetString( "color" , Tmp ) )
    {
      FXSIZE Result ;
      ConvToBinary( Tmp , Result ) ;
      *pBack = ( DWORD ) Result ;
      dwResult |= AE_Back ;
    }
  }
  return dwResult ;
}
