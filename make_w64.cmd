@ECHO OFF
TITLE Windows Driver Kit 7.1.0

REM set target=i386
set target=amd64

set C_LANGUAGE_MODE=/TC
REM set C_LANGUAGE_MODE=/TP

set DDK=C:\WinDDK\7600.16385.1
set MSVC=%DDK%\bin\x86\%target%
set include=/I"%DDK%\inc\crt" /I"%DDK%\inc\api"
set libs=/LIBPATH:"%DDK%\lib\crt\%target%" /LIBPATH:"%DDK%\lib\wnet\%target%"

set src=%CD%
set output=/OUT:zs.exe
set C_FLAGS=/Wall /GL /O1 /Os /Oi /Ob1 /GS- %C_LANGUAGE_MODE% /Fa
set files=%src%\main.c %src%\errors.c %src%\flash_io.c %src%\zs_data.c

%MSVC%\cl.exe /MD %include% %C_FLAGS% %files% /link %output% %libs%
pause
