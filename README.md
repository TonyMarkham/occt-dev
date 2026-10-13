# occt-dev

Reproducible builds of **Open CASCADE Technology 8.0.1** (upstream tag
[`V8.0.1`](https://github.com/Open-Cascade-SAS/OCCT/releases/tag/V8.0.1),
commit `b8f597c`) with per-platform build guides, a headless validation
harness, and the packaged results attached to this repository's
[Releases](https://github.com/TonyMarkham/occt-dev/releases).

Three platforms, one recipe structure, one shared relative install
layout (`INSTALL_DIR_LAYOUT=Unix`):

| Guide | Target | Toolchain | OCCT's compiler floor |
|---|---|---|---|
| [`occt-win.md`](occt-win.md) | Windows x64 | MSVC (VS 2019/2022, `vc14` toolsets) | MSVC ≥ 19.14 |
| [`occt-linux.md`](occt-linux.md) | Linux x86_64 | GCC (Ubuntu 26.04 LTS base) | GCC ≥ 8.0 |
| [`occt-mac.md`](occt-mac.md) | macOS arm64 | Apple Clang (Xcode Command Line Tools) | Apple Clang ≥ 11.0.0 |

All guides assume CMake ≥ 3.10 (`BUILD_USE_PCH` needs ≥ 3.16) and the
Ninja generator, and produce Release shared libraries built as C++17.

## What ships in the release

Six archives — per platform a **dev** zip (the full install tree:
shared libraries, headers, CMake package config, resources, env
scripts) and a **run-only** zip (just the shared libraries plus the
three legal texts, flattened at the zip root):

| Archive | Contents |
|---|---|
| `occt-8.0.1-x86_64-windows-dev.zip` | 50 toolkit DLLs, MSVC import libraries, headers, `lib/cmake/opencascade`, resources, `env.bat` |
| `occt-8.0.1-x86_64-windows-dlls-only.zip` | the 50 DLLs, flattened |
| `occt-8.0.1-x86_64-linux-dev.zip` | 50 unversioned `libTK*.so`, headers, CMake package config, resources, `env.sh` |
| `occt-8.0.1-x86_64-linux-libs-only.zip` | the 50 `.so` files, flattened |
| `occt-8.0.1-arm64-macos-dev.zip` | 50 unversioned `libTK*.dylib`, headers, CMake package config, resources, `env.sh` |
| `occt-8.0.1-arm64-macos-libs-only.zip` | the 50 `.dylib` files, flattened |

Build configuration on every platform: `BUILD_MODULE_Draw=OFF` (no
DRAW harness, Tcl/Tk never searched), `USE_FREETYPE=OFF` (no text
rendering), Release, shared libraries. No third-party code is bundled
in the binaries — see
[`third_party/occt/THIRD_PARTY_NOTICES.md`](third_party/occt/THIRD_PARTY_NOTICES.md),
which ships at the root of every archive.

## Using the dev archives

Extract anywhere and point CMake at the extraction root. Consumers
must build as C++17 (OCCT 8.0 requires it on both sides):

```cmake
find_package(OpenCASCADE REQUIRED)  # -DCMAKE_PREFIX_PATH=<extracted root>
target_link_libraries(my_app PRIVATE TKernel TKMath TKBRep TKPrim TKTopAlgo TKXSBase TKDESTEP)
```

The shared libraries must be findable at runtime:

- **Windows** — the DLLs from `bin\` on `PATH` (or next to the exe).
- **Linux** — `lib/` on `LD_LIBRARY_PATH`.
- **macOS** — `lib/` on `DYLD_LIBRARY_PATH` (the dylibs use `@rpath`
  install names).

The run-only zips are for *running* something already built against
these libraries — extract next to the executable or onto the platform's
library search path. They are not self-sufficient for building.

## Building from source

Each guide is self-contained: prerequisites → source patch (where
required) → configure/build/install → validation → packaging.

- **Windows** needs no source modification.
- **Linux and macOS** apply exactly one local modification (section 2.1
  of each guide): the `SOVERSION`/`VERSION` target properties in
  `adm/cmake/occt_toolkit.cmake` are removed so the toolkits build as
  plain unversioned libraries with no symlink chain. The modification
  is recorded in a dated comment in the source and disclosed in
  `THIRD_PARTY_NOTICES.md` (LGPL).

## Validation

`sample/` is the harness every guide runs after installing: it builds a
10 × 20 × 30 mm box, writes it to a STEP file, reads the file back, and
asserts the volume survived the round trip:

```
STEP written: hello_occt.step
STEP read back: 1 root(s)
Volume: 6000 mm^3
```

with exit code 0. That proves CMake package discovery, the C++17
headers, the shared libraries at runtime, and a real STEP round trip —
end to end.

## Repository layout

```
occt-win.md          Windows build recipe
occt-linux.md        Linux build recipe
occt-mac.md          macOS build recipe
sample/              headless validation consumer (hello_occt.cpp, CMakeLists.txt)
third_party/occt/    verbatim license texts from the source tag + THIRD_PARTY_NOTICES.md
dist/                gitignored — installed trees and packaged zips
tmp/                 gitignored — source checkouts, build trees, logs
```

## License

OCCT is LGPL-2.1 with the OCCT LGPL exception. The verbatim texts
(`LICENSE_LGPL_21.txt`, `OCCT_LGPL_EXCEPTION.txt`) live in
`third_party/occt/` and ship at the root of every archive; dev archives
additionally carry them under `share/doc/opencascade/`.

FreeType-based text rendering is deliberately off in these builds; each
guide documents how to enable it against the platform's FreeType
(Homebrew on macOS, the system packages on Linux, the official
third-party bundle on Windows).
