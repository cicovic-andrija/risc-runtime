# RISC emulator and programming tools

An implementation of an emulator for an abstract RISC instruction set,
and a toolchain consisting of an assembler and a linker that can
compile the specified assembly language and produce a binary file,
ready to be loaded and executed in the emulator.

## Specification

Read the full specification of the project, in Serbian: [ETF_SS](./doc/pdf/doc.pdf)

The specification also contains detailed explanation of the solution.

## Quick build

In order to compile the entire toolchain, execute the following commands:

```
$ git clone https://github.com/cicovic-andrija/virtual-risc-runtime.git
$ mkdir -p ~/bin/
$ make
```

## Build each program separately

It is possible to compile each tool separately, using an appropriate
`make` rule:

```
$ git clone https://github.com/cicovic-andrija/virtual-risc-runtime.git
$ mkdir -p ~/bin/
$ make assembler
$ make linker
$ make emulator
```

This can also be accomplished by executing `make` command from
within each tool's directory. For example, to build a linker, execute:

```
$ git clone https://github.com/cicovic-andrija/virtual-risc-runtime.git
$ cd linker/
$ make
```

## Assembler usage

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

## Linker usage

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

## Emulator usage

```
$ emu [exec_file]
```

## Examples

Some example programs, written in assembly language, together with
accompanying Makefiles for building with the toolchain, can be found
under `./examples`.
