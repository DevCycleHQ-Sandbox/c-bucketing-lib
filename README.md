# DevCycle C Bucketing Library
---
This is a wrapper around our existing WASM based bucketing library. The purpose
of this library is to provide support for more languages than those that currently support WASM.

## Setup
We use git submodules for sourcing the wasmtime library, which has some dependencies that are needed.
Check `scripts/brew-install-depends.sh` for the list (assuming developing on OSX).

Once this repo is checked out - run `git submodule update --init --recursive` to update the submodules inside at the root level and cascade down.


## Usage

This library is meant to be a 1:1 passthrough for all the data required to call the 
bucketing library in web assembly.
This assumes that the implementing languages that use this library will handle all the string parsing into language specific datatypes.

## Developing

We use CLion as the main IDE - but any IDE compatible with CMake will work.
