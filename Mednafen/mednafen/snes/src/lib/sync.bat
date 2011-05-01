rmdir /Q /S nall
rmdir /Q /S ruby

mkdir nall
mkdir ruby
xcopy /E ..\..\..\nall nall
xcopy /E ..\..\..\ruby ruby
