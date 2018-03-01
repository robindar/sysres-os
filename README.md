# Systèmes et Réseaux - Projet S2 #

Projet Rock'n Roll : OS Pour Raspberry Pi en C++

## Dependencies ##

- [Qemu](https://www.qemu.org/) - Open source machine emulator and virtualizer
- [GCC](https://gcc.gnu.org/) - the GNU Compiler Collection


## Installation ##

Create a file **tools.mk** with the paths to gcc, objcopy and qemu

```bash
./configure
make
```

Different paths for each tool can be specified with **--TOOL=PATH**
```
./configure --gcc=/usr/bin/aarch64-linux-gnu-gcc --qemu=/usr/bin/qemu-system-aarch64
```

By default, **which** will be used to find those paths if not specified

You can then launch the OS with the **run** target

```bash
make run
```

## Development ##

The default and *run* targets should be enough for all regular tasks.

If necessary, the following *make* targets are available
- **make**, by default, make binary file for kernel
- **make clean**, to clean temporary build files
- **make mrproper**, to remove all non-source files

## Versioning ##

We use [SemVer](https://semver.org/) for versioning.

See the list of tags for available versions

## Authors ##

 - **Waïss AZIZIAN**
 - **David ROBIN**

## License ##

This project is licensed under the GNU GPL v3 License - see [LICENSE.txt](LICENSE.txt) for details

## Acknowledgement ##

Many ideas and pieces of code used in this project were taken from [OS Dev](https://wiki.osdev.org/)

