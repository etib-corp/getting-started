---
name: verify-app
description: Build the desktop application, run its tests and run it for a fixed number of frames to confirm a change works. Use after any change to src/, platforms/desktop/, scripts/, cmake/ or CMakeLists.txt, and whenever asked to build, run, test or check the app.
---

# Verify the application

Run the steps in order and stop at the first failure.

## 1. Build

```sh
cmake --preset default                       # only when build/ is missing or CMake files changed
cmake --build --preset default --parallel 2>&1 | grep -E "error|Error [0-9]|Built target getting_started"
```

The first configure downloads the libraries and the first build compiles them
(several minutes). Use the `siblings` preset instead of `default`, everywhere
below, when the user works on a library checked out next to this repository.

The libraries print many warnings while they compile. Only errors, and
warnings in files under `src/` or `tests/`, are about this repository.

## 2. Tests without a window

```sh
ctest --preset default
```

These start Python but no window, so they run on any machine.

## 3. Run the application

```sh
GETTING_STARTED_MAX_FRAMES=120 ./build/bin/getting_started > build/run.log 2>&1; echo "exit=$?"
grep -a "^app.py\|\[getting_started\]" build/run.log
```

This opens a window for a few seconds, so it needs a display and a Vulkan
driver. Skip it and say so when the machine has neither.

A good run has `exit=0`, the line `app.py: stopped after 120 frames`, and no
`[getting_started]` line (those are script errors and rejected settings).

The log has tens of thousands of lines because debug builds of the libraries
log every frame. Never read it whole: grep it. Use `grep -a`, the log has
colour codes.

To check that a text is drawn:
`grep -a "Rendering text" build/run.log | grep -a "<the text>" | tail -1`

## Known noise

These come from the libraries and are not caused by a change here (guillaume's
own widget gallery prints them too):

- `[VulkanValidation] [Error] ... expects VkImage ... to be in layout`
- `[VulkanValidation] [Warning] ... Vertex attribute at location N not consumed`
- `[evan::GPUMesh] [Warning] Vertex buffer size is zero`

## 4. Before handing back

```sh
cmake --preset default -DAPP_DEV_MODE=ON && cmake --build --preset default --parallel   # what CI builds
./tools/run-clang-format.sh        # needs clang-format 21.1.8
```

Set `APP_DEV_MODE` back to `OFF` afterwards if it was off before.

A change under `src/` or `cmake/` also affects the Android build, which this
skill does not build: follow the `android-headset` skill, or say that the
Android build was not checked.

Report what ran and what was skipped. A click or another interaction cannot be
checked this way: say that it was not exercised.
