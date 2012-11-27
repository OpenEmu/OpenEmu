:: Script for migrating saved games in Windows 2000/XP/Vista/7
::
:: Put this batch file into the ScummVM directory
:: This script will copy any saved games located in the
:: old default location, to the new default location.
::
:: (c) 2012 ScummVM Team
::

@echo off
echo ScummVM Saved Games Migration Script
echo The default location for saved games changed
echo in Windows NT4/2000/XP/Vista/7 for ScummVM 1.5.0
echo This script will copy any saved games stored in
echo the old default location, to the new default location
pause

if defined APPDATA goto :test2
echo.
echo Unable to access the Application Data variable!
pause
goto :eof

:test2
if exist "%APPDATA%" goto :test3
echo.
echo Application Data directory doesn't exist!
pause
goto :eof

:test3
if exist "%APPDATA%\ScummVM\" goto :test4
echo.
echo ScummVM Application Data directory doesn't exist!
pause
goto :eof

:test4
if exist "%APPDATA%\ScummVM\Saved Games\" goto :copyfiles
echo.
echo ScummVM Saved Games directory doesn't exist!
pause
goto :eof

:copyfiles
echo Copying ScummVM Saved Games...
xcopy /EXCLUDE:migration.txt /F /-Y * "%APPDATA%\ScummVM\Saved Games"

echo.
echo All saved games have been copied to the new location of "%APPDATA%\ScummVM\Saved Games"
pause
