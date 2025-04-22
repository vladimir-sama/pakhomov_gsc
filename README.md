# Pakhomov GSC

Vladimir Alexandre Pakhomov

Concept and Implementation: 2025

**Pakhomov GSC** (Generative Seed Compression) is an experimental file compression concept based on deterministic pseudo-random generation.

The idea is simple: for any file, there exists a seed and a length such that a deterministic pseudo-random number generator (PRNG) will generate the exact bit sequence of the original file. If such a seed and length are found, the file can be represented by just those two numbers.

This project explores that concept in C++ with a fast multithreaded implementation.

## How It Works

1. The program reads a binary file.
2. It converts the file to a vector of bits.
3. Using a seeded PRNG (e.g., Xorshift), it attempts to generate a matching bit sequence of the same length.
4. It searches through seeds in parallel using multiple CPU threads.
5. If a match is found, it outputs:
   - The matching seed
   - The bit length

This effectively "compresses" the file to two numbers: the PRNG seed and the bit length. While practically unfeasible for large files, this is a conceptual exploration of ultimate compression and entropy mapping.

## Use Case

This project is intended for research, theoretical exploration, and experimentation in data entropy, compression theory, and algorithmic generation. It is not a practical compression tool.

## Build Instructions

```bash
g++ -std=c++20 -O3 -pthread -o pakhomov_gsc pakhomov_gsc.cpp
```
