# Android application (Meta headsets)

The Gradle project that packages the application as an APK for Meta Quest
headsets. It has no Java or Kotlin code: the activity is
`android.app.NativeActivity` and everything runs in the native library built
from `app/cpp`.

```text
settings.gradle.kts, build.gradle.kts, gradle/   the Gradle project
app/build.gradle.kts      SDK and NDK versions, application id, assets
app/AndroidManifest.xml   native activity, VR category
app/cpp/CMakeLists.txt    native build: pinned libraries, src/, engine assets,
                          Python runtime
app/cpp/main.cpp          the Android host: lifecycle, Python hooks, frame loop
app/cpp/AssetTree.*       copies the Python files from the APK to the device
```

What it shares with the desktop build, so there is one copy of each:

- `../../src` — the scenes, the shared context and the frame helpers;
- `../../cmake/Dependencies.cmake` — the pinned revisions of evan,
  guillaume, maïs and utility;
- `../../scripts` — the Python hooks, packaged as APK assets;
- `../../assets` — the application assets, packaged as APK assets;
- the engine shaders and fonts, copied from the pinned libraries into
  `app/build/generated/engine-assets` by the native build.

## Requirements

- JDK 17 or newer
- The Android SDK: set `ANDROID_HOME`, or create `local.properties` here with
  `sdk.dir=/path/to/Android/sdk` (Android Studio does it for you)
- The SDK's **CMake 3.31.5** package. Gradle does not download it: install
  it in Android Studio (Settings > Languages & Frameworks > Android SDK >
  SDK Tools, tick "Show Package Details", CMake 3.31.5) or with
  `sdkmanager "cmake;3.31.5"`
- Accepted SDK licences, so Gradle can download the NDK (27.3.13750724) and
  the Android 36 platform by itself
- A headset in developer mode, and `adb`, to install and run

## Build, install, run

```sh
./gradlew assembleDebug
adb install -r app/build/outputs/apk/debug/app-debug.apk
adb shell am start -n com.etib.getting_started/android.app.NativeActivity
```

The first build fetches the libraries and compiles them for `arm64-v8a`,
which takes several minutes.

`./gradlew assembleRelease` builds an optimized APK signed with the debug
key, so it installs on a development headset. Set up your own signing config
in `app/build.gradle.kts` before distributing a build.

## Python

The application embeds the official Android build of Python, downloaded
from python.org by the native build and checked against its SHA-256
(`../../cmake/AndroidPython.cmake`: version, checksums and what is left out
of the standard library). Nothing has to be installed, on the computer or on
the headset.

What goes in the APK:

```text
lib/arm64-v8a/libpython3.15.so        the interpreter, linked by the application
lib/arm64-v8a/lib{crypto,ssl,sqlite3}_python.so   used by ssl, hashlib, sqlite3
assets/python/lib/python3.15/         the standard library, without its tests
assets/scripts/                       ../../scripts
```

Python imports from files, and APK assets are not files. Each of the two
asset trees has a `manifest.txt` (a stamp, then the list of its files). At
start-up the host copies a tree to the application's private directory
(`files/python`, `files/scripts`) when its stamp changed, so only the first
start after an installation or an update pays for the copy, then starts
Python with `PYTHONHOME` set to `files/python`.

- Edit `../../scripts/app.py`, rebuild and reinstall: the new script is
  copied at the next start.
- The script must also run on desktop, with an older Python: see the notes
  at the top of `scripts/app.py`.
- Only the standard library is available. A pure-Python package can be added
  by copying it into `../../scripts`.
- When Python or the script cannot start, the reason is logged and the
  application runs without the hooks.

To look at what was copied on a headset with a debug build:

```sh
adb shell "run-as com.etib.getting_started ls files/scripts files/python/lib"
```

## Logs

Nothing is printed on a terminal. The libraries log to logcat, with the
logger name as tag:

```sh
adb logcat --pid="$(adb shell pidof -s com.etib.getting_started)"
```

| Tag | Lines |
| --- | --- |
| `getting_started` | the host: script errors with their traceback, fatal errors |
| `python.stdout`, `python.stderr` | what the script prints |
| `evan::...`, `guillaume::...`, `utility::...` | the libraries |

## Passthrough and hand tracking

The application starts in VR. evan can enable the AR extensions of the
runtime: call `platform->setOptionArExtensionsEnabled(true)` in
`app/cpp/main.cpp` before the engine is created, and declare what you use in
`app/AndroidManifest.xml`:

```xml
<uses-feature android:name="com.oculus.feature.PASSTHROUGH" android:required="true" />
<uses-feature android:name="oculus.software.handtracking" android:required="false" />
<uses-permission android:name="com.oculus.permission.HAND_TRACKING" />
```

## Renaming

The same name appears in four places: `APP_NAME` in `app/cpp/CMakeLists.txt`
(the library is `<APP_NAME>_android`), `android.app.lib_name` and the label
in `app/AndroidManifest.xml`, and `namespace` / `applicationId` in
`app/build.gradle.kts`.
