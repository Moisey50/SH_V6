#include "stdafx.h"
#include "DLLLoader.h"
#include <string.h>
#include <fxfc/FXRegistry.h>

CDLLLoader::CDLLLoader( LPCTSTR pName )
{

	if (pName) 
		SetRegistryFolder(pName) ; 
	FXRegistry Reg( (LPCTSTR)m_RegistryFolder ) ;

	// shifriss 04/08/10: the following is done to run the SysTabeDaemon.exe process in invisible mode. 
	// if not this process will run in visible mode while loading the dll but it is annoying. 
	int iNEXEs = Reg.GetRegiInt( "AppLoading" , "N Preloaded EXEs" , 0 ) ;
    int i;
	for ( i = 0 ; i < iNEXEs ; i++ )
	{
		CString RegItemName ;
		RegItemName.Format( "EXEName_%d" , i ) ;
		CString EXEName = Reg.GetRegiString( "AppLoading" , (LPCTSTR)RegItemName , "Empty" ) ;
		if ( !EXEName.IsEmpty()  &&  (EXEName != "Empty") )
		{
 			FXRegistry RegChp( "TheFileX\\SHStudio") ; 
 			CString CHPLocation = RegChp.GetRegiString("Paths", "ChpPath", "C:\\Program Files\\File X\\Stream Handler\\Chp.exe" ); 
 			CHPLocation.Insert(0,"\"");
			CHPLocation+="\" ";
			CHPLocation.Insert(0, "cmd /c ");                             
 			CHPLocation+=EXEName;  
			system(CHPLocation);     
		}
	}
	int iNDLLs = Reg.GetRegiInt( "AppLoading" , "N Preloaded DLLs" , 0 ) ;
	for ( i = 0 ; i < iNDLLs ; i++ )
	{
		CString RegItemName ;
		RegItemName.Format( "DLLName_%d" , i ) ;
		CString DLLName = Reg.GetRegiString( "AppLoading" , (LPCTSTR)RegItemName , "Empty" ) ;
		if ( !DLLName.IsEmpty()  &&  (DLLName != "Empty") )
		{
			HMODULE h = ::LoadLibrary( (LPCSTR)DLLName ) ;
			if ( h )
				m_hLoadedDLLs.Add( h ) ;
		}
	}

}

CDLLLoader::~CDLLLoader(void)
{
	for ( int i = 0 ; i < (m_hLoadedDLLs.GetUpperBound()-1) ; i++ )
	{
		::FreeLibrary( m_hLoadedDLLs[i] ) ;
	}
}
