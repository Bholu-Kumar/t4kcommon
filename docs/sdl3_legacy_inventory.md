# SDL3 Legacy Implementation Inventory

This document is a precise, file-by-file catalogue of every symbol, pattern,
and API call in the t4kcommon codebase that belongs to SDL 1.2 (or earlier
compatibility shims) and has no direct equivalent under the SDL3 API surface.
Each entry records the source file, exact line number, the legacy construct,
and the SDL3 category it falls under.

The document is structured by subsystem, not by file, so that a developer can
work through one architectural concern at a time.

---

## 1. Video Subsystem - Window and Screen Management

### 1.1 SDL_SetVideoMode (removed in SDL2, absent in SDL3)

| File | Line | Legacy Call |
|------|------|-------------|
| `src/t4k_sdl.c` | 526 | `SDL_SetVideoMode(new_res_x, new_res_y, PIXEL_BITS, SDL_SWSURFACE\|SDL_HWPALETTE)` |
| `src/t4k_sdl.c` | 559 | `SDL_SetVideoMode(window ? win_res_x : fs_res_x, ...)` |

`SDL_SetVideoMode` was the only way to create a rendering surface in SDL 1.2.
SDL3 replaces the entire concept with a `SDL_Window` / `SDL_Renderer` /
`SDL_Texture` pipeline. There is no single-function replacement.

### 1.2 SDL_GetVideoSurface (removed)

| File | Line | Legacy Call |
|------|------|-------------|
| `src/t4k_sdl.c` | 68 | `SDL_GetVideoSurface()` |
| `src/t4k_sdl.c` | 71 | `SDL_GetVideoSurface()` |

This function returned a pointer to the primary display surface. SDL3 has no
primary display surface concept. The window and renderer are distinct objects.

### 1.3 SDL_Flip (removed)

| File | Line | Legacy Call |
|------|------|-------------|
| `src/t4k_sdl.c` | 827 | `SDL_Flip(screen)` |
| `src/t4k_sdl.c` | 836 | `SDL_Flip(screen)` |
| `src/t4k_sdl.c` | 867 | `SDL_Flip(screen)` |
| `src/t4k_sdl.c` | 876 | `SDL_Flip(screen)` |
| `src/t4k_sdl.c` | 920 | `SDL_Flip(screen)` |
| `src/t4k_sdl.c` | 929 | `SDL_Flip(screen)` |
| `src/t4k_menu.c` | 912 | `SDL_Flip(T4K_GetScreen())` |
| `src/t4k_menu.c` | 1068 | `SDL_Flip(T4K_GetScreen())` |

`SDL_Flip` swapped front and back buffers on a hardware-accelerated surface.
SDL3 uses `SDL_RenderPresent(renderer)` as the equivalent operation on an
`SDL_Renderer`.

### 1.4 SDL_UpdateRect and SDL_UpdateRects (removed)

| File | Line | Legacy Call |
|------|------|-------------|
| `src/t4k_sdl.c` | 472 | `SDL_UpdateRect(surf, rect->x, rect->y, rect->w, rect->h)` |
| `src/t4k_sdl.c` | 546 | `SDL_UpdateRect(screen, 0, 0, 0, 0)` |
| `src/t4k_sdl.c` | 584 | `SDL_UpdateRect(screen, 0, 0, 0, 0)` |
| `src/t4k_sdl.c` | 1156 | `SDL_UpdateRects(screen, numupdates, dstupdate)` |
| `src/t4k_menu.c` | 511 | `SDL_UpdateRect(T4K_GetScreen(), 0, 0, 0, 0)` |
| `src/t4k_menu.c` | 863 | `SDL_UpdateRect(T4K_GetScreen(), tmp_rect.x, tmp_rect.y, tmp_rect.w, tmp_rect.h)` |
| `src/t4k_menu.c` | 889 | `SDL_UpdateRect(T4K_GetScreen(), tmp_rect.x, tmp_rect.y, tmp_rect.w, tmp_rect.h)` |
| `src/t4k_menu.c` | 1482 | `SDL_UpdateRect(T4K_GetScreen(), 0, 0, 0, 0)` |

These functions notified SDL to refresh dirty regions of a software surface.
SDL3 uses the renderer pipeline where `SDL_RenderPresent` handles the full
screen refresh. Partial-region update optimisation is done differently.

### 1.5 SDL_FULLSCREEN flag (removed)

| File | Line | Legacy Pattern |
|------|------|----------------|
| `src/t4k_sdl.c` | 524 | `screen->flags & SDL_FULLSCREEN` |
| `src/t4k_sdl.c` | 556 | `screen->flags & SDL_FULLSCREEN` |
| `src/t4k_sdl.c` | 562 | `screen->flags ^ SDL_FULLSCREEN` |

SDL3 window flags use `SDL_WINDOW_FULLSCREEN`. Querying fullscreen state is
done with `SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN`.

### 1.6 Window Manager Grab (removed)

| File | Line | Legacy Call |
|------|------|-------------|
| `src/t4k_menu.c` | 513 | `SDL_WM_GrabInput(SDL_GRAB_OFF)` |

`SDL_WM_GrabInput` is entirely removed in SDL3. Its replacement is
`SDL_SetWindowMouseGrab(window, SDL_FALSE)`.

---

## 2. Surface Creation and Format Conversion

### 2.1 SDL_SWSURFACE flag (removed)

| File | Line | Legacy Usage |
|------|------|--------------|
| `src/t4k_sdl.c` | 130 | `SDL_CreateRGBSurface(SDL_SWSURFACE\|SDL_SRCALPHA, ...)` |
| `src/t4k_sdl.c` | 276 | `SDL_CreateRGBSurface(SDL_SWSURFACE, ...)` |
| `src/t4k_sdl.c` | 392 | `SDL_ConvertSurface(S1, fmt1, SDL_SWSURFACE)` |
| `src/t4k_sdl.c` | 648 | `SDL_CreateRGBSurface(src->flags, ...)` |
| `src/t4k_sdl.c` | 1396 | `SDL_CreateRGBSurface(SDL_SWSURFACE, ...)` |
| `src/t4k_menu.c` | 1129 | `SDL_CreateRGBSurface(SDL_SWSURFACE\|SDL_SRCALPHA, ...)` |
| `src/t4k_loaders.c` | 325 | `SDL_CreateRGBSurface(SDL_SWSURFACE \| SDL_SRCALPHA, ...)` |

`SDL_SWSURFACE` and `SDL_HWSURFACE` flags are gone. `SDL_CreateRGBSurface` in
SDL3 takes `0` as the flags argument. The function signature itself also changed
to `SDL_CreateSurface(width, height, format)` where format is an `SDL_PixelFormat`
enum value.

### 2.2 SDL_HWPALETTE flag (removed)

| File | Line | Legacy Usage |
|------|------|--------------|
| `src/t4k_sdl.c` | 529 | `SDL_SWSURFACE\|SDL_HWPALETTE` |

Palette management was overhauled. This flag no longer exists in SDL3.

### 2.3 SDL_DisplayFormat and SDL_DisplayFormatAlpha (removed)

| File | Line | Legacy Call |
|------|------|-------------|
| `src/t4k_sdl.c` | 318 | `SDL_DisplayFormat(out)` |
| `src/t4k_sdl.c` | 325 | `SDL_DisplayFormatAlpha(out)` |
| `src/t4k_sdl.c` | 329 | `SDL_DisplayFormat(out)` |
| `src/t4k_sdl.c` | 443 | `SDL_DisplayFormatAlpha(tmpS)` |
| `src/t4k_sdl.c` | 1449 | `SDL_DisplayFormatAlpha(bg)` |
| `src/t4k_loaders.c` | 624 | `SDL_DisplayFormat(img)` |
| `src/t4k_loaders.c` | 630 | `SDL_DisplayFormatAlpha(img)` |
| `src/t4k_loaders.c` | 639 | `SDL_DisplayFormat(img)` |
| `src/t4k_loaders.c` | 670 | `SDL_DisplayFormat(orig)` |

These functions converted a surface to the screen's native pixel format for
fast blitting. SDL3 replaces both with `SDL_ConvertSurface(surface, format)`
where the format is now an `SDL_PixelFormat` enum value, not a pointer to
`SDL_PixelFormat` struct. There is no longer a concept of a "display format"
because rendering goes through textures.

### 2.4 SDL_SRCCOLORKEY and SDL_SRCALPHA surface flags (removed)

| File | Line | Legacy Usage |
|------|------|--------------|
| `src/t4k_sdl.c` | 263 | `flags & SDL_SRCCOLORKEY` |
| `src/t4k_sdl.c` | 264 | `in->flags &= ~SDL_SRCCOLORKEY` |
| `src/t4k_sdl.c` | 267 | `flags & SDL_SRCALPHA` |
| `src/t4k_sdl.c` | 268 | `in->flags &= ~SDL_SRCALPHA` |
| `src/t4k_sdl.c` | 315 | `flags & SDL_SRCCOLORKEY` |
| `src/t4k_sdl.c` | 316 | `in->flags \|= SDL_SRCCOLORKEY` |
| `src/t4k_sdl.c` | 321 | `out->flags \|= SDL_SRCCOLORKEY` |
| `src/t4k_sdl.c` | 323 | `flags & SDL_SRCALPHA` |
| `src/t4k_sdl.c` | 324 | `in->flags \|= SDL_SRCALPHA` |

These flags are absent from SDL3. SDL3 surfaces have alpha blending enabled by
default. Color keying is set via `SDL_SetSurfaceColorKey(surface, SDL_TRUE, key)`.
Alpha blending mode is set via `SDL_SetSurfaceBlendMode`.

### 2.5 SDL_SetAlpha (removed)

| File | Line | Legacy Call |
|------|------|-------------|
| `src/t4k_loaders.c` | 669 | `SDL_SetAlpha(orig, SDL_RLEACCEL, SDL_ALPHA_OPAQUE)` |

`SDL_SetAlpha` is removed. SDL3 provides `SDL_SetSurfaceAlphaMod(surface, alpha)`
to set a per-surface alpha modulator. `SDL_RLEACCEL` as an argument is also gone.

### 2.6 SDL_RLEACCEL flag (removed)

| File | Line | Legacy Usage |
|------|------|--------------|
| `src/t4k_sdl.c` | 1448 | `SDL_SetColorKey(bg, SDL_SRCCOLORKEY\|SDL_RLEACCEL, color_key)` |
| `src/t4k_loaders.c` | 637 | `SDL_SetColorKey(img, (SDL_SRCCOLORKEY \| SDL_RLEACCEL), ...)` |

RLE encoding as a surface flag is removed. `SDL_SetColorKey` signature in
SDL3 is `SDL_SetSurfaceColorKey(surface, SDL_bool enabled, Uint32 key)`.

### 2.7 SDL_Surface->colorkey field (removed)

| File | Line | Legacy Usage |
|------|------|--------------|
| `src/t4k_sdl.c` | 265 | `colorkey = in->format->colorkey` |
| `src/t4k_sdl.c` | 317 | `in->format->colorkey = colorkey` |
| `src/t4k_sdl.c` | 322 | `out->format->colorkey = colorkey` |

The `colorkey` field was embedded directly in `SDL_PixelFormat`. SDL3 moves this
to an opaque internal state. Use `SDL_GetSurfaceColorKey(surface, &key)` to
read it back.

### 2.8 SDL_Surface->flags direct access (removed pattern)

| File | Line | Legacy Usage |
|------|------|--------------|
| `src/t4k_sdl.c` | 259 | `flags = in->flags` |
| `src/t4k_sdl.c` | 524 | `screen->flags & SDL_FULLSCREEN` |
| `src/t4k_sdl.c` | 556 | `screen->flags & SDL_FULLSCREEN` |
| `src/t4k_sdl.c` | 562 | `screen->flags ^ SDL_FULLSCREEN` |
| `src/t4k_sdl.c` | 648 | `SDL_CreateRGBSurface(src->flags, ...)` (copying flags) |

SDL3 surfaces still have a public `flags` field but many of the old flag
constants are gone. Code relying on surface flags to detect capabilities
must be rewritten using the dedicated query functions.

---

## 3. Low-Level Blit Operations

### 3.1 SDL_LowerBlit (removed)

| File | Line | Legacy Call |
|------|------|-------------|
| `src/t4k_sdl.c` | 1126 | `SDL_LowerBlit(blits[i].src, blits[i].srcrect, screen, blits[i].dstrect)` |

`SDL_LowerBlit` was an internal, unchecked blit that bypassed clipping. SDL3
does not expose this function. All blits go through `SDL_BlitSurface` or the
renderer API.

---

## 4. Event System

### 4.1 SDL_EventMask and SDL_EVENTMASK macro (removed)

| File | Line | Legacy Usage |
|------|------|--------------|
| `src/t4k_common.h` | 957 | `SDL_EventType T4K_WaitForEvent(SDL_EventMask events)` |
| `src/t4k_sdl.c` | 603 | `SDL_EventType T4K_WaitForEvent(SDL_EventMask events)` |
| `src/t4k_sdl.c` | 610 | `SDL_EVENTMASK(evt.type) & events` |

`SDL_EventMask` is a removed type. SDL3 uses `SDL_EventType` directly.
The `SDL_EVENTMASK()` macro for converting an event type to a bitmask is also
removed. Event filtering in SDL3 is done with `SDL_AddEventWatch()` or by
comparing `event.type` directly.

### 4.2 SDLKey type (removed)

| File | Line | Legacy Usage |
|------|------|--------------|
| `src/t4k_main.c` | 129 | `SDLKey key = event->key.keysym.sym` |

`SDLKey` is renamed `SDL_Keycode` in SDL2 and continues under that name in SDL3.

### 4.3 SDL_BUTTON_WHEELUP and SDL_BUTTON_WHEELDOWN (removed)

| File | Line | Legacy Usage |
|------|------|--------------|
| `src/t4k_menu.c` | 598 | `event.button.button == SDL_BUTTON_WHEELUP` |
| `src/t4k_menu.c` | 615 | `event.button.button == SDL_BUTTON_WHEELDOWN` |

These pseudo-button constants were removed in SDL2. SDL3 delivers mouse wheel
input through `SDL_EVENT_MOUSE_WHEEL` events with the `event.wheel.y` field.

---

## 5. Threading API

### 5.1 SDL_KillThread (removed)

| File | Line | Legacy Call |
|------|------|-------------|
| `src/t4k_tts.c` | 103 | `SDL_KillThread(tts_thread)` |
| `src/t4k_tts.c` | 256 | `SDL_KillThread(tts_thread)` |

`SDL_KillThread` was always dangerous (it killed a thread without cleanup) and
was removed starting in SDL2. SDL3 has no equivalent. The correct approach is
cooperative thread termination: set a shared flag variable that the thread
function checks and returns from voluntarily.

### 5.2 SDL_CreateThread signature change

| File | Line | Legacy Call |
|------|------|-------------|
| `src/t4k_tts.c` | 156 | `SDL_CreateThread(tts_thread_func, &data_to_pass)` |
| `src/t4k_tts.c` | 316 | `SDL_CreateThread(tts_thread_func, &data_to_pass)` |

SDL3 `SDL_CreateThread` requires a name string as the second argument:
`SDL_CreateThread(fn, "thread_name", data)`. The two-argument form does not
compile.

---

## 6. Audio Subsystem (SDL_mixer API changes)

### 6.1 Mix_QuerySpec (signature/behavior changed)

| File | Line | Legacy Usage |
|------|------|-------------|
| `src/t4k_main.c` | 101 | `n_timesopened = Mix_QuerySpec(&frequency, &format, &channels)` |

`Mix_QuerySpec` was deprecated in SDL_mixer 2.x and removed in SDL_mixer 3.x.
In SDL3 / SDL_mixer 3, the audio subsystem is opened once and you query state
through `Mix_GetDevice()` or pass format values directly at open time. The
"times opened" pattern is gone.

---

## 7. Text Rendering (SDL_ttf API changes)

### 7.1 TTF_GlyphMetrics (signature changed)

| File | Line | Legacy Usage |
|------|------|-------------|
| `src/t4k_sdl.c` | 1584 | `TTF_GlyphMetrics(font, t[i], NULL, NULL, NULL, &h, NULL)` |

SDL3_ttf changes the function to `TTF_GetGlyphMetrics(font, ch, &minx, &maxx, &miny, &maxy, &advance)`.
The function name itself changed and the character argument is now a `Uint32`
(Unicode codepoint) rather than a `Uint16`.

### 7.2 TTF_FontAscent (function status)

| File | Line | Legacy Usage |
|------|------|-------------|
| `src/t4k_sdl.c` | 1588 | `TTF_FontAscent(font)` |

`TTF_FontAscent` remains available in SDL3_ttf under the same name. However,
because `TTF_GlyphMetrics` on line 1584 must change, the entire offset
calculation block must be reviewed.

---

## 8. Global screen Surface Pattern (architectural issue)

### 8.1 extern SDL_Surface* screen

| File | Line | Legacy Usage |
|------|------|-------------|
| `src/t4k_common.h` | 171 | `extern SDL_Surface* screen` |
| `src/t4k_sdl.c` | 35 | `SDL_Surface* screen = NULL` |
| Throughout `t4k_sdl.c` | multiple | All direct reads/writes of `screen` |
| Throughout `t4k_menu.c` | multiple | All calls to `T4K_GetScreen()` used as blit target |

The entire architecture assumes a software `SDL_Surface*` is the render target.
SDL3 replaces this with a `SDL_Window` + `SDL_Renderer` pair. The `screen`
global, `T4K_GetScreen()`, and every `SDL_BlitSurface(..., screen, ...)` call
participates in this pattern and must be redesigned.

---

## 9. Image Loading (SDL_image API)

### 9.1 IMG_Load (still present, but context changed)

| File | Line | Usage |
|------|------|-------|
| `src/t4k_loaders.c` | 751 | `IMG_Load(pngfn)` |
| `src/t4k_loaders.c` | 758 | `IMG_Load(pngfn)` |
| `src/t4k_loaders.c` | 958 | `IMG_Load(fn)` |

`IMG_Load` exists in SDL3_image but returns an `SDL_Surface`. All returned
surfaces feed into `set_format()` which calls the removed `SDL_DisplayFormat`
family. The loading itself is not broken but the downstream conversion is.

---

## 10. Pixel Format Struct Direct Field Access

### 10.1 SDL_PixelFormat->Rmask/Gmask/Bmask/Amask (struct changed)

| File | Line | Legacy Usage |
|------|------|-------------|
| `src/t4k_loaders.c` | 313 | `T4K_GetScreen()->format->Rmask` |
| `src/t4k_loaders.c` | 314 | `T4K_GetScreen()->format->Gmask` |
| `src/t4k_loaders.c` | 315 | `T4K_GetScreen()->format->Bmask` |
| `src/t4k_loaders.c` | 316 | `T4K_GetScreen()->format->Amask` |
| `src/t4k_loaders.c` | 320 | `T4K_GetScreen()->format->Amask` |
| `src/t4k_loaders.c` | 326 | `T4K_GetScreen()->format->BitsPerPixel` |

In SDL3, `SDL_PixelFormat` is an enum (integer), not a struct. All per-surface
format information that was in the old `SDL_PixelFormat` struct is accessed via
`SDL_GetPixelFormatDetails(format)` which returns a `const SDL_PixelFormatDetails*`.

### 10.2 SDL_PixelFormat->BitsPerPixel / BytesPerPixel (struct changed)

| File | Line | Legacy Usage |
|------|------|-------------|
| `src/t4k_sdl.c` | 156 | `s->format->BytesPerPixel` |
| `src/t4k_sdl.c` | 369 | `fmt1->BitsPerPixel` |
| `src/t4k_sdl.c` | 377 | `fmt2->BitsPerPixel` |
| `src/t4k_sdl.c` | 649 | `src->format->BitsPerPixel` |
| `src/t4k_loaders.c` | 326 | `T4K_GetScreen()->format->BitsPerPixel` |
| `src/t4k_pixels.c` | 156 | `surface->pitch`, `surface->pixels` (still valid) |

`SDL_Surface` still has `format` as an `SDL_PixelFormat` value (now an enum
integer), `pixels`, `pitch`, `w`, and `h`. The `.format->BitsPerPixel` pattern
must become `SDL_BITSPERPIXEL(surface->format)` or a lookup through
`SDL_GetPixelFormatDetails`.

---

## 11. Byteorder Macro Usage

### 11.1 SDL_BYTEORDER / SDL_BIG_ENDIAN (still valid in SDL3)

| File | Line | Usage |
|------|------|-------|
| `src/t4k_common.h` | 114 | `#if SDL_BYTEORDER == SDL_BIG_ENDIAN` |
| `src/t4k_pixels.c` | 103 | `if (SDL_BYTEORDER == SDL_BIG_ENDIAN)` |

These constants remain in SDL3 under the same names. No change needed here.

---

## Summary Table

| Category | File(s) | Count of Affected Sites |
|----------|---------|------------------------|
| SDL_SetVideoMode | t4k_sdl.c | 2 |
| SDL_GetVideoSurface | t4k_sdl.c | 2 |
| SDL_Flip | t4k_sdl.c, t4k_menu.c | 8 |
| SDL_UpdateRect / SDL_UpdateRects | t4k_sdl.c, t4k_menu.c | 8 |
| SDL_FULLSCREEN flag | t4k_sdl.c | 3 |
| SDL_WM_GrabInput | t4k_menu.c | 1 |
| SDL_SWSURFACE / SDL_HWPALETTE | t4k_sdl.c, t4k_menu.c, t4k_loaders.c | 7 |
| SDL_DisplayFormat / DisplayFormatAlpha | t4k_sdl.c, t4k_loaders.c | 9 |
| SDL_SRCCOLORKEY / SDL_SRCALPHA flags | t4k_sdl.c, t4k_loaders.c | 10 |
| SDL_SetAlpha | t4k_loaders.c | 1 |
| SDL_RLEACCEL | t4k_sdl.c, t4k_loaders.c | 2 |
| SDL_Surface->colorkey direct access | t4k_sdl.c | 3 |
| SDL_LowerBlit | t4k_sdl.c | 1 |
| SDL_EventMask / SDL_EVENTMASK | t4k_common.h, t4k_sdl.c | 3 |
| SDLKey | t4k_main.c | 1 |
| SDL_BUTTON_WHEELUP / WHEELDOWN | t4k_menu.c | 2 |
| SDL_KillThread | t4k_tts.c | 2 |
| SDL_CreateThread (2-arg form) | t4k_tts.c | 2 |
| Mix_QuerySpec | t4k_main.c | 1 |
| TTF_GlyphMetrics (old signature) | t4k_sdl.c | 1 |
| SDL_PixelFormat struct field access | t4k_sdl.c, t4k_loaders.c | 14 |
| Global SDL_Surface* screen architecture | t4k_common.h, t4k_sdl.c | architectural |

**Total discrete call sites requiring change: approximately 82**
