#pragma once
#include <gadgets\shkernel.h>
#include <networks\NetInterface.h>
#include <networks\shsprotocol.h>

class CNetBuilder : public IGraphbuilder
{
  INetClient* m_pNetClient;
  FXLockObject	m_Lock , m_InOutLock;
  CSHSMessage* m_pTmpInOut;
  HANDLE m_evResponse;
  DWORD m_nTimeout;
  DWORD m_idNetClient;
  BYTE m_cMsg;
  BOOL m_bRuning;
public:
  static LPCTSTR enumgraphs;
  CNetBuilder( LPCTSTR host , WORD port );
  virtual ~CNetBuilder();

protected:
  void _stdcall ShutDown();	// Intended to destroy current graph

public:
  void _stdcall Start();
  void _stdcall Stop();
  void _stdcall Pause();
  void _stdcall StepFwd();
  BOOL _stdcall IsRuning();
  BOOL _stdcall IsPaused();
  FXString _stdcall GetID();
  void    _stdcall SetID( LPCTSTR newID );
  BOOL _stdcall SetDirty( BOOL set );
  BOOL _stdcall IsDirty();

  BOOL _stdcall Save( LPCTSTR fileName , CStringArray* BlockUIDs = NULL );
  int _stdcall  Load( LPCTSTR fileName , LPCTSTR script = NULL , bool bClearOld = true );
  BOOL _stdcall GetScriptPath( FXString& uid , FXString& path );
  BOOL _stdcall GetScript( FXString& script , CStringArray* BlockUIDs = NULL );
  BOOL _stdcall GetProperties( FXString& props );
  void _stdcall SetProperties( LPCTSTR props );

  // Access to view section. Subject of changes
  void _stdcall ViewSectionSetConfig( bool DoGlyphs , bool DoFloatWnds )
  {
    ASSERT( FALSE );
  };
  virtual void _stdcall ViewSectionSetGraphPos( CDRect& Pos )
  {
    ASSERT( FALSE ) ;
  }
  virtual void _stdcall ViewSectionGetGraphPos( CDRect& Pos )
  {
    ASSERT( FALSE ) ;
  }
  virtual CPoint ViewSectionGetViewOffset() { ASSERT( FALSE ) ; return CPoint() ; }
  virtual void ViewSectionSetViewOffset( CPoint offset ) { ASSERT( FALSE ) ; };
  virtual double ViewSectionGetViewScale() { ASSERT( FALSE ) ; return 1.0 ; };
  virtual void ViewSectionSetViewScale( double scale ) { ASSERT( FALSE ) ; };
  virtual void _stdcall SetViewed( BOOL bViewed )
  {
    ASSERT( FALSE ) ;
  }
  virtual BOOL _stdcall GetViewed() 
  {
    ASSERT( FALSE ) ;
    return FALSE ;
  }

  BOOL _stdcall SetGlyph( LPCTSTR gadget , int x , int y )
  {
    ASSERT( FALSE ); return FALSE;
  }
  BOOL _stdcall RemoveGlyph( LPCTSTR name )
  {
    ASSERT( FALSE ); return FALSE;
  }
  virtual BOOL _stdcall SetFloatWnd( LPCTSTR name , double x ,
    double y , double w , double h , LPCTSTR pSelected = NULL )
  {
    return FALSE /*m_ViewSection.SetFloatWnd( name , x , y , w , h , pSelected )*/;
  }
  virtual BOOL _stdcall GetFloatWnd( LPCTSTR name , double& x ,
    double& y , double& w , double& h , FXString * pSelected = NULL )
  {
    return FALSE/*m_ViewSection.GetFloatWnd( name , x , y , w , h , pSelected )*/;
  }

  double _stdcall  GetTime();

  void _stdcall SendMsg( int msgLevel , LPCTSTR src , int msgId , LPCTSTR msgText );
  void _stdcall PrintMsg( int msgLevel , LPCTSTR src , int msgId , LPCTSTR msgText );

  BOOL _stdcall CreateGadget( LPCTSTR GadgetClassName );
  UINT _stdcall RegisterCurGadget( FXString& uid , LPCTSTR params = NULL );
  void _stdcall UnregisterGadget( FXString& uid );
  UINT _stdcall ListGadgetConnectors( FXString& uidGadget , CStringArray& inputs , CStringArray& outputs , CStringArray& duplex );
  BOOL _stdcall RegisterGadgetClass( CRuntimeGadget* RuntimeGadget );
  void _stdcall UnregisterGadgetClass( CRuntimeGadget* RuntimeGadget );
  CGadget* _stdcall GetGadget( LPCTSTR uid );
  UINT _stdcall KindOf( FXString& uid );

  BOOL _stdcall AggregateBlock( FXString& uid , CStringArray& Gadgets , LPCTSTR loadPath = NULL );
  BOOL _stdcall ExtractBlock( FXString& uid , CMapStringToString* renames = NULL );

  BOOL _stdcall ConnectRendererAndMonitor( LPCTSTR uid , CWnd* pParentWnd , LPCTSTR Monitor , CRenderGadget* &RenderGadget );
  BOOL _stdcall SetRendererCallback( FXString& uid , RenderCallBack rcb , void* cbData );
  BOOL _stdcall GetRenderMonitor( FXString& uid , FXString& monitor );
  BOOL _stdcall SetOutputCallback( LPCTSTR idPin , OutputCallback ocb , void* pClient );
  BOOL _stdcall SendDataFrame( CDataFrame* pDataFrame , LPCTSTR idPin );

  BOOL _stdcall Connect( const FXString& uid1 , const FXString& uid2 );
  BOOL _stdcall Disconnect( LPCTSTR uidConnector );
  BOOL _stdcall Disconnect( LPCTSTR pin1 , LPCTSTR pin2 );
  BOOL _stdcall IsConnected( LPCTSTR uidConnector , CStringArray& uidTo );
  BOOL _stdcall IsValid( LPCTSTR UID );
  void _stdcall EnumGadgetLinks( FXString& uidGadget , CStringArray& dstGadgets );
  BOOL _stdcall GetOutputIDs( const FXString& uidPin , FXString& uidOut , CStringArray* uidInComplementary );
  BOOL _stdcall RenderDebugOutput( FXString& uidPin , CWnd* pRenderWnd );
  BOOL _stdcall RenderDebugInput( FXString& uidPin , CWnd* pRenderWnd );
  BOOL _stdcall GetElementInfo( FXString& uid , FXString& infostring );
  BOOL _stdcall RenameGadget( FXString& uidOld , FXString& uidNew , CStringArray* uidsToChange , CStringArray* uidsNewNames );
  BOOL _stdcall GetPinLabel( FXString& uidPin , FXString& label );
  BOOL _stdcall SetPinLabel( FXString& uidPin , LPCTSTR label );

  BOOL _stdcall IsGadgetSetupOn( FXString& uid );
  BOOL _stdcall GetGadgetMode( FXString& uid , int& mode );
  BOOL _stdcall SetGadgetMode( FXString& uid , int mode );
  BOOL _stdcall GetOutputMode( FXString& uid , CFilterGadget::OutputMode& mode );
  BOOL _stdcall SetOutputMode( FXString& uid , CFilterGadget::OutputMode mode );
  BOOL _stdcall GetGadgetThreadsNumber( FXString& uid , int& n );
  BOOL _stdcall SetGadgetThreadsNumber( FXString& uid , int n );
  BOOL _stdcall GetGadgetIsMultyCoreAllowed( FXString& uid );
  BOOL _stdcall SetGadgetStatus( FXString& uid , LPCTSTR statusname , bool status );
    BOOL _stdcall GetGadgetStatus(FXString& uid, LPCTSTR statusname, bool& status);

  UINT _stdcall GetGadgetClassAndLineage( FXString& uidGadget , FXString& Class , FXString& Lineage );
  void _stdcall EnumGadgets( CStringArray& srcGadgets , CStringArray& dstGadgets );
  void _stdcall EnumGadgetClassesAndLineages( CUIntArray& Types , CStringArray& Classes , CStringArray& Lineages );
  void _stdcall EnumGadgetClassesAndPlugins( CStringArray& Classes , CStringArray& Plugins );

  BOOL _stdcall IsComplexGadget( FXString& uid );
  BOOL _stdcall IsLibraryComplexGadget( FXString& uid );
  void _stdcall SetLocalComplexGadget( FXString& uid );


  IGraphbuilder* _stdcall GetSubBuilder( FXString& uid );

  BOOL _stdcall EnumModifiedGadgets( CStringArray& Gadgets );

  int _stdcall  GetGraphInputsCount();
  CInputConnector* _stdcall GetGraphInput( int n );
  int _stdcall  GetGraphOutputsCount();
  COutputConnector* _stdcall GetGraphOutput( int n );
  int _stdcall  GetGraphDuplexPinsCount();
  CDuplexConnector* _stdcall GetGraphDuplexPin( int n );

  bool _stdcall ScanProperties( LPCTSTR gadgetUID , LPCTSTR text , bool& Invalidate );
  bool _stdcall PrintProperties( LPCTSTR gadgetUID , FXString& text );
  bool _stdcall ScanSettings( LPCTSTR gadgetUID , FXString& text );
  void _stdcall SetExecutionStatus( CExecutionStatus* Status );

  IPluginLoader* _stdcall GetPluginLoader();
  void _stdcall EnumPluginLoaders( CPtrArray& PluginLoaders );
  UINT _stdcall RegisterGadget( FXString& , void* );
  void _stdcall GrantGadgets( IGraphbuilder* pBuilder , CStringArray* uids = NULL , CMapStringToString* renames = NULL );
  void _stdcall Detach();
  int EnumAndArrangeGadgets( FXStringArray& GadgetList ) ;
  virtual void * GetModifiedUIDSsArray() { return NULL ; }  ;

private:
  DWORD MsgUid( DWORD idMsg );
  BOOL Transact( CSHSMessage& msgQuery );
  void Close();
  int NewGraph();
  void* EnumGraphs();
  int LoadGraph( LPCTSTR fileName , bool bClearOld );
  int LoadScript( LPCTSTR script , bool bClearOld );
  void OnReceive( LPCVOID lpData , int cbData );
  void OnNetEvent( UINT nEvent , LPVOID pParam );

  friend void CALLBACK fnClientRecv( LPCVOID lpData , int cbData , LPVOID pHost );
  friend void CALLBACK fnClientEvent( UINT nEvent , LPVOID pParam , LPVOID pHost );
};
