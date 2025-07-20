// PluginLoader.h: interface for the CPluginLoader class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PLUGINLOADER_H__4832B7CD_2872_4BD3_9A96_72C848F61E3A__INCLUDED_)
#define AFX_PLUGINLOADER_H__4832B7CD_2872_4BD3_9A96_72C848F61E3A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// #include <files\futils.h>
#include <Gadgets\gadbase.h>


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


class CPluginLoader : public IPluginLoader
{
	CMapStringToPtr m_Plugins;
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
	virtual int _stdcall CheckPluginExist(CString& Plugin, LPCTSTR PluginFolder = _T("Plugins"));
	virtual int _stdcall IncludePlugin(LPVOID pGraphBuilder, CString& Plugin, LPCTSTR PluginFolder = _T("Plugins"));
private:
	BOOL FindPlugins(CStringArray& PluginNames, LPCTSTR PluginFolder);
	BOOL LoadPlugin(LPVOID pGraphBuilder, LPCTSTR Plugin);
	BOOL UnloadPlugin(LPVOID pGraphBuilder, LPCTSTR Plugin);
};

__forceinline CPluginLoader::~CPluginLoader()
{
	POSITION pos = m_Plugins.GetStartPosition();
	CString path;
	HINSTANCE hDll;
	while (pos)
	{
		m_Plugins.GetNextAssoc(pos, path, (void*&)hDll);
		if (hDll)
		{
			AfxFreeLibrary(hDll);
			TRACE("_PLUGIN_ \"%s\" unloaded!\n", path);
		}
	}
	m_Plugins.RemoveAll();
}

__forceinline void CPluginLoader::RegisterPlugins(LPVOID pGraphBuilder, LPCTSTR PluginFolder)
{
	CStringArray PluginNames;
	if (!FindPlugins(PluginNames, PluginFolder))
		return;
	while (PluginNames.GetSize())
	{
		LoadPlugin(pGraphBuilder, PluginNames.GetAt(0));
		PluginNames.RemoveAt(0);
	}
}


#endif // !defined(AFX_PLUGINLOADER_H__4832B7CD_2872_4BD3_9A96_72C848F61E3A__INCLUDED_)