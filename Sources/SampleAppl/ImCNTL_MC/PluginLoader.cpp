#include "StdAfx.h"
#include "PluginLoader.h"
#include <Gadgets\gadbase.h>
#include <files\futils.h>


#define THIS_MODULENAME _T("PluginLoader.LoadPlugin")

/*__forceinline*/ BOOL CPluginLoader::LoadPlugin(LPVOID pGraphBuilder, LPCTSTR Plugin)
{
	typedef void (__stdcall *TVDB400_PLUGIN_ENTRY)(LPVOID);
	HINSTANCE hDll = AfxLoadLibrary(Plugin);
	if (!hDll)
	{
		SENDWARN_1(_T("Error: plugin DLL '%s' can't be loaded. Check dependencies!"),Plugin);
		return FALSE;
	}
	TVDB400_PLUGIN_ENTRY FnReg = (TVDB400_PLUGIN_ENTRY)GetProcAddress((HMODULE)hDll, "?PluginEntry@@YGXPAX@Z");
	BOOL bResult = FALSE;
	if (FnReg)
	{
		FnReg(pGraphBuilder);
		m_Plugins.SetAt(Plugin, hDll);
		TRACE(_T("_PLUGIN_ \"%s\" loaded!\n"), Plugin);
		SENDINFO_1(_T("Plugin \"%s\" loaded!"), Plugin);
		bResult = TRUE;
	}
	else
	{
		AfxFreeLibrary(hDll);
		SENDERR_1(_T("Plugin \"%s\" has not PluginEntry entry"), Plugin);
	}
	return bResult;
}
#undef THIS_MODULENAME

#define THIS_MODULENAME _T("PluginLoader.UnloadPlugin")

/*__forceinline*/ BOOL CPluginLoader::UnloadPlugin(LPVOID pGraphBuilder, LPCTSTR Plugin)
{
	typedef void (__stdcall *TVDB400_PLUGIN_EXIT)(LPVOID);
	HINSTANCE hDll = NULL;
	if (!m_Plugins.Lookup(Plugin, (void*&)hDll) || !hDll)
	{
		SENDWARN_1(_T("Plugin \"%s\" is not loaded"), Plugin);
		return TRUE;
	}

	TVDB400_PLUGIN_EXIT FnExit = (TVDB400_PLUGIN_EXIT)GetProcAddress((HMODULE)hDll, "?PluginExit@@YGXPAX@Z");
	if (!FnExit)
	{
		SENDERR_1(_T("Plugin \"%s\" has not PluginExit entry"), Plugin);
		return FALSE;
	}

	FnExit(pGraphBuilder);
	m_Plugins.RemoveKey(Plugin);
	AfxFreeLibrary(hDll);
	SENDINFO_1(_T("Plugin \"%s\" unloaded"), Plugin);
	return TRUE;
}
#undef THIS_MODULENAME

#define THIS_MODULENAME _T("PluginLoader::FindPlugins")
/*__forceinline*/ BOOL CPluginLoader::FindPlugins(CStringArray& PluginNames, LPCTSTR PluginFolder = _T("Plugins"))
{
	PluginNames.RemoveAll();
	CString StartDir;
	//if (!GetStartingDirectory(StartDir)) // this function fails in case runing through shortcats
    StartDir=GetStartDir();
    if (StartDir.IsEmpty())
    {
        SENDERR_0(_T("Can't detect sartup durectory"));
		return FALSE;
    }
    SENDINFO_1(_T("Start up directory found: '%s'"),StartDir);
	CString strTempl = (StartDir + PluginFolder) + _T("\\*.dll");
	CFileFind ff;
	BOOL bFound = ff.FindFile(strTempl);
	while (bFound)
	{
		bFound = ff.FindNextFile();
		if (ff.IsDots() || ff.IsDirectory())
			continue;
		PluginNames.Add(ff.GetFilePath());
	}
	return (PluginNames.GetSize());
}

#undef THIS_MODULENAME

#define THIS_MODULENAME _T("PluginLoader.CheckPluginExist")
int CPluginLoader::CheckPluginExist(CString& Plugin, LPCTSTR PluginFolder)
{
	CString fileName = GetFileName(Plugin);
	CString StartDir;
	if (!GetStartingDirectory(StartDir))
	{
		SENDERR_0(_T("Can not get starting directory"));
		return PL_PLUGINFOLDERNOTFOUND;
	}
	CString pluginName = (StartDir + PluginFolder) + _T("\\") + fileName;
	CFileStatus fs;
	if (CFile::GetStatus(pluginName, fs) && !(fs.m_attribute & (CFile::volume | CFile::directory)))
		return PL_SUCCESS;
	return PL_PLUGINNOTFOUND;
}
#undef THIS_MODULENAME

#define THIS_MODULENAME _T("PluginLoader.IncludePlugin")
int CPluginLoader::IncludePlugin(LPVOID pGraphBuilder, CString& Plugin, LPCTSTR PluginFolder)
{
	if (!LoadPlugin(pGraphBuilder, Plugin))
		return PL_NOPLUGINENTRY;
	CString fileName = GetFileName(Plugin);
	CString StartDir;
	if (!GetStartingDirectory(StartDir))
	{
		SENDERR_0(_T("Can not get starting directory"));
		return PL_PLUGINFOLDERNOTFOUND;
	}
	CString pluginName = (StartDir + PluginFolder) + _T("\\") + fileName;
	if (pluginName != Plugin)
	{
		CFile::Rename(Plugin, pluginName);
		SENDINFO_2(_T("Plugin \"%s\" moved to \"%s\" folder"), Plugin, PluginFolder);
	}
	return PL_SUCCESS;
}
#undef THIS_MODULENAME
