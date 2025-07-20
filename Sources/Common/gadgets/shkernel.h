// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the SHKERNEL_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// SHKERNEL_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifndef _SHKERNEL_INCLUDED
#define _SHKERNEL_INCLUDED

#include <gadgets\gadbase.h>

#ifdef _DEBUG
#define SHKERNEL_DLL_NAME "shkerneld.dll"
#define SHKERNEL_LIB_NAME "shkerneld.lib"
#else
#define SHKERNEL_DLL_NAME "shkernel.dll"
#define SHKERNEL_LIB_NAME "shkernel.lib"
#endif

#ifndef SHKERNEL_DLL
#define FX_EXT_KERNEL __declspec(dllimport)
#pragma comment(lib, SHKERNEL_LIB_NAME)
#else
#define FX_EXT_KERNEL __declspec(dllexport)
#endif

// Plugin entries
#if defined(_WIN64)
    #define ENTRY_GETHELPITEM   _T("?GetHelpItem@@YAIPEBD@Z")
    #define ENTRY_GETPLUGINNAME _T("?GetPluginName@@YAPEBDXZ")
    #define ENTRY_GETSWVERSION  _T("?GetSWVersion@@YAPEBDXZ")
    #define ENTRY_PLUGINENTRY   _T("?PluginEntry@@YAXPEAX@Z")
    #define ENTRY_PLUGINEXIT    _T("?PluginExit@@YAXPEAX@Z")
#else
    #define ENTRY_GETHELPITEM _T("?GetHelpItem@@YGIPBD@Z")
    #define ENTRY_GETPLUGINNAME _T("?GetPluginName@@YGPBDXZ")
    #define ENTRY_GETSWVERSION _T("?GetSWVersion@@YGPBDXZ")
    #define ENTRY_PLUGINENTRY _T("?PluginEntry@@YGXPAX@Z")
    #define ENTRY_PLUGINEXIT _T("?PluginExit@@YGXPAX@Z")
#endif
// gadget help system
void FX_EXT_KERNEL attachshkernelDLL();

class CDRect ;

typedef struct taghelpitem
{
  char      classname[ GADGET_NAME_MAXLENGTH ];
  unsigned  helpID;
}helpitem;

class CGraphSettingsDialog ;

class IGraphbuilder
{
protected:
  volatile DWORD    m_cRefs;
  IGraphMsgQueue*   m_pMsgQueue;
  CExecutionStatus* m_pStatus;
  CSetupObject    * m_pGraphSetupObject ;
protected:
  IGraphbuilder();
  virtual ~IGraphbuilder();
public:
  enum
  {
    TVDB400_GT_ANY = 0 ,
    TVDB400_GT_GADGET ,
    TVDB400_GT_CAPTURE ,
    TVDB400_GT_FILTER ,
    TVDB400_GT_RENDER ,
    TVDB400_GT_CTRL ,
    TVDB400_GT_COMPLEX ,
    TVDB400_GT_PORT ,
    TVDB400_GT_OTHER ,
  };

public:

  FX_EXT_KERNEL void _stdcall SetMsgQueue( IGraphMsgQueue* pMsgQueue );
  FX_EXT_KERNEL IGraphMsgQueue* _stdcall GetMsgQueue();
  FX_EXT_KERNEL void AddRef();
  FX_EXT_KERNEL DWORD Release();
  void SetGraphSetupObject( CSetupObject    * pGraphSetupObject ) 
  {
    if ( m_pGraphSetupObject )
      m_pGraphSetupObject->Delete();
    m_pGraphSetupObject = pGraphSetupObject;
  };
  FX_EXT_KERNEL BOOL IsSetupOn() ;
  CSetupObject * GetGraphSetupObject()
  {
    return m_pGraphSetupObject ;
  }

  /// Virtual prototypes

protected:
  //public:
  virtual void _stdcall ShutDown() = 0;	// Intended to destroy current graph

public:
  virtual void _stdcall Start() = 0;
  virtual void _stdcall Stop() = 0;
  virtual void _stdcall Pause() = 0;
  virtual void _stdcall StepFwd() = 0;
  virtual BOOL _stdcall IsRuning() = 0;
  virtual BOOL _stdcall IsPaused() = 0;
  virtual FXString _stdcall GetID() = 0;
  virtual void    _stdcall SetID( LPCTSTR newID ) = 0;
  virtual BOOL _stdcall SetDirty( BOOL set = TRUE ) = 0;
  virtual BOOL _stdcall IsDirty() = 0;

  virtual BOOL _stdcall Save( LPCTSTR fileName , CStringArray* BlockUIDs = NULL ) = 0;
  virtual int _stdcall  Load( LPCTSTR fileName , LPCTSTR script = NULL , bool bClearOld = true ) = 0;
  virtual BOOL _stdcall GetScriptPath( FXString& uid , FXString& path ) = 0;
  virtual BOOL _stdcall GetScript( FXString& script , CStringArray* BlockUIDs = NULL ) = 0;
  virtual BOOL _stdcall GetProperties( FXString& props ) = 0;
  virtual void _stdcall SetProperties( LPCTSTR props ) = 0;

  // Access to view section. Subject of changes
  virtual void _stdcall ViewSectionSetConfig( bool DoGlyphs , bool DoFloatWnds ) = 0;
  virtual void _stdcall ViewSectionSetGraphPos( CDRect& Pos ) = 0;
  virtual void _stdcall ViewSectionGetGraphPos( CDRect& Pos ) = 0;
  virtual CPoint ViewSectionGetViewOffset() = 0 ;
  virtual void ViewSectionSetViewOffset( CPoint offset ) = 0 ;
  virtual double ViewSectionGetViewScale() = 0 ;
  virtual void ViewSectionSetViewScale( double scale ) = 0 ;
  virtual void _stdcall SetViewed( BOOL bViewed ) = 0;
  virtual BOOL _stdcall GetViewed() = 0;
  virtual BOOL _stdcall SetGlyph( LPCTSTR gadget , int x , int y ) = 0;
  virtual BOOL _stdcall RemoveGlyph( LPCTSTR name ) = 0;
  virtual BOOL _stdcall SetFloatWnd( LPCTSTR name , double x , double y , 
    double w , double h , LPCTSTR pSelected = NULL ) = 0;
  virtual BOOL _stdcall GetFloatWnd( LPCTSTR name , double& x , double& y , 
    double& w , double& h , FXString * pSelected = NULL ) = 0;

  virtual double _stdcall  GetTime() = 0;

  virtual void _stdcall SendMsg( int msgLevel , LPCTSTR src , int msgId , LPCTSTR msgText ) = 0;
  virtual void _stdcall PrintMsg( int msgLevel , LPCTSTR src , int msgId , LPCTSTR msgText ) = 0;

  virtual BOOL _stdcall CreateGadget( LPCTSTR GadgetClassName ) = 0;
  virtual UINT _stdcall RegisterCurGadget( FXString& uid , LPCTSTR params = NULL ) = 0;
  virtual void _stdcall UnregisterGadget( FXString& uid ) = 0;
  virtual UINT _stdcall ListGadgetConnectors( FXString& uidGadget , CStringArray& inputs , CStringArray& outputs , CStringArray& duplex ) = 0;
  virtual BOOL _stdcall RegisterGadgetClass( CRuntimeGadget* RuntimeGadget ) = 0;
  virtual void _stdcall UnregisterGadgetClass( CRuntimeGadget* RuntimeGadget ) = 0;
  virtual CGadget* _stdcall GetGadget( LPCTSTR uid ) = 0;
  virtual UINT _stdcall KindOf( FXString& uid ) = 0;

  virtual BOOL _stdcall AggregateBlock( FXString& uid , CStringArray& Gadgets , LPCTSTR loadPath = NULL ) = 0;
  virtual BOOL _stdcall ExtractBlock( FXString& uid , CMapStringToString* renames = NULL ) = 0;

  virtual BOOL _stdcall ConnectRendererAndMonitor( LPCTSTR uid , CWnd* pParentWnd , LPCTSTR Monitor , CRenderGadget* &RenderGadget ) = 0;
  virtual BOOL _stdcall SetRendererCallback( FXString& uid , RenderCallBack rcb , void* cbData ) = 0;
  virtual BOOL _stdcall GetRenderMonitor( FXString& uid , FXString& monitor ) = 0;
  virtual BOOL _stdcall SetOutputCallback( LPCTSTR idPin , OutputCallback ocb , void* pClient ) = 0;
  virtual BOOL _stdcall SendDataFrame( CDataFrame* pDataFrame , LPCTSTR idPin ) = 0;

  virtual BOOL _stdcall Connect( const FXString& uid1 , const FXString& uid2 ) = 0;
  virtual BOOL _stdcall Disconnect( LPCTSTR uidConnector ) = 0;
  virtual BOOL _stdcall Disconnect( LPCTSTR pin1 , LPCTSTR pin2 ) = 0;
  virtual BOOL _stdcall IsConnected( LPCTSTR uidConnector , CStringArray& uidTo ) = 0;
  virtual BOOL _stdcall IsValid( LPCTSTR UID ) = 0;
  virtual void _stdcall EnumGadgetLinks( FXString& uidGadget , CStringArray& dstGadgets ) = 0;
  virtual BOOL _stdcall GetOutputIDs( const FXString& uidPin , FXString& uidOut , CStringArray* uidInComplementary ) = 0;
  virtual BOOL _stdcall RenderDebugOutput( FXString& uidPin , CWnd* pRenderWnd ) = 0;
  virtual BOOL _stdcall RenderDebugInput( FXString& uidPin , CWnd* pRenderWnd ) = 0;
  virtual BOOL _stdcall GetElementInfo( FXString& uid , FXString& infostring ) = 0;
  virtual BOOL _stdcall RenameGadget( FXString& uidOld , FXString& uidNew , CStringArray* uidsToChange , CStringArray* uidsNewNames ) = 0;
  virtual BOOL _stdcall GetPinLabel( FXString& uidPin , FXString& label ) = 0;
  virtual BOOL _stdcall SetPinLabel( FXString& uidPin , LPCTSTR label ) = 0;

  virtual BOOL _stdcall IsGadgetSetupOn( FXString& uid ) = 0;
  virtual BOOL _stdcall GetGadgetMode( FXString& uid , int& mode ) = 0;
  virtual BOOL _stdcall SetGadgetMode( FXString& uid , int mode ) = 0;
  virtual BOOL _stdcall GetOutputMode( FXString& uid , CFilterGadget::OutputMode& mode ) = 0;
  virtual BOOL _stdcall SetOutputMode( FXString& uid , CFilterGadget::OutputMode mode ) = 0;
  virtual BOOL _stdcall GetGadgetThreadsNumber( FXString& uid , int& n ) = 0;
  virtual BOOL _stdcall SetGadgetThreadsNumber( FXString& uid , int n ) = 0;
  virtual BOOL _stdcall SetGadgetStatus( FXString& uid , LPCTSTR statusname , bool status ) = 0;
	virtual BOOL _stdcall GetGadgetStatus(FXString& uid, LPCTSTR statusname, bool& status) = 0;

  virtual BOOL _stdcall GetGadgetIsMultyCoreAllowed( FXString& uid ) = 0;

  virtual UINT _stdcall GetGadgetClassAndLineage( FXString& uidGadget , FXString& Class , FXString& Lineage ) = 0;
  virtual void _stdcall EnumGadgets( CStringArray& srcGadgets , CStringArray& dstGadgets ) = 0;
  virtual void _stdcall EnumGadgetClassesAndLineages( CUIntArray& Types , CStringArray& Classes , CStringArray& Lineages ) = 0;
  virtual void _stdcall EnumGadgetClassesAndPlugins( CStringArray& Classes , CStringArray& Plugins ) = 0;

  virtual BOOL _stdcall IsComplexGadget( FXString& uid ) = 0;
  virtual BOOL _stdcall IsLibraryComplexGadget( FXString& uid ) = 0;
  virtual void _stdcall SetLocalComplexGadget( FXString& uid ) = 0;

  virtual IGraphbuilder* _stdcall GetSubBuilder( FXString& uid ) = 0;

  virtual BOOL _stdcall EnumModifiedGadgets( CStringArray& Gadgets ) = 0;

  virtual int _stdcall  GetGraphInputsCount() = 0;
  virtual CInputConnector* _stdcall GetGraphInput( int n ) = 0;
  virtual int _stdcall  GetGraphOutputsCount() = 0;
  virtual COutputConnector* _stdcall GetGraphOutput( int n ) = 0;
  virtual int _stdcall  GetGraphDuplexPinsCount() = 0;
  virtual CDuplexConnector* _stdcall GetGraphDuplexPin( int n ) = 0;

  virtual bool _stdcall ScanProperties( LPCTSTR gadgetUID , LPCTSTR text , bool& Invalidate ) = 0;
  virtual bool _stdcall PrintProperties( LPCTSTR gadgetUID , FXString& text ) = 0;
  virtual bool _stdcall ScanSettings( LPCTSTR gadgetUID , FXString& text ) = 0;
  virtual void _stdcall SetExecutionStatus( CExecutionStatus* Status ) = 0;
  //protected:
  virtual IPluginLoader* _stdcall GetPluginLoader() = 0;
  virtual void _stdcall EnumPluginLoaders( CPtrArray& PluginLoaders ) = 0;
  virtual UINT _stdcall RegisterGadget( FXString& , void* ) = 0;
  virtual void _stdcall GrantGadgets( IGraphbuilder* pBuilder , CStringArray* uids = NULL , CMapStringToString* renames = NULL ) = 0;
  virtual void _stdcall Detach() = 0;
  virtual int EnumAndArrangeGadgets( FXStringArray& GadgetList ) = 0 ;
  virtual void * GetModifiedUIDSsArray() = 0 ;
};

FX_EXT_KERNEL IGraphbuilder* Tvdb400_CreateBuilder( CExecutionStatus* Status = NULL );	// access to graph builder
FX_EXT_KERNEL IGraphbuilder* Tvdb400_GetNetBuilder( LPCTSTR host , WORD port ); // access to network graph builder
FX_EXT_KERNEL BOOL           GetUserLibraryPath( FXString& path );
FX_EXT_KERNEL LPCTSTR        GetUserLibExtension();

// just for compatibility with old versions
typedef IGraphbuilder IGraphBuilderEx;



//
// Complex
//
class FX_EXT_KERNEL Complex : public CGadget
{
  IGraphbuilder* m_pBuilder;
  FXString    m_LoadPath;
  CPtrArray   m_Outputs;
  CPtrArray   m_Inputs;
  CPtrArray   m_Duplex;
  CExecutionStatus* m_pStatus;
protected:
  Complex();
public:
  virtual void ShutDown();
  void Collapse();
  void InitExecutionStatus( CExecutionStatus* Status );
  virtual int GetInputsCount();
  virtual int GetOutputsCount();
  virtual int GetDuplexCount();
  CInputConnector* GetInputConnector( int n );
  COutputConnector* GetOutputConnector( int n );
  CDuplexConnector* GetDuplexConnector( int n );
  virtual bool PrintProperties( FXString& text );
  virtual bool ScanProperties( LPCTSTR text , bool& Invalidate );
  virtual bool ScanSettings( FXString& text );
  virtual void SetBuilder( IGraphbuilder* pBuilder , BOOL bLoadLocalPlugins = TRUE );
  virtual void SetBuilderName( LPCTSTR name )
  {
    m_pBuilder->SetID( name );
  };
  void SetLoadPath( LPCTSTR lpszPath );
  virtual bool IsComplex()
  {
    return true;
  }
  void SetGraphMsgQueue( IGraphMsgQueue* MsgQueue );
  LPCTSTR GetLoadPath() const
  {
    return m_LoadPath;
  };
  void    ClearLoadPath()
  {
    m_LoadPath = "";
  };
  IGraphbuilder* Builder()
  {
    return m_pBuilder;
  };
  virtual const IPluginLoader* GetPluginLoader()
  {
    return ((!m_pBuilder) ? NULL : (const IPluginLoader*) m_pBuilder->GetPluginLoader());
  };
  BOOL IsDirty()
  {
    if ( GetBuilder() ) return GetBuilder()->IsDirty(); return FALSE;
  };
  IGraphbuilder* GetBuilder();
protected:
  void UnregisterConnectors();
  void RegisterConnectors();
private:
  virtual int DoJob();
  friend class ComplexDlg;
  DECLARE_RUNTIME_GADGET( Complex );
};




#endif