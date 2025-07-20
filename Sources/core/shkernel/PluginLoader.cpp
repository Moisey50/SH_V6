#include "StdAfx.h"
#include "PluginLoader.h"
#include <Gadgets\shkernel.h>

#define THIS_MODULENAME _T("PluginLoader.LoadPlugin")

#define USE_AFX_LOADLIBRARY
#ifdef USE_AFX_LOADLIBRARY
#define LOADLIBRARY AfxLoadLibrary
#define FREELIBRARY AfxFreeLibrary
#else
#define LOADLIBRARY ::LoadLibrary
#define FREELIBRARY ::FreeLibrary
#endif

CMapStringToPtr CPluginLoader::m_RegisteredPlugins ;
CMapStringToPtr CPluginLoader::m_RegisteredDLLs ;


typedef void( __stdcall *TVDB400_PLUGIN_ENTRY )(LPVOID);


BOOL CPluginLoader::LoadPlugin( 
  LPVOID pGraphBuilder , LPCTSTR Plugin , FXSIZE iTimeout_ms )
{
  typedef LPCTSTR( __stdcall *TVDB400_PLUGIN_SWVERSION )();

  HINSTANCE hDll = NULL;
  __try
  {
    if ( iTimeout_ms )
    {
      Sleep( (DWORD) iTimeout_ms ) ;
      TRACE( "\nLoad %s" , Plugin ) ;
    }
    hDll = LOADLIBRARY( Plugin );
    if ( hDll && iTimeout_ms )
    {
      Sleep( (DWORD) iTimeout_ms ) ;
      TRACE( "\nhDLL=0x%x " , hDll ) ;
    }
  }
  __except ( 1 )
  {
    hDll = NULL;
  }
  if ( !hDll )
  {
    SENDWARN_1( _T( "Error: plugin DLL '%s' can't be loaded. Check dependencies!" ) , Plugin );
    m_RegisteredPlugins.SetAt( Plugin , (void*) (-1) ) ; // Problem with loading
    return FALSE;
  }
  TVDB400_PLUGIN_SWVERSION FnSWVersion = (TVDB400_PLUGIN_SWVERSION) GetProcAddress( (HMODULE) hDll , ENTRY_GETSWVERSION );
  LPCTSTR pVer ;
  if ( (FnSWVersion == NULL) || (strcmp( (pVer = FnSWVersion()) , STREAMHANDLER_VERSION ) != 0) )
  {
    if ( FnSWVersion != NULL ) 
      FREELIBRARY( hDll );
    SENDERR_1( _T( "Plugin \"%s\" is incompatible with current version of software" ) , Plugin );
    m_RegisteredPlugins.SetAt( Plugin , (void*)(-2) ) ; // incompatible version
    return FALSE;
  }
  if ( iTimeout_ms )
  {
    Sleep( (DWORD) iTimeout_ms ) ;
    TRACE( " Version=%s " , pVer) ;
  }

  TVDB400_PLUGIN_ENTRY FnReg = (TVDB400_PLUGIN_ENTRY) GetProcAddress( (HMODULE) hDll , ENTRY_PLUGINENTRY );
  BOOL bResult = FALSE;
  if ( FnReg )
  {
    bool success = true;
    __try
    {
      FnReg( pGraphBuilder );
      if ( iTimeout_ms )
      {
        Sleep( (DWORD) iTimeout_ms ) ;
      }
    }
    __except ( 1 )
    {
      success = false;
      FREELIBRARY( hDll );
      m_RegisteredPlugins.SetAt( Plugin , (void*) (-3) ) ; // not proper registration
    }
    if ( success )
    {
      m_Plugins.SetAt( Plugin , hDll );
      m_RegisteredDLLs.SetAt( Plugin , hDll ) ;
      m_RegisteredPlugins.SetAt( Plugin , (void*)1 ) ;
      TRACE( _T( "_PLUGIN_ \"%s\" loaded!\n" ) , Plugin );
      SENDINFO_1( _T( "Plugin \"%s\" loaded!" ) , Plugin );
      bResult = TRUE;
    }
    else
    {
      SENDERR_1( _T( "Plugin \"%s\" has not PluginEntry entry or is incompatible" ) , Plugin );
      m_RegisteredPlugins.SetAt( Plugin , (void*) (-4) ) ; // This is not plugin entry
    }
  }
  else
  {
    FREELIBRARY( hDll );
    SENDERR_1( _T( "Plugin \"%s\" has not PluginEntry entry" ) , Plugin );
    m_RegisteredPlugins.SetAt( Plugin , (void*) (-5) ) ; // Doesn't have plugin entry
  }
  return bResult;
}
#undef THIS_MODULENAME

#define THIS_MODULENAME _T("PluginLoader.UnloadPlugin")

BOOL CPluginLoader::UnloadPlugin( LPVOID pGraphBuilder , LPCTSTR Plugin )
{
  typedef void( __stdcall *TVDB400_PLUGIN_EXIT )(LPVOID);
  HINSTANCE hDll = NULL;
  if ( !m_Plugins.Lookup( Plugin , (void*&) hDll ) || !hDll )
  {
    SENDWARN_1( _T( "Plugin \"%s\" is not loaded" ) , Plugin );
    return TRUE;
  }

  TVDB400_PLUGIN_EXIT FnExit = (TVDB400_PLUGIN_EXIT) GetProcAddress( (HMODULE) hDll , ENTRY_PLUGINEXIT );
  if ( !FnExit )
  {
    SENDERR_1( _T( "Plugin \"%s\" has not PluginExit entry" ) , Plugin );
    return FALSE;
  }

  FnExit( pGraphBuilder );
  m_Plugins.RemoveKey( Plugin );
  FREELIBRARY( hDll );
  SENDINFO_1( _T( "Plugin \"%s\" unloaded" ) , Plugin );
  return TRUE;
}
#undef THIS_MODULENAME

#define THIS_MODULENAME _T("PluginLoader::FindPlugins")

BOOL CPluginLoader::FindPlugins( CStringArray& PluginNames , 
  LPVOID pGraphBuilder , LPCTSTR PluginFolder )
{
  PluginNames.RemoveAll();
  CString StartDir;
  StartDir = FxGetAppPath();
  if ( StartDir.IsEmpty() )
  {
    SENDERR_0( _T( "Can't detect sartup durectory" ) );
    return FALSE;
  }
  SENDINFO_1( _T( "Start up directory found: '%s'" ) , StartDir );
  CString strTempl = (StartDir + _T( '\\' ) + PluginFolder) + _T( "\\*.dll" );
  CFileFind ff;
  BOOL bFound = ff.FindFile( strTempl );
  int iNChecked = 0 ;
  while ( bFound )
  {
    bFound = ff.FindNextFile();
    if ( ff.IsDots() || ff.IsDirectory() )
      continue;
    CString PluginPath = ff.GetFilePath() ;
    iNChecked++ ;
    void * p = NULL ;
    BOOL bRes = m_RegisteredPlugins.Lookup( PluginPath , p ) ;
    if ( !bRes ) // NO such plugin, necessary to add to names
    {
      PluginNames.Add( PluginPath) ;
    }
    else
    {
      FXSIZE iNRefs = (FXSIZE) p ;
      if ( iNRefs > 0 ) // if this is plugin dll, increment NRefs
      {
        m_RegisteredPlugins.SetAt( PluginPath , (void*) (++iNRefs) ) ;
        bRes = m_RegisteredDLLs.Lookup( PluginPath , p ) ;
        HINSTANCE hDLL = (HINSTANCE) p ;
        m_Plugins.SetAt( PluginPath , hDLL ) ; // for local builder
        TVDB400_PLUGIN_ENTRY FnReg = (TVDB400_PLUGIN_ENTRY) GetProcAddress( (HMODULE) hDLL , ENTRY_PLUGINENTRY );
        BOOL bResult = FALSE;
        if ( FnReg )
        {
          bool success = true;
          try
          {
            FnReg( pGraphBuilder );
          }
          catch ( ... )
          {
            ASSERT( 0 ) ;
          }
        }
      }
//       m_RegisteredDLLs.SetAt( PluginPath , hDLL ) ; // for all builders in application
      // Otherwise nothing to do
    }
  }
  return (iNChecked != 0);
}

#undef THIS_MODULENAME

#define THIS_MODULENAME _T("PluginLoader.CheckPluginExist")
int CPluginLoader::CheckPluginExist( FXString& Plugin , LPCTSTR PluginFolder )
{
  FXString fileName = FxGetFileName( Plugin );
  FXString StartDir;
  if ( !FxGetAppPath( StartDir ) )
  {
    SENDERR_0( _T( "Can not get starting directory" ) );
    return PL_PLUGINFOLDERNOTFOUND;
  }
  FXString pluginName = (StartDir + PluginFolder) + _T( "\\" ) + fileName;
  CFileStatus fs;
  if ( CFile::GetStatus( pluginName , fs ) && !(fs.m_attribute & (CFile::volume | CFile::directory)) )
    return PL_SUCCESS;
  return PL_PLUGINNOTFOUND;
}
#undef THIS_MODULENAME

#define THIS_MODULENAME _T("PluginLoader.IncludePlugin")
int CPluginLoader::IncludePlugin( LPVOID pGraphBuilder , FXString& Plugin , LPCTSTR PluginFolder )
{
  if ( !LoadPlugin( pGraphBuilder , Plugin ) )
    return PL_NOPLUGINENTRY;
  FXString fileName = FxGetFileName( Plugin );
  FXString StartDir;
  if ( !FxGetAppPath( StartDir ) )
  {
    SENDERR_0( _T( "Can not get starting directory" ) );
    return PL_PLUGINFOLDERNOTFOUND;
  }
  FXString pluginName = (StartDir + PluginFolder) + _T( "\\" ) + fileName;
  if ( pluginName != (LPCTSTR) Plugin )
  {
    CFile::Rename( Plugin , pluginName );
    SENDINFO_2( _T( "Plugin \"%s\" moved to \"%s\" folder" ) , Plugin , PluginFolder );
  }
  return PL_SUCCESS;
}
#undef THIS_MODULENAME

#define THIS_MODULENAME _T("PluginLoader.~CPluginLoader")
CPluginLoader::~CPluginLoader()
{
  POSITION pos = m_Plugins.GetStartPosition();
  CString path;
  HINSTANCE hDll;
  int iPlugInCntr = 0 ;
  int iPlugInsQty = (int) m_Plugins.GetCount();
  bool bIsSkiped = false;
  while ( pos )
  {
    m_Plugins.GetNextAssoc( pos , path , (void*&) hDll );

    if ( hDll )
    {
      //if ( path.Find( "FRenderDll" , 0 ) >= 0 )
      //  bIsSkiped = true;
      //else
      void * p = NULL ;
      BOOL bRes = m_RegisteredPlugins.Lookup( path , p ) ;
      if ( bRes )
      {
        FXSIZE iNRefs = (FXSIZE) p ;
        if ( --iNRefs == 0 ) // Last reference, unload DLL
        {
          AfxFreeLibrary( hDll );
          TRACE( "_PLUGIN_ #%d of %d \"%s\" is%s unloaded!\n" , iPlugInCntr++ , iPlugInsQty , path , !bIsSkiped ? "" : " NOT" );
          m_RegisteredPlugins.RemoveKey( path ) ;
          m_RegisteredDLLs.RemoveKey( path ) ;
        }
        else  // save the rest references
          m_RegisteredPlugins.SetAt( path , (void*)iNRefs ) ; //

      }
      bIsSkiped = false;
    }
  }
  m_Plugins.RemoveAll();
}
#undef THIS_MODULENAME

#define THIS_MODULENAME _T("PluginLoader.RegisterPlugins")
void CPluginLoader::RegisterPlugins( LPVOID pGraphBuilder , LPCTSTR PluginFolder )
{
  CStringArray PluginNames;
  if ( !FindPlugins( PluginNames , pGraphBuilder , PluginFolder ) )
    return;
  size_t iNDLLs = PluginNames.GetSize() ;
  int iNLoaded = 0 ;
  while ( PluginNames.GetSize() )
  {
    const CString& PathName = PluginNames.GetAt( 0 ) ;
    FXSIZE iTimeout = ( PathName.Find( "ptgrey2_0" ) >= 0 ) ?
      100 : 0 ;
    iNLoaded += LoadPlugin( pGraphBuilder , PathName , iTimeout ) ;
    PluginNames.RemoveAt( 0 );
  }

  SENDINFO( "Found %d DLLs, Loaded %d DLLs" , iNDLLs , iNLoaded ) ;
}
#undef THIS_MODULENAME

