#!/usr/bin/env python3
"""
Generate C API macros for remaining SOUI interfaces

This script creates basic C API macro files for interfaces that haven't been
manually created yet. It generates a standard template for each interface.
"""

import os
import re
from pathlib import Path

# List of interfaces that already have C API files
EXISTING_CAPI = {
    'SAccelerator-i.h': 'SAccelerator-capi.h',
    'STimer-i.h': 'STimer-capi.h', 
    'SRender-i.h': 'SRender-capi.h',
    'SWindow-i.h': 'SWindow-capi.h',
    'SAdapter-i.h': 'SAdapter-capi.h',
    'SAnimation-i.h': 'SAnimation-capi.h',
    'SFactory-i.h': 'SFactory-capi.h',
    'SRunnable-i.h': 'SRunnable-capi.h'
}

# Interfaces to generate
INTERFACES_TO_GENERATE = [
    'SAttrStorage-i.h',
    'SCtrl-i.h', 
    'SEvtArgs-i.h',
    'SGradient-i.h',
    'SHostPresenter-i.h',
    'SHttpClient-i.h',
    'SImgDecoder-i.h',
    'SListViewItemLocator-i.h',
    'SMatrix-i.h',
    'SMessageBox-i.h',
    'SMsgLoop-i.h',
    'SNativeWnd-i.h',
    'SNcPainter-i.h',
    'SNotifyCenter-i.h',
    'SObjFactory-i.h',
    'SPathEffect-i.h',
    'SRealWndHandler-i.h',
    'SResProvider-i.h',
    'SResProviderMgr-i.h',
    'SScriptModule-i.h',
    'SSkinPool-i.h',
    'SSkinobj-i.h',
    'STaskLoop-i.h',
    'STileViewItemLocator-i.h',
    'STimelineHandler-i.h',
    'STransform-i.h',
    'STranslator-i.h',
    'STreeViewItemLocator-i.h',
    'SValueAnimator-i.h',
    'SWndContainer-i.h',
    'sacchelper-i.h',
    'saccproxy-i.h',
    'sapp-i.h',
    'scaret-i.h',
    'shostwnd-i.h',
    'sinterpolator-i.h',
    'sipcobj-i.h',
    'slayout-i.h',
    'slog-i.h',
    'smenu-i.h',
    'smenuex-i.h',
    'sobject-i.h',
    'stooltip-i.h'
]

def extract_interfaces_from_file(file_path):
    """Extract interface names from a header file"""
    interfaces = []
    try:
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
            
        # Find DECLARE_INTERFACE patterns
        pattern = r'#undef\s+INTERFACE\s*\n\s*#define\s+INTERFACE\s+(\w+)\s*\n\s*DECLARE_INTERFACE'
        matches = re.finditer(pattern, content, re.MULTILINE)
        
        for match in matches:
            interface_name = match.group(1)
            if interface_name and not interface_name.startswith('_'):
                interfaces.append(interface_name)
                
    except Exception as e:
        print(f"Warning: Could not parse {file_path}: {e}")
        
    return interfaces

def generate_basic_capi_header(interface_file, interfaces):
    """Generate a basic C API header for the given interfaces"""
    base_name = interface_file.replace('-i.h', '').replace('.h', '')
    header_guard = f"__{base_name.upper()}_CAPI_H__"
    
    content = f"""#ifndef {header_guard}
#define {header_guard}

#include "../{interface_file}"

#ifdef __cplusplus
extern "C" {{
#endif

/*
 * C API Helper Macros for {base_name} Interfaces
 * These macros provide C-style function call syntax for C++ interface methods
 * 
 * Note: This is a basic template. You may need to add specific methods
 * based on the actual interface definitions.
 */

"""
    
    # Generate basic macros for each interface
    for interface in interfaces:
        content += f"""/* {interface} C API Macros */
#define {interface}_AddRef(This) \\
    ((This)->lpVtbl->AddRef(This))

#define {interface}_Release(This) \\
    ((This)->lpVtbl->Release(This))

#define {interface}_OnFinalRelease(This) \\
    ((This)->lpVtbl->OnFinalRelease(This))

/* Add more {interface} methods here as needed */

"""
    
    # Generate helper functions
    content += """/*
 * C API Helper Functions (Optional - for more C-like usage)
 */

"""
    
    for interface in interfaces:
        content += f"""/* {interface} Helper Functions */
static inline long {interface}_AddRef_C({interface}* pThis)
{{
    return {interface}_AddRef(pThis);
}}

static inline long {interface}_Release_C({interface}* pThis)
{{
    return {interface}_Release(pThis);
}}

static inline void {interface}_OnFinalRelease_C({interface}* pThis)
{{
    {interface}_OnFinalRelease(pThis);
}}

"""
    
    # Add convenience macros
    content += """/*
 * Convenience macros for common operations
 */

"""
    
    for interface in interfaces:
        content += f"""#define {interface}_SafeAddRef(This) \\
    SOUI_SafeAddRef((IUnknown*)(This))

#define {interface}_SafeRelease(This) \\
    SOUI_SafeRelease((IUnknown**)(This))

"""
    
    content += """#ifdef __cplusplus
}
#endif

#endif /* """ + header_guard + """ */"""
    
    return content

def main():
    """Main function to generate C API headers"""
    script_dir = Path(__file__).parent
    interface_dir = script_dir.parent
    
    print("Generating C API headers for remaining SOUI interfaces...")
    
    generated_count = 0
    
    for interface_file in INTERFACES_TO_GENERATE:
        interface_path = interface_dir / interface_file
        
        if not interface_path.exists():
            print(f"Warning: Interface file {interface_file} not found")
            continue
            
        # Extract interface names from the file
        interfaces = extract_interfaces_from_file(interface_path)
        
        if not interfaces:
            print(f"Warning: No interfaces found in {interface_file}")
            continue
            
        # Generate C API header
        capi_content = generate_basic_capi_header(interface_file, interfaces)
        
        # Write to output file
        base_name = interface_file.replace('-i.h', '').replace('.h', '')
        output_file = script_dir / f"{base_name}-capi.h"
        
        # Skip if file already exists
        if output_file.exists():
            print(f"Skipping {output_file.name} (already exists)")
            continue
            
        try:
            with open(output_file, 'w', encoding='utf-8') as f:
                f.write(capi_content)
                
            print(f"Generated {output_file.name} with {len(interfaces)} interfaces: {', '.join(interfaces)}")
            generated_count += 1
            
        except Exception as e:
            print(f"Error writing {output_file}: {e}")
    
    print(f"\nGenerated {generated_count} new C API headers")
    
    # Update the main soui-capi.h file
    update_main_header(script_dir)

def update_main_header(capi_dir):
    """Update the main soui-capi.h file to include all generated headers"""
    main_header = capi_dir / "soui-capi.h"
    
    if not main_header.exists():
        print("Warning: Main header soui-capi.h not found")
        return
        
    # Find all *-capi.h files
    capi_files = list(capi_dir.glob("*-capi.h"))
    capi_files = [f for f in capi_files if f.name != "soui-capi.h"]
    
    # Read current content
    try:
        with open(main_header, 'r', encoding='utf-8') as f:
            content = f.read()
    except Exception as e:
        print(f"Error reading main header: {e}")
        return
        
    # Find the include section
    include_pattern = r'(/\* Include all (?:individual|generated) C API headers \*/.*?)(/\* .*? \*/|\n\n)'
    match = re.search(include_pattern, content, re.DOTALL)
    
    if match:
        # Generate new include section
        includes = "/* Include all generated C API headers */\n"
        for capi_file in sorted(capi_files):
            includes += f'#include "{capi_file.name}"\n'
        includes += "\n"
        
        # Replace the include section
        new_content = content[:match.start()] + includes + content[match.end():]
        
        try:
            with open(main_header, 'w', encoding='utf-8') as f:
                f.write(new_content)
            print(f"Updated {main_header.name} with {len(capi_files)} includes")
        except Exception as e:
            print(f"Error updating main header: {e}")
    else:
        print("Warning: Could not find include section in main header")

if __name__ == "__main__":
    main()
