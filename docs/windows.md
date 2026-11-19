# Windows

Everything needed to build and run the application on a Windows computer,
from a machine with nothing installed.

The application is built with MSVC and the Visual Studio generator. CMake
downloads evan, guillaume, maïs and utility during the first configure, so
only the tools below have to be installed by hand.

## What you need

| Tool | Version | What it is for |
| --- | --- | --- |
| Visual Studio 2022 Build Tools | the latest 17.x, with the **Desktop development with C++** workload | MSVC compiles the application and the four libraries, and brings the Windows SDK |
| CMake | 3.28 or newer | Configures and builds |
| Git | any recent version | `FetchContent` clones the libraries at their pinned revisions |
| Python | 3.9 to 3.14, 64-bit, with its development files | maïs embeds the interpreter, so `Python.h` and `python3XX.lib` are needed |
| Vulkan SDK | 1.3 or newer | evan links `vulkan-1.lib` and loads its shaders; the SDK provides the headers, the import library and the validation layers |
| A Vulkan-capable GPU driver | current | The window is drawn with Vulkan; the driver ships the loader in `System32` |

The python.org installer and winget's `Python.Python.3.12` package include the
development files. A `python` command that only opens the Microsoft Store is
an execution alias, not an installation: turn it off under *Settings > Apps >
Advanced app settings > App execution aliases*.

## 1. Install the compiler

In an elevated terminal:

```powershell
winget install --id Microsoft.VisualStudio.2022.BuildTools --exact `
    --accept-package-agreements --accept-source-agreements `
    --override "--quiet --wait --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended"
```

`--includeRecommended` adds the MSVC x64/x86 toolset and the Windows SDK. The
same result comes from the Visual Studio Installer with the *Desktop
development with C++* workload, in the full Visual Studio or in Build Tools.

Check that the toolset is there; the path is printed when it is:

```powershell
& "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" `
    -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
    -property installationPath
```

## 2. Install CMake and Git

```powershell
winget install --id Kitware.CMake --exact --accept-package-agreements --accept-source-agreements
winget install --id Git.Git --exact --accept-package-agreements --accept-source-agreements
```

Both installers add themselves to `PATH`. If they do not, add
`C:\Program Files\CMake\bin` and `C:\Program Files\Git\cmd` to the user
`PATH` and open a new terminal.

```powershell
cmake --version   # 3.28 or newer
git --version
```

## 3. Install Python

```powershell
winget install --id Python.Python.3.12 --exact --scope user `
    --accept-package-agreements --accept-source-agreements
```

A user-scoped install needs no administrator rights and puts the interpreter
and its `Scripts` directory on the user `PATH`. Check the development files,
which is what the configure step looks for:

```powershell
python --version
Test-Path "$env:LOCALAPPDATA\Programs\Python\Python312\include\Python.h"   # True
```

If several interpreters are installed, tell CMake which one to use with
`-DPython_EXECUTABLE=<path to python.exe>`; the include and library
directories follow from it.

## 4. Install the Vulkan SDK

Download the Windows installer from [vulkan.lunarg.com](https://vulkan.lunarg.com/sdk/home)
and install it; it lands in `C:\VulkanSDK\<version>`. Point the environment
at it, because CMake's `FindVulkan` reads `VULKAN_SDK` and recent SDK
versions no longer ship a script that sets it:

```powershell
[Environment]::SetEnvironmentVariable('VULKAN_SDK', 'C:\VulkanSDK\1.4.363.1', 'User')
[Environment]::SetEnvironmentVariable(
    'Path',
    [Environment]::GetEnvironmentVariable('Path', 'User') + ';C:\VulkanSDK\1.4.363.1\Bin',
    'User'
)
```

Open a new terminal, then check that the loader and a device are found:

```powershell
vulkaninfo --summary
```

## 5. Build, test and run

In a new terminal, from the directory that contains the checkout:

```powershell
git clone https://github.com/etib-corp/getting-started.git
cd getting-started
cmake --preset default
cmake --build --preset default --parallel
ctest --preset default
cmake --build --preset default --target run
```

- The first configure downloads the four libraries at the revisions pinned in
  `cmake/Dependencies.cmake`; it needs network access and takes a few minutes.
- The first build compiles them with the application. A Windows Debug build
  takes tens of minutes on a laptop; later builds only compile what changed.
- `ctest` runs the two scripting tests, which start Python but open no window.
- The `run` target builds, copies the runtime files next to the executable and
  starts it from `build\bin\Debug`.

The `release` preset builds the same application without tests, optimized and
in `build-release`; it is the one to time or ship.

To check a change without a person closing the window, limit the frames:

```powershell
$env:GETTING_STARTED_MAX_FRAMES = 120
cmake --build --preset default --target run
```

or start the executable yourself, from the directory of the executable:

```powershell
cd build\bin\Debug
$env:GETTING_STARTED_MAX_FRAMES = 120
.\getting_started.exe
```

The window opens, draws 120 frames, prints `app.py: stopped after 120 frames`
and exits. The same run is registered as the `app.smoke` test with
`-DAPP_ENABLE_GPU_TESTS=ON`.

## What the build does differently on Windows

Three Windows details are handled by the repository, so the commands above
work as they are:

| Detail | Where | Why |
| --- | --- | --- |
| The application is compiled with the release MSVC runtime (`MultiThreadedDLL`) | `cmake/Dependencies.cmake` | utility and evan force that runtime on everything they build, in every configuration, so a Debug application built with the default `/MDd` does not link ([utility#131](https://github.com/etib-corp/utility/issues/131), [evan#183](https://github.com/etib-corp/evan/issues/183)) |
| OpenAL Soft's C++20 modules are turned off | `cmake/Dependencies.cmake` | utility's vendored OpenAL Soft enables them for the Visual Studio generator, and MSBuild cannot build them (`error C2230: could not find module 'alc.context'`, [utility#132](https://github.com/etib-corp/utility/issues/132)) |
| The interpreter's directory comes first on `PATH` for the tests and the `run` target | `cmake/Runtime.cmake`, `tests/CMakeLists.txt` | The executables load `python3XX.dll` from the Python installation, which is not on `PATH` when the build was configured with `-DPython_EXECUTABLE`; without it they stop at start-up with `0xc0000135` |

The first two are workarounds for the pinned library revisions and are meant
to disappear with the issues above.

## Editor

Visual Studio Code, with the *CMake Tools* and *C/C++* extensions, configures
the same presets: *CMake: Select Configure Preset* > `default`, then *CMake:
Build* and *CMake: Debug*. When the editor does not inherit the variables
set above, add `-DPython_EXECUTABLE` under `cmake.configureSettings` in
`settings.json`, and start the editor from a session that has `VULKAN_SDK`.

## Troubleshooting

| Symptom | Cause and fix |
| --- | --- |
| `Could NOT find Vulkan` | `VULKAN_SDK` is not set in the process that configures. Environment variables are read at start-up: open a new terminal, then configure again. |
| `Could NOT find Python` or a missing `Python.h` | The interpreter found is not the one with the development files. Pass `-DPython_EXECUTABLE=<path to python.exe>`. |
| `error LNK2038: mismatch detected for 'RuntimeLibrary'` | An old build directory configured before this repository set the runtime. Delete `build\` and configure again, or pass `-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL`. |
| `error C2230: could not find module 'alc.context'` | An old build directory configured with OpenAL modules on. Delete `build\` and configure again, or pass `-DALSOFT_ENABLE_MODULES=OFF`. |
| The tests fail with `Exit code 0xc0000135` | `0xc0000135` is `STATUS_DLL_NOT_FOUND`, here `python3XX.dll`. Add the Python installation directory to `PATH`; the tests and the `run` target add it themselves when CMake knows the interpreter. |
| Starting the executable says it was "blocked by your organization's Device Guard policy" | Windows Smart App Control refuses freshly built, unsigned executables and has no exclusion list. Turn it off under *Windows Security > App & browser control > Smart App Control*; this cannot be undone without reinstalling Windows, and it needs a restart. |
| `Could not create named generator "Visual Studio 17 2022"` | No compatible Visual Studio is installed: install the Build Tools (step 1), or select another generator with `-G`. |
| Validation-layer messages in the terminal | A Debug build enables them when the SDK's layers are installed. The warnings come from the libraries and appear in their own examples too. |

The same steps run in CI: `.github/workflows/build.yml` uses `windows-latest`,
`actions/setup-python` with Python 3.12 and the LunarG SDK via
`humbletim/setup-vulkan-sdk`.
