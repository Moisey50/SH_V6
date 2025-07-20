#pragma once
#include <fxfc/fxfc.h>
#include <gadgets/gadbase.h>
#include <shcore.h>
#include "helpers/UserBaseGadget.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>      // std::stringstream

enum GM_Object
{
  GM_Unknown = -1 ,
  GM_Text ,
  GM_Figure ,
  GM_Arc
};

class GMarkObject
{
public:
  GM_Object m_Type ;
  DWORD     m_Color ;
  DWORD     m_Size ; // size for text, cross size for points
  DWORD     m_Back ; // background color for text, if == 1, no background
  DWORD     m_Thickness ; // Line thickness
  cmplx     m_cArcCenter ;
  double    m_dRadius_pix ;
  double    m_dAngFrom_deg ;
  double    m_dAngTo_deg ;
  bool      m_bIsCircle = false ;
  bool      m_bIsInitialized ;
  BOOL      m_bClose = FALSE ;
  FXString  m_Text ;
  FXString  m_ParamsAsString ;
  FXString   m_CoordsAsText ;
  CmplxVector m_Coords ;

  GMarkObject( LPCTSTR AsText = NULL ) ;
  GMarkObject( GMarkObject& Origin ) ;

  GMarkObject& operator=( GMarkObject& Origin ) ;
  CDataFrame * GetArcFrame( const cmplx& cVFSize ) ;
  CFigureFrame * GetFigureFrame( const cmplx& cVFSize ) ;
  CTextFrame * GetTextFrame( const cmplx& cVFSize ) ;
  CDataFrame * GetFrameForOutput( const cmplx& cVFSize ) ;
  void GetProperties() ;
  int ScanProperties( LPCTSTR pAsText ) ;
  int AnalyseCoordsForCircle() ;
  LPCTSTR GetTypeAsString() 
  {
    switch( m_Type )
    {
//      case GM_Unknown: return "Unknown" ;
      case GM_Text: return "Text" ;
      case GM_Figure: return "Figure" ;
      case GM_Arc: return "Arc" ;
    }
    return "Unknown" ;
  }
};

typedef vector<GMarkObject> GMarkObjects ;

class GMarking :
  public UserBaseGadget 
{
public:
  FXString m_GadgetName ;
  int      m_iNObjects = 0 ;
  bool     m_bRescanProp = false ;
//   GMarkObjects m_Objects ;
  FXLockObject m_Lock ; // control and processing inter protection
  GMarkObject  m_GObjects[ 50 ] ;

public:
  GMarking() ;

  void PropertiesRegistration();
  void ConnectorsRegistration();
  static void ConfigParamChange( LPCTSTR pName , 
    void* pObject , bool& bInvalidate , bool& bInitRescan ) ;
  CDataFrame* DoProcessing( const CDataFrame* pDataFrame );
  void AsyncTransaction( CDuplexConnector* pConnector , CDataFrame* pParamFrame );
  virtual void OnScanSettings() ;
  DECLARE_RUNTIME_GADGET( GMarking );
};

