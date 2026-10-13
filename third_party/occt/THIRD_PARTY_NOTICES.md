# Third-party notices — OCCT 8.0.1

These notices cover the binaries contained in the
`occt-8.0.1-x86_64-windows-*.zip`, `occt-8.0.1-x86_64-linux-*.zip`, and
`occt-8.0.1-arm64-macos-*.zip` archives built from this repository.

## Open CASCADE Technology (OCCT)

- **Source:** <https://github.com/Open-Cascade-SAS/OCCT>, tag `V8.0.1`
  (commit `b8f597c677811d1f9f4d8a97f5ae2825c0353a42`).
  - **Windows build:** unmodified source.
  - **Linux build:** one local modification — the `SOVERSION`/`VERSION`
    target properties in `adm/cmake/occt_toolkit.cmake` are removed so
    the toolkits build as plain unversioned `libTK*.so` files (see
    `occt-linux.md`, section 2.1). The modification is part of the
    build inputs, not a binary patch.
  - **macOS build:** the same one local modification as Linux — the
    `SOVERSION`/`VERSION` target properties in
    `adm/cmake/occt_toolkit.cmake` are removed so the toolkits build as
    plain unversioned `libTK*.dylib` files (see `occt-mac.md`, section
    2.1). The modification is part of the build inputs, not a binary
    patch.
- **License:** GNU Lesser General Public License version 2.1, with the
  OCCT LGPL exception.
- **Full license texts, included in every archive:**
  `LICENSE_LGPL_21.txt`, `OCCT_LGPL_EXCEPTION.txt` (verbatim copies of
  the files at the source root above). Each dev archive additionally
  carries them under `share/doc/opencascade/` (all three guides
  configure `INSTALL_DIR_LAYOUT=Unix`, so every install shares the same
  relative layout).
- **Binary provenance:**
  - **Windows x64:** built 2026-10-13 with MSVC 19.44.35229 (Visual
    Studio 2022 17.14, vc14 toolset), CMake + Ninja, Release, shared
    libraries (DLLs). Configure switches: `BUILD_MODULE_Draw=OFF`,
    `USE_FREETYPE=OFF`, `INSTALL_DIR_LAYOUT=Unix` — full command in
    `occt-win.md`, section 2.
  - **Linux x86_64:** built 2026-10-13 on Ubuntu 26.04 LTS with GCC
    15.2.0, CMake + Ninja, Release, shared libraries (.so). Configure
    switches: `BUILD_MODULE_Draw=OFF`, `USE_FREETYPE=OFF`,
    `INSTALL_DIR_LAYOUT=Unix` — full command in `occt-linux.md`,
    section 2.2.
  - **macOS arm64:** built 2026-10-13 on macOS 27.0.1 with Apple clang
    21.0.0 (Command Line Tools 27.0.0), CMake 4.2.0 + Ninja, Release,
    shared libraries (.dylib). Configure switches:
    `BUILD_MODULE_Draw=OFF`, `USE_FREETYPE=OFF`,
    `INSTALL_DIR_LAYOUT=Unix` — full command in `occt-mac.md`,
    section 2.2.

## Bundled third-party code inside the built toolkits

- **None.** A scan of the `V8.0.1` source tree found no vendored
  third-party components in the built toolkits. (The gl2ps library,
  bundled inside TKOpenGl in older OCCT releases, is not present in
  8.0.1, and TKOpenGl's dependency list references only OCCT toolkits
  and system libraries.)

## External libraries linked by the binaries

- **Windows:** none redistributed. No external third-party libraries
  are linked. Optional third-party support (FreeType, Tcl/Tk,
  FreeImage, TBB, RapidJSON, Draco, VTK, FFmpeg, OpenVR, jemalloc) is
  compiled out in this build (see the configure switches above), and
  the only non-OCCT libraries referenced are Windows operating-system
  components (`opengl32`, `user32`, `gdi32`, `advapi32`, ...),
  supplied by the OS.
- **Linux:** none redistributed. The same optional third-party support
  is compiled out in this build (see the configure switches above).
  The only non-OCCT libraries linked are operating-system components
  supplied by Ubuntu: `GL` and `X11` (via `libgl1-mesa-dev` /
  `libx11-dev`), plus `pthread`, `rt`, `dl`, and `libm`/`libstdc++`
  from glibc/GCC — system-provided, none shipped in the archives.
- **macOS:** none redistributed. The same optional third-party support
  is compiled out in this build (see the configure switches above).
  The only non-OCCT libraries linked are operating-system components
  supplied by macOS: the AppKit, IOKit, and OpenGL frameworks, plus
  `objc` and libSystem — system-provided, none shipped in the archives.