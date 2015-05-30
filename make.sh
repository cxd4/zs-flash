mkdir -p obj
src="."

FLAGS_C="-masm=intel -Os -std=c89 -pedantic -ansi -Wall"
OBJ_LIST="obj/main.o obj/errors.o obj/flash_io.o"

echo Compiling C sources...
cc -S $FLAGS_C -o obj/main.asm          $src/main.c
cc -S $FLAGS_C -o obj/errors.asm        $src/errors.c
cc -S $FLAGS_C -o obj/flash_io.asm      $src/flash_io.c

echo Assembling compiled sources...
as -o obj/main.o                        obj/main.asm
as -o obj/errors.o                      obj/errors.asm
as -o obj/flash_io.o                    obj/flash_io.asm

echo Linking assembled objects...
cc -s -o zs $OBJ_LIST
