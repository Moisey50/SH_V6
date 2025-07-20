#ifndef EXEC_AND_REDIRECT_INC
#define EXEC_AND_REDIRECT_INC

inline BOOL ExecAndRedirect( LPCTSTR commandLine , FXString& Output , LPDWORD lpExitCode )
{
  HANDLE hOutput;
  Output.Empty();
  HANDLE hStdOutputWritePipe , hStdOutput , hStdError;
  CreatePipe( &hOutput , &hStdOutputWritePipe , NULL , 0 );	// create a non-inheritable pipe
  DuplicateHandle( GetCurrentProcess() , hStdOutputWritePipe ,
    GetCurrentProcess() , &hStdOutput ,	// duplicate the "write" end as inheritable stdout
    0 , TRUE , DUPLICATE_SAME_ACCESS );
  DuplicateHandle( GetCurrentProcess() , hStdOutput ,
    GetCurrentProcess() , &hStdError ,	// duplicate stdout as inheritable stderr
    0 , TRUE , DUPLICATE_SAME_ACCESS );
  CloseHandle( hStdOutputWritePipe );								// no longer need the non-inheritable "write" end of the pipe

  PROCESS_INFORMATION pi;
  STARTUPINFO si;
  ZeroMemory( &si , sizeof( STARTUPINFO ) );
  si.cb = sizeof( STARTUPINFO );
  si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
  si.hStdInput = GetStdHandle( STD_INPUT_HANDLE );	// (this is bad on a GUI app)
  si.hStdOutput = hStdOutput;
  si.hStdError = hStdError;
  si.wShowWindow = SW_HIDE;						// IMPORTANT: hide subprocess console window
  TCHAR commandLineCopy[ 1024 ];					// CreateProcess requires a modifiable buffer
  _tcscpy( commandLineCopy , commandLine );
  if ( !CreateProcess( NULL , commandLineCopy , NULL , NULL , TRUE ,
    CREATE_NEW_CONSOLE , NULL , NULL , &si , &pi ) )
  {
    CloseHandle( hStdOutput );
    CloseHandle( hStdError );
    CloseHandle( hOutput );
    hOutput = INVALID_HANDLE_VALUE;
    return FALSE;
  }
  WaitForSingleObject( pi.hProcess , INFINITE );
  GetExitCodeProcess( pi.hProcess , lpExitCode );

  CloseHandle( pi.hThread );
  CloseHandle( hStdOutput );
  CloseHandle( hStdError );

  CHAR buffer[ 65 ];
  DWORD read;
  while ( ReadFile( hOutput , buffer , 64 , &read , NULL ) )
  {
    buffer[ read ] = '\0';
    Output += buffer;
  }

  CloseHandle( hOutput );
  CloseHandle( pi.hProcess );
  return TRUE;
}


static HWND hFound ;

BOOL CALLBACK EnumChildProc(
  _In_ HWND   hWnd ,
  _In_ LPARAM lParam
)
{
  TCHAR Buf[ 1000 ] ;
  int length = ::GetWindowTextLength( hWnd );
  if ( 0 == length )
    return TRUE;

  Buf[ 0 ] = 0 ;

  if ( GetWindowText( hWnd , Buf , sizeof( Buf ) / sizeof( Buf[ 0 ] ) ) )
  {
    _tcsupr( Buf ) ;
    if ( _tcsstr( Buf , (LPCTSTR) lParam ) == Buf )
    {
      hFound = hWnd ;
      return FALSE ;  // stop: found
    }
    else // enum child windows
    {
      BOOL bChildFound = EnumChildWindows( hWnd , EnumChildProc , (LPARAM) lParam ) ;
      return FALSE ;
    }
  }

  return TRUE; // continue
}


BOOL CALLBACK enumWindowsProc(
  __in  HWND hWnd ,
  __in  LPARAM lParam
)
{
  TCHAR Buf[ 1000 ] ;
  int length = ::GetWindowTextLength( hWnd );
  if ( 0 == length ) 
    return TRUE;

  Buf[ 0 ] = 0 ;

  if ( GetWindowText( hWnd , Buf , sizeof( Buf ) / sizeof( Buf[ 0 ] ) ) )
  {
    _tcsupr( Buf ) ;
    if ( _tcsstr( Buf , (LPCTSTR) lParam ) == Buf )
    {
      hFound = hWnd ;
      return FALSE ;  // stop: found
    }
    else // enum child windows
    {
      BOOL bChildFound = EnumChildWindows( hWnd , EnumChildProc , (LPARAM) lParam ) ;
      return (hFound == NULL) ;
    }
  }

  return TRUE; // continue
}

inline HWND GetWindowHandle( LPCTSTR pCaption , HWND hMainWindow = NULL )
{
  hFound = NULL ;
  BOOL bWindowFound = ( hMainWindow) ? 
    ::EnumChildWindows( hMainWindow , EnumChildProc , (LPARAM) pCaption ) :
    ::EnumWindows( enumWindowsProc , (LPARAM)pCaption );
  return hFound ;
}


#endif