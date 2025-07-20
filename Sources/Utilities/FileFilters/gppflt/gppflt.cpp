// gppflt.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "gppflt.h"
#include <gdiplus.h>
#include <helpers\profile.h>
#include <comdef.h>
#include <video\TVFrame.h>
#include <files\futils.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma comment( lib, "GdiPlus.lib" )
#define DLL_EXPORT(TYPE) __declspec(dllexport) TYPE __stdcall

//
//	Note!
//
//		If this DLL is dynamically linked against the MFC
//		DLLs, any functions exported from this DLL which
//		call FXSIZEo MFC must have the AFX_MANAGE_STATE macro
//		added at the very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls FXSIZEo MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

/////////////////////////////////////////////////////////////////////////////
// CGppfltApp

BEGIN_MESSAGE_MAP(CGppfltApp, CWinApp)
	//{{AFX_MSG_MAP(CGppfltApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGppfltApp construction

CGppfltApp::CGppfltApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CGppfltApp object

CGppfltApp theApp;

#define DLL_EXPORT(TYPE) __declspec(dllexport) TYPE __stdcall

#pragma warning(disable :4996)

STDAPI DllRegisterServer()
{
	char Path[MAX_PATH + _MAX_FNAME];
	LPTSTR pntr;
	if (SearchPath(NULL, "gppflt.dll", NULL, MAX_PATH + _MAX_FNAME, Path, &pntr)!=0)
    {
	    char MainKey[MAX_PATH];
        strcpy(MainKey,"SOFTWARE\\"); strcat(MainKey, COMPANY_NAME); strcat(MainKey,"\\");
	    if (!WriteLMRegistryString(MainKey, "", "")) return SELFREG_E_CLASS;
	    strcat(MainKey, "FILEFILTERS\\");
	    if (!WriteLMRegistryString(MainKey, "", "")) return SELFREG_E_CLASS;
	    if (!WriteLMRegistryString(MainKey, "GDIPP", Path)) return SELFREG_E_CLASS;
	    return S_OK;
    }
    return SELFREG_E_TYPELIB;
}

STDAPI DllUnregisterServer()
{
	char MainKey[MAX_PATH];
	strcpy(MainKey,"SOFTWARE\\"); strcat(MainKey, COMPANY_NAME); strcat(MainKey,"\\");
	strcat(MainKey, "FILEFILTERS\\");
    return (RegistryKeyDelete(MainKey, "GDIPP"))?S_OK:SELFREG_E_TYPELIB;
}

DLL_EXPORT(const char*) GetInfo()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return "TIFF files filter";
}

DLL_EXPORT(const char*) GetFilterString()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return "TIFF files (*.tif)|*.tif|JPEG files (*.jpg)|*.jpg|PNG files (*.png)|*.png|";
}

DLL_EXPORT(bool) Save(LPBITMAPINFOHEADER pic, const char* fName, bool ShowDlg)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return theApp.Save(pic, fName);
}

DLL_EXPORT(DWORD) Load(LPBITMAPINFOHEADER* pic, const char* fName)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return theApp.Load(pic, fName);
}

DLL_EXPORT(void) FreeBMIH(LPBITMAPINFOHEADER* pic)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    if ((pic) && (*pic)) { free(*pic); *pic=NULL; };
}

inline LPBITMAPINFOHEADER HBITMAP2LPBMIH(HBITMAP hBmp, DWORD &dSize)
{
    BITMAP bmp;
    WORD    cClrBits;
    DWORD   ClrUsed=0, PaletteSize=0;
    dSize=0;
    if (!GetObject(hBmp, sizeof(BITMAP), (LPSTR)&bmp)) 
        return NULL;
    
    cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel); 
    if (cClrBits == 1) 
        cClrBits = 1; 
    else if (cClrBits <= 4) 
        cClrBits = 4; 
    else if (cClrBits <= 8) 
        cClrBits = 8; 
    else if (cClrBits <= 16) 
        cClrBits = 16; 
    else if (cClrBits <= 24) 
        cClrBits = 24; 
    else cClrBits = 32; 
    int iSize=(((bmp.bmWidth * cClrBits +31)&~31)/8)* bmp.bmHeight;
    if (cClrBits < 24) 
    {
        ClrUsed = (1<<cClrBits); 
        PaletteSize = ClrUsed * sizeof (RGBQUAD); 
    }
    LPBITMAPINFOHEADER lpBMIH= (LPBITMAPINFOHEADER)malloc(sizeof(BITMAPINFOHEADER)+PaletteSize+iSize);
    if (lpBMIH)
    {
        memset(lpBMIH,0,sizeof(BITMAPINFOHEADER));
        dSize=sizeof(BITMAPINFOHEADER)+PaletteSize+iSize;
        LPBYTE data=(LPBYTE)lpBMIH+sizeof(BITMAPINFOHEADER);
        lpBMIH->biSize=sizeof(BITMAPINFOHEADER);
        lpBMIH->biWidth = bmp.bmWidth; 
        lpBMIH->biHeight = bmp.bmHeight; 
        lpBMIH->biPlanes = bmp.bmPlanes; 
        lpBMIH->biBitCount = bmp.bmBitsPixel; 
        lpBMIH->biClrUsed = ClrUsed;
        lpBMIH->biCompression = BI_RGB;
        lpBMIH->biSizeImage=0;
        lpBMIH->biClrImportant = 0;
        memcpy(data,bmp.bmBits,PaletteSize+iSize);
    }
    return lpBMIH;
}

BOOL CGppfltApp::InitInstance()
{
    BOOL res=CWinApp::InitInstance();
    return res;
}

int CGppfltApp::ExitInstance()
{
    return CWinApp::ExitInstance();
}

using namespace Gdiplus;

DWORD CGppfltApp::Load(LPBITMAPINFOHEADER* pic, const char* fName)
{
    HBITMAP hbmReturn=NULL;
    *pic=NULL;
    DWORD dSize=0;
    GdiplusStartup(&m_gdiplusToken, &m_gdiplusStartupInput, NULL);
    {    
        _bstr_t fnName=_variant_t(fName);
        Bitmap gbm(fnName);
        Status sts=gbm.GetHBITMAP(Gdiplus::Color(), &hbmReturn);
        if (sts==Ok)
        {
            *pic=HBITMAP2LPBMIH(hbmReturn, dSize);
        }
        DeleteObject(hbmReturn);
    }
    GdiplusShutdown(m_gdiplusToken);
	return dSize;
}

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
   UINT  num = 0;          // number of image encoders
   UINT  size = 0;         // size of the image encoder array in bytes

   Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

   Gdiplus::GetImageEncodersSize(&num, &size);
   if(size == 0)
      return -1;  // Failure

   pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
   if(pImageCodecInfo == NULL)
      return -1;  // Failure

   Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

   for(UINT j = 0; j < num; ++j)
   {
      if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
      {
         *pClsid = pImageCodecInfo[j].Clsid;
         free(pImageCodecInfo);
         return j;  // Success
      }    
   }

   free(pImageCodecInfo);
   return -1;  // Failure
}

bool  CGppfltApp::Save(LPBITMAPINFOHEADER pic, const char* fName)
{
    bool res=true;
    Gdiplus::Status sts=GdiplusStartup(&m_gdiplusToken, &m_gdiplusStartupInput, NULL);
    if (sts!=Gdiplus::Ok) return false;
    {   
        CString ext=GetFileExtension(CString(fName));
        _bstr_t format;
        if (ext.CompareNoCase("jpg")==0)
            format=L"image/jpeg";
        else if (ext.CompareNoCase("gif")==0)
            format=L"image/gif";
        else if (ext.CompareNoCase("tif")==0)
            format=L"image/tiff";
        else if (ext.CompareNoCase("png")==0)
            format=L"image/png";
        else
            res=false;
        if (res)
        {
            _bstr_t fnName = _variant_t(fName);
            CLSID  encoderClsid;
            INT    result;
            result = GetEncoderClsid(format, &encoderClsid);
            if (result>=0)
            {
                Gdiplus::Bitmap gbm((BITMAPINFO *)pic,GetData(pic));
                sts=gbm.Save(fnName,&encoderClsid);
                res=(sts==Gdiplus::Ok);
            }
            else res=false;
        }
    }
    Gdiplus::GdiplusShutdown(m_gdiplusToken);
    return res;
}