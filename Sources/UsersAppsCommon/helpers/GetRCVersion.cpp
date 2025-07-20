// GetRCVersion.cpp: Implementation of class CGetRCVersion.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GetRCVersion.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Constructor/Destructor
//////////////////////////////////////////////////////////////////////

FXString CGetRCVersion::m_csFileVersion;
FXString CGetRCVersion::m_csProductVersion;

CGetRCVersion::CGetRCVersion()
{

}

CGetRCVersion::~CGetRCVersion()
{

}

/***********************************************************************************/
/*                                                                                 */
/* Class:   CGetRCVersion                                                          */
/* Method:  GetVersionInfo                                                         */
/*                                                                                 */
/* Parameters:                                                                     */
/* -----------                                                                     */
/*   HMODULE hLib                                                                  */
/*                Handle to the module that contains the resource (EXE or DLL)     */            
/*                A value of NULL specifies the current applications resources     */ 
/*                                                                                 */
/*   FXString csEntry                                                               */
/*                Specifies the name of the resource. For more information,        */
/*                see the Remarks section.                                         */
/*                                                                                 */
/* Return Values:                                                                  */
/* --------------                                                                  */
/* If the function succeeds, the return value is a string containing the value     */
/* of the specified resource.                                                      */ 
/* If the function fails, the returned string is empty. To get extended error      */
/* information, call GetLastError.                                                 */
/*                                                                                 */
/* Remarks:                                                                        */
/* --------                                                                        */
/* Since the Win32 API resource information is encoded in Unicode, this method     */
/* also strips the strings from Unicode.                                           */
/*                                                                                 */
/* The following valid values for csEntry, as specified by Microsoft are:          */
/*   CompanyName, FileDescription, FileVersion, InternalName, LegalCopyright,      */
/*   OriginalFilename, ProductName, ProductVersion, Comments, LegalTrademarks,     */
/*   PrivateBuild, SpecialBuild                                                    */
/*                                                                                 */
/* Opening the rc-file as "text" or with a text-editor allows you to add further   */
/* entries to your version information structure and it is retrievable using       */
/* this same method.                                                               */
/*                                                                                 */
/***********************************************************************************/

FXString CGetRCVersion::GetVersionInfo(HMODULE hLib, FXString csEntry)
{
  FXString csRet;

  if (hLib == NULL)
    hLib = AfxGetResourceHandle();
  
  HRSRC hVersion = FindResource( hLib, MAKEINTRESOURCE(VS_VERSION_INFO), RT_VERSION );
  if (hVersion != NULL)
  {
    HGLOBAL hGlobal = LoadResource( hLib, hVersion ); 
    if ( hGlobal != NULL)  
    {  
  
      LPVOID versionInfo  = LockResource(hGlobal);  
      if (versionInfo != NULL)
      {
        DWORD vLen,langD;
        BOOL retVal;    
    
        LPVOID retbuf=NULL;
    
        static char fileEntry[256];

        sprintf_s(fileEntry,"\\VarFileInfo\\Translation");
        retVal = VerQueryValue(versionInfo,fileEntry,&retbuf,(UINT *)&vLen);
        if (retVal && vLen==4) 
        {
          memcpy(&langD,retbuf,4);            
          sprintf_s(fileEntry, "\\StringFileInfo\\%02X%02X%02X%02X\\%s",
                  (langD & 0xff00)>>8,langD & 0xff,(langD & 0xff000000)>>24, 
                  (langD & 0xff0000)>>16, (LPCTSTR)csEntry);            
        }
        else 
          sprintf_s(fileEntry, "\\StringFileInfo\\%04X04B0\\%s",  GetUserDefaultLangID(), csEntry.GetString());

        if (VerQueryValue(versionInfo,fileEntry,&retbuf,(UINT *)&vLen)) 
          csRet = (char*)retbuf;
      }
    }

    UnlockResource( hGlobal );  
    FreeResource( hGlobal );  
  }

  return csRet;
}

/***********************************************************************************/
/*                                                                                 */
/* Class:   CGetRCVersion                                                          */
/* Method:  FormatVersion                                                          */
/*                                                                                 */
/* Parameters:                                                                     */
/* -----------                                                                     */
/*   FXString cs                                                                    */
/*                Specifies a version number such as "FileVersion" or              */
/*                "ProductVersion" in the format "m, n, o, p"                      */
/*                (e.g. "1, 2, 3, a")                                              */
/*                                                                                 */
/* Return Values:                                                                  */
/* --------------                                                                  */
/* If the function succeeds, the return value is a string containing the version   */
/* in the format "m.nop" (e.g. "1.23a")                                            */ 
/*                                                                                 */
/* If the function fails, the returned string is empty.                            */
/*                                                                                 */
/***********************************************************************************/
FXString CGetRCVersion::FormatVersion(FXString& csInfo)
{
  FXString csRet;
  FXString cs( csInfo ) ;
  if (!cs.IsEmpty())
  {
    cs.TrimRight();
    int iPos = cs.Find(',');
    if (iPos == -1)
      return "";
    cs.TrimLeft();
    cs.TrimRight();
    csRet.Format("%s.", cs.Left(iPos));

    while (1)
    {
      cs = cs.Mid(iPos + 1);
      cs.TrimLeft();
      iPos = cs.Find(',');
      if (iPos == -1)
      {
        csRet +=cs;
        break;
      }
      csRet += cs.Left(iPos);
    }
  }

  return csRet;
}

/***********************************************************************************/
/*                                                                                 */
/* Class:   CGetRCVersion                                                          */
/* Method:  GetFileVersionX                                                        */
/*                                                                                 */
/* Parameters:                                                                     */
/* -----------                                                                     */
/*                                                                                 */
/* Return Values:                                                                  */
/* --------------                                                                  */
/* If the function succeeds, the return value is a string containing the           */
/* "FileVersion" in the format "m.nop" (e.g. "1.23a")                              */ 
/*                                                                                 */
/* If the function fails, the returned string is empty.                            */
/*                                                                                 */
/***********************************************************************************/
FXString CGetRCVersion::GetFileVersionX()
{
  if (m_csFileVersion.IsEmpty())
  {
    FXString csVersion = FormatVersion(GetVersionInfo(NULL, "FileVersion"));
    m_csFileVersion.Format("Version %s (Build %s)", csVersion, GetVersionInfo(NULL, "SpecialBuild"));
  }

  return m_csFileVersion;
}

/***********************************************************************************/
/*                                                                                 */
/* Class:   CGetRCVersion                                                          */
/* Method:  GetFileVersionX                                                        */
/*                                                                                 */
/* Parameters:                                                                     */
/* -----------                                                                     */
/*                                                                                 */
/* Return Values:                                                                  */
/* --------------                                                                  */
/* If the function succeeds, the return value is a string containing the           */
/* "ProductVersion" in the format "m.nop" (e.g. "1.23a")                           */ 
/*                                                                                 */
/* If the function fails, the returned string is empty.                            */
/*                                                                                 */
/***********************************************************************************/
FXString CGetRCVersion::GetProductVersionX()
{
  if (m_csProductVersion.IsEmpty())
    m_csProductVersion = FormatVersion(GetVersionInfo(NULL, "ProductVersion"));

  return m_csProductVersion;
}