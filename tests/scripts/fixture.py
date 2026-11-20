"""Fixture for tests/ScriptingTest.cpp: one use of each host binding."""

import host


def configure(settings):
    settings.window_title = "fixture"
    settings.width = 640
    settings.height = 480
    settings.target_fps = 30
    settings.msaa_samples = 4
    settings.camera_controls = False


def on_update(context):
    context.status = f"frame {context.index}, {context.clicks} clicks"


def on_shutdown(context):
    host.request_quit()


def write_read_only_field(context):
    context.clicks = 99
