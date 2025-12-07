# RVSim - RISCV32i functional simulator

## Install and build

Follow these steps to install the project

### 0. Prerequisites

See [`.github/workflows/workflow.yml`](https://github.com/UjeNeTORT/rvsim/blob/main/.github/workflows/workflow.yml)
for the list of prereqs.

### 1. Clone the repository

```bash
git clone https://github.com/UjeNeTORT/rvsim --recursive
cd rvsim
```

### 2. Pull docker image

```bash
docker pull ujenetort/rv32_interpreter:latest
```

### 3. Build the project

```bash
docker run --rm -it -v $(pwd):/rv32 ujenetort/rv32_interpreter:latest bash
```

### 4. Test (optional)

```bash
cd build
./test
```

### 5. Run simulator on some examples

```bash
./rvsim --elf ../test/elf/echo/echo.elf
