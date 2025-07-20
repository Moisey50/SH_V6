::**********************************************
::Name:    Full Auto Build Solutions for SH Only
::Author:  Yuri Sapozhnikov (+972-(0)5442-39193)
::Company:                            File X Ltd
::Version:                               rev.002
::Date: 	                          29/09/2020
::Web:                       https://ss64.com/nt
::YouTube:
::Description: Automaticaly builds all solutions
::             for all Platforms fo Release and
::             Debug
::**********************************************
@::!/dos/rocks
@ECHO OFF
 :: https://ss64.com/

 set OutputFolderPath=%SH_OUTPUT%
 ::echo %OutputFolderPath%
 set Compiler="%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Enterprise\Common7\IDE\devenv.com"

 set VS_YEAR[0]=2017

 set VS_Version[0]=Community
 set VS_Version[1]=Enterprise
 
 set COMMAND[0]=Build
 set COMMAND[1]=Clean
 set COMMAND[2]=Rebuild

 set Platform[0]=x86
 set Platform[1]=x64

 set Configuration[0]=Debug
 set Configuration[1]=Release
 
 set SolutionName[0]=core
 set SolutionName[1]=gadgets
 set SolutionName[2]=FileXGadgets
 set SolutionName[3]=Utilities
 
 set ProjectName[0]=shwrapper
 set ProjectName[1]=CSWrapper
 set ProjectName[2]=GraphDeploy
 set ProjectName[3]=UIFor2Views
 set ProjectName[4]=UI_NViews
 set ProjectName[5]=SH_ShMemStaticLibInterface

 set cmdIdx=0
 
 call set Cmd=%%COMMAND[%cmdIdx%]%%
 
 call TITLE Full Auto %Cmd%
 call :fnc_PerformSolutions %Cmd%
 
 ::for /F "tokens=*" %%p in ('dir /b /s /a:d %OutputFolderPath%\*') do call :fnc_DeleteIntemediateFolderWithContent %%p
 
 echo "Done"
 pause
 exit /B %ERRORLEVEL%

 :fnc_DeleteIntemediateFolderWithContent
 setlocal
  set fp=%~1
  echo %fp%
  set fn=%fp:~-11%
  ::echo %fn%
  
  if /i not %fn%==Intermediate rmdir /Q /S "%fp%"
  ::& del /Q "%fp%"
  
 endlocal
 exit /b %ERRORLEVEL%

 :fnc_PerformSolutions
 setlocal
	set Cmd=%~1
	
	SET /a "idx=0"
	:loop_Sln
	::pause
	::echo %idx%
	if not defined SolutionName[%idx%] goto :endLoop_Sln
	call set SlnName=%%SolutionName[%idx%]%%
		
		echo %Cmd%ing the '%SlnName%.sln'
		::pause
		::echo if %idx% NEQ 3
		::pause
		if NOT "%idx%"=="3"	 call :fnc_PerformPlatforms %Cmd%,%SlnName%
		
		if "%idx%"=="3" (^
    		call :fnc_PerformPlatforms %Cmd%,%SlnName%,%%ProjectName[0]%%^
			& call :fnc_PerformPlatforms %Cmd%,%SlnName%,%%ProjectName[1]%%^
			& call :fnc_PerformPlatforms %Cmd%,%SlnName%,%%ProjectName[2]%%^
			& call :fnc_PerformPlatforms %Cmd%,%SlnName%,%%ProjectName[3]%%^
			& call :fnc_PerformPlatforms %Cmd%,%SlnName%,%%ProjectName[4]%%^
			& call :fnc_PerformPlatforms %Cmd%,%SlnName%,%%ProjectName[5]%%^
			)
	
	SET /a "idx+=1"
	GOTO :loop_Sln
	
	:endLoop_Sln
 endlocal
 EXIT /B %ERRORLEVEL%


 :fnc_PerformPlatforms
 setlocal
	set Cmd=%~1
	set SlnName=%~2
	set PrjctName=%~3
	
	SET /a "idx=0"
	:loop_Pfrm	
	if not defined Platform[%idx%] goto :endLoop_Pfrm
	call set Pltfrm=%%Platform[%idx%]%%
		
		call :fnc_PerformConfigurations %Cmd%,%SlnName%,%Pltfrm%,%PrjctName%
	
	SET /a "idx+=1"
	GOTO :loop_Pfrm
	
	:endLoop_Pfrm
 endlocal
 EXIT /B %ERRORLEVEL%


 :fnc_PerformConfigurations
 setlocal
	set Cmd=%~1
	set SlnName=%~2
	set Pltfrm=%~3
	set PrjctName=%~4
	
	SET /a "idx=0"
	:loop_Cfg
	if not defined Configuration[%idx%] goto :endLoop_Cfg
	call set Cnfg=%%Configuration[%idx%]%%
	
		call :fnc_PerformAction %Cmd%,%SlnName%,%Pltfrm%,%Cnfg%,%PrjctName%
		
	SET /a "idx+=1"
	::echo cnfg id is %idx%
	::pause
	GOTO :loop_Cfg
	
	:endLoop_Cfg
 endlocal
 EXIT /B %ERRORLEVEL%

 
 :fnc_PerformAction
 setlocal
	set "Cmd=%~1"
	set "SlnName=%~2"
	set "Pltfrm=%~3"
	set "Cnfg=%~4"
	set "PrjctName=%~5"
	
	call set BuldResultsLogPath=%OutputFolderPath%\AutoBuildResults\%SlnName%
	IF NOT EXIST %BuldResultsLogPath% mkdir %BuldResultsLogPath%
	
	call :fnc_GetTimestamp TimeStamp
	::pause
	set "BuldResultsLogFilePath=%BuldResultsLogPath%\%TimeStamp%.%Cmd%.%SlnName%"
	
	::pause
	echo Platform and Configuration - "%Pltfrm%|%Cnfg%"
	
	::pause
	set "Prjct="
	if defined PrjctName  set "Prjct=/Project %PrjctName%" & set "BuldResultsLogFilePath=%BuldResultsLogFilePath%.%PrjctName%"
	
	set BuldResultsLogFilePath="%BuldResultsLogFilePath%.%Pltfrm%.%Cnfg%.Log"
	
	set FullCmd= /%Cmd% "%Cnfg%|%Pltfrm%" %Prjct% "%cd%\%SlnName%\%SlnName%.sln"
	echo %FullCmd%
	%Compiler% %FullCmd% > "%BuldResultsLogFilePath%"
	echo Find %Cmd% results at %BuldResultsLogFilePath%
	
	::pause
 endlocal
 EXIT /B %ERRORLEVEL%
 
  
 :fnc_GetTimestamp
 rem IMPORTANT!
 rem This function should RETURN value,
 rem so DON'T insert here 'setlocal' and 'endlocal'

	for /f "delims=" %%a in ('wmic OS Get localdatetime ^| find "."') do call set DateTime=%%a

	call set Yr=%DateTime:~0,4%
	call set Mon=%DateTime:~4,2%
	call set Day=%DateTime:~6,2%
	call set Hr=%DateTime:~8,2%
	call set Min=%DateTime:~10,2%
	call set Sec=%DateTime:~12,2%

	call set ts=%Yr%-%Mon%-%Day%_%Hr%-%Min%-%Sec%

	::return value
	set %~1=%ts%
 EXIT /B %ERRORLEVEL%