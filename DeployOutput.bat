::**********************************************
::Name:                      Deploy Output lib's
::Author:  Yuri Sapozhnikov (+972-(0)5442-39193)
::Company:                            File X Ltd
::Version:                               rev.002
::Date: 	                          28/03/2019
::Web:                      https://ss64.com/nt
::YouTube:
::Description: Deploys the Output content from
::             the 'SH_SOURCE' to the
::             'SH_OUTPUT' folder.
::**********************************************

@ECHO OFF
 TITLE SH Output Deployment
 :: https://ss64.com/
 
 set "SH_Out=%SH_OUTPUT%"
 set "SH_SOURCE_Out=%cd%\Output"
 ECHO.
 ECHO SH Output Deployment folder path value
 ECHO SH_Out = %SH_OUTPUT%
 ECHO SH_SOURCE_Out = %SH_SOURCE_Out%
 ECHO.
  
 if defined SH_Out (
	if defined SH_SOURCE_Out (
		rem ATTENTION: The variable that should be checked by DEFINED operator should be WITHOUT percentiles [%...%]!
		
		if "%SH_Out%"=="%SH_SOURCE_Out%" (
			ECHO SH_Out path is same as SH_SOURCE_Out. Nothing to deploy !
		) else (
			if exist "%SH_Out%" (
				ECHO Clean SH Output Deployment folder content
				Echo Deleting "%SH_Out%" ...
				(del /F /S /Q "%SH_Out%" && rd /S /Q "%SH_Out%") || ECHO Failed to remove the '%SH_Out%' folder!
			)
			
			TIMEOUT  10
			
			ECHO.
			ECHO.
			ECHO. 
			ECHO Deploy SH Output content [EXCLUDE 'Intermediate' folder]
			ECHO.
			for /f "tokens=*" %%V in ('dir /b /ad .\Output') do (
				echo %%V - Visual Studio version folder name
				for /f "tokens=*" %%P in ('dir /b /ad .\Output\%%V') do (
					echo %%P - Platform folder name
					for /f "tokens=*" %%F in ('dir /b /ad .\Output\%%V\%%P') do (
						echo %%F - Build configuration folder name
						if "%%F"=="Intermediate" (
							echo Skip copying .\Output\%%V\%%P\%%F
						) else (
							echo Start copying  to the %SH_OUTPUT%\%%V\%%P\%%F
							XCOPY /V /S /Y /C  "Output\%%V\%%P\%%F\*" "%SH_OUTPUT%\%%V\%%P\%%F\*"
						)
					)
				)
			)
		)
	)
 ) else (
	ECHO Global var. SH_OUTPUT is NOT defined yet.
	ECHO Look for the SetSystemEnvironmen-SH_Output.bat script
	ECHO and then try this script again.
)
 
 ECHO.
 ECHO Exiting deployment script ...
 ECHO.
 ECHO.
 ECHO.
 ECHO.
 PAUSE
