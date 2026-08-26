#include <snvoxeng/snvoxeng/Renderer.hpp>

#include <snvoxeng/snvoxeng/vk/Device.hpp>
#include <snvoxeng/snvoxeng/vk/SurfaceKHR.hpp>
#include <snvoxeng/snvoxeng/vk/SwapchainKHR.hpp>
#include <snvoxeng/snvoxeng/vk/Image.hpp>
#include <snvoxeng/snvoxeng/vk/ImageView.hpp>

#include <snvoxeng/snvoxeng/vk/Queue.hpp>

#include <snvoxeng/snvoxeng/vk/DeviceMemory.hpp>

#include <snvoxeng/snvoxeng/vk/CommandPool.hpp>
#include <snvoxeng/snvoxeng/vk/CommandBuffersContainer.hpp>

#include <snvoxeng/snvoxeng/vk/Semaphore.hpp>
#include <snvoxeng/snvoxeng/vk/Fence.hpp>

#include <memory>
#include <snvoxeng/snvoxeng/utils/dumb_vector.hpp>

using namespace sn::voxeng;

struct Renderer::data_t
{
	static constexpr size_t MAX_FRAMES_IN_FLIGHT = 2u;
	size_t m_currentFrame{ 0 };

	vk::Device* m_pDevice;
	vk::SurfaceKHR* m_pSurfaceKHR;

	uint32_t m_graphicsQueueFamilyIndex;
	uint32_t m_computeQueueFamilyIndex;

	vk::Queue m_computeQueue;
	vk::Queue m_graphicsQueue;

	// --- Swapchain KHR ---
	vk::SwapchainKHR m_swapchainKHR;
	size_t c_swapchainKHR_image_count;

	// --- Frame Resources ---
	struct FrameResources
	{
		vk::Image canvasImage;
		vk::ImageView canvasImageView;

		FrameResources(const vk::Device& device, VkExtent3D extent)
			: canvasImage(vk::Image::Builder()
				.withDevice(device)
				.withImageType(VK_IMAGE_TYPE_2D)
				.withFormat(VK_FORMAT_R8G8B8A8_UNORM)
				.withExtent(extent)
				.withMipLevels(1u)
				.withArrayLayers(1u)
				.withSamples(VK_SAMPLE_COUNT_1_BIT)
				.withTiling(VK_IMAGE_TILING_OPTIMAL)
				.withUsage(VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
				.withSharingMode(VK_SHARING_MODE_EXCLUSIVE)
				.withInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
				.build())
			, canvasImageView(vk::ImageView::Builder()
				.withImage(canvasImage)
				.withViewType(VK_IMAGE_VIEW_TYPE_2D)
				.withSubresourceRange({
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1
					})
				.build()
			)
		{
		}
		~FrameResources() noexcept = default;

		FrameResources(const FrameResources&) = delete;
		FrameResources& operator=(const FrameResources&) = delete;
		FrameResources(FrameResources&&) = delete;
		FrameResources& operator=(FrameResources&&) = delete;
	};
	std::unique_ptr<FrameResources> m_upFrameResources;

	// --- Command Pools & Buffers ---
	vk::CommandPool m_computeCommandPool;
	vk::CommandBuffersContainer m_computeCommandBuffersContainer;
	dumb_vector<vk::CommandBuffer> m_computeCommandBuffers;

	vk::CommandPool m_graphicsCommandPool;
	vk::CommandBuffersContainer m_graphicsCommandBuffersContainer;
	dumb_vector<vk::CommandBuffer> m_graphicsCommandBuffers;

	// --- Sync ---
	dumb_vector<vk::Semaphore> m_imageAvailableSemaphores;
	dumb_vector<vk::Semaphore> m_computeFinishedSemaphores;
	dumb_vector<vk::Semaphore> m_renderFinishedSemaphores;
	dumb_vector<vk::Fence> m_inFlightFences;

	void recordCopyCanvasToSwapchain(vk::CommandBuffer& graphicsCmd, uint32_t imageIndex) const
	{
		VkImage currentSwapchainImage = m_swapchainKHR.getImages()[imageIndex].vkHandle();

		VkImageMemoryBarrier barriers[]{ {
				// Canvas Image: Acquire ownership & transition to TRANSFER_SRC
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				.srcAccessMask = 0,
				.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
				.oldLayout = VK_IMAGE_LAYOUT_GENERAL,
				.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				.srcQueueFamilyIndex = m_computeQueueFamilyIndex,
				.dstQueueFamilyIndex = m_graphicsQueueFamilyIndex,
				.image = m_upFrameResources->canvasImage.vkHandle(),
				.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
			},{
				// Swapchain Image: UNDEFINED -> TRANSFER_DST
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				.srcAccessMask = 0,
				.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
				.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.image = currentSwapchainImage,
				.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
			}
		};
		graphicsCmd.cmdPipelineBarrier(
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			0, {}, {}, barriers
		);

		// 2. Copy Image
		VkImageCopy copyRegions[]{ {
			.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
			.srcOffset = { 0, 0, 0 },
			.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
			.dstOffset = { 0, 0, 0 },
			.extent = { m_swapchainKHR.getImageExtent().width, m_swapchainKHR.getImageExtent().height, 1u }
		} };
		graphicsCmd.cmdCopyImage(
			m_upFrameResources->canvasImage.vkHandle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			currentSwapchainImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			copyRegions
		);

		// 3. Swapchain Image -> PRESENT_SRC_KHR
		VkImageMemoryBarrier presentBarriers[]{ {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
			.dstAccessMask = 0,
			.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = currentSwapchainImage,
			.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
		} };
		graphicsCmd.cmdPipelineBarrier(
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			0, {}, {}, presentBarriers
		);
	}

public:
	data_t(
		vk::Device& device,
		vk::SurfaceKHR& surfaceKHR,
		uint32_t graphicsQueueFamilyIndex,
		uint32_t computeQueueFamilyIndex,
		size_t graphicsQueueIndex,
		size_t computeQueueIndex
	)
		: m_pDevice(&device)
		, m_pSurfaceKHR(&surfaceKHR)
		, m_graphicsQueueFamilyIndex(graphicsQueueFamilyIndex)
		, m_computeQueueFamilyIndex(computeQueueFamilyIndex)
		, m_computeQueue(m_pDevice->getQueue(computeQueueIndex))
		, m_graphicsQueue(m_pDevice->getQueue(graphicsQueueIndex))
		// --- Swapchain KHR ---
		, m_swapchainKHR(vk::SwapchainKHR::Builder()
			.withDevice(*m_pDevice)
			.withSurfaceKHR(*m_pSurfaceKHR)
			.addQueueFamilyIndices(m_graphicsQueueFamilyIndex)
			.withImageUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT)
			.build())
		, c_swapchainKHR_image_count(m_swapchainKHR.getImages().size())
		// --- Frame Resources ---
		, m_upFrameResources(std::make_unique<FrameResources>(
			*m_pDevice, VkExtent3D{ m_swapchainKHR.getImageExtent().width, m_swapchainKHR.getImageExtent().height, 1u }
		))
		// --- Command Pools & Buffers ---
		, m_computeCommandPool(vk::CommandPool::Builder()
			.withDevice(*m_pDevice)
			.withQueueFamilyIndex(m_computeQueueFamilyIndex)
			.withFlags(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)
			.build())
		, m_computeCommandBuffersContainer(m_computeCommandPool.allocateCommandBuffers(MAX_FRAMES_IN_FLIGHT))
		, m_graphicsCommandPool(vk::CommandPool::Builder()
			.withDevice(*m_pDevice)
			.withQueueFamilyIndex(m_graphicsQueueFamilyIndex)
			.withFlags(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)
			.build())
		, m_graphicsCommandBuffersContainer(m_graphicsCommandPool.allocateCommandBuffers(MAX_FRAMES_IN_FLIGHT))
	{
		// --- Command Buffers ---
		m_computeCommandBuffers.reserve(MAX_FRAMES_IN_FLIGHT);
		m_graphicsCommandBuffers.reserve(MAX_FRAMES_IN_FLIGHT);
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			m_computeCommandBuffers.emplace_back(m_computeCommandBuffersContainer.get(i));
			m_graphicsCommandBuffers.emplace_back(m_graphicsCommandBuffersContainer.get(i));
		}

		// --- Sync ---
		m_imageAvailableSemaphores.reserve(MAX_FRAMES_IN_FLIGHT);
		m_computeFinishedSemaphores.reserve(MAX_FRAMES_IN_FLIGHT);
		m_inFlightFences.reserve(MAX_FRAMES_IN_FLIGHT);
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			m_imageAvailableSemaphores.emplace_builder(vk::Semaphore::Builder()
				.withDevice(*m_pDevice)
				);
			m_computeFinishedSemaphores.emplace_builder(vk::Semaphore::Builder()
				.withDevice(*m_pDevice)
				);
			m_inFlightFences.emplace_builder(vk::Fence::Builder()
				.withDevice(*m_pDevice)
				.withFlags(VK_FENCE_CREATE_SIGNALED_BIT)
				);
		}

		m_renderFinishedSemaphores.reserve(c_swapchainKHR_image_count);
		for (size_t i = 0; i < c_swapchainKHR_image_count; ++i)
		{
			m_renderFinishedSemaphores.emplace_builder(vk::Semaphore::Builder()
				.withDevice(*m_pDevice)
				);
		}
	}
	~data_t() noexcept = default;


	inline bool recreateSwapchainKHR()
	{
		m_pDevice->waitIdle();
		if (!m_swapchainKHR.recreate()) return false;

		m_upFrameResources = std::make_unique<FrameResources>(
			*m_pDevice, VkExtent3D{ m_swapchainKHR.getImageExtent().width, m_swapchainKHR.getImageExtent().height, 1u }
			);

		if (c_swapchainKHR_image_count != m_swapchainKHR.getImages().size())
		{
			c_swapchainKHR_image_count = m_swapchainKHR.getImages().size();
			m_renderFinishedSemaphores.reserve(c_swapchainKHR_image_count);
			for (size_t i = 0; i < c_swapchainKHR_image_count; ++i)
			{
				m_renderFinishedSemaphores.emplace_builder(vk::Semaphore::Builder()
					.withDevice(*m_pDevice)
					);
			}
		}

		m_currentFrame = 0;
		return true;
	}


	inline std::optional<Renderer::FrameContext> beginFrame()
	{
		auto& inFlightFence = m_inFlightFences[m_currentFrame];
		if (inFlightFence.wait() != VK_SUCCESS) [[unlikely]]
			throw std::runtime_error("Failed to wait for fence.");

		uint32_t imageIndex = 0;
		VkResult result = m_swapchainKHR.acquireNextImageKHR(
			UINT64_MAX,
			m_imageAvailableSemaphores[m_currentFrame].vkHandle(),
			VK_NULL_HANDLE,
			&imageIndex
		);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) [[unlikely]]
			return std::nullopt;
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) [[unlikely]]
			throw std::runtime_error("Failed to acquire swapchain image.");

		inFlightFence.reset();

		vk::CommandBuffer computeCmd = m_computeCommandBuffers[m_currentFrame];
		vk::CommandBuffer graphicsCmd = m_graphicsCommandBuffers[m_currentFrame];
		computeCmd.reset();
		graphicsCmd.reset();

		VkCommandBufferBeginInfo beginInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
		};

		computeCmd.begin(beginInfo);
		graphicsCmd.begin(beginInfo);

		return FrameContext{
			.imageIndex = imageIndex,
			.frameIndex = m_currentFrame,
			.computeCmd = computeCmd,
			.graphicsCmd = graphicsCmd
		};
	}

	inline bool endFrame(uint32_t imageIndex)
	{
		auto computeCmd = m_computeCommandBuffers[m_currentFrame];
		auto graphicsCmd = m_graphicsCommandBuffers[m_currentFrame];

		recordCopyCanvasToSwapchain(graphicsCmd, imageIndex);

		computeCmd.end();
		graphicsCmd.end();

		// ------------------------------------------------------------------------
		// 1. COMPUTE SUBMIT
		// ------------------------------------------------------------------------
		{
			const VkCommandBuffer c_buffers[] = { computeCmd.vkHandle() };
			const VkSemaphore c_signals[] = { m_computeFinishedSemaphores[m_currentFrame].vkHandle() };

			VkSubmitInfo computeSubmitInfo{
				.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
				.commandBufferCount = 1u,
				.pCommandBuffers = c_buffers,
				.signalSemaphoreCount = 1u,
				.pSignalSemaphores = c_signals
			};

			if (m_computeQueue.submit(computeSubmitInfo, VK_NULL_HANDLE) != VK_SUCCESS) [[unlikely]]
				throw std::runtime_error("Failed to submit compute command buffer.");
		}

		// ------------------------------------------------------------------------
		// 2. GRAPHICS SUBMIT
		// ------------------------------------------------------------------------
		{
			const VkSemaphore g_waits[] = {
				m_imageAvailableSemaphores[m_currentFrame].vkHandle(),
				m_computeFinishedSemaphores[m_currentFrame].vkHandle()
			};
			const VkPipelineStageFlags g_waitStages[] = {
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT
			};
			const VkCommandBuffer g_buffers[] = { graphicsCmd.vkHandle() };
			const VkSemaphore g_signals[] = { m_renderFinishedSemaphores[imageIndex].vkHandle() };

			VkSubmitInfo graphicsSubmitInfo{
				.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
				.waitSemaphoreCount = 2u,
				.pWaitSemaphores = g_waits,
				.pWaitDstStageMask = g_waitStages,
				.commandBufferCount = 1u,
				.pCommandBuffers = g_buffers,
				.signalSemaphoreCount = 1u,
				.pSignalSemaphores = g_signals
			};

			if (m_graphicsQueue.submit(graphicsSubmitInfo, m_inFlightFences[m_currentFrame].vkHandle()) != VK_SUCCESS) [[unlikely]]
				throw std::runtime_error("Failed to submit graphics command buffer.");
		}

		// ------------------------------------------------------------------------
		// 3. PRESENT
		// ------------------------------------------------------------------------
		{
			VkSemaphore renderFinishedSemaphores[] = { m_renderFinishedSemaphores[imageIndex].vkHandle() };
			VkSwapchainKHR swapchains[] = { m_swapchainKHR.vkHandle() };

			VkPresentInfoKHR presentInfo{
				.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
				.waitSemaphoreCount = 1u,
				.pWaitSemaphores = renderFinishedSemaphores,
				.swapchainCount = 1u,
				.pSwapchains = swapchains,
				.pImageIndices = &imageIndex
			};

			VkResult result = m_graphicsQueue.presentKHR(presentInfo);

			m_currentFrame = (m_currentFrame + 1u) % MAX_FRAMES_IN_FLIGHT;

			switch (result)
			{
			[[likely]] case VK_SUCCESS:
				break;

			case VK_SUBOPTIMAL_KHR:
			case VK_ERROR_OUT_OF_DATE_KHR:
				return false; // recreate needed

			[[unlikely]] default:
				throw std::runtime_error("Failed to present swapchain image.");
			}
		}

		return true;
	}



	inline const size_t getMaxFramesInFlight() const noexcept { return MAX_FRAMES_IN_FLIGHT; }

	inline const vk::SwapchainKHR& getSwapchain() const noexcept { return m_swapchainKHR; }
	inline const vk::Image& getCanvasImage() const noexcept { return m_upFrameResources->canvasImage; }
	inline const vk::ImageView& getCanvasImageView() const noexcept { return m_upFrameResources->canvasImageView; }

	inline vk::SwapchainKHR& getSwapchain() noexcept { return m_swapchainKHR; }
	inline vk::Image& getCanvasImage() noexcept { return m_upFrameResources->canvasImage; }
	inline vk::ImageView& getCanvasImageView() noexcept { return m_upFrameResources->canvasImageView; }
};

Renderer::Renderer(
	vk::Device& device,
	vk::SurfaceKHR& surfaceKHR,
	uint32_t graphicsQueueFamilyIndex,
	uint32_t computeQueueFamilyIndex,
	size_t graphicsQueueIndex,
	size_t computeQueueIndex
)
	: m_pData(new data_t{ device, surfaceKHR, graphicsQueueFamilyIndex, computeQueueFamilyIndex, graphicsQueueIndex, computeQueueIndex }) {}
Renderer::~Renderer() noexcept { delete m_pData; }

bool Renderer::recreateSwapchainKHR() { return m_pData->recreateSwapchainKHR(); }

std::optional<Renderer::FrameContext> Renderer::beginFrame() { return m_pData->beginFrame(); }
bool Renderer::endFrame(uint32_t imageIndex) { return m_pData->endFrame(imageIndex); }

const size_t Renderer::getMaxFramesInFlight() const noexcept { return m_pData->getMaxFramesInFlight(); }

const vk::SwapchainKHR& Renderer::getSwapchain() const noexcept { return m_pData->getSwapchain(); }
const vk::Image& Renderer::getCanvasImage() const noexcept { return m_pData->getCanvasImage(); }
const vk::ImageView& Renderer::getCanvasImageView() const noexcept { return m_pData->getCanvasImageView(); }

vk::SwapchainKHR& Renderer::getSwapchain() noexcept { return m_pData->getSwapchain(); }
vk::Image& Renderer::getCanvasImage() noexcept { return m_pData->getCanvasImage(); }
vk::ImageView& Renderer::getCanvasImageView() noexcept { return m_pData->getCanvasImageView(); }
