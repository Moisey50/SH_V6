#pragma once

#include <classes/drect.h>

typedef struct _tagGlyphPosition
{
    FXString    name;
    POINT       pnt;
}GlyphPosition;

class FloatWndPosition
{
public:
  FloatWndPosition() { pnt.x = pnt.y = sz.cx = sz.cy = 0 ;  }
  FloatWndPosition( LPCTSTR Name , double x , double y ,
    double w , double h , LPCTSTR Selected )
  { 
    name = Name ;
    pnt.x = x ; pnt.y = y ; sz.cx = w ; sz.cy = h ;
    selected = Selected ;
  }
  FloatWndPosition& operator=( FloatWndPosition& Orig )
  {
    name = Orig.name ;
    pnt = Orig.pnt ;
    sz = Orig.sz ;
    selected = Orig.selected ;
    return *this ;
  }
  FXString    name;
  DPOINT      pnt;
  DSIZE       sz;
  FXString    selected ; // name of selected tab (window)
};

class FX_EXT_SHBASE CViewSectionParser: public FXStringArray
{
protected:
    FXArray<GlyphPosition,GlyphPosition&> m_Glyphs;
    FXArray<FloatWndPosition,FloatWndPosition&> m_FloatWnds;
    CDRect m_GraphViewPos ;
    CPoint      m_ViewOffset ;
    double      m_dViewScale ;

    BOOL   m_bIsViewed ;
private:
    int    FindGlyph(LPCTSTR name);
    int    FindFloatWnd(LPCTSTR name);
public:
    CViewSectionParser();
    ~CViewSectionParser(void);
    void    SetConfig(bool DoGlyphs, bool DoFloatWnds);
///
    void    RemoveAll();
    bool    Parse(LPCTSTR script);
    bool    SetGlyph(LPCTSTR gadget, int x, int y);
    bool    GetGlyph(LPCTSTR name, int& x, int& y);
    bool    SetFloatWnd(LPCTSTR name, double x, double y, double w, double h , LPCTSTR pszSelected = NULL );
    bool    GetFloatWnd(LPCTSTR name, double& x, double& y, double& w, double& h , FXString *  pSelected = NULL );
    bool    RenameGlyph(LPCTSTR oldname, LPCTSTR newname);
    bool    RemoveGlyph(LPCTSTR name);
    FXString GetViewSection();
    bool    GetViewPropertyString(FXString& prop);
    void    GetGraphViewPos( CDRect& Pos ) ;
    void    SetGraphViewPos( const CDRect& Pos ) ;
    CPoint GetViewOffset() { return m_ViewOffset ; }
    void SetViewOffset( CPoint offset ) { m_ViewOffset = offset ; };
    double GetViewScale() const { return m_dViewScale; };
    void SetViewScale( double scale ) { m_dViewScale = scale ; };
    void    SetViewed( BOOL bViewed ) { m_bIsViewed = bViewed ; }
    BOOL    GetViewed() { return m_bIsViewed ; }
};
