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

	// --- Canvas Image ---
	vk::Image m_canvasImage;
	vk::DeviceMemory m_canvasImageMemory;
	vk::ImageView m_canvasImageView;

	// --- Command Pools & Buffers ---
	vk::CommandPool m_computeCommandPool;
	vk::CommandBuffersContainer m_computeCommandBuffersContainer;
	std::vector<vk::CommandBuffer> m_computeCommandBuffers;

	vk::CommandPool m_graphicsCommandPool;
	vk::CommandBuffersContainer m_graphicsCommandBuffersContainer;
	std::vector<vk::CommandBuffer> m_graphicsCommandBuffers;

	// --- Sync ---
	std::vector<vk::Semaphore> m_imageAvailableSemaphores;
	std::vector<vk::Semaphore> m_computeFinishedSemaphores;
	std::vector<vk::Semaphore> m_renderFinishedSemaphores;
	std::vector<vk::Fence> m_inFlightFences;

	void recordCopyCanvasToSwapchain(vk::CommandBuffer& graphicsCmd, uint32_t imageIndex) const
	{
		VkImage currentSwapchainImage = m_swapchainKHR.getImages()[imageIndex].vkHandle();

		// 1. ACQUIRE барьер для Canvas + Prepare Swapchain Image
		VkImageMemoryBarrier barriers[2]{};

		// Canvas Image: Acquire ownership & transition to TRANSFER_SRC
		barriers[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barriers[0].srcAccessMask = 0; // Игнорируется при ACQUIRE
		barriers[0].dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barriers[0].oldLayout = VK_IMAGE_LAYOUT_GENERAL;
		barriers[0].newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barriers[0].srcQueueFamilyIndex = m_computeQueueFamilyIndex;
		barriers[0].dstQueueFamilyIndex = m_graphicsQueueFamilyIndex;
		barriers[0].image = m_canvasImage.vkHandle();
		barriers[0].subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

		// Swapchain Image: UNDEFINED -> TRANSFER_DST
		barriers[1].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barriers[1].srcAccessMask = 0;
		barriers[1].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barriers[1].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barriers[1].newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barriers[1].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barriers[1].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barriers[1].image = currentSwapchainImage;
		barriers[1].subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

		graphicsCmd.cmdPipelineBarrier(
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			0, {}, {}, { barriers, 2 }
		);

		// 2. Copy Image
		VkImageCopy copyRegion{
			.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
			.srcOffset = { 0, 0, 0 },
			.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
			.dstOffset = { 0, 0, 0 },
			.extent = { m_swapchainKHR.getImageExtent().width, m_swapchainKHR.getImageExtent().height, 1u }
		};

		graphicsCmd.cmdCopyImage(
			m_canvasImage.vkHandle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			currentSwapchainImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			{ &copyRegion, 1 }
		);

		// 3. Swapchain Image -> PRESENT_SRC_KHR
		VkImageMemoryBarrier presentBarrier{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
			.dstAccessMask = 0,
			.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = currentSwapchainImage,
			.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
		};

		graphicsCmd.cmdPipelineBarrier(
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			0, {}, {}, { &presentBarrier, 1 }
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
		// --- Canvas Image ---
		, m_canvasImage(vk::Image::Builder()
			.withDevice(*m_pDevice)
			.withImageType(VK_IMAGE_TYPE_2D)
			.withFormat(VK_FORMAT_R8G8B8A8_UNORM)
			.withExtent({ m_swapchainKHR.getImageExtent().width, m_swapchainKHR.getImageExtent().height, 1u })
			.withMipLevels(1u)
			.withArrayLayers(1u)
			.withSamples(VK_SAMPLE_COUNT_1_BIT)
			.withTiling(VK_IMAGE_TILING_OPTIMAL)
			.withUsage(VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
			.withSharingMode(VK_SHARING_MODE_EXCLUSIVE)
			.withInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
			.build())
		, m_canvasImageMemory(
			[&]() -> vk::DeviceMemory {
				auto image_mem_req = m_canvasImage.getMemoryRequirements();
				auto image_mem_type = m_pDevice->getPhysicalDevice().findMemoryType(
					image_mem_req.memoryTypeBits,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
				);
				if (image_mem_type == vk::PhysicalDevice::nmatch)
					throw std::runtime_error("Failed to find requested GPU's memory");

				auto image_memory = vk::DeviceMemory::Builder()
					.withDevice(*m_pDevice)
					.withAllocationSize(image_mem_req.size)
					.withMemoryTypeIndex(image_mem_type)
					.build();
				image_memory.bindImage(m_canvasImage, 0);
				return image_memory;
			}())
		, m_canvasImageView(vk::ImageView::Builder()
			.withImage(m_canvasImage)
			.withViewType(VK_IMAGE_VIEW_TYPE_2D)
			.withSubresourceRange({
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
				})
			.build())
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
			m_computeCommandBuffers.push_back(m_computeCommandBuffersContainer.get(i));
			m_graphicsCommandBuffers.push_back(m_graphicsCommandBuffersContainer.get(i));
		}

		// --- Sync ---
		m_imageAvailableSemaphores.reserve(MAX_FRAMES_IN_FLIGHT);
		m_computeFinishedSemaphores.reserve(MAX_FRAMES_IN_FLIGHT);
		m_inFlightFences.reserve(MAX_FRAMES_IN_FLIGHT);
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			m_imageAvailableSemaphores.push_back(vk::Semaphore::Builder()
				.withDevice(*m_pDevice)
				.build());
			m_computeFinishedSemaphores.push_back(vk::Semaphore::Builder()
				.withDevice(*m_pDevice)
				.build());
			m_inFlightFences.push_back(vk::Fence::Builder()
				.withDevice(*m_pDevice)
				.withFlags(VK_FENCE_CREATE_SIGNALED_BIT)
				.build());
		}

		m_renderFinishedSemaphores.reserve(c_swapchainKHR_image_count);
		for (size_t i = 0; i < c_swapchainKHR_image_count; ++i)
		{
			m_renderFinishedSemaphores.push_back(vk::Semaphore::Builder()
				.withDevice(*m_pDevice)
				.build());
		}
	}
	~data_t() noexcept = default;


	inline bool recreateSwapchainKHR()
	{
		m_pDevice->waitIdle();
		if (!m_swapchainKHR.recreate()) return false;

		m_canvasImageView.~ImageView();
		m_canvasImageMemory.~DeviceMemory();
		m_canvasImage.~Image();

		new (&m_canvasImage) vk::Image(vk::Image::Builder()
			.withDevice(*m_pDevice)
			.withImageType(VK_IMAGE_TYPE_2D)
			.withFormat(VK_FORMAT_R8G8B8A8_UNORM)
			.withExtent({ m_swapchainKHR.getImageExtent().width, m_swapchainKHR.getImageExtent().height, 1u })
			.withMipLevels(1u)
			.withArrayLayers(1u)
			.withSamples(VK_SAMPLE_COUNT_1_BIT)
			.withTiling(VK_IMAGE_TILING_OPTIMAL)
			.withUsage(VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
			.withSharingMode(VK_SHARING_MODE_EXCLUSIVE)
			.withInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
			.build());

		auto image_mem_req = m_canvasImage.getMemoryRequirements();
		auto image_mem_type = m_pDevice->getPhysicalDevice().findMemoryType(
			image_mem_req.memoryTypeBits,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
		if (image_mem_type == vk::PhysicalDevice::nmatch)
			throw std::runtime_error("Failed to find requested GPU's memory");

		new (&m_canvasImageMemory) vk::DeviceMemory(vk::DeviceMemory::Builder()
			.withDevice(*m_pDevice)
			.withAllocationSize(image_mem_req.size)
			.withMemoryTypeIndex(image_mem_type)
			.build());
		m_canvasImageMemory.bindImage(m_canvasImage, 0);

		new (&m_canvasImageView) vk::ImageView(vk::ImageView::Builder()
			.withImage(m_canvasImage)
			.withViewType(VK_IMAGE_VIEW_TYPE_2D)
			.withSubresourceRange({
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
				})
			.build());

		if (c_swapchainKHR_image_count != m_swapchainKHR.getImages().size())
		{
			c_swapchainKHR_image_count = m_swapchainKHR.getImages().size();
			m_renderFinishedSemaphores.clear();
			m_renderFinishedSemaphores.reserve(c_swapchainKHR_image_count);
			for (size_t i = 0; i < c_swapchainKHR_image_count; ++i)
			{
				m_renderFinishedSemaphores.push_back(vk::Semaphore::Builder()
					.withDevice(*m_pDevice)
					.build());
			}
		}

		m_currentFrame = 0;
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

			if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) [[unlikely]]
				return false;
			else if (result != VK_SUCCESS) [[unlikely]]
				throw std::runtime_error("Failed to present swapchain image.");
		}

		return true;
	}



	inline const vk::SwapchainKHR& getSwapchain() const noexcept { return m_swapchainKHR; }
	inline const vk::Image& getCanvasImage() const noexcept { return m_canvasImage; }
	inline const vk::ImageView& getCanvasImageView() const noexcept { return m_canvasImageView; }

	inline vk::SwapchainKHR& getSwapchain() noexcept { return m_swapchainKHR; }
	inline vk::Image& getCanvasImage() noexcept { return m_canvasImage; }
	inline vk::ImageView& getCanvasImageView() noexcept { return m_canvasImageView; }
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

const vk::SwapchainKHR& Renderer::getSwapchain() const noexcept { return m_pData->getSwapchain(); }
const vk::Image& Renderer::getCanvasImage() const noexcept { return m_pData->getCanvasImage(); }
const vk::ImageView& Renderer::getCanvasImageView() const noexcept { return m_pData->getCanvasImageView(); }

vk::SwapchainKHR& Renderer::getSwapchain() noexcept { return m_pData->getSwapchain(); }
vk::Image& Renderer::getCanvasImage() noexcept { return m_pData->getCanvasImage(); }
vk::ImageView& Renderer::getCanvasImageView() noexcept { return m_pData->getCanvasImageView(); }
