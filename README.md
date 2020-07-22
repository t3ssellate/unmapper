# Unmapper

An automatic tool for fixing dumped PE files for decompilation. Especially useful in malware analysis.

## What does it do?

Unmapper takes a memdump of a PE file that has been loaded to memory, a common case with malware loaders.
It modifies the PE's headers such that a decompiler, or other static analysis tools, will be able
to load the file correctly and without errors.

## Usage

`$ unmapper.exe <filename>`
