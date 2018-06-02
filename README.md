# Systèmes et Réseaux - Projet S2 #

Projet Rock'n Roll : OS Pour Raspberry Pi en C

## Dependencies ##

- [Qemu](https://www.qemu.org/) - Open source machine emulator and virtualizer
- [GCC](https://gcc.gnu.org/) - the GNU Compiler Collection


## Installation ##

You need to compile qemu from source with specific flags to support aarch64

```bash
git clone git://git.qemu.org/qemu.git qemu.git
cd qemu.git
./configure --target-list=aarch64-softmmu
make
make install
```

To compile the OS, you also need the *aarch64-linux-gnu* suite, with gcc, ld, and objdump

These should be available for your specific distribution in the official repositories

You can then go to the OS source directory, and configure with **--TOOL=PATH**

```bash
./configure --LOG=V --GDB=aarch64-linux-gnu-gdb
make
```

By default, **which** will be used to find those paths if not specified

You can then launch the OS in QEMU with the **run** target

```bash
make run
```

And build for hardware with

```bash
make deploy
```

## Development ##

The default and *run* targets should be enough for all regular tasks.

If necessary, the following *make* targets are available
- **make**, by default, make binary file for kernel
- **make clean**, to clean temporary build files
- **make clearlogs**, to clean all qemu logs
- **make mrproper**, to remove all non-source files
- **make asm_dump**, to build and print a dump of the binary file produced
- **make deploy**, to build for hardware
- **make gdb**, to build for QEMU with GDB support and wait on port 1234
- **make stub**, to connect GDB to QEMU on port 1234, use with (make gdb)

The makefile also includes the proper checks to ensure that all mandatory
executables are present before trying to compile

Variables set at configure time can also be given as arguments to make
in case you want to override the previously configured defaults

**make run LOG=I** for instance will compile with log level "information", even
if **./configure --LOG=V** was used for verbosity

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

