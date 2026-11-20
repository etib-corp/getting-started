/*
 Copyright (c) 2026 ETIB Corporation

 Permission is hereby granted, free of charge, to any person obtaining a copy of
 this software and associated documentation files (the "Software"), to deal in
 the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do
 so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

#include "scripting/Scripting.hpp"

#include <pybind11/embed.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

namespace app::scripting
{
	namespace
	{
		/**
		 * @brief Hand a host object to Python without copying it.
		 *
		 * mais takes ownership of one reference to the returned Python
		 * object, so the wrapper is released instead of destroyed here. The
		 * native object stays owned by the host.
		 * @tparam Type The bound C++ type.
		 * @param value Address of the host object.
		 * @return The Python wrapper, as a PyObject pointer.
		 */
		template<typename Type> void *borrowedReference(const void *value)
		{
			return py::cast(static_cast<const Type *>(value),
							py::return_value_policy::reference)
				.release()
				.ptr();
		}
	}	 // namespace

	mais::Error registerBindings(mais::ScriptRuntime &runtime,
								 Services &services)
	{
		return runtime.registerModule(
			"host", [&services](mais::PythonModule &module) {
				py::module_ &host = module.as<py::module_>();

				// A class must be bound before a hook can receive an
				// instance of it.
				py::class_<Settings>(host, "Settings")
					.def_readwrite("window_title", &Settings::windowTitle)
					.def_readwrite("width", &Settings::width)
					.def_readwrite("height", &Settings::height)
					.def_readwrite("target_fps", &Settings::targetFps)
					.def_readwrite("msaa_samples", &Settings::msaaSamples)
					.def_readwrite("camera_controls",
								   &Settings::cameraControls);

				py::class_<FrameContext>(host, "FrameContext")
					.def_readonly("index", &FrameContext::index)
					.def_readonly("delta_seconds", &FrameContext::deltaSeconds)
					.def_readonly("elapsed_seconds",
								  &FrameContext::elapsedSeconds)
					.def_readonly("clicks", &FrameContext::clicks)
					.def_readwrite("status", &FrameContext::status);

				// A command is a request: the host reads the flag at the
				// point of its loop where quitting is safe.
				host.def(
					"request_quit",
					[&services]() {
						services.quitRequested = true;
					},
					"Ask the application to close after the current frame.");
			});
	}

	mais::ScriptArgument settingsArgument(Settings &settings)
	{
		return mais::ScriptArgument::native(&settings,
											&borrowedReference<Settings>);
	}

	mais::ScriptArgument frameArgument(FrameContext &context)
	{
		return mais::ScriptArgument::native(&context,
											&borrowedReference<FrameContext>);
	}

}	 // namespace app::scripting
