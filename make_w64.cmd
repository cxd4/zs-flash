@ECHO OFF

set MinGW=C:/msys64/mingw64
set src=%CD%
set obj=%src%/obj

set FLAGS_C=-masm=intel -Os -std=c89 -pedantic -ansi -Wall
set OBJ_LIST=%obj%/main.o %obj%/errors.o %obj%/flash_io.o %obj%/zs_data.o

if not exist obj (
mkdir obj
)
cd %MinGW%/bin

echo Compiling C sources...
%MinGW%/bin/gcc.exe -S %FLAGS_C% -o %obj%/main.asm      %src%/main.c
%MinGW%/bin/gcc.exe -S %FLAGS_C% -o %obj%/errors.asm    %src%/errors.c
%MinGW%/bin/gcc.exe -S %FLAGS_C% -o %obj%/zs_data.asm   %src%/zs_data.c
%MinGW%/bin/gcc.exe -S %FLAGS_C% -o %obj%/flash_io.asm  %src%/flash_io.c

echo Assembling compiled sources...
%MinGW%/bin/as.exe               -o %obj%/main.o        %obj%/main.asm
%MinGW%/bin/as.exe               -o %obj%/errors.o      %obj%/errors.asm
%MinGW%/bin/as.exe               -o %obj%/zs_data.o     %obj%/zs_data.asm
%MinGW%/bin/as.exe               -o %obj%/flash_io.o    %obj%/flash_io.asm

echo Linking assembled objects...
rem ld -s -o %src%/zs lib/crt2.o -lmingw32 -lgcc -lmingwex -lmsvcrt -lkernel32
%MinGW%/bin/gcc.exe -s -o %src%/zs.exe %OBJ_LIST%

pause
