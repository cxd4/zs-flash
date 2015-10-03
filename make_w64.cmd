@ECHO OFF

set target=amd64
set MSVC=%USERPROFILE%\cmsc\msvc\bin\%target%
set libs=/LIBPATH:"%MSVC%\..\..\lib\%target%"
set include=/I"%MSVC%\..\..\include\crt"

set src=%CD%
set output=/OUT:zs.exe
REM set C_FLAGS=/Wall /GL /O1 /Os /Oi /Ob1 /GS- /TP
set C_FLAGS=/Wall /GL /O1 /Os /Oi /Ob1 /GS- /TC

set files=%src%\main.c %src%\errors.c %src%\flash_io.c %src%\zs_data.c

%MSVC%\cl.exe /MD %include% %C_FLAGS% %files% %EXTRA_LIBS% /link %output% %libs%

pause
