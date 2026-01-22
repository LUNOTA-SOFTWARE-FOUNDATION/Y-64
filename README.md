# The Y-64 architecture

The Y-64 architecture is a 64-bit system-on-chip architecture designed for personal computing with
the goal of providing certain similarities to x86-64 while being far more compact and stripped down.

## Project structure

This repository contains the implementation of the architecture in various forms (i.e., virtual machines,
toolchains, hardware descriptions, etc).

- bios/: Y-64 BIOS sources
- emul/: Emulation / virtual machine
- spec/: Specifications and documentation
- sv/:   System verilog hardware descriptions
- test/: SoC verilog testbenches

## Chip series

- Y-64A: First generation of Y-64 chips, targeting embedded systems [LATEST]
