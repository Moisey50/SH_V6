//  $File : imgfiles.cpp - interface for loading different formats of image files
//  (C) Copyright The FileX Ltd 2002. 
//
//
//
//  Revision History (excluding minor changes for specific compilers)
//   13 May 02 First release version, all followed changes must be listed below  (Andrey Chernushich)
// 
//   26 Oct 23 Last editing: conversion to tiff format is added (Moisey Bernstein)
//   15 Feb 24 Last editing: conversion to Jpg format is added (Moisey Bernstein)


#include "stdafx.h"

#ifndef LINK_FILEFILTERS
#define LINK_FILEFILTERS
#endif

#include <files\fileerrormessages.h>
#include <video\stdcodec.h>
#include <afxtempl.h>
#include <files\imgfiles.h>
#include <files\FileFilter.h>
#include <tiffio.h>
#include <comdef.h>
#include <gdiplus.h>

#pragma comment( lib, "GdiPlus.lib" )
#ifndef LINK_FILEFILTERS
#else
#include "gdiplusimgloader.h"
#endif
#include <helpers\RegKeys.h>
#ifndef USE_FXFÑ
#include <files\futils.h>
#endif

using namespace Gdiplus ;

RGBQUAD GrayPalette[ 256 ] ;

RGBQUAD * FillGrayPalette( RGBQUAD * pPalette )
{
  for ( int i = 0 ; i < 256 ; i++ )
  {
    BYTE * p = ( BYTE* ) &pPalette[ i ] ;
    *( p++ ) = i ;
    *( p++ ) = i ;
    *( p++ ) = i ;
    *( p++ ) = 0 ;
  }
  return pPalette ;
}

RGBQUAD * pGrayPalette = FillGrayPalette( GrayPalette ) ;
static char BASED_CODE BasedInFTypeFilter[] = "Bitmap files (*.bmp)|*.bmp||";

CString      _InFTypeFilter( BasedInFTypeFilter );
CString      _OutFTypeFilter( BasedInFTypeFilter );
CString      _AllTypes( "*.bmp" );
CString      _FiltersString( BasedInFTypeFilter );
char         _AllTypesExtensions[ 2048 ] = "bmp\0\0";
BOOL         _FileFiltersLoaded = FALSE;
int          _InstanceCntr = 0;
CArray<CFileFilter* , CFileFilter*> _FileFilters;

#ifndef LINK_FILEFILTERS
#endif

void WriteLogV( const char *lpszFormat , va_list argList )
{
  CString tmpS;
  tmpS.FormatV( lpszFormat , argList );
  FxSendLogMsg( MSG_ERROR_LEVEL , "ImgFiles" , 0 , "%s" , ( LPCTSTR ) tmpS ) ;
  //SENDERR_0( tmpS );
}

void WriteLog( LPCTSTR lpszFormat , ... )
{
  va_list argList;
  va_start( argList , lpszFormat );
  WriteLogV( lpszFormat , argList );
  va_end( argList );
}



void AVIFilesSupported()
{
  _AllTypes += ";*.avi";
  _FiltersString.Insert( _FiltersString.GetLength() - 1 , "AVI files (*.avi)|*.avi|" );
  _InFTypeFilter.Format( "All supprted files|%s|%s" , ( LPCTSTR ) _AllTypes , ( LPCTSTR ) _FiltersString );
  _OutFTypeFilter.Format( "%s|%s" , ( LPCTSTR ) _AllTypes , ( LPCTSTR ) _FiltersString );
}

LPCTSTR GetInputFileFilter()
{
  return _InFTypeFilter;
}

LPCTSTR GetOutputFileFilter()
{
  return _OutFTypeFilter;
}

void _LoadFileFilter( LPCTSTR path2lib )
{
#ifndef LINK_FILEFILTERS
  CFileFilter* newOne = new CFileFilter( path2lib );
  if ( !newOne->IsLoaded() )
  {
    delete newOne;
    return;
  }
  _FileFilters.Add( newOne );
  CString filter = newOne->GetFilterString();
  _FiltersString = filter + _FiltersString;
  // separate just an extentions
  {
    LPCTSTR p1 = filter , p2;
    while ( ( p1 ) && ( *p1 ) )
    {
      p1 = strchr( p1 , '|' );
      if ( ( p1 ) && ( *p1 ) )
      {
        p2 = strchr( p1 + 1 , '|' );
        if ( p2 )
        {
          CString newType = filter.Mid( ( int ) ( p1 - filter ) + 1 , ( int ) ( p2 - p1 ) - 1 );
          _AllTypes += ';' + newType;
          CString ext = newType.Mid( 2 ); // suggest first char are *., skip them...
          {
            char* ate = _AllTypesExtensions;
            while ( ( *ate ) || ( *( ate + 1 ) ) ) ate++; // Seek to the end of the string
            ate++;
            memcpy( ate , ext , ext.GetLength() );
            ate += ext.GetLength(); *ate = 0; ate++; *ate = 0; ate++;
          }
          p1 = p2 + 1;
        }
      }
    }
  }
  _InFTypeFilter.Format( "All supprted files|%s|%s" , ( LPCTSTR ) _AllTypes , ( LPCTSTR ) _FiltersString );
  _OutFTypeFilter.Format( "%s" , ( LPCTSTR ) _FiltersString );
#else
  _FiltersString = _T( "TIFF files (*.tif)|*.tif|JPEG files (*.jpg)|*.jpg|PNG files (*.png)|*.png|" ) + _FiltersString;
#endif
}

void UseFileFilters()
{
  if ( !_FileFiltersLoaded )
  {
    CString tmpS;
    CString key; key.Format( "Software\\%s\\FILEFILTERS" , COMPANY_NAME );
    CRegKeys rg( HKEY_LOCAL_MACHINE , key , KEY_READ );
    while ( rg.GetNextKey( tmpS ) )
    {
      _LoadFileFilter( tmpS );
    }
    _FileFiltersLoaded = TRUE;
  }
  _InstanceCntr++;
}


void FileFiltersDone()
{
  _InstanceCntr--;
  if ( _InstanceCntr == 0 )
  {
#ifndef LINK_FILEFILTERS
    for ( int i = 0; i < _FileFilters.GetSize(); i++ )
    {
      delete _FileFilters.GetAt( i );
    }
    _FileFilters.RemoveAll();
#endif
    _InFTypeFilter = BasedInFTypeFilter;
    _AllTypes = "*.bmp";
    _FiltersString = BasedInFTypeFilter;
    _FileFiltersLoaded = FALSE;
    memcpy( _AllTypesExtensions , "bmp\0\0" , 5 );
  }
}


const char *GetAllFilesExtensions()
{
  return _AllTypesExtensions;
}

const char *GetFilesExtensions( int ItemSelected )
{
  static char retV[ 20 ];
  if ( ItemSelected == 1 )
    return _AllTypesExtensions;
  CString extRq = _get_mask_item( _AllTypesExtensions , ItemSelected - 1 );
  memset( retV , 0 , 20 );
  strcpy( retV , extRq );
  return retV;
}

inline LPBITMAPINFOHEADER i_loadDIB( LPCTSTR fName )
{
  CFile iF;
  CFileException ex;
  BITMAPFILEHEADER bmfh;
  BITMAPINFOHEADER* result = NULL;

  if ( !iF.Open( fName , CFile::modeRead | CFile::shareDenyWrite , &ex ) )
  {
    WriteLog( ERROR_CANTREADFILE );
    return( false );
  }

  FSIZE length = iF.GetLength() - sizeof( bmfh );
  if ( ( int ) length < 0 ) return false;

  iF.Read( &bmfh , sizeof( bmfh ) );
  do
  {
    if ( bmfh.bfType != BMAP_HEADER_MARKER )
    {
      iF.Close();
#ifndef LINK_FILEFILTERS
      if ( _FileFilters.GetSize() )
      {
        for ( int i = 0; i < _FileFilters.GetSize(); i++ )
        {
          CString Info = _FileFilters.GetAt( i )->GetInfo();
          if ( ( Info == "JPG files filter" ) && ( bmfh.bfType == JPEG_HEADER_MARKER ) )
            result = _FileFilters.GetAt( i )->Load( fName );
          else
            result = _FileFilters.GetAt( i )->Load( fName );
          if ( result == NULL )
            continue;
          return result;
        }
      }
#else
      result = LoadImg( fName );
      return result;
#endif
      return NULL;
    }
    result = ( BITMAPINFOHEADER* ) malloc( ( size_t ) length );
    iF.Read( result , ( DWORD ) length );
    iF.Close();
    result->biXPelsPerMeter = result->biYPelsPerMeter = 0;
    if ( result->biCompression == BI_RGB
      && result->biPlanes == 1 && result->biBitCount == 8
      && result->biClrUsed )
    {
      // Check gray color table
      DWORD i = 0 ;
      for ( ; i < result->biClrUsed ; i++ )
      {
        LPBYTE pC = ( LPBYTE ) ( ( ( RGBQUAD* ) ( result + 1 ) ) + i ) ;
        if ( ( *( pC++ ) != i ) || ( *( pC++ ) != i ) || ( *( pC++ ) != i ) )
          break ;
      }
      if ( i == result->biClrUsed ) // OK, color  table is grey, no colors
        result->biCompression = BI_Y8 ;
    }
    if ( result->biCompression == BI_Y8 || result->biCompression == BI_Y800 )
    {
      //Remove color table
      if ( result->biClrUsed )
      {
        int iClrTabSize = sizeof( RGBQUAD ) * result->biClrUsed ;
        memcpy( result + 1 , ( ( LPBYTE ) ( result + 1 ) ) + iClrTabSize ,
          ( int ) length - iClrTabSize ) ;
        result->biClrUsed = 0 ;
      }
      LPBYTE pImage = ( LPBYTE ) ( result + 1 ) ;
      int iWidth = result->biWidth , iHeight = result->biHeight ;
      BYTE Tmp[ 20000 ] ;
      if ( ( ( iWidth & 3 ) == 0 ) && ( ( iHeight & 3 ) == 0 ) )
      {
        // Image vertical flip for SH internal format
        for ( int iy = 0 ; iy < iHeight / 2 ; iy++ )
        {
          LPBYTE pLow = pImage + iy * iWidth ;
          LPBYTE pHigh = pImage + ( iHeight - iy - 1 ) * iWidth ;
          memcpy( Tmp , pLow , iWidth ) ;
          memcpy( pLow , pHigh , iWidth ) ;
          memcpy( pHigh , Tmp , iWidth ) ;
        }
      }
      else
      {
        int iNewWidth = iWidth ; // ( iWidth & ~0x3 ) ;
        int iNewHeight = iHeight ; // ( iHeight & ~0x3 ) ;

        BYTE * pTmpBuf = new BYTE[ result->biSizeImage ] ;
        memcpy( pTmpBuf , pImage , result->biSizeImage ) ;
        // Image vertical flip for SH internal format
        LPBYTE pLow = pTmpBuf ;
        LPBYTE pNewLow = pImage ;
        LPBYTE pHigh = pTmpBuf + ( iHeight - 1 ) * iWidth ;
        LPBYTE pNewHigh = pImage + ( iNewHeight - 1 ) * iNewWidth ;
        for ( int iy = 0 ; iy < iHeight / 2 ; iy++ )
        {
          memcpy( pNewLow , pHigh , iNewWidth ) ;
          memcpy( pNewHigh , pLow , iNewWidth ) ;
          pLow += iWidth ;
          pNewLow += iNewWidth ;
          pHigh -= iWidth ;
          pNewHigh -= iNewWidth ;
        }
        delete pTmpBuf ;
        result->biWidth = iNewWidth ;
        result->biHeight = iNewHeight ;
        result->biSizeImage = iNewWidth * iNewHeight ;

      }
      break ;
    }
  } while ( 0 );
  return result;
}

LPBITMAPINFOHEADER loadDIB( LPCTSTR fName , bool bDoYUV )
{
  LPBITMAPINFOHEADER lpbmih = i_loadDIB( fName );
  if ( lpbmih )
  {
    if ( lpbmih->biCompression == BI_Y16 )  // separate case of Y16
    {
#if (defined _DEBUG) && (defined _DETAIL_LOG)
      TRACE( "+++ stdcodec.cpp: Load - Y16 formats\n" );
#endif
      return lpbmih;
    }
    if ( !bDoYUV )
      return lpbmih ;
    if ( !makeYUV9( &lpbmih ) )
    {
      free( lpbmih ); lpbmih = NULL;
    }
  }
  return( lpbmih );
}

bool saveToFile( const LPCTSTR fname ,
  const LPBITMAPINFOHEADER pBmihSrc , const LPBYTE pDataSrc ,
  int imageSize , int bmOffset , bool withHeader )
{
  bool res = false;

  //YuriS_20180806 The more-secure version ('_sopen_s(...)') of the _open(...) can not be used
  //here because image data writed with shift of half-of-width.
  int fHandle = _open( fname , _O_CREAT | _O_WRONLY | _O_BINARY , _S_IREAD | _S_IWRITE );
  if ( fHandle == -1 )
  {
    FXString err;
    err.Format( "Open failed on the '%s' file" , fname );

    perror( err );
    return( res );
  }

  if ( imageSize < 0 )
    imageSize = pBmihSrc->biSizeImage;

  if ( bmOffset < 0 )
    bmOffset = pBmihSrc->biSize;

  BITMAPFILEHEADER bmfH = { BMAP_HEADER_MARKER , 0 , 0 , 0 , 0 };
  if ( withHeader )
  {
    bmfH.bfSize = sizeof( BITMAPFILEHEADER ) + sizeof( BITMAPINFOHEADER ) + imageSize; // pframe->lpBMIH->biSizeImage;
    bmfH.bfOffBits = ( DWORD )sizeof( BITMAPFILEHEADER ) + bmOffset; // pframe->lpBMIH->biSize;
  }
  if ( withHeader )
  {
    _write( fHandle , &bmfH , sizeof( BITMAPFILEHEADER ) );
    _write( fHandle , pBmihSrc , pBmihSrc->biSize );
  }
  _write( fHandle , pDataSrc , imageSize );
  res = true;

  _close( fHandle );
  return res;
}
CFileFilter* FindFilter( LPCTSTR Extention )
{
  CFileFilter* retVal;
  for ( int i = 0; i < _FileFilters.GetSize(); i++ )
  {
    retVal = _FileFilters.GetAt( i );
    CString Info = retVal->GetFilterString();
    if ( Info.Find( CString( "|*." ) + Extention + "|" ) >= 0 ) return retVal;
  }
  return NULL;
}

bool saveRAW( LPCTSTR fname , LPBITMAPINFOHEADER bmih ,
  LPBYTE pData , bool withHeader )
{
  if ( !bmih ) return false;

  LPBYTE src = ( pData ) ? pData : ( ( LPBYTE ) bmih ) + bmih->biSize;

  int SizeImage = 0;
  int bmOffset = bmih->biSize;

  if ( bmih->biSizeImage )
    SizeImage = bmih->biSizeImage;
  else
  {
    switch ( bmih->biBitCount )
    {
      case 8:
        SizeImage = bmih->biHeight*bmih->biWidth + sizeof( RGBQUAD )*bmih->biClrUsed;
        bmOffset += sizeof( RGBQUAD )*bmih->biClrUsed;
        break;
      case 16:
        SizeImage = bmih->biHeight*bmih->biWidth * 2;
        break;
      case 24:
        SizeImage = bmih->biHeight*bmih->biWidth * 3;
        break;
      case 32:
        SizeImage = bmih->biHeight*bmih->biWidth * 4;
        break;
    }
  }

  return saveToFile( fname , bmih , src , SizeImage , bmOffset , withHeader );
}

bool SaveImage( LPCTSTR fname , LPBITMAPINFOHEADER pframe , bool ShowDialog )
{
  BOOL ffManual = FALSE;
  if ( !_FileFiltersLoaded )
  {
    UseFileFilters();
    ffManual = TRUE;
  }
#ifdef USE_FXFÑ
  FXString Ext = FxGetFileExtension( FXString( fname ) );
#else
  CString Ext = GetFileExtension( fname );
#endif
  if ( Ext.CompareNoCase( "bmp" ) == 0 )
  {
    FileFiltersDone();
    return saveDIB( fname , pframe );
  }
  CFileFilter* Saver = FindFilter( Ext );
  if ( !Saver )
  {
    WriteLog( "Cant' save file with extention '%s'" , ( LPCTSTR ) Ext );
    return FALSE;
  }
  bool res = Saver->Save( pframe , fname , ShowDialog );
  if ( ffManual )
  {
    FileFiltersDone();
  }
  return res;
}

bool saveSH2BMP( LPCTSTR fname , LPBITMAPINFOHEADER lpbmih ,
  LPBYTE pData , bool withHeader )
{
  if ( !lpbmih )
    return false;
  double dStart = GetHRTickCount() ;


  LPBYTE src = ( pData ) ? pData : ( ( LPBYTE ) lpbmih ) + lpbmih->biSize;

  BITMAPINFOHEADER bmih ;
  memcpy_s( &bmih , sizeof( bmih ) , lpbmih , sizeof( bmih ) ) ;
  int SizeImage = 0;

  BITMAPFILEHEADER bmfH = { BMAP_HEADER_MARKER , 0 , 0 , 0 , 0 };
  int bmOffset = bmih.biSize + sizeof( BITMAPFILEHEADER ) ;
  if ( bmih.biSizeImage )
    SizeImage = bmih.biSizeImage;
  int iInterleave = bmih.biWidth ;
  bmih.biClrUsed = 0 ;
  bool bY8Format = ( bmih.biCompression == BI_Y800 || bmih.biCompression == BI_Y8
    || ( ( bmih.biCompression == BI_RGB ) && ( bmih.biBitCount == 8 ) ) ) ;
  switch ( bmih.biBitCount )
  {
    case 8:
      if ( bY8Format )
        bmih.biCompression = 0 ;
      bmih.biClrUsed = 256 ;
      if ( !SizeImage )
        bmih.biSizeImage = SizeImage = bmih.biHeight*bmih.biWidth ;
      break;
    case 16:
      if ( !SizeImage )
        bmih.biSizeImage = SizeImage = bmih.biHeight*bmih.biWidth * 2;
      iInterleave *= 2 ;
      break;
    case 24:
      if ( !SizeImage )
        bmih.biSizeImage = SizeImage = bmih.biHeight*bmih.biWidth * 3;
      iInterleave *= 3 ;
      break;
    case 32:
      if ( !SizeImage )
        bmih.biSizeImage = SizeImage = bmih.biHeight*bmih.biWidth * 4;
      iInterleave *= 3 ;
      break;
  }

  int iNWritePerRow = iInterleave ;
  int iPadding = ( !bY8Format ) ? ( ( ~iInterleave ) + 1 ) & 3 : 0 ;
  int iStringSize = iInterleave + iPadding ;
  int iImageSizeInFile = iStringSize * bmih.biHeight ;
  int iColorTableSize = sizeof( RGBQUAD ) * bmih.biClrUsed ;
  bmfH.bfOffBits = bmOffset + iColorTableSize ;
  bmfH.bfSize = bmfH.bfOffBits + iImageSizeInFile ;


  FILE * fHandle = NULL ;
  errno_t err = _tfopen_s( &fHandle , fname , _T( "wb" ) );
  if ( err != ERROR_SUCCESS )
  {
    FXString errs = FxLastErr2Mes( "'%s' file opening ERROR: " , "" ) ;

    FxSendLogMsg( MSG_ERROR_LEVEL , _T( "saveSH2BMP" ) , 0 , ( LPCTSTR ) errs , ( LPCTSTR ) fname ) ;
    return false ;
  }
  size_t iNWritten = 0 ;
  if ( withHeader )
  {
    iNWritten += fwrite( &bmfH , 1 , sizeof( BITMAPFILEHEADER ) , fHandle );
    ASSERT( iNWritten ) ;
    iNWritten += fwrite( &bmih , 1 , bmih.biSize , fHandle );
    if ( iColorTableSize && !bY8Format )
      iNWritten += fwrite( pGrayPalette , 1 , sizeof( RGBQUAD ) * bmih.biClrUsed , fHandle ) ;
  }
  BYTE PaddingBytes[] = { 0 , 0 , 0 , 0 } ;
  switch ( bmih.biBitCount )
  {
    case 24:
    case 32:
    {   // rows are in straight order (image has inverse order)
      for ( int i = 0 ; i < bmih.biHeight ; i++ )
      {
        iNWritten += fwrite( src + iNWritePerRow * i ,
          1 , iNWritePerRow , fHandle ) ;
        if ( iPadding )
          iNWritten += fwrite( PaddingBytes , 1 , iPadding , fHandle ) ;
      }
    }
    break ;
    case 8:
      if ( iColorTableSize )
        iNWritten += fwrite( pGrayPalette , 1 , sizeof( RGBQUAD ) * bmih.biClrUsed , fHandle ) ;
      for ( int i = bmih.biHeight - 1 ; i >= 0 ; i-- )
      {
        iNWritten += fwrite( src + iNWritePerRow * i ,
          1 , iNWritePerRow , fHandle ) ;
        if ( iPadding )
          iNWritten += fwrite( PaddingBytes , 1 , iPadding , fHandle ) ;
      }
      break ;
    case 16:
    {  // rows are in inverse order
      for ( int i = bmih.biHeight - 1 ; i >= 0 ; i-- )
      {
        iNWritten += fwrite( src + iNWritePerRow * i ,
          1 , iNWritePerRow , fHandle ) ;
        if ( iPadding )
          iNWritten += fwrite( PaddingBytes , 1 , iPadding , fHandle ) ;
      }
    }
    break ;
  }
  fclose( fHandle ) ;
  TRACE( "\n         %d bytes is written in %.1f ms into %s" ,
    ( int ) iNWritten , GetHRTickCount() , fname ) ;
  return ( iNWritten > bmfH.bfOffBits ) ;
}

bool saveSH2Tiff( LPCTSTR fname , LPBITMAPINFOHEADER lpbmih ,
  LPBYTE pData )
{
  if ( !lpbmih )
    return false;
  double dStart = GetHRTickCount() ;


  LPBYTE src = ( pData ) ? pData : ( ( LPBYTE ) lpbmih ) + lpbmih->biSize;

  BITMAPINFOHEADER bmih ;
  memcpy_s( &bmih , sizeof( bmih ) , lpbmih , sizeof( bmih ) ) ;
  int SizeImage = 0;

  BITMAPFILEHEADER bmfH = { BMAP_HEADER_MARKER , 0 , 0 , 0 , 0 };
  int bmOffset = bmih.biSize + sizeof( BITMAPFILEHEADER ) ;
  if ( bmih.biSizeImage )
    SizeImage = bmih.biSizeImage;
  int iInterleave = bmih.biWidth ;
  bmih.biClrUsed = 0 ;
  bool bY8Format = ( bmih.biCompression == BI_Y800 || bmih.biCompression == BI_Y8
    || ( ( bmih.biCompression == BI_RGB ) && ( bmih.biBitCount == 8 ) ) ) ;
  bool bMono = ( bmih.biCompression == BI_Y800 )
    || ( bmih.biCompression == BI_Y8 )
    || ( bmih.biCompression == BI_Y16 ) ;
  bool bRGB = ( bmih.biCompression == BI_RGB ) ;

  switch ( bmih.biBitCount )
  {
    case 8:
      if ( bY8Format )
        bmih.biCompression = 0 ;
      bmih.biClrUsed = 256 ;
      if ( !SizeImage )
        bmih.biSizeImage = SizeImage = bmih.biHeight*bmih.biWidth ;
      break;
    case 16:
      if ( !SizeImage )
        bmih.biSizeImage = SizeImage = bmih.biHeight*bmih.biWidth * 2;
      iInterleave *= 2 ;
      break;
    case 24:
      if ( !SizeImage )
        bmih.biSizeImage = SizeImage = bmih.biHeight*bmih.biWidth * 3;
      iInterleave *= 3 ;
      break;
    case 32:
      if ( !SizeImage )
        bmih.biSizeImage = SizeImage = bmih.biHeight*bmih.biWidth * 4;
      iInterleave *= 3 ;
      break;
  }

  TIFF * output_image = NULL ;
  // Open the TIFF file
  if ( ( output_image = TIFFOpen( fname , "w" ) ) == NULL )
  {
    FxSendLogMsg( MSG_ERROR_LEVEL , "saveSH2Tiff" , 0 ,
      "Can't open for writing the file %s " , fname ) ;
    return false ;
  }

  uint32 uPhotometric = bMono ? PHOTOMETRIC_MINISBLACK :
    bRGB ? PHOTOMETRIC_RGB : 40000 + GetVideoFormatEnum( bmih.biCompression ) ; // YUV and so on.
  TIFFSetField( output_image , TIFFTAG_IMAGEWIDTH , bmih.biWidth );
  TIFFSetField( output_image , TIFFTAG_IMAGELENGTH , bmih.biHeight );
  TIFFSetField( output_image , TIFFTAG_SAMPLESPERPIXEL , 1 );
  TIFFSetField( output_image , TIFFTAG_BITSPERSAMPLE , bmih.biBitCount );
  TIFFSetField( output_image , TIFFTAG_ROWSPERSTRIP , bmih.biHeight );
  TIFFSetField( output_image , TIFFTAG_ORIENTATION , ( int ) ORIENTATION_TOPLEFT );
  TIFFSetField( output_image , TIFFTAG_PLANARCONFIG , PLANARCONFIG_CONTIG );
  TIFFSetField( output_image , TIFFTAG_COMPRESSION , COMPRESSION_NONE );
  TIFFSetField( output_image , TIFFTAG_PHOTOMETRIC , uPhotometric );


  // Write the information to the file

  tsize_t image_s;
  if ( ( image_s = TIFFWriteEncodedStrip( output_image , 0 , src , SizeImage ) ) == -1 )
  {
    FxSendLogMsg( MSG_ERROR_LEVEL , "saveSH2Tiff" , 0 , "Can't write tif file %s " , fname ) ;
  }
  else
  {
    FxSendLogMsg( MSG_INFO_LEVEL , "saveSH2Tiff" , 0 , "File %s is written" , fname ) ;
  }

  TIFFWriteDirectory( output_image );
  TIFFClose( output_image );

  TRACE( "\n         %d bytes is written in %.1f ms into %s" ,
    ( int ) image_s , GetHRTickCount() , fname ) ;
  return ( image_s >= SizeImage ) ;
}

pTVFrame GetFromTiff( LPCTSTR pFileName )
{
  pTVFrame pTVF = NULL ;
  TIFF* tif = TIFFOpen( pFileName , "r" );
  if ( tif )
  {
    uint32 w , h;
    uint32 uBitsPerPixel , uCompression , uPhotometric ;

    TIFFGetField( tif , TIFFTAG_IMAGEWIDTH , &w );
    TIFFGetField( tif , TIFFTAG_IMAGELENGTH , &h );
    TIFFGetField( tif , TIFFTAG_BITSPERSAMPLE , &uBitsPerPixel );
    TIFFGetField( tif , TIFFTAG_COMPRESSION , &uCompression );
    TIFFGetField( tif , TIFFTAG_PHOTOMETRIC , &uPhotometric );
    uBitsPerPixel &= 0x0000ffff ;
    uCompression &= 0x0000ffff ;
    uPhotometric &= 0x0000ffff ;

    uint32 npixels = w * h ;
    uint32 uSize = ( npixels * uBitsPerPixel ) / 8 ;
    uint32 uSHCompression = 0xffffffff ;
    if ( uCompression == COMPRESSION_NONE )
    {
      switch ( uPhotometric )
      {
        case PHOTOMETRIC_MINISBLACK:
        {
          switch ( uBitsPerPixel )
          {
            case 16: uSHCompression = BI_Y16 ; break ;
            case 8:  uSHCompression = BI_Y8 ; break ;
            default: break ;
          }
        }
        break ;
        case PHOTOMETRIC_RGB: uSHCompression = BI_RGB ; break ;
        default:
        {
          if ( uPhotometric >= 40000 && ( uPhotometric < 41000 ) )
            uSHCompression = GetVideoFormatForEnum( uPhotometric - 40000 ) ;
        } ;
      }
    }
    if ( uSHCompression != 0xffffffff ) // we found something known
    {
      switch ( uSHCompression )
      {
        case BI_RGB: pTVF = makeNewRGBFrame( w , h ) ; break ;
        case BI_Y8:
        case BI_Y800: pTVF = makeNewY8Frame( w , h ) ; break ;
        case BI_Y16: pTVF = makeNewY16Frame( w , h ) ; break ;
        case BI_YUV9: pTVF = makeNewYUV9Frame( w , h ) ; break ;
        case BI_YUV12:
        case BI_NV12:
        {
          pTVF = makeNewYUV12Frame( w , h ) ;
          pTVF->lpBMIH->biCompression = uSHCompression ;
          break ;
        }
      }
      if ( pTVF )
      {
        tdata_t pRaster = malloc( uSize ) ;
        if ( pRaster )
        {
          if ( TIFFReadRawStrip( tif , 0 , pRaster , uSize ) )
          {
            memcpy( pTVF->lpBMIH + 1 , pRaster , uSize ) ;
          }
          else
          {
            freeTVFrame( pTVF ) ;
            pTVF = NULL ;
          }
          free( pRaster ) ;
        }
      }
    }
    TIFFClose( tif );
  }
  return pTVF ;
}

int GetEncoderClsid( const WCHAR* format , CLSID* pClsid )
{
  UINT  num = 0;          // number of image encoders
  UINT  size = 0;         // size of the image encoder array in bytes

  ImageCodecInfo* pImageCodecInfo = NULL;

  GetImageEncodersSize( &num , &size );
  if ( size == 0 )
    return -1;  // Failure

  pImageCodecInfo = ( ImageCodecInfo* ) ( malloc( size ) );
  if ( pImageCodecInfo == NULL )
    return -1;  // Failure

  GetImageEncoders( num , size , pImageCodecInfo );

  for ( UINT j = 0; j < num; ++j )
  {
    if ( wcscmp( pImageCodecInfo[ j ].MimeType , format ) == 0 )
    {
      *pClsid = pImageCodecInfo[ j ].Clsid;
      free( pImageCodecInfo );
      return j;  // Success
    }
  }

  free( pImageCodecInfo );
  return -1;  // Failure
}

// Output by standard for Windows encoder 
// enum StandardEncoder
// {
//   SE_BMP = 0 ,
//   SE_JPG ,
//   SE_PNG ,
//   SE_GIF ,
//   SE_TIFF
// };

bool saveSH2StandardEncoder( StandardEncoder OutFormat ,
  LPCTSTR fname , LPBITMAPINFOHEADER lpbmih , LPBYTE pData )
{
  if ( !lpbmih )
    return false;
  double dStart = GetHRTickCount() ;

  LPBYTE pSourceBytes = ( pData ) ? pData : ( ( LPBYTE ) lpbmih ) + lpbmih->biSize;

  DWORD dwSizeImage = 0 ;

  DWORD dwCompression = lpbmih->biCompression ;

  if ( ( dwCompression == BI_RGB ) && ( lpbmih->biBitCount == 8 ) )
    dwCompression = BI_Y800 ;

  INT iStride = 0 ;
  PixelFormat format = PixelFormatUndefined ;
  BYTE * pImageForSaving = NULL ;
  LPBITMAPINFOHEADER pLPBMIHConverted = NULL ;
  switch ( lpbmih->biCompression )
  {
  case BI_Y8:
  case BI_Y800:
      iStride = ( lpbmih->biWidth & 0xfffffffc ) ;
    dwSizeImage = iStride *
      ( lpbmih->biHeight & 0xfffffffc ) ;
    if ( lpbmih->biWidth & 0x3 ) // width should be multiple of 4
    {
      pImageForSaving = new BYTE[ dwSizeImage ] ;
      BYTE * pRow = pSourceBytes ;
      BYTE * pDst = pImageForSaving ;
      BYTE * pDstEnd = pDst + dwSizeImage ;
      while ( pDst < pDstEnd )
      {
        memcpy( pDst , pRow , iStride ) ;
        pDst += iStride ;
        pRow += lpbmih->biWidth ;
      }
    }
    format = PixelFormat8bppIndexed ;
    break;
  case BI_Y16:
    iStride = ( lpbmih->biWidth & 0xfffffffe ) * 2 ;
    dwSizeImage = iStride *
      ( lpbmih->biHeight & 0xfffffffc ) ;
    if ( lpbmih->biWidth & 0x1 ) // width in bytes should be multiple of 4
    {
      pImageForSaving = new BYTE[ dwSizeImage ] ;
      BYTE * pRow = pSourceBytes ;
      BYTE * pDst = pImageForSaving ;
      BYTE * pDstEnd = pDst + dwSizeImage ;
      while ( pDst < pDstEnd )
      {
        memcpy( pDst , pRow , iStride ) ;
        pDst += iStride ;
        pRow += lpbmih->biWidth * 2 ;
      }
    }
    format = PixelFormat16bppGrayScale ;
    break;
  case BI_YUV12:
    pLPBMIHConverted = yuv12rgb24( lpbmih , pSourceBytes ) ;
  case BI_YUV9:
    if ( !pLPBMIHConverted )
      pLPBMIHConverted = yuv9rgb24( lpbmih , pSourceBytes ) ;
    if ( pLPBMIHConverted )
    {
      lpbmih = pLPBMIHConverted ;
      pSourceBytes = ( ( LPBYTE ) lpbmih ) + lpbmih->biSize;
      BYTE * pUp = pSourceBytes ;
      iStride = lpbmih->biWidth * 3 ;
      BYTE * pDown = pUp + iStride * ( lpbmih->biHeight - 1 ) ;
      BYTE Tmp[ 20000 ] ;
      while ( pUp < pDown )
      {
        memcpy( Tmp , pUp , iStride ) ;
        memcpy( pUp , pDown , iStride ) ;
        memcpy( pDown , Tmp , iStride ) ;
        pUp += iStride ;
        pDown -= iStride ;
      }
    }
    else
      break ;
  case BI_RGB:
    iStride = ( lpbmih->biWidth & 0xfffffffc ) * 3 ;
    dwSizeImage = iStride * ( lpbmih->biHeight & 0xfffffffc ) ;
    if ( lpbmih->biWidth & 0x3 ) // width should be multiple of 4
    {
      pImageForSaving = new BYTE[ dwSizeImage ] ;
      BYTE * pRow = pSourceBytes ;
      BYTE * pDst = pImageForSaving ;
      BYTE * pDstEnd = pDst + dwSizeImage ;
      while ( pDst < pDstEnd )
      {
        memcpy( pDst , pRow , iStride ) ;
        pDst += iStride ;
        pRow += lpbmih->biWidth * 3 ;
      }
    }
    format = PixelFormat24bppRGB ;
    break;
  }

  bool bResult = false ;
  if ( (format != PixelFormatUndefined) && iStride )
  {

    LPCTSTR pEncoder = NULL ;
    switch( OutFormat )
    {
      case SE_BMP: pEncoder = "image/bmp" ; break ;
      case SE_JPG: pEncoder = "image/jpeg" ; break ;
      case SE_PNG: pEncoder = "image/png" ; break ;
      case SE_GIF: pEncoder = "image/gif" ; break ;
      case SE_TIFF: pEncoder = "image/tiff" ; break ;
    } ;

    if ( pEncoder )
    {
      _bstr_t ClsName = _variant_t( pEncoder );

        // Initialize GDI+.
      GdiplusStartupInput gdiplusStartupInput;
      ULONG_PTR gdiplusToken;
      GdiplusStartup( &gdiplusToken , &gdiplusStartupInput , NULL );

      CLSID pngClsid;
      Status Res = ( Gdiplus::Status )GetEncoderClsid( ClsName , &pngClsid );
      if ( Res < 0 )
      {
        FxSendLogMsg( MSG_ERROR_LEVEL , "saveSH2Jpg" , 0 ,
          "encoder '%s' is not installed for file %s" , pEncoder , fname ) ;
      }
      else
      {
        Bitmap BMPforJpg( lpbmih->biWidth , lpbmih->biHeight , iStride ,
          format , ( pImageForSaving ) ? pImageForSaving : pSourceBytes ) ;
        _bstr_t FileName = _variant_t( fname );
        Res = BMPforJpg.Save( FileName , &pngClsid , NULL );
        if ( Res >= 0 )
        {
          TRACE( "\n Image saved into file %s in %.1f ms" ,
            fname , GetHRTickCount() - dStart ) ;
          bResult = true ;
        }
      }
      GdiplusShutdown( gdiplusToken );
    }
  }
  if ( pImageForSaving )
    delete[] pImageForSaving ;

  return bResult ;
}

#ifndef LINK_FILEFILTERS

#endif
