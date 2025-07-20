::**********************************************
::Name:   Add/change System enviornment variable
::Author:  Yuri Sapozhnikov (+972-(0)5442-39193)
::Company:                            File X Ltd
::Version:                               rev.003
::Date: 	                          14/03/2023
::Web:   https://ss64.com/vb/syntax-elevate.html
::YouTube:
::Description: Add/change 'SH_OUTPUT" System 
::             enviornment variable. Elevates 
::             to Admin if nesseceary.
::
::Revisions History
::
::Rev.002 Date: 28/03/2019 Cause: Fixed for folder
::                                with spaces.
:: 
::**********************************************
@Echo Off
TITLE Add/change 'SH_OUTPUT" System Environment Variable

Setlocal EnableDelayedExpansion
                                rem 1. Defines that all variables initialesed between SETLOCAL and ENDLOCAL flags are local.
                                rem 2. The variables DELAYED expansion is performed each time the line is executed,
                                rem    or for each loop in a FOR looping command.
								rem    For simple commands this will make NO noticable difference, but with loop commands like FOR,
                                rem    compound or bracketed () expressions delayed expansion will allow you to always see the current value of the variable.
								rem    When delayed expansion is in effect, variables can be immediately read using !variable_name! notation
                                rem    (instead %variable_name% notation). Anyway the %variable_name% notation variable can be used
                                rem    but that will continue to show the INITIAL value (expanded at the beginning of the line).
								rem    EXAMPLE:
								rem        
								rem        @echo off
								rem        SETLOCAL EnableDelayedExpansion
								rem        Set "_var=first"
								rem        Set "_var=second" & Echo %_var% !_var!
								rem        
								rem    This will output: first second
								rem    The value of the !_var! variable is evaluated as late as possible, while the %_var% variable works just as before.
								rem    [ref: https://ss64.com/nt/delayedexpansion.html ]
:: First check if we are running As Admin/Elevated
FSUTIL dirty query %SystemDrive% >nul
if %errorlevel% EQU 0 goto START
   Echo The '%~nx0' batch is now running as NON admin with [%*] arguments!
   ::WHEN: %~nx0 - Expand %0 argument to a file name and extension only.
   ::              (REMEMBER: %0 argument allways references to THIS batch file)
   ::              [NOTE: More information about filename parameter extention options
   ::                     here - https://ss64.com/nt/syntax-args.html >> "Parameter Extensions"]
   ::      %* - refers to all the arguments (e.g. %1 %2 %3 %4 %5 ...%255) passed to this batch in NON elevated mode.
   ::           (REMEMBER: only arguments %1 to %9 can be referenced by number)
   ::           [NOTE: More information about filename parameter extention options
   ::                  here - https://ss64.com/nt/syntax-args.html >> "Parameter Extensions"]

::When this batch launched as User than
::Create and run a temporary VBScript to elevate this batch file
   Set "_vbsFileName=~ElevateMe.vbs" rem ATTENTION: [ref: https://ss64.com/nt/set.html]
                                     rem     1. Variable initialization should be made without SPACES around '='
                                     rem        (e.g.: 'Set _vbsFileName=~ElevateMe.vbs' is right and
                                     rem               'Set _vbsFileName = ~ElevateMe.vbs' is NOT right)
								     rem     2. Avoid starting variable names with a number,
                                     rem        this will avoid the variable being mis-interpreted as a parameter.
									 rem        [ref: https://ss64.com/nt/set.html]

   Set "_batchFile=%~f0"
   ::WHEN: %~f0 - full pathname of THIS batch file
   Set "_Args=%_vbsFileName% %*"
   ::WHEN: %_vbsFileName% - temporary vbscript file name passed as 1st argumet to this batch launched in ELEVATED mode.
   ::                       In the ELEVATED mode this argument will be used to delete the %_vbsFileName% file.
   ::      %* - refers to all the arguments (e.g. %1 %2 %3 %4 %5 ...%255) passed to this batch in NON elevated mode.
   ::           (REMEMBER: only arguments %1 to %9 can be referenced by number)
   
   rem Uncomment following line to DEBUG!
   rem Echo _batchFile= %_batchFile%
   rem Uncomment following line to DEBUG!
   rem Echo _Args= %_Args%
   
   rem Uncomment following line to DEBUG!
   rem Echo Starting to create the Temporary VBScript file at "%temp%\%_vbsFileName%" ...
   :: Writes VBScript [all echoed text inside ()] to the temporary "%temp%\%_vbsFileName%" file
   (
	Echo Set UAC = CreateObject^("Shell.Application"^)
	
	rem Uncomment following line to DEBUG! ('cmd' with '/k' launched from the vbscript will open
    rem a new Command Promt window).
	rem	Echo UAC.ShellExecute "cmd", "/k ""%_batchFile% %_Args%""", "", "runas", 1
	
	rem Comment following line to DEBUG!
	rem
	Echo UAC.ShellExecute "cmd", "/c ""%_batchFile% %_Args%""", "", "runas", 1
	
	rem EXAMPLE after variables opened:
    rem  UAC.ShellExecute "cmd", "/k ""D:\Workspace\SH_Sources\SetSystemEnvironmen.bat output""", "", "runas", 1
	
	Echo Set UAC = Nothing
   ) > "%temp%\%_vbsFileName%"
   rem Uncomment following line to DEBUG!
   rem Echo Temporary VBScript file successfully created at "%temp%\%_vbsFileName%" ...
   
   ::Execue temporary VBScript
   cscript "%temp%\%_vbsFileName%"

   Exit /B

:START
:: Launches the code which requires Admin/elevation permitions
:: set the current directory to the batch file location
cd /d %~dp0
   ::WHEN: %~dp0 - Expand %0 argument to a drive letter and path only.
   ::              (REMEMBER: %0 argument allways references to THIS batch file)
   ::              [NOTE: More information about filename parameter extention options
   ::                     here - https://ss64.com/nt/syntax-args.html >> "Parameter Extensions"]

Echo The '%~nx0' batch is now running as admin with [%*] arguments
   ::WHEN: %~nx0 - Expand %0 argument to a file name and extension only.
   ::              (REMEMBER: %0 argument allways references to THIS batch file)
   ::              [NOTE: More information about filename parameter extention options
   ::                     here - https://ss64.com/nt/syntax-args.html >> "Parameter Extensions"]
   ::      %* - refers to all the arguments (e.g. %1 %2 %3 %4 %5 ...%255) passed to this batch in NON elevated mode.
   ::           (REMEMBER: only arguments %1 to %9 can be referenced by number)
   ::           [NOTE: More information about filename parameter extention options
   ::                  here - https://ss64.com/nt/syntax-args.html >> "Parameter Extensions"]

Set "_sysVarName=SH_OUTPUT"
Set "_sysVarValueAct=%SH_OUTPUT%"
Set "_sysVarValueNew=%cd%\Output"
Set "_actDeplOutputHereFileName=DEPLOYMENT_OUTPUT_IS_HERE.txt"

if defined _sysVarValueAct (
   rem ATTENTION: The variable that should be checked by DEFINED operator should be WITHOUT percentiles [%...%]!
   if exist "%_sysVarValueAct%\..\%_actDeplOutputHereFileName%" (
      Echo Deleting "%_sysVarValueAct%\..\%_actDeplOutputHereFileName%" ...
      del "%_sysVarValueAct%\..\%_actDeplOutputHereFileName%"
   )
) 
 
Echo SETX %_sysVarName% %_sysVarValueNew% /m called.
SETX %_sysVarName% "%_sysVarValueNew%" /m >%temp%\~SetXRes.txt

rem Reads the file %temp%\~SetXRes.txt,
rem Removes all the blank lines
rem Writes result to %temp%\~SetXRes2.txt
FINDSTR /v "^$" %temp%\~SetXRes.txt >%temp%\~SetXRes2.txt
Set /p _setVarRes= < %temp%\~SetXRes2.txt 
del %temp%\~SetXRes?.txt

if defined _setVarRes (
   rem ATTENTION: The variable that should be checked by DEFINED operator should be WITHOUT percentiles [%...%]!
    Echo %_setVarRes%
    Echo Creating "%_sysVarValueNew%\..\%_actDeplOutputHereFileName%" ...
    (
	 Echo %_sysVarName%=%_sysVarValueNew%
	 Echo %_setVarRes%
    ) >"%_sysVarValueNew%\..\%_actDeplOutputHereFileName%"
	
	if NOt exist "%_sysVarValueNew%" (
		md %_sysVarValueNew%
	)
	
    Echo Created "%_sysVarValueNew%\..\%_actDeplOutputHereFileName%" ...
) ELSE (
  Echo SETX %_sysVarName% %_sysVarValueNew% /m
  Echo Action failed!
)

if NOT [%1]==[] (
   if exist "%temp%\%1" (
      Echo Deleting "%temp%\%1" ...
      del "%temp%\%1"
   )
)

Endlocal rem Defines that all variables initialesed between SETLOCAL and ENDLOCAL flags are local.

exit /b