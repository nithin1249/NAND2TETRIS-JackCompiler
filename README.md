# NAND2TETRIS Toolchain (Custom C++ Implementation)

This repository contains a complete custom toolchain for the **NAND2TETRIS** computer systems course, implemented fully in **C++**. The aim is to recreate the entire software stack that drives the Hack computer — from high-level Jack source code down to binary machine code — using clean architecture, modular components, and accurate conformance to the official specifications.

This project demonstrates how a high-level language is gradually lowered into executable machine instructions through multiple well-defined stages. Each stage of the pipeline is implemented as an independent module with its own headers, source files, and test tools.

---

## Overview

This toolchain implements the full Jack-to-Hack compilation pipeline. The architecture is divided into the following major components:

```
 Jack Program
      ↓
 [Tokenizer]  →  Tokens
      ↓
 [Parser]  →  Parse Tree (AST)
      ↓
 [Jack Compiler]  →  VM Code (.vm)
      ↓
 [VM Translator]  →  Hack Assembly (.asm)
      ↓
 [Hack Assembler]  →  Hack Machine Code (.hack)
      ↓
 [Emulator] (WIP) → Execution
```

Each stage takes structured input, transforms it according to the NAND2TETRIS specifications, and provides clean output for the next module.

### Component Summary

* **Tokenizer**: Breaks source code into valid tokens.
* **Parser**: Constructs the program structure based on Jack grammar.
* **Compiler**: Converts the parsed structure into VM instructions.
* **VM Translator**: Converts stack-based VM instructions to Hack Assembly.
* **Assembler**: Converts assembly to 16-bit Hack binary machine code.
* **Emulator (WIP)**: Simulates the Hack CPU and memory system.

---

## Features

* End‑to‑end Jack‑to‑Hack toolchain.
* Clean modular architecture (Tokenizer → Parser → Compiler → VM Translator → Assembler).
* Testable components with standalone executables.
* CLion + CMake build workflow.
* Strict adherence to NAND2TETRIS specifications.
* Error‑handled tokenizer and parser with detailed diagnostics.

---

## Directory Structure

```
project/
├── Compiler/
│   ├── Tokenizer.h
│   ├── Parser.h
│   ├── Compiler.h
│   └── ...
├── VMTranslator/
│   └── ...
├── Assembler/
│   └── ...
├── Emulator/ (WIP)
│   └── ...
├── tests/
│   └── jack_tokens.cpp
├── CMakeLists.txt
└── README.md
```

---

## Build & Run

### Clone and Build

```bash
git clone <repository-url>
cd <project>
cmake -B build
cmake --build build
```

### Run Tokenizer

```bash
./build/jack_tokens <path-to-jack-file>
```

### Run Parser

```bash
./build/jack_parser <file>
```

Each module builds a small CLI tool for testing.

---

## How the Toolchain Works

### 1. Tokenizer

* Loads the entire `.jack` file into a `std::string`.
* Produces tokens (keywords, symbols, identifiers, integers, strings).
* Implements spec‑correct handling of whitespace and comments.
* Generates errors for illegal characters (e.g., negative integers).

### 2. Parser

* Builds a full Parse Tree according to Jack grammar.
* Implements recursive‑descent parsing.
* Produces structured syntax trees for the compiler.

### 3. Jack → VM Compiler

* Converts Jack class structures into VM functions.
* Handles expressions, control flow, subroutines.
* Matches exact VM output specification.

### 4. VM Translator

* Stack‑based VM instruction translation.
* Memory segments, comparisons, arithmetic, branching.

### 5. Hack Assembler

* Two‑pass assembler generating `.hack` binary files.
* Handles symbols, labels, and A/C-instructions.

### 6. Emulator (In Progress)

* Planned features:

    * RAM + ROM simulation
    * ALU operations
    * Program counter and instruction cycle
    * GUI visualizer later

---

## Goals of the Project

* Demonstrate mastery of low‑level system construction.
* Provide a real project showcasing compiler design, parsing, tokenization, and virtual machine implementation.
* Build a complete, professional‑grade toolchain instead of relying on course‑provided tools.

---

## Future Improvements

* Full Hack CPU emulator.
* Chip-level simulator for logic gates.
* Optimization passes in compilation.
* GUI frontend for code visualization.

---

## License

Specify the license here if applicable.
