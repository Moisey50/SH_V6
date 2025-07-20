// Acquire.cpp: implementation of the CAcquire class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"

/////////////////////////////////////////////////////////////////
// Written on base of code:
// TWAIN source code:
// Copyright (C) '91-'92 TWAIN Working Group:
// Aldus, Caere, Eastman-Kodak, Logitech,
// Hewlett-Packard Corporations.
// All rights reserved.
/////////////////////////////////////////////////////////////////

#include "Acquire.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

#define MAXDIRSIZE 2048
#define DSMNAME "TWAIN_32.DLL"
#define VALID_HANDLE    32      // valid windows handle SB >= 32

inline float FIX32ToFloat( TW_FIX32 fix32 )
{
  float   floater = 0;

  floater = (float) fix32.Whole + (float) (fix32.Frac / 65536.0);
  return(floater);
}

BOOL FlipBitMap( LPBYTE hBM , TW_INT16 PixType )
{
  LPBITMAPINFO            pdib;
  BYTE*                   pDib;
  unsigned char TW_HUGE   *pbuffer;
  unsigned char TW_HUGE   *tempptr;
  unsigned char TW_HUGE   *tempptrsave;
  LONG                    Width;
  LONG                    Height;
  LONG                    Linelength;
  LONG                    indexH;
  DWORD                   Size;
  DWORD                   SizeImage;
  WORD                    BitCount;
  DWORD                   ClrUsed;
  DWORD                   offset;
  TW_UINT16               pixels;
  TW_INT32                items;
  TW_UINT32               i;
  BYTE                    SaveRed;
  BYTE                    SaveBlue;

  pDib = hBM;
  pdib = (LPBITMAPINFO) pDib;

  Width = pdib->bmiHeader.biWidth;
  Height = pdib->bmiHeader.biHeight;
  Size = pdib->bmiHeader.biSize;
  SizeImage = pdib->bmiHeader.biSizeImage;
  BitCount = pdib->bmiHeader.biBitCount;
  ClrUsed = pdib->bmiHeader.biClrUsed;

  if ( tempptrsave = (LPBYTE) malloc( SizeImage ) )
  {
    tempptr = tempptrsave;

    // calculate offset to start of the bitmap data
    offset = sizeof( BITMAPINFOHEADER );
    offset += pdib->bmiHeader.biClrUsed * sizeof( RGBQUAD );

    Linelength = (((Width*BitCount + 31) / 32) * 4);

    //Goto Last line in bitmap
    offset += (Linelength * (Height - 1));

  #ifdef WIN32
    pDib = pDib + offset - Linelength;
  #endif

    //For each line
    for ( indexH = 1; indexH < Height; indexH++ )
    {
    #ifdef WIN32
      memcpy( tempptr , pDib , Linelength );
      pDib -= (Linelength);
    #else
      MemoryRead( hBM , offset , tempptr , Linelength );
      offset -= (Linelength);
    #endif
      tempptr += Linelength;
    }

    // Copy temp over hBM
    pbuffer = (unsigned char TW_HUGE *) pdib;
    pbuffer += sizeof( BITMAPINFOHEADER );
    pbuffer += pdib->bmiHeader.biClrUsed * sizeof( RGBQUAD );

  #ifdef WIN32
    memcpy( pbuffer , tempptrsave , SizeImage );
  #else
    MemoryRead( temp , 0 , pbuffer , SizeImage );
  #endif

    //Flip RGB color table
    if ( PixType == TWPT_RGB )
    {
      pbuffer = (unsigned char TW_HUGE *)pdib;
      pbuffer += sizeof( BITMAPINFOHEADER );
      pbuffer += pdib->bmiHeader.biClrUsed * sizeof( RGBQUAD );

      pixels = (TW_UINT16) pdib->bmiHeader.biWidth;
      for ( items = 0; items < Height; items++ )
      {
        tempptr = pbuffer;
        for ( i = 0; i < pixels; i++ )
        {
          //Switch Red byte and Blue byte
          SaveRed = (BYTE) *tempptr;
          SaveBlue = (BYTE)*(tempptr + 2);
          (BYTE) *tempptr = SaveBlue;
          (BYTE)*(tempptr + 2) = SaveRed;
          //increment to next triplet
          tempptr += 3;
        }
        pbuffer += Linelength;
      }
    }

    //Free
    free( tempptrsave );
    return(TRUE);
  }
  else
    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
//    CAcquire class implementation
///////////////////////////////////////////////////////////////////////////////

// Protected members

TW_UINT16 CAcquire::CallDSMEntry( pTW_IDENTITY pApp , pTW_IDENTITY pSrc , 
  TW_UINT32 DG , TW_UINT16 DAT , TW_UINT16 MSG , TW_MEMREF pData )
{
  TW_UINT16 twRC = m_DSMEntry( pApp , pSrc , DG , DAT , MSG , pData );

  if ( (twRC != TWRC_SUCCESS) && (DAT != DAT_EVENT) )
  {
    VERIFY( m_DSMEntry( pApp , pSrc , DG_CONTROL , DAT_STATUS , MSG_GET ,
      (TW_MEMREF) &m_GlobalStatus ) == TWRC_SUCCESS );
    TRACE( "CallDSMEntry function: call failed with RC = %d, CC = %d.\n" ,
      twRC , m_GlobalStatus.ConditionCode );
  }
  return twRC;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAcquire::CAcquire()
{
  m_ErrorMessage = "";
  m_CallBack = NULL;
  m_CallBackParam = NULL;
  m_DSMDLL = NULL;
  m_DSMEntry = NULL;
  m_TWDSMOpen = FALSE;
  m_TWDSOpen = FALSE;
  m_AcquireNow = FALSE;
  m_ShowInterface = TRUE;
  m_CaptureMode = Native;
  memset( &m_GlobalStatus , 0 , sizeof( TW_STATUS ) );
  memset( &m_appID , 0 , sizeof( TW_IDENTITY ) );
  memset( &m_dsID , 0 , sizeof( TW_IDENTITY ) );

  m_appID.Id = 0; 				// init to 0, but Source Manager will assign real value
  m_appID.Version.MajorNum = 1;
  m_appID.Version.MinorNum = 0;
  m_appID.Version.Language = TWLG_USA;
  m_appID.Version.Country = TWCY_USA;

  strcpy( m_appID.Version.Info , "TWAIN_32 Application 1.0.0.0  2000" );
  strcpy( m_appID.ProductName , AfxGetApp()->m_pszAppName );

  m_appID.ProtocolMajor = 1;//TWON_PROTOCOLMAJOR;
  m_appID.ProtocolMinor = 7;//TWON_PROTOCOLMINOR;
  m_appID.SupportedGroups = DG_IMAGE | DG_CONTROL;
  strcpy( m_appID.Manufacturer , COMPANY_NAME );
  strcpy( m_appID.ProductFamily , "CAcquire class" );
}

CAcquire::~CAcquire()
{}

BEGIN_MESSAGE_MAP( CAcquire , CWnd )
  //{{AFX_MSG_MAP(CAcquire)
  ON_WM_DESTROY()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CAcquire::Create( CWnd* Parent , TW_DataRdy* pCallBack , LPVOID pParam )
{
  if ( m_hWnd ) return(FALSE);
  m_CallBack = pCallBack;
  m_CallBackParam = pParam;
  LPCTSTR lpszCtrlClass = AfxRegisterWndClass( CS_PARENTDC , NULL , NULL );
  if ( !CWnd::CreateEx( 0 , lpszCtrlClass , "TWAINCTRL" , WS_CHILD , 1 , 1 , 0 , 0 , Parent->m_hWnd , NULL , NULL ) )
  {
    return(FALSE);
  }

  // Get path to TWAIN.DLL (Just 32-bit release)
  char* winDir = m_DSMName.GetBuffer( MAXDIRSIZE + 1 );
  GetWindowsDirectory( winDir , MAXDIRSIZE );
  if ( winDir[ strlen( winDir ) - 1 ] != '\\' ) strcat( winDir , "\\" );
  m_DSMName.ReleaseBuffer();
  m_DSMName += DSMNAME;
  return(TRUE);
}

BOOL CAcquire::OpenSourceManager()
{
  OFSTRUCT      ofStruct;
  TW_UINT16     twRC = TWRC_FAILURE;

  // Now let's try to load this library
  if ( m_TWDSMOpen )
  {
    m_ErrorMessage = "Source manager already open";
    return(FALSE);
  }

  memset( &ofStruct , 0 , sizeof( OFSTRUCT ) );

  if ( (OpenFile( m_DSMName , &ofStruct , OF_EXIST ) != -1) &&
    (m_DSMDLL = LoadLibrary( m_DSMName )) != NULL &&
    (m_DSMDLL >= (HANDLE) VALID_HANDLE) &&
    (m_DSMEntry = (DSMENTRYPROC) GetProcAddress( m_DSMDLL , MAKEINTRESOURCE( 1 ) )) != NULL )
  {
    // This call performs four important functions:
    //  	- opens/loads the SM
    //    	- passes the handle to the app's window to the SM
    //    	- returns the SM assigned appID.id field
    //    	- be sure to test the return code for SUCCESSful open of SM

    twRC = CallDSMEntry( &m_appID , NULL , DG_CONTROL , DAT_PARENT , MSG_OPENDSM , (TW_MEMREF) &m_hWnd );
    switch ( twRC )
    {
      case TWRC_SUCCESS:
        // Needed for house keeping.  Do single open and do not
        // close SM which is not already open ....
        m_TWDSMOpen = TRUE;
        break;
      default:
        // Trouble opening the SM, inform the user
        m_TWDSMOpen = FALSE;
        m_ErrorMessage = "OpenDSM failure\r\n";
        break;
    }
  }
  else
  {
    m_DSMEntry = NULL;
    if ( m_DSMDLL )
    {
      FreeLibrary( m_DSMDLL ); m_DSMDLL = NULL;
    };
    m_ErrorMessage = "Can't start TWAIN32.DLL";
    return(FALSE);
  }
  return(TRUE);
}


void CAcquire::CloseSourceManager()
{
  if ( m_TWDSOpen )
  {
    TW_UINT16     twRC = TWRC_FAILURE;

    twRC = CallDSMEntry( &m_appID , NULL , DG_CONTROL , DAT_IDENTITY , MSG_CLOSEDS , &m_dsID );
    if ( twRC == TWRC_SUCCESS )
    {
      m_TWDSOpen = FALSE;
      memset( &m_dsID , 0 , sizeof( m_dsID ) );
    }
  }
  if ( m_TWDSMOpen )
  {
    TW_UINT16     twRC = TWRC_FAILURE;

    twRC = CallDSMEntry( &m_appID , NULL , DG_CONTROL , DAT_PARENT , MSG_CLOSEDSM , &m_hWnd );
    if ( twRC == TWRC_SUCCESS )
    {
      m_TWDSMOpen = FALSE;
    }
  }
  m_DSMEntry = NULL;
  if ( m_DSMDLL )
  {
    FreeLibrary( m_DSMDLL ); m_DSMDLL = NULL;
  };
}


void CAcquire::OnDestroy()
{
  CloseSourceManager();
  CWnd::OnDestroy();
}


void CAcquire::SelectSource()
{
  TW_UINT16 twRC = TWRC_FAILURE;
  TW_IDENTITY NewDSIdentity;

  m_ErrorMessage = "";
  if ( OpenSourceManager() )
  {
    if ( IsSourceOpen() )
    {
      m_ErrorMessage = "Source already open";
      return;
    }
    // I will settle for the system default.  Shouldn't I get a highlight
  // on system default without this call?
    twRC = CallDSMEntry( &m_appID , NULL , DG_CONTROL , DAT_IDENTITY , MSG_GETDEFAULT , (TW_MEMREF) &NewDSIdentity );
    // This call performs one important function:
    // - should cause SM to put up dialog box of available Source's
    // - tells the SM which application, appID.id, is requesting, REQUIRED
    // - returns the SM assigned NewDSIdentity.id field, you check if changed
    //  (needed to talk to a particular Data Source)
    // - be sure to test return code, failure indicates SM did not close !!
    twRC = CallDSMEntry( &m_appID , NULL , DG_CONTROL , DAT_IDENTITY , MSG_USERSELECT , (TW_MEMREF) &NewDSIdentity );
    switch ( twRC )
    {
      case TWRC_SUCCESS:
        m_dsID = NewDSIdentity;
        break;
      case TWRC_CANCEL:
        break;
      default:
        m_ErrorMessage = "Can't execute SelectSource function";
        break;
    }
    CloseSourceManager();
  }
  else m_ErrorMessage = "Can't load source manager";
}

BOOL CAcquire::DoIt()
{
  TW_CAPABILITY   cap;
  pTW_ONEVALUE    pval = NULL;
  TW_INT16        status = TWRC_FAILURE;
  TW_USERINTERFACE  twUI;

  m_ErrorMessage = "";

  if ( OpenSourceManager() )
  {
    if ( !IsSourceManagerReady() )  return(FALSE);

    if ( !IsSourceOpen() )
    {
      memset( &m_dsID , 0 , sizeof( m_dsID ) );
      status = CallDSMEntry( &m_appID , NULL , DG_CONTROL , DAT_IDENTITY , MSG_OPENDS , &m_dsID );
      switch ( status )
      {
        case TWRC_SUCCESS:
          // do not change flag unless we successfully open
          m_TWDSOpen = TRUE;
          break;
        default:
          break;
      }
    }
    if ( !IsSourceOpen() )
    {
      m_ErrorMessage = "Can't open TWAIN source";
      return(FALSE);
    }

    status = TWRC_FAILURE;
    memset( &cap , 0 , sizeof( TW_CAPABILITY ) );
    cap.Cap = ICAP_XFERMECH;
    cap.ConType = TWON_ONEVALUE;

    if ( cap.hContainer = GlobalAlloc( GHND , sizeof( TW_ONEVALUE ) ) )
    {
      pval = (pTW_ONEVALUE) GlobalLock( cap.hContainer );
      pval->ItemType = TWTY_UINT16;

      pval->Item = TWSX_MEMORY;

      GlobalUnlock( cap.hContainer );

      status = CallDSMEntry( &m_appID , &m_dsID , DG_CONTROL , DAT_CAPABILITY , MSG_SET , (TW_MEMREF) &cap );

      GlobalFree( (HANDLE) cap.hContainer );
      if ( status != TWRC_SUCCESS )
      {
        m_ErrorMessage = "Capabilty information retriving problem.";
        CloseSourceManager();
        return(FALSE);
      }
    }
    else
    {
      m_ErrorMessage = "Memory allocation problem";
      CloseSourceManager();
      return(FALSE);
    }
    if ( !m_AcquireNow )
    {
      memset( &twUI , 0 , sizeof( TW_USERINTERFACE ) );
      twUI.hParent = m_hWnd;
      twUI.ShowUI = m_ShowInterface;
      status = CallDSMEntry( &m_appID , &m_dsID , DG_CONTROL , DAT_USERINTERFACE , MSG_ENABLEDS , (TW_MEMREF) &twUI );
      if ( status != TWRC_SUCCESS )
      {
        m_ErrorMessage = "Can't set user interface.";
        CloseSourceManager();
        return(FALSE);
      }
      m_AcquireNow = TRUE;
    }
    else
    {
      m_ErrorMessage = "TWAIN source already open.";
    }
  }
  return(status == TWRC_SUCCESS);
}


BOOL CAcquire::PreTranslateMessage( MSG* pMsg )
{
  if ( IsSourceManagerReady() && IsSourceOpen() )
  {
    TW_UINT16  twRC = TWRC_NOTDSEVENT;
    TW_EVENT   twEvent;

    memset( &twEvent , 0 , sizeof( TW_EVENT ) );
    twEvent.pEvent = (TW_MEMREF) pMsg;

    twRC = CallDSMEntry( &m_appID , &m_dsID , DG_CONTROL , DAT_EVENT , MSG_PROCESSEVENT , (TW_MEMREF) &twEvent );
    switch ( twEvent.TWMessage )
    {
      case MSG_XFERREADY:
        if ( !m_AcquireNow ) break;
        switch ( m_CaptureMode )
        {
          case Native:
            DoNativeTransfer();
            break;
          case Memory:
            DoMemTransfer();
            break;
          case File:
            break;
        }
      case MSG_CLOSEDSREQ:
      case MSG_CLOSEDSOK:
      {
        if ( !m_AcquireNow ) break;

        TW_UINT16 twRC = TWRC_FAILURE;
        TW_USERINTERFACE twUI;

        memset( &twUI , 0 , sizeof( TW_USERINTERFACE ) );
        twUI.hParent = this->m_hWnd;
        twUI.ShowUI = TWON_DONTCARE8;

        twRC = CallDSMEntry( &m_appID , &m_dsID , DG_CONTROL , DAT_USERINTERFACE , MSG_DISABLEDS , (TW_MEMREF) &twUI );
        m_AcquireNow = FALSE;
        if ( m_TWDSOpen == TRUE )
        {
          twRC = CallDSMEntry( &m_appID , NULL , DG_CONTROL , DAT_IDENTITY , MSG_CLOSEDS , &m_dsID );
          m_TWDSOpen = FALSE;
        }
        CloseSourceManager();
        break;
      }
      default:
        break;
    }
  }
  return CWnd::PreTranslateMessage( pMsg );
}

BOOL CAcquire::DoMemTransfer()
{
  TW_UINT16 twRC = TWRC_FAILURE;
  TW_IMAGEINFO        info;
  TW_SETUPMEMXFER     setup;
  TW_CAPABILITY       cap;
  TW_IMAGEMEMXFER     xfer;

  if ( !m_CallBack ) return(FALSE);

  memset( &setup , 0 , sizeof( TW_SETUPMEMXFER ) );
  memset( &info , 0 , sizeof( TW_IMAGEINFO ) );

  twRC = CallDSMEntry( &m_appID , &m_dsID , DG_IMAGE , DAT_IMAGEINFO , MSG_GET , (TW_MEMREF) &info );
  if ( twRC != TWRC_SUCCESS )
  {
    m_ErrorMessage = "Can't retrive image info";
    return(FALSE);
  }
  twRC = CallDSMEntry( &m_appID , &m_dsID , DG_CONTROL , DAT_SETUPMEMXFER , MSG_GET , (TW_MEMREF) &setup );
  if ( twRC != TWRC_SUCCESS )
  {
    m_ErrorMessage = "Can't retrive setup info";
    return(FALSE);
  }
  TW_UINT32 size = (((((TW_INT32) info.ImageWidth*info.BitsPerPixel + 31) / 32) * 4)*info.ImageLength);
  int blocks = (int) (size / setup.Preferred);
  size = (blocks + 1) * setup.Preferred;

  cap.Cap = ICAP_UNITS;
  cap.ConType = TWON_DONTCARE16;
  cap.hContainer = NULL;
  twRC = CallDSMEntry( &m_appID , &m_dsID , DG_CONTROL , DAT_CAPABILITY , MSG_GETCURRENT , (TW_MEMREF) &cap );
  if ( twRC != TWRC_SUCCESS )
  {
    m_ErrorMessage = "Can't retrive capability info";
    return(FALSE);
  }
  pTW_ONEVALUE pOneV = (pTW_ONEVALUE) GlobalLock( cap.hContainer );
  TW_UINT16 Units = (TW_UINT16) (pOneV->Item);
  GlobalUnlock( cap.hContainer );
  GlobalFree( (HANDLE) cap.hContainer );

  float XRes = FIX32ToFloat( info.XResolution );
  float YRes = FIX32ToFloat( info.YResolution );


  BITMAPINFOHEADER* pdib = (BITMAPINFOHEADER*) malloc( sizeof( BITMAPINFOHEADER ) + size + 256 * sizeof( RGBQUAD ) );

  pdib->biSize = sizeof( BITMAPINFOHEADER );
  pdib->biWidth = info.ImageWidth;
  pdib->biHeight = info.ImageLength;

  pdib->biPlanes = 1;
  pdib->biBitCount = info.BitsPerPixel;

  pdib->biCompression = BI_RGB;
  pdib->biSizeImage = size;

  switch ( Units )
  {
    case TWUN_INCHES:
      pdib->biXPelsPerMeter = (LONG) ((XRes / 2.54) * 100);
      pdib->biYPelsPerMeter = (LONG) ((YRes / 2.54) * 100);
      break;
    case TWUN_CENTIMETERS:
      pdib->biXPelsPerMeter = (LONG) (XRes * 100);
      pdib->biYPelsPerMeter = (LONG) (YRes * 100);
      break;
    case TWUN_PICAS:
    case TWUN_POINTS:
    case TWUN_TWIPS:
    case TWUN_PIXELS:
    default:
      pdib->biXPelsPerMeter = 0;
      pdib->biYPelsPerMeter = 0;
      break;
  }

  RGBQUAD *rgb = (RGBQUAD *) (((LPBYTE) pdib) + sizeof( BITMAPINFOHEADER ));
  TW_UINT16 PixelFlavor;
  TW_UINT16 index;
  TW_PALETTE8 pal;

  switch ( info.PixelType )
  {
    case TWPT_BW:
      pdib->biClrUsed = 2;
      pdib->biClrImportant = 0;

      cap.Cap = ICAP_PIXELFLAVOR;
      cap.ConType = TWON_DONTCARE16;
      cap.hContainer = NULL;

      twRC = CallDSMEntry( &m_appID , &m_dsID , DG_CONTROL , DAT_CAPABILITY , MSG_GETCURRENT , (TW_MEMREF) &cap );
      if ( twRC != TWRC_SUCCESS )
      {
        free( pdib );
        m_ErrorMessage = "Can't retrive capability info";
        return(FALSE);

      }
      if ( cap.ConType != TWON_ONEVALUE )
      {
        PixelFlavor = TWPF_CHOCOLATE;
      }
      else
      {
        pOneV = (pTW_ONEVALUE) GlobalLock( cap.hContainer );
        PixelFlavor = (TW_UINT16) (pOneV->Item);
        GlobalUnlock( cap.hContainer );
      }
      GlobalFree( (HANDLE) cap.hContainer );
      if ( PixelFlavor == 0 )
      {
        rgb[ 0 ].rgbGreen = 0x0000;
        rgb[ 0 ].rgbBlue = 0x0000;
        rgb[ 0 ].rgbReserved = 0;

        rgb[ 1 ].rgbRed = 0x00FF;
        rgb[ 1 ].rgbGreen = 0x00FF;
        rgb[ 1 ].rgbBlue = 0x00FF;
        rgb[ 1 ].rgbReserved = 0;
      }
      else
      {
        rgb[ 0 ].rgbRed = 0x0000;
        rgb[ 0 ].rgbRed = 0x00FF;
        rgb[ 0 ].rgbGreen = 0x00FF;
        rgb[ 0 ].rgbBlue = 0x00FF;
        rgb[ 0 ].rgbReserved = 0;

        rgb[ 1 ].rgbRed = 0x0000;
        rgb[ 1 ].rgbGreen = 0x0000;
        rgb[ 1 ].rgbBlue = 0x0000;
        rgb[ 1 ].rgbReserved = 0;
      }
      break;
    case TWPT_GRAY:
      pdib->biClrUsed = 256;
      for ( index = 0; index < 256; index++ )
      {
        rgb[ index ].rgbRed = (BYTE) index;
        rgb[ index ].rgbGreen = (BYTE) index;
        rgb[ index ].rgbBlue = (BYTE) index;
        rgb[ index ].rgbReserved = 0;
      }
      break;

    case TWPT_RGB:
      pdib->biClrUsed = 0;
      break;

    case TWPT_PALETTE:
    case TWPT_CMY:
    case TWPT_CMYK:
    case TWPT_YUV:
    case TWPT_YUVK:
    case TWPT_CIEXYZ:
    default:
      twRC = CallDSMEntry( &m_appID , &m_dsID , DG_IMAGE , DAT_PALETTE8 , MSG_GET , (TW_MEMREF) &pal );
      if ( twRC != TWRC_SUCCESS )
      {
        pdib->biClrImportant = 0;
        pdib->biClrUsed = 256;
        for ( index = 0; index < pal.NumColors; index++ )
        {
          rgb[ index ].rgbRed = (BYTE) index;
          rgb[ index ].rgbGreen = (BYTE) index;
          rgb[ index ].rgbBlue = (BYTE) index;
          rgb[ index ].rgbReserved = 0;
        }
      }
      else
      {
        pdib->biClrUsed = pal.NumColors;
        pdib->biClrImportant = 0;
        for ( index = 0; index < pal.NumColors; index++ )
        {
          rgb[ index ].rgbRed = pal.Colors[ index ].Channel1;
          rgb[ index ].rgbGreen = pal.Colors[ index ].Channel2;
          rgb[ index ].rgbBlue = pal.Colors[ index ].Channel3;
          rgb[ index ].rgbReserved = 0;
        }
      }
      break;
  }

  unsigned char TW_HUGE  *ptr = (unsigned char TW_HUGE *) pdib;
  ptr += sizeof( BITMAPINFOHEADER );
  ptr += pdib->biClrUsed * sizeof( RGBQUAD );

  twRC = CallDSMEntry( &m_appID , &m_dsID , DG_CONTROL , DAT_SETUPMEMXFER , MSG_GET , (TW_MEMREF) &setup );
  if ( twRC != TWRC_SUCCESS )
  {
    free( pdib );
    m_ErrorMessage = "Can't retrive capability info";
    return(FALSE);
  }

  xfer.Memory.Flags = TWMF_APPOWNS | TWMF_POINTER;
  xfer.Memory.Length = setup.Preferred;
  xfer.Memory.TheMem = ptr;

  TW_PENDINGXFERS			twPendingXfer;
  memset( &twPendingXfer , 0 , sizeof( TW_PENDINGXFERS ) );
  BOOL WellDone = FALSE;
  do
  {
    TW_UINT16 twRC2;

    twRC = CallDSMEntry( &m_appID , &m_dsID , DG_IMAGE , DAT_IMAGEMEMXFER , MSG_GET , (TW_MEMREF) &xfer );

    switch ( twRC )
    {
      case TWRC_SUCCESS:
        ptr += xfer.BytesWritten;
        xfer.Memory.TheMem = ptr;
        break;
      case TWRC_XFERDONE:
        WellDone = TRUE;
        FlipBitMap( (LPBYTE) pdib , info.PixelType );
      case TWRC_CANCEL:
      default:
        twRC2 = CallDSMEntry( &m_appID , &m_dsID , DG_CONTROL , DAT_PENDINGXFERS , MSG_ENDXFER , (TW_MEMREF) &twPendingXfer );
    }
  } while ( twRC == TWRC_SUCCESS );

  if ( WellDone ) m_CallBack( m_CallBackParam , (LPBYTE) pdib , sizeof( BITMAPINFOHEADER ) + size + pdib->biClrUsed * sizeof( RGBQUAD ) );
  free( pdib );
  return(TRUE);
}



BOOL CAcquire::DoNativeTransfer()
{
  TW_UINT16           twRC = TWRC_FAILURE;
  TW_UINT16           twRC2 = TWRC_FAILURE;
  TW_UINT32           hBitMap = NULL;
  HANDLE              hbm_acq = NULL;     // handle to bit map from Source to ret to App
  TW_PENDINGXFERS     twPendingXfer;

  if ( !m_CallBack ) return(FALSE);
  BOOL WellDone = FALSE;
  do
  {
    twRC = CallDSMEntry( &m_appID , &m_dsID , DG_IMAGE ,
      DAT_IMAGENATIVEXFER , MSG_GET , (TW_MEMREF) &hBitMap );
    switch ( twRC )
    {
      case TWRC_XFERDONE:  // Session is in State 7
        hbm_acq = (HBITMAP) (size_t)hBitMap;
        twRC2 = CallDSMEntry( &m_appID , &m_dsID , DG_CONTROL , DAT_PENDINGXFERS , MSG_ENDXFER , (TW_MEMREF) &twPendingXfer );
        if ( twRC2 != TWRC_SUCCESS )
        {
          m_ErrorMessage = "Can't retrive image info";
        }
        WellDone = TRUE;
        break;

      case TWRC_CANCEL:   // Session is in State 7
        twRC2 = CallDSMEntry( &m_appID , &m_dsID , DG_CONTROL , DAT_PENDINGXFERS , MSG_ENDXFER , (TW_MEMREF) &twPendingXfer );
        if ( twRC2 != TWRC_SUCCESS )
        {
          m_ErrorMessage = "Can't cancel image acquire";
        }
        break;

      case TWRC_FAILURE:  //Session is in State 6
        m_ErrorMessage = "Error of  image acquiring";

        twRC2 = CallDSMEntry( &m_appID , &m_dsID , DG_CONTROL , DAT_PENDINGXFERS , MSG_ENDXFER , (TW_MEMREF) &twPendingXfer );
        break;

      default:    //Sources should never return any other RC
        twRC2 = CallDSMEntry( &m_appID , &m_dsID , DG_CONTROL , DAT_PENDINGXFERS , MSG_ENDXFER , (TW_MEMREF) &twPendingXfer );
        if ( twRC2 != TWRC_SUCCESS )
        {
          m_ErrorMessage = "Can't cancel image acquire";
        }
        break;
    }

  } while ( twPendingXfer.Count != 0 );

  if ( WellDone )
  {
    BITMAPINFOHEADER* pdib = (BITMAPINFOHEADER*) GlobalLock( hbm_acq );
    DWORD size = pdib->biSizeImage;
    if ( size == 0 )
    {
      size = (pdib->biWidth*pdib->biHeight*(pdib->biBitCount >> 3));
    }
    m_CallBack( m_CallBackParam , (LPBYTE) pdib , sizeof( BITMAPINFOHEADER ) + size + pdib->biClrUsed * sizeof( RGBQUAD ) );
    GlobalUnlock( hbm_acq );
  }
  return(TRUE);
}
