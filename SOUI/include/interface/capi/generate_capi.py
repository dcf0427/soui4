#!/usr/bin/env python3
"""
SOUI C API Generator

This script automatically generates C API helper macros for all SOUI C++ interfaces.
It parses interface header files and creates corresponding C API headers with macros
that provide C-style function call syntax for C++ interface methods.

Usage:
    python generate_capi.py

The script will:
1. Scan all *-i.h files in the interface directory
2. Parse interface definitions using DECLARE_INTERFACE macros
3. Extract method signatures using STDMETHOD patterns
4. Generate C API macros for each interface
5. Create individual *-capi.h files for each interface
6. Update the main soui-capi.h file to include all generated headers
"""

import os
import re
import sys
from pathlib import Path
from typing import List, Dict, Tuple, Optional

class InterfaceMethod:
    """Represents a single interface method"""
    def __init__(self, return_type: str, name: str, params: str, is_const: bool = False):
        self.return_type = return_type
        self.name = name
        self.params = params
        self.is_const = is_const
        
    def __str__(self):
        const_str = " SCONST" if self.is_const else ""
        return f"{self.return_type} {self.name}({self.params}){const_str}"

class InterfaceDefinition:
    """Represents a complete interface definition"""
    def __init__(self, name: str, base_interface: str = ""):
        self.name = name
        self.base_interface = base_interface
        self.methods: List[InterfaceMethod] = []
        
    def add_method(self, method: InterfaceMethod):
        self.methods.append(method)

class CAPIGenerator:
    """Main generator class for C API macros"""
    
    def __init__(self, interface_dir: str, output_dir: str):
        self.interface_dir = Path(interface_dir)
        self.output_dir = Path(output_dir)
        self.interfaces: Dict[str, InterfaceDefinition] = {}
        
        # Ensure output directory exists
        self.output_dir.mkdir(exist_ok=True)
        
    def parse_interface_file(self, file_path: Path) -> List[InterfaceDefinition]:
        """Parse a single interface header file"""
        interfaces = []
        
        try:
            with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
        except Exception as e:
            print(f"Warning: Could not read {file_path}: {e}")
            return interfaces
            
        # Find all DECLARE_INTERFACE patterns
        interface_pattern = r'#undef\s+INTERFACE\s*\n\s*#define\s+INTERFACE\s+(\w+)\s*\n\s*DECLARE_INTERFACE(?:_\((\w+),\s*(\w+)\)|\((\w+)\))'
        
        for match in re.finditer(interface_pattern, content, re.MULTILINE):
            if match.group(2):  # DECLARE_INTERFACE_(name, base)
                interface_name = match.group(2)
                base_interface = match.group(3)
            else:  # DECLARE_INTERFACE(name)
                interface_name = match.group(4)
                base_interface = ""
                
            interface = InterfaceDefinition(interface_name, base_interface)
            
            # Find the interface body (between { and })
            start_pos = match.end()
            brace_count = 0
            body_start = -1
            body_end = -1
            
            for i, char in enumerate(content[start_pos:], start_pos):
                if char == '{':
                    if body_start == -1:
                        body_start = i + 1
                    brace_count += 1
                elif char == '}':
                    brace_count -= 1
                    if brace_count == 0:
                        body_end = i
                        break
                        
            if body_start != -1 and body_end != -1:
                body = content[body_start:body_end]
                self._parse_interface_methods(interface, body)
                
            interfaces.append(interface)
            
        return interfaces
    
    def _parse_interface_methods(self, interface: InterfaceDefinition, body: str):
        """Parse methods from interface body"""
        # Pattern to match STDMETHOD declarations
        method_pattern = r'STDMETHOD(?:_\(([^,]+),\s*(\w+)\)|\((\w+)\))\s*\(([^)]*)\)\s*(SCONST)?\s*(?:OVERRIDE\s*)?PURE;'
        
        for match in re.finditer(method_pattern, body, re.MULTILINE | re.DOTALL):
            if match.group(1):  # STDMETHOD_(type, name)
                return_type = match.group(1).strip()
                method_name = match.group(2).strip()
            else:  # STDMETHOD(name)
                return_type = "HRESULT"
                method_name = match.group(3).strip()
                
            params = match.group(4).strip() if match.group(4) else ""
            is_const = match.group(5) is not None
            
            # Clean up parameters
            params = re.sub(r'\s+', ' ', params)
            params = params.replace('THIS_', '').replace('THIS', '').strip()
            if params.startswith(','):
                params = params[1:].strip()
                
            method = InterfaceMethod(return_type, method_name, params, is_const)
            interface.add_method(method)
    
    def generate_capi_header(self, interface: InterfaceDefinition, original_header: str) -> str:
        """Generate C API header content for a single interface"""
        header_guard = f"__{interface.name.upper()}_CAPI_H__"
        original_include = f"../{original_header}"
        
        content = f"""#ifndef {header_guard}
#define {header_guard}

#include "{original_include}"

#ifdef __cplusplus
extern "C" {{
#endif

/*
 * C API Helper Macros for {interface.name} Interface
 * These macros provide C-style function call syntax for C++ interface methods
 */

"""
        
        # Generate macros for each method
        for method in interface.methods:
            macro_name = f"{interface.name}_{method.name}"
            
            # Build parameter list for macro
            if method.params:
                # Parse parameters to build macro call
                param_names = []
                for param in method.params.split(','):
                    param = param.strip()
                    if param:
                        # Extract parameter name (last word before any default value)
                        param_parts = param.split()
                        if param_parts:
                            name = param_parts[-1]
                            # Remove any default value assignment
                            name = name.split('=')[0].strip()
                            # Remove pointer/reference indicators from name
                            name = name.lstrip('*&')
                            param_names.append(name)
                
                if param_names:
                    macro_params = "This, " + ", ".join(param_names)
                    call_params = "This, " + ", ".join(param_names)
                else:
                    macro_params = "This"
                    call_params = "This"
            else:
                macro_params = "This"
                call_params = "This"
                
            content += f"#define {macro_name}({macro_params}) \\\n"
            content += f"    ((This)->lpVtbl->{method.name}({call_params}))\n\n"
        
        # Generate helper functions
        content += "/*\n * C API Helper Functions (Optional - for more C-like usage)\n"
        content += " * These functions provide an alternative C-style API\n */\n\n"
        
        for method in interface.methods:
            func_name = f"{interface.name}_{method.name}_C"
            
            # Build function signature
            if method.params:
                func_params = f"{interface.name}* pThis"
                if method.params.strip():
                    func_params += f", {method.params}"
            else:
                func_params = f"{interface.name}* pThis"
                
            content += f"static inline {method.return_type} {func_name}({func_params})\n"
            content += "{\n"
            
            if method.return_type.strip() != "void":
                content += f"    return {interface.name}_{method.name}(pThis"
            else:
                content += f"    {interface.name}_{method.name}(pThis"
                
            if method.params:
                param_names = []
                for param in method.params.split(','):
                    param = param.strip()
                    if param:
                        param_parts = param.split()
                        if param_parts:
                            name = param_parts[-1].split('=')[0].strip().lstrip('*&')
                            param_names.append(name)
                if param_names:
                    content += ", " + ", ".join(param_names)
                    
            content += ");\n}\n\n"
        
        content += """#ifdef __cplusplus
}
#endif

#endif /* """ + header_guard + """ */"""
        
        return content
    
    def process_all_interfaces(self):
        """Process all interface files in the directory"""
        interface_files = list(self.interface_dir.glob("*-i.h"))
        
        print(f"Found {len(interface_files)} interface files")
        
        all_interfaces = []
        
        for file_path in interface_files:
            print(f"Processing {file_path.name}...")
            interfaces = self.parse_interface_file(file_path)
            
            for interface in interfaces:
                if interface.methods:  # Only process interfaces with methods
                    all_interfaces.append((interface, file_path.name))
                    
                    # Generate C API header
                    capi_content = self.generate_capi_header(interface, file_path.name)
                    
                    # Write to output file
                    output_file = self.output_dir / f"{interface.name}-capi.h"
                    with open(output_file, 'w', encoding='utf-8') as f:
                        f.write(capi_content)
                        
                    print(f"  Generated {output_file.name} with {len(interface.methods)} methods")
        
        # Update main header
        self._generate_main_header(all_interfaces)
        
        print(f"\nGenerated C API headers for {len(all_interfaces)} interfaces")
    
    def _generate_main_header(self, interfaces: List[Tuple[InterfaceDefinition, str]]):
        """Generate the main soui-capi.h header that includes all others"""
        content = """#ifndef __SOUI_CAPI_H__
#define __SOUI_CAPI_H__

/*
 * SOUI C API Helper Macros
 * 
 * This header provides C-style function call macros for all SOUI C++ interfaces.
 * Auto-generated by generate_capi.py - DO NOT EDIT MANUALLY
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Include all generated C API headers */
"""
        
        for interface, _ in interfaces:
            content += f'#include "{interface.name}-capi.h"\n'
            
        content += """
/* Common utility macros and functions */
#define SOUI_SUCCEEDED(hr) ((HRESULT)(hr) >= 0)
#define SOUI_FAILED(hr) ((HRESULT)(hr) < 0)

#ifdef __cplusplus
}
#endif

#endif /* __SOUI_CAPI_H__ */"""
        
        main_header = self.output_dir / "soui-capi.h"
        with open(main_header, 'w', encoding='utf-8') as f:
            f.write(content)
            
        print(f"Updated {main_header.name}")

def main():
    """Main entry point"""
    script_dir = Path(__file__).parent
    interface_dir = script_dir.parent
    output_dir = script_dir
    
    generator = CAPIGenerator(str(interface_dir), str(output_dir))
    generator.process_all_interfaces()

if __name__ == "__main__":
    main()
