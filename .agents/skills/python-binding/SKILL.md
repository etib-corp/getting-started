---
name: python-binding
description: Expose a value, a setting or a command of the application to the Python scripts, add or change a script hook, or debug a script error. Use for anything that touches src/scripting/, scripts/app.py or the mais runtime calls in platforms/desktop/main.cpp.
---

# Python bindings and hooks

Three places always change together:

| Place | Role |
| --- | --- |
| `src/scripting/Services.hpp` | The plain structures Python sees (`Settings`, `FrameContext`, `Services`). No mais or pybind11 include: Android uses it too. |
| `src/scripting/Scripting.cpp` | The pybind11 bindings of the `host` module. The only file that includes pybind11. |
| `platforms/desktop/main.cpp` | Fills the structures before a hook, calls the hook, acts on the result. |

## Expose a value to scripts

1. Add the field to `FrameContext` (per-frame) or `Settings` (start-up) in
   `Services.hpp`, with a Doxygen comment saying who writes it.
2. Bind it in `registerBindings()`: `def_readonly` when the host writes it,
   `def_readwrite` when the script writes it. Python names are `snake_case`.
3. In `platforms/desktop/main.cpp`, write it before the hook is called, or
   read it after.
4. Use it in `scripts/app.py`.
5. Cover it in `tests/ScriptingTest.cpp` and `tests/scripts/fixture.py`.

## Add a command

A command is a request. Bind a function with `host.def(...)` that only sets a
flag in `Services`; the frame loop in `platforms/desktop/main.cpp` reads the
flag and acts.
`request_quit` is the model.

## Add a setting

Add the field to `Settings`, bind it, add its accepted range to `validate()`
and use it where the window or the engine is created. Settings are only read
at start-up: the libraries have no runtime reconfiguration.

## Add a hook

Call it from `platforms/desktop/main.cpp` with
`runtime.callOptional(policy, scriptModule, "name", { ...arguments })`. Pick the policy on purpose:

- `Stop` for start-up hooks: a failure must prevent the next step.
- `ReportOnce` inside the frame loop: one report per failure streak, the
  window stays open.
- `Continue` for shutdown: nothing may prevent the exit.

## Android

The Android build has no Python. A value the script computes on desktop has
to be computed by the Android host too, or the feature is missing on the
headset: put the fallback in `src/Frame.cpp` (see `writeDefaultStatus`) and
call it from `platforms/android/app/cpp/main.cpp`.

## Never do this

- Bind `evan::Engine`, a scene, a widget, or `mais::ScriptRuntime` itself.
- Let a hook block (sleep, network, waiting): it runs inside the frame, on the
  main thread, which is the only thread allowed to call Python.
- Hand Python an object that dies before `runtime.shutdown()`. Everything
  bound lives in `app::Context`.
- Register bindings after `runtime.initialize()`.
- Add a dependency to the `mais` target: its configure-time check fails.

The reasons are in `build/_deps/mais-src/docs/HOST_INTEGRATION.md`.

## Debug a script error

Errors are printed as `[getting_started] <step>: <message>` followed by the
Python traceback. `scripting.app_script` reproduces most of them without a
window: `ctest --preset default -R scripting`.

mais calls are checked with `if (error)`: an `Error` or a `CallReport`
converts to `true` when the call failed.

## Finish

Follow the `verify-app` skill: both scripting tests must pass.
