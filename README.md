# Compiler Translator

This project is a C++ program that reads a JSON file with instruction format rules and creates a JSON file with encoded instructions.

The program works with a small RISC-like instruction system.
It reads information about:

- instruction length
- operands and their sizes
- instruction groups
- instruction names

After that, it generates an output JSON file with:

- instruction name
- fields
- bit positions (`msb`, `lsb`)
- fixed values or `"+"` for operands

## What the program does

The program:

1. reads the input JSON file
2. parses instruction formats and operands
3. builds encoding for every instruction
4. writes the result to the output JSON file

It also supports:

- fixed fields like `F` and `OPCODE`
- operand fields like `R0`, `R1`, `IMM`, `DISP`, `CODE`
- reserved fields like `RES0`
- validation for incorrect input data

## Project structure

Example project structure:

- `src/` — source files
- `include/` — header files
- `main.cpp` — program entry point
- `encode.cpp` — instruction encoding logic
- `dump.cpp` — output and printing logic

## Input

The input file must be a JSON file.

It describes:

- total instruction length
- all possible fields
- instruction groups
- operands for every format

## Output

The output file is also a JSON file.

It contains all encoded instructions with their fields and bit positions.

## Build the program

This project uses **CMake**.

### 1. Create a build directory

```bash
mkdir build
cd build
```

### 2. Run CMake

```bash
cmake ..
```

### 3. Build the program
```bash
make
```
### 4. Running example
```bash
./compilator_translator ../test/input.json ../test/output.json
```
