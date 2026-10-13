# Building Open CASCADE Technology on macOS

This repository builds **OCCT 8.0.1** (tag `V8.0.1`) from source on
**macOS (Apple Silicon, arm64)** and installs the result into the repo's
gitignored `dist/` tree. The install is validated with the headless
consumer in `sample/`, which builds a 10 × 20 × 30 mm box, writes it to a
STEP file, reads it back, and asserts `Volume: 6000 mm^3`.

The repository keeps one recipe per platform with a shared structure
(`occt-win.md`, `occt-linux.md`); where content overlaps, keep the docs
consistent.

- Repo: <https://github.com/Open-Cascade-SAS/OCCT>
- Tag: <https://github.com/Open-Cascade-SAS/OCCT/releases/tag/V8.0.1>
- License: LGPL-2.1 with exception (`LICENSE_LGPL_21.txt` +
  `OCCT_LGPL_EXCEPTION.txt` in the source; both are installed into the
  install tree)

> ## Prerequisites
>
> Everything comes from two sources on macOS — Apple's **Xcode Command
> Line Tools** (compiler, git) and **Homebrew** (CMake, Ninja). No
> third-party bundles to download; `zip` ships with macOS itself.
>
> 1. Install the Command Line Tools (skip when a full Xcode or the CLT
>    is already installed):
>
>    ```
>    xcode-select --install
>    ```
>
>    A GUI prompt appears on a bare machine; if the tools are already
>    present, the command prints a note saying so. Either outcome is
>    fine — the `clang` check below is the gate.
> 2. Install Homebrew if absent (see <https://brew.sh> for the current
>    installer line), then:
>
>    ```
>    brew install cmake ninja
>    ```
>
>    If this aborts with `Your Xcode ... is too outdated` or `Your
>    Command Line Tools are too outdated` on a new macOS, that is
>    Homebrew's own toolchain policy, not this guide's requirement —
>    see Troubleshooting.
>
> The pieces, and why each is here:
>
> - Xcode CLT — Apple clang with C++17 support, plus `git`. OCCT
>   hard-checks the compiler version with a fatal error: *Apple Clang
>   version 11.0.0 or newer is required for C++17 support*
>   (`adm/cmake/occt_macros.cmake` at V8.0.1). Any Command Line Tools
>   release of the last several years clears it; the machine this guide
>   was written against runs macOS 27.0.1. Keep Xcode and the Command
>   Line Tools at consistent versions — a mixed state breaks linking
>   before OCCT is even involved (the recovery steps are in section
>   2.2a).
> - `cmake` — OCCT declares `cmake_minimum_required (VERSION 3.10)` at
>   V8.0.1, which CMake 4 accepts (CMake 4 removed compatibility only
>   below 3.5). `BUILD_USE_PCH` additionally needs CMake ≥ 3.16.
> - `ninja` — the Ninja build generator.
> - `/usr/bin/zip` — Info-ZIP, part of macOS itself; used by the
>   packaging section. Nothing to install; the check below confirms it.
>
> No X11 or OpenGL development packages are needed — the prerequisite
> set is smaller than Linux's. Verified at the tag: the top-level
> `CMakeLists.txt` sets `USE_XLIB` OFF explicitly on APPLE, and OCCT's
> Apple CSF table (`occt_set_apple_csf_vars()` in
> `adm/cmake/occt_csf.cmake`) maps the windowing/graphics CSF variables
> to system frameworks only — AppKit, IOKit, `objc`, and the OpenGL
> framework, all supplied by macOS.
>
> Everything else the build touches is OCCT itself: FreeType stays out
> with the stock `-DUSE_FREETYPE=OFF` (see the Windows guide's *Why no
> third-party bundle?* — the same toolkit-closure logic applies here;
> the stock kill switch compile-guards the font code via
> `HAVE_FREETYPE`), and Tcl/Tk is never searched because
> `BUILD_MODULE_Draw=OFF` keeps DRAWEXE out of the toolkit list.
>
> Run every command from the repository root:
>
> ```
> cd /Users/tony/git/occt-dev
> ```
>
> After installing, confirm the toolchain resolves. Versions printed
> here are informational — anything the CLT and Homebrew give you clears
> OCCT's floors (Apple Clang ≥ 11.0.0, CMake ≥ 3.10), so this is a check
> that the installs above actually happened, not a gate:
>
> ```
> clang --version | head -1
> ```
>
> ```
> cmake --version | head -1
> ```
>
> ```
> ninja --version
> ```
>
> ```
> zip -v 2>&1 | head -1
> ```
>
> ```
> xcrun --show-sdk-path
> ```
>
> The SDK this prints must belong to the same toolchain as the clang
> above — it should live under the developer directory that
> `xcode-select -p` names. A path under a *different* installation
> (Xcode selected, SDK under `/Library/Developer/CommandLineTools`, or
> the reverse) is the mixed state that breaks linking — the recovery
> steps are in section 2.2a.
>
> **A source patch is required on macOS.** Unlike the Windows guide,
> this guide makes exactly one local modification to the OCCT source:
> `adm/cmake/occt_toolkit.cmake` sets `VERSION`/`SOVERSION` target
> properties, which on Mach-O produce versioned
> `libTKernel.8.0.1.dylib` files plus a symlink chain
> (`libTKernel.dylib → libTKernel.8.dylib → libTKernel.8.0.1.dylib`).
> This repository installs plain unversioned `libTK*.dylib` files, so
> the patch (section 2.1) removes both properties. No stock flag
> achieves this: `BUILD_SOVERSION_NUMBERS` is forced to `2` on
> non-Windows platforms by the top-level CMakeLists regardless of the
> cache value (macOS counts as non-Windows there), and `VERSION` is
> always set. The modification is kept recorded in a dated comment — it
> must be disclosed when the binaries are redistributed (LGPL).

> ## 1. Get the OCCT source at the release tag
>
> Clone the tag into the repo's gitignored source/build area:
>
> ```
> git clone --depth 1 --branch V8.0.1 https://github.com/Open-Cascade-SAS/OCCT.git tmp/occt/OCCT
> ```
>
> Git prints a detached-HEAD note (`You are in 'detached HEAD' state`);
> that is expected when checking out a tag and can be ignored. If
> `tmp/occt/OCCT` already exists, use the existing `V8.0.1` checkout
> rather than cloning over it — and make sure the section-2.1 patch is
> present in it (verify with the `grep` in 2.1).

> ## 2. Configure, build, install to the repo's dist
>
> Confirm step 1's clone is where the commands expect it (prints `clone
> OK`); the dist directory is allowed to not exist until 2.4:
>
> ```
> test -d tmp/occt/OCCT && echo "clone OK"
> ```
>
> OCCT installs directly into `dist/occt/arm64-macos`; there is no
> external install and no copy step afterwards. The install prefix is
> **baked in at configure time** — the generated `env.sh`/`custom.sh`
> and CMake package files all derive from it, so
> `cmake --install --prefix <elsewhere>` afterwards does not relocate
> the install. Decide the prefix now.
>
> Each stage command pipes its output through `tee` into a log file in
> `tmp/occt/` — `configure.log`, `build.log`, `install.log` — so
> progress streams to the console while the full output is captured for
> sharing a failed stage. In a pipeline the exit status comes from
> `tee`; run `set -o pipefail` first when scripting these stages (both
> `bash` and the default `zsh` support it), and otherwise check
> `echo $?` right after each stage — it must be `0`.

> ### 2.1 Apply the unversioned-library patch
>
> In `tmp/occt/OCCT/adm/cmake/occt_toolkit.cmake`, replace this block
> (lines 207–209 at V8.0.1, verified against the tag):
>
> ```cmake
> set_target_properties (${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "${PRECOMPILED_DEFS}"
>                                                   SOVERSION     "${OCC_SOVERSION}"
>                                                   VERSION       "${OCC_VERSION_MAJOR}.${OCC_VERSION_MINOR}.${OCC_VERSION_MAINTENANCE}")
> ```
>
> with:
>
> ```cmake
> # Local modification 2026-10-13: removed SOVERSION/VERSION so that plain
> # unversioned libTK*.dylib files are built and installed (no symlink chain).
> set_target_properties (${PROJECT_NAME} PROPERTIES
>                        COMPILE_FLAGS "${PRECOMPILED_DEFS}")
> ```
>
> The comment is not decoration: it marks the one local modification
> that must be preserved (and disclosed) when the binaries are
> redistributed. Verify the patch is in place (must print the dated
> comment line):
>
> ```
> grep -n "Local modification" tmp/occt/OCCT/adm/cmake/occt_toolkit.cmake
> ```
>
> and that the original property lines are gone (must print nothing):
>
> ```
> grep -n "SOVERSION     \"" tmp/occt/OCCT/adm/cmake/occt_toolkit.cmake
> ```
>
> Two blocks nearby are stock OCCT code and must be left alone: the
> APPLE-only `BUILD_WITH_INSTALL_RPATH`/`INSTALL_NAME_DIR` block (line
> 189 at V8.0.1) stays dormant because `INSTALL_NAME_DIR` defaults to
> the empty string, and the `if (NOT WIN32)` symlink-creation block
> stays dormant because `BUILD_SHARED_LIBRARY_NAME_POSTFIX` defaults to
> empty. Neither is related to this patch.

> ### 2.2 Configure — generates the build plan; nothing compiles yet
>
> ```
> cmake -S tmp/occt/OCCT -B tmp/occt/build -G Ninja \
>       -DCMAKE_BUILD_TYPE=Release \
>       -DINSTALL_DIR="$PWD/dist/occt/arm64-macos" \
>       -DINSTALL_DIR_LAYOUT=Unix \
>       -DINSTALL_DIR_WITH_VERSION=OFF \
>       -DBUILD_LIBRARY_TYPE=Shared \
>       -DBUILD_MODULE_Draw=OFF \
>       -DUSE_FREETYPE=OFF \
>       2>&1 | tee tmp/occt/configure.log
> ```
>
> The switches:
>
> - `-S tmp/occt/OCCT` / `-B tmp/occt/build` — source (the clone from
>   step 1) and build directory (all intermediate output; deleting it
>   deletes the build and nothing else).
> - `-G Ninja` — the Ninja generator; Homebrew's `ninja` supplies it.
> - `-DCMAKE_BUILD_TYPE=Release` — explicitly select an optimized build.
>   With a single-config generator this also keeps OCCT's release output
>   in unsuffixed directories (`lib`, not `lib`/`libd`/`libi`).
> - `-DINSTALL_DIR=...` — OCCT's own name for the install prefix (same
>   as `CMAKE_INSTALL_PREFIX`).
> - `-DINSTALL_DIR_LAYOUT=Unix` / `-DINSTALL_DIR_WITH_VERSION=OFF` —
>   both are **defaults on macOS too** (verified at the tag: the
>   top-level CMakeLists selects the Unix layout for every non-Windows
>   platform, APPLE included, and `INSTALL_DIR_WITH_VERSION` defaults to
>   OFF); spelled out so the command matches the Windows and Linux
>   guides switch for switch. Result: flat
>   `bin`/`lib`/`include/opencascade` layout, shared by all three
>   platform docs.
> - `-DBUILD_LIBRARY_TYPE=Shared` — build `.dylib` files to distribute.
>   `Shared` is the default; it is spelled out for clarity.
> - `-DBUILD_MODULE_Draw=OFF` — skip the DRAW test harness: DRAWEXE is
>   not built and Tcl/Tk is never searched. OCCT validation in this
>   guide goes through section 3 instead.
> - `-DUSE_FREETYPE=OFF` — OCCT's stock switch for building without
>   FreeType. It skips the FreeType search, leaves `CSF_FREETYPE` empty
>   (toolkit link lines skip it), and drops `-DHAVE_FREETYPE`, so the
>   font code in TKService compiles without `ft2build.h` (text rendering
>   becomes a no-op). TKService/TKV3d/TKOpenGl are still built (see
>   Prerequisites), so `lib/` contains the full toolkit set, minus font
>   support. Note that the Apple CSF path never pulls fontconfig —
>   `CSF_fontconfig` is set only in the UNIX branch of
>   `adm/cmake/occt_csf.cmake` — so even the FreeType-enabled variant of
>   this build needs no fontconfig on macOS (see *Enabling FreeType text
>   support*).
>
> Two Apple-clang specifics are stock at this tag, both visible in
> `adm/cmake/occt_defs_flags.cmake`: the build forces `-stdlib=libc++`
> (libstdc++ is outdated on macOS), and adds
> `-Wno-elaborated-enum-base` to quiet stricter warnings that newer
> Apple clang emits for Apple's own system headers. No action needed —
> listed so the flags in the build log are recognized.
>
> Two stock side effects of a successful configure, both harmless here:
> the build tree self-registers in the user's CMake package registry
> (`~/.cmake/packages/OpenCASCADE/...` — OCCT calls `export(PACKAGE)`
> unconditionally; `-DCMAKE_EXPORT_PACKAGE_REGISTRY=OFF` does *not*
> prevent it and only earns an unused-variable warning), and while the
> registry points at `tmp/occt/build`, consumers configured with
> `CMAKE_PREFIX_PATH` resolve the install before the registry is ever
> consulted. The real trap is a consumer *cache* pinning
> `OpenCASCADE_DIR` — see Troubleshooting.
>
> A successful configure ends with `Configuring done` / `Generating done`
> and **no** FreeType/X11 search lines in the log. If a previous
> configure attempt failed partway, delete the half-configured build
> directory first (`rm -rf tmp/occt/build`) so its stale cache cannot
> interfere. If you previously customized `INSTALL_DIR_LIB`,
> `INSTALL_DIR_INCLUDE`, or other install subdirectories in this build
> cache, make them match the switches above or use a fresh build
> directory.
>
> If configure instead fails with `Check for working C compiler ...
> - broken` and an `ld: tapi error`, go straight to section 2.2a.

> ### 2.2a Recovery — mixed Xcode / Command Line Tools (`ld: tapi error`)
>
> The machine's Xcode and Command Line Tools are mixed versions: the
> linker is older than the SDK it links against (`cc`/`xcrun` pick the
> SDK matching the running macOS from whichever installation has it;
> with Xcode 26.4 selected and Command Line Tools 27.0.0 installed,
> Xcode's old `ld` links against the CLT's `MacOSX27.0.sdk`, whose
> text-based stubs use the new `arm64e.x1` architecture tags). Select
> one consistent toolchain — the CLT 27.0.0 set — then rebuild the plan:
>
> **R1** Select the CLT (asks for your password):
>
> ```
> sudo xcode-select --switch /Library/Developer/CommandLineTools
> ```
>
> **R2** Confirm the selection moved. Must print
> `/Library/Developer/CommandLineTools`:
>
> ```
> xcode-select -p
> ```
>
> **R3** Confirm the linker followed. Must show `ld-27037.1`, not the
> Xcode-era `ld-1266.x`:
>
> ```
> ld -v 2>&1 | head -2
> ```
>
> **R4** Confirm the SDK resolves under the CLT. Must print
> `/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk`:
>
> ```
> xcrun --show-sdk-path
> ```
>
> **R5** Delete the half-configured build directory:
>
> ```
> rm -rf tmp/occt/build
> ```
>
> **R6** Re-run the configure command above and continue with 2.3.

> ### 2.3 Build — the 10–30 minute part on a typical machine (estimate)
>
> ```
> cmake --build tmp/occt/build --parallel 2>&1 | tee tmp/occt/build.log
> ```
>
> `--parallel` uses Ninja's default parallelism (all cores). To bound
> memory usage, give it a job count, for example `--parallel 8`. The
> build tree holds several GB of object files; keep roughly 20 GB free.
> When it finishes, `echo $?` must be `0` and
> `grep -inE "error|fatal" tmp/occt/build.log` should print nothing. If
> the build failed, resolve the first error in the log; rerunning
> install alone cannot produce missing files.

> ### 2.4 Install — copies headers, libraries, resources into the dist
>
> Run this only after the build has completed successfully. If reusing a
> dist from an older installation, first delete its obsolete `TK*.dylib`
> files; CMake does not remove stale installed files.
>
> ```
> cmake --install tmp/occt/build 2>&1 | tee tmp/occt/install.log
> ```
>
> Installed layout (`dist/occt/arm64-macos/`):
>
> ```
> <prefix>/
> ├── bin/                          env.sh, custom.sh, custom_clang_64.sh
> ├── lib/                          libTKernel.dylib ... (unversioned .dylib)
> │   └── cmake/opencascade/        OpenCASCADEConfig.cmake, targets files
> ├── include/opencascade/          public headers, Standard_Version.hxx
> └── share/
>     ├── opencascade/resources/    UnitsAPI, XSTEPResource, StdResource, ...
>     └── doc/opencascade/          LICENSE_LGPL_21.txt, OCCT_LGPL_EXCEPTION.txt
> ```
>
> Because of the 2.1 patch there are no version suffixes or symlink
> chains to check for — `libTKernel.dylib` is a plain file. Verify the
> install (each line must print its OK message):
>
> ```
> test -f dist/occt/arm64-macos/lib/libTKernel.dylib && test ! -L dist/occt/arm64-macos/lib/libTKernel.dylib && echo "unversioned OK"
> ```
>
> ```
> test -f dist/occt/arm64-macos/lib/cmake/opencascade/OpenCASCADEConfig.cmake && echo "cmake config OK"
> ```
>
> ```
> test -f dist/occt/arm64-macos/share/doc/opencascade/LICENSE_LGPL_21.txt && echo "license OK"
> ```
>
> All three must print OK — the first is the proof the 2.1 patch took
> effect. The CMake package resolves its install prefix from the config
> file's own location, so consumers can point `CMAKE_PREFIX_PATH` at the
> prefix above (or at any directory containing it) and
> `find_package(OpenCASCADE)` works.
>
> One more fact worth having before section 3 — what dyld sees:
>
> ```
> otool -D dist/occt/arm64-macos/lib/libTKernel.dylib
> ```
>
> prints the dylib's install name — expected
> `@rpath/libTKernel.dylib`. That is CMake's macOS default with OCCT's
> settings: OCCT sets no `INSTALL_NAME_DIR` (the default is the empty
> string, so the toolkit's APPLE block stays dormant), and with
> `cmake_minimum_required (VERSION 3.10)` the CMP0042 policy defaults to
> NEW, which makes `@rpath` the install name. Consumers therefore
> resolve the dylibs through the rpath entries baked into the binaries
> and/or `DYLD_LIBRARY_PATH` — never through absolute library paths.

> ## 3. Validate with sample/ (headless STEP round-trip)
>
> Section 2 left you a working OCCT install. This section proves it works
> by running a small program from `sample/` that builds a 10 × 20 × 30 mm
> box, writes it to a STEP file, reads that file back, and checks the
> volume survived the round trip. If the printed volume is 6000 mm^3,
> headers, shared libraries, and STEP support all work — end to end.
>
> The program is two files:
>
> - `sample/hello_occt.cpp` — the program: box → STEP → read back →
>   assert volume (silences OCCT's noisy transfer banner first).
> - `sample/CMakeLists.txt` — its build file: declares C++17, finds the
>   installed OCCT, links the seven toolkits whose APIs the program calls
>   (`TKernel TKMath TKBRep TKPrim TKTopAlgo TKXSBase TKDESTEP` —
>   everything else follows through toolkit dependencies).
>
> Run these commands from the repo root, in order, one at a time:
>
> **3.1** Confirm the install from section 2 is present. Must print `OK`
> — if it prints nothing, finish section 2 before continuing:
>
> ```
> test -f dist/occt/arm64-macos/lib/cmake/opencascade/OpenCASCADEConfig.cmake && echo OK
> ```
>
> **3.2** Remove any previous sample build (stale caches point at the
> wrong OpenCASCADE):
>
> ```
> rm -rf sample/build
> ```
>
> **3.3** Point the sample at the install and generate its build plan.
> Success: a line starting with `-- Found OpenCASCADE:` naming
> `dist/occt/arm64-macos/...`:
>
> ```
> cmake -S sample -B sample/build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$PWD/dist/occt/arm64-macos"
> ```
>
> If you instead see `include could not find requested file:
> ...OpenCASCADE<Module>Targets.cmake`, see Troubleshooting.
>
> **3.4** Compile the sample (seconds, not minutes):
>
> ```
> cmake --build sample/build --parallel
> ```
>
> **3.5** Make the OCCT libraries findable at runtime. The dylibs carry
> `@rpath` install names (see the `otool -D` note in 2.4), so dyld
> expands them against the rpath entries baked into the binaries; this
> export adds a directory dyld always searches first — the same thing
> OCCT's own `env.sh` sets up on macOS — and keeps the run deterministic
> regardless of how those entries came out. Lasts only for this session:
>
> ```
> export DYLD_LIBRARY_PATH="$PWD/dist/occt/arm64-macos/lib:$DYLD_LIBRARY_PATH"
> ```
>
> **3.6** Go where the built exe is:
>
> ```
> cd sample/build
> ```
>
> **3.7** Run the validation:
>
> ```
> ./hello_occt
> ```
>
> **3.8** Print the exit code. Must be `0`:
>
> ```
> echo $?
> ```
>
> Expected on screen after 3.7:
>
> ```
> STEP written: hello_occt.step
> STEP read back: 1 root(s)
> Volume: 6000 mm^3
> ```
>
> **3.9** Back to the repo root:
>
> ```
> cd ../..
> ```
>
> The volume assertion tolerates 0.01 mm^3; the analytic box volume is
> 6000 mm^3. The run proves the full loop: CMake finds the installed
> package through `CMAKE_PREFIX_PATH`, a C++17 consumer compiles against
> the OCCT 8.0 headers, the dylibs load from `lib/` (via the `@rpath`
> install names plus the `DYLD_LIBRARY_PATH` fallback from 3.5), and a
> real STEP round-trip preserves geometry. It does not exercise
> Visualization, viewers, or deployment to another machine — but nothing
> about the STEP path needs extra environment setup (`CSF_*` variables
> are not required for this round-trip).
>
> Note the program clears the default messenger's printers first:
> `Message::DefaultMessenger()->ChangePrinters().Clear()`. OCCT 8.0.1
> prints a `****** Statistics on Transfer (Write) ******` banner
> unconditionally during STEP transfer (from
> `XSControl_TransferWriter::TransferWriteShape` via the default
> messenger); clearing the printers keeps the sample's output
> deterministic. Do not remove that line when touching the sample.

> ## 4. Package the install as zips
>
> Section 2's install is a tree; this section turns it into two archive
> files under `dist/zips/`:
>
> - **dev** — everything: shared libraries, headers, CMake package
>   config, resources, `env.sh`/`custom.sh`/`custom_clang_64.sh`. For
>   consumers who build against OCCT.
> - **libs-only** — just the `lib/*.dylib` files, flattened at the zip
>   root. For consumers who only need to *run* something already built
>   against these libraries.
>
> Both archives carry the legal texts at the zip root
> (`LICENSE_LGPL_21.txt`, `OCCT_LGPL_EXCEPTION.txt`,
> `THIRD_PARTY_NOTICES.md`) — required when the archives are used in
> other repositories. The legal files live in the repo under
> `third_party/occt/`; the dev archive additionally carries the two
> license texts inside `share/doc/opencascade/` (installed by section 2).
>
> Run from the repo root, in order, one at a time:
>
> **4.1** Provenance first. `third_party/occt/THIRD_PARTY_NOTICES.md` is
> a repo-tracked file that describes **all** platform builds, and the
> archives inherit it verbatim — the zips must never carry a record
> that omits the platform whose binaries they contain. Confirm its
> macOS block is current for the build you are packaging (as committed,
> it matches: 2026-10-13, macOS 27.0.1, Apple clang 21.0.0 with Command
> Line Tools 27.0.0). If the toolchain or the build changed since — for
> example a FreeType rebuild — update the block before zipping:
>
> ```
> grep -n "macOS arm64" third_party/occt/THIRD_PARTY_NOTICES.md
> ```
>
> ✅ Prints the macOS provenance line. If it prints nothing, the file is
> missing its macOS block — stop and fix the file before zipping.
>
> If the file changes after the zips were built, re-run 4.6 and 4.8 so
> the archives carry the updated file.
>
> **4.2** Create the output folder (gitignored, like all of `dist/`):
>
> ```
> mkdir -p dist/zips
> ```
>
> **4.3** Go into the install tree. Everything after this must run from
> the repo root again, so this and step 4.5 are a pair:
>
> ```
> cd dist/occt/arm64-macos
> ```
>
> **4.4** Build the dev zip from the whole install, preserving relative
> layout (a consumer extracts it anywhere and points
> `CMAKE_PREFIX_PATH` at the extraction root). The `.` stores the
> current directory's *contents* at the zip root — no `dist/occt/...`
> prefix inside. Tens of thousands of headers make this the slow one:
>
> ```
> zip -qr ../../zips/occt-8.0.1-arm64-macos-dev.zip .
> ```
>
> **4.5** Back to the repo root — the remaining steps use
> `third_party/occt/` paths. Three levels up from
> `dist/occt/arm64-macos` (`cd ../..` would land in `dist/` — not far
> enough):
>
> ```
> cd ../../..
> ```
>
> **4.6** Add the three legal files at the dev zip root (`-j` junk-paths
> flattens them out of `third_party/occt/`):
>
> ```
> zip -qj dist/zips/occt-8.0.1-arm64-macos-dev.zip third_party/occt/THIRD_PARTY_NOTICES.md third_party/occt/LICENSE_LGPL_21.txt third_party/occt/OCCT_LGPL_EXCEPTION.txt
> ```
>
> **4.7** Build the libs-only zip — flattened `.dylib` files at the root:
>
> ```
> zip -qj dist/zips/occt-8.0.1-arm64-macos-libs-only.zip dist/occt/arm64-macos/lib/*.dylib
> ```
>
> **4.8** Add the same three legal files at the libs-only zip root:
>
> ```
> zip -qj dist/zips/occt-8.0.1-arm64-macos-libs-only.zip third_party/occt/THIRD_PARTY_NOTICES.md third_party/occt/LICENSE_LGPL_21.txt third_party/occt/OCCT_LGPL_EXCEPTION.txt
> ```
>
> **4.9** Check both zips exist and are not empty:
>
> ```
> ls -lh dist/zips
> ```
>
> ✅ Exactly two `.zip` rows, both well above zero; dev is the larger of
> the two.
>
> Notes:
>
> - `zip` updates an existing archive silently (and asks nothing when
>   re-adding); to rebuild from scratch, delete the zip first.
> - Because of the 2.1 patch, `lib/` contains only plain unversioned
>   `.dylib` files and no symlinks, so the `lib/*.dylib` glob in 4.7
>   captures exactly the toolkit libraries — no symlink targets or
>   versioned duplicates slip in.
> - The libs-only zip is flat by design; extract it next to an executable
>   (or onto a `DYLD_LIBRARY_PATH`-listed directory) — nothing inside
>   references folders. It is *not* self-sufficient for building: without
>   headers and the CMake package, consumers cannot link new code against
>   these libraries.
> - macOS dylibs are **not** symbol-stripped at link time: the `-s` /
>   `-Wl,-s` Release strip is applied on Linux only (the stock comment
>   says "`-s` is obsolete on macOS" — `adm/cmake/occt_defs_flags.cmake`
>   at V8.0.1). A libs-only zip larger than the Linux one is expected,
>   not a defect.
> - The Apple linker ad-hoc-signs every binary it produces (the default
>   on arm64, per `ld(1)`); nothing in this guide signs or notarizes
>   beyond that.
> - Names pin the version on purpose (`8.0.1`): when swapping versions
>   (see that section below), re-run this section afterwards so the
>   archives never claim the wrong version.
> - The three legal files at each zip root are verbatim copies from the
>   OCCT source tag. `THIRD_PARTY_NOTICES.md` records the provenance and
>   the scanned conclusion that no third-party code is bundled in the
>   binaries (see `AGENTS.md`, verified facts; the 8.0.1 scan found no
>   vendored components — gl2ps, bundled in older releases, is absent —
>   and the only license the binaries carry is OCCT's LGPL-2.1 +
>   exception). Re-run that scan when swapping versions or tags.

> ## 5. Optional cleanup
>
> After installation and validation, if you do not need the source/build
> tree for further work:
>
> ```
> rm -rf tmp/occt
> ```
>
> This deletes the source checkout and the build output, but leaves
> `dist/occt/arm64-macos` and the zips. Unlike the Windows guide,
> re-cloning the stock tag does **not** restore the build inputs exactly:
> the section-2.1 patch must be reapplied from this document. The stale
> `~/.cmake/packages` registry entry pointing at the deleted
> `tmp/occt/build` is harmless (consumers resolve `CMAKE_PREFIX_PATH`
> first); remove it manually with
> `rm -rf ~/.cmake/packages/OpenCASCADE/*tmp*` if you want it gone.

> ## Enabling FreeType text support (optional)
>
> The build above ships without font rendering (`USE_FREETYPE=OFF`). If
> you ever need text in viewers or fonts in BRep text builders, rebuild
> against Homebrew's FreeType:
>
> 1. Install the library:
>
>    ```
>    brew install freetype
>    ```
>
>    Unlike the Linux guide, no fontconfig is needed — verified at the
>    tag: the fontconfig code in
>    `src/Visualization/TKService/Font/Font_FontMgr.cxx` is guarded
>    `!defined(__APPLE__)`, the Apple CSF table defines no
>    `CSF_fontconfig`, and the font manager instead uses macOS's own
>    font directories (`/System/Library/Fonts`, `/Library/Fonts`).
> 2. Reconfigure in a **fresh build directory** (`rm -rf tmp/occt/build`
>    or a new `-B` path) with `-DUSE_FREETYPE=OFF` removed. With no
>    `3RDPARTY_DIR` given, OCCT falls back to CMake's built-in
>    `find_package (Freetype)` (verified at V8.0.1 — the same builtin
>    search the Linux guide relies on), which covers Homebrew's standard
>    prefix. If the automatic search misses it, point the three
>    locations explicitly (cache-variable names verified in
>    `adm/cmake/freetype.cmake`; the Windows-only `freetype.dll` demand
>    does not exist on macOS):
>
>    ```
>    cmake -S tmp/occt/OCCT -B tmp/occt/build -G Ninja \
>          -DCMAKE_BUILD_TYPE=Release \
>          -DINSTALL_DIR="$PWD/dist/occt/arm64-macos" \
>          -DINSTALL_DIR_LAYOUT=Unix \
>          -DINSTALL_DIR_WITH_VERSION=OFF \
>          -DBUILD_LIBRARY_TYPE=Shared \
>          -DBUILD_MODULE_Draw=OFF \
>          -D3RDPARTY_FREETYPE_INCLUDE_DIR_ft2build="$(brew --prefix freetype)/include" \
>          -D3RDPARTY_FREETYPE_INCLUDE_DIR_freetype2="$(brew --prefix freetype)/include/freetype2" \
>          -D3RDPARTY_FREETYPE_LIBRARY="$(brew --prefix freetype)/lib/libfreetype.dylib" \
>          2>&1 | tee tmp/occt/configure.log
>    ```
>
>    Keep `BUILD_MODULE_Draw=OFF`; Tcl/Tk stays out.
> 3. Rebuild, reinstall, and re-run section 3. Re-run section 4
>    afterwards so the zips carry the font-enabled binaries and an
>    updated provenance note. One redistribution caveat: the
>    font-enabled dylibs reference Homebrew's `libfreetype*.dylib` by
>    its absolute install path, so binaries built this way depend on
>    Homebrew being present on the consuming machine — carrying
>    FreeType into the zips is out of scope here.
>
> Do not keep a stale cache configured with `USE_FREETYPE=OFF` and merely
> flip the switch — OCCT's cache-consistency checks may leave half-set
> `3RDPARTY_FREETYPE_*` variables; a fresh build directory avoids the
> whole class of problems.

> ## Useful switches (add to the `cmake -S` line)
>
> | Switch | What it does |
> |---|---|
> | `-DBUILD_USE_PCH=ON` | Precompiled headers (needs CMake ≥ 3.16). Default OFF; a large build speedup for a bigger build tree. |
> | `-DBUILD_OPT_PROFILE=Production` | On Clang adds `-O3 -fomit-frame-pointer -flto` plus linker `-flto`. The `-ffunction-sections` / `-Wl,--gc-sections` extras the Linux profile gets are deliberately skipped on macOS (verified at V8.0.1). Default is `Default`. |
> | `-DUSE_TBB=ON` | TBB-backed parallel computations (`brew install tbb`; without it OCCT's native parallelism is used). |
> | `-DUSE_MMGR_TYPE=FLEXIBLE\|TBB\|JEMALLOC` | OCCT's internal memory manager for TKernel. Default `NATIVE`. |
> | `-DBUILD_MODULE_<Name>=OFF` | Per-module toggles: `FoundationClasses`, `ModelingData`, `ModelingAlgorithms`, `Visualization`, `ApplicationFramework`, `DataExchange`, `Draw`. All default ON. Note the dependency closure still builds TKService/TKV3d even with Visualization off. |
>
> OCCT 8.0.1 builds as C++17 (`BUILD_CPP_STANDARD` default; C++20,
> C++23, and C++26 are selectable via the same cache variable). Apple
> clang builds add `-Wall -Wextra` and `-Wshorten-64-to-32`, link `-lm`
> into shared libraries, and force `-stdlib=libc++`. Unlike the Linux
> guide, Release builds on macOS are **not** symbol-stripped at link
> time — that strip is applied on Linux only ("`-s` is obsolete on
> macOS", `adm/cmake/occt_defs_flags.cmake`), so expect larger dylibs,
> not a defect. By stock default Release builds disable OCCT exceptions
> (`BUILD_RELEASE_DISABLE_EXCEPTIONS=ON` adds `-DNo_Exception`); leave
> it unless you know you need the alternative.
>
> Platform docs (`overview-doc.zip`, `refman-doc.zip`) are attached to
> the release and are platform-independent — download and unzip them if
> you want offline documentation.

> ## Swapping versions
>
> Replace `V8.0.1` in the step-1 clone command and re-run steps 2–3 — the
> dist is rebuilt from the new tag. If retaining the old source checkout,
> replace it with the requested tag before rebuilding and **re-apply the
> section-2.1 patch** — check the new tag's
> `adm/cmake/occt_toolkit.cmake` first: the block must still be the three
> `set_target_properties` lines (its position can shift between tags).
> Also re-check the compiler floor in `adm/cmake/occt_macros.cmake`
> (Apple Clang ≥ 11.0.0 at V8.0.1) and the module layout (OCCT 8.0
> reorganized `src/` into module directories, and per-toolkit `EXTERNLIB`
> became `EXTERNLIB.cmake`). Install into a fresh prefix or delete the
> old installed files first; reinstalling does not clean them up. Repeat
> validation, then re-run section 4 — zip names pin the version. Tag
> naming differs by era: 8.0.x uses dots (`V8.0.1`) while older-era tags
> use underscores (`V8_0_0`, `V7_9_3`).

> ## Troubleshooting
>
> - **`echo $?` prints `0` even though the stage failed** — you are
>   reading the exit code of `tee`, not of CMake. In a pipeline the exit
>   status comes from the last command; run `set -o pipefail` first when
>   scripting, or check the log's tail for the actual error.
> - **`Apple Clang version 11.0.0 or newer is required for C++17
>   support`** — literal OCCT fatal from configure; the selected
>   compiler is too old. Update Xcode or the Command Line Tools
>   (`xcode-select --install`, or Software Update), delete
>   `tmp/occt/build`, and reconfigure.
> - **`Your Xcode (26.1.1) at /Applications/Xcode.app is too outdated` /
>   `Your Command Line Tools are too outdated`** (aborting
>   `brew install`) — Homebrew's own toolchain policy, unrelated to
>   OCCT's floors: on macOS 27 it requires Xcode ≥ 27.0 **and** Command
>   Line Tools ≥ 27.0.0, and the two checks are independent — an old
>   `Xcode.app` in `/Applications` trips the first check even when
>   `xcode-select` points at the CLT. There is no bypass variable (the
>   check lives in Homebrew's
>   `Library/Homebrew/extend/os/mac/diagnostic.rb`). Fix by updating
>   Xcode from the App Store to 27.0 **and** installing the matching
>   Command Line Tools (Software Update, or the download from
>   <https://developer.apple.com/download/all/>), then re-run
>   `brew install cmake ninja`. The alternative Homebrew itself
>   suggests — removing Xcode 26.1.1 and relying on an up-to-date CLT
>   only — also clears the gate, but means giving up the installed
>   Xcode. OCCT's own floor (Apple Clang ≥ 11) is not the issue; any
>   current clang is fine for the build.
> - **`ld: tapi error: malformed file` / `unknown architecture
>   arm64e.x1-macos`**, from configure's `Check for working C compiler
>   ... - broken` — a mixed-toolchain state: the linker is older than
>   the SDK it links against. `cc`/`xcrun` pick the default SDK
>   matching the running macOS from whichever installation has it: with
>   Xcode 26.4 (which carries only a `MacOSX26.4.sdk`) selected and
>   Command Line Tools 27.0.0 installed, the driver linked with Xcode's
>   old `ld` against the CLT's `MacOSX27.0.sdk`, whose text-based stub
>   files use the new `arm64e.x1` architecture tags. The recovery steps
>   live with the failure — section 2.2a, steps R1–R6: select one
>   consistent toolchain, verify, delete `tmp/occt/build`,
>   reconfigure. (Keeping Xcode selected and pinning `SDKROOT` to its
>   own `MacOSX26.4.sdk` also links, but it builds against the older
>   SDK and is not what this guide does.)
> - **`Could not find headers of used third-party products:
>   3RDPARTY_FREETYPE_INCLUDE_DIR_ft2build`** (or the libraries variant)
>   — the FreeType search ran because `USE_FREETYPE` stayed at its
>   default ON. Add `-DUSE_FREETYPE=OFF`, delete `tmp/occt/build`, and
>   reconfigure — or install Homebrew's FreeType and drop the switch
>   (see *Enabling FreeType text support*).
> - **`file INSTALL cannot find .../libTKernel.dylib`** — the build did
>   not finish. Check `tmp/occt/build.log` for the first error, rebuild
>   to success, then run `cmake --install` again.
> - **`include could not find requested file:
>   ...OpenCASCADE<Module>Targets.cmake`**, with
>   `tmp/occt/build/OpenCASCADEConfig.cmake` in the message — the
>   sample's `find_package` resolved the OCCT **build tree**: section 2's
>   build+install never finished, and the build tree had self-registered
>   in `~/.cmake/packages/`, which CMake consults once the configured
>   prefix misses. Worse, the *found-and-loaded* build-tree config gets
>   cached — `OpenCASCADE_DIR:PATH=...tmp/occt/build` in
>   `sample/build/CMakeCache.txt` — so later re-runs keep resolving the
>   build tree even after the install exists. The fix needs no registry
>   surgery: finish section 2, delete `sample/build` (clearing the pinned
>   `OpenCASCADE_DIR`), and reconfigure with the prefix set — CMake
>   consults `CMAKE_PREFIX_PATH` before the registry, so the install
>   wins and the registry entry (written by OCCT's own configure, and
>   harmless once the prefix resolves) is never read.
> - **`Could not find a package configuration file provided by
>   "OpenCASCADE"`** when configuring the sample — `CMAKE_PREFIX_PATH`
>   must point at the install prefix (`dist/occt/arm64-macos`), under
>   which `lib/cmake/opencascade/OpenCASCADEConfig.cmake` exists. A
>   prefix set only at install time (`--prefix`) does not work — the
>   prefix is baked at configure time.
> - **`dyld: Library not loaded: ... libTKernel.dylib ... Reason: image
>   not found`** when starting `hello_occt` — the OCCT `lib/` directory
>   is not reachable in that session. Re-run the
>   `export DYLD_LIBRARY_PATH=...` line from section 3.5 (a fresh
>   terminal loses it). To inspect the resolution state directly, use
>   the `otool -D` note from 2.4 on a dylib and
>   `otool -l sample/build/hello_occt | grep -A2 LC_RPATH` on the
>   executable.
> - **`****** Statistics on Transfer (Write) ******` appears in the
>   sample's output** — the messenger-silencing line was removed from
>   `sample/hello_occt.cpp`. OCCT 8.0.1 prints the banner unconditionally
>   during STEP transfer; restore the line.
> - **OCCT headers fail with C++ errors in the sample** — the consumer
>   must build as C++17 (OCCT 8.0 requires it on both sides);
>   `sample/CMakeLists.txt` sets it. Do not lower `CMAKE_CXX_STANDARD`.
> - **CMake switches into a vcpkg flow, asks for `VCPKG_ROOT`** — OCCT
>   auto-enables its vcpkg mode when `CMAKE_TOOLCHAIN_FILE` matches
>   `vcpkg.cmake`. This guide does not use vcpkg; drop that toolchain
>   file from the command/environment and reconfigure in a fresh build
>   dir.
