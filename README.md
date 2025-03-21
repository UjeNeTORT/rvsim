# RVSim - RISCV32i functional simulator

## Install and build

Follow these steps to install the project

### 1. Clone the repository

```bash
git clone https://github.com/UjeNeTORT/rvsim
```

### 2. Build the project

... using script
```bash
cd rvsim
chmod +x build.sh
./build.sh build_sh Release
```

*You can also build manually following the steps from the script if you need something specific*

### 3. Test everything (optional)

```bash
cd build_sh
./test
```

### 4. Run simulator on some examples

```bash
./rvsim --istate=../test/insn/add/001.bstate --ostate=001.bstate
```

You will see something like this:

![first run image](img/first_run.png)


It basically says that it started executing at some PC,
prints a trace of all decoded instructions it executed and upon encountering an unknown one it stops the execution and prints its PC.

(I will rework the exiting logic soon)

### `.bstate` ???
> Let me clarify what `.bstate` is:

`.bstate` is a file format which stores a simulator state in binary format

It can be examined with `xxd` utility, or `hexdump`, or any other hex viewer.

You will see something like this:

![a lot of bytes...](img/xxd_bstate.png)

This file stores:
1. position of PC
2. registers
3. memory state

It also contains some signatures to make it easier to read (each one is 16 bytes) and to protect it to a certain extent.

First signature is a general one,
after it there are 4 bytes for PC value, after PC there is a registers section with its own signature and the last section is a full memory dump.

#### Generating `.bstate` from assembly sourse

Nobody wants to write such a file manually. Luckily, we have a script at `test/scripts/` which is designed to address this issue.

*Lets use it!*

```bash
cd test/scripts/
./testgen.sh ../insn/add/src/001.s > ../../deleteme.bstate
```
The above example compilers `001.s` into
an object file, mixes it up with registers
(*`test/scripts/regs_zero.bin` - all
registers = 0 by default*) and adds
signatures. The result is `deleteme.bstate` file
which can be run after that on simulator. Nasty workaround while ELFs are not supported.

I want to support ELF soon in order not to mess up with such files, but i will not delete them as they are still very usefull for testing and for recording checkpoints of execution.


