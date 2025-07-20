// GraphBuilder.h: interface for the CGraphBuilder class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GRAPHBUILDER_H__D12B759A_7971_4717_938D_CCD9FE3037ED__INCLUDED_)
#define AFX_GRAPHBUILDER_H__D12B759A_7971_4717_938D_CCD9FE3037ED__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Gadgets\shkernel.h>
#include <shbase\shbase.h>
#include "timer.h"
#include "GadgetFactory.h"
#include "PluginLoader.h"

class CNamedGadgets : public CMapStringToPtr
{
public:
  CNamedGadgets();
  ~CNamedGadgets();

  CGadget* Lookup( LPCTSTR uid );
  CGadget* GetNextGadget( POSITION& pos , FXString& uid );
  CGadget* GetNextGadget( POSITION& pos );
  CGadget* GetNextGadgetOrdered( FXString& uid , LPCTSTR uidLast );
  void ListUIDs( CStringArray& UIDs );
  bool GetGadgetUID( CGadget* gadget , FXString& name );
  void RemoveKey( FXString& uid , BOOL bEraseMemory = TRUE );
  void RemoveAll();
};

class CGraphBuilder : public IGraphbuilder
{
  FXString        m_UID;
  CGadgetFactory  m_GadgetFactory;
  CNamedGadgets   m_Gadgets;
  static CHPTimer m_GraphTimer;
  CPluginLoader	  m_PluginLoader;
  CMapStringToPtr m_InputWrappers;
  FXLockObject		m_InputWrappersLock;
  BOOL            m_bIsDirty;
  FXLockObject		m_PropLock;
  CViewSectionParser m_ViewSection;
  LockingFIFO<FXString> m_ModifiedUIDs ;
public:
  CGraphBuilder( CExecutionStatus* Status );
  virtual ~CGraphBuilder();

  // IGraphbuilder members
protected:
  //public:
  virtual void _stdcall ShutDown();
public:
  virtual void    _stdcall Start();
  virtual void    _stdcall Stop();
  virtual void    _stdcall Pause();
  virtual void    _stdcall StepFwd();
  virtual BOOL    _stdcall IsRuning();
  virtual BOOL    _stdcall IsPaused();
  virtual FXString _stdcall GetID();
  virtual void    _stdcall SetID( LPCTSTR newID );

  virtual BOOL   _stdcall SetDirty( BOOL bDirty = TRUE )
  {
    BOOL bWasDirty = m_bIsDirty;
    m_bIsDirty = bDirty;
    //TRACE("Builder %s is%sdirty\n", GetID(), (m_bIsDirty ? _T(" ") : _T(" not "))); 
    return bWasDirty;
  };
  virtual BOOL   _stdcall IsDirty();

  virtual void _stdcall Detach();

  virtual BOOL _stdcall Save( LPCTSTR fileName , CStringArray* BlockUIDs = NULL );
  virtual int  _stdcall Load( LPCTSTR fileName , LPCTSTR script = NULL , bool bClearOld = true );
  virtual BOOL _stdcall GetScriptPath( FXString& uid , FXString& script );
  virtual BOOL _stdcall GetScript( FXString& script , CStringArray* BlockUIDs = NULL );
  virtual BOOL _stdcall GetProperties( FXString& props );
  virtual void _stdcall SetProperties( LPCTSTR props );

  // Access to view section. Subject of changes
  virtual void _stdcall ViewSectionSetConfig( bool DoGlyphs , bool DoFloatWnds )
  {
    m_ViewSection.SetConfig( DoGlyphs , DoFloatWnds );
  }
  virtual void _stdcall ViewSectionSetGraphPos( CDRect& Pos )
  {
    m_ViewSection.SetGraphViewPos( Pos ) ;
  }
  virtual void _stdcall ViewSectionGetGraphPos( CDRect& Pos )
  {
    m_ViewSection.GetGraphViewPos( Pos ) ;
  }
  virtual CPoint ViewSectionGetViewOffset() { return m_ViewSection.GetViewOffset() ; }
  virtual void ViewSectionSetViewOffset( CPoint offset ) { m_ViewSection.SetViewOffset( offset ) ; };
  virtual double ViewSectionGetViewScale() { return m_ViewSection.GetViewScale() ; };
  virtual void ViewSectionSetViewScale( double scale ) { m_ViewSection.SetViewScale( scale ); };
  virtual void _stdcall SetViewed( BOOL bViewed )
  {
    m_ViewSection.SetViewed( bViewed ) ;
  }
  virtual BOOL _stdcall GetViewed()
  {
    return m_ViewSection.GetViewed() ;
  }

  virtual BOOL _stdcall SetGlyph( LPCTSTR gadget , int x , int y )
  {
    m_bIsDirty = true; return (m_ViewSection.SetGlyph( gadget , x , y ) != false);
  }
  virtual BOOL _stdcall RemoveGlyph( LPCTSTR name )
  {
    return (m_ViewSection.RemoveGlyph( name ) != false);
  }
  virtual BOOL _stdcall SetFloatWnd( LPCTSTR name , double x ,
    double y , double w , double h , LPCTSTR pSelected = NULL );
  virtual BOOL _stdcall GetFloatWnd( LPCTSTR name , double& x ,
    double& y , double& w , double& h , FXString * pSelected = NULL );

  virtual IPluginLoader* _stdcall GetPluginLoader()
  {
    return &m_PluginLoader;
  };

  virtual double _stdcall GetTime()
  {
    return m_GraphTimer.GetTime();
  };

  virtual void _stdcall SendMsg( int msgLevel , LPCTSTR src , int msgId , LPCTSTR msgText )
  {
    if ( m_pMsgQueue ) m_pMsgQueue->AddMsg( msgLevel , src , msgId , msgText );
  };
  virtual void _stdcall PrintMsg( int msgLevel , LPCTSTR src , int msgId , LPCTSTR msgText );

  virtual BOOL _stdcall CreateGadget( LPCTSTR GadgetClassName )
  {
    return m_GadgetFactory.CreateGadget( GadgetClassName );
  };
  virtual UINT _stdcall RegisterCurGadget( FXString& uid , LPCTSTR params = NULL );
  virtual void _stdcall UnregisterGadget( FXString& uid );
  virtual UINT _stdcall ListGadgetConnectors( FXString& uidGadget , CStringArray& inputs , CStringArray& outputs , CStringArray& duplex );
  virtual BOOL _stdcall RegisterGadgetClass( CRuntimeGadget* RuntimeGadget )
  {
    return m_GadgetFactory.RegisterGadgetClass( RuntimeGadget );
  };
  virtual void _stdcall UnregisterGadgetClass( CRuntimeGadget* RuntimeGadget );
  virtual CGadget* _stdcall GetGadget( LPCTSTR uid );
  virtual UINT _stdcall KindOf( FXString& uid );

  virtual BOOL _stdcall AggregateBlock( FXString& uid , CStringArray& Gadgets , LPCTSTR loadPath = NULL );
  virtual BOOL _stdcall ExtractBlock( FXString& uid , CMapStringToString* renames = NULL );

  virtual BOOL _stdcall ConnectRendererAndMonitor( LPCTSTR uid , CWnd* pParentWnd , LPCTSTR Monitor , CRenderGadget* &RenderGadget );
  virtual BOOL _stdcall SetRendererCallback( FXString& uid , RenderCallBack rcb , void* cbData );
  virtual BOOL _stdcall GetRenderMonitor( FXString& uid , FXString& monitor );
  virtual BOOL _stdcall SetOutputCallback( LPCTSTR idPin , OutputCallback ocb , void* pClient );
  virtual BOOL _stdcall SendDataFrame( CDataFrame* pDataFrame , LPCTSTR idPin );

  virtual BOOL _stdcall Connect( const FXString& uid1 , const FXString& uid2 );
  virtual BOOL _stdcall Disconnect( LPCTSTR uidConnector );
  virtual BOOL _stdcall Disconnect( LPCTSTR pin1 , LPCTSTR pin2 );
  virtual BOOL _stdcall IsConnected( LPCTSTR uidConnector , CStringArray& uidTo );
  virtual BOOL _stdcall IsValid( LPCTSTR UID );
  virtual void _stdcall EnumGadgetLinks( FXString& uidGadget , CStringArray& dstGadgets );
  virtual BOOL _stdcall GetOutputIDs( const FXString& uidPin , FXString& uidOut , CStringArray* uidInComplementary );
  virtual BOOL _stdcall RenderDebugOutput( FXString& uidPin , CWnd* pRenderWnd );
  virtual BOOL _stdcall RenderDebugInput( FXString& uidPin , CWnd* pRenderWnd );
  virtual BOOL _stdcall GetElementInfo( FXString& uid , FXString& infostring );
  virtual BOOL _stdcall RenameGadget( FXString& uidOld , FXString& uidNew , CStringArray* uidsToChange , CStringArray* uidsNewNames );
  virtual BOOL _stdcall GetPinLabel( FXString& uidPin , FXString& label );
  virtual BOOL _stdcall SetPinLabel( FXString& uidPin , LPCTSTR label );

  virtual BOOL _stdcall IsGadgetSetupOn( FXString& uid );
  virtual BOOL _stdcall GetGadgetMode( FXString& uid , int& mode );
  virtual BOOL _stdcall SetGadgetMode( FXString& uid , int mode );
  virtual BOOL _stdcall GetOutputMode( FXString& uid , CFilterGadget::OutputMode& mode );
  virtual BOOL _stdcall SetOutputMode( FXString& uid , CFilterGadget::OutputMode mode );
  virtual BOOL _stdcall GetGadgetThreadsNumber( FXString& uid , int& n );
  virtual BOOL _stdcall SetGadgetThreadsNumber( FXString& uid , int n );
  virtual BOOL _stdcall SetGadgetStatus( FXString& uid , LPCTSTR statusname , bool status );
	virtual BOOL _stdcall GetGadgetStatus(FXString& uid, LPCTSTR statusname, bool& status);

  virtual BOOL _stdcall GetGadgetIsMultyCoreAllowed( FXString& uid );

  virtual UINT _stdcall GetGadgetClassAndLineage( FXString& uidGadget , FXString& Class , FXString& Lineage );
  virtual void _stdcall EnumGadgets( CStringArray& srcGadgets , CStringArray& dstGadgets );
  virtual void _stdcall EnumGadgetClassesAndLineages( CUIntArray& Types , CStringArray& Classes , CStringArray& Lineages )
  {
    m_GadgetFactory.EnumGadgetClassesAndLineages( Types , Classes , Lineages );
  };
  virtual void _stdcall EnumGadgetClassesAndPlugins( CStringArray& Classes , CStringArray& Plugins )
  {
    m_GadgetFactory.EnumGadgetClassesAndPlugins( Classes , Plugins );
  };

  virtual BOOL _stdcall IsComplexGadget( FXString& uid );
  virtual BOOL _stdcall IsLibraryComplexGadget( FXString& uid );
  virtual void _stdcall SetLocalComplexGadget( FXString& uid );


  virtual IGraphbuilder* _stdcall GetSubBuilder( FXString& uid );

  virtual BOOL _stdcall EnumModifiedGadgets( CStringArray& Gadgets );

  virtual int  _stdcall GetGraphInputsCount();
  virtual CInputConnector* _stdcall GetGraphInput( int n );
  virtual int  _stdcall GetGraphOutputsCount();
  virtual COutputConnector* _stdcall GetGraphOutput( int n );
  virtual int _stdcall  GetGraphDuplexPinsCount();
  virtual CDuplexConnector* _stdcall GetGraphDuplexPin( int n );

  virtual void _stdcall EnumPluginLoaders( CPtrArray& PluginLoaders );
  virtual bool _stdcall ScanProperties( LPCTSTR gadgetUID , LPCTSTR text , bool& Invalidate );
  virtual bool _stdcall PrintProperties( LPCTSTR gadgetUID , FXString& text );
  virtual bool _stdcall ScanSettings( LPCTSTR gadgetUID , FXString& text );
  virtual void _stdcall SetExecutionStatus( CExecutionStatus* Status );

  //protected:
  virtual UINT _stdcall RegisterGadget( FXString& uid , void* Gadget );
  virtual void _stdcall GrantGadgets( IGraphbuilder* pBuilder , CStringArray* uids = NULL , CMapStringToString* renames = NULL );
  virtual void _stdcall Dummy()
  {};
  BOOL GetGadgetName( CGadget* gadget , FXString& Name );
private:
  CConnector* GetConnector( const FXString& uid );
  void		EncodeGadgetLinks( FXString& uidGadget , CStringArray& Links , CStringArray* ToGadgets = NULL );
  void		TranslateBlock( CStringArray& uids , CStringArray& script );
  BOOL		FindConnector( CConnector* Connector , FXString& uid , FXString* uidHost = NULL );
  void		ValidateUID( FXString& uid );
  static double __stdcall GetGraphTime();
public:
  // Function returns graph gadget list in alphabet order
  virtual int EnumAndArrangeGadgets( FXStringArray& GadgetList );
  virtual void * GetModifiedUIDSsArray() { return &m_ModifiedUIDs ; }  ;
};

#endif // !defined(AFX_GRAPHBUILDER_H__D12B759A_7971_4717_938D_CCD9FE3037ED__INCLUDED_)
