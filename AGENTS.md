# AGENTS.md

## Repository purpose

A starter application that wires three ETIB libraries together: evan (Vulkan
rendering; GLFW on desktop, OpenXR on Meta headsets), guillaume (UI framework)
and maïs (embedded Python scripting, desktop only). It is an application, not a library: nothing here is
installed or exported.

## Tech stack

- **Language**: C++20, plus Python for the application scripts
- **Build system**: CMake 3.28+ with presets (`default`, `release`, `siblings`)
- **Dependencies**: fetched with FetchContent at the revisions pinned in
  `cmake/Dependencies.cmake`; nothing is vendored
- **Tests**: CTest with a plain executable (`tests/ScriptingTest.cpp`)
- **Formatter**: clang-format 21.1.8 (LLVM-based, 80 columns, tabs)
- **Android**: a Gradle project in `platforms/android` (AGP, NDK, native
  activity) that reuses `src/` and the pinned libraries
- **CI**: GitHub Actions on Linux, macOS and Windows, plus the Android APK

## Build and test

```bash
cmake --preset default
cmake --build --preset default --parallel
ctest --preset default
cmake --build --preset default --target run   # opens a window
```

`GETTING_STARTED_MAX_FRAMES=120 ./build/bin/getting_started` runs the
application for 120 frames and exits, which is the way to check a change
without a person closing the window.

## Where things go

- `platforms/desktop/main.cpp` — the desktop host. The only file that knows
  all three libraries: start-up order, frame loop, when each Python hook is
  called.
- `platforms/android/` — the Gradle project and the Android host
  (`app/cpp/main.cpp`): OpenXR, APK assets, activity lifecycle, no Python.
- `src/` — code shared by both hosts, listed in `src/CMakeLists.txt`. It
  depends on guillaume only.
  - `src/Application.hpp` — the list of scenes.
  - `src/scenes/` — guillaume scenes. UI code only.
  - `src/AppContext.*`, `src/Frame.*` — state and per-frame helpers shared by
    the hosts and the scenes.
  - `src/scripting/Services.hpp` — the plain state the hosts fill and Python
    sees.
- `src/scripting/Scripting.*` — the adapter between the desktop host and
  Python. The only code that includes pybind11; not built for Android.
- `scripts/` — Python run by the application. Shell scripts go in `tools/`.
- `assets/` — application assets, copied next to the executable.

## Rules

- Scripts receive plain values in the structures of
  `src/scripting/Services.hpp`. Do not bind the engine, a scene or a widget.
- A Python command sets a flag in `Services`; the host acts on it in its loop.
- Everything Python can reach must outlive the `mais::ScriptRuntime`: keep it
  in `app::Context`.
- Hooks run on the main thread inside the frame. No blocking calls in them.
- Shared code under `src/` is compiled for Android: no mais, pybind11, evan
  or desktop-only API in it. A feature driven by a script needs a C++
  fallback, or it is missing on the headset.
- Add a scene to `src/Application.hpp`, never to one host only.
- Do not add a dependency to the `mais` target: its configure-time boundary
  check fails the build.
- Engine shaders and fonts are copied from the pinned libraries by
  `cmake/EngineAssets.cmake`, for both builds. Do not commit copies of them.
- Every source file starts with the MIT license header, and public
  declarations carry Doxygen comments, as in the libraries.
- Run `./tools/run-clang-format.sh` before committing.
- When a binding changes, update `tests/ScriptingTest.cpp` and
  `tests/scripts/fixture.py`.

## Reporting problems

When you find a bug or anything else worth fixing - a build or documentation
problem, a missing feature, a mismatch between two libraries - create a GitHub
issue on the repository that causes it: `etib-corp/evan`, `etib-corp/guillaume`,
`etib-corp/mais`, `etib-corp/utility`, or `etib-corp/getting-started` for this
application.

- Do it even when you worked around the problem and even when it is unrelated
  to the task. Do not ask first; give the issue link in your answer.
- File it where the code has to change, not where the symptom shows. Do not
  patch a library from this repository.
- Search for an existing issue first, and keep private data (home directory
  paths, tokens, e-mail addresses) out of the text: the repositories are
  public.

The `report-issue` skill has the steps, the title and label conventions and
the issue body.

## Agent tooling

`AGENTS.md` is the single source of instructions. Skills and subagents live
under `.agents/`; Claude Code reads them through symlinks, so there is one
copy of everything:

```text
AGENTS.md
CLAUDE.md        -> AGENTS.md
.agents/skills/  one directory per skill, each with a SKILL.md
.agents/agents/  one Markdown file per subagent
.claude/skills   -> ../.agents/skills
.claude/agents   -> ../.agents/agents
```

Edit the files under `.agents/`, never through `.claude/`. On Windows, clone
with `git config core.symlinks true` so the links are created.

| Skill | Use it to |
| --- | --- |
| `verify-app` | Build, test and run the desktop application for 120 frames. |
| `build-ui` | Add widgets, scenes and scene transitions. |
| `python-binding` | Expose values, settings, commands or hooks to Python. |
| `bump-libraries` | Move evan, guillaume, mais or utility to another revision. |
| `report-issue` | File an issue on the repository that causes a problem. |
| `android-headset` | Build, install and debug the APK for Meta headsets. |

| Subagent | Use it to |
| --- | --- |
| `library-scout` | Get exact signatures from the pinned library sources. |
| `app-verifier` | Run the verification and get a short report. |
| `boundary-reviewer` | Check a change against the rules above. |

## Git conventions

Commit and push only when a human asks for it. Commit messages follow
Conventional Commits, as in the libraries (`feat(scope): ...`, `fix: ...`).
