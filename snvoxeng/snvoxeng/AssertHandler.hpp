#pragma once

#include <snvoxeng/snvoxeng/dll-defines.hpp>

namespace sn::voxeng::details
{
	// Replaces the default snassert break action (__debugbreak) with a
	// fail-fast process termination (__fastfail on Windows): the assertion
	// report has already been written to stderr at that point, and breaking
	// into a debugger is useless in headless runs / CI / windowed apps.
	//
	// Installed automatically when the engine DLL is loaded. Keep this around
	// in case someone resets the hook via setBreakFn(nullptr).
	SNVOXENG_API void installFatalAssertHandler() noexcept;
}