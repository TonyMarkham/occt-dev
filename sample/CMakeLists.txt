# Headless validation consumer for the OCCT platform docs (occt-win.md).
#
# Configure against the guide's install (from the repo root, in Developer
# PowerShell for VS 2022):
#   cmake -S sample -B sample\build -G Ninja -DCMAKE_BUILD_TYPE=Release "-DCMAKE_PREFIX_PATH=$PWD\dist\occt\x86_64-windows"
#
# Expected run output ends with "Volume: 6000 mm^3" and exit code 0
# (see occt-win.md, section 3).

cmake_minimum_required (VERSION 3.16)

project (hello_occt LANGUAGES CXX)

# OCCT 8.0 requires C++17 on both sides: the libraries are built as C++17
# and the headers expect a C++17 consumer.
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

# No COMPONENTS list: OpenCASCADEConfig.cmake then imports every enabled
# module's toolkit targets. Requesting a subset leaves targets referenced
# through the toolkit dependency chain (TKCAF, TKService, ...) undefined
# and configure fails with "imported targets are referenced, but are
# missing".
find_package (OpenCASCADE REQUIRED)

add_executable (hello_occt hello_occt.cpp)

# Toolkits whose APIs the sample calls directly; the rest of the OCCT
# dependency chain (TKShHealing, TKCAF, TKXCAF, ...) arrives through the
# imported targets' interface dependencies.
target_link_libraries (hello_occt PRIVATE
                       TKernel TKMath TKBRep TKPrim TKTopAlgo TKXSBase TKDESTEP)
