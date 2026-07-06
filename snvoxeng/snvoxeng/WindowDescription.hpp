#pragma once

#include <cstdint>

namespace sn::voxeng
{
	struct WindowDescription_t
	{
        uint32_t width{ 1280u };
        uint32_t height{ 720u };

        enum class platform_type {
            headless,
            win32,
            linux_wayland,
            linux_x11,
            macos_metal,
        } platform{ platform_type::headless };

        // HINSTANCE / wl_display* / ...
        void* handle_param1{ nullptr };
        // HWND / wl_surface* / ...
        void* handle_param2{ nullptr };
    };
}
