# Handoff — rebuilding this dist on Linux (x86_64)

This is the working handoff for reproducing the Windows dist on a Linux
box. It assumes you have just cloned this repository and nothing else.
When the polished `occt-linux.md` gets written, it replaces this file.

Read the **differences first** — everything else follows the Windows
recipe (`occt-win.md`) step for step.

## The five differences from Windows

1. **A source patch is REQUIRED on Linux.** OCCT's `adm/cmake/occt_toolkit.cmake`
   sets `VERSION`/`SOVERSION` target properties, producing versioned
   `libTKernel.so.8.0.1` + symlink chains. On Windows those properties are
   irrelevant (DLLs are never versioned), so `occt-win.md` needs no patch —
   Linux needs it to get plain unversioned `libTK*.so` files. No stock flag
   does it (`BUILD_SOVERSION_NUMBERS` is overridden; its Linux default is 2).
   This is the only source modification; keep it recorded (step 2.1) — the
   modified source is part of LGPL redistribution provenance.
2. **The package registry is `~/.cmake/packages/`**, not a Windows registry.
   Same trap, different mechanism: OCCT's `export(PACKAGE)` runs on every
   successful configure and points consumers at `tmp/occt/build`. Harmless
   while consumers set `CMAKE_PREFIX_PATH`; the real trap is the consumer
   cache pinning `OpenCASCADE_DIR` (see Gotchas).
3. **Runtime library discovery is `LD_LIBRARY_PATH`**, not `PATH`. The
   sample exe finds `libTK*.so` via the environment variable, not the
   directory-of-the-exe.
4. **X11 + OpenGL dev packages are needed** — the toolkit dependency
   closure builds TKOpenGl (Visualization module stays enabled, mirroring
   the Windows dist), and on Linux TKOpenGl links `X11` + `GL` system
   libraries. That's two small `apt` packages, not a third-party bundle.
   FreeType AND fontconfig stay out via `-DUSE_FREETYPE=OFF`; Tcl/Tk stay
   out via `-DBUILD_MODULE_Draw=OFF`.
5. **Linux binaries are smaller by design.** OCCT's Release flags add `-s`
   (GCC) / `-Wl,-s` (Clang) — release `.so` files are symbol-stripped at
   link time, which Windows DLLs cannot be. A smaller dev zip than the
   Windows one is expected, not a defect.

## Prerequisites

Debian/Ubuntu-family (names differ elsewhere): compiler with C++17
(GCC ≥ 8 is hard-checked by OCCT with a fatal error), CMake ≥ 3.16,
Ninja, Git, and `zip` for the packaging section.

```bash
sudo apt install build-essential cmake ninja-build git zip libx11-dev libgl1-mesa-dev
```

## 1. Get the source

From the repo root (`~/occt-dev` or wherever you cloned this repo):

```bash
cd ~/occt-dev
```

```bash
git clone --depth 1 --branch V8.0.1 https://github.com/Open-Cascade-SAS/OCCT.git tmp/occt/OCCT
```

✅ Branch checkout note about detached HEAD is expected.

## 2. Configure, build, install to dist

### 2.1 Apply the unversioned-library patch

In `tmp/occt/OCCT/adm/cmake/occt_toolkit.cmake`, replace this block
(lines 207–209 at V8.0.1):

```cmake
set_target_properties (${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "${PRECOMPILED_DEFS}"
                                                  SOVERSION     "${OCC_SOVERSION}"
                                                  VERSION       "${OCC_VERSION_MAJOR}.${OCC_VERSION_MINOR}.${OCC_VERSION_MAINTENANCE}")
```

with:

```cmake
# Local modification 2026-10-13: removed SOVERSION/VERSION so that plain
# unversioned libTK*.so files are built and installed (no symlink chain).
set_target_properties (${PROJECT_NAME} PROPERTIES
                       COMPILE_FLAGS "${PRECOMPILED_DEFS}")
```

The comment is not decoration: it marks the one local modification that
must be preserved (and disclosed) when the binaries are redistributed.

### 2.2 Configure

```bash
cmake -S tmp/occt/OCCT -B tmp/occt/build -G Ninja \
      -DCMAKE_BUILD_TYPE=Release \
      -DINSTALL_DIR="$PWD/dist/occt/x86_64-linux" \
      -DINSTALL_DIR_LAYOUT=Unix \
      -DINSTALL_DIR_WITH_VERSION=OFF \
      -DBUILD_LIBRARY_TYPE=Shared \
      -DBUILD_MODULE_Draw=OFF \
      -DUSE_FREETYPE=OFF \
      2>&1 | tee tmp/occt/configure.log
```

(`INSTALL_DIR_LAYOUT=Unix` and `INSTALL_DIR_WITH_VERSION=OFF` are the
defaults on Linux; spelled out so the command matches the Windows one
switch for switch. In a pipeline the exit status comes from `tee` — run
`set -o pipefail` first if you script these.)

✅ Success: `Configuring done` / `Generating done`, and the log contains
**no** FreeType/X11 search lines.

### 2.3 Build — the 10–30 minute part

```bash
cmake --build tmp/occt/build --parallel 2>&1 | tee tmp/occt/build.log
```

✅ Success: `$LASTEXITCODE` has no meaning here — check `echo $?` is `0`
and that `grep -inE "error|fatal" tmp/occt/build.log` prints nothing.

### 2.4 Install

```bash
cmake --install tmp/occt/build 2>&1 | tee tmp/occt/install.log
```

### 2.5 Verify the install

Each command must succeed / print what's shown:

```bash
test -f dist/occt/x86_64-linux/lib/libTKernel.so && test ! -L dist/occt/x86_64-linux/lib/libTKernel.so && echo "unversioned OK"
```

```bash
test -f dist/occt/x86_64-linux/lib/cmake/opencascade/OpenCASCADEConfig.cmake && echo "cmake config OK"
```

```bash
test -f dist/occt/x86_64-linux/share/doc/opencascade/LICENSE_LGPL_21.txt && echo "license OK"
```

✅ All three print their OK messages. The unversioned check is the proof
the 2.1 patch took effect.

## 3. Validate with sample/

Same harness, same expected output as `occt-win.md` section 3
(`STEP written:` / `STEP read back: 1 root(s)` / `Volume: 6000 mm^3`,
exit code 0).

**3.1** Confirm the install exists:

```bash
test -f dist/occt/x86_64-linux/lib/cmake/opencascade/OpenCASCADEConfig.cmake && echo OK
```

**3.2** Clear any previous sample build (stale caches pin the wrong
OpenCASCADE):

```bash
rm -rf sample/build
```

**3.3** Configure against the install:

```bash
cmake -S sample -B sample/build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$PWD/dist/occt/x86_64-linux"
```

✅ `-- Found OpenCASCADE: .../dist/occt/x86_64-linux/lib/cmake/opencascade`.

**3.4** Build:

```bash
cmake --build sample/build --parallel
```

**3.5** Make the libraries findable at runtime (session-only):

```bash
export LD_LIBRARY_PATH="$PWD/dist/occt/x86_64-linux/lib:$LD_LIBRARY_PATH"
```

**3.6** Run:

```bash
cd sample/build && ./hello_occt && echo "exit=$?"
```

**3.7** Back to the repo root:

```bash
cd ../..
```

## 4. Package the install as zips

Same split as Windows: **dev** (full install tree) and **dlls-only**
(`lib/*.so`, flattened). Both zips carry the three legal files
(`LICENSE_LGPL_21.txt`, `OCCT_LGPL_EXCEPTION.txt`, `THIRD_PARTY_NOTICES.md`)
from `third_party/occt/` at the zip root.

**4.1** Provenance first — open `third_party/occt/THIRD_PARTY_NOTICES.md`
and update the *Binary provenance* block for this build (date, Linux,
GCC version, same configure switches). The current text describes the
Windows build; a Linux zip inheriting it would lie.

**4.2** Output folder:

```bash
mkdir -p dist/zips
```

**4.3** Dev zip (whole install tree, relative layout preserved):

```bash
(cd dist/occt/x86_64-linux && zip -qr ../../zips/occt-8.0.1-x86_64-linux-dev.zip .)
```

**4.4** Legal files at the dev zip root:

```bash
zip -j dist/zips/occt-8.0.1-x86_64-linux-dev.zip third_party/occt/THIRD_PARTY_NOTICES.md third_party/occt/LICENSE_LGPL_21.txt third_party/occt/OCCT_LGPL_EXCEPTION.txt
```

**4.5** DLLs-only zip (flattened `.so` + legal files at root):

```bash
cd dist/occt/x86_64-linux/lib
```

```bash
zip -qj ../../../zips/occt-8.0.1-x86_64-linux-dlls-only.zip '*.so'
```

```bash
zip -qj ../../../zips/occt-8.0.1-x86_64-linux-dlls-only.zip ~/occt-dev/third_party/occt/THIRD_PARTY_NOTICES.md ~/occt-dev/third_party/occt/LICENSE_LGPL_21.txt ~/occt-dev/third_party/occt/OCCT_LGPL_EXCEPTION.txt
```

```bash
cd ~/occt-dev
```

**4.6** Check:

```bash
ls -la dist/zips
```

✅ Two zips, non-zero; dev larger than dlls-only.

## 5. Cleanup

```bash
rm -rf tmp/occt
```

Leaves `dist/occt/x86_64-linux` and the zips. Re-cloning the stock tag
does NOT restore the 2.1 patch — reapply it from this handoff.

## Gotchas (the ones that bit on Windows, in Linux clothing)

- **Do not configure the sample before the install exists.** OCCT's
  `export(PACKAGE)` registers the build tree in `~/.cmake/packages/`
  on every successful configure; a consumer whose prefix misses falls
  back to it, loads the build-tree config, fails on missing per-module
  target files, and — worse — caches `OpenCASCADE_DIR` in
  `sample/build/CMakeCache.txt`. Every re-run then replays the failure
  even after the install appears. Recovery: delete `sample/build`,
  reconfigure with the prefix set (prefix wins over the registry).
- **No `-DCMAKE_EXPORT_PACKAGE_REGISTRY=OFF`.** It does nothing against
  OCCT's explicit `export(PACKAGE)` call and only prints an
  unused-variable warning.
- **Module toggles don't prune the build.** The top-level CMakeLists
  follows the transitive toolkit closure, so TKService/TKV3d build even
  with `BUILD_MODULE_Visualization=OFF`, and TKXCAF→TKV3d drags the
  FreeType demand in. That's why FreeType is switched off with
  `-DUSE_FREETYPE=OFF` (stock, compile-guarded), not by disabling the
  module.
- **Fresh build dir** if a previous configure attempt failed mid-way.
- **Swapping tags**: re-check the patch block and the third-party scan
  (`AGENTS.md` documents both), then re-run section 4 — zip names pin
  the version.
