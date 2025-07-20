#pragma once

#include <gadgets\FigureFrame.h>
#include <classes\drect.h>
#include <shbase\shbase.h>
#include <helpers/GraphicsOnImage.h>

#define EVT_UPDATE_BY_TIMER 10 


// CPlotGraphView


class CPlotGraphView : public CWnd
{
  DECLARE_DYNAMIC(CPlotGraphView)
protected:
  FXString        m_Name;
  CPoint          m_MousePnt;
  double          m_ScaleX;
  double          m_ScaleY;
  CRect           m_ClientRect;
  CArray <CFont*,CFont*> m_Fonts;
  CUIntArray      m_Sizes;

  FXLockObject    m_DataLock;
  CFigure         m_Data;
  GraphicsData    m_Graphics ;
  CDRect          m_GraphFrame;
  CXYMinMaxes     m_MinMaxes ;
  int             m_iViewNet ;
  CXYMinMaxes     m_Labels ;

  //CPen          m_Pen;
  CGDIBank		m_GDI;

  BOOL			  m_bAutoFit;
  BOOL        m_bViewRanges ;
  int         m_iNSamples ;
  int         m_iTakenDrawings ;
  BOOL        m_bViewNames ;
  BOOL        m_bRemoveAllGraphs ;
  FXString    m_LogFileName ;
  FXString    m_LastTimeStamp ;
  double      m_dSamplePeriod_ms ;
  DWORD       m_dwLastID ;
  vector<BOOL> m_LastUpdatedDrawings ;
  CmplxVector  m_LastValues ;


  UINT_PTR     m_uiTimerID ;
public:
  CPlotGraphView(LPCTSTR name="");
  virtual ~CPlotGraphView();
protected:
  DECLARE_MESSAGE_MAP()

private:
  BOOL    CreateFontWithSize( UINT uiSize ) ;
  CFont * GetFont( UINT uiSize )
  {
    for ( int i = 0 ; i <= m_Fonts.GetUpperBound(); i++ )
    {
      if ( m_Sizes[i] == uiSize ) return m_Fonts[i];
    }
    if (CreateFontWithSize( uiSize ))
      return m_Fonts[m_Fonts.GetUpperBound()];
    else
      return NULL ;
  }



public:
  virtual BOOL Create(CWnd* pParentWnd, DWORD dwAddStyle=0,UINT nID=0);
  void    Render(const CDataFrame* pDataFrame);
  BOOL    GetMouseXY(CDPoint& dpnt);
  void    SetAutoFit( BOOL bAutoFit) { m_bAutoFit = bAutoFit; }
  afx_msg void OnPaint();
  afx_msg void OnMouseMove(UINT nFlags, CPoint point);
  BOOL AddOrCorrectFigure( const CDataFrame * pFrame ) ;
  BOOL ViewSpectrum( const CArrayFrame * pRe , 
    const CArrayFrame * pIm = NULL , const CArrayFrame * pPhase = NULL ) ;
  void ParseAttributes( const CDataFrame * pFrame , FXGFigure *  pTargetFigure ) ;
  PlacementMode AnalyzeLabel( const CDataFrame * pFrame , FXString& DrawingName ) ;
  BOOL AppendToFigure( const CQuantityFrame * pQuantity ) ;
  void SetNSamples( int iNSamples ) { m_iNSamples = iNSamples ; } ;
  int  GetNSamples() { return m_iNSamples ; } ;
  void SetViewNet( int iViewNet ) { m_iViewNet = iViewNet ; } ;
  int  GetViewNet() { return m_iViewNet ; } ;
  void SetLabels( CXYMinMaxes& MinMaxes ) { m_Labels = MinMaxes ; }
  void GetLabels( CXYMinMaxes& MinMaxes ) { MinMaxes = m_Labels ; }
  void SetViewNames( BOOL bSet ) { m_bViewNames = bSet ;}
  BOOL IsViewNames() { return m_bViewNames ; }
  void SetMinMaxes( CXYMinMaxes& MinMaxes ) { m_MinMaxes = MinMaxes ; }
  CXYMinMaxes& GetMinMaxes() { return m_MinMaxes ; }
  void WriteMinMaxes( FXPropertyKit& pk ) ; // write into pk
  BOOL SetMinMaxes( FXPropertyKit& pk ) ; // get from pk and set internal values
  void SetViewRanges( BOOL bView ) { m_bViewRanges = bView ; }
  BOOL GetViewRanges() { return m_bViewRanges ; }
  void SetSamplePeriod( double dPeriod_ms ) ;
  double GetSamplePeriod()  { return m_dSamplePeriod_ms ; }
  void RemoveAllGraphs() { m_bRemoveAllGraphs = true ; }
  afx_msg void OnTimer( UINT_PTR nIDEvent );
  afx_msg void OnSize(UINT nType , int cx , int cy);
};


