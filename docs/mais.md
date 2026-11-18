# maïs

[maïs](https://github.com/etib-corp/mais) is the scripting runtime of the
application. It owns the embedded Python interpreter, loads the application
script, calls its hooks and exposes a small, host-defined set of C++ values
and commands to it through pybind11.

maïs is the layer that makes the application scriptable without teaching the
engine or the UI about Python.

## What it does

- Starts and stops an embedded Python interpreter, and manages the module
  search paths.
- Loads the script modules named by the host and calls the functions the host
  chooses, with arguments the host builds.
- Registers host bindings before Python starts, so a script can
  `import host` and reach selected C++ objects.
- Reports script failures as `mais::Error` with the Python traceback, and
  applies a `FailurePolicy` to repeated failures.
- Measures how long each hook call took, in the returned `CallReport`.

## What it does not do

- It does not create an engine, a window or a UI, and does not own the frame
  loop. The host calls the hooks where it wants them.
- It knows nothing about evan or guillaume.
- It does not package the application's scripts; `cmake/Runtime.cmake` copies
  `scripts/` next to the executable.
- It does not expose widgets or the engine. Only what the host binds is
  reachable from Python.

## Key concepts

| Term | Meaning |
| --- | --- |
| **Runtime** (`mais::ScriptRuntime`) | Owns the interpreter and the loaded modules. |
| **Host** | This application: creates the runtime, binds its values, calls the hooks. |
| **Binding** | A selected C++ object or function exposed to Python with pybind11. |
| **Hook** | A script function the host calls, such as `configure` or `on_update`. Hooks are host-defined names, not enforced by maïs. |
| **Failure policy** | What to do when a hook fails: `Continue`, `ReportOnce` or `Stop`. |

## How this application uses it

Start-up, in `src/main.cpp`:

1. `app::context()` is created first, then `mais::ScriptRuntime runtime`.
   The context outlives the runtime: Python keeps references to objects it
   contains.
2. `startScripting()` calls `app::scripting::registerBindings()` **before**
   `runtime.initialize()`, then `addSearchPath("scripts")` and
   `loadModule("app")`.
3. `configure()` calls the `configure` hook of the `app` module with
   `FailurePolicy::Stop`, then validates the proposal with
   `app::scripting::validate()`. An invalid proposal is reported and the
   defaults are used.
4. After the window is up, `on_start` is called with `FailurePolicy::Stop`.
5. Every frame, `on_update` is called with `FailurePolicy::ReportOnce`: a
   broken hook prints its traceback once and the window stays open.
6. After the last frame, `on_shutdown` is called with
   `FailurePolicy::Continue`, so a failing hook never prevents the exit. The
   window and the scenes are destroyed when the frame loop returns, and
   `runtime.shutdown()` runs after that.

The script hooks are all optional and live in `scripts/app.py`:

| Hook | When | What it receives |
| --- | --- | --- |
| `configure(settings)` | Before the window exists. | The settings proposal, writable. |
| `on_start(context)` | Once, window open and first scene entered. | The frame context; only `status` is writable. |
| `on_update(context)` | Every frame, before guillaume draws. | The frame context; only `status` is writable. |
| `on_shutdown(context)` | Once, after the last frame. | The frame context; only `status` is writable. |

`src/scripting/` is the adapter and the only code that includes pybind11:

- `Scripting.hpp` declares the plain structures Python sees (`Settings`,
  `FrameContext`, `Services`) and includes no pybind11 header, so the rest of
  the application does not depend on it.
- `Scripting.cpp` binds the `host` module: the `Settings` and `FrameContext`
  classes, and the `request_quit()` command. It wraps host objects with
  `mais::ScriptArgument::native()` and a borrowed reference, so the host keeps
  ownership and Python only borrows.
- A command is a request, not an action: `host.request_quit()` sets
  `Services::quitRequested`, and the host loop reads it at the point where
  quitting is safe.

In the sample, the button increments `clicks`, the script reads it in
`on_update` and writes `status`, and the host shows that text in the window.
That is the full round trip: widget, host, Python, host, widget.

## Where the code lives

| File | Role |
| --- | --- |
| `src/main.cpp` | Starts and stops the runtime, calls the hooks at fixed points. |
| `src/scripting/Scripting.hpp` | What Python can see, declared without pybind11. |
| `src/scripting/Scripting.cpp` | The only pybind11 code: the `host` module and the arguments. |
| `scripts/app.py` | The hooks the host calls. |
| `tests/ScriptingTest.cpp` | Checks each binding with `tests/scripts/fixture.py`. |

## Version

The application pins a maïs commit on its `main` branch in
`cmake/Dependencies.cmake`, because maïs has no release tag yet and the
application needs `ScriptArgument::native()`. To move to a newer revision,
change it there and rebuild; see "Update a library" in the
[README](../README.md#update-a-library).

## Learn more

- [maïs README](https://github.com/etib-corp/mais#readme)
- [Host integration guide](https://github.com/etib-corp/mais/blob/main/docs/HOST_INTEGRATION.md)
- [Architecture](https://github.com/etib-corp/mais/blob/main/docs/ARCHITECTURE.md)
- [Getting Started](https://github.com/etib-corp/mais/blob/main/docs/GETTING_STARTED.md)
- [Error Handling](https://github.com/etib-corp/mais/blob/main/docs/ERROR_HANDLING.md)
- [Threading](https://github.com/etib-corp/mais/blob/main/docs/THREADING.md)
