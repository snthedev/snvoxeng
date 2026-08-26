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
#include <vector>
#include <snassert/snassert.hpp>
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

	// --- Canvas ownership/layout tracking (canvas is VK_SHARING_MODE_EXCLUSIVE) ---
	// true right after creation/recreation: the first writes come from compute.
	bool m_canvasOwnedByCompute{ true };
	VkImageLayout m_canvasImageLayout{ VK_IMAGE_LAYOUT_UNDEFINED };

	enum class FrameState { Idle, InFrame };
	FrameState m_frameState{ FrameState::Idle };

	vk::Queue m_computeQueue;
	vk::Queue m_graphicsQueue;

	bool separateQueueFamilies() const noexcept
	{
		return m_computeQueueFamilyIndex != m_graphicsQueueFamilyIndex;
	}

	// --- Canvas descriptor slots & resize callback ---
	struct CanvasImageSlot { VkDescriptorSet set; uint32_t binding; uint32_t arrayElement; };
	std::vector<CanvasImageSlot> m_canvasImageSlots;
	Renderer::CanvasResizedFn m_canvasResizedFn{ nullptr };
	void* m_canvasResizedUserdata{ nullptr };

	// --- Swapchain KHR ---
	vk::SwapchainKHR m_swapchainKHR;
	size_t c_swapchainKHR_image_count;

	VkFormat resolveCanvasFormat() const
	{
		// Prefer the swapchain format: an identical format guarantees a
		// lossless vkCmdCopyImage and keeps the storage-image writes
		// consistent with what ends up on screen (no channel reordering).
		// Fall back to R8G8B8A8_UNORM when the swapchain format cannot serve
		// as a storage/transfer-source image on this device.
		const VkFormat candidates[]{
			m_swapchainKHR.getImageFormat(),
			VK_FORMAT_R8G8B8A8_UNORM,
		};

		constexpr auto requiredFeatures =
			VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT;

		const VkPhysicalDevice physicalDevice = m_pDevice->getPhysicalDevice().vkHandle();
		for (const VkFormat candidate : candidates)
		{
			VkFormatProperties properties{};
			vkGetPhysicalDeviceFormatProperties(physicalDevice, candidate, &properties);
			if ((properties.optimalTilingFeatures & requiredFeatures) == requiredFeatures)
				return candidate;
		}

		snassert(false, "No suitable canvas image format found",
			"The swapchain format and R8G8B8A8_UNORM both lack STORAGE_IMAGE | TRANSFER_SRC support");
		return candidates[1];
	}

	// --- Frame Resources ---
	struct FrameResources
	{
		vk::Image canvasImage;
		vk::ImageView canvasImageView;

		FrameResources(const vk::Device& device, VkExtent3D extent, VkFormat format)
			: canvasImage(vk::Image::Builder()
				.withDevice(device)
				.withImageType(VK_IMAGE_TYPE_2D)
				.withFormat(format)
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

	void recordAcquireCanvasForCompute(vk::CommandBuffer& computeCmd)
	{
		// Canvas: take ownership back (graphics -> compute when the families
		// are separate) and make it writable by the compute shader (GENERAL).
		const bool needOwnership = separateQueueFamilies() && !m_canvasOwnedByCompute;
		const VkImage image = m_upFrameResources->canvasImage.vkHandle();

		if (needOwnership)
		{
			// 1. Pure ownership acquire. The layouts MUST match the release
			//    barrier recorded at the end of the previous frame's graphics
			//    commands (see recordPresentCanvasToSwapchain).
			VkImageMemoryBarrier acquire_barriers[]{ {
					.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
					.srcAccessMask = 0,
					.dstAccessMask = 0,
					.oldLayout = m_canvasImageLayout,
					.newLayout = m_canvasImageLayout,
					.srcQueueFamilyIndex = m_graphicsQueueFamilyIndex,
					.dstQueueFamilyIndex = m_computeQueueFamilyIndex,
					.image = image,
					.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
				}
			};
			computeCmd.cmdPipelineBarrier(
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				0, {}, {}, acquire_barriers
			);
			m_canvasOwnedByCompute = true;
		}

		if (m_canvasImageLayout != VK_IMAGE_LAYOUT_GENERAL)
		{
			// 2. Plain layout transition to GENERAL (image already belongs to
			//    the compute side here).
			VkImageMemoryBarrier layout_barriers[]{ {
					.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
					.srcAccessMask = 0,
					.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT,
					.oldLayout = m_canvasImageLayout,
					.newLayout = VK_IMAGE_LAYOUT_GENERAL,
					.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
					.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
					.image = image,
					.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
				}
			};
			computeCmd.cmdPipelineBarrier(
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				0, {}, {}, layout_barriers
			);
			m_canvasImageLayout = VK_IMAGE_LAYOUT_GENERAL;
		}

		m_canvasOwnedByCompute = true;
	}

	void recordPresentCanvasToSwapchain(vk::CommandBuffer& graphicsCmd, uint32_t imageIndex) const
	{
		VkImage currentSwapchainImage = m_swapchainKHR.getImages()[imageIndex].vkHandle();

		const bool separate = separateQueueFamilies();

		VkImageMemoryBarrier barriers[]{ {
				// Canvas Image: acquire ownership. The layouts must match the
				// release barrier recorded into the compute command buffer -
				// this barrier completes the ownership transfer and leaves the
				// image ready for the copy below.
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				.srcAccessMask = 0,
				.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
				.oldLayout = VK_IMAGE_LAYOUT_GENERAL,
				.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				.srcQueueFamilyIndex = separate ? m_computeQueueFamilyIndex : VK_QUEUE_FAMILY_IGNORED,
				.dstQueueFamilyIndex = separate ? m_graphicsQueueFamilyIndex : VK_QUEUE_FAMILY_IGNORED,
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

		// 2.5 Canvas Image: release ownership back to the compute queue family.
		// Pure ownership barrier: its layouts must match the acquire barrier
		// recorded at the beginning of the next frame (the layout transition
		// itself happens on the compute side afterwards).
		if (separate)
		{
			VkImageMemoryBarrier release_back_barriers[]{ {
					.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
					.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
					.dstAccessMask = 0,
					.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					.srcQueueFamilyIndex = m_graphicsQueueFamilyIndex,
					.dstQueueFamilyIndex = m_computeQueueFamilyIndex,
					.image = m_upFrameResources->canvasImage.vkHandle(),
					.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
				}
			};
			graphicsCmd.cmdPipelineBarrier(
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				0, {}, {}, release_back_barriers
			);
		}

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
		vk::Queue graphicsQueue,
		vk::Queue computeQueue
	)
		: m_pDevice(&device)
		, m_pSurfaceKHR(&surfaceKHR)
		, m_graphicsQueueFamilyIndex(device.getQueueFamilyIndex(graphicsQueue.getContainerIdx()))
		, m_computeQueueFamilyIndex(device.getQueueFamilyIndex(computeQueue.getContainerIdx()))
		, m_computeQueue(computeQueue)
		, m_graphicsQueue(graphicsQueue)
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
			*m_pDevice, VkExtent3D{ m_swapchainKHR.getImageExtent().width, m_swapchainKHR.getImageExtent().height, 1u },
			resolveCanvasFormat()
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
			*m_pDevice, VkExtent3D{ m_swapchainKHR.getImageExtent().width, m_swapchainKHR.getImageExtent().height, 1u },
			resolveCanvasFormat()
			);

		writeCanvasImageSlots();

		if (m_canvasResizedFn) [[unlikely]]
			m_canvasResizedFn(m_canvasResizedUserdata, m_swapchainKHR.getImageExtent());

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

		m_canvasOwnedByCompute = true;
		m_canvasImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		m_frameState = data_t::FrameState::Idle;
		m_currentFrame = 0;
		return true;
	}

	void writeCanvasImageSlots()
	{
		if (m_canvasImageSlots.empty()) [[likely]] return;

		const VkImageView imageView = m_upFrameResources->canvasImageView.vkHandle();

		std::vector<VkDescriptorImageInfo> imageInfos;
		imageInfos.reserve(m_canvasImageSlots.size());
		for (const CanvasImageSlot& slot : m_canvasImageSlots)
			imageInfos.push_back(VkDescriptorImageInfo{
				.sampler = VK_NULL_HANDLE,
				.imageView = imageView,
				.imageLayout = VK_IMAGE_LAYOUT_GENERAL,
				});

		std::vector<VkWriteDescriptorSet> writes;
		writes.reserve(m_canvasImageSlots.size());
		for (size_t i = 0; i < m_canvasImageSlots.size(); ++i)
		{
			const CanvasImageSlot& slot = m_canvasImageSlots[i];
			writes.push_back(VkWriteDescriptorSet{
				.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet = slot.set,
				.dstBinding = slot.binding,
				.dstArrayElement = slot.arrayElement,
				.descriptorCount = 1u,
				.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
				.pImageInfo = &imageInfos[i],
				});
		}

		m_pDevice->updateDescriptorSets(
			static_cast<uint32_t>(writes.size()), writes.data(), 0u, nullptr);
	}

	void attachCanvasImage(VkDescriptorSet set, uint32_t binding, uint32_t arrayElement)
	{
		m_canvasImageSlots.push_back(CanvasImageSlot{ set, binding, arrayElement });
		writeCanvasImageSlots();
	}

	void setCanvasResizedCallback(Renderer::CanvasResizedFn fn, void* userdata)
	{
		m_canvasResizedFn = fn;
		m_canvasResizedUserdata = userdata;
	}


	inline std::optional<Renderer::FrameContext> beginFrame()
	{
		snassert(m_frameState == FrameState::Idle,
			"Renderer::beginFrame called while already inside a frame",
			"submitFrame() must be called before the next beginFrame()");

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

		recordAcquireCanvasForCompute(computeCmd);

		m_frameState = FrameState::InFrame;
		return FrameContext{
			.imageIndex = imageIndex,
			.frameIndex = m_currentFrame,
			.computeCmd = computeCmd,
			.graphicsCmd = graphicsCmd
		};
	}

	inline bool submitFrame(const Renderer::FrameContext& frame)
	{
		snassert(m_frameState == FrameState::InFrame,
			"Renderer::submitFrame called outside of a frame",
			"submitFrame() must follow a successful beginFrame()");
		snassert(frame.frameIndex == m_currentFrame,
			"Stale Renderer::FrameContext",
			"The frame context does not match the current internal frame slot");

		auto computeCmd = m_computeCommandBuffers[m_currentFrame];
		auto graphicsCmd = m_graphicsCommandBuffers[m_currentFrame];
		const uint32_t imageIndex = frame.imageIndex;

		// Canvas: release ownership (compute -> graphics when the families are
		// separate) and transition GENERAL -> TRANSFER_SRC_OPTIMAL for the copy.
		// The matching acquire barrier is recorded into the graphics command
		// buffer by recordPresentCanvasToSwapchain().
		{
			const bool separate = separateQueueFamilies();

			VkImageMemoryBarrier release_barriers[]{ {
					.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
					.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT,
					.dstAccessMask = 0,
					.oldLayout = VK_IMAGE_LAYOUT_GENERAL,
					.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					.srcQueueFamilyIndex = separate ? m_computeQueueFamilyIndex : VK_QUEUE_FAMILY_IGNORED,
					.dstQueueFamilyIndex = separate ? m_graphicsQueueFamilyIndex : VK_QUEUE_FAMILY_IGNORED,
					.image = m_upFrameResources->canvasImage.vkHandle(),
					.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
				}
			};
			computeCmd.cmdPipelineBarrier(
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				0, {}, {}, release_barriers
			);
			m_canvasImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			m_canvasOwnedByCompute = false;
		}

		recordPresentCanvasToSwapchain(graphicsCmd, frame.imageIndex);

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
			m_frameState = FrameState::Idle;

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
	vk::Queue graphicsQueue,
	vk::Queue computeQueue
)
	: m_pData(new data_t{ device, surfaceKHR, graphicsQueue, computeQueue }) {}
Renderer::~Renderer() noexcept { delete m_pData; }

bool Renderer::recreateSwapchainKHR() { return m_pData->recreateSwapchainKHR(); }

std::optional<Renderer::FrameContext> Renderer::beginFrame() { return m_pData->beginFrame(); }
bool Renderer::submitFrame(const FrameContext& frame) { return m_pData->submitFrame(frame); }

void Renderer::attachCanvasImage(VkDescriptorSet set, uint32_t binding, uint32_t arrayElement) { m_pData->attachCanvasImage(set, binding, arrayElement); }
void Renderer::setCanvasResizedCallback(CanvasResizedFn fn, void* userdata) { m_pData->setCanvasResizedCallback(fn, userdata); }

const size_t Renderer::getMaxFramesInFlight() const noexcept { return m_pData->getMaxFramesInFlight(); }

const vk::SwapchainKHR& Renderer::getSwapchain() const noexcept { return m_pData->getSwapchain(); }
const vk::Image& Renderer::getCanvasImage() const noexcept { return m_pData->getCanvasImage(); }
const vk::ImageView& Renderer::getCanvasImageView() const noexcept { return m_pData->getCanvasImageView(); }

vk::SwapchainKHR& Renderer::getSwapchain() noexcept { return m_pData->getSwapchain(); }
vk::Image& Renderer::getCanvasImage() noexcept { return m_pData->getCanvasImage(); }
vk::ImageView& Renderer::getCanvasImageView() noexcept { return m_pData->getCanvasImageView(); }
