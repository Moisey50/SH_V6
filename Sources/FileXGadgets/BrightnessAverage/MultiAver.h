// MultiAver.h : Declaration of the MultiAver class



#ifndef __INCLUDE__MultiAver_H__
#define __INCLUDE__MultiAver_H__


#pragma once
#include "helpers\UserBaseGadget.h"
#include <math\Intf_sup.h>
#include <gadgets\ContainerFrame.h>
#include <helpers\FramesHelper.h>
#include <helpers\PropertyKitEx.h>

enum MA_WorkingMode
{
  MA_WM_Unknown = 0 ,
  MA_WM_Teaching ,
  MA_WM_Average 
};


class NamedRectangle
{
public:
  CRect m_Rect ;
  FXString m_ObjectName ;
  COLORREF m_Color ;
  FXString m_AsString ;
  double   m_dLastValue ;

  NamedRectangle( LPCTSTR pObjectName = NULL ,
    CRect * pRect = NULL , COLORREF Color = RGB(255,0,0) )
  {
    if ( pObjectName )
      m_ObjectName = pObjectName ;
    if ( pRect )
      m_Rect = *pRect ;
    else
      memset( &m_Rect , 0 , sizeof( m_Rect ) ) ;
    m_Color = Color ;
    m_dLastValue = 0. ;
    m_AsString = ToString() ;
  }

  bool ScanProperties( LPCTSTR pStr )
  {
    FXPropKit2 pk( pStr ) ;
    pk.Trim() ;
    if ( IsOpenBracket( pk[ 0 ] ) )
      pk.Delete( 0 ) ;
    bool bRes = pk.GetString( _T( "Obj" ) , m_ObjectName ) ;
    bool bResRect = GetArray( pk , _T( "R" ) , _T( 'd' ) , 4 , &m_Rect ) == 4 ;
    if ( bResRect )
    {
      m_Rect.right += m_Rect.left ;
      m_Rect.bottom += m_Rect.top ;
    }
    bRes |= bResRect ;
    FXString ColorAsString ;
    if ( pk.GetString( _T("C") , ColorAsString ) )
    {
      FXSIZE Tmp ;
      ConvToBinary( ColorAsString , Tmp ) ;
      m_Color = ( COLORREF )Tmp ;
      bRes = true ;
    }

    m_AsString = ToString() ;
    m_dLastValue = 0. ;
    return bRes ;
  }
  bool PrintProperties( FXString& Result )
  {

    FXPropKit2 pk ;
    pk.WriteString( _T( "Obj" ) , m_ObjectName ) ;
    CRect r = m_Rect ;
    r.right -= r.left ;
    r.bottom -= r.top ;
    WriteArray( pk , _T( "R" ) , _T( 'd' ) , 4 , r ) ;
    pk.WriteUIntNotDecimal( _T( "C" ) , (UINT) m_Color , _T( 'x' ) ) ;
    Result += (_T( '(' )) ;
    Result += pk + _T( ")" ) ;
    return true ;
  }
  NamedRectangle& operator=( const NamedRectangle& Orig )
  {
    m_Rect = Orig.m_Rect ;
    m_ObjectName = Orig.m_ObjectName ;
    m_Color = Orig.m_Color ;
    m_AsString = Orig.m_AsString ;
    m_dLastValue = Orig.m_dLastValue ;
    return *this ;
  }

  FXString ToString()
  {
    FXString AsText ;
    PrintProperties( AsText ) ; 
    return AsText ;
  }

  bool FromString( FXString& Orig )
  {
    return ScanProperties( Orig ) ;
  }
};

class NamedRectangles : public FXArray<NamedRectangle> {};




class MultiAver : public UserBaseGadget
{
protected:
	MultiAver();
  FXLockObject    m_ParametersProtect ;
  NamedRectangles m_Areas ;
  NamedRectangles m_AreasForGUI ;
  bool           m_bAreasChanged ;
  MA_WorkingMode m_WorkingMode ;
  int            m_iRadius ;
  FXString       m_GadgetInfo ;
  int            m_iDeleteIndex ;
  FXString       m_DeleteAsString ;
  int            m_iIterator ;
  double         m_dLastBase ;
  double         m_dLastWhiteValue ;
  double         m_dLastCorrection ;
  double         m_dBaseAverage ;
  int            m_iAveraging ;
  int            m_iNAccumulated ;
  double         m_dK ;
  double         m_dOffset ;
  
public:

	void PropertiesRegistration();
	void ConnectorsRegistration();
  virtual bool PrintProperties( FXString& text ) ;
  virtual bool ScanProperties( LPCTSTR text , bool& Invalidate ) ;
  virtual bool ScanSettings( FXString& Settings ) ;
  static void ConfigParamChange( LPCTSTR pName , void* pGadget , 
    bool& bInvalidate , bool& bRescanProperties) ;

	CDataFrame* DoProcessing(const CDataFrame* pDataFrame);
	void AsyncTransaction(CDuplexConnector* pConnector, CDataFrame* pParamFrame);
  bool PrintOwnProperties( FXString& text ) ;
  bool SaveConfigParameters( LPCTSTR pFileName ) ;
  bool LoadConfigParameters( LPCTSTR pFileName ) ;
  int CleanDoubledAreas() ;

	DECLARE_RUNTIME_GADGET(MultiAver);
};

#endif	// __INCLUDE__MultiAver_H__

