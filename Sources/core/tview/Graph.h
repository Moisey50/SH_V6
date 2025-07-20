// Graph.h: interface for the CGraph class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GRAPH_H__D039FF4A_4F82_4208_AB79_A6D0763035A2__INCLUDED_)
#define AFX_GRAPH_H__D039FF4A_4F82_4208_AB79_A6D0763035A2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <gadgets\shkernel.h>
#include "afxtempl.h"
#include <gadgets\tview.h>

#define GLYPH_CONNECTOR_OFFSET 2

//#define GLYPHS_DRAW_PLAIN
#define GLYPHS_DRAW_ROUND
//#define GLYPHS_DRAW_EDGE

#define TVDB400_TRACE_GLYPHS

#ifdef TVDB400_TRACE_GLYPHS

class CGlyphsHeap
{
private:
  static CGlyphsHeap* m_pThis;
  CMapPtrToPtr m_Glyphs;
  FXLockObject m_Lock;
  volatile int m_Refs;
protected:
  CGlyphsHeap();
  ~CGlyphsHeap();
public:
  static CGlyphsHeap* Get();
  BOOL Register( void* pGlyph );
  BOOL Unregister( void* pGlyph );
  void AddRef() { m_Refs++; };
  void Release() { if ( !--m_Refs ) delete this; };
};

#endif


//
// CGlyph
//  CConnectorGlyph
//  CCompositeGlyph
//   CGadgetGlyph
//    CRenderGlyph
//   CCollectionGlyph
//   CAggregateGlyph
//

class CSketchView;
class CSketchViewMod;
class CCollectionGlyph;
class CGlyph
{
protected:
  CSketchView *m_View;
  CPoint  m_Pos;
  int     m_idIcon;
  int     m_EvadeCount; //for connector glyphs it's a flag for drawing (0 - connections still not drawn). For others - number of evasions of connectors
  CPtrArray m_EvadedGlyphs;
  ViewType m_ViewType;
  int     cornerCounter[ 4 ];
public:
  CGlyph( CSketchView *view ) :
    m_View( view )
  {
    m_Pos = CPoint( 0 , 0 );
    m_idIcon = -1;
    m_EvadeCount = 0;
#ifdef TVDB400_TRACE_GLYPHS
    CGlyphsHeap::Get()->Register( this );
#endif
  };

  CGlyph( CSketchView *view , ViewType vt ) :
    m_View( view )
  {
    m_Pos = CPoint( 0 , 0 );
    m_idIcon = -1;
    m_EvadeCount = 0;
    SetViewType( vt );
#ifdef TVDB400_TRACE_GLYPHS
    CGlyphsHeap::Get()->Register( this );
#endif
  };

  CGlyph( CSketchView *view , const CGlyph& gl ) :
    m_View( view )
  {
    m_Pos = gl.m_Pos;
    m_idIcon = gl.m_idIcon;
    m_EvadeCount = gl.m_EvadeCount;
    SetViewType( gl.m_ViewType );
#ifdef TVDB400_TRACE_GLYPHS
    CGlyphsHeap::Get()->Register( this );
#endif
  };

  virtual ~CGlyph()
  {
#ifdef TVDB400_TRACE_GLYPHS
    CGlyphsHeap::Get()->Unregister( this );
#endif
  };

  virtual void IncEvadeCount() { m_EvadeCount++; }
  virtual void DecEvadeCount() { m_EvadeCount--; }
  virtual void ResetEvadeCount() { m_EvadeCount = 0; m_EvadedGlyphs.RemoveAll(); }
  virtual void AddEvadedGlyph( void *gl ) { m_EvadedGlyphs.Add( gl ); }
  virtual void* GetEvadedGlyph( int i ) { return m_EvadedGlyphs.GetAt( i ); }
  virtual int GetEvadedGlyphSize() { return ( int ) m_EvadedGlyphs.GetSize(); }
  virtual int GetEvadeCount() { return m_EvadeCount; }
  virtual BOOL SearchEvadedGlyph( void *gl );
  virtual BOOL IsGadgetGlyph() { return FALSE; }

  virtual void ResetCornerCounter() { for ( int i = 0; i < 4; i++ ) cornerCounter[ i ] = 0; }
  virtual void IncCornerCounter( int index ) { cornerCounter[ index ]++; }
  virtual void DecCornerCounter( int index ) { cornerCounter[ index ]--; }
  virtual int  GetCornerCounter( int index ) { return cornerCounter[ index ]; }

  virtual void SetViewType( ViewType vt ) { m_ViewType = vt; }
  virtual ViewType GetViewType() { return m_ViewType; }

  virtual BOOL ConnectTo( CGlyph* pGlyph ) { return FALSE; };
  virtual BOOL DisconnectWire( CGlyph* pGlyph ) { return FALSE; };
  virtual CGlyph* FindByUID( LPCTSTR uid ) { return NULL; };

  virtual BOOL GetUID( FXString& uid ) { return FALSE; };
  virtual void SetUID( LPCTSTR uid ) {};

  virtual void Draw( CDC* pDC , bool bActivityOnly = false ) = 0;
  virtual BOOL GetRgn( CRgn* pRgn , BOOL bSelfRgn = FALSE ) = 0;
  virtual CGlyph* Intersect( CPoint& pt , BOOL bSelfRgn/* = TRUE*/ ) = 0;
  CPoint GetPos() { return m_Pos; };
  virtual void OffsetPos( CPoint& offset );
  void SetIcon( int idIcon ) { m_idIcon = idIcon; };
  virtual int GetIcon() { return m_idIcon; };
  virtual BOOL IsDraggable() { return TRUE; };
  virtual CCollectionGlyph* Expand() { return NULL; };
  virtual void SetNeedUpdate( BOOL bNeed = TRUE ) {};
  virtual BOOL IsSelectionPriorityHigh() { return TRUE; };
  virtual CWnd* GetDebugWnd() { return NULL; };
  virtual void EnumOpenDebugWnds( CStringArray& uids ) {};
  virtual BOOL HasContextMenu() { return FALSE; };
  virtual void FillContextMenu( CMenu* Menu ) {};
  virtual void DestroyContextSubmenus() {};

  virtual void DestroyIn() = 0;
  virtual void CleanUp() = 0;
  virtual void StopDebuggingPins() {};
  virtual void ShowSetupDlgAt( CPoint& point ) {};
  virtual void ShowAffinityDlgAt( CPoint& point ) {};
};

class CCompositeGlyph : public CGlyph
{
protected:
  CArray<CGlyph* , CGlyph*> m_Components;
public:
  CCompositeGlyph( CSketchView *view ) : CGlyph( view ) 
  { m_Components.SetSize( 0 , 1 ); };
  CCompositeGlyph( CSketchView *view , ViewType vt ) : CGlyph( view , vt ) 
  { m_Components.SetSize( 0 , 1 ); SetViewType( vt ); }
  CCompositeGlyph( CSketchView *view , const CCompositeGlyph& cp ) : CGlyph( view ) 
  { m_Components.Copy( cp.m_Components ); }
  virtual ~CCompositeGlyph() {};

  virtual void ResetEvadeCount();

  virtual void SetViewType( ViewType vt );

  void	Add( CGlyph* Component ) { m_Components.Add( Component ); };
  void	Insert( CGlyph* Component , int i ) 
  { m_Components.InsertAt( i , Component ); };
  virtual	CGlyph* FindByUID( LPCTSTR uid );
  bool	Remove( CGlyph* Component , bool bFreeMemory = false );
  CGlyph*	GetChildAt( int i ) { return m_Components.GetAt( i ); };
  int		GetChildrenCount() { return ( int ) m_Components.GetSize(); };
  virtual void EnumOpenDebugWnds( CStringArray& uids );

  virtual void Draw( CDC* pDC , bool bActivityOnly = false );
  virtual BOOL GetRgn( CRgn* pRgn , BOOL bSelfRgn = FALSE );
  virtual CGlyph* Intersect( CPoint& pt , BOOL bSelfRgn );
  virtual BOOL Intersect( CGlyph* gl );

  virtual void OffsetPos( CPoint& offset );

  virtual void DestroyIn();
  virtual void CleanUp();
  virtual void StopDebuggingPins();
  virtual void DestroyIn( BOOL delete_from_collection );
};

#define NO_DATA_PASSED 0
#define DATA_PASSED    1
#define DATA_SKIPPED   2

enum EnumConnectorType
{
  Enum_CT_Unknow = -1 ,
  Enum_CT_Input = 0 ,
  Enum_CT_Output ,
  Enum_CT_Control ,
};

class CConnectorGlyph : public CCompositeGlyph
{
protected:
  FXString    m_UID;
  bool        m_bHotSpot;
  CSize       m_Size;
  CGlyph     *father;
  CWnd       *m_pDebugWnd;
  CConnector *m_pConnector;
  EnumConnectorType m_ConnectorType;
public:
  CConnectorGlyph( CSketchView *view , LPCTSTR uid );
  CConnectorGlyph( CSketchView *view , LPCTSTR uid , ViewType vt );
  CConnectorGlyph( CSketchView *view , LPCTSTR uid , CGlyph *f );
  CConnectorGlyph( CSketchView *view , LPCTSTR uid , ViewType vt , CGlyph *f );
  virtual BOOL GetUID( FXString& uid ) { uid = m_UID; return TRUE; };
  virtual void Draw( CDC* pDC , bool bActivityOnly = false );
  virtual BOOL GetRgn( CRgn* pRgn , BOOL bSelfRgn = FALSE );
  virtual CGlyph* Intersect( CPoint& pt , BOOL bSelfRgn );
  virtual BOOL IsDraggable() { return FALSE; };
  virtual void DestroyIn();
  virtual void CleanUp();
  virtual void StopDebuggingPins();
  virtual BOOL ConnectTo( CGlyph *pGlyph );
  virtual BOOL DisconnectWire( CGlyph *pGlyph );
  virtual CPoint GetConnectionPoint();
  virtual CGlyph* GetFather();
  virtual CWnd* GetDebugWnd();
  virtual void EnumOpenDebugWnds( CStringArray& uids );
  virtual BOOL SwitchConnectionTo( CGlyph *glyph );
  virtual CGlyph* FindByUID( LPCTSTR uid );
  virtual void SetUID( LPCTSTR uid ) { m_UID = uid; };
  virtual BOOL HasContextMenu() { return TRUE; };
  virtual void FillContextMenu( CMenu* Menu );
  virtual int GetDataPassed();
  CGadget * GetGadgetAndPin( EnumConnectorType& Type , int& iPin );
  CConnector * GetConnector();
};

class CWireGlyph : public CGlyph
{
protected:
  FXString m_UID;	// universal identifier created by graph builder
  CGlyph *pInput , *pOutput;
public:
  CWireGlyph( CSketchView *view , LPCTSTR uid );
  CWireGlyph( CSketchView *view , LPCTSTR uid , ViewType vt );

  virtual BOOL SetConnection( CGlyph* pInput , CGlyph *pOutput );
  virtual BOOL SetInputConnection( CGlyph *input );
  virtual BOOL SetOutputConnection( CGlyph *output );
  virtual CGlyph* FindByUID( LPCTSTR uid ) { return ( uid == m_UID ) ? this : NULL; };

  virtual void Draw( CDC* pDC , bool bActivityOnly = false );
  virtual BOOL GetRgn( CRgn* pRgn , BOOL bSelfRgn = FALSE );
  virtual CGlyph* Intersect( CPoint& pt , BOOL bSelfRgn );
  virtual BOOL IsDraggable() { return TRUE; };
  virtual void DestroyIn();
  virtual void CleanUp();
  virtual BOOL SwitchConnectionTo( CGlyph *glyph );
  virtual CGlyph* GetInput() { return pInput; }
  virtual CGlyph*GetOutput() { return pOutput; }
  virtual BOOL GetUID( FXString &uid ) { uid = m_UID; return TRUE; }
  virtual BOOL IsSelectionPriorityHigh() { return FALSE; };
  virtual void SetUID( LPCTSTR uid ) { m_UID = uid; };

  virtual void Disconnect();
};

class CEvadeWireGlyph : public CWireGlyph
{
private:
  CArray<CPoint , CPoint> endArray;
public:
  CEvadeWireGlyph( CSketchView *view , LPCTSTR uid );
  CEvadeWireGlyph( CSketchView *view , LPCTSTR uid , ViewType vt );
  virtual void Draw( CDC* pDC , bool bActivityOnly = false );
  virtual BOOL GetRgn( CRgn* pRgn , BOOL bSelfRgn = FALSE );
  virtual BOOL GetLocalRgn( CRgn* pRgn , int i , BOOL bSelfRgn = FALSE );
  virtual void SetEndArrayRev( CArray <CPoint , CPoint> *eA );
protected:
  void DrawGlyph( CDC* pDC );
  virtual BOOL _GetRgn( CRgn* pRgn , BOOL bSelfRgn = FALSE );
  virtual CRect FindSuitableCornerDirection( CRect rcLink , CRect res , CRect res_measured , int evCnt , int xc , int yc , int start , int *increm , CGlyph *gl , bool tryDifferent );
  virtual int CheckForRepeatCorners( int cornerCounter[ 4 ] );
  virtual bool CheckMins( int a1 , int a2 , int delta , int min );
  virtual double calcDTDummy( CRect rcLink );
  virtual int GetNextXcYcDummy( CRect rcLink , double t , int &xc , int &yc );
  virtual double calcDTStraight( CRect rcLink );
  virtual int GetNextXcYcStraight_xFirst( CRect rcLink , double t , int &xc , int &yc );
  virtual int GetNextXcYcStraight_yFirst( CRect rcLink , double t , int &xc , int &yc );
  virtual CArray<CPoint , CPoint>* GetEndArray() { return &endArray; };
  //search functions - its problem is filling endArray with appropriate values
  virtual void DummySearch();
  virtual void MySearch();
  virtual int DistToRect( CRect rc , CPoint pt );
  virtual void StraightSearch();
  //help functions for mysearch
  virtual int GetToHorizon( CSketchViewMod * view , CPoint input , CPoint output );
  virtual int SlideToPoint( CSketchViewMod * view , CPoint input , CPoint output );
  virtual void OptimizeTrack( CSketchViewMod * view );
  virtual CPoint MoveVertToObstacle( CSketchViewMod * view , CPoint * pPos , int startDir , LONG limit );
  virtual CPoint MoveHorzToObstacle( CSketchViewMod * view , CPoint * pPos , int startDir , LONG limit );
  virtual CPoint BypassObstacleHorz( CSketchViewMod * view , CPoint * pPos , int startDir , bool bClockwise , CPoint output );
  virtual CPoint BypassObstacleVert( CSketchViewMod * view , CPoint * pPos , int startDir , bool bClockwise , CPoint output );
  virtual CPoint PierceObstacleShortest( CSketchViewMod * view , CPoint *pPos );
  virtual CPoint PierceObstacle( CSketchViewMod * view , CPoint * pPos , int startDir );
  virtual CPoint CheckForCycles( CSketchViewMod * view , CPoint output );
  int GetNextDirection( int iDir , bool bClockwise );
  void MovePoint( CPoint *pPt , int iDir );
};


class CGadgetGlyph : public CCompositeGlyph
{
  friend class CPortalGlyph ;
  FXString        m_UID;		// universal identifier created by graph builder
  CSize           m_Size;
  int             m_nInputs;
  int             m_nOutputs;
  int             m_nDuplex;
  int             m_nThreads;
  DWORD           m_dwBodyColor = 0xffffff ;
  BOOL            m_bNeedUpdate;	// flagged if glyph possibly needs to be reconstructed
  CMenu           m_ModesMenu , m_ThreadsMenu , m_OutputModeMenu;
public:
  CGadgetGlyph( CSketchView *view , LPCTSTR uid ,
    CStringArray& inputs , CStringArray& outputs , CStringArray& duplex );
  CGadgetGlyph( CSketchView *view , LPCTSTR uid ,
    CStringArray& inputs , CStringArray& outputs , CStringArray& duplex ,
    ViewType vt );

  virtual void ResetEvadeCount();
  virtual BOOL IsGadgetGlyph() { return TRUE; }
  
  virtual CGlyph* FindByUID( LPCTSTR uid );
  virtual BOOL GetUID( FXString& uid );
  virtual void SetUID( LPCTSTR uid );

  virtual void Draw( CDC* pDC , bool bActivityOnly = false );
  virtual BOOL GetRgn( CRgn* pRgn , BOOL bSelfRgn = FALSE );
  virtual CGlyph* Intersect( CPoint& pt , BOOL bSelfRgn );
  virtual void DestroyIn();
  virtual void CleanUp();
  virtual void ShowSetupDlgAt( CPoint& point );
  virtual void ShowAffinityDlgAt( CPoint& point );
  virtual void SetNeedUpdate( BOOL bNeed = TRUE ) { m_bNeedUpdate = bNeed; };
  virtual BOOL HasContextMenu() { return TRUE; };
  virtual void FillContextMenu( CMenu* Menu );
  virtual void DestroyContextSubmenus();
private:
  void ArrangeChildren();
protected:
  void UpdatePins( CStringArray& inputs , CStringArray& outputs , CStringArray& duplex );
  void SetSize( CSize& sz ) { m_Size = sz; ArrangeChildren(); };
public:
  CSize GetSize() const { return m_Size; };
  virtual void SetBodyColor( DWORD dwColor ) { m_dwBodyColor = dwColor ; };
  virtual DWORD GetBodyColor() { return m_dwBodyColor ; };
};

class CComplexGlyph : public CGadgetGlyph
{
public:
  CComplexGlyph( CSketchView *view , LPCTSTR uid , CStringArray& inputs , CStringArray& outputs , CStringArray& duplex ) :
    CGadgetGlyph( view , uid , inputs , outputs , duplex )
  {};
  CComplexGlyph( CSketchView *view , LPCTSTR uid , CStringArray& inputs , CStringArray& outputs , CStringArray& duplex , ViewType vt ) :
    CGadgetGlyph( view , uid , inputs , outputs , duplex , vt )
  {};

  virtual void Draw( CDC* pDC , bool bActivityOnly = false );
  virtual BOOL GetRgn( CRgn* pRgn , BOOL bSelfRgn = FALSE );
  virtual void FillContextMenu( CMenu* Menu );
};


class CFloatWnd;
class CRenderViewFrame ;
class CRenderGlyph : public CGadgetGlyph
{
  friend class CRenderViewFrame;
  CRenderViewFrame*  m_pWndFrame;
  CRenderGadget*     m_pRenderGadget;
  CSize				 m_defaultWndSize;
  bool				m_constructed;
public:
  CRenderGlyph( CSketchView *view , LPCTSTR uid , CStringArray& inputs , CStringArray& outputs , CStringArray& duplex );
  CRenderGlyph( CSketchView *view , LPCTSTR uid , CStringArray& inputs , CStringArray& outputs , CStringArray& duplex , ViewType vt );
  virtual ~CRenderGlyph();
  virtual void DestroyIn();
  CRenderGadget* GetGadget() { return m_pRenderGadget; };
  CWnd* GetRenderWnd();
  virtual void Draw( CDC* pDC , bool bActivityOnly = false );
  bool IsConstructed() const { return m_constructed; }
  int GetIcon();
private:
  void UpdateSize( CSize sz );
  void SetNewDefaultWndSize( CSize sz ) { m_defaultWndSize = sz; }
};

class CCollectionGlyph : public CCompositeGlyph
{
public:
  CCollectionGlyph( CSketchView *view ) :CCompositeGlyph( view ) {};
  CCollectionGlyph( CSketchView *view , const CCollectionGlyph* pCollection );
  CCollectionGlyph( CSketchView *view , ViewType vt ) :CCompositeGlyph( view ) { SetViewType( vt ); }
  CCollectionGlyph( CSketchView *view , const CCollectionGlyph* pCollection , ViewType vt );
  virtual ~CCollectionGlyph() {};

  void Select( CCompositeGlyph* pGlyph );
  void IntersectRect( LPRECT rc );
  virtual BOOL IsDraggable();
  virtual void DestroyIn();
};

class CPortalGlyph : public CGadgetGlyph
{
public:
  CPortalGlyph( CSketchView *view , LPCTSTR uid ,
    CStringArray& inputs , CStringArray& outputs , CStringArray& dummy );
  CPortalGlyph( CSketchView *view , LPCTSTR uid ,
    CStringArray& inputs , CStringArray& outputs ,
    CStringArray& dummy , ViewType vt );

  virtual void ResetEvadeCount();
  virtual BOOL IsGadgetGlyph() { return TRUE; }

  virtual void Draw( CDC* pDC , bool bActivityOnly = false );
  virtual BOOL GetRgn( CRgn* pRgn , BOOL bSelfRgn = FALSE );
  virtual CGlyph* Intersect( CPoint& pt , BOOL bSelfRgn );
  virtual void DestroyIn();
  virtual void CleanUp();
  virtual void ShowSetupDlgAt( CPoint& point );
  virtual void SetNeedUpdate( BOOL bNeed = TRUE ) { m_bNeedUpdate = bNeed; };
  virtual BOOL HasContextMenu() { return TRUE; };
  virtual void FillContextMenu( CMenu* Menu );
  virtual void DestroyContextSubmenus();
private:
  void ArrangeChildren();
protected:
  void UpdatePins( CStringArray& inputs , CStringArray& outputs );
  void SetSize( CSize& sz ) { m_Size = sz; ArrangeChildren(); };
public:
  CSize GetSize() const { return m_Size; };
};

class CAggregateGlyph : public CGadgetGlyph
{
  CCollectionGlyph* m_pInternalGlyphs;
public:
  CAggregateGlyph( CSketchView *view , CCollectionGlyph* pInternals ,
    LPCTSTR uid , CStringArray& inputs , CStringArray& outputs ,
    CStringArray& duplex );
  CAggregateGlyph( CSketchView *view , CCollectionGlyph* pInternals ,
    LPCTSTR uid , CStringArray& inputs , CStringArray& outputs ,
    CStringArray& duplex , ViewType vt );

  virtual void OffsetPos( CPoint& offset );
  virtual CCollectionGlyph* Expand();
  virtual void DestroyIn();
  virtual void ShowSetupDlgAt( CPoint& point );

private:
  void SwitchConnections( BOOL bOnVirtualPins );
};

#endif // !defined(AFX_GRAPH_H__D039FF4A_4F82_4208_AB79_A6D0763035A2__INCLUDED_)
