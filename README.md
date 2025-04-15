# MiCra  
**Memory Interface Cache for Random Access**

A lightweight caching layer between MRAM and WRAM for the UPMEM PiM systems.

## Overview

MiCra is a cache management system designed for Processing-in-Memory (PiM) architectures based on DPUs. It facilitates efficient data movement between the DPU's volatile **MRAM** and high-speed **WRAM** scratchpad. Since the compute unit can only access WRAM directly, MiCra aims to reduce redundant MRAM accesses by managing cached blocks in WRAM.

## Directory Structure

- `app/` – Application examples or benchmarks using MiCra.
- `src/cpu/` – Host-side logic interfacing with the DPU.
- `src/dpu/` – DPU-side code implementing the cache and memory coordination.

## Features

- Transparent handling of MRAM ↔ WRAM data transfers via DMA.
- Reduction of redundant memory accesses to optimize performance.
- Modular architecture for seamless integration into diverse PiM applications.
- Support for configurable caching strategies (e.g., LRU, direct-mapped).

## Getting Started