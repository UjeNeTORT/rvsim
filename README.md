# RVSim - RISCV32i functional simulator

## Install and build

Follow these steps to install the project

### 0. Prerequisites

See [`.github/workflows/workflow.yml`](https://github.com/UjeNeTORT/rvsim/blob/main/.github/workflows/workflow.yml)
for the list of prereqs.

### 1. Clone the repository

```bash
git clone https://github.com/UjeNeTORT/rvsim
cd rvsim
git submodule update --init --recursive
```

### 2. Build the project

... using script
```bash
cd rvsim
chmod +x build.sh
./build.sh build_sh clang Release
```

*You can also build manually following the steps from the script if you need something specific*

### 3. Test everything (optional)

```bash
cd build_sh
./test
```

### 4. Run simulator on some examples

```bash
./rvsim --elf ../test/elf/echo/echo.elf

Simulator reads ELF using [ELFIO library](https://github.com/serge1/ELFIO).
