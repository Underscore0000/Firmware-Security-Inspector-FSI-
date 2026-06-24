cmake_minimum_required(VERSION 3.20)
project(FirmwareSecurityInspector VERSION 1.0.0 LANGUAGES C CXX ASM_NASM)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================================
# Build options
# ============================================================================
option(BUILD_TESTS "Build test executables" OFF)
option(BUILD_CLI "Build CLI tool" ON)

# ============================================================================
# Platform detection - WINDOWS ONLY
# ============================================================================
if(NOT WIN32)
    message(FATAL_ERROR "This CMakeLists.txt is for Windows only!")
endif()

set(PLATFORM "Windows")
set(PLATFORM_SOURCES src/platform/windows_platform.c)

# ============================================================================
# NASM - Windows x64
# ============================================================================
enable_language(ASM_NASM)
set(CMAKE_ASM_NASM_FLAGS "-f win64")

# ============================================================================
# Qt6
# ============================================================================
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
qt_standard_project_setup()

# ============================================================================
# ASM sources
# ============================================================================
set(ASM_SOURCES
    src/asm/cpuid.asm
    src/asm/registers.asm
    src/asm/msr.asm
    src/asm/rdtsc.asm
)

# ============================================================================
# C sources (core)
# ============================================================================
set(CORE_SOURCES
    src/modules/cpu.c
    src/modules/registers.c
    src/modules/msr.c
    src/modules/firmware.c
    src/modules/secureboot.c
    src/modules/tpm.c
    src/modules/acpi.c
    src/modules/smbios.c
    src/modules/pcie.c
    src/modules/memory.c
    src/modules/security_audit.c
    src/report/report.c
    src/report/report_json.c
    src/report/report_html.c
    src/report/report_txt.c
    src/snapshot/snapshot.c
    src/snapshot/snapshot_diff.c
    ${PLATFORM_SOURCES}
)

# ============================================================================
# GUI sources (C++)
# ============================================================================
set(GUI_SOURCES
    src/gui/main.cpp
    src/gui/mainwindow.cpp
    src/gui/widgets/dashboard_widget.cpp
    src/gui/widgets/cpu_widget.cpp
    src/gui/widgets/msr_widget.cpp
    src/gui/widgets/acpi_widget.cpp
    src/gui/widgets/security_audit_widget.cpp
    src/gui/widgets/developer_widget.cpp
    src/gui/widgets/hex_viewer.cpp
    src/gui/widgets/firmware_widget.cpp
    src/gui/widgets/secureboot_widget.cpp
    src/gui/widgets/tpm_widget.cpp
    src/gui/widgets/smbios_widget.cpp
    src/gui/widgets/pcie_widget.cpp
    src/gui/widgets/memory_widget.cpp
    src/gui/widgets/registers_widget.cpp
)

# ============================================================================
# Include directories
# ============================================================================
set(INCLUDE_DIRS
    src/modules
    src/report
    src/snapshot
    src/platform
    src/gui
    src/gui/widgets
)

# ============================================================================
# TARGET: fsi (main GUI)
# ============================================================================
add_executable(fsi WIN32
    ${ASM_SOURCES}
    ${CORE_SOURCES}
    ${GUI_SOURCES}
)

target_include_directories(fsi PRIVATE ${INCLUDE_DIRS})
target_compile_definitions(fsi PRIVATE _WIN32)

if(WIN32)
    target_link_libraries(fsi PRIVATE Qt6::Core Qt6::Widgets setupapi)
else()
    target_link_libraries(fsi PRIVATE Qt6::Core Qt6::Widgets)
endif()

# ============================================================================
# TARGET: fsi-cli
# ============================================================================
if(BUILD_CLI)
    add_executable(fsi-cli
        ${ASM_SOURCES}
        ${CORE_SOURCES}
        src/cli/cli_main.c
    )

    target_include_directories(fsi-cli PRIVATE ${INCLUDE_DIRS})
    target_compile_definitions(fsi-cli PRIVATE _WIN32)

    if(WIN32)
        target_link_libraries(fsi-cli PRIVATE setupapi)
    endif()
endif()

# ============================================================================
# Build message
# ============================================================================
message(STATUS "")
message(STATUS "========================================")
message(STATUS "  FIRMWARE SECURITY INSPECTOR")
message(STATUS "========================================")
message(STATUS "  Platform:     ${PLATFORM}")
message(STATUS "  Build Type:   ${CMAKE_BUILD_TYPE}")
message(STATUS "  ASM Flags:    ${CMAKE_ASM_NASM_FLAGS}")
message(STATUS "  Qt Version:   ${Qt6_VERSION}")
message(STATUS "  Build CLI:    ${BUILD_CLI}")
message(STATUS "========================================")
message(STATUS "")