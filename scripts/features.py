"""Functions called by the scripting tour.

`./build/bin/getting_started --scripting-tour` runs
platforms/desktop/ScriptingTour.cpp, which calls the functions below one by
one to show what the maïs runtime does: typed return values, arguments,
introspection, error reports and failure policies. Nothing here is used by
the application itself, whose hooks are in app.py.

Like app.py, this file must stay valid for Python 3.9 to 3.15.
"""

import time

import host


def answer():
    """An integer, read back with call<std::int64_t>()."""
    return 42


def ratio():
    """A float, read back with call<double>()."""
    return 0.5


def is_ready():
    """A boolean, read back with call<bool>()."""
    return True


def name():
    """A string, read back with call<std::string>()."""
    return "features.py"


def nothing():
    """No value: a typed call succeeds and hasValue() is false."""
    return None


def describe(integer, number, flag, text):
    """Report the Python type of each argument the host sent."""
    kinds = [type(value).__name__ for value in (integer, number, flag, text)]
    return f"{' '.join(kinds)}: {integer} {number} {flag} {text}"


def read_context(context):
    """Use a native argument: read two fields, write the writable one."""
    context.status = f"frame {context.index}, {context.clicks} click(s)"


def write_read_only(context):
    """Try to write a field the host bound as read-only: Python raises."""
    context.clicks = 99


def _check(value):
    """Private helper (leading underscore): not listed by listFunctions()."""
    if value < 0:
        raise ValueError("value must not be negative")


def fail():
    """Raise from a nested call, so the traceback has two frames here."""
    _check(-1)


def flaky(should_fail):
    """Fail on demand: a hook that breaks for a few frames, then recovers."""
    if should_fail:
        raise RuntimeError("flaky() was asked to fail")


def wait(milliseconds):
    """Take a known time, to compare with CallReport::duration."""
    time.sleep(milliseconds / 1000)


def quit():
    """Send a command to the host: it only sets a flag there."""
    host.request_quit()
