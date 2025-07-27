# SOUI C API Helper Macros

This directory contains C API helper macros for SOUI C++ interfaces, enabling C code to call SOUI interface methods using familiar C syntax.

## Overview

SOUI uses COM-style C++ interfaces defined with `DECLARE_INTERFACE` macros. While these interfaces can be called from C code using vtable function pointers, the syntax is verbose and error-prone. These C API helper macros provide a more convenient and readable way to call interface methods from C code.

## Files

### Core C API Headers
- **soui-capi.h** - Main header that includes all C API macros
- **SAccelerator-capi.h** - C API macros for accelerator interfaces
- **STimer-capi.h** - C API macros for timer interface
- **SRender-capi.h** - C API macros for rendering interfaces (IBrushS, IPenS, IBitmapS, IFontS, etc.)
- **SWindow-capi.h** - C API macros for window interface (100+ methods)
- **SAdapter-capi.h** - C API macros for list/tree adapter interfaces
- **SAnimation-capi.h** - C API macros for animation interfaces
- **SFactory-capi.h** - C API macros for SOUI factory interface
- **SRunnable-capi.h** - C API macros for runnable interface
- **SResProvider-capi.h** - C API macros for resource provider interface

### Generation Tools
- **generate_capi.py** - Python script to auto-generate C API macros
- **generate_remaining_capi.py** - Python script to generate basic templates for remaining interfaces
- **generate_basic_capi.bat** - Batch script for basic C API generation

## Usage

### Basic Usage

1. Include the main header in your C code:
```c
#include "SOUI/include/interface/capi/soui-capi.h"
```

2. Use the C API macros instead of direct vtable calls:

**Before (Direct vtable calls):**
```c
ITimer* timer = GetTimer();
timer->lpVtbl->StartTimer(timer, 1000, TRUE, 0);
timer->lpVtbl->KillTimer(timer);
timer->lpVtbl->Release(timer);
```

**After (Using C API macros):**
```c
ITimer* timer = GetTimer();
ITimer_StartTimer(timer, 1000, TRUE, 0);
ITimer_KillTimer(timer);
ITimer_Release(timer);
```

### Interface-Specific Usage

#### Timer Interface
```c
#include "STimer-capi.h"

ITimer* timer = CreateTimer();
if (timer) {
    // Start a 1-second repeating timer
    ITimer_StartTimer(timer, 1000, TRUE, 0);
    
    // Or use convenience macros
    ITimer_StartRepeatingTimer(timer, 1000);
    
    // Stop the timer
    ITimer_KillTimer(timer);
    
    // Release the timer
    ITimer_Release(timer);
}
```

#### Window Interface
```c
#include "SWindow-capi.h"

IWindow* window = GetWindow();
if (window) {
    // Set window text
    IWindow_SetWindowText(window, L"Hello World");
    
    // Get window text
    LPCWSTR text = IWindow_GetWindowText(window, FALSE);
    
    // Show/hide window
    IWindow_Show(window);  // Convenience macro
    IWindow_Hide(window);  // Convenience macro
    
    // Move window
    RECT rect = {10, 10, 200, 100};
    IWindow_Move(window, &rect);
    
    // Find child window
    IWindow* child = IWindow_FindChildByID(window, 1001, 1);
    
    IWindow_Release(window);
}
```

#### Rendering Interface
```c
#include "SRender-capi.h"

IBitmapS* bitmap = CreateBitmap();
if (bitmap) {
    // Load bitmap from file
    HRESULT hr = IBitmapS_LoadFromFile(bitmap, L"image.png");
    if (SOUI_SUCCEEDED(hr)) {
        // Get bitmap dimensions
        UINT width = IBitmapS_Width(bitmap);
        UINT height = IBitmapS_Height(bitmap);
        
        printf("Bitmap size: %u x %u\n", width, height);
    }
    
    IBitmapS_Release(bitmap);
}
```

### Helper Functions

Each interface also provides C-style helper functions with `_C` suffix:

```c
// Using macros
ITimer_StartTimer(timer, 1000, TRUE, 0);

// Using helper functions (equivalent)
ITimer_StartTimer_C(timer, 1000, TRUE, 0);
```

### Convenience Macros

Many interfaces provide convenience macros for common operations:

```c
// Window convenience macros
IWindow_Show(window);           // SetVisible(TRUE, TRUE)
IWindow_Hide(window);           // SetVisible(FALSE, TRUE)
IWindow_Enable(window);         // EnableWindow(TRUE, TRUE)
IWindow_Disable(window);        // EnableWindow(FALSE, TRUE)
IWindow_SetText(window, text);  // SetWindowText(text)
IWindow_GetText(window);        // GetWindowText(FALSE)

// Timer convenience macros
ITimer_StartOneShotTimer(timer, 1000);     // StartTimer(1000, FALSE)
ITimer_StartRepeatingTimer(timer, 1000);   // StartTimer(1000, TRUE)
```

### Error Handling

Use the provided error checking macros:

```c
HRESULT hr = IBitmapS_LoadFromFile(bitmap, L"image.png");
if (SOUI_SUCCEEDED(hr)) {
    // Success
} else if (SOUI_FAILED(hr)) {
    // Error
}
```

### Reference Counting

All interfaces support reference counting through `AddRef` and `Release`:

```c
IWindow* window = GetWindow();
IWindow_AddRef(window);  // Increment reference count

// Use window...

IWindow_Release(window); // Decrement reference count
IWindow_Release(window); // Release original reference
```

For safer reference management, use the safe macros:

```c
IWindow* window = GetWindow();
SOUI_SafeAddRef(window);    // NULL-safe AddRef
SOUI_SafeRelease(window);   // NULL-safe Release, sets pointer to NULL
```

## Macro Naming Convention

All C API macros follow a consistent naming pattern:

- **Interface macros**: `InterfaceName_MethodName(This, ...)`
- **Helper functions**: `InterfaceName_MethodName_C(pThis, ...)`
- **Convenience macros**: `InterfaceName_ConvenienceName(This, ...)`

Examples:
- `ITimer_StartTimer(timer, 1000, TRUE, 0)`
- `IWindow_SetWindowText(window, L"Text")`
- `IBitmapS_LoadFromFile(bitmap, L"file.png")`

## Generating Additional C API Macros

To generate C API macros for additional interfaces:

1. **Using Python script** (recommended):
```bash
cd SOUI/include/interface/capi
python generate_capi.py
```

2. **Using batch script** (basic templates):
```cmd
cd SOUI\include\interface\capi
generate_basic_capi.bat
```

3. **Manual creation**: Follow the pattern in existing files like `STimer-capi.h`

## Integration with Build System

To use these C API macros in your project:

1. Add the capi directory to your include path
2. Include `soui-capi.h` in your C source files
3. Ensure the original SOUI interface headers are accessible

Example CMakeLists.txt:
```cmake
target_include_directories(your_target PRIVATE 
    ${SOUI_ROOT}/include/interface/capi
    ${SOUI_ROOT}/include/interface
)
```

## Notes

- These macros are designed for C code calling SOUI C++ interfaces
- The macros assume the standard COM vtable layout
- All macros are inline and have no runtime overhead
- The `This` parameter is always the first parameter in macros
- Helper functions provide type safety and can be used in debuggers

## Contributing

When adding new C API macros:

1. Follow the existing naming conventions
2. Include both macros and helper functions
3. Add convenience macros for common operations
4. Update the main `soui-capi.h` file
5. Add usage examples to this README
