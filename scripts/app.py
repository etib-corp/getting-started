"""Application script, run by the host through mais.

The host (platforms/desktop/main.cpp, or platforms/android/app/cpp/main.cpp
on a headset) owns the window, the engine and the frame loop. It calls the
functions below at fixed points of a session. Every function is optional:
delete the ones you do not need.

The same script runs on desktop, with the Python installed on the machine
(3.9 or newer), and on a headset, with the Python 3.15 packaged in the APK:
keep it valid for both. On a headset, `print` goes to `adb logcat` (tag
`python.stdout`) and the whole standard library is available except
`tkinter`, `idlelib`, `venv` and `ensurepip`.

What a script can see is defined in src/scripting/Services.hpp and bound in
src/scripting/Scripting.cpp. Add fields and commands there.

Keep hooks fast: they run on the main thread, inside the frame.
"""

import host


def configure(settings):
    """Propose the window settings, before the window exists.

    The host validates the proposal and falls back to its defaults when a
    value is out of range. A headset has no window: the proposal is ignored.
    """
    settings.window_title = "Getting started"
    settings.width = 1280
    settings.height = 800


def on_start(context):
    """Called once, when the window is open and the first scene is built."""
    print("app.py: started")


def on_update(context):
    """Called every frame, before the frame is drawn.

    `context` is read-only except for `status`, which the host shows in the
    window.
    """
    seconds = int(context.elapsed_seconds)
    context.status = (
        f"Python says: {context.clicks} click(s), running for {seconds} s"
    )

    # Commands are requests: the host closes the window after this frame.
    if context.clicks >= 10:
        host.request_quit()


def on_shutdown(context):
    """Called once, after the last frame."""
    print(f"app.py: stopped after {context.index} frames")
