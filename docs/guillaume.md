# guillaume

[guillaume](https://github.com/etib-corp/guillaume) is the UI framework of the
application. It provides the application shell, an ECS-based composition
model, scenes, widgets, layout, event dispatching, theming and storage.

It sits on top of an engine: anything that opens a window or draws is the
engine's job. In this repository that engine is evan.

## What it does

- Owns the frame loop through `guillaume::Application`, the event bus, the
  systems and the scenes; it drives the engine's poll, draw and present.
- Composes the UI with an ECS: entities carry components, systems run in the
  Event, Measure, Layout and Render phases.
- Builds and lays out widgets such as views, text, buttons, images and lists
  through the builders under `guillaume/entities/`.
- Manages scenes and their transitions, with `onEnter()` and `onExit()`
  called at the right time.
- Routes typed events to subscribers.
- Applies a theme: colour roles and type roles rather than hard-coded values.
- Stores values in `LocalStorage` (SQLite-backed, persistent) or
  `SessionStorage` (in memory).

## What it does not do

- It does not create the window or issue Vulkan calls; it calls the
  `utility::Engine` it was given, which is evan's engine here.
- It does not run Python. The script reaches the UI only through the host:
  it writes a value in `app::Context`, and the host updates the scene.
- It does not package the application's scripts or assets; the build does.

## Key concepts

| Term | Meaning |
| --- | --- |
| **Application** | `guillaume::Application<Scenes...>`: owns the engine, scenes, systems and event bus. The constructor enters the first scene; `routine()` runs one frame. |
| **Scene** | A self-contained UI state with its own entities and components. Scenes are created by the application, not by `main()`. |
| **ECS** | Entities, components and systems. Every system belongs to one of four phases: Event, Measure, Layout, Render. |
| **Widget** | A ready-made entity type (`View`, `Text`, `Button`, …) with a builder under `guillaume/entities/`. |
| **Event bus** | Routes typed events to the listeners and systems subscribed to them. |
| **Theme** | Named colour and type roles (`SchemeColorRole`, `TypeRole`) resolved against the active theme. |

## How this application uses it

- `src/Application.hpp` names `app::Application`, a
  `guillaume::Application<...>` whose template lists every scene of the
  application; the first is shown at start-up. Each host constructs it with
  `application(resources, std::move(engine))`. The engine passed in is
  evan's.
- The host loop calls `application.shouldQuit()` and `application.routine()`
  once per iteration. `routine()` polls input, publishes the events, runs the
  four phases on the active scene, resets the changed flags, performs a
  pending scene switch, then presents the frame.
- `HomeScene::onEnter()` in `src/scenes/HomeScene.cpp` builds a root `View`
  as a vertical column: `Surface` colour, `Start` cross-axis alignment,
  20 px spacing, 32 px padding, 28 px border radius. Its children are a
  `Text` title, a `Text` subtitle, a `Text` status line and a `Button`.
- The button's click handler only increments
  `context().scripting.frame.clicks`. The script reads it on its next
  `on_update()` and writes `context.status`; the host calls the callback
  registered in `HomeScene::onEnter()` (`context().showStatus`) to update the
  status line.
- Two more buttons of `HomeScene` call `goToScene<...>()` to open the
  widget gallery: eleven scenes under `src/scenes/gallery/`, one file per
  family of widgets, plus the theme (colour schemes, tokens, accessibility)
  and the two storages. The table is in the
  [README](../README.md#guillaume-the-widget-gallery).
- `HomeScene::onExit()` clears the callback, the entity registry and the
  cached widget pointers, which is the pattern to copy when adding a scene.
- Scenes are created by guillaume with the arguments of the `Scene`
  constructor, so `main()` cannot pass them state directly. That is why both
  sides reach the shared `app::Context` through `app::context()`.

## Where the code lives

| File | Role |
| --- | --- |
| `src/scenes/HomeScene.hpp` | The start-up scene, its widgets and its lifecycle overrides. |
| `src/scenes/HomeScene.cpp` | Builds the column, the texts and the button; registers `showStatus`. |
| `src/scenes/gallery/GalleryPages.hpp` | The gallery pages and the table of what each one shows. |
| `src/scenes/gallery/Page.cpp` | The column, rows and navigation every page shares. |
| `src/scenes/gallery/*Page.cpp` | One page each: the code that creates the widgets. |
| `src/Application.hpp` | Lists the scenes in `guillaume::Application`. |
| `platforms/desktop/main.cpp`, `platforms/android/app/cpp/main.cpp` | Construct the application and run `routine()`. |
| `src/AppContext.hpp` | The state a scene and the host share. |

## Version

The application pins guillaume 2026.6.1 in `cmake/Dependencies.cmake`. That
revision has no tag yet, so the file carries the commit hash. To move to a
newer revision, change it there and rebuild; see "Update a library" in the
[README](../README.md#update-a-library).

## Learn more

- [guillaume README](https://github.com/etib-corp/guillaume#readme)
- [How Guillaume Works](https://github.com/etib-corp/guillaume/blob/main/docs/HOW_GUILLAUME_WORKS.md)
- [Architecture](https://github.com/etib-corp/guillaume/blob/main/docs/ARCHITECTURE.md)
- [Getting Started](https://github.com/etib-corp/guillaume/blob/main/docs/GETTING_STARTED.md)
- [Theming](https://github.com/etib-corp/guillaume/blob/main/docs/THEMING.md)
- [Widget gallery](https://github.com/etib-corp/guillaume/tree/main/examples/evan)
