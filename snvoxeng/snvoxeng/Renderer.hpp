#pragma once

#include <snvoxeng/snvoxeng/dll-defines.hpp>

#include <snvoxeng/snvoxeng/vk/CommandBuffer.hpp>

#include <optional>

namespace sn::voxeng
{
    namespace vk
    {
        class Device;
        class SurfaceKHR;
        class SwapchainKHR;
        class Image;
        class ImageView;

        class CommandBuffer;
        //class DeviceMemory;
    }

	class SNVOXENG_API Renderer
	{
        struct FrameContext
        {
            uint32_t imageIndex;
            vk::CommandBuffer computeCmd;
            vk::CommandBuffer graphicsCmd;
        };

        struct data_t;
        data_t* m_pData;

	public:
        Renderer(
            vk::Device& device,
            vk::SurfaceKHR& surfaceKHR,
            uint32_t graphicsQueueFamilyIndex,
            uint32_t computeQueueFamilyIndex,
            size_t graphicsQueueIndex,
            size_t computeQueueIndex
            );
        ~Renderer() noexcept;

        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

        bool recreateSwapchainKHR();

        // std::nullopt means that the swapchain needs to be recreated.
        std::optional<FrameContext> beginFrame();
        // false means that the swapchain needs to be recreated.
        bool endFrame(uint32_t imageIndex);

        const vk::SwapchainKHR& getSwapchain() const noexcept;
        const vk::Image& getCanvasImage() const noexcept;
        const vk::ImageView& getCanvasImageView() const noexcept;

        vk::SwapchainKHR& getSwapchain() noexcept;
        vk::Image& getCanvasImage() noexcept;
        vk::ImageView& getCanvasImageView() noexcept;
	};
}
