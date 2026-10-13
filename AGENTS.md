# Project purpose

Platform-specific documentation for building/installing **OCCT 8.0.1**
native binaries into this repo's gitignored `dist/` tree. All three
platforms (Linux, macOS, Windows) build from the release tag `V8.0.1`;
Windows requires no local source modification, Linux and macOS patch
`adm/cmake/occt_toolkit.cmake` for unversioned library names. `sample/`
is the headless validation consumer every doc uses.

## Working with the user

- The user runs the builds and installs. The docs are instructions for the
  user to execute, not for the agent. Do not run OCCT configure/build/
  install, install system dependencies, or launch validation runs unless
  explicitly asked. Read-only checks (file existence, header inspection,
  checksum lookups) are always fine.
- **Communicate before acting.** Before running any command: one line
  saying what it does, why it is needed, and whether it is read-only or
  writes somewhere. No unexplained commands, ever.
- **Touch only what was asked.** When the user names a file, edit only
  that file. Any additional file change — even a correctness fix
  discovered mid-task — is proposed in words first and waits for an
  explicit yes.
- **Never write outside an explicit request** — no file, registry,
  cache, or temp-space changes without the user's OK. Experiments that
  touch machine state need consent first. Read-only inspections are
  allowed, but announced as read-only.
- **Answer first, act later.** When asked a question, answer it in words
  before running any tool. When corrected or accused, acknowledge in one
  line and fix the behavior — no defensiveness, no self-justifying
  essays, no research side-quests to relitigate a point.
- **Do not grovel either.** When the user is hostile, stay matter-of-fact:
  keep answering, keep the work moving, and do not compensate with
  apologies, exclamation-strewn compliance, or hedged mush. The rule
  above cuts self-defense; this one cuts capitulation. Steady and useful
  beats both.
- **Keep the shape of answers readable.** Runbooks are numbered steps:
  command + expected result, with nothing woven between them. Any
  analysis or doc commentary comes after the steps, clearly separated.
- **One command per code block.** In docs and in chat, never put two or
  more commands inside one fenced block; every command gets its own
  block so each can be copied and run individually.
- **One open question at a time**, restated at the end of the message
  when a decision is pending. Silence from the user means wait, not
  proceed.
- This repo is deliberately **docs-only**: no build scripts, no CI, no
  packaging automation. Do not add unrequested automation; if asked, prefer
  small inline commands over scripts.
- A stated decision is settled: do not re-open settled choices, silently
  deviate from a choice, or wait for confirmation on it.
- When writing to a doc, verify facts against sources before claiming
  them — the release page (asset names, digests, sizes), the OCCT source at
  the pinned tag (CMake snippets, module layout), installed headers, and a
  validated install under `dist/`. Clearly separate "verified" from
  "assumed" in what you tell the user.
- Keep the three platform docs consistent where they share content. The
  `occt_toolkit.cmake` unversioned-library patch appears in `occt-linux.md`
  and `occt-mac.md` only; `occt-win.md` documents why Windows needs no
  patch. All three use `INSTALL_DIR_LAYOUT=Unix` so installs share one
  relative layout, and all three disable the Draw module.

## Repository map

- `occt-win.md` — Windows build recipe (present). Build from tag into
  `dist/occt/x86_64-windows` with the Visualization module disabled, so
  the build needs no third-party downloads.
- `occt-linux.md` / `occt-mac.md` — the Linux/macOS recipes; to be added
  (mirror the greyhound-snake `occt-linux.md` structure; each
  self-contained: prerequisites → install → validation).
- `sample/` — headless CMake consumer (`hello_occt.cpp`): builds a
  10 × 20 × 30 mm box, writes a STEP file, reads it back, asserts
  `Volume: 6000 mm^3`. This is the validation harness the docs reference;
  its expected output is part of each doc's spec.
- `third_party/occt/` — OCCT LGPL-2.1-with-exception license texts
  (verbatim from the source tag) + `THIRD_PARTY_NOTICES.md`; packed at
  the zip root by both archives in `occt-win.md` section 4.
- `dist/` and `tmp/` — gitignored local state; never commit them.

## Verified facts pinned by the docs

Re-check these when touching a new tag or doc section:

- OCCT 8.0.1 = tag `V8.0.1` (dots). Older-era tags use underscores
  (`V8_0_0`, `V7_9_3`).
- OCCT 8.0 reorganized `src/` into module directories
  (`src/ModelingAlgorithms/TKPrim/...`); toolkit package lists live in
  `src/<Module>/TOOLKITS.cmake` / `PACKAGES.cmake`.
- Unversioned libraries (Linux/macOS) require a local modification to
  `adm/cmake/occt_toolkit.cmake` (remove both `VERSION` and `SOVERSION`);
  no stock flag achieves this (`BUILD_SOVERSION_NUMBERS=0` is overridden).
  On Windows the same block is irrelevant: DLL naming ignores
  `VERSION`/`SOVERSION` and OCCT's symlink rule is `if (NOT WIN32)`, so
  `occt-win.md` needs no source patch.
- OCCT 8.0 `Make*` classes construct lazily: the parameterized constructor
  only prepares the primitive — call `Build()` explicitly before `IsDone()`
  / `Shape()` (verified with `BRepPrimAPI_MakeBox`).
- Windows MSVC floor: 19.14 (VS 2017 15.7), enforced by a `FATAL_ERROR` in
  `adm/cmake/occt_macros.cmake`; all 19.x toolsets are identified as
  `vc14`, and `MSVC_VERSION >= 2000` fails with `Unrecognized
  MSVC_VERSION`.
- What gets built is decided by the toolkit dependency closure, not the
  module toggles: the top-level CMakeLists expands transitive `TK*`
  dependencies without filtering and runs `add_subdirectory` over the
  whole closure, so TKService/TKV3d are built even with
  `BUILD_MODULE_Visualization=OFF` (TKXCAF and other built toolkits
  depend on TKV3d, whose EXTERNLIB demands `CSF_FREETYPE`). Verified
  against a real configure log 2026-10-13 (ft2build fatal). The stock
  no-FreeType path is `-DUSE_FREETYPE=OFF` (`CSF_FREETYPE` set empty,
  font code compile-guarded by `HAVE_FREETYPE`); the official MSVC x64
  bundle for V8.0.1 is `3rdparty-vc14-64.zip` (188,827,053 bytes,
  sha256 `12d04f688c583b8e896758f65cd7012458fee921cedb792ec6caaf469b372598`).
  Tcl/Tk is only demanded by Draw (`BUILD_MODULE_Draw=OFF` keeps it out);
  OpenGL needs only the SDK's `opengl32.lib`.
- OCCT's top-level CMakeLists calls `export(PACKAGE OpenCASCADE)`
  explicitly; every successful configure self-registers the build tree in
  the user package registry
  (`HKCU:\Software\Kitware\CMake\Packages\OpenCASCADE`). This happens even
  with `-DCMAKE_EXPORT_PACKAGE_REGISTRY=OFF` — that flag only gates
  CMake's *implicit* CMP0090 registration, not OCCT's explicit
  `export(PACKAGE)` call (verified by micro-experiment 2026-10-13); the
  flag only earns an unused-variable warning in the configure log. The
  entry is OCCT's, points at `tmp\occt\build`, and is harmless while
  consumers resolve via `CMAKE_PREFIX_PATH` (consulted before the
  registry). Observed 2026-10-13 (sample configured before the install
  existed, cached the build-tree config via `OpenCASCADE_DIR`).
- No third-party code is bundled inside the built toolkits at V8.0.1:
  a source-tree scan found no vendored components (gl2ps — bundled in
  older releases — is absent from 8.0.1), and TKOpenGl's EXTERNLIB
  lists only OCCT toolkits + Windows system libs. The only license the
  binaries carry is OCCT's LGPL-2.1 + exception. Verified 2026-10-13;
  re-run the scan when swapping tags
  (`Get-ChildItem tmp\occt\OCCT\src -Recurse -File |
  Where-Object Name -match 'LICENSE|COPYING|NOTICE'` plus a
  TKOpenGl/gl2ps existence check).
- When a third-party directory is used on Windows, OCCT searches
  `3RDPARTY_DIR` for product subdirectories (`freetype*`, ...) and
  requires `freetype.lib` + `freetype.dll`; `env.bat` bakes
  `THIRDPARTY_DIR` and `custom.bat` bakes the per-product dirs at
  configure time, and the CMake package config resolves its prefix from
  its own file location. The official MSVC x64 bundle for V8.0.1 is
  `3rdparty-vc14-64.zip` (188,827,053 bytes,
  sha256 `12d04f688c583b8e896758f65cd7012458fee921cedb792ec6caaf469b372598`).
- STEP writer transfer statistics print unconditionally on the default
  messenger in 8.0.1 (`XSControl_TransferWriter::TransferWriteShape`);
  `sample/` silences them via
  `Message::DefaultMessenger()->ChangePrinters().Clear()`.
