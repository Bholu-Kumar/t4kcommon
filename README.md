# t4kcommon (Tux4Kids Common Library - SDL3)

`t4kcommon` is the shared core C library for Tux4Kids applications, including **TuxType** and **TuxMath**. This repository has been fully upgraded to **SDL3**.

---

## Prerequisites

Ensure you have the following packages and dependencies installed on your Linux system:

```bash
# Build Tools & Core Libraries (Debian / Ubuntu / Linux Mint)
sudo apt-get update
sudo apt-get install -y build-essential cmake pkg-config libxml2-dev librsvg2-dev libpng-dev libespeak-ng-dev
```

### Required SDL3 Satellite Libraries
- `sdl3`
- `sdl3-image`
- `sdl3-ttf`
- `sdl3-mixer`
- `sdl3-net` (optional)

---

## Build & Installation Guide

### Step 1: Clone the Repository
```bash
git clone https://github.com/Midhun-M-git/t4kcommon.git
cd t4kcommon
```

### Step 2: Build with CMake
```bash
# Create build directory
mkdir -p build && cd build

# Configure build with installation prefix
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local

# Compile shared library
make -j$(nproc)
```

### Step 3: Install
```bash
# Install shared library and public headers
sudo make install
sudo ldconfig
```

---

## Technical Highlights
- **SDL3 Migration**: Modernized graphics pipeline moving away from legacy SDL 1.2 surfaces.
- **TTS Synthesis**: Integrated `espeak-ng` text-to-speech with locale fallback.
- **Multi-language Support**: UTF-8 and multibyte string handling.
