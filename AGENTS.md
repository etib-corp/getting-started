# AGENTS.md

## Repository purpose

A starter desktop application that wires three ETIB libraries together: evan
(Vulkan rendering, GLFW window), guillaume (UI framework) and maïs (embedded
Python scripting). It is an application, not a library: nothing here is
installed or exported.

## Tech stack

- **Language**: C++20, plus Python for the application scripts
- **Build system**: CMake 3.28+ with presets (`default`, `release`, `siblings`)
- **Dependencies**: fetched with FetchContent at the revisions pinned in
  `cmake/Dependencies.cmake`; nothing is vendored
- **Tests**: CTest with a plain executable (`tests/ScriptingTest.cpp`)
- **Formatter**: clang-format 21.1.8 (LLVM-based, 80 columns, tabs)
- **CI**: GitHub Actions on Linux, macOS and Windows

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

- `src/main.cpp` — the host. The only file that knows all three libraries:
  start-up order, frame loop, when each Python hook is called.
- `src/scenes/` — guillaume scenes. UI code only; no pybind11, no evan.
- `src/scripting/` — the adapter between the host and Python. The only code
  that includes pybind11.
- `src/AppContext.*` — state shared by the host loop and the scenes.
- `scripts/` — Python run by the application. Shell scripts go in `tools/`.
- `assets/` — application assets, copied next to the executable.

## Rules

- Scripts receive plain values in the structures of
  `src/scripting/Scripting.hpp`. Do not bind the engine, a scene or a widget.
- A Python command sets a flag in `Services`; the host acts on it in its loop.
- Everything Python can reach must outlive the `mais::ScriptRuntime`: keep it
  in `app::Context`.
- Hooks run on the main thread inside the frame. No blocking calls in them.
- Do not add a dependency to the `mais` target: its configure-time boundary
  check fails the build.
- Engine shaders and fonts are copied from the pinned libraries by
  `cmake/Runtime.cmake`. Do not commit copies of them.
- Every source file starts with the MIT license header, and public
  declarations carry Doxygen comments, as in the libraries.
- Run `./tools/run-clang-format.sh` before committing.
- When a binding changes, update `tests/ScriptingTest.cpp` and
  `tests/scripts/fixture.py`.

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
| `verify-app` | Build, test and run the application for 120 frames. |
| `build-ui` | Add widgets, scenes and scene transitions. |
| `python-binding` | Expose values, settings, commands or hooks to Python. |
| `bump-libraries` | Move evan, guillaume, mais or utility to another revision. |

| Subagent | Use it to |
| --- | --- |
| `library-scout` | Get exact signatures from the pinned library sources. |
| `app-verifier` | Run the verification and get a short report. |
| `boundary-reviewer` | Check a change against the rules above. |

## Git conventions

Commit and push only when a human asks for it. Commit messages follow
Conventional Commits, as in the libraries (`feat(scope): ...`, `fix: ...`).
