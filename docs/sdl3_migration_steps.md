# SDL3 Migration Steps

Precise per-line change specification for every deprecated call identified
in `sdl3_legacy_inventory.md`. Each issue block maps to one GitHub issue.

---

## Issue 1 - Introduce SDL_Window and SDL_Renderer; remove global SDL_Surface* screen

**Files:** `src/t4k_sdl.c`, `src/t4k_common.h`
**Priority:** Blocker. All other issues depend on this one.

### Architectural change required

SDL3 has no primary display surface. The entire render target model must shift
from `SDL_Surface*` to `SDL_Window* + SDL_Renderer*`.

Add two new module-private globals to `t4k_sdl.c`:

```c
// Replace:
SDL_Surface* screen = NULL;

// With:
static SDL_Window*   sdl_window   = NULL;
static SDL_Renderer* sdl_renderer = NULL;
```

Expose getters via the existing header pattern:

```c
// Add to t4k_common.h:
SDL_Window*   T4K_GetWindow(void);
SDL_Renderer* T4K_GetRenderer(void);
```

The existing `extern SDL_Surface* screen` declaration in `t4k_common.h` line 171
and `T4K_GetScreen()` in `t4k_sdl.c` line 66-74 must be deprecated together.
Keep `T4K_GetScreen()` returning NULL or a streaming texture surface during the
transition to avoid breaking callers all at once.

---

## Issue 2 - Replace SDL_SetVideoMode with SDL_CreateWindow + SDL_CreateRenderer

**File:** `src/t4k_sdl.c`
**Lines:** 526-529 (`T4K_ChangeWindowSize`), 559-562 (`T4K_SwitchScreenMode`)

### T4K_ChangeWindowSize (line 520-551)

```c
// Remove:
screen = SDL_SetVideoMode(new_res_x, new_res_y, PIXEL_BITS,
                          SDL_SWSURFACE|SDL_HWPALETTE);

// Replace with:
SDL_SetWindowSize(sdl_window, new_res_x, new_res_y);
// No return value check needed; window resize is best-effort.
// Renderer automatically follows window size.
```

Also remove line 524 `if(!(screen->flags & SDL_FULLSCREEN))` - replace with:

```c
Uint32 wflags = SDL_GetWindowFlags(sdl_window);
if(!(wflags & SDL_WINDOW_FULLSCREEN))
```

Remove line 546 `SDL_UpdateRect(screen, 0, 0, 0, 0)` - replace with:
```c
SDL_RenderPresent(sdl_renderer);
```

### T4K_SwitchScreenMode (line 554-586)

```c
// Remove:
int window = (screen->flags & SDL_FULLSCREEN);
screen = SDL_SetVideoMode(..., screen->flags ^ SDL_FULLSCREEN);

// Replace with:
Uint32 wflags = SDL_GetWindowFlags(sdl_window);
bool currently_fullscreen = (wflags & SDL_WINDOW_FULLSCREEN) != 0;
SDL_SetWindowFullscreen(sdl_window,
    currently_fullscreen ? 0 : SDL_WINDOW_FULLSCREEN);
```

Remove line 584 `SDL_UpdateRect(screen, 0, 0, 0, 0)` - replace with:
```c
SDL_RenderPresent(sdl_renderer);
```

---

## Issue 3 - Remove SDL_GetVideoSurface from T4K_GetScreen

**File:** `src/t4k_sdl.c`
**Lines:** 66-74

```c
// Remove entirely:
SDL_Surface* T4K_GetScreen()
{
    if (screen != SDL_GetVideoSurface())
    {
        fprintf(stderr, "Video Surface changed from outside of SDL_Extras!\n");
        screen = SDL_GetVideoSurface();
    }
    return screen;
}

// Replace with a stub that returns NULL until full renderer migration:
SDL_Surface* T4K_GetScreen(void)
{
    return screen; // Will be NULL; callers must migrate to T4K_GetRenderer()
}
```

---

## Issue 4 - Replace SDL_Flip with SDL_RenderPresent

**File:** `src/t4k_sdl.c` lines 827, 836, 867, 876, 920, 929
**File:** `src/t4k_menu.c` lines 912, 1068

```c
// Remove in every location:
SDL_Flip(screen);
// or
SDL_Flip(T4K_GetScreen());

// Replace with:
SDL_RenderPresent(T4K_GetRenderer());
```

Eight call sites total. Each is a one-line substitution.

---

## Issue 5 - Replace SDL_UpdateRect and SDL_UpdateRects

**File:** `src/t4k_sdl.c` lines 472, 546, 584, 1156
**File:** `src/t4k_menu.c` lines 511, 863, 889, 1482

In the SDL3 renderer model, partial-rect updates do not exist at the
`SDL_UpdateRect` level. The replacement is always `SDL_RenderPresent`.

```c
// Remove all occurrences of:
SDL_UpdateRect(surf, x, y, w, h);
SDL_UpdateRect(screen, 0, 0, 0, 0);
SDL_UpdateRects(screen, numupdates, dstupdate);

// Replace with:
SDL_RenderPresent(T4K_GetRenderer());
```

For `T4K_UpdateRect` (t4k_sdl.c line 470-473), the function body becomes:

```c
void T4K_UpdateRect(SDL_Surface* surf, SDL_Rect* rect)
{
    // surf parameter kept for API compatibility; ignored in SDL3 renderer path.
    SDL_RenderPresent(T4K_GetRenderer());
}
```

---

## Issue 6 - Replace SDL_CreateRGBSurface flag arguments

**File:** `src/t4k_sdl.c` lines 130, 276, 392, 648, 1396
**File:** `src/t4k_menu.c` line 1129
**File:** `src/t4k_loaders.c` line 325

SDL3 removes the flags parameter from `SDL_CreateRGBSurface`.
The replacement function is `SDL_CreateSurface(w, h, format)`.

```c
// Remove pattern:
SDL_CreateRGBSurface(SDL_SWSURFACE|SDL_SRCALPHA, w, h, 32,
                     rmask, gmask, bmask, amask);
// Replace with:
SDL_CreateSurface(w, h, SDL_PIXELFORMAT_RGBA32);

// Remove pattern:
SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32, rmask, gmask, bmask, amask);
// Replace with:
SDL_CreateSurface(w, h, SDL_PIXELFORMAT_RGBA32);

// Remove pattern (t4k_zoom, src->flags passed through):
SDL_CreateRGBSurface(src->flags, new_w, new_h, src->format->BitsPerPixel,
                     src->format->Rmask, ...);
// Replace with:
SDL_CreateSurface(new_w, new_h, src->format);
```

For `SDL_ConvertSurface(S1, fmt1, SDL_SWSURFACE)` at line 392:
```c
// Remove:
tmpS = SDL_ConvertSurface(S1, fmt1, SDL_SWSURFACE);
// Replace with:
tmpS = SDL_ConvertSurface(S1, S1->format);
```

---

## Issue 7 - Remove SDL_DisplayFormat and SDL_DisplayFormatAlpha

**File:** `src/t4k_sdl.c` lines 318, 325, 329, 443, 1449
**File:** `src/t4k_loaders.c` lines 624, 630, 639, 670

In SDL3 there is no "display format" for surfaces. Use `SDL_ConvertSurface`
with a specific pixel format, or upload directly to a texture.

```c
// Remove:
SDL_DisplayFormat(surface)
// Replace with:
SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGB24)

// Remove:
SDL_DisplayFormatAlpha(surface)
// Replace with:
SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32)
```

In `set_format()` at `t4k_loaders.c` lines 617-648, the entire switch block
that calls `SDL_DisplayFormat` and `SDL_DisplayFormatAlpha` must be updated:

```c
// IMG_REGULAR case (line 624):
return SDL_ConvertSurface(img, SDL_PIXELFORMAT_RGB24);

// IMG_ALPHA case (line 630):
return SDL_ConvertSurface(img, SDL_PIXELFORMAT_RGBA32);

// IMG_COLORKEY case (lines 636-639):
SDL_SetSurfaceColorKey(img, SDL_TRUE,
    SDL_MapRGB(SDL_GetPixelFormatDetails(img->format), NULL, 255, 255, 0));
return SDL_ConvertSurface(img, SDL_PIXELFORMAT_RGB24);
```

---

## Issue 8 - Remove SDL_SRCCOLORKEY, SDL_SRCALPHA flags and colorkey field access

**File:** `src/t4k_sdl.c` lines 263-268, 315-324

In `T4K_Flip()`, the code saves and restores colorkey/alpha flags.
SDL3 provides dedicated query functions:

```c
// Remove flag-based detection and direct field access:
flags = in->flags;
if (flags & SDL_SRCCOLORKEY) { colorkey = in->format->colorkey; }
if (flags & SDL_SRCALPHA)    { ... }

// Replace with:
SDL_bool has_colorkey = SDL_GetSurfaceColorKey(in, &colorkey);
SDL_BlendMode blend_mode;
SDL_GetSurfaceBlendMode(in, &blend_mode);
```

After the flip operation, restore state:
```c
if (has_colorkey)
    SDL_SetSurfaceColorKey(out, SDL_TRUE, colorkey);
SDL_SetSurfaceBlendMode(out, blend_mode);
```

---

## Issue 9 - Replace SDL_SetAlpha and SDL_RLEACCEL

**File:** `src/t4k_loaders.c` line 669
**File:** `src/t4k_sdl.c` line 1448, `t4k_loaders.c` line 637

```c
// Remove (t4k_loaders.c line 669):
SDL_SetAlpha(orig, SDL_RLEACCEL, SDL_ALPHA_OPAQUE);
// Replace with:
SDL_SetSurfaceBlendMode(orig, SDL_BLENDMODE_NONE);
// SDL_ALPHA_OPAQUE (255) is the default; no alpha mod call needed.

// Remove (t4k_sdl.c line 1448):
SDL_SetColorKey(bg, SDL_SRCCOLORKEY|SDL_RLEACCEL, color_key);
// Replace with:
SDL_SetSurfaceColorKey(bg, SDL_TRUE, color_key);
```

---

## Issue 10 - Fix SDL_PixelFormat struct field access

**File:** `src/t4k_loaders.c` lines 313-326
**File:** `src/t4k_sdl.c` lines 156, 369, 377, 649

SDL3 `SDL_PixelFormat` is now an `SDL_PixelFormat` enum (Uint32), not a struct.
The `SDL_Surface->format` field is now `SDL_PixelFormat` (enum).
Struct details are accessed via `SDL_GetPixelFormatDetails(format)`.

```c
// Remove:
SDL_Surface* screen_surf = T4K_GetScreen();
Rmask = screen_surf->format->Rmask;
Gmask = screen_surf->format->Gmask;
// ...etc

// Replace with (t4k_loaders.c render_svg_from_handle):
// Use a fixed RGBA32 format for the destination surface.
// The Rmask/Gmask dance is unnecessary when creating with a named format.
dest = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_ARGB8888);

// For BitsPerPixel/BytesPerPixel access:
// Remove:  src->format->BitsPerPixel
// Replace: SDL_BITSPERPIXEL(src->format)

// Remove:  s->format->BytesPerPixel  (t4k_sdl.c line 156)
// Replace: SDL_BYTESPERPIXEL(s->format)

// Remove:  fmt1->BitsPerPixel  (t4k_sdl.c line 369, used via SDL_PixelFormat*)
// Replace: SDL_BITSPERPIXEL(S1->format)
```

---

## Issue 11 - Replace SDL_LowerBlit in T4K_UpdateScreen

**File:** `src/t4k_sdl.c` line 1126

```c
// Remove:
SDL_LowerBlit(blits[i].src, blits[i].srcrect, screen, blits[i].dstrect);

// Replace with:
SDL_BlitSurface(blits[i].src, blits[i].srcrect, screen, blits[i].dstrect);
// Note: the entire blit-queue system (T4K_UpdateScreen) must be reviewed
// after Issue 1 is resolved, as screen will no longer be a valid blit target.
```

---

## Issue 12 - Fix SDL_EventMask and SDL_EVENTMASK

**File:** `src/t4k_common.h` line 957, `src/t4k_sdl.c` lines 603-610

`SDL_EventMask` does not exist in SDL3. `T4K_WaitForEvent` must be redesigned.

```c
// Remove:
SDL_EventType T4K_WaitForEvent(SDL_EventMask events);

// Replace signature in t4k_common.h:
SDL_EventType T4K_WaitForEvent(Uint32 event_type_mask);

// In t4k_sdl.c, replace body:
SDL_EventType T4K_WaitForEvent(Uint32 event_type_mask)
{
    SDL_Event evt;
    while (1)
    {
        while (SDL_PollEvent(&evt))
        {
            if ((1u << evt.type) & event_type_mask)
                return evt.type;
        }
        SDL_Delay(10);
    }
}
// Callers must update their arguments from SDL_KEYDOWNMASK to (1<<SDL_EVENT_KEY_DOWN) etc.
```

---

## Issue 13 - Fix SDLKey type in t4k_main.c

**File:** `src/t4k_main.c` line 129

```c
// Remove:
SDLKey key = event->key.keysym.sym;

// Replace with:
SDL_Keycode key = event->key.key;
// Note: in SDL3 the field is event->key.key, not event->key.keysym.sym
```

---

## Issue 14 - Fix mouse wheel events in t4k_menu.c

**File:** `src/t4k_menu.c` lines 598-629

SDL3 delivers wheel events as `SDL_EVENT_MOUSE_WHEEL`, not as button presses.

```c
// Remove from SDL_MOUSEBUTTONDOWN case:
if(event.button.button == SDL_BUTTON_WHEELUP)   { ... }
else if(event.button.button == SDL_BUTTON_WHEELDOWN) { ... }

// Add a new case in the event switch:
case SDL_EVENT_MOUSE_WHEEL:
{
    if(event.wheel.y > 0)  // scroll up
    {
        // existing WHEELUP logic
    }
    else if(event.wheel.y < 0)  // scroll down
    {
        // existing WHEELDOWN logic
    }
    break;
}
```

Also update all SDL event type constants throughout t4k_menu.c:

| Old Constant | New Constant |
|---|---|
| `SDL_QUIT` | `SDL_EVENT_QUIT` |
| `SDL_MOUSEMOTION` | `SDL_EVENT_MOUSE_MOTION` |
| `SDL_MOUSEBUTTONDOWN` | `SDL_EVENT_MOUSE_BUTTON_DOWN` |
| `SDL_KEYDOWN` | `SDL_EVENT_KEY_DOWN` |

---

## Issue 15 - Fix SDL_KillThread and SDL_CreateThread in t4k_tts.c

**File:** `src/t4k_tts.c` lines 103, 156, 256, 316

### SDL_KillThread removal (lines 103, 256)

```c
// Remove:
SDL_KillThread(tts_thread);
tts_thread = NULL;

// Replace with cooperative shutdown:
// Add a module-level flag:
static SDL_atomic_t tts_stop_requested;

// In T4K_Tts_stop(), set the flag and wait:
SDL_SetAtomicInt(&tts_stop_requested, 1);
if(tts_thread)
{
    SDL_WaitThread(tts_thread, NULL);
    tts_thread = NULL;
}
SDL_SetAtomicInt(&tts_stop_requested, 0);

// In tts_thread_func(), check the flag before speaking:
if(SDL_GetAtomicInt(&tts_stop_requested))
    return 0;
```

### SDL_CreateThread signature (lines 156, 316)

```c
// Remove:
tts_thread = SDL_CreateThread(tts_thread_func, &data_to_pass);

// Replace with (SDL3 requires a name string as second argument):
tts_thread = SDL_CreateThread(tts_thread_func, "tts_worker", &data_to_pass);
```

---

## Issue 16 - Fix Mix_QuerySpec in t4k_main.c

**File:** `src/t4k_main.c` lines 96-106

`Mix_QuerySpec` is removed in SDL_mixer 3. The audio open-count pattern is
also removed. SDL_mixer 3 opens a single device.

```c
// Remove:
int frequency, channels, n_timesopened;
Uint16 format;
n_timesopened = Mix_QuerySpec(&frequency, &format, &channels);
while (n_timesopened)
{
    Mix_CloseAudio();
    n_timesopened--;
}

// Replace with:
Mix_CloseAudio();
// SDL_mixer 3 tracks open state internally; one call is sufficient.
```

---

## Issue 17 - Fix TTF_GlyphMetrics signature in t4k_sdl.c

**File:** `src/t4k_sdl.c` lines 1582-1588

```c
// Remove:
TTF_GlyphMetrics(font, t[i], NULL, NULL, NULL, &h, NULL);

// Replace with (SDL3_ttf function renamed and char type changed):
int minx, maxx, miny, maxy, advance;
TTF_GetGlyphMetrics(font, (Uint32)(unsigned char)t[i],
                    &minx, &maxx, &miny, &maxy, &advance);
h = maxy;
```

`TTF_FontAscent` on line 1588 remains valid in SDL3_ttf. No change needed there
beyond verifying the include path `<SDL3_ttf/SDL_ttf.h>` is consistent.

---

## Issue 18 - Fix SDL_WM_GrabInput in t4k_menu.c

**File:** `src/t4k_menu.c` line 513

```c
// Remove:
SDL_WM_GrabInput(SDL_GRAB_OFF);

// Replace with:
SDL_SetWindowMouseGrab(T4K_GetWindow(), SDL_FALSE);
```

---

## Issue 19 - Update T4K_DarkenScreen to use renderer/texture path

**File:** `src/t4k_sdl.c` lines 484-517

This function directly manipulates `screen->pixels`. With a renderer, the
pixel data is not directly accessible this way.

The SDL3 approach is to use an `SDL_Texture` with `SDL_BLENDMODE_BLEND`
and render a semi-transparent black rectangle:

```c
void T4K_DarkenScreen(Uint8 bits)
{
    // bits=1 -> 50% dark, bits=2 -> 75% dark (same visual effect as before)
    Uint8 alpha = 255 - (255 >> bits);
    SDL_SetRenderDrawBlendMode(T4K_GetRenderer(), SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(T4K_GetRenderer(), 0, 0, 0, alpha);
    SDL_RenderFillRect(T4K_GetRenderer(), NULL);
}
```

---

## Issue 20 - Migrate T4K_BlackOutline text rendering to SDL3_ttf surface path

**File:** `src/t4k_sdl.c` lines 1341-1455

Beyond the `SDL_DisplayFormatAlpha` and `SDL_SetColorKey` fixes covered in
Issues 7 and 9, the background surface creation at line 1396 must change:

```c
// Remove:
bg = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32, rmask, gmask, bmask, amask);

// Replace:
bg = SDL_CreateSurface(w, h, SDL_PIXELFORMAT_RGBA32);
```

And the color key mechanism at lines 1402-1403 can be replaced with
a transparent fill since SDL3 surfaces support alpha natively:

```c
// Remove color_key workaround, use transparent clear instead:
SDL_FillSurfaceRect(bg, NULL, SDL_MapSurfaceRGBA(bg, 0, 0, 0, 0));
```

---

## Dependency Order

The issues must be resolved in the following sequence to avoid cascading
build failures:

1. Issue 1 (Window/Renderer architecture) - foundation for all rendering issues
2. Issue 2 (SDL_SetVideoMode) - depends on Issue 1
3. Issue 3 (SDL_GetVideoSurface) - depends on Issue 1
4. Issue 6 (SDL_CreateRGBSurface flags) - prerequisite for surface-creating issues
5. Issue 10 (SDL_PixelFormat struct) - prerequisite for Issues 7, 8, 20
6. Issues 4, 5 (SDL_Flip, SDL_UpdateRect) - depends on Issue 1
7. Issues 7, 8, 9 (DisplayFormat, SRCCOLORKEY, SetAlpha) - depends on Issues 6, 10
8. Issue 11 (SDL_LowerBlit) - depends on Issue 1
9. Issue 12 (SDL_EventMask)
10. Issue 13 (SDLKey)
11. Issue 14 (mouse wheel)
12. Issues 15, 16, 17, 18 (TTS, audio, TTF, WM)
13. Issues 19, 20 (DarkenScreen, BlackOutline) - depends on Issues 4, 7, 9
