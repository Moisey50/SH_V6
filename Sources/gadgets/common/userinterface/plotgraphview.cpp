// E:\Works\Image Processing II\common\userinterface\PlotGraphView.cpp : implementation file
//

#include "stdafx.h"
#include "TVStatistics.h"
#include <userinterface\PlotGraphView.h>
#include <gadgets\arrayframe.h>
#include <gadgets\quantityframe.h>
#include <gadgets\textframe.h>
#include <helpers\propertykitEx.h>
#include <helpers\FramesHelper.h>
#include <iostream>
#include <fstream>

#define SCALEF 1.5

#define ARR_SIZE(x) (sizeof(x)/sizeof(x[0]))

static COLORREF GraphColors[] =
{
  RGB( 255 , 0 , 0 ) ,
  RGB( 0 , 255 , 0 ) ,
  RGB( 80 , 80 , 255 ) ,
  RGB( 255 , 255 , 255 ) ,
  RGB( 255 , 255 , 0 ) ,
  RGB( 0 , 255 , 255 ) ,
  RGB( 255 , 0 , 255 ) ,
  RGB( 128 , 128 , 128 )
};

int ParseNumber( FXString& pAsText )
{
  int iOutValue = 0;
  if ( pAsText[ 0 ] == '0' )
  {
    switch ( tolower( pAsText[ 1 ] ) )
    {
    case 'x':
      {
        if ( _stscanf_s( ((LPCTSTR) pAsText) + 2 , _T( "%x" ) , &iOutValue ) != 1 )
          iOutValue = 0;
      }
      break;
    case 'b':
      {
        for ( int i = 0; i < pAsText.GetLength() - 2; i++ )
        {
          iOutValue <<= 1;
          iOutValue |= (pAsText[ i + 2 ] == '1');
        }
      }
      break;
    }

  }
  else if ( isxdigit( pAsText[ 0 ] ) )
  {
    if ( _stscanf_s( ((LPCTSTR) pAsText) , _T( "%x" ) , &iOutValue ) != 1 )
      iOutValue = 0;
  }
  else
    iOutValue = atoi( pAsText );

  return iOutValue;
}

#define SZ_INT sizeof(int)
#define SZ_DBL sizeof(double)
#define SZ_DPOINT sizeof(DPOINT)
#define SZ_GENQ sizeof(GENERICQUANTITY)

void AppendFigure( CFigure& Figure , const CArrayFrame* pArrayFrame )
{
  int count = pArrayFrame->GetCount() , i;
  void* pData = pArrayFrame->GetData();
  switch ( pArrayFrame->GetElementSize() )
  {
  case SZ_INT:
    {
      int* y = (int*) pData;
      DPOINT dp;
      for ( i = 0; i < count; i++ )
      {
        dp.x = (double) i;
        dp.y = (double) y[ i ];
        Figure.AddPoint( dp );
      }
    }
    break;
  case SZ_DBL:
    {
      double* y = (double*) pData;
      DPOINT dp;
      for ( i = 0; i < count; i++ )
      {
        dp.x = (double) i;
        dp.y = y[ i ];
        Figure.AddPoint( dp );
      }
    }
    break;
  case SZ_DPOINT:
    {
      for ( i = 0; i < count; i++ )
      {
        Figure.AddPoint( ((DPOINT*) pData)[ i ] );
      }
    }
    break;
  case SZ_GENQ:
    {
      LPGENERICQUANTITY gq = (LPGENERICQUANTITY) pData;
      DPOINT dp;
      for ( int i = 0; i < count; i++ )
      {
        if ( gq[ i ]._type == CGenericQuantity::GQ_COMPLEX )
        {
          dp.x = gq[ i ]._c.x;
          dp.y = gq[ i ]._c.y;
        }
        else
        {
          dp.x = (double) i;
          dp.y = ((gq[ i ]._type == CGenericQuantity::GQ_FLOATING) ? gq[ i ]._d : (double) gq[ i ]._i);
        }
        Figure.AddPoint( dp );
      }
    }
    break;
  }
}

// CPlotGraphView

IMPLEMENT_DYNAMIC( CPlotGraphView , CWnd )

CPlotGraphView::CPlotGraphView( LPCTSTR name ) :
  m_Name( name )
  //       m_Pen(PS_SOLID,1,RGB(0,255,0)),
  , m_bAutoFit( FALSE )
  , m_iTakenDrawings( 0 )
  , m_bViewNames( TRUE )
  , m_bViewRanges( TRUE )
  , m_iViewNet( 9 )
  , m_dSamplePeriod_ms( 0. )
  , m_uiTimerID( 0 )
{
  memset( &m_Labels.m_dXMin , 0 , sizeof( m_Labels.m_dXMin ) * 4 );
}

CPlotGraphView::~CPlotGraphView()
{
  for ( int i = 0; i < m_Fonts.GetCount(); i++ )
  {
    delete m_Fonts[ i ];
  }
}


BEGIN_MESSAGE_MAP( CPlotGraphView , CWnd )
  ON_WM_PAINT()
  ON_WM_MOUSEMOVE()
  ON_WM_TIMER()
  ON_WM_SIZE()
END_MESSAGE_MAP()

// CPlotGraphView message handlers

BOOL CPlotGraphView::Create( CWnd* pParentWnd , DWORD dwAddStyle , UINT nID )
{
  if ( m_hWnd ) return(FALSE);
  BOOL RESULT;
  RECT rect;

  pParentWnd->GetClientRect( &rect );
  LPCTSTR lpszDIBVClass = AfxRegisterWndClass(
    CS_HREDRAW | CS_VREDRAW | CS_PARENTDC , LoadCursor( NULL , IDC_ARROW ) ,
    (HBRUSH) ::GetStockObject( BLACK_BRUSH ) );
  RESULT = CWnd::Create( lpszDIBVClass , "PlotGraphView" ,
    WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | dwAddStyle , rect , pParentWnd , nID );
  if ( RESULT )
  {
    Invalidate();
  }
  return (RESULT);
}

void CPlotGraphView::OnPaint()
{
  CWnd * pParent = GetParent();
  if ( !pParent )
    return;
  CRect ParentClientRect , ThisWindowRect;
  pParent->GetClientRect( &ParentClientRect );
  GetWindowRect( &ThisWindowRect );
  if ( (ParentClientRect.Width() != ThisWindowRect.Width())
    || ParentClientRect.Height() != ThisWindowRect.Height() )
  {
    SetWindowPos( NULL , ParentClientRect.left ,
      ParentClientRect.top , ParentClientRect.right ,
      ParentClientRect.bottom , SWP_NOZORDER );
  }
  bool bInverse = false ;

  CPaintDC dc( this );
  FXAutolock al( m_DataLock );
  GetClientRect( &m_ClientRect );
  dc.FillSolidRect( &m_ClientRect , RGB( 0 , 0 , 0 ) );
  CPen * OldPen = NULL;
  CFont * pOldFont = NULL;

  if ( !m_Graphics.m_Figures.GetCount() )
  {
    //     COLORREF PenColors[] = { RGB(0, 255, 0), RGB(255, 255, 0), RGB(255, 0, 0), RGB(128, 0, 255) };
    //     int nColors = sizeof(PenColors) / sizeof(COLORREF);
    //     int iCurColor = 0;
    // 
    //     if ((m_Data.GetSize()==0) || (m_GraphFrame.IsRectEmpty()))
    //       return;
    // 
    //     double min = (m_bAutoFit) ? m_GraphFrame.bottom : m_Min;
    //     double height = (m_bAutoFit) ? m_GraphFrame.Height() : (m_Max - m_Min);
    //     m_ScaleX=((double)m_ClientRect.Width()-1)/m_GraphFrame.Width();
    //     m_ScaleY=((double)m_ClientRect.Height()-1)/height;
    //     OldPen = (CPen*)dc.SelectObject(m_GDI.GetPen(PS_SOLID, 1, PenColors[iCurColor]));
    // 
    //     int i=0,j=0;
    //     CPoint *pl= new CPoint[m_Data.GetSize()];
    //     pl[i]=CPoint((int)((m_Data.GetAt(0).x-m_GraphFrame.left)*m_ScaleX+0.5),
    //       m_ClientRect.bottom-(int)((m_Data.GetAt(0).y-min)*m_ScaleY+0.5));
    //     i++; j++;
    //     while (i<m_Data.GetSize())
    //     {
    //       while ((i<m_Data.GetSize()) && (m_Data.GetAt(i).x >= m_Data.GetAt(i - 1).x))
    //       {
    //         pl[j]=CPoint((int)((m_Data.GetAt(i).x-m_GraphFrame.left)*m_ScaleX+0.5),
    //           m_ClientRect.bottom-(int)((m_Data.GetAt(i).y-min)*m_ScaleY+0.5));
    //         i++; j++;
    //       }
    //       dc.Polyline(pl,j);
    //       iCurColor = (iCurColor + 1) % nColors;
    //       dc.SelectObject(m_GDI.GetPen(PS_SOLID, 1, PenColors[iCurColor]));
    //       i++; j=0;
    //     }
    //     delete [] pl;
  }
  else
  {
    CPointArray Points;
    for ( int iFigCnt = 0; iFigCnt < m_Graphics.m_Figures.GetCount(); iFigCnt++ )
    {
      double dOffX = 0. , dOffY = 0.;
      CXYMinMaxes MinMaxes = m_MinMaxes;
      FXGFigure& Fig = m_Graphics.m_Figures[ iFigCnt ];
      CDPointArray& Pts = Fig;
      if ( Pts.GetCount() )
      {
        if ( m_bAutoFit )
        {
          MinMaxes.m_dXMin = (Fig.m_WhatDataToUse == USE_Y) ? 0. : Fig.m_MinMaxes.m_dXMin;
          MinMaxes.m_dXMax = (Fig.m_WhatDataToUse == USE_Y) ? Fig.GetCount() : Fig.m_MinMaxes.m_dXMax;
          MinMaxes.m_dYMin = (Fig.m_WhatDataToUse == USE_X) ? 0. : Fig.m_MinMaxes.m_dYMin;
          MinMaxes.m_dYMax = (Fig.m_WhatDataToUse == USE_X) ? Fig.GetCount() : Fig.m_MinMaxes.m_dYMax;
        }
        double dDataWidth = MinMaxes.dAmplX();
        double dDataHeight = MinMaxes.dAmplY();
        double dGraphWidth = m_ClientRect.Width() * Fig.m_DrawArea.Width();
        double dGraphHeight = fabs( m_ClientRect.Height() * Fig.m_DrawArea.Height() );
        if ( dGraphHeight == 0. || dGraphWidth == 0. )
          continue;
        CRect DrawArea( ROUND( m_ClientRect.Width() * Fig.m_DrawArea.left ) ,
          ROUND( m_ClientRect.Height() * Fig.m_DrawArea.top ) ,
          ROUND( m_ClientRect.Width() * Fig.m_DrawArea.right ) ,
          ROUND( m_ClientRect.Height() * Fig.m_DrawArea.bottom ) );
        m_ScaleX = (dDataWidth == 0.) ? 1. : dGraphWidth / dDataWidth;
        m_ScaleY = (dDataHeight == 0.) ? 1. : dGraphHeight / dDataHeight;
        double dOffX = m_ClientRect.Width() * Fig.m_DrawArea.left - (MinMaxes.m_dXMin * m_ScaleX);
        double dOffY = fabs( m_ClientRect.Height() * Fig.m_DrawArea.bottom ) + ((MinMaxes.m_dYMin/* - 5*/)* m_ScaleY);
        CPen NewPen( PS_SOLID , Fig.m_dwLineWidth , Fig.m_Color );
        if ( !OldPen )
          OldPen = (CPen*) dc.SelectObject( &NewPen );
        else
          dc.SelectObject( &NewPen );

        if ( Points.GetCount() < Pts.GetCount() )
          Points.SetSize( Pts.GetCount() );
        int i = 0;
        for ( ; i < Pts.GetCount(); i++ )
        {
          double dXBound = RetBound( Pts[ i ].x , MinMaxes.m_dXMin , MinMaxes.m_dXMax );
          double dYBound = RetBound( Pts[ i ].y , MinMaxes.m_dYMin , MinMaxes.m_dYMax );
          Points[ i ] = CPoint(
            ROUND( dOffX + (dXBound * m_ScaleX) ) ,
            ROUND( dOffY - (dYBound * m_ScaleY) ) );
        }
        if ( i > 1 )
          dc.Polyline( Points.GetData() , (int) Pts.GetCount() );
        if ( m_bViewNames )
        {
          int iFontNumber = ROUND( dGraphHeight / 50 );
          if ( !pOldFont )
            pOldFont = dc.SelectObject( GetFont( iFontNumber ) );
          else
            dc.SelectObject( GetFont( ROUND( dGraphHeight / 50 ) ) );

          dc.SetTextColor( Fig.m_Color );
          CPoint toff( 5 , (iFigCnt * iFontNumber * 3) / 2 );
          CRect rc( toff , CSize( 50 , iFontNumber ) );
          dc.DrawText( Fig.m_ObjectName , (int) Fig.m_ObjectName.GetLength() ,
            &rc , DT_LEFT | DT_NOCLIP );
        }
        bool bInverse = (Fig.m_PlaceMode == PMODE_APPEND) ;
      }
    }
  }
  double dGraphWidth = m_ClientRect.Width();
  double dGraphHeight = fabs( m_ClientRect.Height() );
  CRect DrawArea( m_ClientRect );
  dc.SetTextColor( 0xc0c0c0 );

  if ( m_iViewNet )
  {
    CPen NewPen( PS_SOLID , 1 , 0xc0c0c0 );
    if ( !OldPen )
      OldPen = (CPen*) dc.SelectObject( &NewPen );
    else
      dc.SelectObject( &NewPen );
    double dLineDistX = dGraphWidth / (m_iViewNet + 1);
    double dLineDistY = dGraphHeight / (m_iViewNet + 1);
    for ( int i = 0; i < m_iViewNet; i++ )
    {
      CPoint Up( DrawArea.left + ROUND( dLineDistX * (i + 1) ) , DrawArea.top );
      dc.MoveTo( Up );
      CPoint Down( Up.x , DrawArea.bottom );
      dc.LineTo( Down );
      CPoint Left( DrawArea.left , DrawArea.top + ROUND( dLineDistY * (i + 1) ) );
      dc.MoveTo( Left );
      CPoint Right( DrawArea.right , Left.y );
      dc.LineTo( Right );
    }
    CPoint tOff( ROUND( m_ClientRect.Width() * 0.88 ) , 5 );
    CRect rc( tOff , CSize( 50 , 15 ) );
    FXString Text;
    Text.Format( "(%g,%g)" , m_MinMaxes.m_dXMax , m_MinMaxes.m_dYMax );
    dc.DrawText( Text , (int) Text.GetLength() , &rc , DT_LEFT | DT_NOCLIP );

    CXYMinMaxes Labels = m_Labels;
    double dDistX = Labels.m_dXMax - Labels.m_dXMin;
    if ( dDistX != 0. )
    {
      double dStep = dDistX / (m_iViewNet + 1);
      for ( int i = 0; i < m_iViewNet + 1; i++ )
      {
        double dVal = (bInverse) ? -(m_Labels.m_dXMax - dStep * i) : Labels.m_dXMin + dStep * i;
        Text.Format( "%7.1f" , dVal );
        CPoint LT( DrawArea.left + ROUND( dLineDistX * i ) + 2 ,
          DrawArea.bottom - 20 );
        CRect rc( LT , CSize( 40 , 15 ) );
        dc.DrawText( Text , (int) Text.GetLength() , &rc , DT_LEFT | DT_NOCLIP );
      }
    }
    if ( m_dSamplePeriod_ms != 0. )
    {
      CRect rc( DrawArea.left + 2 , DrawArea.bottom - 20 , DrawArea.left + 40 , DrawArea.bottom - 2 ) ;
      dc.DrawText( "Seconds" , 7 , &rc , DT_LEFT | DT_NOCLIP );
    }
    double dDistY = Labels.m_dYMax - Labels.m_dYMin;
    if ( dDistY != 0. )
    {
      double dStep = dDistY / (m_iViewNet + 1);
      for ( int i = 0; i < m_iViewNet + 1; i++ )
      {
        double dVal = Labels.m_dYMin + dStep * i;
        Text.Format( "%7.1f" , dVal );
        CPoint LT( DrawArea.right - 41 ,
          DrawArea.bottom - 20 - ROUND( dLineDistY * i ) );
        CRect rc( LT , CSize( 40 , 15 ) );
        dc.DrawText( Text , (int) Text.GetLength() , &rc , DT_LEFT | DT_NOCLIP );
      }
    }
  }

  if ( m_bViewRanges )
  {
    CPoint tOff( ROUND( m_ClientRect.Width() * 0.7 ) , 5 );
    CRect rc( tOff , CSize( 50 , 15 ) );
    FXString Text;
    Text.Format( "(%g,%g)" , m_MinMaxes.m_dXMax , m_MinMaxes.m_dYMax ) ,
      dc.DrawText( Text , (int) Text.GetLength() , &rc , DT_LEFT | DT_NOCLIP );
  }

  if ( OldPen )
    dc.SelectObject( OldPen );
  if ( pOldFont )
    dc.SelectObject( pOldFont );
}

BOOL    CPlotGraphView::GetMouseXY( CDPoint& dpnt )
{
  FXAutolock al( m_DataLock );
  POINT pt;
  if ( ::GetCursorPos( &pt ) )
  {
    dpnt.x = dpnt.y = 0.0;
    if ( (WindowFromPoint( pt )->m_hWnd == this->m_hWnd) )
    {
      dpnt.x = m_MousePnt.x / m_ScaleX;
      dpnt.y = (((double) m_ClientRect.Height()) - m_MousePnt.y) / m_ScaleY;
      return true;
    }
  }
  return false;
}

void CPlotGraphView::ParseAttributes( const CDataFrame * pFrame , FXGFigure *  pTargetFigure )
{
  const FXPropertyKit * pk = pFrame->Attributes();
  if ( !pk->IsEmpty() )
  {
    FXString Tmp;
    if ( pk->GetString( _T( "color" ) , Tmp ) )
      pTargetFigure->m_Color = ParseNumber( Tmp );
    int iTmp;
    if ( pk->GetInt( _T( "width" ) , iTmp ) )
      pTargetFigure->m_dwLineWidth = iTmp;
    if ( pk->GetInt( _T( "drawmode" ) , iTmp ) )
      pTargetFigure->m_DrawMode = (FIG_DRAW_MODE) iTmp;
    if ( pk->GetInt( _T( "drawdir" ) , iTmp ) )
      pTargetFigure->m_DrawDir = (FIG_DRAW_DIR) iTmp;

    if ( pk->GetInt( _T( "reverse" ) , iTmp ) )
      pTargetFigure->m_bReverse = (iTmp != 0);
    if ( pk->GetInt( _T( "timing" ) , iTmp ) )
      pTargetFigure->EnableTimingSave( iTmp != 0 );
    CDRect Area;
    if ( GetArray( *pk , _T( "drawarea" ) , _T( 'f' ) , 4 , &Area ) == 4 )
    {
      pTargetFigure->m_DrawArea = Area;
      Bound( pTargetFigure->m_DrawArea.left , 0. , 1.0 );
      Bound( pTargetFigure->m_DrawArea.top , 0. , 1.0 );
      Bound( pTargetFigure->m_DrawArea.right , 0. , 1.0 );
      Bound( pTargetFigure->m_DrawArea.bottom , 0. , 1.0 );
      pTargetFigure->m_DrawArea.NormalizeRect();
      if ( pTargetFigure->m_DrawArea.IsRectEmpty() )
      {
        pTargetFigure->m_DrawArea.top = pTargetFigure->m_DrawArea.left = 0.0;
        pTargetFigure->m_DrawArea.bottom = pTargetFigure->m_DrawArea.right = 1.0;
      }
    }
    CXYMinMaxes MinMaxes;
    if ( GetArray( *pk , _T( "minmaxes" ) , _T( 'f' ) , 4 , &MinMaxes ) == 4 )
    {
      MinMaxes.Normalize();
      pTargetFigure->m_MinMaxes = MinMaxes;
    }
    if ( pk->GetString( _T( "use" ) , Tmp ) )
    {
      Tmp = Tmp.MakeLower() ;
      if ( Tmp == _T( "x" ) )
      {
        pTargetFigure->m_WhatDataToUse = USE_X ;
        pTargetFigure->m_SecondAxisMode = BASE_MODE_STEP_ONE ;
      }
      else if ( Tmp == _T( "y" ) )
      {
        pTargetFigure->m_WhatDataToUse = USE_Y ;
        pTargetFigure->m_SecondAxisMode = BASE_MODE_STEP_ONE ;
      }
      else if ( Tmp == _T( "xy" ) )
      {
        pTargetFigure->m_WhatDataToUse = USE_XY ;
        pTargetFigure->m_SecondAxisMode = BASE_MODE_2D ;
      }
    }
  }
}

PlacementMode CPlotGraphView::AnalyzeLabel(
  const CDataFrame * pFrame , FXString& DrawingName )
{
  PlacementMode PlaceMode = PMODE_REPLACE;
  FXString Label = pFrame->GetLabel();
  if ( !Label.IsEmpty() )
  {
    int iSemicolonPos = (int) Label.Find( _T( ':' ) );
    if ( iSemicolonPos > 0 )
    {
      if ( Label.Find( _T( "Append:" ) ) >= 0 )
        PlaceMode = PMODE_APPEND;
      else if ( Label.Find( _T( "Insert:" ) ) >= 0 )
        PlaceMode = PMODE_INSERT;
      else if ( Label.Find( _T( "Replace:" ) ) >= 0 )
        PlaceMode = PMODE_REPLACE;
      else if ( Label.Find( _T( "Remove:" ) ) >= 0 )
        PlaceMode = PMODE_REMOVE;
      else if ( Label.Find( _T( "RemoveAll:" ) ) >= 0 )
      {
        PlaceMode = PMODE_REMOVE_ALL;
        m_bRemoveAllGraphs = true;
      }
      DrawingName = Label.Mid( iSemicolonPos + 1 );
    }
    else
      DrawingName = Label;
  }

  if ( DrawingName.IsEmpty() )
  {
    //    DrawingName.Format( _T("Unknown%d") , m_iTakenDrawings++ ) ;
    DrawingName = _T( "Unknown" );
  }

  return PlaceMode;
}
BOOL CPlotGraphView::AddOrCorrectFigure( const CDataFrame * pFrameFig )
{
  FXString DrawingName;
  const CFigureFrame * pFrame = pFrameFig->GetFigureFrame();
  PlacementMode PlaceMode = AnalyzeLabel( pFrame , DrawingName );
  int iIndex = -1 ;
  FXGFigure * pFig = m_Graphics.GetFigureByName( DrawingName , &iIndex );
  bool bTimedViewMode = (m_dSamplePeriod_ms != 0.) ;
  if ( pFig )
  {
    ParseAttributes( pFrame , pFig ) ;
    switch ( PlaceMode )
    {
    case PMODE_INSERT:
      {
        if ( !bTimedViewMode )
        {
          const CDPointArray * pNewArray = (const CDPointArray*) pFrame;
          CDPointArray * pArray = (CDPointArray *) pFig;
          pArray->InsertAt( 0 , pNewArray );
          if ( pFig->GetCount() > m_iNSamples )
            pFig->SetSize( m_iNSamples );
        }
        else
        {
          if ( (int) m_LastValues.size() <= iIndex )
            m_LastValues.resize( iIndex + 1 ) ;
          m_LastValues[ iIndex ] = CDPointToCmplx( ((const CDPointArray*) pFrame)->GetAt( 0 ) ) ;
        }
      }
      break;
    case PMODE_APPEND:
      if ( !bTimedViewMode )
      {
        pFig->Append( *pFrame );
        if ( pFig->GetCount() > m_iNSamples )
          pFig->RemoveAt( 0 , pFig->GetCount() - m_iNSamples );
      }
      else
      {
        if ( (int) m_LastValues.size() <= iIndex )
          m_LastValues.resize( iIndex + 1 ) ;
        m_LastValues[ iIndex ] = CDPointToCmplx( ((const CDPointArray*) pFrame)->GetAt( 0 ) ) ;
      }
      break;
    case PMODE_REPLACE:
      pFig->Copy( *pFrame );
      break;
    case PMODE_REMOVE:
      m_Graphics.RemoveFigureByName( DrawingName );
      return true;
    case PMODE_REMOVE_ALL:
      m_bRemoveAllGraphs = true;
      return true;
    }
  }
  else
  {
    if ( (DrawingName.Find( "Unknown" ) >= 0) && (m_Graphics.m_Figures.GetCount() >= ARR_SIZE( GraphColors )) )
    {
      return FALSE;
    }
    else
    {
      FXGFigure NewFigure;
      m_Graphics.m_Figures.Add( NewFigure );
      int iFigIndex = (int) m_Graphics.m_Figures.GetUpperBound();
      pFig = &m_Graphics.m_Figures[ iFigIndex ];
      //     pFig->m_MinMaxes.m_dXMax = 1000 ;
      //     pFig->m_MinMaxes.m_dYMax = 1000 ;
      pFig->SetObjectName( DrawingName );
      pFig->m_Color = GraphColors[ iFigIndex % ARR_SIZE( GraphColors ) ];
      pFig->m_MinMaxes = m_MinMaxes;
      ParseAttributes( pFrame , pFig );
      pFig->Copy( *pFrame );
    }
  }

  CDPointArray& Pts = *pFig;
  switch ( pFig->m_WhatDataToUse )
  {
  case USE_X:
    switch ( pFig->m_SecondAxisMode )
    {
    case BASE_MODE_STEP_ONE:
      {
        for ( int i = 0; i < pFig->GetCount(); i++ )
          Pts[ i ].y = i;
      }
      break;
    case BASE_MODE_STEP_SCALE:
      {
        for ( int i = 0; i < pFig->GetCount(); i++ )
          Pts[ i ].y = i * m_ScaleY;
      }
      break;
    case BASE_DENSITY:
      break;
    default:
      break;
    }
    break;
    break;
  case USE_Y:
    switch ( pFig->m_SecondAxisMode )
    {
    case BASE_MODE_STEP_ONE:
      {
        for ( int i = 0; i < pFig->GetCount(); i++ )
          Pts[ i ].x = i;
      }
      break;
    case BASE_MODE_STEP_SCALE:
      {
        for ( int i = 0; i < pFig->GetCount(); i++ )
          Pts[ i ].x = i * m_ScaleX;
      }
      break;
    case BASE_DENSITY:
      break;
    default:
      break;
    }
    break;
  case USE_XY:
    break;
  }
  if ( pFig->m_DrawMode != FIG_DRAW_ABSOLUTE )
  {
    CXYMinMaxes& pMM = pFig->m_MinMaxes;
    CDPointArray& Pts = *pFig;
    for ( int i = 0; i < pFig->GetCount(); i++ )
    {
      if ( pFig->m_DrawMode & 1 )
      {
        if ( Pts[ i ].x < pMM.m_dXMin )
          pMM.m_dXMin = Pts[ i ].x;
        if ( Pts[ i ].x > pMM.m_dXMax )
          pMM.m_dXMax = Pts[ i ].x;
      }
      if ( pFig->m_DrawMode & 2 )
      {
        if ( Pts[ i ].y < pMM.m_dYMin )
          pMM.m_dYMin = Pts[ i ].y;
        if ( Pts[ i ].y > pMM.m_dYMax )
          pMM.m_dYMax = Pts[ i ].y;
      }
    }
  }
  return (pFig != NULL);
}

BOOL CPlotGraphView::ViewSpectrum( const CArrayFrame * pRe ,
  const CArrayFrame * pIm , const CArrayFrame * pPhase )
{
  FXString DrawingName( "Spectrum" );

  int iFigIndex = -1 ;
  FXGFigure * pFig = m_Graphics.GetFigureByName( DrawingName , &iFigIndex );
  if ( !pRe || (pRe->GetElementSize() != sizeof( DPOINT )) )
    return FALSE;
  DPOINT * pReData = (DPOINT*) pRe->GetData();
  DWORD dwLen = pRe->GetCount();
  DPOINT * pImData = (pIm && pIm->GetCount() == dwLen) ? (DPOINT*) pIm->GetData() : NULL;

  if ( pFig )
    pFig->RemoveAll();
  else
  {
    FXGFigure NewFigure;
    m_Graphics.m_Figures.Add( NewFigure );
    int iFigIndex = (int) m_Graphics.m_Figures.GetUpperBound();
    pFig = &m_Graphics.m_Figures[ iFigIndex ];
    pFig->SetObjectName( DrawingName );
    pFig->m_Color = GraphColors[ iFigIndex % ARR_SIZE( GraphColors ) ];
    ParseAttributes( pRe , pFig );
  }
  double dMin = 1e300;
  double dMax = -1e300;
  int iHalfLen = dwLen / 2;
  for ( int i = 0; i < iHalfLen; i++ )
  {
    double dAmpl;
    if ( pIm )
    {
      dAmpl = sqrt( pReData[ i ].y * pReData[ i ].y + pImData[ i ].y * pImData[ i ].y );
//      CDPoint Pt( pReData[ i ].x , dAmpl );
      CDPoint Pt( i , dAmpl );
      pFig->Add( Pt );
    }
    else
    {
      pFig->Add( pReData[ i ] );
      dAmpl = pReData[ i ].y;
    }
    if ( dAmpl > dMax )
      dMax = dAmpl;
    if ( dAmpl < dMin )
      dMin = dAmpl;
  }
//   double dXMin = pFig->GetAt( 0 ).x;
//   double dXMax = pFig->GetAt( ( int ) pFig->GetUpperBound() ).x;
  double dXMin = 0 ;
  double dXMax = iHalfLen ;
  pFig->m_MinMaxes.SetMinMaxes( dXMin , dMin , dXMax , dMax );

  return (pFig != NULL);
}

BOOL CPlotGraphView::AppendToFigure( const CQuantityFrame * pQuantity )
{
  CDPoint NewPt;

  switch ( pQuantity->_type )
  {
  case GQ_INT: // integer
  case GQ_FLOAT: // double
    NewPt.x = 0;
    NewPt.y = (double) *pQuantity;
    break;
  case GQ_CMPLX:
  case GQ_DPNT:
    NewPt = (CDPoint) *pQuantity;
    break;
  default: return false;
  }
  FXString DrawingName;
  PlacementMode PlaceMode = AnalyzeLabel( pQuantity , DrawingName );

  bool bTimedView = (m_dSamplePeriod_ms > 0.) ;
  int iFigIndex = -1 ;
  FXGFigure * pFig = m_Graphics.GetFigureByName( DrawingName , &iFigIndex );
  if ( pFig )
  {
    pFig->m_PlaceMode = PlaceMode ;
    if ( !m_LogFileName.IsEmpty() && pQuantity->GetId() ) // ID should be not zero
    {
      if ( iFigIndex >= 0 )
      {
        if ( bTimedView )
        {
          if ( m_LastUpdatedDrawings.size() < (size_t) m_Graphics.m_Figures.Count() )
          {
            vector<BOOL> Tmp = m_LastUpdatedDrawings ;
            m_LastUpdatedDrawings.resize( m_Graphics.m_Figures.Count() ) ;
            for ( size_t i = Tmp.size() ; i < (size_t) m_Graphics.m_Figures.Count() ; i++ )
              m_LastUpdatedDrawings[ i ] = FALSE ;
            m_LastValues.resize( m_Graphics.m_Figures.Count() ) ;
          }
          m_LastUpdatedDrawings[ iFigIndex ] = TRUE ;
        }
        m_LastValues[ iFigIndex ] = CDPointToCmplx( NewPt ) ;
        if ( m_LastTimeStamp.IsEmpty() )
          m_LastTimeStamp = GetTimeAsString_ms() ;
        if ( pQuantity->GetId() != m_dwLastID ) // OK, there is new series, may be necessary to save to file
        {
          if ( m_dwLastID == 0 )
          {
            // first sample in file
            FXString Out( "Time Stamp,ID," )  ;
            for ( int i = 0 ; i < m_Graphics.m_Figures.Size() ; i++ )
              Out += m_Graphics.m_Figures[ i ].m_ObjectName + ((i == m_Graphics.m_Figures.GetUpperBound()) ? ' ' : ',') ;
            Out += '\n' ;
            ofstream myfile( (LPCTSTR) m_LogFileName , ios_base::app );
            if ( myfile.is_open() )
            {
              myfile.write( Out , Out.GetLength() ) ;
              myfile.close() ;
            }
          }
          m_dwLastID = pQuantity->GetId() ;
          FXString Out( m_LastTimeStamp ) , Addition ;
          Addition.Format( ",%d" , m_dwLastID ) ;
          Out += Addition ;
          for ( int i = 0 ; i < m_Graphics.m_Figures.Size() ; i++ )
          {
            if ( m_LastUpdatedDrawings[ i ] )
              Addition.Format( ",%g" , m_LastValues[ i ].imag() ) ;
            else
              Addition = _T( ",   " ) ;
            Out += Addition ;
          }
          Out += '\n' ;
          ofstream myfile( (LPCTSTR) m_LogFileName , ios_base::app );
          if ( myfile.is_open() )
          {
            myfile.write( Out , Out.GetLength() ) ;
            myfile.close() ;
          }
          m_LastTimeStamp.Empty() ;
          for ( auto iter = m_LastUpdatedDrawings.begin() ; iter != m_LastUpdatedDrawings.end() ; iter++ )
            *iter = FALSE ;
        }
      }
    }
    switch ( PlaceMode )
    {
    case PMODE_INSERT:
      {
        if ( !bTimedView )
        {
          pFig->InsertAt( 0 , NewPt );
          pFig->UpdateTiming( true ); // insert into timing array
          if ( pFig->GetCount() > m_iNSamples )
          {
            pFig->SetSize( m_iNSamples );
            pFig->CorrectLength( true , m_iNSamples );
          }
        }
      }
      break;
    case PMODE_APPEND:
      if ( !bTimedView )
      {
        pFig->AddPoint( NewPt );
        if ( pFig->GetCount() > m_iNSamples )
        {
          pFig->RemoveAt( 0 );
          pFig->CorrectLength( true , m_iNSamples );
        }
      }
      break;
    case PMODE_REPLACE:
      pFig->SetAt( 0 , NewPt );
      pFig->ReplaceTiming( 0 );
      break;
    case PMODE_REMOVE:
      m_Graphics.RemoveFigureByName( DrawingName );
      return true;
    }
  }
  else if ( PlaceMode == PMODE_REMOVE )
    return false;
  else if ( PlaceMode == PMODE_REMOVE_ALL )
  {
    m_bRemoveAllGraphs = 1;
    return false;
  }
  else // New figure for show
  {
    FXGFigure NewFigure;
    NewFigure.SetXYUse( USE_Y );
    NewFigure.SetSecondAxisMode( BASE_MODE_STEP_ONE );
    m_Graphics.m_Figures.Add( NewFigure );
    int iFigIndex = (int) m_Graphics.m_Figures.GetUpperBound();
    pFig = &m_Graphics.m_Figures[ iFigIndex ];
    pFig->m_MinMaxes.m_dXMax = 150;
    pFig->m_MinMaxes.m_dYMax = 500;
    pFig->SetObjectName( DrawingName );
    pFig->m_Color = GraphColors[ iFigIndex % ARR_SIZE( GraphColors ) ];
    ParseAttributes( pQuantity , pFig );
    pFig->AddPoint( NewPt );
    m_LastValues.resize( m_Graphics.m_Figures.Count() ) ;
    m_LastValues.push_back( CDPointToCmplx( NewPt ) ) ;
  }

  CDPointArray& Pts = *pFig;
  switch ( pFig->m_WhatDataToUse )
  {
  case USE_X:
    switch ( pFig->m_SecondAxisMode )
    {
    case BASE_MODE_STEP_ONE:
      {
        for ( int i = 0; i < pFig->GetCount(); i++ )
          Pts[ i ].y = i;
      }
      break;
    case BASE_MODE_STEP_SCALE:
      {
        for ( int i = 0; i < pFig->GetCount(); i++ )
          Pts[ i ].y = i * m_ScaleY;
      }
      break;
    case BASE_DENSITY:
      break;
    default:
      break;
    }
    break;
    break;
  case USE_Y:
    switch ( pFig->m_SecondAxisMode )
    {
    case BASE_MODE_STEP_ONE:
      {
        for ( int i = 0; i < pFig->GetCount(); i++ )
          Pts[ i ].x = i;
      }
      break;
    case BASE_MODE_STEP_SCALE:
      {
        for ( int i = 0; i < pFig->GetCount(); i++ )
          Pts[ i ].x = i * m_ScaleX;
      }
      break;
    case BASE_DENSITY:
      break;
    default:
      break;
    }
    break;
  case USE_XY:
    break;
  }
  if ( pFig->m_DrawMode != FIG_DRAW_ABSOLUTE )
  {
    CXYMinMaxes& pMM = pFig->m_MinMaxes;
    CDPointArray& Pts = *pFig;
    for ( int i = 0; i < pFig->GetCount(); i++ )
    {
      if ( pFig->m_DrawMode & 1 )
      {
        if ( Pts[ i ].x < pMM.m_dXMin )
          pMM.m_dXMin = Pts[ i ].x;
        if ( Pts[ i ].x > pMM.m_dXMax )
          pMM.m_dXMax = Pts[ i ].x;
      }
      if ( pFig->m_DrawMode & 2 )
      {
        if ( Pts[ i ].y < pMM.m_dYMin )
          pMM.m_dYMin = Pts[ i ].y;
        if ( Pts[ i ].y > pMM.m_dYMax )
          pMM.m_dYMax = Pts[ i ].y;
      }
    }
  }
  return (pFig != NULL);
}

void CPlotGraphView::Render( const CDataFrame* pDataFrame )
{
  FXAutolock al( m_DataLock );
  if ( m_bRemoveAllGraphs )
  {
    m_Graphics.RemoveData();
    m_bRemoveAllGraphs = false;
  }
  const CArrayFrame* ReFrame = NULL;
  const CArrayFrame* ImFrame = NULL;
  const CArrayFrame* AmpFrame = NULL;
  const CArrayFrame* PhFrame = NULL;

  // 1. Trying to extract data for plot as figure frames
  const CDataFrame* pFrame = NULL;
  BOOL bIsAddedData = false;
  if ( pDataFrame->IsContainer() )
  {
    CFramesIterator* Iterator = pDataFrame->CreateFramesIterator( figure );
    int iTakenCounter = 0;
    if ( Iterator )
    {
      pFrame = (CFigureFrame *) Iterator->Next( NULL );
      if ( pFrame )
      {
        bIsAddedData = true;
        while ( pFrame )
        {
          bIsAddedData |= AddOrCorrectFigure( pFrame );
          pFrame = Iterator->Next( NULL );
        };
      }
      delete Iterator;
    }
    // trying to extract values or arrays of values
    Iterator = pDataFrame->CreateFramesIterator( quantity );
    if ( Iterator )
    {
      CDataFrame* pFrame = Iterator->Next( DEFAULT_LABEL );
      while ( pFrame )
      {
        const CQuantityFrame* pQuantity = pFrame->GetQuantityFrame( NULL );
        if ( pQuantity )
          bIsAddedData |= AppendToFigure( pQuantity );
        pFrame = Iterator->Next( DEFAULT_LABEL );
      }
      delete Iterator;
    }
    Iterator = pDataFrame->CreateFramesIterator( arraytype );
    if ( Iterator )
    {
      CDataFrame* pFrame = Iterator->Next( DEFAULT_LABEL );
      while ( pFrame )
      {
        if ( !ReFrame )
        {
          if ( ReFrame = pFrame->GetArrayFrame( "Re" ) )
          {
            pFrame = Iterator->Next( DEFAULT_LABEL );
            continue;
          }
        }
        if ( !ImFrame )
        {
          if ( ImFrame = pFrame->GetArrayFrame( "Im" ) )
          {
            pFrame = Iterator->Next( DEFAULT_LABEL );
            continue;
          }
        }
        if ( !AmpFrame )
        {
          if ( AmpFrame = pFrame->GetArrayFrame( "Amp" ) )
          {
            pFrame = Iterator->Next( DEFAULT_LABEL );
            continue;
          }
        }
        if ( !PhFrame )
        {
          if ( PhFrame = pFrame->GetArrayFrame( "Ph" ) )
          {
            pFrame = Iterator->Next( DEFAULT_LABEL );
            continue;
          }
        }
        pFrame = Iterator->Next( DEFAULT_LABEL );
      }
      delete Iterator;
      if ( ReFrame )
        bIsAddedData |= ViewSpectrum( ReFrame , ImFrame , PhFrame );
      else if ( AmpFrame )
        bIsAddedData |= ViewSpectrum( AmpFrame );
    }
  }
  else
  {
    pFrame = pDataFrame->GetFigureFrame( DEFAULT_LABEL );
    if ( pFrame )
      bIsAddedData |= AddOrCorrectFigure( pFrame );

    const CQuantityFrame* pQuantity = pDataFrame->GetQuantityFrame( NULL );
    if ( pQuantity )
      bIsAddedData |= AppendToFigure( pQuantity );
    const CTextFrame * pControl = pDataFrame->GetTextFrame() ;
    if ( pControl )
    {
      if ( _tcscmp( pControl->GetLabel() , _T( "SaveFileName" ) ) == 0 )
      {
        m_LogFileName = pControl->GetString() ;
        m_LastUpdatedDrawings.clear() ;
        m_dwLastID = 0 ;
        m_LastTimeStamp.Empty() ;
      }
    }
  }
  if ( bIsAddedData && (m_dSamplePeriod_ms == 0.) )  // Is NOT added figure frame data?
  {
    //     m_GraphFrame.left=m_GraphFrame.right=m_Data[0].x;
    //     m_GraphFrame.top=m_GraphFrame.bottom=m_Data[0].y;
    //     for (int i = 1 ; i < m_Graphics.m_Figures.GetSize() ; i++ )
    //     {
    //       if (m_Data[i].x<m_GraphFrame.left)  
    //         m_GraphFrame.left=m_Data[i].x;
    //       if (m_Data[i].x>m_GraphFrame.right) 
    //         m_GraphFrame.right=m_Data[i].x;
    //       if (m_Data[i].y<m_GraphFrame.bottom) 
    //         m_GraphFrame.bottom=m_Data[i].y;
    //       if (m_Data[i].y>m_GraphFrame.top)   
    //         m_GraphFrame.top=m_Data[i].y;
    //     }
    Invalidate( FALSE );
  }
}
void CPlotGraphView::OnMouseMove( UINT nFlags , CPoint point )
{
  m_MousePnt = point;
  CWnd::OnMouseMove( nFlags , point );
}

BOOL CPlotGraphView::CreateFontWithSize( UINT iSize )
{
  CFont * pFont = new CFont;
  if ( pFont->CreateFont(
    (int) (iSize*SCALEF + 0.5) ,              // nHeight
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
void CPlotGraphView::WriteMinMaxes( FXPropertyKit& pk )
{
  WriteArray( pk , "RangeX" , _T( 'f' ) , 2 , &m_MinMaxes.m_dXMin );
  WriteArray( pk , "RangeY" , _T( 'f' ) , 2 , &m_MinMaxes.m_dYMin );
}

BOOL CPlotGraphView::SetMinMaxes( FXPropertyKit& pk )
{
  BOOL bRes = false;
  CXYMinMaxes MinMaxes = m_MinMaxes;
  if ( GetArray( pk , "RangeX" , _T( 'f' ) , 2 , &MinMaxes.m_dXMin ) == 2 )
  {
    m_MinMaxes.m_dXMin = MinMaxes.m_dXMin;
    m_MinMaxes.m_dXMax = MinMaxes.m_dXMax;
    bRes = true;
  }
  if ( GetArray( pk , "RangeY" , _T( 'f' ) , 2 , &MinMaxes.m_dYMin ) == 2 )
  {
    m_MinMaxes.m_dYMin = MinMaxes.m_dYMin;
    m_MinMaxes.m_dYMax = MinMaxes.m_dYMax;
    bRes = true;
  }
  return bRes;
}
void CPlotGraphView::SetSamplePeriod( double dPeriod_ms )
{
  m_dSamplePeriod_ms = (dPeriod_ms >= 0.) ? dPeriod_ms : 0. ;
  if ( m_uiTimerID )
  {
    KillTimer( m_uiTimerID ) ;
    m_uiTimerID = 0 ;
  }
  if ( m_dSamplePeriod_ms )
  {
    m_uiTimerID = SetTimer( EVT_UPDATE_BY_TIMER , ROUND( m_dSamplePeriod_ms ) , NULL ) ;
  }
}



void CPlotGraphView::OnTimer( UINT_PTR nIDEvent )
{
  for ( int i = 0 ; i < m_Graphics.m_Figures.Count() ; i++ )
  {
    if ( (int) m_LastValues.size() > i )
    {
      FXGFigure * pGFig = &m_Graphics.m_Figures[ i ];
      CmplxArray * pFig = (CmplxArray *) pGFig;
      switch ( pGFig->m_PlaceMode )
      {
      case PMODE_INSERT:
        {
          pFig->InsertAt( 0 , m_LastValues[ i ] );
          if ( pFig->GetCount() > m_iNSamples )
            pFig->SetSize( m_iNSamples );
        }
        break;
      case PMODE_APPEND:
        pFig->Add( m_LastValues[ i ] );
        if ( pFig->GetCount() > m_iNSamples )
          pFig->RemoveAt( 0 , pFig->GetCount() - m_iNSamples );
        break;
      default:
        return ;
      }

      CDPointArray& Pts = (CDPointArray&) *pGFig ;
      switch ( pGFig->m_WhatDataToUse )
      {
      case USE_X:
        switch ( pGFig->m_SecondAxisMode )
        {
        case BASE_MODE_STEP_ONE:
          {
            for ( int i = 0; i < pFig->GetCount(); i++ )
              Pts[ i ].y = i;
          }
          break;
        case BASE_MODE_STEP_SCALE:
          {
            for ( int i = 0; i < pFig->GetCount(); i++ )
              Pts[ i ].y = i * m_ScaleY;
          }
          break;
        case BASE_DENSITY:
          break;
        default:
          break;
        }
        break;
        break;
      case USE_Y:
        switch ( pGFig->m_SecondAxisMode )
        {
        case BASE_MODE_STEP_ONE:
          {
            for ( int i = 0; i < pFig->GetCount(); i++ )
              Pts[ i ].x = i;
          }
          break;
        case BASE_MODE_STEP_SCALE:
          {
            for ( int i = 0; i < pFig->GetCount(); i++ )
              Pts[ i ].x = i * m_ScaleX;
          }
          break;
        case BASE_DENSITY:
          break;
        default:
          break;
        }
        break;
      case USE_XY:
        break;
      }
      if ( pGFig->m_DrawMode != FIG_DRAW_ABSOLUTE )
      {
        CXYMinMaxes& pMM = pGFig->m_MinMaxes;
        for ( int i = 0; i < pGFig->GetCount(); i++ )
        {
          if ( pGFig->m_DrawMode & 1 )
          {
            if ( Pts[ i ].x < pMM.m_dXMin )
              pMM.m_dXMin = Pts[ i ].x;
            if ( Pts[ i ].x > pMM.m_dXMax )
              pMM.m_dXMax = Pts[ i ].x;
          }
          if ( pGFig->m_DrawMode & 2 )
          {
            if ( Pts[ i ].y < pMM.m_dYMin )
              pMM.m_dYMin = Pts[ i ].y;
            if ( Pts[ i ].y > pMM.m_dYMax )
              pMM.m_dYMax = Pts[ i ].y;
          }
        }
      }
    }
  }

  if ( m_Graphics.m_Figures.Count() && m_LastValues.size() )
    Invalidate( FALSE ) ;

  CWnd::OnTimer( nIDEvent );
}



void CPlotGraphView::OnSize( UINT nType , int cx , int cy )
{
  CWnd::OnSize( nType , cx , cy );


}
