# zs-flash
N64 flash RAM editor for _Legend of Zelda:  Majora's Mask_.

All user information and guides to using this program can be gathered from the [wiki](https://github.com/cxd4/zs-flash/wiki).

## Compiling and Building

For most non-Windows systems, a primitive, easy kind of `make` can be performed via:
```shell
cd zs-flash/
./make.sh
```

If compiling and linking on a Windows operating system, the provided `make_w64.cmd` is the recommended way of building clean, small binaries that should work out of the box on every version since Windows XP 64-bit using the old `msvcrt.dll`.  Simply double-clicking this batch script should build right away on Windows, provided that the Microsoft [WDK](https://www.microsoft.com/en-us/download/confirmation.aspx?id=11800) is installed from the ISO archive contents.

It is also possible to build on Windows using the same two commands specified above for building on Unix-based systems, provided that they are run within Git or MSYS shells, which should have set up the appropriative environment path variables in the Windows configuration for system compiler directories using MinGW port of GCC.
