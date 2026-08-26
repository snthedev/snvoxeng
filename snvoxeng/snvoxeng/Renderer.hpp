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
        class Queue;
    }

	class SNVOXENG_API Renderer
	{
        struct FrameContext
        {
            uint32_t imageIndex;
            size_t frameIndex;
            vk::CommandBuffer computeCmd;
            vk::CommandBuffer graphicsCmd;
        };

        struct data_t;
        data_t* m_pData;

	public:
        // Queues carry their family indices internally: callers never deal
        // with queue families or synchronization - all canvas ownership
        // transfers, layout transitions and submission ordering live here.
        Renderer(
            vk::Device& device,
            vk::SurfaceKHR& surfaceKHR,
            vk::Queue graphicsQueue,
            vk::Queue computeQueue
            );
        ~Renderer() noexcept;

        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

        bool recreateSwapchainKHR();

        // std::nullopt means that the swapchain needs to be recreated.
        // On success the returned compute command buffer is already begun and
        // primed with the canvas acquire barrier: record the compute workload
        // right away, then hand the context back to submitFrame().
        std::optional<FrameContext> beginFrame();
        // Records the canvas release/copy/present section, submits both
        // command buffers and presents. Must follow a successful beginFrame()
        // with the SAME FrameContext. false means that the swapchain needs to
        // be recreated.
        bool submitFrame(const FrameContext& frame);

        // Descriptor slot types kept in sync with the canvas image view.
        // When shader reflection lands, the high-level layer will resolve the
        // canvas binding automatically and feed it through this same API.
        void attachCanvasImage(VkDescriptorSet set, uint32_t binding, uint32_t arrayElement = 0);

        // Fired after the canvas (and swapchain) have been recreated with a
        // new extent. Plain function pointer + userdata: stable ABI boundary.
        using CanvasResizedFn = void (*)(void* userdata, VkExtent2D newExtent);
        void setCanvasResizedCallback(CanvasResizedFn fn, void* userdata);

        const size_t getMaxFramesInFlight() const noexcept;

        const vk::SwapchainKHR& getSwapchain() const noexcept;
        const vk::Image& getCanvasImage() const noexcept;
        const vk::ImageView& getCanvasImageView() const noexcept;

        vk::SwapchainKHR& getSwapchain() noexcept;
        vk::Image& getCanvasImage() noexcept;
        vk::ImageView& getCanvasImageView() noexcept;
	};
}
