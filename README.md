# Getting started

A ready-to-develop desktop application built on the three XIDER libraries:

| Library | What it does | In this application | Read more |
| --- | --- | --- | --- |
| [evan](https://github.com/etib-corp/evan) | Opens the window and renders with Vulkan, through its GLFW desktop backend. | Creates the window and the engine guillaume draws through. | [docs/evan.md](docs/evan.md) |
| [guillaume](https://github.com/etib-corp/guillaume) | UI framework: application shell, scenes, widgets, layout, events, theme and storage. | Owns the frame and the `HomeScene` widgets. | [docs/guillaume.md](docs/guillaume.md) |
| [maïs](https://github.com/etib-corp/mais) | Embedded Python runtime: interpreter lifecycle, script loading, hooks and host bindings. | Runs `scripts/app.py` and exposes the `host` module. | [docs/mais.md](docs/mais.md) |

Each page explains what the library does on its own, what it leaves to the
others, which parts the application uses and where they appear in `src/`.
[docs/README.md](docs/README.md) shows how the three fit together.

Clone it, build it, and you get a window with a few widgets and a Python
script driving one of them. Then replace the scene and the script with your
own.

## Prerequisites

- CMake 3.28 or newer and a C++20 compiler
- Git and network access (the first configure downloads the libraries)
- The Vulkan SDK ([LunarG](https://vulkan.lunarg.com/sdk/home))
- Python 3.9 to 3.14 **with its development files** (`python3-dev` on Debian
  and Ubuntu; included in the python.org, Homebrew and Xcode installations)
- On Linux, the window-system packages:
  `bash tools/install_ubuntu_dependencies.sh`
- On Windows, [docs/windows.md](docs/windows.md) has the commands for the
  compiler, CMake, Git, Python and the Vulkan SDK, and explains what the build
  does differently there.

## Build and run

```sh
cmake --preset default
cmake --build --preset default --parallel
cmake --build --preset default --target run
```

The first configure fetches evan, guillaume, maïs and their dependencies, and
the first build compiles them: allow a few minutes. Later builds only compile
what you change.

The executable is `build/bin/getting_started`
(`build\bin\Debug\getting_started.exe` with the Visual Studio generator, which
adds the configuration to the path). It can be started from any directory, as
long as the Python installation stays on `PATH` on Windows.

Run the tests with:

```sh
ctest --preset default
```

## What is in the repository

```text
CMakeLists.txt            the application target; rename the app here
CMakePresets.json         default (debug), release, siblings (local libraries)
cmake/
  Dependencies.cmake      the pinned revisions of the libraries
  Runtime.cmake           what is copied next to the executable
src/
  main.cpp                the host: start-up order and frame loop
  AppContext.hpp          state shared by the frame loop and the scenes
  scenes/HomeScene.cpp    the start-up scene: build your UI here
  scripting/Scripting.*   what Python can see of the application
docs/                     one page per library: what it does and where we use it,
                          plus docs/windows.md for the Windows setup
scripts/app.py            the Python hooks
assets/                   your models, images and textures
tests/                    tests that run without a window
tools/                    formatting and Linux set-up scripts
.github/workflows/        build and formatting checks
```

## How a session runs

`src/main.cpp` owns the order. The libraries do not know about each other.

1. maïs starts Python and loads `scripts/app.py`.
2. `configure(settings)`: the script proposes the window title and size.
3. The host validates the proposal. An invalid one is reported and replaced
   by the defaults.
4. evan creates the window and the engine; guillaume enters `HomeScene`.
5. `on_start(context)`.
6. Every frame: `on_update(context)`, then one guillaume frame (events,
   layout, render, present).
7. `on_shutdown(context)`, then Python stops.

In the sample, the button increments `context.clicks`, the script reads it in
`on_update` and writes `context.status`, and the host shows that text in the
window. This is the full round trip: widget, host, Python, host, widget.

The same sequence, library by library, is in
[docs/README.md](docs/README.md).

A failing `on_update` is printed once with its Python traceback and the window
stays open.

## Common tasks

### Add widgets

Edit `HomeScene::onEnter()` in `src/scenes/HomeScene.cpp`. Each widget has a
builder under `guillaume/entities/`. The
[widget gallery](https://github.com/etib-corp/guillaume/tree/main/examples/evan)
shows every widget with the code that creates it. See
[docs/guillaume.md](docs/guillaume.md) for how guillaume builds, lays out and
themes them.

### Add a scene

1. Copy `src/scenes/HomeScene.*` to a new class and add the `.cpp` to
   `add_executable()` in `CMakeLists.txt`.
2. List the class in `guillaume::Application<HomeScene, MyScene>` in
   `src/main.cpp`. The first one is shown at start-up.
3. Switch from a click handler with `goToScene<MyScene>()`.

### Expose something to Python

1. Add a field to `FrameContext` (or a new structure) in
   `src/scripting/Scripting.hpp`.
2. Bind it in `registerBindings()` in `src/scripting/Scripting.cpp`:
   `def_readonly` for values the script reads, `def_readwrite` for values it
   writes, `host.def(...)` for a command.
3. Fill it before the hook, or act on it after the hook, in `src/main.cpp`.
4. Add a check in `tests/ScriptingTest.cpp`.

Scripts receive plain values, never the engine or a widget. A command sets a
flag that the host reads; it does not act by itself. See
[docs/mais.md](docs/mais.md) for the runtime, the hooks and the ownership
rules, and maïs's
[host integration guide](https://github.com/etib-corp/mais/blob/main/docs/HOST_INTEGRATION.md)
for the reasons.

### Add an asset

Put the file under `assets/` and load it by its path relative to that
directory, for example `models/chair.obj`. See `assets/README.md`.

### Edit a script

Edit `scripts/app.py`, then run the `run` target again: scripts are copied on
every build, and nothing is recompiled.

### Update a library

Change the revision in `cmake/Dependencies.cmake`, then configure and build
again. The engine's shaders and the fonts come from the same pinned sources,
so they follow the revision.

### Work on a library and the application together

With evan, guillaume and maïs checked out next to this repository:

```sh
cmake --preset siblings
cmake --build --preset siblings --parallel
cmake --build --preset siblings --target run
```

The application then builds against the local checkouts instead of the pinned
revisions. For a single library, pass
`-DFETCHCONTENT_SOURCE_DIR_GUILLAUME=/path/to/guillaume` (or `_EVAN`, `_MAIS`,
`_UTILITY`) to the `default` preset.

### Rename the application

Change `APP_NAME` at the top of `CMakeLists.txt`. The executable and the
targets follow. The window title is set in `scripts/app.py`.

## Tests

| Test | What it checks |
| --- | --- |
| `scripting.app_script` | `scripts/app.py` loads, proposes valid settings and survives a few frames. |
| `scripting.bindings` | Each binding of the `host` module, with `tests/scripts/fixture.py`. |
| `app.smoke` | The window opens, 120 frames are drawn, the application exits. |

`app.smoke` needs a display and a Vulkan driver, so it is off by default:

```sh
cmake --preset default -DAPP_ENABLE_GPU_TESTS=ON
```

The same frame limit is available by hand:
`GETTING_STARTED_MAX_FRAMES=120 ./build/bin/getting_started`.

## Code style

C++ is formatted with clang-format **21.1.8** (LLVM-based, 80 columns, tabs).
The version is pinned because the output changes between releases:

```sh
pip install 'clang-format==21.1.8'
./tools/run-clang-format.sh
```

To format staged files on every commit:

```sh
git config core.hooksPath .githooks
```

`-DAPP_DEV_MODE=ON` compiles the application's own code with `-Wall -Wextra
-Wpedantic -Werror`. CI builds with it.

## Troubleshooting

- **`Could NOT find Vulkan`**: install the Vulkan SDK and set `VULKAN_SDK`
  before configuring. Recent Windows SDKs no longer ship a script that sets
  it; [docs/windows.md](docs/windows.md) has the two lines.
- **Windows will not start the executable, "blocked by your organization's
  Device Guard policy"**: Smart App Control blocks freshly built, unsigned
  binaries and has no exclusion list. See the Windows page.
- **`Could NOT find Python` or a missing `Python.h`**: install the Python
  development files, or select an interpreter with
  `-DPython_EXECUTABLE="$(command -v python3)"`.
- **Vulkan validation messages in the terminal**: a debug build enables the
  validation layers when they are installed. The image-layout errors and the
  "vertex attribute not consumed" warnings come from the libraries; the
  guillaume widget gallery prints the same ones.
- **Very verbose output**: debug builds of the libraries log every frame.
  Use the `release` preset, or redirect the output, when that gets in the way.

## License

Released under the [MIT License](LICENSE). The fonts copied next to the
executable (Roboto, Material Symbols) keep their own licenses, which are
copied with them.
