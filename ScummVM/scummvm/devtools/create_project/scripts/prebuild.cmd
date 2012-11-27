@echo off

REM ---------------------------------------------------------------
REM -- Pre-Build Script
REM ---------------------------------------------------------------
REM
REM Generate file with revision number
REM
REM Expected parameters
REM    Root folder (the source root folder)
REM    Target folder (the build output folder, will be used to copy internal_revision.h)

if "%~1"=="" goto error_root
if "%~2"=="" goto error_target

REM Run the revision script
@call cscript "%~1/devtools/create_project/scripts/revision.vbs" "%~1" "%~2" 1>NUL
if not %errorlevel% == 0 goto error_script
goto done

:error_root
echo Invalid root folder (%~1)!
goto done

:error_target
echo Invalid target folder (%~2)!
goto done

:error_script:
echo An error occured while running the revision script!

:done
exit /B0
