---
name: android-headset
description: Build, install, run and debug the Android application for Meta headsets (Quest), or change anything under platforms/android/. Use when asked about the headset, VR, OpenXR, the APK, Gradle, adb or logcat, and after a change to src/ or cmake/ that must also work on the headset.
---

# Android application for Meta headsets

`platforms/android/` is a Gradle project with a fully native activity. It
reuses `src/` (scenes, context, frame helpers) and `cmake/Dependencies.cmake`
(the pinned libraries), with evan's OpenXR backend. There is no Python in
this build: `mais` is left out and the host writes the frame context itself.

| File | Role |
| --- | --- |
| `app/cpp/main.cpp` | The Android host: lifecycle, OpenXR platform, frame loop. |
| `app/cpp/CMakeLists.txt` | Native build; copies the engine shaders and fonts into the APK assets. |
| `app/build.gradle.kts` | SDK and NDK versions, application id, assets directories. |
| `app/AndroidManifest.xml` | `NativeActivity`, VR category, headset features. |

## Requirements

A JDK 17 or newer and the Android SDK, with `ANDROID_HOME` set or
`platforms/android/local.properties` containing `sdk.dir=<path>`. Gradle
downloads the NDK and the Android platform when the licences are accepted,
but not CMake: the SDK package `cmake;3.31.5` must be installed (the version
is set in `app/build.gradle.kts`). Check before building:

```sh
java -version; echo "$ANDROID_HOME"; ls "$ANDROID_HOME/cmake"
```

With Android Studio installed and no JDK on the PATH, use its bundled one:
`export JAVA_HOME="/Applications/Android Studio.app/Contents/jbr/Contents/Home"`
(macOS) and `ANDROID_HOME="$HOME/Library/Android/sdk"`.

`[CXX1300] CMake '3.31.5' was not found` means the SDK package is missing.

If they are missing, do not install them yourself: say what is missing and
that CI builds the APK (job "Android" of `.github/workflows/build.yml`).

## Build

```sh
cd platforms/android
./gradlew assembleDebug 2>&1 | tail -40
```

The APK is `app/build/outputs/apk/debug/app-debug.apk`. The first build
fetches and compiles the libraries for `arm64-v8a` (several minutes).

When a build fails inside CMake, the real error is in
`app/.cxx/Debug/<hash>/arm64-v8a/` (`CMakeFiles/CMakeConfigureLog.yaml`, or
the Ninja output printed by Gradle with `--info`).

## Install and run

The headset must be in developer mode and connected over USB, with the
computer authorised on the headset.

```sh
adb devices
adb install -r app/build/outputs/apk/debug/app-debug.apk
adb shell am start -n com.etib.getting_started/android.app.NativeActivity
adb logcat --pid="$(adb shell pidof -s com.etib.getting_started)"
```

Nothing is printed on a terminal on Android: the libraries log to logcat,
with the logger name as tag (`evan::...`, `guillaume::...`, `utility::...`);
the host's own fatal errors use the tag `getting_started`. Debug builds log
every frame: filter with `grep`, and use `adb logcat -d` to take a snapshot
instead of following the log. A
crash at start-up is usually a missing asset (check `unzip -l` of the APK for
`assets/shaders`, `assets/fonts`) or a missing OpenXR feature in the manifest.

## What an agent can and cannot verify

- With the SDK installed: that the APK builds and what it contains.
- With a headset connected: installation, start-up and the log.
- Never: what the user sees and whether pointing or pinching works. Ask the
  user to look and say what was not checked.

## Changing things

- **Shared code**: anything in `src/CMakeLists.txt` is compiled for Android.
  No desktop-only API, no mais, no pybind11 there.
- **Per-frame behaviour**: the loop in `app/cpp/main.cpp` mirrors the desktop
  one. A value `scripts/app.py` writes on desktop needs a C++ fallback in
  `src/Frame.cpp` or it is missing on the headset.
- **Passthrough and hand tracking**: call
  `platform->setOptionArExtensionsEnabled(true)` in `main.cpp` and declare the
  features in the manifest (`com.oculus.feature.PASSTHROUGH`,
  `oculus.software.handtracking` and the `com.oculus.permission.HAND_TRACKING`
  permission). evan's `docs/AR_ROADMAP.md` lists what is supported.
- **Assets**: `assets/` at the repository root is packaged as is. Engine
  shaders and fonts are generated into `app/build/generated/engine-assets`;
  do not commit copies.
- **Application id, name, versions**: `app/build.gradle.kts` and the label in
  the manifest. The library name in the manifest (`android.app.lib_name`)
  must match the CMake project name.
- **Release builds** are signed with the debug key so they install. A
  distributed build needs a real signing config; never commit a keystore.

Problems in evan's OpenXR backend or in a library go to that library's
repository: see the `report-issue` skill.
