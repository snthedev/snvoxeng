#include <snvoxeng/snvoxeng/AssertHandler.hpp>

#include <snassert/snassert.hpp>

#if defined(_WIN32)
#include <Windows.h>
#include <intrin.h>
#else
#include <cstdlib>
#endif

namespace sn::voxeng::details
{
	namespace
	{
		// snassert break hook replacement: the default __debugbreak() only
		// helps when a debugger is attached, and is silently useless in
		// headless runs, CI and windowed builds. Fail fast instead - by this
		// point snassert has already written the full report to stderr.
		void fatalAssertBreak() noexcept
		{
#if defined(_WIN32)
			__fastfail(FAST_FAIL_FATAL_APP_EXIT);
#else
			std::abort();
#endif
		}

		struct AutoFatalAssertHandler
		{
			AutoFatalAssertHandler() noexcept { installFatalAssertHandler(); }
		};

		const AutoFatalAssertHandler g_autoFatalAssertHandler;
	}

	void installFatalAssertHandler() noexcept
	{
		sn::assert::details::debugBreaker::setBreakFn(&fatalAssertBreak);
	}
}