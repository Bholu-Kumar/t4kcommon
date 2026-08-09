# SDL3 Migration Guide & Documentation

This document provides a comprehensive technical overview of the migration of the `t4kcommon` library from legacy **SDL 1.2** to **SDL3**.

---

## 1. Architectural Changes

### Display Architecture: Surface Model $\rightarrow$ Window & Renderer Model
In SDL 1.2, rendering targeted a primary software screen surface (`SDL_Surface* screen`). In SDL3, display management is handled through explicit `SDL_Window` and `SDL_Renderer` objects.

* **Global Surface Accessor Deprecation:** The global `screen` variable and legacy `T4K_GetScreen()` surface calls have been supplemented with modern window and renderer getters.
* **New Public APIs in `t4k_common.h`:**
  ```c
  SDL_Window*   T4K_GetWindow(void);
  SDL_Renderer* T4K_GetRenderer(void);
  void          T4K_PresentScreen(void);
  ```

---

## 2. File-by-File Technical Breakdown

### `src/t4k_common.h`
* Updated core headers to include `<SDL3/SDL.h>`, `<SDL3_image/SDL_image.h>`, and `<SDL3_mixer/SDL_mixer.h>`.
* Added public prototypes for `T4K_GetWindow()`, `T4K_GetRenderer()`, and `T4K_PresentScreen()`.
* Added macro compatibility layer for pixel format queries: `SDL_FreeSurface`, `SDL_MapRGB`, `SDL_MapRGBA`, `SDL_GetRGB`, `SDL_GetRGBA`.
* Updated `T4K_WaitForEvent` parameter signature from `SDL_EventMask` to `Uint32 event_type_mask`.

### `src/t4k_sdl.c`
* **Header Hygiene:** Added missing standard C library headers `<stdio.h>` and `<stdlib.h>`.
* **Pixel Format Handling:** Refactored `T4K_Blend()` and `T4K_zoom()` to process `SDL_PixelFormat` as an `enum` rather than a struct pointer, using `SDL_BITSPERPIXEL()` and `SDL_BYTESPERPIXEL()` macros.
* **Text Rendering (SDL3_ttf):**
  * Updated surface rendering calls from `TTF_RenderUTF8_Blended` to `TTF_RenderText_Blended(font, text, length, color)`.
  * Updated font dimension queries from `TTF_SizeUTF8` to `TTF_GetStringSize(font, text, length, &w, &h)`.
* **Event Polling:** Refactored `T4K_WaitForEvent()` to loop with `SDL_PollEvent()` and mask matching against event types.

### `src/t4k_loaders.c`
* Replaced legacy surface creation `SDL_CreateRGBSurface()` with `SDL_CreateSurface(w, h, format)`.
* Replaced `SDL_DisplayFormat()` and `SDL_DisplayFormatAlpha()` calls with `SDL_ConvertSurface(surf, SDL_PIXELFORMAT_RGBA32)`.
* Updated colorkey and blend mode setting to use `SDL_SetSurfaceColorKey()` and `SDL_SetSurfaceBlendMode()`.

### `src/t4k_menu.c`
* **Mouse Wheel Event Handling:** Migrated mouse wheel handling from `SDL_MOUSEBUTTONDOWN` (buttons 4/5) to dedicated `SDL_EVENT_MOUSE_WHEEL` events.
* **Event Constants:** Modernized event constants to SDL3 names (`SDL_EVENT_QUIT`, `SDL_EVENT_KEY_DOWN`, `SDL_EVENT_MOUSE_BUTTON_DOWN`).
* **Input Grabbing:** Replaced legacy `SDL_WM_GrabInput()` with `SDL_SetWindowMouseGrab(T4K_GetWindow(), SDL_FALSE)`.

### `src/t4k_main.c`
* Replaced `SDLKey` with `SDL_Keycode` and updated event field access from `event->key.keysym.sym` to `event->key.key`.
* Updated audio teardown from legacy `Mix_QuerySpec` loop to `Mix_CloseAudio()`.

### `src/t4k_tts.c`
* **Thread Management:** Replaced `SDL_KillThread()` with safe thread join `SDL_WaitThread()`.
* **Thread Creation:** Updated `SDL_CreateThread()` to pass the required thread name string (`"tts_worker"`).
* **Fallback Stubs:** Added `#else` stub implementations for all `T4K_Tts_*` functions to support environments without Speech Dispatcher or eSpeak headers.

### `src/t4k_test.c`
* Added standard I/O includes (`<stdio.h>`, `<stdlib.h>`, `<string.h>`).
* Fixed command-line flag handling for non-interactive test mode.

### `cmake-modules/Files.cmake` & `src/CMakeLists.txt`
* Added `${T4K_SRC_ROOT}/t4k_tts.c` to `T4K_COMMON_SOURCES`.
* Configured `link_directories(/usr/local/lib)` to ensure system linking resolves custom SDL3 library installations first.

---

## 3. SDL 1.2 $\rightarrow$ SDL 3 API Mapping Reference

| Legacy SDL 1.2 Call | SDL 3 Equivalent |
|---|---|
| `SDL_SetVideoMode(w, h, bpp, flags)` | `SDL_SetWindowSize(window, w, h)` / `SDL_SetWindowFullscreen(window, flags)` |
| `SDL_GetVideoSurface()` | Removed. Target `SDL_Window*` + `SDL_Renderer*` |
| `SDL_Flip(screen)` | `SDL_RenderPresent(renderer)` |
| `SDL_UpdateRect(screen, x, y, w, h)` | `SDL_RenderPresent(renderer)` |
| `SDL_CreateRGBSurface(flags, w, h, bpp, r, g, b, a)` | `SDL_CreateSurface(w, h, format)` |
| `SDL_DisplayFormat(surf)` | `SDL_ConvertSurface(surf, SDL_PIXELFORMAT_RGB24)` |
| `SDL_DisplayFormatAlpha(surf)` | `SDL_ConvertSurface(surf, SDL_PIXELFORMAT_RGBA32)` |
| `SDL_SetAlpha(surf, flags, alpha)` | `SDL_SetSurfaceBlendMode(surf, blend_mode)` |
| `SDL_LowerBlit(...)` | `SDL_BlitSurface(...)` |
| `SDL_WM_GrabInput(...)` | `SDL_SetWindowMouseGrab(window, bool)` |
| `TTF_RenderUTF8_Blended(font, text, color)` | `TTF_RenderText_Blended(font, text, length, color)` |
| `TTF_SizeUTF8(font, text, &w, &h)` | `TTF_GetStringSize(font, text, length, &w, &h)` |
| `TTF_GlyphMetrics(...)` | `TTF_GetGlyphMetrics(...)` |
| `SDL_KillThread(thread)` | `SDL_WaitThread(thread, status)` |

---

## 4. Verification & Testing

The migrated library builds cleanly and passes all test suites:
1. `libt4k_common.so` compiles with zero errors under GCC/Clang with standard C99 flags.
2. `t4k_test` executable links and passes runtime library initialization.

---

## 5. Build Prerequisites & Setup on New Devices

When cloning and building `t4kcommon` or building SDL3 dependencies on a clean environment or new device, follow these setup requirements:

### System Dependencies (Debian / Ubuntu)

```bash
sudo apt update
sudo apt install -y build-essential cmake pkg-config libxml2-dev librsvg2-dev libpng-dev libxtst-dev
```

> [!IMPORTANT]
> **SDL3 X11 XTEST Dependency (`libxtst-dev`):**
> If building SDL3 from source on Linux/X11 systems, CMake will fail if `libxtst-dev` is missing (`Couldn't find dependency package for XTEST`). Install `libxtst-dev` or configure the SDL3 build with `-DSDL_X11_XTEST=OFF`.

### Building `t4kcommon`

From the root of the repository:

```bash
# Configure and build
cmake -B build -S .
cmake --build build

# Install (optional)
sudo cmake --install build
```

