Run the release build script for the TinTin2 VST3 plugin and fix any compiler errors that arise.

Steps:
1. Run `./build_release.sh 2>&1` from the repo root and capture the full output.
2. If the build succeeds, report success and the install path.
3. If there are compiler **errors** (not warnings), identify the root cause in the source files and fix them with minimal changes — do not refactor logic, rename things, or clean up surrounding code. Just get it compiling.
4. Re-run the build after fixing to confirm it passes.
5. Do NOT delete the build folder between attempts — CMake caches intermediate state there. Only delete it if the user explicitly asks for a clean build.

Note: Warnings are acceptable for now. Only treat `error:` lines as blockers.
