# Scripts to generate small .bstate files for testing

## Prerequisites
Required for compilation of riscv files

```bash
riscv32-elf-as --version
```

Required for extracting entry points from obj files
```bash
riscv32-elf-objdump --version
```

## Usage

- to generate bstate from assembly

```bash
./testgen.sh <path_to_s> > path_to.bstate
```

- generate bstate from object file (snippy.o.elf for ex.)

```bash
./fromobj.sh MODEL_SIGN.bin regs_zero.bin MEM_SIGN.bin path_to_object_file.o > path_to.bstate 
```


