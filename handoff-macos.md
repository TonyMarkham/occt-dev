# Handoff — writing `occt-mac.md` (macOS)

Advice for the session that will write the macOS build guide. It
assumes you have just cloned this repository. `occt-win.md` and
`occt-linux.md` are the finished references; this file is the macOS
session's starting brief, in the same spirit as `handoff-linux.md`
was for Linux. Delete it when `occt-mac.md` lands.

## Verify at the tag before writing anything

Do not copy claims from the other two docs — curl the raw files at
`V8.0.1` and grep them. This method worked well for Windows and Linux:

```
curl -s https://raw.githubusercontent.com/Open-Cascade-SAS/OCCT/V8.0.1/adm/cmake/<file>.cmake
```

Files that earned their keep: top-level `CMakeLists.txt`,
`adm/cmake/occt_csf.cmake`, `adm/cmake/occt_toolkit.cmake`,
`adm/cmake/occt_defs_flags.cmake`, `adm/cmake/occt_macros.cmake`.

macOS-specific facts to confirm at the tag (three of the five are
already verified — re-check when touching the tag):

1. **No X11 on macOS.** `USE_XLIB` defaults OFF on APPLE (verified:
   top-level CMakeLists sets it explicitly); CSF libraries come from
   `occt_set_apple_csf_vars()` in `occt_csf.cmake` — AppKit/IOKit/objc
   frameworks, OpenGL framework when `USE_XLIB` is off. Prerequisites
   are *smaller* than Linux's: no X11/GL dev packages, just compiler +
   CMake + Ninja + zip (Homebrew).
2. **The patch is needed on macOS too.** Mach-O produces versioned
   dylibs (`libTKernel.8.0.1.dylib` + symlink chain) unless the same
   three `set_target_properties` lines in `adm/cmake/occt_toolkit.cmake`
   lose `SOVERSION`/`VERSION`. Keep the dated-comment convention and
   verify with `grep -n` — do not trust line numbers (they were
   207–209 at V8.0.1, but verify). Note the APPLE-only
   `BUILD_WITH_INSTALL_RPATH`/`INSTALL_NAME_DIR` block nearby is
   unrelated to the patch; leave it alone.
3. **Do not reuse the Linux size story.** The `-s` link-time strip is
   applied for Clang on Linux only, and explicitly skipped on macOS
   ("obsolete on macOS" comment in `occt_defs_flags.cmake`); GCC's
   `-s` does not exist there. Measure dylib sizes after building;
   never predict them in the docs or release notes.
4. **The runtime-library step (macOS equivalent of Linux 3.5) is the
   top trap.** `LD_LIBRARY_PATH` does not exist; the candidate is
   `DYLD_LIBRARY_PATH` — but whether the unversioned dylibs resolve
   via install_name/rpath or need DYLD must be *tested on the machine*,
   not assumed. This is the doc's most likely copy of the bug that bit
   twice on Linux (see process lessons).
5. **Compiler floor:** Apple Clang ≥ 11.0 (fatal check verified at
   `occt_macros.cmake`; generic Clang ≥ 7.0).

## Doc invariants (keep the three platform docs consistent)

- `INSTALL_DIR_LAYOUT=Unix`, `INSTALL_DIR_WITH_VERSION=OFF` — same
  relative layout everywhere (on macOS these are *not* the defaults to
  double-check the top-level CMakeLists logic, unlike Linux where they
  are; spell them out as on Windows).
- `BUILD_MODULE_Draw=OFF`, `BUILD_LIBRARY_TYPE=Shared`,
  `USE_FREETYPE=OFF` (FreeType optional section: Homebrew
  `freetype`; fontconfig is not in the macOS CSF path — verify).
- Install into `dist/occt/<arch>-macos` — pick the arch name (arm64 vs
  x86_64) from the machine actually used and keep it consistent with
  the zip names.
- Validation is `sample/` only: box → STEP → read back →
  `Volume: 6000 mm^3`, exit 0. Never remove the
  `Message::DefaultMessenger()->ChangePrinters().Clear()` line from
  `sample/hello_occt.cpp`.
- Packaging mirrors section 4: dev zip + libs-only zip (`*.dylib`
  instead of `*.so`), three legal files at each zip root. The 2.1-style
  patch guarantees no symlink chains, so the glob captures exactly the
  toolkit libraries.

## Process lessons (each one cost real time on Linux)

- **Every step must be one copy-pastable command with its expected
  result.** A prose-only step gets skipped (the THIRD_PARTY_NOTICES.md
  provenance update was); a compound subshell gets cursed at (the
  `(cd ... && zip ...)` was).
- **Mentally execute the whole runbook against a real filesystem before
  handing it over.** The `cd ../..`-instead-of-`../../..` bug and the
  landing-in-`dist/` trap were both catchable at the desk. After every
  `cd`, re-check what every later relative path assumes about cwd.
- **The doc must carry its own dependency install.** Never phrase
  anything as "already present" — the reader's machine is bare by
  definition (this correction was applied to the Linux doc after the
  fact; start with it from step one).
- **Changing a shared file makes every shipped artifact from it stale.**
  When `third_party/occt/THIRD_PARTY_NOTICES.md` gained its Linux
  provenance, the already-uploaded draft-release assets silently
  diverged from the repo file. Rule: shared file changes → immediately
  reconcile everything derived from it, including GitHub assets. This
  works without the original build machine for text-only entries:
  download the asset, `zip -q <archive> THIRD_PARTY_NOTICES.md` from a
  directory holding the new file, verify with
  `unzip -p <archive> THIRD_PARTY_NOTICES.md | grep <marker>`,
  re-upload with `gh release upload --clobber`.
- **Write release bodies to a file and `gh release edit --notes-file`.**
  The hand-edited v0.1.0 body had an unclosed code fence and lost two
  headers. Also: `gh release edit` on a draft returns an
  `untagged-...` URL — that is normal; verify with `gh release view`.
- **After the build, before zipping:** update the provenance file with
  real versions/dates, then zip; if the file changes after zipping,
  re-zip and re-upload. `grep -n` the patch block before and after the
  edit; verify unversioned output with
  `test -f .../lib/libTKernel.dylib && test ! -L .../lib/libTKernel.dylib`.
- **The repo file `THIRD_PARTY_NOTICES.md` is one dual-platform record**
  (Windows x64 unmodified, Linux x86_64 with the one-file patch). Add a
  macOS provenance block to it during the macOS packaging step — do not
  fork it per platform — and refresh all shipped assets afterwards.