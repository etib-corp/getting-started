# Getting started

A ready-to-develop application built on the three XIDER libraries. It runs on
desktop (macOS, Linux, Windows) and on Meta headsets (Android, OpenXR):

| Library | What it does | In this application | Read more |
| --- | --- | --- | --- |
| [evan](https://github.com/etib-corp/evan) | Renders with Vulkan: a GLFW window on desktop, OpenXR on a headset. | Creates the window or the headset session, and the engine guillaume draws through. | [docs/evan.md](docs/evan.md) |
| [guillaume](https://github.com/etib-corp/guillaume) | UI framework: application shell, scenes, widgets, layout, events, theme and storage. | Owns the frame and the `HomeScene` widgets. | [docs/guillaume.md](docs/guillaume.md) |
| [maïs](https://github.com/etib-corp/mais) | Embedded Python runtime: interpreter lifecycle, script loading, hooks and host bindings. | Runs `scripts/app.py` and exposes the `host` module, on desktop and on a headset. | [docs/mais.md](docs/mais.md) |

Each page explains what the library does on its own, what it leaves to the
others, which parts the application uses and where they appear in the code.
[docs/README.md](docs/README.md) shows how the three fit together.

Clone it, build it, and you get a window with a few widgets and a Python
script driving one of them. Then replace the scene and the script with your
own.

The application also shows each library at work, with the code next to the
feature: a widget gallery, a tour of the scripting runtime and the engine
settings a script can propose. See [Feature tour](#feature-tour).

## Prerequisites

- CMake 3.28 or newer and a C++20 compiler
- Git and network access (the first configure downloads the libraries)
- The Vulkan SDK ([LunarG](https://vulkan.lunarg.com/sdk/home))
- Python 3.9 to 3.15 **with its development files** (`python3-dev` on Debian
  and Ubuntu; included in the python.org, Homebrew and Xcode installations)
- On Linux, the window-system packages:
  `bash tools/install_ubuntu_dependencies.sh`

## Build and run

```sh
cmake --preset default
cmake --build --preset default --parallel 8
cmake --build --preset default --target run
```

The first configure fetches evan, guillaume, maïs and their dependencies, and
the first build compiles them: allow a few minutes. Later builds only compile
what you change. Always give `--parallel` a number: with Makefiles, a bare
`--parallel` starts every compilation at once.

The executable is `build/bin/getting_started`. It can be started from any
directory.

Run the tests with:

```sh
ctest --preset default
```

## What is in the repository

```text
CMakeLists.txt            the desktop build; rename the app here
CMakePresets.json         default (debug), release, siblings (local libraries)
cmake/
  Dependencies.cmake      the pinned revisions of the libraries
  AndroidPython.cmake     the python.org package embedded in the Android build
  AssetTree.cmake         packages a directory as APK assets with a manifest
  EngineAssets.cmake      the shaders and fonts taken from the libraries
  Runtime.cmake           what is copied next to the desktop executable
src/                      code shared by every platform
  Application.hpp         the list of scenes
  AppContext.hpp          state shared by the frame loop and the scenes
  Frame.hpp               what every host does around a frame
  scenes/HomeScene.cpp    the start-up scene: build your UI here
  scenes/gallery/         the widget gallery: one scene per widget family
  scripting/Services.hpp  the state the hosts fill and Python can see
  scripting/Scripting.*   the Python bindings
platforms/
  desktop/main.cpp        the desktop host: start-up order and frame loop
  desktop/ScriptingTour.* the tour of the scripting runtime
  android/                the Gradle project for Meta headsets
docs/                     one page per library: what it does and where we use it
scripts/app.py            the Python hooks
scripts/features.py       the Python side of the scripting tour
assets/                   your models, images and textures
tests/                    tests that run without a window
tools/                    formatting and Linux set-up scripts
.github/workflows/        build and formatting checks
```

## How a session runs

On desktop, `platforms/desktop/main.cpp` owns the order. The libraries do not
know about each other.

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

On a headset, `platforms/android/app/cpp/main.cpp` follows the same order
with the Python packaged in the APK. Two differences: the proposal of step 2
is validated but not used (there is no window), and a script that fails to
start does not stop the application: the error goes to `adb logcat` and the
host writes the status line itself.

## Feature tour

Each feature of the libraries has code in this repository that uses it, and a
way to see it run.

### guillaume: the widget gallery

The `Widget gallery` and `Theme and storage` buttons of the start-up scene
open eleven pages, one scene and one source file each, under
`src/scenes/gallery/`:

| Page | File | What it shows |
| --- | --- | --- |
| 1 | `ButtonsPage.cpp` | `Button`, `IconButton`, floating action buttons, FAB menu, `SegmentedButton` |
| 2 | `SelectionPage.cpp` | `Checkbox`, `RadioButton`, `RadioGroup`, `Switch`, `Chip` |
| 3 | `TextPage.cpp` | `Text` (type scale), `Icon`, `Image`, `TextField`, `Search` |
| 4 | `ContainersPage.cpp` | `Surface` (elevation), `Card` |
| 5 | `ListsPage.cpp` | `Divider`, `Badge`, `List` |
| 6 | `FeedbackPage.cpp` | `ProgressIndicator`, `LoadingIndicator`, `Snackbar`, `Dialog`, `Menu`, `Tooltip` |
| 7 | `NavigationPage.cpp` | `AppBar`, `NavigationBar` |
| 8 | `DrawerPage.cpp` | `NavigationDrawer` |
| 9 | `MeshPage.cpp` | `Mesh`: a 3D model in the layout |
| 10 | `ThemePage.cpp` | colour schemes, a theme token changed at run time, accessibility |
| 11 | `StoragePage.cpp` | `LocalStorage` (kept after a restart) and `SessionStorage` |

`Page.cpp` holds what the pages share: the column, the rows (nested views)
and the Previous / Home / Next buttons, which are scene transitions. Every
interaction prints a `[gallery]` line in the terminal. To start on a page:

```sh
GETTING_STARTED_PAGE=6 ./build/bin/getting_started
```

Pages 1 to 9 follow guillaume's own
[widget gallery](https://github.com/etib-corp/guillaume/tree/main/examples/evan).

### maïs: the scripting tour

```sh
./build/bin/getting_started --scripting-tour
```

runs `platforms/desktop/ScriptingTour.cpp` instead of the application, without
a window. It calls the functions of `scripts/features.py` and prints one
`[tour]` line per check: lifecycle and its misuse, search paths, module
loading, introspection, arguments, typed return values, native arguments,
error reports with the Python traceback and its frames, the three failure
policies, call duration, a host command, thread affinity and restart. The
`scripting.tour` test runs it.

The hooks of the application itself (`configure`, `on_start`, `on_update`,
`on_shutdown`) are in `scripts/app.py` and called from
`platforms/desktop/main.cpp`.

### evan: engine settings and report

`configure(settings)` in `scripts/app.py` also proposes three engine settings,
validated like the window size and applied in `runSession()`
(`platforms/desktop/main.cpp`):

| Setting | Effect | evan API |
| --- | --- | --- |
| `target_fps` | Frame rate limit; 0 leaves the pacing to the display. | `Engine::setTargetFps()` |
| `msaa_samples` | Anti-aliasing: 1 (off), 2 or 4 samples. | `RenderSettings::msaaSamples` |
| `camera_controls` | W/A/S/D, Q/E, arrow keys and right-button drag move the view. | `Engine::setShouldCaptureViewportInput()` |

```sh
./build/bin/getting_started --engine-report
```

prints, after 120 frames, what the engine reports about itself
(`reportEngine()` in `platforms/desktop/main.cpp`): frame time, frame rate
limit, batching switches, last error, statistics window and AR capability.
evan's own switches are environment variables, for example `EVAN_STATS=1`,
`EVAN_PRESENT_MODE=immediate` or `EVAN_MSAA=4`.

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
   `src/CMakeLists.txt`.
2. List the class in the `app::Application` alias in `src/Application.hpp`.
   The first one is shown at start-up, on every platform.
3. Switch from a click handler with `goToScene<MyScene>()`.

### Expose something to Python

1. Add a field to `FrameContext` (or a new structure) in
   `src/scripting/Services.hpp`.
2. Bind it in `registerBindings()` in `src/scripting/Scripting.cpp`:
   `def_readonly` for values the script reads, `def_readwrite` for values it
   writes, `host.def(...)` for a command.
3. Fill it before the hook, or act on it after the hook, in
   `platforms/desktop/main.cpp`.
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
cmake --build --preset siblings --parallel 8
cmake --build --preset siblings --target run
```

The application then builds against the local checkouts instead of the pinned
revisions. For a single library, pass
`-DFETCHCONTENT_SOURCE_DIR_GUILLAUME=/path/to/guillaume` (or `_EVAN`, `_MAIS`,
`_UTILITY`) to the `default` preset.

### Rename the application

Change `APP_NAME` at the top of `CMakeLists.txt`. The executable and the
targets follow. The window title is set in `scripts/app.py`. For Android,
also change `APP_NAME` in `platforms/android/app/cpp/CMakeLists.txt`, the
library name and label in `AndroidManifest.xml`, and the application id in
`app/build.gradle.kts`.

## Meta headset (Android)

`platforms/android` is a Gradle project that builds the same application as
an APK for Meta Quest headsets. It reuses `src/`, `assets/` and the pinned
libraries, with evan's OpenXR backend and a fully native activity.

You need a JDK 17 or newer and the Android SDK (`ANDROID_HOME` set, or
`sdk.dir=<path>` in `platforms/android/local.properties`), with the SDK's
CMake 3.31.5 package installed (SDK Manager > SDK Tools, or
`sdkmanager "cmake;3.31.5"`). Gradle downloads the NDK itself. Opening
`platforms/android` in Android Studio works too.

```sh
cd platforms/android
./gradlew assembleDebug
adb install -r app/build/outputs/apk/debug/app-debug.apk
```

The headset must be in developer mode and connected over USB. The application
then appears in the headset's library under "Unknown sources". Logs go to
`adb logcat`.

Differences from desktop:

- **Python comes with the APK.** The build downloads the Android package
  of Python 3.15.2 from [python.org](https://www.python.org/downloads/release/python-3152/)
  (`cmake/AndroidPython.cmake`), links its `libpython3.15.so` and packages its
  standard library and `scripts/` as assets. At the first start they are
  copied to the application's private directory. `print` goes to
  `adb logcat` with the tag `python.stdout`.
- **One script, two Pythons.** Desktop runs `scripts/app.py` with the Python
  of the machine, the headset with 3.15: keep the script valid for both and
  on the standard library (no pip package is in the APK).
- **A script problem does not stop the application.** It is logged with the
  tag `getting_started` and the host writes the status line itself
  (`app::writeDefaultStatus` in `src/Frame.cpp`).
- **No window settings.** The headset decides the resolution.
- **VR by default.** For passthrough and hand tracking, see the comment in
  `platforms/android/app/cpp/main.cpp`.

See [platforms/android/README.md](platforms/android/README.md) for details.

## Tests

| Test | What it checks |
| --- | --- |
| `scripting.app_script` | `scripts/app.py` loads, proposes valid settings and survives a few frames. |
| `scripting.bindings` | Each binding of the `host` module, with `tests/scripts/fixture.py`. |
| `scripting.tour` | The scripting tour: every check of `--scripting-tour` passes. |
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

- **`Could NOT find Vulkan`**: install the Vulkan SDK and, on macOS and
  Windows, run its environment script (or set `VULKAN_SDK`) before configuring.
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
