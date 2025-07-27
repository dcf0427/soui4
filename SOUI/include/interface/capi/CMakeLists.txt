# SOUI C API Helper Macros
# CMake configuration for integrating C API macros

cmake_minimum_required(VERSION 3.10)

# Create an interface library for SOUI C API
add_library(soui_capi INTERFACE)

# Set the include directories
target_include_directories(soui_capi INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

# Define compile definitions for C interface usage
target_compile_definitions(soui_capi INTERFACE
    CINTERFACE  # Force C-style interface definitions
)

# Install headers
install(FILES
    soui-capi.h
    SAccelerator-capi.h
    STimer-capi.h
    SRender-capi.h
    SWindow-capi.h
    README.md
    DESTINATION include/soui/capi
)

# Optional: Build example (only if building examples is enabled)
option(BUILD_SOUI_CAPI_EXAMPLE "Build SOUI C API example" OFF)

if(BUILD_SOUI_CAPI_EXAMPLE)
    # Create example executable
    add_executable(soui_capi_example example.c)
    
    # Link with SOUI C API
    target_link_libraries(soui_capi_example PRIVATE soui_capi)
    
    # Set C standard
    set_property(TARGET soui_capi_example PROPERTY C_STANDARD 99)
    
    # Add compile definitions
    target_compile_definitions(soui_capi_example PRIVATE
        CINTERFACE
        _CRT_SECURE_NO_WARNINGS  # For MSVC
    )
    
    # Install example
    install(TARGETS soui_capi_example
        RUNTIME DESTINATION bin/examples
    )
    
    install(FILES example.c
        DESTINATION share/soui/examples
    )
endif()

# Function to help other projects use SOUI C API
function(target_use_soui_capi target_name)
    # Link with SOUI C API interface library
    target_link_libraries(${target_name} PRIVATE soui_capi)
    
    # Ensure C interface definitions are used
    target_compile_definitions(${target_name} PRIVATE CINTERFACE)
    
    # Set C standard if not already set
    get_target_property(target_type ${target_name} TYPE)
    if(target_type STREQUAL "EXECUTABLE" OR target_type STREQUAL "STATIC_LIBRARY" OR target_type STREQUAL "SHARED_LIBRARY")
        get_target_property(c_standard ${target_name} C_STANDARD)
        if(NOT c_standard)
            set_property(TARGET ${target_name} PROPERTY C_STANDARD 99)
        endif()
    endif()
endfunction()

# Export the target for use by other projects
export(TARGETS soui_capi FILE soui_capi-targets.cmake)

# Create a config file for find_package
include(CMakePackageConfigHelpers)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/soui_capi-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/soui_capi-config.cmake"
    INSTALL_DESTINATION lib/cmake/soui_capi
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/soui_capi-config-version.cmake"
    VERSION 1.0.0
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/soui_capi-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/soui_capi-config-version.cmake"
    DESTINATION lib/cmake/soui_capi
)

install(EXPORT soui_capi-targets
    FILE soui_capi-targets.cmake
    DESTINATION lib/cmake/soui_capi
)

# Print configuration summary
message(STATUS "SOUI C API Configuration:")
message(STATUS "  Include directory: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "  Build example: ${BUILD_SOUI_CAPI_EXAMPLE}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")

# Usage instructions
message(STATUS "")
message(STATUS "To use SOUI C API in your project:")
message(STATUS "  1. Add to your CMakeLists.txt:")
message(STATUS "     find_package(soui_capi REQUIRED)")
message(STATUS "     target_use_soui_capi(your_target)")
message(STATUS "  2. Include in your C code:")
message(STATUS "     #include <soui-capi.h>")
message(STATUS "")

# Create the config file template
file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/soui_capi-config.cmake.in" "
@PACKAGE_INIT@

include(\"\${CMAKE_CURRENT_LIST_DIR}/soui_capi-targets.cmake\")

# Function to help other projects use SOUI C API
function(target_use_soui_capi target_name)
    target_link_libraries(\${target_name} PRIVATE soui_capi)
    target_compile_definitions(\${target_name} PRIVATE CINTERFACE)
    
    get_target_property(target_type \${target_name} TYPE)
    if(target_type STREQUAL \"EXECUTABLE\" OR target_type STREQUAL \"STATIC_LIBRARY\" OR target_type STREQUAL \"SHARED_LIBRARY\")
        get_target_property(c_standard \${target_name} C_STANDARD)
        if(NOT c_standard)
            set_property(TARGET \${target_name} PROPERTY C_STANDARD 99)
        endif()
    endif()
endfunction()

check_required_components(soui_capi)
")

# Add custom target for generating additional C API headers
add_custom_target(generate_soui_capi
    COMMAND ${CMAKE_COMMAND} -E echo "Generating additional SOUI C API headers..."
    COMMAND python generate_capi.py || echo "Python not available, skipping auto-generation"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating SOUI C API headers"
)

# Add custom target for cleaning generated files
add_custom_target(clean_generated_capi
    COMMAND ${CMAKE_COMMAND} -E remove -f *-capi.h
    COMMAND ${CMAKE_COMMAND} -E echo "Cleaned generated C API headers"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Cleaning generated SOUI C API headers"
)
