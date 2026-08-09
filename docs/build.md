# Building t4kcommon (SDL3 Branch)

## Prerequisites & Dependencies

### 1. System Packages & Tools

Run the following command to install required build tools and core dependencies:

```bash
sudo apt update
sudo apt install -y build-essential cmake pkg-config gettext libtool \
                    librsvg2-dev libxml2-dev libespeak-ng-dev
```

---

### 2. Installing SDL3 & Satellite Libraries

If your package manager does not provide SDL3 packages, install SDL3 and satellite libraries from source:

#### A. SDL3 Core
```bash
git clone https://github.com/libsdl-org/SDL.git -b main
cd SDL
cmake -B build
cmake --build build
sudo cmake --install build
cd ..
```

#### B. SDL3_image
```bash
git clone https://github.com/libsdl-org/SDL_image.git -b main
cd SDL_image
cmake -B build
cmake --build build
sudo cmake --install build
cd ..
```

#### C. SDL3_ttf
```bash
git clone https://github.com/libsdl-org/SDL_ttf.git -b main
cd SDL_ttf
cmake -B build
cmake --build build
sudo cmake --install build
cd ..
```

#### D. SDL3_mixer
```bash
git clone https://github.com/libsdl-org/SDL_mixer.git -b main
cd SDL_mixer
cmake -B build
cmake --build build
sudo cmake --install build
cd ..
```

#### E. SDL3_net
```bash
git clone https://github.com/libsdl-org/SDL_net.git -b main
cd SDL_net
cmake -B build
cmake --build build
sudo cmake --install build
cd ..
```

*Note: Run `sudo ldconfig` after installing shared libraries to update system linker caches.*

```bash
sudo ldconfig
```

---

## Build & Install `t4kcommon`

Using **CMake**:

```bash
# 1. Create build directory
mkdir -p build
cd build

# 2. Configure project
PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH cmake ..

# 3. Compile library and test suite
cmake --build .

# 4. Install library to system
sudo cmake --install .
```
