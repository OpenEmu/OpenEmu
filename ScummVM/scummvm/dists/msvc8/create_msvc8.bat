@echo off

echo.
echo Automatic creation of the MSVC9 project files
echo.

if "%~1"=="/stable" goto stable
if "%~1"=="/STABLE" goto stable
if "%~1"=="/all"    goto all
if "%~1"=="/ALL"    goto all
if "%~1"=="/tools"  goto tools
if "%~1"=="/TOOLS"  goto tools
if "%~1"=="/clean"  goto clean_check
if "%~1"=="/CLEAN"  goto clean_check
if "%~1"=="/help"   goto command_help
if "%~1"=="/HELP"   goto command_help
if "%~1"=="/?"      goto command_help

if "%~1"==""        goto check_tool

echo Invalid command parameter: %~1
echo.

:command_help
echo Valid command parameters are:
echo   stable   Generated stable engines project files
echo   all      Generate all engines project files
echo   tools    Generate project files for the devtools
echo   clean    Clean generated project files
echo   help     Show help message
goto done

:check_tool
if not exist create_project.exe goto no_tool

:question
echo.
set batchanswer=S
set /p batchanswer="Enable stable engines only, or all engines? (S/a)"
if "%batchanswer%"=="s" goto stable
if "%batchanswer%"=="S" goto stable
if "%batchanswer%"=="a" goto all
if "%batchanswer%"=="A" goto all
goto question

:no_tool
echo create_project.exe not found in the current folder.
echo You need to build it first and copy it in this
echo folder
goto done

:all
echo.
echo Creating project files with all engines enabled (stable and unstable)
echo.
create_project ..\.. --enable-all-engines --msvc --msvc-version 8
goto done

:stable
echo.
echo Creating normal project files, with only the stable engines enabled
echo.
create_project ..\.. --msvc --msvc-version 8
goto done

:tools
echo.
echo Creating tools project files
echo.
create_project ..\.. --tools --msvc --msvc-version 8
goto done

:clean_check
echo.
set cleananswer=N
set /p cleananswer="This will remove all project files. Are you sure you want to continue? (N/y)"
if "%cleananswer%"=="n" goto done
if "%cleananswer%"=="N" goto done
if "%cleananswer%"=="y" goto clean
if "%cleananswer%"=="Y" goto clean
goto clean_check

:clean
echo.
echo Removing all project files
del /Q *.vcproj* > NUL 2>&1
del /Q *.vsprops > NUL 2>&1
del /Q *.sln* > NUL 2>&1
del /Q scummvm* > NUL 2>&1
del /Q devtools* > NUL 2>&1
goto done

:done
echo.
pause
