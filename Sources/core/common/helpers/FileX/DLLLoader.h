#ifndef DLLLOADER_INC
#define DLLLOADER_INC

#include <afxtempl.h>

class CDLLLoader
{
private:
  CArray<HMODULE,HMODULE> m_hLoadedDLLs ;
  CString m_RegistryFolder ;
public:
      CDLLLoader(LPCTSTR pName = NULL)  ;
      ~CDLLLoader(void);
 void SetRegistryFolder( LPCTSTR pName ) { m_RegistryFolder = pName; } ;
};

#endif
