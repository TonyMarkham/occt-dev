# Third-party notices — OCCT 8.0.1 (Windows x64)

These notices cover the binaries contained in the
`occt-8.0.1-x86_64-windows-*.zip` archives built from this repository.

## Open CASCADE Technology (OCCT)

- **Source:** <https://github.com/Open-Cascade-SAS/OCCT>, tag `V8.0.1`
  (commit `b8f597c677811d1f9f4d8a97f5ae2825c0353a42`), built unmodified.
- **License:** GNU Lesser General Public License version 2.1, with the
  OCCT LGPL exception.
- **Full license texts, included in every archive:**
  `LICENSE_LGPL_21.txt`, `OCCT_LGPL_EXCEPTION.txt` (verbatim copies of
  the files at the source root above). The dev archive additionally
  carries them under `share\doc\opencascade\`.
- **Binary provenance:** built 2026-10-13 on Windows x64 with MSVC
  19.44.35229 (Visual Studio 2022 17.14, vc14 toolset), CMake + Ninja,
  Release, shared libraries (DLLs). Configure switches:
  `BUILD_MODULE_Draw=OFF`, `USE_FREETYPE=OFF`,
  `INSTALL_DIR_LAYOUT=Unix` — full command in `occt-win.md`, section 2.

## Bundled third-party code inside the built toolkits

- **None.** A scan of the `V8.0.1` source tree found no vendored
  third-party components in the built toolkits. (The gl2ps library,
  bundled inside TKOpenGl in older OCCT releases, is not present in
  8.0.1, and TKOpenGl's dependency list references only OCCT toolkits
  and Windows system libraries.)

## External libraries linked by the binaries

- **None.** No external third-party libraries are linked. Optional
  third-party support (FreeType, Tcl/Tk, FreeImage, TBB, RapidJSON,
  Draco, VTK, FFmpeg, OpenVR, jemalloc) is compiled out in this build
  (see the configure switches above), and the only non-OCCT libraries
  referenced are Windows operating-system components (`opengl32`,
  `user32`, `gdi32`, `advapi32`, ...), supplied by the OS.
