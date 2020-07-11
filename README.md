# Educational toolchain: assembler, linker and emulator
## School of Electrical Engineering, University of Belgrade, 2018.

## Brief project description
This project is a Software Engineering Program 3rd year System Software
course project assignment. It is an educational project to implement a
CPU emulator for a specified RISC architecture
and a toolchain for it, consisting of an assembler and a linker.

Read full project description [here](./doc/pdf/doc.pdf) (`./doc/pdf/doc.pdf`).

***

## Quick installation
In order to compile and install an entire toolchain, execute the
following commands:
```
$ git clone https://github.com/cicovic-andrija/ETF-System-Software.git
$ mkdir -p ~/bin/
$ make
```

***

## Separate installation of each tool
It is possible to install each tool separately, using an appropriate
`make` rule:
```
$ git clone https://github.com/cicovic-andrija/ETF-System-Software.git
$ mkdir -p ~/bin/
$ make assembler
$ make linker
$ make emulator
```
This can also be accomplished by executing `make` command from
within each tool's directory. For example, to build a linker, execute:
```
$ git clone https://github.com/cicovic-andrija/ETF-System-Software.git
$ cd linker/
$ make
```

***

## Usage

### Assembler
```
$ ass [-o file] [-t file] [-l file] [-a addr] [-h] input_file
```
|Option |Explanation                                  |
|-------|---------------------------------------------|
|-o file|Specify relocatable object output binary file|
|-t file|Specify relocatable object output text file  |
|-l file|Specify log file                             |
|-a addr|Define load address for sections             |
|-h     |Print help message and exit                  |

### Linker
```
$ lnk [-o file] [-t file] [-l file] [-i] [-h] object_file...
```
|Option |Explanation                                            |
|-------|-------------------------------------------------------|
|-o file|Specify executable output binary file                  |
|-t file|Specify executable output text file                    |
|-l file|Specify log file                                       |
|-i     |Ignore section load addresses predefined in input files|
|-h     |Print help message and exit                            |

### Emulator
```
$ emu [exec_file]
```

***

## Documentation & examples
Documentation, which includes full project description, explanation
of the solution and installation instructions,
written in LaTeX and compiled into PDF, can be found under `./doc/`
directory tree. Also, each C header file contains brief descriptive
comments about every exported function. Some example programs, written
in assembly language, together with accompanying Makefiles, can be found
under `./examples` directory tree.

***

