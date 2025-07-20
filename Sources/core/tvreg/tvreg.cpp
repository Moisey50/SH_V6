// tvreg.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "tvreg.h"
#include <security\basesecurity.h>
#include <conio.h>
#include <fstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// The one and only application object

CWinApp theApp;

using namespace std;

// Flags:
// -s  silent mode (no messages)

// Command line parameters formats
// 1. "-i eval<ENTER>" - Install evaluation license (for 30 days)
// 2. "-d <n days> <ENTER>" - correct license duration
// 3. "-kif<ENTER>" - do form key file and full license info file for current system 
//      This mode is for service people. 
// 4. "-kg<ENTER>" - do form encoded license file for permanent or limited 
//      license obtaining. Usually this file has name "InfoForLicense_<ComputerName>.txt"
// 5. "-ku <SrcFileName><ENTER>" -  generate unlimited license from external key file. 
//      Usually this file is produced on other computer by "-kg" command and has name
//      "InfoForLicense_<ComputerName>.txt".
// 6. "-ky <SrcFileName><ENTER>" -  generate 1 year license from external key file
//      Usually this file is produced on other computer by "-kg" command and has name
//      "InfoForLicense_<ComputerName>.txt".
// 7. "-ks <SrcFileName><ENTER>" -  Set license from file <SrcFileName>. This file 
//      should be formed by "-ku.." or "-ky.." command on the same or on another computer.
// 8. "-kv <SrcFileName><ENTER>" -  View license info from file
//
//     License info is saved in in registry folder Computer\HKEY_CURRENT_USER\SOFTWARE\FileX\tvdb400, 
// in binary item "hinfo" in HEX form. The same content could be written by 
// "-kif" or "-kg" commands and will be used for license viewing, checking, forming and/or modifications
// by other commands.
//
//    Command "-i eval" could be used with SHSTudio.exe for initial evaluation version
// installation which should be followed tvreg.exe starting  with "-kg.." command line 
// on the same computer followed by tvreg.exe starting with "-ku.." command line 
// or "-ky.." command line on the same or another computer and tvreg starting 
// with "ks.." command.

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
  if (argc<2)
  {
    cout<<"Usage:\ntvreg [-s] \"<Company name software will be registered to>\""<<endl;
    cout<<"flag -s means silent operation without messaging"<<endl ;
    getch();
    return 7;
  }
  bool bSilent = false ;
  int iSilentNum = 1 ;
  CString CompanyName ;
  for ( ; iSilentNum < argc ; iSilentNum++ )
  {
    if ( !strcmp(argv[iSilentNum],"-s") )
    {
      bSilent = true ;
      break ;
    }
  }
  
  bool bEval = (!strcmp(argv[1],"-i")) && (!strcmp(argv[2],"eval")) ;
  if ( !bEval )
  {
    if ( iSilentNum == 1 )
      CompanyName = argv[2] ;
    else
      CompanyName = argv[1] ;
  }
  
  bool bCorrectEval = ( !strcmp( argv[ 1 ] , "-d" ) ) ;
  int iDays = ( bCorrectEval ) ? atoi( argv[ 2 ] ) : 0 ;
  bool bFormKeyAndInfoFiles = ( !strcmp( argv[ 1 ] , "-kif" ) ) ;

  CString KeyFile , NDaysAsString ;
  int iNDays = -1 ;
  bool bFormFileForLicensing = ( !strcmp( argv[ 1 ] , "-kg" ) ) ;
  bool bGetLicenseFromFile = ( !strcmp( argv[ 1 ] , "-ks" ) ) && (argc >= 3) ;
  bool bFormUnlimitedLicenseFromFile = ( !strcmp( argv[ 1 ] , "-ku" ) ) && ( argc >= 3 ) ;
  bool bForm1YearLicenseFromFile = ( !strcmp( argv[ 1 ] , "-ky" ) ) && ( argc >= 3 ) ;
  bool bViewLicenseInfoFromFile = ( !strcmp( argv[ 1 ] , "-kv" ) ) && ( argc >= 3 ) ;
  // CU in registry 
  int nRetCode = 0;
// initialize MFC and print and error on failure
  if ( !AfxWinInit( ::GetModuleHandle( NULL ) , NULL , ::GetCommandLine() , 0 ) )
  {
    if ( !bSilent )
    {
      cerr << _T( "Fatal Error: MFC initialization failed" ) << endl;
      getch();
    }
    nRetCode = 1;
  }
  else if ( bCorrectEval )
  {
    if ( iDays != 0 )
      correct_eval( iDays ) ;
  }
  else if ( bFormFileForLicensing )
  {
    SystemIDs IDs ;
    IDs.LoadLicenseInfo() ;
    return ( IDs.SaveSystemInfo( false , FFC_DoInfoForLicenseFile ) ? 0 : 6 ); // save to file, if OK return 0
  }
  else if ( bFormKeyAndInfoFiles )
  {
    SystemIDs IDs ;
    IDs.LoadLicenseInfo() ;
    // save to file, and save decoded info as text to view file, if OK return 0
    return ( IDs.SaveSystemInfo( false , FFC_WriteSysInfoAndKey ) ? 0 : 6 ); 
  }
  else if ( bGetLicenseFromFile
    || bFormUnlimitedLicenseFromFile 
    || bForm1YearLicenseFromFile 
    || bViewLicenseInfoFromFile )
  {
    KeyFile = argv[2] ;

    if ( KeyFile.IsEmpty() )
    {
      cerr << _T( "Fatal Error: no file name; command format is 'tvreg.exe -ks <File Name>'\n" );
      return 7 ;
    }
   // Form encoded file
    std::fstream SrcFile( (LPCTSTR)KeyFile , std::fstream::in );
    if ( SrcFile.is_open() )
    {
      TCHAR Char ;
      FXString AsHex ;
      while ( (Char = SrcFile.get()) != EOF )
        AsHex += Char ;
      SrcFile.close() ;

      std::string Encoded( ( LPCTSTR ) AsHex ) ;

      SystemIDs IDs , LocalIDs ;
      if ( IDs.DecodeSystemInfo( Encoded , IDs ) )
      {
        if ( bGetLicenseFromFile )
        {
          LocalIDs.GetThisSystemInfo() ;
          if ( LocalIDs.m_Info.Compare( IDs.m_Info ) )
          {
            IDs.SaveSystemInfo( true ) ; // to registry
            cout << _T( "Success: License updated\n" );
            return 0 ;
          }
          cerr << _T( "Fatal Error: License file is not for this computer\n" );
          return 8 ;
        }
        else if ( bFormUnlimitedLicenseFromFile )
        {
          IDs.m_Info.m_SHLicenseStatus = "NoRestrictions" ;
          IDs.m_Info.m_LicenseDate = GetTimeStamp().Left( 8 ) ;
          IDs.m_Info.m_Licensee = IDs.m_Info.m_ComputerName ;
          IDs.m_SaveFileName = IDs.m_Info.m_ComputerName + _T("_License.dat") ;
          if ( IDs.SaveSystemInfo( false ) )
          {
            cout << _T( "Success: Unlimited License file " ) << (LPCTSTR) IDs.m_SaveFileName << " Is written.\n" ;
            return 0 ;
          }
          cerr << _T( "Fatal Error: Can't save license to file\n" );
          return 11 ;
        }
        else if ( bForm1YearLicenseFromFile )
        {
          IDs.m_Info.m_SHLicenseStatus.Format( "TimeLimited, 365 days" ) ;
          IDs.m_Info.m_LicenseDate = GetTimeStamp().Left( 8 ) ;
          IDs.m_Info.m_Licensee = IDs.m_Info.m_ComputerName ;
          IDs.m_SaveFileName = IDs.m_Info.m_ComputerName + _T( "_License.dat" ) ;
          if ( IDs.SaveSystemInfo( false ) )
          {
            cout << _T( "Success: 365 days license file " ) << ( LPCTSTR ) IDs.m_SaveFileName << " Is written.\n" ;
            return 0 ;
          }
          cerr << _T( "Fatal Error: Can't save license to file\n" );
          return 11 ;
        }
        else if ( bViewLicenseInfoFromFile )
        {
          IDs.ViewLicenseInfoOnCOUT() ;
          return 0 ;
        }
        cerr << _T( "Fatal Error: Unknown key set\n" );
        return 8 ;
      }
      cerr << _T( "Fatal Error: Bad License file content\n" );
      return 9 ;
    }
    cerr << _T( "Fatal Error: Can't open license file\n" );
    return 10 ;
  }
//   else if ( isevaluation() > EOL_NoRestriction ) // evaluation or limited license duration
//   {
//     HKEY app_key = openthis();
//     if ( !app_key )
//     {
//       app_key = createthis();
//     }
//     if ( !app_key )
//     {
//       if ( !bSilent )
//       {
//         cerr << _T( "Fatal Error: Can't create proper records in registry" );
//         getch();
//       }
//       return 2;
//     }
//     else
//       RegCloseKey( app_key );
//     if ( !is_evalkey_exists() )
//     {
//       if ( !bSilent )
//       {
//         cerr << _T( "Fatal Error: Evaluation mode is not set" );
//         getch();
//       }
//       return 3;
//     }
//     else if (  )
//     {
//       setupdata sd;
//       _read_eval_key( &sd );
//       if ( sd._seed == 0 ) // CU in registry
//       {
//         strcpy_s( sd._registredto , ( LPCTSTR ) CompanyName ) ;
//         if ( !_create_permanent_key( &sd ) )
//         {
//           if ( !bSilent )
//           {
//             cerr << _T( "Fatal Error: Unexpected error" );
//             getch();
//           }
//           return 4;
//         }
//         if ( !bSilent )
//         {
//           cerr << _T( "Success!" );
//           getch();
//         }
//         return 0;
// 
//       }
//       else if ( !_decode( &sd ) )
//       {
//         if ( !bSilent )
//         {
//           cerr << _T( "Fatal Error: Unexpected error" );
//           getch();
//         }
//         return 4;
//       }
//       sd._eval_or_license = ( char ) EOL_NoRestriction ;
//       if ( strlen( argv[ 1 ] ) > ( REGISTREDTO_MAXLEN - 1 ) )
//         argv[ 1 ][ REGISTREDTO_MAXLEN - 1 ] = 0;
//       strcpy( sd._registredto , ( LPCTSTR ) CompanyName );
//       _encode( &sd );
//       if ( _create_eval_key( &sd ) )
//       {
//         if ( !bSilent )
//         {
//           cerr << _T( "Success!" );
//           getch();
//         }
//         return 0;
//       }
//       else
//       {
//         if ( !bSilent )
//         {
//           cerr << _T( "Fatal Error: Unexpected error" );
//           getch();
//         }
//         return 5;
//       }
//     }
//   }
  else 
  {
    if ( !bSilent )
    {
      int iRestDays = 0 ;
      bool bLocalMachine = true ;
      switch ( isevaluation( bLocalMachine , &iRestDays ) )
      {
        case EOL_NoRestriction:
          cout << _T( "Stream Handler has unlimited license on this computer\n" ); 
          getch() ;
          break ;
        case EOL_Evaluation:
          cout << _T( "Stream Handler has evaluation license on this computer for " ) << iRestDays << " days\n" ; 
          getch() ;
          break ;
        case EOL_License:
          cout << _T( "Stream Handler has time limited license on this computer for " ) << iRestDays << " days\n" ;
          getch() ;
          break ;
        default:
          cerr << _T( "Fatal Error: Before usage the Stream Handler must be registered for evaluation!" ); 
          getch() ;
          break ;
      }
    }
    return 5;
  }

  if ((GetWindowsVersionInfo()>=6) && (!IsRunAsAdmin()) )
  {
    CString tmpS;
    if ( !bEval )
    {
      if ( bCorrectEval )
      {
        tmpS = argv[ 1 ] ;
        tmpS += " " ;
        tmpS += argv[ 2 ] ;
      }
      else
      {
        tmpS = ( ( bSilent ) ? CString( "-s " ) + CString( "\"" ) + CompanyName
          : CString( "\"" ) + CompanyName ) + "\"";
      }
    }
    if (!ElevatePrivilege( (bEval ?  NULL : (LPCTSTR)tmpS) ) )
    {
      cerr << _T("Fatal Error: The Administrative rights required to change a key") << endl;
      getch();
    }
    return -6;
  }
  nRetCode = 0;
  bool bLocalMachine = true ;
  // initialize MFC and print and error on failure
  if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
  {
    if ( !bSilent )
    {
      cerr << _T("Fatal Error: MFC initialization failed") << endl;
      getch();
    }
    nRetCode = 1;
  }
  else if ( bCorrectEval )
  {
    if ( iDays != 0 )
    {
      correct_eval( iDays ) ;
    }
  }
  else if ( isevaluation( bLocalMachine ) )
  {
    HKEY app_key=openthis();
    if (!app_key)
    {
      app_key=createthis();
    }
    if (!app_key) 
    {
      if ( !bSilent )
      {
        cerr<<_T("Fatal Error: Can't create proper records in registry");
        getch();
      }
      return 2;
    }
    else 
      RegCloseKey(app_key);
    if (!is_evalkey_exists())
    {
      if ( !bSilent )
      {
        cerr<<_T("Fatal Error: Evaluation mode is not set");
        getch();
      }
      return 3;
    }
    setupdata sd;
    _read_eval_key(&sd);
    if (!_decode(&sd)) 
    {
      if ( !bSilent )
      {
        cerr<<_T("Fatal Error: Unexpected error");
        getch();
      }
      return 4;
    }
    sd._eval_or_license = (char)EOL_NoRestriction ;
    if (strlen(argv[1])>(REGISTREDTO_MAXLEN-1))
      argv[1][REGISTREDTO_MAXLEN-1]=0;
    strcpy(sd._registredto,(LPCTSTR)CompanyName);
    _encode(&sd);
    if (_create_eval_key(&sd))
    {
      if ( !bSilent )
      {
        cerr<<_T("Success!");
        getch();
      }
      return 0;
    }
    else
    {
      if ( !bSilent )
      {
        cerr<<_T("Fatal Error: Unexpected error");
        getch();
      }
      return 5;
    }
  }
  else
  {
    if ( !bSilent )
    {
      cerr<<_T("Fatal Error: Before usage the Stream Handler must be registered for evaluation!");
      getch();
    }
    return 5;
  }
  return nRetCode;
}
