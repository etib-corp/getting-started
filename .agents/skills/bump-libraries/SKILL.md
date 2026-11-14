---
name: bump-libraries
description: Update evan, guillaume, mais or utility to a newer revision, or diagnose a build that broke after such an update. Use when asked to upgrade, bump or pin a library version.
---

# Update a pinned library

The revisions are the four `APP_*_REVISION` variables at the top of
`cmake/Dependencies.cmake`. Nothing else stores a version.

## Steps

1. Find the target revision. Prefer a release tag; use a full commit hash when
   the release has no tag.

   ```sh
   git ls-remote --tags https://github.com/etib-corp/<library>.git | tail -5
   git ls-remote https://github.com/etib-corp/<library>.git refs/heads/main
   ```

2. Change one variable. Update one library at a time so a failure has one
   cause. Keep the comment next to the variable true.
3. Read what changed before building: `CHANGELOG.md` and, for guillaume,
   `docs/MIGRATION_GUIDE.md` in the library's repository. Commits marked `!`
   (for example `refactor(ecs)!:`) change the API.
4. Reconfigure and build, then follow the `verify-app` skill, including the
   windowed run: a shader or font mismatch only shows at run time.

   ```sh
   cmake --preset default && cmake --build --preset default --parallel 8
   ```

## Things that move together

- **utility**: evan and guillaume both build against it. It is pinned here so
  both get the same revision. When a newer evan or guillaume fails to compile
  inside its own sources, utility is probably too old: move it to the
  revision of utility's `main` at the date of the library's release.
- **Shaders and fonts** are copied from guillaume's
  `examples/evan/assets` and `fonts/` at the pinned revision
  (`cmake/EngineAssets.cmake`, used by the desktop and Android builds). If
  guillaume moves those directories, update the paths there. If evan changes
  its shaders and guillaume's copy lags, the window opens and draws nothing or reports pipeline errors.
- **mais and pybind11**: mais selects the pybind11 version and the supported
  Python range. A configure error naming the Python version comes from mais.
- **CMake options**: the three libraries share option names (`BUILD_TESTING`,
  `BUILD_EXAMPLES`, `DEV_MODE`, ...). `cmake/Dependencies.cmake` forces the
  shared ones off. Check a new library version for new shared options.
- **Android**: the same revisions are used by the Android build, with evan's
  OpenXR backend. A bump can break only that build; check it with the
  `android-headset` skill or CI.

## Do not

- Point a revision at a branch name: builds would stop being reproducible.
- Edit anything under `build/_deps/`. To try a change in a library, use a
  local checkout with the `siblings` preset or
  `-DFETCHCONTENT_SOURCE_DIR_<NAME>=<path>`.
