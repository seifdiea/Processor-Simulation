# Harvard Architecture Processor Simulation

## Overview
This project implements a **simulated pipelined processor** using the C programming language.
The processor design follows **Package 4** specifications and is intended to model the internal
behavior of a simple CPU with a **three-stage pipeline**.

The simulator provides a detailed, cycle-by-cycle visualization of instruction execution,
demonstrating how instructions flow through the **Fetch, Decode, and Execute** stages of the
pipeline.

## Processor Architecture
- **Architecture Type**: Harvard Architecture
- **Pipeline Model**: 3-stage pipeline (Fetch, Decode, Execute)

### Memory Architecture
- **Instruction Memory**
  - 1024 entries
  - Each entry is 16 bits
- **Data Memory**
  - 2048 entries
  - Each entry is 8 bits

### Registers
- **General Purpose Registers (GPRs)**
  - 64 registers (R0 – R63)
  - Each register is 8 bits
- **Program Counter (PC)**
  - 16 bits
- **Status Register (SREG)**
  - 8 bits
  - Flags: Carry (C), Zero (Z), Negative (N), Overflow (V), Sign (S)

## Instruction Set Architecture
- **Instruction Size**: 16 bits
- **Supported Instruction Formats**:
  - R-format
  - I-format
- **Supported Instructions**:
  - ADD, SUB, MUL
  - ANDI, EOR
  - MOVI
  - SAL, SAR
  - LDR, STR
  - BEQZ, BR

Status flags are updated according to instruction execution results.

## Pipeline Execution
- All instructions pass through the three pipeline stages
- Up to three instructions can be in execution simultaneously
- Pipeline execution introduces a one-cycle delay between stages
- Branch instructions may cause the pipeline to be flushed

## Functional Components
- **Instruction Loading**: Reads assembly instructions from a text file
- **Instruction Parsing**: Converts assembly code into binary representation
- **Fetch Stage**: Retrieves instructions from instruction memory and updates PC
- **Decode Stage**: Extracts opcode and operands
- **Execute Stage**: Performs operations, updates registers, memory, and SREG

## Output
During execution, the simulator prints:
- Current clock cycle
- Instruction present in each pipeline stage
- Register and memory updates
- Pipeline flush events when applicable

After execution completes, the simulator outputs:
- Final contents of all general-purpose registers
- Instruction memory contents
- Data memory contents

## How to Run
1. Compile the project using a C compiler
2. Ensure an `instructions.txt` file is present containing the assembly program
3. Run the executable to start the simulation
4. Observe the pipeline behavior and state changes through console output

## Technologies Used
- C
- File I/O
- Bitwise operations
- Data structures for pipeline and memory simulation

## Academic Context
Developed for:
CSEN 601 – Computer Architecture  
German University in Cairo  
Faculty of Media Engineering and Technology
