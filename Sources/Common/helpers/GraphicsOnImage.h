#include <classes\dpoint.h>
#include <classes\dRect.h>
#include <fxfc\fxfc.h>
#include <math/intf_sup.h>
#include <math\hbmath.h>
#include <video\TVFrame.h>
#include <minwinbase.h>
#include <sysinfoapi.h>
#include <vector>


// defines for object selection by cursor
#define CURSOR_TOLERANCE 10

#define CURSOR_NO_SELECTION     0

// On rectangular area
#define CURSOR_ON_LEFT_BORDER   1
#define CURSOR_ON_TOP_BORDER    2
#define CURSOR_ON_RIGHT_BORDER  3
#define CURSOR_ON_BOTTOM_BORDER 4
#define CURSOR_ON_LTCORNER      5
#define CURSOR_ON_TRCORNER      6
#define CURSOR_ON_RBCORNER      7
#define CURSOR_ON_BLCORNER      8

// For all types of object
#define CURSOR_ON_WHOLE_SELECTION   9

enum SelectionByCursor
{
  CBS_NoSelection = 0 ,
  // On rectangular area
  CBS_LeftBorder ,
  CBS_TopBorder ,
  CBS_RightBorder ,
  CBS_BottomBorder ,
  CBS_LTCorner ,
  CBS_TRCorner ,
  CBS_RBCorner ,
  CBS_BLCorner ,
  // For all types of object
  CBS_Whole 
};

// Where is neighborhood foound
#define NO_NEIGHBORHOOD         0
#define IS_NEAR_FIRST_EDGE      1 // near corner on rectangular object
#define IS_NEAR_SECOND_EDGE     2 // near corner on rectangular object
#define IS_NEAR_SEGMENT         3 // Not near corner

enum{ SELECTED_NONE , SELECTED_POINT , SELECTED_RECT , SELECTED_FIGURE } ;


//typedef CArray<CPoint,CPoint&> CPointArray ;



class CXYMinMaxes
{
public:
  CXYMinMaxes() { m_dXMin = m_dYMin = 0. ; m_dXMax = 200 ; m_dYMax = 500 ; }
  double dAmplX() { return (m_dXMax - m_dXMin) ; }
  double dAmplY() { return (m_dYMax - m_dYMin) ; }
  void Normalize() 
  { 
    if ( m_dXMax < m_dXMin )
      swapdouble( m_dXMin , m_dXMax ) ;
    if ( m_dYMax < m_dYMin )
      swapdouble( m_dYMin , m_dYMax ) ;
  }
  void SetMinMaxes( double dXMin , double dYMin , double dXMax , double dYMax )
  {
    m_dXMin = dXMin ; m_dXMax = dXMax ; m_dYMin = dYMin ; m_dYMax = dYMax ;
    Normalize() ;
  }
  double m_dXMin , m_dXMax , m_dYMin , m_dYMax ;
};



// Figure draw modes 
enum FIG_DRAW_MODE
{
  FIG_DRAW_ABSOLUTE = 0 ,
  FIG_DRAW_FIT_X ,
  FIG_DRAW_FIT_Y ,
  FIG_DRAW_FIT_XY 
}  ;

enum FIG_USE_DATA
{
  USE_X ,
  USE_Y ,
  USE_XY
} ;

enum FIG_BASE_MODE
{
  BASE_MODE_2D ,         // use second coordinate for second axis
  BASE_MODE_STEP_ONE ,   // next value will be drawn with step 1 on second axis
  BASE_MODE_STEP_SCALE , // Step will be taken from m_Scale for second axis
  BASE_DENSITY              // Time in seconds * m_Scale
};

enum FIG_DRAW_DIR
{
  DIRDRAW_LR ,
  DIRDRAW_RL ,
  DIRDRAW_DU ,
  DIRDRAW_UD
};

enum PlacementMode
{
  PMODE_INSERT = 1 ,
  PMODE_APPEND ,
  PMODE_REPLACE ,
  PMODE_REMOVE ,
  PMODE_REMOVE_ALL
};

enum AttribExtracted
{
  AE_Color = 1 ,
  AE_Thickness = 2,
  AE_Sz = 4,
  AE_Style = 8,
  AE_Back = 16
};
// the next does checking about point position near horizontal or vertical segment
int PointNearAlignedSegment( CPoint& Pt ,
  CPoint& Seg1 , CPoint& Seg2 , int iTolerance = CURSOR_TOLERANCE ,
  int * pDist = NULL ) ;

DWORD ExtractAttributes( const FXPropertyKit * pAttributes ,
  DWORD * pColor = NULL , int * piThickness = NULL ,
  int * piSz = NULL , int * piStyle = NULL , DWORD * pBack = NULL ) ;

class SelectableGraphics
{
public:
  SelectableGraphics( bool bSetSelectable = false )
  { 
    m_bSelectable = bSetSelectable ; 
    m_bSelected = false ;
    m_bSelectedForAdjustment = false ;
    m_iDistToCursor = 100000 ;
    m_CursorForm = NULL ;
    m_Color = 0xff ;
    m_bMatched = true ;
    m_iSizeMult = 1 ;
    m_dwLineWidth = 1 ;
    m_Style = PS_SOLID ;
    m_Back = 1 ; // no background
  }

  bool IsSelectable() { return m_bSelectable ; }
  void SetSelectable( bool bSet ) { m_bSelectable = bSet ; }
  bool IsSelected() { return m_bSelected ; }
  void SetSelected( bool bSet ) { m_bSelected = bSet ; }
  bool IsForAdjustment() { return m_bSelectedForAdjustment ; }
  void SetForAdjustment( bool bSet ) { m_bSelectedForAdjustment = bSet ; }
  void SetObjectName( LPCTSTR pName ) { m_ObjectName = pName ; }
  LPCTSTR GetObjectName() { return (LPCTSTR) m_ObjectName ; }
  bool GetMatched() { return m_bMatched ; }
  void SetMatched( bool bMatched ) { m_bMatched = bMatched ;}
  void CopySelectData( const SelectableGraphics& Orig )
  {
    memcpy( &m_bSelectable , &Orig.m_bSelectable , 
      (BYTE*)&m_ObjectName - (BYTE*)&m_bSelectable ) ;
    m_ObjectName = Orig.m_ObjectName ;
  }
  SelectableGraphics& operator=(const  SelectableGraphics & Orig )
  {
    memcpy( &m_bSelectable , &Orig.m_bSelectable ,
      ( BYTE* ) &m_ObjectName - ( BYTE* ) &m_bSelectable ) ;
    m_ObjectName = Orig.m_ObjectName ;
    return *this ;
  }
  bool  m_bSelectable ;
  bool  m_bSelected ;
  bool  m_bSelectedForAdjustment ;
  int   m_iDistToCursor ;
  int   m_iSizeMult ;
  LPCSTR m_CursorForm ;
  DWORD m_Color ;
  DWORD m_dwLineWidth ;
  int   m_Style ;
  DWORD m_Back ;
  bool  m_bMatched ;
  FXString m_ObjectName ;
};

class CGPoint: public CDPoint, public SelectableGraphics
{
public:
  CGPoint( DWORD Color = RGB(255,40,40) , DWORD dwLineWidth = 1 )
  {
    m_Color = Color ;
    m_dwLineWidth = dwLineWidth ;
    x = y = 0. ;
  };
  CGPoint(const CGPoint& Pt)
  {
    *this = Pt ;
  }
  CGPoint(const cmplx& Pt , DWORD Color = RGB(255,40,40) , DWORD dwLineWidth = 1 )
  {
    x = Pt.real() ;
    y = Pt.imag() ;
    m_Color = Color ;
    m_dwLineWidth = dwLineWidth ;
  }
  CGPoint(const CDPoint& Pt , DWORD Color = RGB(255,40,40) , DWORD dwLineWidth = 1 )
  {
    x = Pt.x ;
    y = Pt.y ;
    m_Color = Color ;
    m_dwLineWidth = dwLineWidth ;
  }
  CGPoint(const CPoint& Pt , DWORD Color = RGB(255,40,40) , DWORD dwLineWidth = 1 )
  {
    x = Pt.x ;
    y = Pt.y ;
    m_Color = Color ;
    m_dwLineWidth = dwLineWidth ;
  }

  FXString ToString()
  {
    FXString retV ;
    retV.Format("GPoint (%g,%g)", x , y);
    return retV;
  }
};

class FXGFigure: public CDPointArray, public SelectableGraphics
{
public:
  FXGFigure( DWORD Color = RGB(255,40,40) , DWORD dwLineWidth = 1 )
  {
    m_Color = Color ;
    m_dwLineWidth = dwLineWidth ;
    m_dScaleX = m_dScaleY = 1.0 ;
    m_DrawArea.right = m_DrawArea.bottom = 1.0 ;
    m_DrawMode = FIG_DRAW_ABSOLUTE ;
    m_WhatDataToUse = USE_XY ;
    m_SecondAxisMode = BASE_MODE_2D ;
    m_DrawDir = DIRDRAW_LR ;
    m_PlaceMode = PMODE_APPEND ;
    m_bReverse = false ;
    m_bSaveTiming = false ;
    m_Style = PS_SOLID ;
  };
  FXGFigure(const FXGFigure& fig)
  {
    CopyFigData( fig ) ;
  }
  void SetXYUse( FIG_USE_DATA Use ) { m_WhatDataToUse = Use ; }
  void SetSecondAxisMode ( FIG_BASE_MODE Use ) { m_SecondAxisMode = Use ; }
  void EnableTimingSave(bool bEnable) { m_bSaveTiming = bEnable;  }
  bool IsTimingEnabled() { return m_bSaveTiming; }
  int CorrectLength(bool bInserted , size_t iLen)
  {
    if ( m_bSaveTiming && m_Timing.size() > iLen )
    {
      if (bInserted)
        m_Timing.resize(iLen);
      else
        m_Timing.erase(m_Timing.begin());
    }
    return (int)m_Timing.size();
  }
  int UpdateTiming(bool bInsert)
  {
    if (m_bSaveTiming)
    {
      _FILETIME Now;
       GetSystemTimePreciseAsFileTime(&Now);
      if (bInsert)
        m_Timing.insert(m_Timing.begin() , Now );
      else // append
        m_Timing.push_back(Now);
      return (int)m_Timing.size();
    }
    return 0; 
  }
  int ReplaceTiming(size_t iIndex)
  {
    if (m_bSaveTiming && (iIndex < m_Timing.size()) )
    {
      _FILETIME Now;
       GetSystemTimePreciseAsFileTime(&Now);
      m_Timing[iIndex] = Now;
      return 1;
    }
    return 0;
  }
  void Clean();
  int  AddPoint(const DPOINT& p)
  {
    Add( CDPoint(p) );
    return (int)GetCount();
  }
  int  AddPoint(const CDPoint& p)
  {
    Add(p);
    UpdateTiming(false);
    return (int)GetCount();
  }
  int  AddPoint( const cmplx& p )
  {
    Add( *(CDPoint*) &p );
    return (int)GetCount() ;
  }
  int  AddPoints( const cmplx * pPts , int iNPts )
  {
    while ( iNPts-- > 0 )
      Add( *(CDPoint*) (pPts++) );

    return (int)GetCount() ;
  }

  int  GetNumberVertex() { return (int)GetCount(); }
  CDPoint& GetAt(int i) { return this->operator[](i); }
  _FILETIME GetTimeAt(int i) { return m_Timing[i]; }
  FXString ToString()
  {
    FXString retV ;
    switch( GetCount() )
    {
    case 1:retV.Format("Point (%f,%f)",GetAt(0).x,GetAt(0).y) ; break ;
    case 2:retV=_T("Segment") ; break ;
    case 3:retV=_T("Triangle") ; break ;
    case 4:retV=_T("Tetragon") ; break ;
    default: retV=_T("Polyline") ; break ;
    }
    return retV;
  }
  FXGFigure& operator =( const FXGFigure& figSrc )
  {
    CopyFigData( figSrc ) ;
    return *this;
  }
  void CopyFigData( const FXGFigure& Orig )
  {
    Copy( Orig ) ;
    CopySelectData( Orig ) ;
    memcpy( &m_dScaleX , &Orig.m_dScaleX , 
      (BYTE*)&m_Timing - (BYTE*)&m_dScaleX ) ;
    if ( !Orig.m_Timing.empty() )
      m_Timing = Orig.m_Timing;
  }
  int SaveToCSV(LPCTSTR pFileName , LPCTSTR pNumFormat = _T("%g"));

  double m_dScaleX ;
  double m_dScaleY ;

  FIG_DRAW_MODE m_DrawMode ;
  FIG_USE_DATA  m_WhatDataToUse ;
  FIG_BASE_MODE m_SecondAxisMode ;
  CDRect        m_DrawArea ; // in normalized to screen 
  CXYMinMaxes   m_MinMaxes ; // for graph normalization
  bool          m_bReverse ; // if true - reverse draw direction
  FIG_DRAW_DIR  m_DrawDir ;
  PlacementMode m_PlaceMode ; // Last redraw direction
  bool          m_bSaveTiming;
  vector<FILETIME> m_Timing;
};

class FXRectangle : public CRect, public SelectableGraphics
{
public:
  FXRectangle( DWORD Color = RGB(255,40,40) , DWORD dwLineWidth = 1 )
  {
    m_Color = Color ;
    m_dwLineWidth = dwLineWidth ;
    m_iSelectedSide = CURSOR_NO_SELECTION ;
    m_Back = 1 ;
  };
  FXRectangle(const FXRectangle& rect)
  {
    *this = rect ;
    CopySelectData( rect ) ;
  }
  FXRectangle& operator =( CRect& rc) 
  {
    *(CRect*)this = rc ;
    //AdjustCursorAreas() ;
    AdjustCursorAreas() ;
    return *this ;
  }
  FXRectangle& operator =( RECT& rc) 
  {
    *(RECT*)this = rc ;
    AdjustCursorAreas() ;
    return *this ;
  }
  void AdjustCursorAreas()
  {
    m_NearEdgesBig = *(CRect*)this ;
    m_NearEdgesSmall = m_NearEdgesBig ;

    m_NearEdgesBig.InflateRect( CURSOR_TOLERANCE , CURSOR_TOLERANCE ) ;
    m_NearEdgesSmall.DeflateRect( 
      m_NearEdgesSmall.Width() > CURSOR_TOLERANCE * 4 ?
        CURSOR_TOLERANCE : m_NearEdgesSmall.Width() / 3 , 
      m_NearEdgesSmall.Height() > CURSOR_TOLERANCE * 4 ?
        CURSOR_TOLERANCE : m_NearEdgesSmall.Height() / 3 ) ;
    m_WholeRectSelection = m_NearEdgesSmall ;
    m_WholeRectSelection.DeflateRect(       
      m_WholeRectSelection.Width() > CURSOR_TOLERANCE * 2 ?
        CURSOR_TOLERANCE : (m_WholeRectSelection.Width() / 2) - 1 , 
     m_WholeRectSelection.Height() > CURSOR_TOLERANCE * 2 ?
        CURSOR_TOLERANCE : (m_WholeRectSelection.Height() / 2) - 1 ) ;

  }
  int CheckCursor( CPoint& Cursor , int iTolerance = CURSOR_TOLERANCE , 
    int * pDist = NULL )
  {

    if ( m_NearEdgesBig.PtInRect( Cursor))
    {
      if ( !m_NearEdgesSmall.PtInRect( Cursor ) ) // i.e. cursor is 
      {                                           // in near border area
        int iRes = PointNearAlignedSegment( Cursor , TopLeft() , 
          (CPoint&)CPoint( right , top ) , iTolerance , pDist ) ;
        switch( iRes )
        {
        case IS_NEAR_FIRST_EDGE:  return CURSOR_ON_LTCORNER ;
        case IS_NEAR_SECOND_EDGE: return CURSOR_ON_TRCORNER ;
        case IS_NEAR_SEGMENT:     return CURSOR_ON_TOP_BORDER ;
        }
        iRes = PointNearAlignedSegment( Cursor ,  
          (CPoint&)CPoint( right , top ) , BottomRight() , iTolerance , pDist ) ;
        switch( iRes )
        {
        case IS_NEAR_FIRST_EDGE:  return CURSOR_ON_TRCORNER ; // should not be
        case IS_NEAR_SECOND_EDGE: return CURSOR_ON_RBCORNER ;
        case IS_NEAR_SEGMENT:     return CURSOR_ON_RIGHT_BORDER ;
        }
        iRes = PointNearAlignedSegment( Cursor ,  
          BottomRight() , (CPoint&) CPoint( left , bottom ) , iTolerance , pDist ) ;
        switch( iRes )
        {
        case IS_NEAR_FIRST_EDGE:  return CURSOR_ON_RBCORNER ; // should not be
        case IS_NEAR_SECOND_EDGE: return CURSOR_ON_BLCORNER ;
        case IS_NEAR_SEGMENT:     return CURSOR_ON_BOTTOM_BORDER ;
        }
        iRes = PointNearAlignedSegment( Cursor ,  
          (CPoint&) CPoint( left , bottom ) , TopLeft() , iTolerance , pDist ) ;
        switch( iRes )
        {
        case IS_NEAR_FIRST_EDGE:  return CURSOR_ON_BLCORNER ; // should not be
        case IS_NEAR_SECOND_EDGE: return CURSOR_ON_TRCORNER ; // should not be
        case IS_NEAR_SEGMENT:     return CURSOR_ON_LEFT_BORDER ;
        }
        //ASSERT(0) ; // something strange
      }
      else if ( !m_WholeRectSelection.PtInRect( Cursor ) )
      {
        if ( pDist )
        {
          int iDist = Cursor.x - left ;
          if ( right - Cursor.x < iDist )
            iDist = right - Cursor.x ;
          if ( Cursor.y - top < iDist )
            iDist = Cursor.y - top ;
          if ( bottom - Cursor.y < iDist )
            iDist = bottom - Cursor.y ;
          *pDist = iDist ;
        }
        return CURSOR_ON_WHOLE_SELECTION ;
      }
    }
    return 0 ;
  }
  CRect m_NearEdgesBig ;
  CRect m_NearEdgesSmall ;
  CRect m_WholeRectSelection ;
  int   m_iSelectedSide ;
};

class FXGText : public FXString , public SelectableGraphics
{
public:
  FXGText( const FXString& tSrc , CPoint Pt , int iSz = 12 ,
    DWORD Color = RGB(255,40,40) , DWORD dwBack = 1 )
  {
    this->_copy( tSrc ) ;
    m_Color = Color ;
    m_Coord = Pt ;
    m_iSz = iSz ;
    if ( dwBack != 1 )
    {
      m_Back = dwBack ;
      m_bUseBackGround = true ;
    }
    else
      m_bUseBackGround = false ;
  }
  FXGText( const FXString& tSrc , CDPoint& Pt , int iSz = 12 ,
    DWORD Color = RGB(255,40,40) , DWORD dwBack = 1 )
  {
    this->_copy( tSrc ) ;
    m_Color = Color ;
    m_Coord = CPoint( ROUND( Pt.x ) , ROUND(Pt.y) ) ;
    m_iSz = iSz ;
    if ( dwBack != 1 )
    {
      m_Back = dwBack ;
      m_bUseBackGround = true ;
    }
    else
      m_bUseBackGround = false ;
  }
  FXGText(const FXString& tSrc, cmplx& Pt, int iSz = 12,
    DWORD Color = RGB(255, 40, 40), DWORD dwBack = 1)
  {
    this->_copy(tSrc);
    m_Color = Color;
    m_Coord = CPoint(ROUND(Pt.real()), ROUND(Pt.imag()));
    m_iSz = iSz;
    if (dwBack != 1)
    {
      m_Back = dwBack;
      m_bUseBackGround = true;
    }
    else
      m_bUseBackGround = false;
  }
  FXGText( const FXString& tSrc )
  {
    this->_copy( tSrc ) ;
    m_Color = RGB(255,40,40) ;
    m_Coord = CPoint( -1 , -1 ) ;
    m_iSz = 12 ;
    m_bUseBackGround = false ;
  }
  FXGText( DWORD Color = RGB(255,40,40) )
  {
    m_Color = Color ;
    m_Coord = CPoint(0,0) ;
    m_iSz = 12 ;
    m_bUseBackGround = false ;
  };
  FXGText(const FXGText& tSrc)
  {
    this->_copy( tSrc ) ;
    m_Color = tSrc.m_Color ;
    m_Coord = tSrc.m_Coord ;
    m_iSz = tSrc.m_iSz ;
    m_bUseBackGround = tSrc.m_bUseBackGround ;
    m_Back = tSrc.m_Back ;
  }
  FXGText& operator =( const FXGText& tSrc )
  {
    this->_copy( tSrc ) ;
    m_Color = tSrc.m_Color ;
    m_Coord = tSrc.m_Coord ;
    m_iSz = tSrc.m_iSz ;
    m_bUseBackGround = tSrc.m_bUseBackGround ;
    m_Back = tSrc.m_Back ;
    return *this;
  }
  FXGText& operator =( const FXString& tSrc )
  {
    this->_copy( tSrc ) ;
    m_Color = RGB(255,40,40) ;
    m_Coord = CPoint( -1 , -1 ) ;
    m_iSz = 12 ;
    m_bUseBackGround = false ;
    return *this;
  }

  CPoint m_Coord ;
  int m_iSz ;
  bool m_bUseBackGround ;
};

#define TVBD400_MAXIMUM_QUANTITY_SIZE	256	// bytes

enum GQ_TYP{ GQ_INT=1, GQ_FLOAT, GQ_DPNT , GQ_CMPLX };
typedef struct tagGQUANTITY
{
  GQ_TYP _type;
  union
  {
    int _i;
    double _d;
    DPOINT _dp;
  };
}GQUANTITY, *LPGQUANTITY;

class CGQuantity : public GQUANTITY
{
public:
  CGQuantity(int i)    { _type = GQ_INT;  _i = i; }
  CGQuantity(double d) { _type = GQ_FLOAT; _d = d; }
  CGQuantity(double re, double im){ _type = GQ_CMPLX; _dp.x = re; _dp.y = im; }
  CGQuantity(const DPOINT& c){ _type = GQ_DPNT; _dp = c ; }
  CGQuantity(const cmplx& c){ _type = GQ_CMPLX; _dp.x = c.real() ; _dp.y = c.imag() ; }
  CGQuantity(LPGQUANTITY gq){ memcpy(LPGQUANTITY(this), gq, sizeof(GQUANTITY)); }
  GQ_TYP GetType() { return _type ; } 
  operator int()
  {
    switch (_type)
    {
    case GQ_INT:      return _i;
    case GQ_FLOAT:     return ROUNDPM(_d) ;
    case GQ_DPNT:       return 0;
    case GQ_CMPLX:      return 0;
    }
    ASSERT(FALSE);
    return 0;
  }
  operator long() 
  {
    switch (_type)
    {
    case GQ_INT:      return (long)_i;
    case GQ_FLOAT:     return ROUNDPM(_d) ;
    case GQ_DPNT:       return 0;
    case GQ_CMPLX:      return 0;
    }
    ASSERT(FALSE);
    return 0;
  }
  operator double()
  {
    switch (_type)
    {
    case GQ_INT:      return (double)_i;
    case GQ_FLOAT:     return _d;
    case GQ_DPNT:       return 0.;
    case GQ_CMPLX:      return 0.;
    }
    ASSERT(FALSE);
    return 0;
  }
  operator DPOINT()
  {
    DPOINT c;
    c.y = 0;
    switch (_type)
    {
    case GQ_INT: c.x = (double)_i; return c;
    case GQ_FLOAT:c.x = _d;         return c;
    case GQ_DPNT: 
    case GQ_CMPLX:                   return _dp;
    }
    ASSERT(FALSE);
    return c;
  }
  operator cmplx()
  {
    switch (_type)
    {
    case GQ_INT: return cmplx((double)_i,0.);
    case GQ_FLOAT:return cmplx(_d,0.);
    case GQ_DPNT: 
    case GQ_CMPLX: return cmplx( _dp.x , _dp.y) ;
    }
    ASSERT(FALSE);
    return cmplx(0.,0.);
  }

  operator LPGQUANTITY() { return (LPGQUANTITY)this; };
  bool operator == (CGQuantity& gq)
  {
    if (_type <= GQ_FLOAT && gq.GetType() <= GQ_FLOAT)
      return ( double() == double(gq) );
    return ( cmplx() == cmplx(gq) );
  }

  bool operator != (CGQuantity& gq)
  {
    if (_type <= GQ_FLOAT && gq.GetType() <= GQ_FLOAT)
      return ( double() != double(gq) );
    return ( cmplx() != cmplx(gq) );
  }
  bool operator <= (CGQuantity& gq)
  {
    if (_type <= GQ_FLOAT && gq.GetType() <= GQ_FLOAT)
      return ( double() <= double(gq) );
    return ( abs(cmplx()) <= abs(cmplx(gq)) );
  }
  bool operator >= (CGQuantity& gq)
  {
    if (_type <= GQ_FLOAT && gq.GetType() <= GQ_FLOAT)
      return ( double() >= double(gq) );
    return ( abs(cmplx()) >= abs(cmplx(gq)) );
  }
  bool operator <  (CGQuantity& gq)
  {
    if (_type <= GQ_FLOAT && gq.GetType() <= GQ_FLOAT)
      return ( double() < double(gq) );
    return ( abs(cmplx()) < abs(cmplx(gq)) );
  }
  bool operator >  (CGQuantity& gq)
  {
    if (_type <= GQ_FLOAT && gq.GetType() <= GQ_FLOAT)
      return ( double() > double(gq) );
    return ( abs(cmplx()) > abs(cmplx(gq)) );
  }
  CGQuantity& operator = (int i)
  {
    _type = GQ_INT;
    _i = i;
    return *this;
  }
  CGQuantity& operator = (double d)
  {
    _type = GQ_FLOAT;
    _d = d;
    return *this;
  }
  CGQuantity& operator = (DPOINT& c)
  {
    _type = GQ_DPNT;
    _dp = c;
    return *this;
  }
  CGQuantity& operator = (cmplx& c)
  {
    _type = GQ_CMPLX;
    _dp.x = c.real() ;
    _dp.y = c.imag() ;
    return *this;
  }
  FXString ToString()
  {
    FXString retVal;
    switch (_type)
    {
    case GQ_INT:
      retVal.Format(_T("%d"), _i);
      break;
    case GQ_FLOAT:
      retVal.Format(_T("%g"), _d);
      break;
    case GQ_CMPLX:
    case GQ_DPNT:
      retVal.Format(_T("(%g,i%g)"), _dp.x, _dp.y);
      break;
    }
    return retVal;
  }
  void FromString(LPCTSTR str) // necessary to invent format for
  {                            // CDPoint and cmplx
    FXString s(str);
    s.Remove(' ') ;
    s.Remove('\t') ;
    s.TrimLeft("-+1234567890");
    if (s.IsEmpty())
    {
      _type = GQ_INT;
      _i = atoi(str);
    }
    else 
    {
      int iOParePos = (int) s.Find('(') ;
      int iPointPos = (int) s.Find('.') ;
      int iCommaPos = (int) s.Find(',') ;
      if ( iOParePos >= 0 )
      {
        int iCParePos = (int) s.Find(')') ;
        if ( iCParePos > iOParePos )
        {
          s = s.Mid(iOParePos + 1 , iCParePos - iOParePos - 1 ) ;
          if ( iCommaPos > iOParePos  &&  iCommaPos < iCParePos )
          {
            int iNScanned = sscanf( (LPCTSTR)s , _T("%lf,%lf") , 
              &_dp.x , &_dp.y ) ;
            ASSERT( iNScanned == 2 ) ;
            _type = GQ_CMPLX ;
            return ;
          }
          else if ( iCommaPos == -1 )
          {
            _type = GQ_FLOAT;
            _d = atof(str);
            return ;
          }
        }
        ASSERT(0) ;
        _type = GQ_FLOAT;
        _d = 0. ;
      }
      else 
      {
          _type = GQ_FLOAT;
          _d = atof(str);
      }
    }
  }

protected:
  CGQuantity()
  {
    memset(LPGQUANTITY(this), 0, sizeof(GQUANTITY));
  }
};

class CQuantity : public CGQuantity
{
public:
  CQuantity( const CGQuantity& qsrc , CPoint Pt , int iSz = 12 ,
    DWORD Color = RGB(255,40,40) )
  {
    ((CGQuantity)*this) = qsrc ;
    m_Color = Color ;
    m_Coord = Pt ;
    m_iSz = iSz ;
  }
  CQuantity( DWORD Color = RGB(255,40,40) )
  {
    m_Color = Color ;
    m_Coord = CPoint(0,0) ;
    m_iSz = 12 ;
  };
  CQuantity(const CQuantity& qSrc)
  {
    *this = qSrc ;
  }
  //   CQuantity& operator =( const CQuantity& qSrc )
  //   {
  //     *this = qSrc ;
  //     return *this;
  //   }

  DWORD m_Color ;
  CPoint m_Coord ;
  int m_iSz ;
};

// class GFigures: public FXArray<FXGFigure,FXGFigure&> {};
// class GPoints: public FXArray<CGPoint,CGPoint&> {};
// class Rectangles: public FXArray<FXRectangle,FXRectangle&> {};
// class Texts: public FXArray<FXGText,FXGText&> {};
// class Quantities: public FXArray<CQuantity,CQuantity&> {};
typedef FXArray<FXGFigure,FXGFigure&> GFigures;
typedef FXArray<CGPoint,CGPoint&> GPoints;
typedef FXArray<FXRectangle,FXRectangle&> Rectangles;
typedef FXArray<FXGText,FXGText&> Texts;
typedef FXArray<CQuantity,CQuantity&> Quantities;

//typedef CArray< FXGFigure , FXGFigure& > GFigures ;
// typedef CArray< CGPoint , CGPoint& > GPoints ;
// typedef CArray< FXRectangle , FXRectangle& > Rectangles ;
// typedef CArray< FXGText , FXGText& > Texts ;
// typedef CArray< CQuantity , CQuantity& > Quantities ;

class GraphicsData  
{
public:
  GFigures    m_Figures ;
  GPoints     m_Points ;
  Rectangles  m_Rects ;
  Texts       m_Texts ;
  Quantities  m_Quantities ;
  bool        m_bFilled ;
  int          m_iSelectedType ;
  int          m_iSelectedIndex ;
  int          m_iSelectedForAdjustmentsIndex ;
  int          m_iSelectedSide ;
  CRect        m_LastSelectedArea ;
  FXString     m_SelectedName ;
  FXString     m_SelectedForAdjustmentName ;
  FXString     m_LastSelectedRectName ;
  double      m_Scale;
  double      m_ScrScale;
  CPoint      m_ScrOffset;
  CPoint      m_IntBmOffset;  //Offset of the object inside of bitmap
  FXArray <CFont* , CFont*> m_Fonts;
  CUIntArray   m_Sizes;
  CRect        m_RectOriginForModification ;

public:
  GraphicsData( GFigures * pFigures = NULL ,
    GPoints * pPoints = NULL , Rectangles * pRects = NULL ,
    Texts * pTexts = NULL , Quantities * pQuantities = NULL )
  {
    if ( pFigures )
      m_Figures.Copy( *pFigures );
    if ( pPoints )
      m_Points.Copy( *pPoints );
    if ( pRects )
      m_Rects.Copy( *pRects );
    if ( pTexts )
      m_Texts.Copy( *pTexts );
    if ( pQuantities )
      m_Quantities.Copy( *pQuantities );
    m_bFilled = (pFigures != NULL) || (pPoints != NULL)
      || (pRects != NULL) || (pTexts != NULL)
      || (pQuantities != NULL) ;
    m_iSelectedForAdjustmentsIndex = m_iSelectedIndex = -1 ;
  }
  ~GraphicsData()
  {
  }
  void Clean()
  {
  }
  void RemoveData( bool bCheckSelected = false )
  {
    if ( !bCheckSelected || ( m_iSelectedIndex < 0 ) )
    {
    m_Rects.RemoveAll() ;
    m_Points.RemoveAll() ;
    }
    else
    {
       if ( m_iSelectedType == SELECTED_RECT ) 
       {
         if ( m_iSelectedIndex >= (int) m_Rects.GetCount() )
           m_Rects.RemoveAll() ;
         else if ( m_iSelectedIndex == 0  )
         {
           if ( m_Rects.GetCount() > 1 )
             m_Rects.RemoveAt( 1 , m_Rects.GetCount() - 1 ) ;
         }
         else
         {
           FXRectangle SelectedRect = m_Rects[m_iSelectedIndex] ;
           m_Rects.RemoveAll() ;
           m_Rects.Add( SelectedRect ) ;
           m_iSelectedIndex = 0 ;
         }
         m_Points.RemoveAll() ;
       }
       else if ( m_iSelectedType == SELECTED_POINT )
       {
         if ( m_iSelectedIndex >= (int) m_Points.GetCount() )
           m_Points.RemoveAll() ;
         else if ( m_iSelectedIndex == 0  )
         {
           if ( m_Points.GetCount() > 1 )
             m_Points.RemoveAt( 1 , m_Points.GetCount() - 1 ) ;
         }
         else
         {
           CGPoint SelectedPoint = m_Points[m_iSelectedIndex] ;
           m_Points.RemoveAll() ;
           m_Points.Add( SelectedPoint ) ;
           m_iSelectedIndex = 0 ;
         }
         m_Rects.RemoveAll() ;
       }
    }
    m_Figures.RemoveAll() ;
    m_Texts.RemoveAll() ;
    m_Quantities.RemoveAll() ;
    SetFilled( false ) ;
  }
  bool IsFilled() { return m_bFilled ; }
  void SetFilled( bool bSet ) { m_bFilled = bSet ; }
  FXGFigure * GetFigureByName( LPCTSTR Name , int * iIndex = NULL )
  {
    for ( int i = 0 ; i < (int) m_Figures.GetCount() ; i++ )
    {
      if ( m_Figures[i].m_ObjectName == Name )
      {
        *iIndex = i ;
        return &m_Figures[ i ] ;
      }
    }
    return NULL ;
  }
  virtual bool RemoveFigureByName( LPCTSTR Name )
  {
    for ( int i = 0 ; i < (int) m_Figures.GetCount() ; i++ )
    {
      if ( m_Figures[i].m_ObjectName == Name )
      {
        m_Figures.RemoveAt( i ) ;
        return true ;
      }
    }
    return false ;
  }
  bool    CreateFontWithSize( UINT uiSize ) ;
  CFont * GetFont( UINT uiSize ) ;
  virtual bool DrawTexts( CDC* dc , bool bInverseBW = false );
  virtual bool DrawFigures( CDC* dc , bool bInverseBW = false );
  virtual bool DrawRectangles( CDC* dc , bool bInverseBW = false );
  virtual bool DrawTexts( HDC hdc , bool bInverseBW = false ) ;
  virtual bool DrawFigures( HDC hdc , bool bInverseBW = false );
  virtual bool DrawRectangles( HDC hdc , bool bInverseBW = false );
  virtual bool LoadGraphics( const CDataFrame * df ) ;
  virtual void Pic2Scr( CPoint &point );
  virtual bool Pic2Scr( DPOINT& pt , CPoint& res ) ;
  virtual void SubPix2Scr( CPoint &point );
  virtual void Scr2Pic( CPoint &point );
  virtual int  Pic2Scr( double len );
  virtual int  Scr2Pic( double len );
  virtual bool CorrectRect( CRect& SelectedRect ,
    CPoint SrcPt , CPoint CursorPnt , SelectionByCursor Side ) ;

} ;

typedef GraphicsData * pGraphicsData ;

inline LPCTSTR GetSelectedSideName( int iSideIndex )
{
  switch( iSideIndex )
  {
    case CURSOR_NO_SELECTION     :return "Not selected" ;
    case CURSOR_ON_LEFT_BORDER   :return "Left" ;
    case CURSOR_ON_TOP_BORDER    :return "Top" ;
    case CURSOR_ON_RIGHT_BORDER  :return "Right" ;
    case CURSOR_ON_BOTTOM_BORDER :return "Bottom" ;
    case CURSOR_ON_LTCORNER      :return "Left-Top" ;
    case CURSOR_ON_TRCORNER      :return "Right-Top" ;
    case CURSOR_ON_RBCORNER      :return "Right-Bottom" ;
    case CURSOR_ON_BLCORNER      :return "Left-Bottom" ;
    case CURSOR_ON_WHOLE_SELECTION: return "Whole" ;
  }
  return "Unknown" ;
}
