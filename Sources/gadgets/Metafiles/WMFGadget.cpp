// WMFGadget.cpp: implementation of the WMF class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Metafiles.h"
#include "WMFGadget.h"
#include <Gadgets\vftempl.h>
#include <gadgets\ContainerFrame.h>
#include <gadgets\VideoFrame.h>
#include <gadgets\MetafileFrame.h>
#include <gadgets\TextFrame.h>
#include <imageproc\colors.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_RUNTIME_GADGET_EX( WMF , CFilterGadget , LINEAGE_WMF , TVDB400_PLUGIN_NAME )
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

WMF::WMF()
{
  m_RectanglePen = ::CreatePen( PS_SOLID , 1 , RGB( 255 , 0 , 0 ) );
  m_pInput = new CInputConnector( vframe );
  m_pOutput = new COutputConnector( vframe );
  CreateFontWithSize( 12 );
  m_OutputMode = modeAppend;
  Resume();
}

void WMF::ShutDown()
{
  CFilterGadget::ShutDown();
  delete m_pInput;
  m_pInput = NULL;
  delete m_pOutput;
  m_pOutput = NULL;
  for ( int i = 0; i <= m_Fonts.GetUpperBound(); i++ )
  {
    if ( m_Fonts[ i ] )
    {
      m_Fonts[ i ]->DeleteObject();
      delete m_Fonts[ i ];
      m_Fonts[ i ] = NULL;
    }
  }
  ::DeleteObject( m_RectanglePen );
}

bool WMF::CreateFontWithSize( UINT iSize )
{
  CFont * pFont = new CFont;
  if ( pFont->CreateFont(
    iSize*SCALE ,                        // nHeight
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


bool WMF::Pic2Scr( const DPOINT& point , CPoint& res )
{
  res.x = ( LONG ) ( SCALE*point.x );
  res.y = ( LONG ) ( SCALE*point.y );
  return ( ( point.x >= m_PictRect.left ) &&
    ( point.y >= m_PictRect.top ) &&
    ( point.x < m_PictRect.right ) &&
    ( point.y < m_PictRect.bottom )
    );
}


CDataFrame* WMF::DoProcessing( const CDataFrame* pDataFrame )
{
  const CVideoFrame* VideoFrame = pDataFrame->GetVideoFrame( DEFAULT_LABEL );
  PASSTHROUGH_NULLFRAME( VideoFrame , pDataFrame );
  CMetafileFrame* wmf = CMetafileFrame::Create();
  wmf->CopyAttributes( pDataFrame );
  m_PictRect = CRect( 0 , 0 , VideoFrame->lpBMIH->biWidth*SCALE , VideoFrame->lpBMIH->biHeight*SCALE );
  CRgn   rgn;
  VERIFY( rgn.CreateRectRgn( m_PictRect.left , m_PictRect.top , m_PictRect.right , m_PictRect.bottom ) );

  if ( ( m_PictRect.left < 0 ) || ( m_PictRect.bottom < 0 ) )	  // shifriss: there was an exception upon closing the program cause those values were negative  
  {
    ASSERT( FALSE ); // to catch a problem A.Ch.
    wmf->Release( wmf );
    return NULL;
  }

  HDC hdc = wmf->StartDraw( m_PictRect ); /////
  ::SelectClipRgn( hdc , rgn );

  bool res = false;
  res = DrawRectangles( hdc , pDataFrame );
  res = DrawFigures( hdc , pDataFrame ) || res;
  res = DrawTexts( hdc , pDataFrame ) || res;
  res = DrawQuantity( hdc , pDataFrame ) || res;

  wmf->EndDraw();
  rgn.DeleteObject();
  if ( res )
    return wmf;
  wmf->Release( wmf );
  return NULL;
};

bool WMF::DrawRectangles( HDC dc , const CDataFrame *df )
{
  CFramesIterator* rectIter = df->CreateFramesIterator( rectangle );
  HGDIOBJ oP = NULL;
  int nmb = 0;
  if ( rectIter )
  {
    FXString atribS;
    CRectFrame* rectFrame = ( CRectFrame* ) rectIter->Next( DEFAULT_LABEL );
    while ( rectFrame )
    {
      HPEN cstPen = NULL;
      if ( rectFrame->Attributes()->GetString( "color" , atribS ) )
      {
        char *eptr;
        unsigned color = strtoul( atribS , &eptr , 16 ); //_swapRB((LPBYTE)&color);
        cstPen = ::CreatePen( PS_SOLID , 1 , color );
        oP = ::SelectObject( dc , cstPen );
      }
      else
        oP = ::SelectObject( dc , m_RectanglePen );
      CRect rc( rectFrame->left*SCALE , rectFrame->top*SCALE , rectFrame->right*SCALE , rectFrame->bottom*SCALE );
      CPoint pl[ 5 ];
      CPoint tl = rc.TopLeft();
      CPoint br = rc.BottomRight();

      pl[ 0 ] = tl;
      pl[ 1 ] = CPoint( br.x , tl.y );
      pl[ 2 ] = br;
      pl[ 3 ] = CPoint( tl.x , br.y );
      pl[ 4 ] = tl;
      ::Polyline( dc , pl , 5 );

      if ( oP )
        ::SelectObject( dc , oP );
      if ( cstPen )
      {
        ::DeleteObject( cstPen );
        cstPen = NULL;
      }
      rectFrame = ( CRectFrame* ) rectIter->Next( DEFAULT_LABEL );
      nmb++;
    }
    delete rectIter; rectIter = NULL;
    return true;
  }
  return false;
}

bool WMF::DrawFigures( HDC dc , const CDataFrame *df )
{
  CPoint pnt;
  HGDIOBJ oP = ::SelectObject( dc , m_RectanglePen );
  CFramesIterator* figIter = df->CreateFramesIterator( figure );
  if ( figIter )
  {
    FXString atribS;
    HPEN cstPen = NULL;
    CFigureFrame* cutFrame = ( CFigureFrame* ) figIter->Next( DEFAULT_LABEL );
    while ( cutFrame )
    {
      if ( !cutFrame->Attributes()->IsEmpty() )
      {
        if ( cutFrame->Attributes()->GetString( "color" , atribS ) )
        {
          char *eptr;
          unsigned color = strtoul( atribS , &eptr , 16 ); //_swapRB((LPBYTE)&color);
          cstPen = ::CreatePen( PS_SOLID , 1 , color );
          oP = SelectObject( dc , cstPen );
        }
        else
          oP = ::SelectObject( dc , m_RectanglePen );
        switch ( cutFrame->GetSize() )
        {
          case 1:
          {
            CPoint pnt;
            if ( Pic2Scr( cutFrame->GetAt( 0 ) , pnt ) )
            {
              ::MoveToEx( dc , pnt.x - 5 * SCALE , pnt.y - 5 * SCALE , NULL );
              ::LineTo( dc , pnt.x + 5 * SCALE , pnt.y + 5 * SCALE );
              ::MoveToEx( dc , pnt.x + 5 * SCALE , pnt.y - 5 * SCALE , NULL );
              ::LineTo( dc , pnt.x - 5 * SCALE , pnt.y + 5 * SCALE );
            }
            break;
          }
          case 2:
          {
            Pic2Scr( cutFrame->GetAt( 0 ) , pnt );
            ::MoveToEx( dc , pnt.x , pnt.y , NULL );
            Pic2Scr( cutFrame->GetAt( 1 ) , pnt );
            ::LineTo( dc , pnt.x , pnt.y );
            break;
          }
          case 4:
          {
            CPoint pl[ 5 ];
            Pic2Scr( cutFrame->GetAt( 0 ) , pnt );
            pl[ 0 ] = pnt;
            Pic2Scr( cutFrame->GetAt( 1 ) , pnt );
            pl[ 1 ] = pnt;
            Pic2Scr( cutFrame->GetAt( 2 ) , pnt );
            pl[ 2 ] = pnt;
            Pic2Scr( cutFrame->GetAt( 3 ) , pnt );
            pl[ 3 ] = pnt;
            Pic2Scr( cutFrame->GetAt( 0 ) , pnt );
            pl[ 4 ] = pnt;
            ::Polyline( dc , pl , 5 );
            break;
          }
          default:
          {
            if ( !cutFrame->GetSize() ) break;
            CPoint *pl = new CPoint[ cutFrame->GetSize() ];
            for ( int i = 0; i < cutFrame->GetSize(); i++ )
            {
              Pic2Scr( cutFrame->GetAt( i ) , pnt );
              pl[ i ] = pnt;
            }
            Polyline( dc , pl , (int)cutFrame->GetSize() );
            delete[] pl;
          }
        }
        if ( cstPen )
        {
          ::SelectObject( dc , oP );
          ::DeleteObject( cstPen );
          cstPen = NULL;
        }
        else
          SelectObject( dc , oP );
      }
      cutFrame = ( CFigureFrame* ) figIter->Next( DEFAULT_LABEL );
    }
    delete figIter; figIter = NULL;
    return true;
  }
  return false;
}

bool WMF::DrawTexts( HDC dc , const CDataFrame *df )
{
  CPoint pnt;
  HGDIOBJ pOldFont = NULL;
  ::SetBkMode( dc , TRANSPARENT );

  CFramesIterator* TextIter = df->CreateFramesIterator( text );
  if ( TextIter )
  {
    CTextFrame* TextFrame = ( CTextFrame* ) TextIter->Next( DEFAULT_LABEL );
    while ( TextFrame )
    {
      FXPropertyKit * pProp = TextFrame->Attributes();
      if ( !pProp->IsEmpty() )
      {
        FXString Text = TextFrame->GetString();
        int color = 0x00ff00 , x , y , iSz;
        int iSize = 12;
        if ( pProp->GetInt( "x" , x ) && pProp->GetInt( "y" , y ) )
        {
          FXString t;
          if ( pProp->GetString( "color" , t ) )
          {
            char *eptr;
            color = strtoul( t , &eptr , 16 );
            //_swapRB((LPBYTE)&color);
          }
          if ( pProp->GetInt( "Sz" , iSz ) )
            iSize = iSz;
          if ( !Text.IsEmpty() || pProp->GetString( "message" , Text ) )
          {
            COLORREF OldColor = ::GetTextColor( dc );
            if ( !pOldFont )
              pOldFont = ::SelectObject( dc , GetFont( iSize )->operator HFONT() );
            else
              ::SelectObject( dc , GetFont( iSize )->operator HFONT() );
            {
              ::SetTextColor( dc , color );
              CRect rc( CPoint( x*SCALE , y*SCALE ) , CSize( 50 * SCALE , 30 * SCALE ) );
              ::DrawText( dc , Text , (int)Text.GetLength() , &rc , DT_LEFT | DT_NOCLIP );
            }
            ::SetTextColor( dc , OldColor );
          }
        }
        //else
        //{
        // Ignore texts without coordinates data
        //pk.GetString("message",Text);
        //x = y = 5*SCALE;
        //}
        // Start drawing. Comment - do not use GDI functions! Just CMetaFileDC members allowed!
      }
      TextFrame = ( CTextFrame* ) TextIter->Next( DEFAULT_LABEL );
    }
    delete TextIter;
    if ( pOldFont )
      ::SelectObject( dc , pOldFont );
    return true;
  }
  return false;
}

bool WMF::DrawQuantity( HDC dc , const CDataFrame *df )
{
  CPoint pnt;
  HGDIOBJ pOldFont = NULL;
  ::SetBkMode( dc , TRANSPARENT );

  CFramesIterator* qIter = df->CreateFramesIterator( quantity );
  if ( qIter )
  {
    CQuantityFrame* qFrame = ( CQuantityFrame* ) qIter->Next( DEFAULT_LABEL );
    while ( qFrame )
    {
      FXPropertyKit * pProp = qFrame->Attributes();
      if ( !pProp->IsEmpty() )
      {
        int color = 0x00ff00 , x , y , iSz;
        int iSize = 12;
        if ( pProp->GetInt( "x" , x ) && pProp->GetInt( "y" , y ) )
        {
          FXString Text; Text.Format( "%d" , ( int ) *qFrame );
          FXString t;
          if ( pProp->GetString( "color" , t ) )
          {
            char *eptr;
            color = strtoul( t , &eptr , 16 );
            //_swapRB((LPBYTE)&color);
          }
          if ( pProp->GetInt( "Sz" , iSz ) )
            iSize = iSz;
          COLORREF OldColor = ::GetTextColor( dc );
          if ( !pOldFont )
            pOldFont = ::SelectObject( dc , GetFont( iSize )->operator HFONT() );
          else
            SelectObject( dc , GetFont( iSize )->operator HFONT() );
          {
            ::SetTextColor( dc , color );
            CRect rc( CPoint( x*SCALE , y*SCALE ) , CSize( 50 * SCALE , 30 * SCALE ) );
            ::DrawText( dc , Text , (int) Text.GetLength() , &rc , DT_LEFT | DT_NOCLIP );
          }
          ::SetTextColor( dc , OldColor );
        }
        //else
        //{
        // Ignore texts without coordinates data
        //pk.GetString("message",Text);
        //x = y = 5*SCALE;
        //}
        // Start drawing. Comment - do not use GDI functions! Just CMetaFileDC members allowed!
      }
      qFrame = ( CQuantityFrame* ) qIter->Next( DEFAULT_LABEL );
    }
    delete qIter;
    if ( pOldFont )
      ::SelectObject( dc , pOldFont );
    return true;
  }
  return false;
}
