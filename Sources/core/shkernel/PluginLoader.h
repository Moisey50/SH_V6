// PluginLoader.h: interface for the CPluginLoader class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PLUGINLOADER_H__4832B7CD_2872_4BD3_9A96_72C848F61E3A__INCLUDED_)
#define AFX_PLUGINLOADER_H__4832B7CD_2872_4BD3_9A96_72C848F61E3A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <gadgets\shkernel.h>

__forceinline CString PluginNameToBackupName(CString& Plugin)
{
	CString BackupName;
	CTime Time = CTime::GetCurrentTime();
	BackupName.Format(_T("%04d_%02d_%02d__%02d_%02d_%02d_%s"), Time.GetYear(), Time.GetMonth(),
		Time.GetDay(), Time.GetHour(), Time.GetMinute(), Time.GetSecond(), Plugin);
	return BackupName;
}

__forceinline CString BackupNameToPluginName(CString& Backup, CTime* Time = NULL)
{
	CString PluginName;
	int year, month, day, hour, minute, second;
	_stscanf(Backup, _T("%04d_%02d_%02d__%02d_%02d_%02d_%s"), &year, &month, &day, &hour, &minute, &second,
		PluginName.GetBufferSetLength(MAX_PATH + 1));
	PluginName.ReleaseBuffer();
	if (Time)
		*Time = CTime(year, month, day, hour, minute, second);
	return PluginName;
}

class RegisteredPlugin
{
public:
  LPCTSTR  m_PluginPath ;
  int      m_iNRefs ;

  RegisteredPlugin( LPCTSTR  PluginPath , bool bIsPlugIn )
  {
    m_PluginPath = PluginPath ;
    m_iNRefs = (bIsPlugIn) ? 1 : -1 ;
  }

  int AddRef() { return ++m_iNRefs ; }
  int Release() { return --m_iNRefs; } // if zero - no users
  RegisteredPlugin& operator=( RegisteredPlugin& Orig )
  {
    m_PluginPath = Orig.m_PluginPath ;
    m_iNRefs = Orig.m_iNRefs ;
    return *this ;
  }
};

class CPluginLoader : public IPluginLoader
{
  static CMapStringToPtr         m_RegisteredPlugins ; // hold # references
  static CMapStringToPtr         m_RegisteredDLLs ;    // hold hINSTANCEs
         CMapStringToPtr         m_Plugins;            // Local instance
public:
	enum
	{
		PL_SUCCESS,
		PL_PLUGINFOLDERNOTFOUND,
		PL_PLUGINNOTFOUND,
		PL_NOPLUGINENTRY,
	};
	CPluginLoader() {};
	virtual ~CPluginLoader();
	virtual void _stdcall RegisterPlugins(LPVOID pGraphBuilder, LPCTSTR PluginFolder = _T("Plugins"));
	virtual int _stdcall CheckPluginExist(FXString& Plugin, LPCTSTR PluginFolder = _T("Plugins"));
	virtual int _stdcall IncludePlugin(LPVOID pGraphBuilder, FXString& Plugin, LPCTSTR PluginFolder = _T("Plugins"));
private:
	BOOL FindPlugins(CStringArray& PluginNames, LPVOID pGraphBuilder , 
    LPCTSTR PluginFolder = _T( "Plugins" ) );
	BOOL LoadPlugin(LPVOID pGraphBuilder, LPCTSTR Plugin , FXSIZE iTimeout_ms = 0 );
	BOOL UnloadPlugin(LPVOID pGraphBuilder, LPCTSTR Plugin);
};



#endif // !defined(AFX_PLUGINLOADER_H__4832B7CD_2872_4BD3_9A96_72C848F61E3A__INCLUDED_)