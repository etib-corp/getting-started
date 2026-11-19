# evan

[evan](https://github.com/etib-corp/evan) is the rendering and platform layer
of the application. It opens the window, owns the Vulkan device and swapchain,
loads shaders and meshes, and draws and presents a frame.

A build of evan has exactly one backend and one platform. This repository
uses the **GLFW backend** on macOS, Linux and Windows, and the **OpenXR
backend** on Android for Meta headsets.

## What it does

- Opens a desktop window, or an OpenXR session on a headset, and routes keyboard, mouse and window events to the
  application.
- Owns the Vulkan instance, logical device, swapchain, per-frame command
  buffers and synchronisation.
- Provides mesh, shader, material and texture helpers, plus the render
  objects registered with the engine and drawn each frame.
- Clears, renders and presents a frame when the application asks for one.

## What it does not do

- It does not choose what to draw or where: there are no widgets, no layout
  and no scene management of the kind guillaume provides.
- It does not run scripts, and knows nothing about Python.
- It does not own the application loop. guillaume asks it to poll, draw and
  present once per frame.

## Key concepts

| Term | Meaning |
| --- | --- |
| **Engine** | Owns the Vulkan state and coordinates update, draw and present. |
| **Platform** (`IPlatform`) | Abstraction of the window and input source. The application provides one implementation. |
| **Backend** | How the platform is implemented: GLFW on desktop, OpenXR on a headset. |
| **Render object** | GPU content registered with the engine and drawn each frame. |
| **Mesh, shader, material** | The description of renderable content. |

## How this application uses it

- `platforms/desktop/main.cpp` includes `<evan/Engine.hpp>`, `<evan/IPlatform.hpp>` and the
  platform header of the host OS.
- `makePlatform()` builds `evan::MacOsDesktopPlatform`,
  `evan::LinuxDesktopPlatform` or `evan::WindowsPlatform` from the settings
  the script proposed. The title and size therefore reach evan through the
  host, after validation.
- `runSession()` creates one `evan::Engine` on the shared
  `utility::RessourceProvider`, then hands it to `app::Application`
  (`src/Application.hpp`). guillaume owns it from there.
- `platforms/android/app/cpp/main.cpp` does the same with
  `evan::AndroidXrPlatform`, reads the files from the APK assets and follows
  the activity lifecycle.
- `application.routine()` is the only place evan is asked to poll input,
  clear, draw and present. The application itself contains no Vulkan call.
- `cmake/Dependencies.cmake` forces `BUILD_FOR_GLFW` and the platform flag of
  the host system (`BUILD_FOR_MACOS`, `BUILD_FOR_LINUX` or
  `BUILD_FOR_WINDOWS`), or `BUILD_FOR_OPENXR` and `BUILD_FOR_ANDROID`, and
  rejects any other system at configure time.
- `cmake/Runtime.cmake` copies the shaders, default texture and fonts next to
  the executable, because they are loaded by relative path (the Android
  build packages them as APK assets). A file placed
  under `assets/` at the same path replaces the library's one.

## Where the code lives

| File | Role |
| --- | --- |
| `platforms/desktop/main.cpp` | Creates the platform and the engine, keeps the frame loop. |
| `platforms/android/app/cpp/main.cpp` | The same on a headset, with OpenXR. |
| `cmake/Dependencies.cmake` | Selects the backend and platform, pins the revision. |
| `cmake/Runtime.cmake` | Copies shaders, textures and fonts to `build/bin/`. |
| `assets/README.md` | Explains how to override a library asset. |

## Version

The application pins evan `v2026.4.7` in `cmake/Dependencies.cmake`. The
shaders and textures are taken from the same pinned sources, so they follow
the revision. To move to another release, change the revision there and
rebuild; see "Update a library" in the [README](../README.md#update-a-library).

## Learn more

- [evan README](https://github.com/etib-corp/evan#readme)
- [How Evan Works](https://github.com/etib-corp/evan/blob/main/docs/HOW_EVAN_WORKS.md)
- [Architecture](https://github.com/etib-corp/evan/blob/main/docs/ARCHITECTURE.md)
- [Getting Started](https://github.com/etib-corp/evan/blob/main/docs/GETTING_STARTED.md)
