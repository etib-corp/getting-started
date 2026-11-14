---
name: build-ui
description: Add or change the user interface - widgets, layout, a new scene or a transition between scenes - with guillaume. Use when asked for a button, text, card, list, dialog, page, screen or any other visible element.
---

# Build UI with guillaume

UI code lives in `src/scenes/`. A scene builds its widgets in `onEnter()` and
removes them in `onExit()`. `src/scenes/HomeScene.cpp` is the model to copy.

## Find the widget's API first

Do not write a builder call from memory: signatures differ between widgets and
between guillaume versions. Read them from the pinned sources:

- Headers: `build/_deps/guillaume-src/headers/guillaume/entities/<widget>.hpp`
  (with the `siblings` preset: `../guillaume/headers/guillaume/entities/`)
- A working call for every widget:
  `build/_deps/guillaume-src/examples/evan/main.cpp` (the widget gallery).
  Grep it for the widget name and copy the pattern.

When `build/_deps/` does not exist yet, run `cmake --preset default` first.
The `library-scout` agent does this lookup and returns only the signatures.

## Rules that the sample relies on

- One root `View` per scene, created with the scene's entity registry and a
  `nullptr` parent. guillaume places root entities in front of the camera.
- A child is built with a builder that takes the parent as its registry
  (`Builder(getComponentRegistry(), *parent)`) and the parent as first
  argument of `make...()`.
- A container shows its children only after `setEntities({...})`.
- Colours come from the theme: `guillaume::getActiveColor(SchemeColorRole::...)`.
  Text styles come from `theme::TypeRole`.
- `onExit()` must clear what `onEnter()` built
  (`getEntityRegistry().clearEntities()` and reset the stored pointers), and
  clear any callback it put in `app::context()`.
- A scene has no per-frame hook. To change a widget every frame, store a
  callback in `app::Context` (see `showStatus`) and call it from the loop in
  `src/main.cpp`.
- Scenes include no pybind11 and no evan header. A click handler records a
  fact in `app::context()`; the host loop and the script react to it.

## Add a scene

1. Copy `src/scenes/HomeScene.hpp` and `.cpp` to the new class name.
2. Add the `.cpp` to `add_executable()` in `CMakeLists.txt`.
3. Add the class to `guillaume::Application<HomeScene, ...>` in
   `src/main.cpp`. The first class is the start-up scene.
4. Switch scenes from a click handler: `goToScene<OtherScene>()`.

## Assets

Images, models and textures go under `assets/` and are loaded by their path
relative to it (`"models/chair.obj"`). They are copied on every build.

## Finish

Follow the `verify-app` skill. Check in the run log that the new texts are
drawn. Every file starts with the MIT license header and members carry
Doxygen comments, as in `HomeScene`.
