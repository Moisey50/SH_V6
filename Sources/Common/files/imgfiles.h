#ifndef _IMGFILES_INC
#define _IMGFILES_INC

#include <gadgets/gadbase.h>
#include <video\tvframe.h>
#include <io.h>
#include <fcntl.h>
#include <share.h>
#include <sys/locking.h>
#include <imageproc\dibutils.h>
#include <sys/stat.h>
#include <imageproc\settings.h>

// Load and unload file format libraries...
FX_EXT_GADGET void FileFiltersDone();
FX_EXT_GADGET void UseFileFilters();
FX_EXT_GADGET void AVIFilesSupported();
// FX_EXT_GADGET void WriteLog( LPCTSTR error , ... );

inline void _get_mask_item_cnt(LPCTSTR masks, int& item)
{
    item=0;
    LPCTSTR pntr=masks;
    while( (*pntr!=0) && (*(pntr+1)!=0))
    {
        pntr=strchr(pntr,0); 
        pntr++;
        if (strlen(pntr)) item++; 
    }
}

inline LPCTSTR _get_mask_item(LPCTSTR masks, int item)
{
    LPCTSTR pntr=masks;
    int n=0;
    int cnt;
    _get_mask_item_cnt(masks,cnt);
    if (item>cnt) item=0;
    while( (*pntr!=0) && (*(pntr+1)!=0))
    {
        if ((strlen(pntr)) && (n==item)) return pntr;
        pntr=strchr(pntr,0); 
        pntr++;
        n++;
    }
    return pntr;
}


inline bool    _cmp_extentions(LPCTSTR fname, LPCTSTR masks, int masks_nmb)
{
    LPCTSTR ext=strrchr(fname,'.');
    if (ext==NULL) return false;
    ext++;
    if (strlen(ext)==0) return false;
    for (int i=0; i<=masks_nmb; i++)
    {
        if ((strnicmp(ext,_get_mask_item(masks,i),strlen(ext))==0) 
            && (strlen(ext)==strlen(_get_mask_item(masks,i))) ) return true;
    }
    return false;
}

FX_EXT_GADGET LPCTSTR        GetInputFileFilter();        // Return string for files filter for open dialog...
FX_EXT_GADGET LPCTSTR        GetOutputFileFilter();       // Return string for files filter for open dialog...
FX_EXT_GADGET LPCTSTR        GetAllFilesExtensions();     // Return list of all supproted extensions
FX_EXT_GADGET LPCTSTR        GetFilesExtensions(int ItemSelected); // return extenstion with position ItemSelected-1 
FX_EXT_GADGET LPBITMAPINFOHEADER loadDIB( LPCTSTR fName , bool bDoYUV = true );  // Load a image file into the memory

FX_EXT_GADGET bool SaveImage(LPCTSTR fname, LPBITMAPINFOHEADER pframe, bool ShowDialog=false);

FX_EXT_GADGET bool saveToFile( const LPCTSTR fname ,
  const LPBITMAPINFOHEADER pBmihSrc , const LPBYTE pDataSrc ,
  int imageSize = -1 , int bmOffset = -1 , bool withHeader = TRUE ) ;


FX_EXT_GADGET bool saveRAW( LPCTSTR fname , LPBITMAPINFOHEADER bmih ,
  LPBYTE pData = NULL , bool withHeader = TRUE );

FX_EXT_GADGET bool saveSH2BMP( LPCTSTR fname , LPBITMAPINFOHEADER bmih ,
  LPBYTE pData = NULL , bool withHeader = TRUE );

FX_EXT_GADGET bool saveSH2Tiff( LPCTSTR fname , LPBITMAPINFOHEADER lpbmih ,
  LPBYTE pData = NULL ) ;

enum StandardEncoder
{
  SE_BMP = 0 ,
  SE_JPG ,
  SE_PNG ,
  SE_GIF ,
  SE_TIFF
};

FX_EXT_GADGET bool saveSH2StandardEncoder( StandardEncoder OutFormat ,
  LPCTSTR fname , LPBITMAPINFOHEADER lpbmih , LPBYTE pData = NULL ) ;

FX_EXT_GADGET pTVFrame GetFromTiff( LPCTSTR pFileName ) ;

__forceinline bool saveNoHeaders( LPCTSTR fname , LPBITMAPINFOHEADER bmih , LPBYTE pData = NULL )
{
  return saveSH2BMP(fname, bmih, pData, false);
}

__forceinline bool saveDIB(const LPCTSTR fname, const pTVFrame pframe)
{  
   if(!pframe) return false;
   
   ASSERT(pframe->lpBMIH);
   ASSERT(pframe->lpBMIH->biCompression!=BI_RGB);

   LPBYTE src=(pframe->lpData)?pframe->lpData:(((LPBYTE)pframe->lpBMIH)+pframe->lpBMIH->biSize);

   return saveToFile(fname, pframe->lpBMIH, src);
}

__forceinline bool saveDIB(const LPCTSTR fname, LPBITMAPINFOHEADER bmih)
{
  return saveSH2BMP(fname, bmih);
}

#endif  //_IMGFILES_INC
