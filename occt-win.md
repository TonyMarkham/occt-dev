# Building Open CASCADE Technology on Windows

This repository builds **OCCT 8.0.1** (tag `V8.0.1`) from source on
**Windows x64 (MSVC)** and installs the result into the repo's gitignored
`dist/` tree. The install is validated with the headless consumer in
`sample/`, which builds a 10 × 20 × 30 mm box, writes it to a STEP file,
reads it back, and asserts `Volume: 6000 mm^3`.

The repository keeps one recipe per platform with a shared structure
(`occt-linux.md`, `occt-mac.md`); where content overlaps, keep the docs
consistent.

- Repo: <https://github.com/Open-Cascade-SAS/OCCT>
- Tag: <https://github.com/Open-Cascade-SAS/OCCT/releases/tag/V8.0.1>
- License: LGPL-2.1 with exception (`LICENSE_LGPL_21.txt` +
  `OCCT_LGPL_EXCEPTION.txt` in the source; both are installed into the
  install tree)

> ## Prerequisites
>
> - **Visual Studio 2019 or 2022** with the *Desktop development with C++*
>   workload. OCCT 8.0.1 hard-requires MSVC ≥ 19.14 (VS 2017 15.7, checked
>   with a `FATAL_ERROR` in `adm/cmake/occt_macros.cmake`); every 19.x
>   toolset is identified internally as `vc14`. The workload supplies the
>   compiler, CMake (3.16+), and Ninja.
> - **Git for Windows** for the clone in step 1.
> - **No third-party downloads.** The one external library any OCCT
>   toolkit can demand — FreeType — is switched off with the stock
>   `-DUSE_FREETYPE=OFF` (step 2); everything else the build touches is
>   either Windows SDK (OpenGL's `opengl32.lib`) or OCCT itself. See
>   *Why no third-party bundle?* below, and *Enabling FreeType text
>   support* if you ever need font rendering.
>
> **Run every command from the repository root in *Developer PowerShell
> for VS 2022*** (Start menu → the Visual Studio 2022 folder → Developer
> PowerShell for VS 2022). That shell configures the compiler
> environment; a plain PowerShell window does not, and CMake fails there
> with "Ninja not found" and no compiler. To convert the current window
> instead of opening a new one (adjust the edition folder if not
> `Community`):
>
> ```
> & "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\Launch-VsDevShell.ps1" -Arch amd64
> ```
>
> **Optional: pin this shell as a Windows Terminal profile.** Windows
> Terminal auto-creates a **Developer PowerShell for VS 2022** profile
> when VS 2022 is installed — check the tab-bar dropdown (`˅`) first. If
> it is there, duplicate it (Settings → that profile → *Duplicate*) and
> set its *Starting directory* to the repository root; the dedicated
> profile below is then optional. To create it explicitly with a pinned
> x64 toolset, use the settings UI (no JSON editing):
>
> 1. Windows Terminal → the dropdown (`˅`) beside the tabs → **Settings**
>    (or `Ctrl+,`).
> 2. Left sidebar, under *Profiles*: **Add a new profile** → **New empty
>    profile**.
> 3. *Name*: `Dev Pwsh (VS 2022, x64)`
> 4. *Command line*:
>    `pwsh.exe -NoExit -ExecutionPolicy Bypass -File "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\Launch-VsDevShell.ps1" -Arch amd64`
>    (paste exactly as written — the UI field takes the unescaped form).
> 5. *Starting directory*: `D:\git\occt-dev` — the repo root; the commands
>    in this guide expand `$PWD` and assume it.
> 6. **Save** (bottom right). The profile appears in the dropdown.
>
> To edit the JSON file directly instead of using the UI:
>
> 1. Open the file: in the Settings page, click **Open JSON file** — the
>    small link in the bottom-left corner of the Settings window, next to
>    *Save* — or run from any shell:
>
>    ```
>    Invoke-Item "$env:LOCALAPPDATA\Packages\Microsoft.WindowsTerminal_8wekyb3d8bbwe\LocalState\settings.json"
>    ```
>
>    (that path is the Store/winget install; the *Open JSON file* link
>    works for every install channel).
> 2. Find `"profiles"` and, inside it, the `"list"` array of profile
>    objects. Paste the new entry after the last profile object, adding a
>    comma to the object before it:
>
>    ```json
>        { "...last existing profile..." },
>        {
>          "guid": "{CE111691-96CE-46F8-B2DB-048749CDB95C}",
>          "name": "Dev Pwsh (VS 2022, x64)",
>          "commandline": "pwsh.exe -NoExit -ExecutionPolicy Bypass -File \"C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\Common7\\Tools\\Launch-VsDevShell.ps1\" -Arch amd64",
>          "startingDirectory": "D:\\git\\occt-dev"
>        }
>      ]
>    ```
>
>    The `guid` is technically optional for hand-added profiles (Terminal
>    mints one from the profile name when absent), but an explicit one
>    keeps the profile addressable — `wt -p {guid}`, `defaultProfile` —
>    and matches the shape of every profile Terminal itself writes.
>
> 3. Save. Terminal hot-reloads `settings.json` on save; the profile
>    appears in the dropdown immediately.
>
> JSON strings need doubled backslashes in paths (`D:\\git\\occt-dev`),
> as shown. The launcher works under both `pwsh.exe` (PowerShell 7) and
> `powershell.exe` (Windows PowerShell 5.1) — use whichever host you
> prefer — and `-Arch amd64` pins the x64 toolset, whereas VS's
> auto-generated profile leaves the arch at the host default.
> `-ExecutionPolicy Bypass` keeps the profile working when script
> execution is policy-restricted. Whichever profile you use, still run
> the verification block above after the shell opens.
>
> After opening it, verify:
>
> ```powershell
> Get-Command cl, ninja, cmake | Format-Table Name, Source
> ```
>
> ```powershell
> $env:VSCMD_ARG_TGT_ARCH
> ```
>
> All three commands must resolve, and the arch variable must print
> `x64`. Commands in this guide use PowerShell's backtick (`` ` ``) for
> multi-line continuation: paste each block in one shot, and keep every
> backtick as the very last character of its line — a trailing space
> after a backtick breaks the command. Never paste cmd-style `^`
> continuations; PowerShell treats `^` as a stray argument.
>
> *Why no third-party bundle?* The Linux guide installs FreeType, X11, and
> OpenGL development packages — via `apt`, which makes it look free.
> Windows has no equivalent system provider, and you cannot simply disable
> the Visualization module instead: OCCT builds every toolkit reachable
> through the dependency graph — the top-level CMakeLists expands
> transitive `TK*` dependencies without filtering (verified at V8.0.1) and
> runs `add_subdirectory` over the whole closure, so TKService and TKV3d
> are built even with `BUILD_MODULE_Visualization=OFF`, because TKXCAF and
> several other built toolkits depend on TKV3d, whose EXTERNLIB demands
> FreeType. The stock kill switch is `-DUSE_FREETYPE=OFF`: the font code
> is compile-guarded (`HAVE_FREETYPE`) and OCCT then builds TKService
> without font/text support. The build below is a pure MSVC + Windows SDK
> build: FreeType off, Tcl/Tk never searched (Draw module off), OpenGL
> from the SDK's `opengl32.lib`.

> ## 1. Get the OCCT source at the release tag
>
> Clone the tag into the repo's gitignored source/build area:
>
> ```
> git clone --depth 1 --branch V8.0.1 https://github.com/Open-Cascade-SAS/OCCT.git tmp\occt\OCCT
> ```
>
> If `tmp\occt\OCCT` already exists, use the existing `V8.0.1` checkout
> rather than cloning over it. Unlike the Linux/macOS recipes, this guide
> makes **no source modifications**, so a stock checkout is all that is
> needed.

> ## 2. Configure, build, install to the repo's dist
>
> Run from the repository root — a dev shell gives no promise about the
> directory it opens in (the `Launch-VsDevShell.ps1` one-liner inherits
> whatever directory the window was in; Start-menu profiles open in the
> home directory unless the profile sets *Starting directory*). Pin it
> and confirm step 1's clone is where the commands below expect it:
>
> ```powershell
> cd D:\git\occt-dev
> ```
>
> ```powershell
> Test-Path tmp\occt\OCCT, dist\occt\x86_64-windows
> ```
>
> The `Test-Path` must print `True` (the clone exists; the dist line is
> allowed to print `False` until section 2 finishes). OCCT installs
> directly into `dist\occt\x86_64-windows`; there is no external install
> and no copy step afterwards.
>
> **No CMake source patch is needed on Windows.** The Linux/macOS guides
> remove `VERSION` and `SOVERSION` from `adm/cmake/occt_toolkit.cmake` to
> get unversioned `libTK*.so` files. That block (lines 207–209 at V8.0.1)
> only affects ELF/Mach-O file naming; Windows DLLs are always plain
> `TKernel.dll` with no version suffixes or symlinks (OCCT's symlink rule
> is explicitly guarded `if (NOT WIN32)`). Leave the file untouched.
>
> Each stage command below pipes its output through `Tee-Object` into a
> log file in `tmp\occt\` — `configure.log`, `build.log`, `install.log` —
> so progress streams to the console while the full output is captured
> for sharing a failed stage. In Windows PowerShell 5.1, native stderr
> lines land in the log wrapped in error-record noise; PowerShell 7+
> writes them cleanly — the text itself is unaffected. Check `$LASTEXITCODE`
> after each stage; it must be `0`.
>
> **Configure** — generates the build plan; nothing compiles yet:
>
> ```
> cmake -S tmp\occt\OCCT -B tmp\occt\build -G Ninja `
>       -DCMAKE_BUILD_TYPE=Release `
>       "-DINSTALL_DIR=$PWD\dist\occt\x86_64-windows" `
>       -DINSTALL_DIR_LAYOUT=Unix `
>       -DINSTALL_DIR_WITH_VERSION=OFF `
>       -DBUILD_LIBRARY_TYPE=Shared `
>       -DBUILD_MODULE_Draw=OFF `
>       -DUSE_FREETYPE=OFF `
>       2>&1 | Tee-Object tmp\occt\configure.log
> ```
>
> The switches:
>
> - `-S tmp\occt\OCCT` — source directory: the clone from step 1.
> - `-B tmp\occt\build` — build directory: all intermediate output goes
>   here; deleting it deletes the build and nothing else.
> - `-G Ninja` — the Ninja generator. It requires the compiler environment
>   of the developer shell (see Prerequisites). Ninja ships with the VS
>   C++ workload.
> - `-DCMAKE_BUILD_TYPE=Release` — explicitly select an optimized build.
>   With a single-config generator this also keeps OCCT's release output in
>   unsuffixed directories (`bin`, not `bin`/`bini`/`bind`).
> - `-DINSTALL_DIR=$PWD\dist\occt\x86_64-windows` — OCCT's own name for the
>   install prefix (same as `CMAKE_INSTALL_PREFIX`): where `cmake --install`
>   puts the final files. `$PWD` is the repo root; the whole switch is
>   quoted so a path containing spaces stays one argument. The value is
>   **baked in at configure time** — the generated `env.bat`/`custom.bat`
>   and CMake package files all derive from it, so
>   `cmake --install --prefix <elsewhere>` afterwards does not fully
>   relocate the install. Decide the prefix now.
> - `-DINSTALL_DIR_LAYOUT=Unix` — flat `bin`/`lib`/`include` layout, the
>   same relative layout the Linux/macOS guides produce. OCCT's Windows
>   default layout would nest binaries under `win64\vc14\` and headers
>   under `inc\`; the Unix layout keeps all three platforms consistent.
> - `-DINSTALL_DIR_WITH_VERSION=OFF` — no version suffix in the include
>   directory: `include\opencascade`, not `include\opencascade-8.0.1`.
> - `-DBUILD_LIBRARY_TYPE=Shared` — build DLLs (plus MSVC import libraries)
>   to distribute. `Shared` is the default; it is spelled out for clarity.
> - `-DBUILD_MODULE_Draw=OFF` — skip the DRAW test harness: DRAWEXE is not
>   built and Tcl/Tk is never searched. OCCT validation in this guide goes
>   through section 3 instead.
> - `-DUSE_FREETYPE=OFF` — OCCT's stock switch for building without
>   FreeType. `USE_FREETYPE` defaults to ON; turning it off skips the
>   FreeType search entirely, defines `CSF_FREETYPE` empty (toolkit link
>   lines skip it), and drops `-DHAVE_FREETYPE`, so the font code in
>   TKService compiles without `ft2build.h` (text rendering becomes a
>   no-op). A successful configure ends with `Configuring done` /
>   `Generating done` and no third-party warnings. TKService/TKV3d/TKOpenGl
>   are still built (the Visualization module stays enabled — see *Why no
>   third-party bundle?*), so `bin\` contains the full DLL set, minus font
>   support.
>
> Two stock side effects of a successful configure, both harmless here:
> the build tree self-registers in the user's CMake package registry
> (`HKCU:\Software\Kitware\CMake\Packages\OpenCASCADE` — OCCT calls
> `export(PACKAGE)` unconditionally; `-DCMAKE_EXPORT_PACKAGE_REGISTRY=OFF`
> does *not* prevent it and only earns an unused-variable warning), and
> while the registry points at `tmp\occt\build`, consumers configured with
> `CMAKE_PREFIX_PATH` resolve the install before the registry is ever
> consulted. The entry dies with the section-4 cleanup.
>
> A successful configure ends with `Configuring done` / `Generating done`.
> If a previous configure attempt failed partway (for example, run from a
> shell without the VS environment), delete the half-configured build
> directory first — `Remove-Item -Recurse -Force tmp\occt\build` — so its
> stale cache cannot interfere. If you previously customized
> `INSTALL_DIR_LIB`, `INSTALL_DIR_INCLUDE`, or other install
> subdirectories in this build cache, make them match the switches above
> or use a fresh build directory.
>
> **Build** — the actual compile; the 20–60 minute part (estimate):
>
> ```
> cmake --build tmp\occt\build --parallel 2>&1 | Tee-Object tmp\occt\build.log
> ```
>
> `--parallel` uses Ninja's default parallelism (all cores; OCCT also adds
> `/MP` for MSVC). To bound memory usage, give it a job count, for example
> `--parallel 8`. The build tree holds several GB of object files and PDBs;
> keep roughly 20 GB free (estimate). When it finishes,
> `Select-String -Pattern "error", "fatal" tmp\occt\build.log` should print
> nothing and `$LASTEXITCODE` must be `0`. If the build failed, resolve the
> first error in the log; rerunning install alone cannot produce missing
> files.
>
> **Install** — copies headers, libraries, resources into the dist:
>
> Run this only after the build has completed successfully. If reusing a
> dist from an older installation, first delete its obsolete `TK*.dll` and
> `TK*.lib` files; CMake does not remove stale installed files.
>
> ```
> cmake --install tmp\occt\build 2>&1 | Tee-Object tmp\occt\install.log
> ```
>
> Installed layout (`dist\occt\x86_64-windows\`):
>
> ```
> <prefix>\
> ├── bin\                          TKernel.dll ... (several dozen DLLs),
> │                                 env.bat, custom.bat, custom_vc14_64.bat
> ├── lib\                          TKernel.lib ... (MSVC import libraries)
> │   └── cmake\opencascade\        OpenCASCADEConfig.cmake, targets files
> ├── include\opencascade\          public headers, Standard_Version.hxx
> └── share\
>     ├── opencascade\resources\    UnitsAPI, XSTEPResource, StdResource, ...
>     └── doc\opencascade\          LICENSE_LGPL_21.txt, OCCT_LGPL_EXCEPTION.txt
> ```
>
> On Windows there are no library symlinks or version suffixes to check
> for — `TKernel.dll` (in `bin\`) and `TKernel.lib` (in `lib\`) are plain
> files. Verify the install (each line must print `True`):
>
> ```powershell
> Test-Path dist\occt\x86_64-windows\bin\TKernel.dll
> ```
>
> ```powershell
> Test-Path dist\occt\x86_64-windows\lib\TKernel.lib
> ```
>
> ```powershell
> Test-Path dist\occt\x86_64-windows\lib\cmake\opencascade\OpenCASCADEConfig.cmake
> ```
>
> The CMake package resolves its install prefix from the config file's
> own location, so consumers can point `CMAKE_PREFIX_PATH` at the prefix
> above (or at any directory containing it) and
> `find_package(OpenCASCADE)` works.

> ## 3. Validate with sample/ (headless STEP round-trip)
>
> Section 2 left you a working OCCT install. This section proves it works
> by running a small program from `sample/` that builds a 10 × 20 × 30 mm
> box, writes it to a STEP file, reads that file back, and checks the
> volume survived the round trip. If the printed volume is 6000 mm^3,
> headers, DLLs, and STEP support all work — end to end.
>
> The program is two files:
>
> - `sample\hello_occt.cpp` — the program: box → STEP → read back →
>   assert volume (silences OCCT's noisy transfer banner first).
> - `sample\CMakeLists.txt` — its build file: declares C++17, finds the
>   installed OCCT, links the seven toolkits whose APIs the program calls
>   (`TKernel TKMath TKBRep TKPrim TKTopAlgo TKXSBase TKDESTEP` —
>   everything else follows through toolkit dependencies).
>
> Run these commands from the repo root, in order, one at a time:
>
> **3.1** Confirm the install from section 2 is present. Must print
> `True` — if it prints `False`, finish section 2 before continuing:
>
> ```powershell
> Test-Path dist\occt\x86_64-windows\lib\cmake\opencascade\OpenCASCADEConfig.cmake
> ```
>
> **3.2** Remove any previous sample build (stale caches point at the
> wrong OpenCASCADE):
>
> ```powershell
> Remove-Item -Recurse -Force sample\build
> ```
>
> **3.3** Point the sample at the install and generate its build plan.
> Success: a line starting with `-- Found OpenCASCADE:` naming
> `dist\occt\x86_64-windows\...`:
>
> ```powershell
> cmake -S sample -B sample\build -G Ninja `
>       -DCMAKE_BUILD_TYPE=Release `
>       "-DCMAKE_PREFIX_PATH=$PWD\dist\occt\x86_64-windows"
> ```
>
> If you instead see `include could not find requested file:
> ...OpenCASCADE<Module>Targets.cmake`, see Troubleshooting.
>
> **3.4** Compile the sample (seconds, not minutes):
>
> ```powershell
> cmake --build sample\build --parallel
> ```
>
> **3.5** Make the OCCT DLLs findable at runtime. Without this, Windows
> refuses to start the exe (`0xc0000135`). Lasts only for this session:
>
> ```powershell
> $env:PATH = "$PWD\dist\occt\x86_64-windows\bin;$env:PATH"
> ```
>
> **3.6** Go where the built exe is:
>
> ```powershell
> cd sample\build
> ```
>
> **3.7** Run the validation:
>
> ```powershell
> .\hello_occt.exe
> ```
>
> **3.8** Print the exit code. Must be `0`:
>
> ```powershell
> $LASTEXITCODE
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
> ```powershell
> cd ..\..
> ```
>
> The volume assertion tolerates 0.01 mm^3; the analytic box volume is
> 6000 mm^3. The run proves the full loop: CMake finds the installed
> package through `CMAKE_PREFIX_PATH`, a C++17 consumer compiles against
> the OCCT 8.0 headers, the DLLs load from `bin\`, and a real STEP
> round-trip preserves geometry. It does not exercise Visualization,
> viewers, or deployment to another machine — but nothing about the STEP
> path needs extra environment setup (`CSF_*` variables are not required
> for this round-trip).
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
> files under `dist\zips\`:
>
> - **dev** — everything: DLLs, import libraries, headers, CMake package
>   config, resources, `env.bat`/`custom.bat`. For consumers who build
>   against OCCT.
> - **dlls-only** — just the `bin\*.dll`, flattened at the zip root. For
>   consumers who only need to *run* something already built against
>   these DLLs.
>
> Both archives carry the legal texts at the zip root
> (`LICENSE_LGPL_21.txt`, `OCCT_LGPL_EXCEPTION.txt`,
> `THIRD_PARTY_NOTICES.md`) — required when the archives are used in
> other repositories. The legal files live in the repo under
> `third_party\occt\`; the dev archive additionally carries the two
> license texts inside `share\doc\opencascade\` (installed by section 2).
>
> Run from the repo root, in order, one at a time:
>
> **4.1** Create the output folder (gitignored, like all of `dist\`):
>
> ```powershell
> New-Item -ItemType Directory -Force dist\zips
> ```
>
> **4.2** Build the dev zip from the whole install. Tens of thousands of
> headers make this the slow one — a few minutes, no progress output
> until it finishes:
>
> ```powershell
> Compress-Archive -Path dist\occt\x86_64-windows\*, third_party\occt\THIRD_PARTY_NOTICES.md, third_party\occt\LICENSE_LGPL_21.txt, third_party\occt\OCCT_LGPL_EXCEPTION.txt -DestinationPath dist\zips\occt-8.0.1-x86_64-windows-dev.zip
> ```
>
> **4.3** Build the DLLs-only zip. Flattens: the DLLs and the three
> legal files sit at the zip root, not inside a `bin\` folder:
>
> ```powershell
> Compress-Archive -Path dist\occt\x86_64-windows\bin\*.dll, third_party\occt\THIRD_PARTY_NOTICES.md, third_party\occt\LICENSE_LGPL_21.txt, third_party\occt\OCCT_LGPL_EXCEPTION.txt -DestinationPath dist\zips\occt-8.0.1-x86_64-windows-dlls-only.zip
> ```
>
> **4.4** Check both zips exist and are not empty:
>
> ```powershell
> Get-ChildItem dist\zips | Select-Object Name, @{n='MB';e={[math]::Round($_.Length/1MB)}}
> ```
>
> ✅ Exactly two `.zip` rows, both with MB values well above zero
> (dev is the larger of the two).
>
> Notes:
>
> - `Compress-Archive` overwrites an existing zip of the same name
>   silently — no "file exists" prompt.
> - The dev zip preserves the install layout (`bin\`, `lib\`,
>   `include\opencascade\`, ...) because `-Path ...\*` keeps relative
>   structure; a consumer extracts it anywhere and points
>   `CMAKE_PREFIX_PATH` at the extraction root.
> - The DLLs-only zip is flat by design; extract it next to an exe (or
>   onto a `PATH`-listed directory) — nothing inside references folders.
> - The DLL-only zip is *not* self-sufficient for building: without the
>   import libraries (`lib\*.lib`) and headers, MSVC cannot link new
>   code against these DLLs.
> - Names pin the version on purpose (`8.0.1`): when swapping versions
>   (see that section below), re-run this section afterwards so the
>   archives never claim the wrong version.
> - The three legal files at each zip root are verbatim copies from the
>   OCCT source tag. `THIRD_PARTY_NOTICES.md` records the provenance and
>   the scanned conclusion that no third-party code is bundled in the
>   binaries (see `AGENTS.md`, verified facts). Re-check that scan when
>   swapping versions.

> ## 5. Optional cleanup
>
> After installation and validation, if you do not need the source/build
> tree for further work:
>
> ```
> Remove-Item -Recurse -Force tmp\occt
> ```
>
> This deletes the source checkout and the build output, but leaves
> `dist\occt\x86_64-windows`. This guide made no local source
> modifications, so re-cloning the stock tag restores the build inputs
> exactly.

> ## Enabling FreeType text support (optional)
>
> The build above ships without font rendering (`USE_FREETYPE=OFF`). If
> you ever need text in viewers or fonts in BRep text builders, rebuild
> with FreeType from OCCT's official bundle:
>
> 1. Download **`3rdparty-vc14-64.zip`** from the same V8.0.1 release page
>    (verified against the release API: 188,827,053 bytes,
>    sha256 `12d04f688c583b8e896758f65cd7012458fee921cedb792ec6caaf469b372598`).
> 2. Extract it and locate the directory that directly contains the product
>    subdirectories (a `freetype-...` directory among them — the asset may
>    nest a zip; extract until you see product directories). This is the
>    OCCT official bundle for the vc14 (any 19.x MSVC) x64 toolset.
> 3. Reconfigure with `-DUSE_FREETYPE=OFF` removed and
>    `-D3RDPARTY_DIR=<that directory>` added, then rebuild and reinstall
>    into a fresh prefix. Keep `BUILD_MODULE_Draw=OFF` — Tcl is not needed
>    for the modules this guide builds, so the bundle's Tcl/Tk
>    directories are simply ignored.
>
> How the discovery works (from `adm/cmake/freetype.cmake` at V8.0.1): OCCT
> looks under `3RDPARTY_DIR` for a subdirectory whose name matches
> `freetype*` (case-insensitive), then requires `ft2build.h` and
> `ftheader.h` (include directories), `freetype.lib` (in a `lib\`
> subdirectory), and `freetype.dll` (in a `bin\` subdirectory). Missing
> headers or libraries abort configure with `Could not find headers/libraries
> of used third-party products`. `-DINSTALL_FREETYPE=ON` copies
> `freetype.dll` into the install's `bin\` so the DLLs load without extra
> PATH entries. Other optional libraries (TBB, FreeImage, ...) follow the
> same pattern from the same bundle. The 8.0.1 release notes call out
> improved Windows/MSYS2 third-party discovery in this series.

> ## Useful switches (add to the `cmake -S` line)
>
> | Switch | What it does |
> |---|---|
> | `-DBUILD_USE_PCH=ON` | Precompiled headers (needs CMake ≥ 3.16). Default OFF; a large build speedup for a bigger build tree. |
> | `-DBUILD_OPT_PROFILE=Production` | Adds `/GF /Gy /Ot /Oy /GL /Oi` and `/LTCG` to MSVC Release flags. The official release binaries use aggressive optimization; default is `Default`. |
> | `-DUSE_TBB=ON` | TBB-backed parallel computations (needs TBB from a third-party bundle; without it OCCT's native parallelism is used). |
> | `-DUSE_MMGR_TYPE=FLEXIBLE\|TBB\|JEMALLOC` | OCCT's internal memory manager for TKernel. Default `NATIVE`. |
> | `-DBUILD_MODULE_<Name>=OFF` | Per-module toggles: `FoundationClasses`, `ModelingData`, `ModelingAlgorithms`, `Visualization`, `ApplicationFramework`, `DataExchange`, `Draw`. All default ON. |
>
> OCCT 8.0.1 builds as C++17 (`BUILD_CPP_STANDARD` default; C++20 is
> selectable) and forces `/fp:precise` for MSVC — do not replace it with
> `/fp:fast`. By stock default Release builds disable OCCT exceptions
> (`BUILD_RELEASE_DISABLE_EXCEPTIONS=ON` adds `-DNo_Exception`); leave it
> unless you know you need the alternative.
>
> Platform docs (`overview-doc.zip`, `refman-doc.zip`) are attached to the
> release and are platform-independent — download and unzip them if you
> want offline documentation.

> ## Swapping versions
>
> Replace `V8.0.1` in the step-1 clone command and re-run steps 2–3 — the
> dist is rebuilt from the new tag. If retaining the old source checkout,
> replace it with the requested tag before rebuilding. No source patch is
> involved on Windows, but check the new tag's CMake code for the MSVC
> recognition (`adm/cmake/occt_macros.cmake`) — a toolset OCCT does not
> recognize fails with `Unrecognized MSVC_VERSION` — and for module
> layout (OCCT 8.0 reorganized `src/` into module directories). Install
> into a fresh prefix or delete the old installed files first; reinstalling
> does not clean them up. Repeat validation. Tag naming differs by era:
> 8.0.x uses dots (`V8.0.1`) while older-era tags use underscores
> (`V8_0_0`, `V7_9_3`).

> ## Troubleshooting
>
> - **`CMake was unable to find a build program corresponding to "Ninja"`,
>   `CMAKE_C_COMPILER not set`, `cl.exe` not found** — the shell is a plain
>   PowerShell window without the VS environment. Open *Developer
>   PowerShell for VS 2022* from the Start menu, or run the
>   `Launch-VsDevShell.ps1` one-liner from Prerequisites in the current
>   window; verify with the `Get-Command cl, ninja, cmake` check, delete
>   the half-configured `tmp\occt\build`, and reconfigure there.
> - **`^` passed to CMake (`Ignoring extra path from command line`) or
>   `-D...` lines run as separate commands** — cmd-style line continuation
>   pasted into PowerShell. Paste the blocks as written — PowerShell
>   continues lines with a trailing backtick, not `^` — and check that no
>   trailing space follows a backtick.
> - **`Microsoft Visual C++ 19.14 (VS 2017 15.7) or newer is required for
>   C++17 support`** — literal OCCT fatal from configure; the selected
>   toolset is too old. Install/upgrade the C++ workload.
> - **`Unrecognized MSVC_VERSION`** — OCCT 8.0.1 recognizes MSVC 19.x
>   (`vc14`) only; a future major toolset is rejected. Check the toolset's
>   version and the OCCT tag's `occt_macros.cmake` before swapping tags.
> - **Ninja exists but you want the VS generator anyway** — use
>   `-G "Visual Studio 17 2022" -A x64` and build with
>   `cmake --build tmp\occt\build --config Release --parallel`; the install
>   layout is unchanged (Release has no bin-letter suffix).
> - **`Could not find headers of used third-party products:
>   3RDPARTY_FREETYPE_INCLUDE_DIR_ft2build`** (or the libraries variant) —
>   the FreeType search ran because `USE_FREETYPE` stayed at its default
>   ON. Add `-DUSE_FREETYPE=OFF`, delete `tmp\occt\build`, and reconfigure
>   — or supply the official bundle via `-D3RDPARTY_DIR=...` and drop the
>   switch (see *Enabling FreeType text support*).
> - **CMake switches into a vcpkg flow, asks for `VCPKG_ROOT`** — OCCT
>   auto-enables its vcpkg mode when `CMAKE_TOOLCHAIN_FILE` matches
>   `vcpkg.cmake`. This guide does not use vcpkg; drop that toolchain file
>   from the command/environment and reconfigure in a fresh build dir.
> - **`file INSTALL cannot find .../TKernel.dll`** — the build did not
>   finish. Check `tmp\occt\build.log` for the first error, rebuild to
>   success, then run `cmake --install` again.
> - **`include could not find requested file:
>   ...OpenCASCADE<Module>Targets.cmake`**, with
>   `tmp\occt\build\OpenCASCADEConfig.cmake` in the message — the sample's
>   `find_package` resolved the OCCT **build tree**: section 2's
>   build+install never finished, and the build tree had self-registered
>   in the user package registry, which CMake consults once the
>   configured prefix misses. Worse, the *found-and-loaded* build-tree
>   config gets cached — `OpenCASCADE_DIR:PATH=...tmp\occt\build` in
>   `sample\build\CMakeCache.txt` — so later re-runs keep resolving the
>   build tree even after the install exists. The fix needs no registry
>   change: finish section 2, delete `sample\build` (clearing the pinned
>   `OpenCASCADE_DIR`), and reconfigure with the prefix set — CMake
>   consults `CMAKE_PREFIX_PATH` before the user registry, so the
>   install wins and the registry entry (written by OCCT's own
>   configure, and harmless once the prefix resolves) is never read.
>   Builds configured with `-DCMAKE_EXPORT_PACKAGE_REGISTRY=OFF` no
>   longer self-register.
> - **`Could not find a package configuration file provided by
>   "OpenCASCADE"`** when configuring the sample — `CMAKE_PREFIX_PATH`
>   must point at the install prefix (`dist\occt\x86_64-windows`), under
>   which `lib\cmake\opencascade\OpenCASCADEConfig.cmake` exists. A prefix
>   set only at install time (`--prefix`) does not work — the prefix is
>   baked at configure time.
> - **`0xc0000135` (STATUS_DLL_NOT_FOUND)** when starting
>   `hello_occt.exe` — the OCCT `bin\` directory is not on PATH in that
>   session. Re-run the `$env:PATH = ...` line from section 3.
> - **`****** Statistics on Transfer (Write) ******` appears in the
>   sample's output** — the messenger-silencing line was removed from
>   `sample\hello_occt.cpp`. OCCT 8.0.1 prints the banner unconditionally
>   during STEP transfer; restore the line.
> - **OCCT headers fail with C++ errors in the sample** — the consumer must
>   build as C++17 (OCCT 8.0 requires it on both sides);
>   `sample\CMakeLists.txt` sets it. Do not lower `CMAKE_CXX_STANDARD`.
