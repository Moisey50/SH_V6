// TectoLibTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#ifndef __wtypes_h__
#include <wtypes.h>
#endif

#ifndef __WINDEF_
#include <windef.h>
#endif

using namespace std ;

#include "winuser.h"
#include <conio.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include "helpers/IPInterface_filex.h"
#include "registry.h"

const char * MainMenu =
"\nSelect operation:\n"
"1 - Init\n"
"2 - Terminate\n"
"3 - Calibration\n"
"4 - SetFlower\n"
"  N - Set flower name for SetFlower operation\n"
"5 - GetParameterList\n"
"6 - Analyze Flower\n" ;

const char * LogLevelView[]=
{
  "DEBUG: ",
  "INFO: " ,
  "WARNING: ",
  "ERROR: ",
  "FATAL: "
};

const char * GetLogLevelAsText( IP_LOG_LEVEL Level )
{
  if ( ( 0 <= ( int ) Level ) && ( ( int ) Level <= 4 ) )
    return LogLevelView[ Level ] ;
  else
    return "UNKNOWN: " ;
  {
  }
}

char LoggerBuf[ 4000 ] ;

extern "C"  void __stdcall Logger( IP_LOG_LEVEL iLogLevel , char * Msg )
{
  strcat_s( LoggerBuf , GetLogLevelAsText( iLogLevel ) ) ;
  strcat_s( LoggerBuf , Msg ) ;
}

bool FormCalibPath( char * Buf , int iBufLen , string& FlowerName )
{
  CRegistry Reg( "TheFileX\\UI_NViews" ) ;
  errno_t eRes = strcpy_s( Buf , iBufLen , Reg.GetRegiString( "Tecto" , "SourcesDir" ,
    "E:/ForProjects/Tecto/Phase2/" ) ) ;
  eRes += strcat_s( Buf , iBufLen , FlowerName.c_str() ) ;
  eRes += strcat_s( Buf , iBufLen , "/" ) ;
  eRes += strcat_s( Buf , iBufLen , Reg.GetRegiString(
    "Tecto" , "CalibFileName" , "calib-1-200.bmp" ) ) ;
  return (eRes == 0) ;
}

int main()
{
  IP_RETURN_CODE RetCode = IP_SUCCESS ;
  std::cout << "Tecto Simulator started!\n";

  CRegistry Reg( "TheFileX\\UI_NViews" ) ;
  string FlowerName = (LPCTSTR)Reg.GetRegiString( "Tecto" , "FlowerName" ,
    "Wax" ) ;
  std::cout << "Flower name '" << FlowerName << "' will be used\n\n"  ;

  char CalibFileName[ 256 ] ;
  ip_RoiPoints roiPoints[] = { { 0 , 0 } , { 0 , 0 } , { 0 , 0 } , { 0 , 0 } } ;
  int iTicketCnt = 0 ;
  int iOperCnt = 0 ;

  LoggerBuf[ 0 ] = 0 ;

  char InChar = 0 ;
  do
  { 
    ++iOperCnt ;
    std::cout << MainMenu ;
    InChar = _getch() ;
    switch ( InChar )
    {
      case '1': 
        RetCode = ipInitialize_filex( Logger ) ;
        break ;
      case '2': 
        RetCode = ipTerminate_filex() ;
        break ;
      case '3':
      {
        if ( !FormCalibPath( CalibFileName , sizeof( CalibFileName ) , FlowerName ) )
        {
          cout << "Can't form calib file path '" << CalibFileName << "'\n" ;
          break ;
        }

        IP_CAM_POSITION camsLayout[] = { FULL_IR } ;
        char * CalibFiles[] = { CalibFileName , NULL } ;
        RetCode = ipAnalyzeCalibration_filex( 1 , CalibFiles , camsLayout ) ;
      }
      break ;
      case '4': 
      {
        if ( !FormCalibPath( CalibFileName , sizeof( CalibFileName ) , FlowerName ) )
        {
          cout << "Can't form calib file path '" << CalibFileName << "'\n" ;
          break ;
        }
        IP_CAM_POSITION camsLayout[] = { FULL_IR } ;
        char * CalibFiles[] = { CalibFileName , NULL } ;

        BOOL bSimulateROIMoving = Reg.GetRegiInt( "Common" , "SimulateROIMoving" , 0 ) ;
        if ( bSimulateROIMoving )
        {
          int iShift = ( iOperCnt % 10 ) * 20 ;
          roiPoints[ 0 ].topleft.x = iShift ;
          roiPoints[ 0 ].topleft.y = 200 + iShift ;
          roiPoints[ 0 ].topright.x = 1300 - iShift ;
          roiPoints[ 0 ].topright.y = 250 + iShift ;
          roiPoints[ 0 ].bottomleft.x = 100 + iShift ;
          roiPoints[ 0 ].bottomleft.y = 700 + iShift ;
          roiPoints[ 0 ].bottomright.x = 1300 - iShift / 2 ;
          roiPoints[ 0 ].bottomright.y = 700 + iShift / 2 ;
        }
        else
          memset( &roiPoints , 0 , sizeof( roiPoints ) ) ;

        RetCode = ipSetFlower_filex( (char*)FlowerName.c_str() , 1 , CalibFiles ,
          camsLayout , roiPoints , NULL ) ;
      }
      break ;
      case 'n':
      case 'N':
        std::cout << "Entry Flower Name: " ;
        std::getline( std::cin , FlowerName ) ;
        std::cout << "Flower name '" << FlowerName << "' will be used\n"  ;
        Reg.WriteRegiString( "Tecto" , "FlowerName" , FlowerName.c_str() ) ;
        RetCode = IP_SUCCESS ;
        break ;
      case '5':
      {
        int iParCount = 0 ;
        char ParameterList[ IP_MAX_PARAMS ][ 100 ] ;
        char FlowerNameTmp[ 128 ] ;
        strcpy_s( FlowerNameTmp , ( char* ) FlowerName.c_str() ) ;
        RetCode = ipGetParametersList_filex( FlowerNameTmp , &iParCount , ParameterList ) ;
        if ( RetCode == IP_SUCCESS )
        {
          for ( int i = 0 ; i < iParCount ; i++ )
            cout << ParameterList[ i ] << '\n' ;
        }
        else
          cout << FlowerNameTmp << '\n' ;
      }
      break ;
      case '6':
      {
        int iNImages = 1 ;
        DWORD dwBufSize = 8192 * 1024 ;
        BYTE * pBuf = new BYTE[ dwBufSize ] ;

        RetCode = ipGetSimulationImages( ( char* ) FlowerName.c_str() ,
          iNImages , pBuf , dwBufSize ) ;

        if ( (RetCode == IP_SUCCESS) && iNImages )
        {
          ip_ImageData images[ 4 ] ;
          memset( images , 0 , sizeof( images ) ) ;
          ip_ImageData * pSrcImage = ( ip_ImageData* ) pBuf ;
          for ( int i = 0 ; i < iNImages ; i++ )
          {
            images[ i ] = *pSrcImage ; // copy image description
            DWORD dwImageSize = GetSHImageBufferSize( pSrcImage ) ;
            DWORD dwImageWithFIleNameSize = dwImageSize + pSrcImage->width ; // file name is after image data
            
            images[ i ].data = new BYTE[ dwImageSize ] ;
            if ( images[ i ].data )
            {
              memcpy_s( images[ i ].data , dwImageSize , pSrcImage + 1 , dwImageSize ) ;
              char * pFileName = ( char* ) ( pSrcImage + 1 ) + dwImageSize ;
              if ( *pFileName )
              {
                char * pTicketBegin = strrchr( pFileName , '\\' ) ;
                if ( !pTicketBegin )
                  pTicketBegin = pFileName ;
                char * pEnd ;
                iTicketCnt = strtoul( pTicketBegin + 1 , &pEnd , 10 ) ;
                cout << "File: " << pFileName << "\n" ;
              }
              if ( i < iNImages - 1 )
                pSrcImage = (ip_ImageData *)(( LPBYTE ) ( pSrcImage + 1 ) + dwImageSize ) ;
            }
            else
            {
              RetCode = IP_GENERAL_ERROR ;
              for ( int j = 0 ; j <= i ; j++ )
              {
                if ( images[ j ].data )
                  delete[] images[ j ].data ;
              }
              std::cout << "Problem with memory allocation for image " << i << "\n"  ;

              break ;
            }

          }
          if ( RetCode == IP_SUCCESS )
          {
            float Params[ IP_MAX_PARAMS ] ;
            memset( Params , 0 , sizeof( Params ) ) ;
            RetCode = ipAnalyzeFlower_filex( iTicketCnt++ , ( char* ) FlowerName.c_str() ,
              1 /* one image*/ , images , (float*)Params ) ;

            if ( RetCode == IP_SUCCESS )
            {
              cout << "Results: " ;
              for ( int i = 0 ; i < IP_MAX_PARAMS ; i++ )
                cout << Params[ i ] << ' ' ;
              cout << '\n' ;
            }
            for ( int i = 0 ; i < iNImages ; i++ )
            {
              if ( images[ i ].data )
                delete[] images[ i ].data ;
            }

          }
        }
        delete pBuf ;
      }
      break ;
      default: 
        cout << "Unknown Command '" << InChar << "'\n" ;
        RetCode = IP_GENERAL_ERROR ;
        break ;
    }

    if ( LoggerBuf[ 0 ] != 0 )
    {
      std::cout << LoggerBuf ;
      LoggerBuf[ 0 ] = 0 ;
    }

    std::cout << "Command: " << InChar << "  Return  " << ipGetErrorString_filex( RetCode ) << "\n" ;
    Sleep( 50 ) ;
  } while ( InChar != VK_ESCAPE );
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
