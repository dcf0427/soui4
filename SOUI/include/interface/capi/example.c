/*
 * SOUI C API Example
 * 
 * This example demonstrates how to use SOUI C++ interfaces from C code
 * using the provided C API helper macros.
 */

#include <stdio.h>
#include <stdlib.h>

/* Define CINTERFACE to force C-style interface definitions */
#define CINTERFACE

/* Include SOUI C API headers */
#include "soui-capi.h"

/* Example: Timer callback function */
void timer_callback(void* user_data)
{
    printf("Timer callback called with user data: %p\n", user_data);
}

/* Example: Working with Timer interface */
void example_timer_usage(void)
{
    printf("=== Timer Interface Example ===\n");
    
    /* Note: In real code, you would get the timer from SOUI framework */
    ITimer* timer = NULL; /* GetTimerFromSoui(); */
    
    if (timer) {
        /* Start a 1-second repeating timer */
        BOOL result = ITimer_StartTimer(timer, 1000, TRUE, (LPARAM)timer_callback);
        if (result) {
            printf("Timer started successfully\n");
            
            /* Let it run for a while... */
            /* Sleep(5000); */
            
            /* Stop the timer */
            ITimer_KillTimer(timer);
            printf("Timer stopped\n");
        } else {
            printf("Failed to start timer\n");
        }
        
        /* Release the timer */
        ITimer_Release(timer);
    }
    
    printf("\n");
}

/* Example: Working with Window interface */
void example_window_usage(void)
{
    printf("=== Window Interface Example ===\n");
    
    /* Note: In real code, you would get the window from SOUI framework */
    IWindow* window = NULL; /* GetWindowFromSoui(); */
    
    if (window) {
        /* Set window text */
        IWindow_SetWindowText(window, L"Hello from C!");
        printf("Window text set\n");
        
        /* Get window text */
        LPCWSTR text = IWindow_GetWindowText(window, FALSE);
        if (text) {
            wprintf(L"Window text: %s\n", text);
        }
        
        /* Show the window */
        IWindow_Show(window);  /* Convenience macro */
        printf("Window shown\n");
        
        /* Move the window */
        RECT rect = {100, 100, 400, 300};
        IWindow_Move(window, &rect);
        printf("Window moved to (100,100) size 300x200\n");
        
        /* Find a child window by ID */
        IWindow* child = IWindow_FindChildByID(window, 1001, 1);
        if (child) {
            printf("Found child window with ID 1001\n");
            IWindow_Release(child);
        } else {
            printf("Child window with ID 1001 not found\n");
        }
        
        /* Get window rectangle */
        RECT windowRect;
        IWindow_GetWindowRect(window, &windowRect);
        printf("Window rect: (%ld,%ld) - (%ld,%ld)\n", 
               windowRect.left, windowRect.top, 
               windowRect.right, windowRect.bottom);
        
        /* Check if window is visible */
        BOOL visible = IWindow_IsVisible(window, TRUE);
        printf("Window is %s\n", visible ? "visible" : "hidden");
        
        /* Release the window */
        IWindow_Release(window);
    }
    
    printf("\n");
}

/* Example: Working with Bitmap interface */
void example_bitmap_usage(void)
{
    printf("=== Bitmap Interface Example ===\n");
    
    /* Note: In real code, you would create bitmap through render factory */
    IBitmapS* bitmap = NULL; /* CreateBitmapFromFactory(); */
    
    if (bitmap) {
        /* Load bitmap from file */
        HRESULT hr = IBitmapS_LoadFromFile(bitmap, L"test_image.png");
        if (SOUI_SUCCEEDED(hr)) {
            printf("Bitmap loaded successfully\n");
            
            /* Get bitmap dimensions */
            UINT width = IBitmapS_Width(bitmap);
            UINT height = IBitmapS_Height(bitmap);
            printf("Bitmap size: %u x %u pixels\n", width, height);
            
            /* Create a scaled version */
            IBitmapS* scaledBitmap = NULL;
            hr = IBitmapS_Scale2(bitmap, &scaledBitmap, width/2, height/2, kMedium_FilterLevel);
            if (SOUI_SUCCEEDED(hr) && scaledBitmap) {
                printf("Created scaled bitmap: %u x %u pixels\n", 
                       IBitmapS_Width(scaledBitmap), 
                       IBitmapS_Height(scaledBitmap));
                IBitmapS_Release(scaledBitmap);
            }
            
            /* Save bitmap to file */
            hr = IBitmapS_Save(bitmap, L"output.png", NULL);
            if (SOUI_SUCCEEDED(hr)) {
                printf("Bitmap saved to output.png\n");
            }
        } else {
            printf("Failed to load bitmap: HRESULT = 0x%08lX\n", hr);
        }
        
        /* Release the bitmap */
        IBitmapS_Release(bitmap);
    }
    
    printf("\n");
}

/* Example: Working with Accelerator interface */
void example_accelerator_usage(void)
{
    printf("=== Accelerator Interface Example ===\n");
    
    /* Note: In real code, you would get these from SOUI framework */
    IAccelerator* acc = NULL;           /* GetAcceleratorFromSoui(); */
    IAcceleratorMgr* accMgr = NULL;     /* GetAcceleratorManagerFromSoui(); */
    IAcceleratorTarget* target = NULL;  /* GetAcceleratorTargetFromSoui(); */
    
    if (acc && accMgr && target) {
        /* Get accelerator properties */
        WORD modifier = IAccelerator_GetModifier(acc);
        WORD key = IAccelerator_GetKey(acc);
        DWORD accCode = IAccelerator_GetAcc(acc);
        
        printf("Accelerator: Modifier=0x%04X, Key=0x%04X, Code=0x%08lX\n", 
               modifier, key, accCode);
        
        /* Register accelerator */
        IAcceleratorMgr_RegisterAccelerator(accMgr, acc, target);
        printf("Accelerator registered\n");
        
        /* Unregister accelerator */
        IAcceleratorMgr_UnregisterAccelerator(accMgr, acc, target);
        printf("Accelerator unregistered\n");
        
        /* Release objects */
        IAccelerator_Release(acc);
        IAcceleratorMgr_Release(accMgr);
        IAcceleratorTarget_Release(target);
    }
    
    printf("\n");
}

/* Example: Safe reference counting */
void example_safe_reference_counting(void)
{
    printf("=== Safe Reference Counting Example ===\n");
    
    IWindow* window = NULL; /* GetWindowFromSoui(); */
    
    /* Safe AddRef - handles NULL pointers */
    long refCount = SOUI_SafeAddRef(window);
    printf("Reference count after SafeAddRef: %ld\n", refCount);
    
    if (window) {
        /* Normal usage */
        IWindow_SetWindowText(window, L"Reference counting example");
        
        /* Check if interface is valid */
        if (SOUI_IsValidInterface(window)) {
            printf("Interface is valid\n");
        }
    }
    
    /* Safe Release - handles NULL pointers and sets pointer to NULL */
    SOUI_SafeRelease(window);
    printf("Window safely released, pointer is now NULL\n");
    
    printf("\n");
}

/* Main function */
int main(void)
{
    printf("SOUI C API Example\n");
    printf("==================\n\n");
    
    /* Note: In a real application, you would initialize SOUI first */
    printf("Note: This example shows the C API syntax.\n");
    printf("In a real application, you would need to:\n");
    printf("1. Initialize the SOUI framework\n");
    printf("2. Get actual interface pointers from SOUI\n");
    printf("3. Use the interfaces as shown below\n\n");
    
    /* Run examples */
    example_timer_usage();
    example_window_usage();
    example_bitmap_usage();
    example_accelerator_usage();
    example_safe_reference_counting();
    
    printf("Example completed.\n");
    return 0;
}

/*
 * Compilation example:
 * 
 * gcc -DCINTERFACE -I../../../ -I../../ -I../ example.c -o example
 * 
 * Note: You would also need to link with SOUI libraries in a real application.
 */
