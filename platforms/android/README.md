# Android application (Meta headsets)

The Gradle project that packages the application as an APK for Meta Quest
headsets. It has no Java or Kotlin code: the activity is
`android.app.NativeActivity` and everything runs in the native library built
from `app/cpp`.

```text
settings.gradle.kts, build.gradle.kts, gradle/   the Gradle project
app/build.gradle.kts      SDK and NDK versions, application id, assets
app/AndroidManifest.xml   native activity, VR category
app/cpp/CMakeLists.txt    native build: pinned libraries, src/, engine assets
app/cpp/main.cpp          the Android host: lifecycle and frame loop
```

What it shares with the desktop build, so there is one copy of each:

- `../../src` — the scenes, the shared context and the frame helpers;
- `../../cmake/Dependencies.cmake` — the pinned revisions of evan, guillaume
  and utility (maïs is left out: there is no Python on Android yet);
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

## Logs

Nothing is printed on a terminal. The libraries log to logcat, with the
logger name as tag:

```sh
adb logcat --pid="$(adb shell pidof -s com.etib.getting_started)"
```

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
