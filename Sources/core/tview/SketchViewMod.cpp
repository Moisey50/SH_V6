#include "stdafx.h"
#include "SketchView.h"
#include "SketchViewMod.h"
#include "resource.h"
#include "GraphTreeComposer.h"

#define GLYPH_CONNECTOR_OFFSET_X (GLYPH_GADGET_WIDTH/10)
#define GLYPH_CONNECTOR_OFFSET_Y (GLYPH_GADGET_HEIGHT/10)

#define GLYPH_INFLATE 5

void CSketchViewMod::DeleteSelected()
{
  CRgn rgn;
  if ( m_SelectedGlyphs.GetRgn( &rgn ) )
  {
    int i;
    for ( i = 0; i < m_SelectedGlyphs.GetChildrenCount(); i++ )
    {
      CGlyph* gl = m_SelectedGlyphs.GetChildAt( i );
      int j;
      for ( j = 0; j < gl->GetEvadedGlyphSize(); j++ )
      {
        CRgn rgn_temp;
        ((CGlyph*) (gl->GetEvadedGlyph( j )))->GetRgn( &rgn_temp );
        rgn.CombineRgn( &rgn , &rgn_temp , RGN_OR );
      }
    }
    CRgn* Rgn = m_Map.AbsToView( &rgn );
    rgn.DeleteObject();
    InvalidateRgn( Rgn , TRUE );
    delete Rgn;
  }
  CSketchView::DeleteSelected();
}

void CSketchViewMod::Paint()
{
  m_Graph.ResetEvadeCount();
  m_SelectedGlyphs.ResetEvadeCount();
  //GetCoherency();
  CSketchView::Paint();
}

void CSketchViewMod::SetViewType( ViewType vt )
{
  m_ViewType = vt;
  for ( int i = 0; i < m_Graph.GetChildrenCount(); i++ )
  {
    m_Graph.GetChildAt( i )->SetViewType( vt );
  }
  Invalidate();
}

CGlyph* CSketchViewMod::IsPointInGlyphRect( CPoint pt , CRect *res_measured , int offset_X , int offset_Y , bool start /*= FALSE*/ )
//function returns number of the nearest border:
// 1 - top
// 2 - right
// 3 - bottom
// 4 - left
{
  for ( int i = 0; i < m_Graph.GetChildrenCount(); i++ )
  {
    CGlyph* gl = m_Graph.GetChildAt( i );
    //     if (!IsGadgetGlyph(gl))
    //       continue;
    CGadgetGlyph *ggl = (CGadgetGlyph*) gl;
    CRect rc_t( ggl->GetPos() , ggl->GetSize() );
    rc_t.InflateRect( GLYPH_INFLATE , GLYPH_INFLATE );
    int ev = gl->GetEvadeCount();
    if ( start )
      ev = 0;
    if ( ev != 0 )
    {
      rc_t.left -= (ev) *offset_X - offset_X / 2;
      rc_t.right += (ev) *offset_X - offset_X / 2;
      rc_t.top -= (ev) *offset_Y - offset_Y / 2;
      rc_t.bottom += (ev) *offset_Y - offset_Y / 2;
    }
    if ( rc_t.PtInRect( pt ) || (pt.y == rc_t.bottom && pt.x >= rc_t.left && pt.x <= rc_t.right) || (pt.x == rc_t.right && pt.y >= rc_t.top && pt.y <= rc_t.bottom) )
    {
      if ( res_measured )
      {
        *res_measured = rc_t;
      }
      return gl;
    }
  }
  return NULL;
}

CGlyph* CSketchViewMod::IsPointInGlyphRect( CPoint pt , CRect *res_measured , int offset_X , int offset_Y , CGlyph *father )
//function returns number of the nearest border:
// 1 - top
// 2 - right
// 3 - bottom
// 4 - left
{
  for ( int i = 0; i < m_Graph.GetChildrenCount(); i++ )
  {
    CGlyph* gl = m_Graph.GetChildAt( i );
    //     if (!IsGadgetGlyph(gl))
    //       continue;
    CGadgetGlyph *ggl = (CGadgetGlyph*) gl;
    CRect rc_t( ggl->GetPos() , ggl->GetSize() );
    int ev = gl->GetEvadeCount();
    if ( gl == father )
      ev = 0;
    if ( ev != 0 )
    {
      rc_t.left -= (ev) *offset_X - offset_X / 2;
      rc_t.right += (ev) *offset_X - offset_X / 2;
      rc_t.top -= (ev) *offset_Y - offset_Y / 2;
      rc_t.bottom += (ev) *offset_Y - offset_Y / 2;
    }
    if ( rc_t.PtInRect( pt ) || (pt.y == rc_t.bottom && pt.x >= rc_t.left && pt.x <= rc_t.right) || (pt.x == rc_t.right && pt.y >= rc_t.top && pt.y <= rc_t.bottom) )
    {
      if ( res_measured )
      {
        *res_measured = rc_t;
      }
      return gl;
    }
  }
  return NULL;
}

CGlyph* CSketchViewMod::IsPointInGlyphRect( int x , int y , CRect *res_measured , int offset_X , int offset_Y , bool start /*= FALSE*/ )
{
  return IsPointInGlyphRect( CPoint( x , y ) , res_measured , offset_X , offset_Y , start );
}

CGlyph* CSketchViewMod::IsPointInGlyphRect( int x , int y , CRect *res_measured , int offset_X , int offset_Y , CGlyph *father )
{
  return IsPointInGlyphRect( CPoint( x , y ) , res_measured , offset_X , offset_Y , father );
}

CGlyph *CSketchViewMod::GraphIntersect( CPoint pt )
{
  CGlyph* Glyph = m_Graph.Intersect( pt , TRUE );
  if ( !Glyph )
    return m_Graph.Intersect( pt , FALSE );
  return Glyph;
}

CGlyph* CSketchViewMod::GraphIntersect( int x , int y )
{
  CPoint temp( x , y );
  CGlyph* Glyph = m_Graph.Intersect( temp , TRUE );
  if ( !Glyph )
    return m_Graph.Intersect( temp , FALSE );
  return Glyph;
}

void CSketchViewMod::GetCoherency()
{
  while ( m_CoherentComponents.GetSize() )
  {
    CCompositeGlyph *gl = m_CoherentComponents.GetAt( 0 );
    m_CoherentComponents.RemoveAt( 0 );
    delete gl;
  }
  for ( int i = 0; i < m_Graph.GetChildrenCount(); i++ )
  {
    CGlyph *gl = m_Graph.GetChildAt( i );
    BOOL wasIntersect = FALSE;
    for ( int j = 0; j < (int) m_CoherentComponents.GetSize(); j++ )
    {
      CCompositeGlyph *glFather = m_CoherentComponents.GetAt( j );
      if ( glFather->Intersect( gl ) )
      {
        wasIntersect = TRUE;
        glFather->Add( gl );
        break;
      }
    }
    if ( wasIntersect )
      continue;
    CCompositeGlyph *glNewFather = new CCompositeGlyph( this );
    glNewFather->Add( gl );
    m_CoherentComponents.Add( glNewFather );
  }
  BOOL NoChanges = FALSE;
  while ( !NoChanges )
  {
    NoChanges = TRUE;
    for ( int i = 0; i < (int) m_CoherentComponents.GetSize(); i++ )
    {
      for ( int j = i + 1; j < (int) m_CoherentComponents.GetSize(); j++ )
      {
        CCompositeGlyph *cg1 , *cg2;
        cg1 = m_CoherentComponents.GetAt( i );
        cg2 = m_CoherentComponents.GetAt( j );
        if ( cg1->Intersect( cg2 ) )
        {
          NoChanges = FALSE;
          CCompositeGlyph *cg3 = new CCompositeGlyph( this );
          int k;
          for ( k = 0; k < cg1->GetChildrenCount(); k++ )
          {
            cg3->Add( cg1->GetChildAt( k ) );
          }
          for ( k = 0; k < cg2->GetChildrenCount(); k++ )
          {
            cg3->Add( cg2->GetChildAt( k ) );
          }
          m_CoherentComponents.RemoveAt( j );
          m_CoherentComponents.RemoveAt( i );
          delete cg1;
          delete cg2;
          m_CoherentComponents.Add( cg3 );
          i = 0;
        }
      }
    }
  }
}

CSketchViewMod::~CSketchViewMod()
{
  while ( m_CoherentComponents.GetSize() )
  {
    CCompositeGlyph *gl = m_CoherentComponents.GetAt( 0 );
    m_CoherentComponents.RemoveAt( 0 );
    delete gl;
  }
}

void CSketchViewMod::ResetCornerCounters()
{
  for ( int i = 0; i < m_Graph.GetChildrenCount(); i++ )
  {
    CGlyph *gl = m_Graph.GetChildAt( i );
    gl->ResetCornerCounter();
  }
  return;
}