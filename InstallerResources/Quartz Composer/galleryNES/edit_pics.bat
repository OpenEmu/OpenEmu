@ECHO off
cls
:start
ECHO 0. Edit Picture #0
ECHO 1. Edit Picture #1
ECHO 2. Edit Picture #2
ECHO 3. Edit Picture #3
ECHO 4. Edit Picture #4
ECHO 5. Edit Picture #5
ECHO 6. Edit Picture #6
ECHO 7. Edit Picture #7
ECHO 8. Edit Picture #8
ECHO 9. Edit Picture #9
set choice=
set /p choice=Choose your destiny:
if not '%choice%'=='' set choice=%choice:~0,1%
if '%choice%'=='0' goto 0
if '%choice%'=='1' goto 1
if '%choice%'=='2' goto 2
if '%choice%'=='3' goto 3
if '%choice%'=='4' goto 4
if '%choice%'=='5' goto 5
if '%choice%'=='6' goto 6
if '%choice%'=='7' goto 7
if '%choice%'=='8' goto 8
if '%choice%'=='9' goto 9
ECHO "%choice%" is not valid. Please try again.
ECHO.
goto start
:0
name geo.chr geo.pal pic0.nam
goto end
:1
name geo.chr geo.pal pic1.nam
goto end
:2
name geo.chr geo.pal pic2.nam
goto end
:3
name geo.chr geo.pal pic3.nam
goto end
:4
name geo.chr geo.pal pic4.nam
goto end
:5
name geo.chr geo.pal pic5.nam
goto end
:6
name geo.chr geo.pal pic6.nam
goto end
:7
name geo.chr geo.pal pic7.nam
goto end
:8
name geo.chr geo.pal pic8.nam
goto end
:9
name geo.chr geo.pal pic9.nam
goto end
:end


