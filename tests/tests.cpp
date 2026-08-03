#include <tests/tests.hpp>

#include <GLFW/glfw3.h>
#if defined(_WIN32)
#define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(__APPLE__)
#define GLFW_EXPOSE_NATIVE_COCOA
#else
#define GLFW_EXPOSE_NATIVE_WAYLAND
#define GLFW_EXPOSE_NATIVE_X11
#endif
#include <GLFW/glfw3native.h>

#include <ostream>
#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>

static sn::voxeng::WindowDescription_t glfw_get_window_descripton(GLFWwindow* window)
{
	sn::voxeng::WindowDescription_t desc;
	{
		int width, height;
		glfwGetWindowSize(window, &width, &height);
		desc.width = static_cast<uint32_t>(width);
		desc.height = static_cast<uint32_t>(height);
	}

	switch (glfwGetPlatform())
	{
#if defined(GLFW_EXPOSE_NATIVE_WIN32)
	case GLFW_PLATFORM_WIN32:
		desc.platform = sn::voxeng::WindowDescription_t::platform_type::win32;
		desc.handle_param1 = reinterpret_cast<void*>(GetModuleHandle(nullptr));
		desc.handle_param2 = reinterpret_cast<void*>(glfwGetWin32Window(window));
		break;
#endif

#if defined(GLFW_EXPOSE_NATIVE_COCOA)
	case GLFW_PLATFORM_COCOA:
		desc.platform = sn::voxeng::WindowDescription_t::platform_type::macos_metal;
		desc.handle_param1 = nullptr;
		desc.handle_param2 = reinterpret_cast<void*>(glfwGetCocoaWindow(window));
		break;
#endif

#if defined(GLFW_EXPOSE_NATIVE_WAYLAND)
	case GLFW_PLATFORM_WAYLAND:
		desc.platform = sn::voxeng::WindowDescription_t::platform_type::linux_wayland;
		desc.handle_param1 = reinterpret_cast<void*>(glfwGetWaylandDisplay());
		desc.handle_param2 = reinterpret_cast<void*>(glfwGetWaylandWindow(window));
		break;
#endif

#if defined(GLFW_EXPOSE_NATIVE_X11)
	case GLFW_PLATFORM_X11:
		desc.platform = sn::voxeng::WindowDescription_t::platform_type::linux_x11;
		desc.handle_param1 = reinterpret_cast<void*>(glfwGetX11Display());
		desc.handle_param2 = reinterpret_cast<void*>(static_cast<uintptr_t>(glfwGetX11Window(window)));
		break;
#endif

	default:
		desc.platform = sn::voxeng::WindowDescription_t::platform_type::headless;
		desc.handle_param1 = nullptr;
		desc.handle_param2 = nullptr;
		break;
	}
	return desc;
}

template <typename T>
static size_t u2bin(const T& val, char* buf = nullptr)
{
	static_assert(std::is_unsigned_v<T>, "val must be an unsigned integer type.");
	constexpr size_t bits = sizeof(T) * 8u;
	if (buf != nullptr)
	{
		for (size_t i = 0; i < bits; ++i)
			buf[i] = (val & ((T)(1) << (bits - i - 1u))) != 0 ? '1' : '0';
	}
	return bits;
}

#include <cstrs/cstrs.hpp>
template <typename T>
static cstrs::cstr u2bin_str(const T& val)
{
	cstrs::cstr buf(u2bin(val));
	u2bin(val, buf.data());
	return buf;
}

static cstrs::cstr VkQueueFlags2str(VkQueueFlags val)
{
	size_t at = 0u;
	cstrs::cstr buf(512, '\0');
	at = buf.fill("[ ", at);
	if ((val & VK_QUEUE_GRAPHICS_BIT) != 0) at = buf.fill("\"GRAPHICS_BIT\", ", at);
	if ((val & VK_QUEUE_COMPUTE_BIT) != 0) at = buf.fill("\"COMPUTE_BIT\", ", at);
	if ((val & VK_QUEUE_TRANSFER_BIT) != 0) at = buf.fill("\"TRANSFER_BIT\", ", at);
	if ((val & VK_QUEUE_SPARSE_BINDING_BIT) != 0) at = buf.fill("\"SPARSE_BINDING_BIT\", ", at);
	if ((val & VK_QUEUE_PROTECTED_BIT) != 0) at = buf.fill("\"PROTECTED_BIT\", ", at);
	if ((val & VK_QUEUE_VIDEO_DECODE_BIT_KHR) != 0) at = buf.fill("\"VIDEO_DECODE_BIT_KHR\", ", at);
	if ((val & VK_QUEUE_VIDEO_ENCODE_BIT_KHR) != 0) at = buf.fill("\"VIDEO_ENCODE_BIT_KHR\", ", at);
	if ((val & VK_QUEUE_OPTICAL_FLOW_BIT_NV) != 0) at = buf.fill("\"OPTICAL_FLOW_BIT_NV\", ", at);
	if ((val & VK_QUEUE_DATA_GRAPH_BIT_ARM) != 0) at = buf.fill("\"DATA_GRAPH_BIT_ARM\", ", at);
	buf.truncate(at);
	if (at > 2u) buf[at - 2u] = ' ';
	buf[at - 1u] = ']';
	return buf;
}

std::ostream& operator<<(std::ostream& os, const VkExtent3D& val)
{
	return os
		<< "{"
		<< " \"width\": " << val.width
		<< ", \"height\": " << val.height
		<< ", \"depth\": " << val.depth
		<< "}"
		;
}
std::ostream& operator<<(std::ostream& os, const VkQueueFamilyProperties& val)
{
	return os
		<< "{"
		<< " \"queueFlags\": " << VkQueueFlags2str(val.queueFlags)
		<< ", \"queueCount\": " << val.queueCount
		<< ", \"timestampValidBits\": " << val.timestampValidBits
		<< ", \"minImageTransferGranularity\": " << val.minImageTransferGranularity
		<< "}"
		;
}

int main()
{
	if (glfwInit() != GLFW_TRUE)
	{
		std::cerr << "glfwInit() failed.\n";
		return 1;
	}

	auto pWindow = glfwCreateWindow(1280, 720, "sn::voxeng", nullptr, nullptr);
	if (pWindow == nullptr)
	{
		std::cerr << "glfwCreateWindow() failed.\n";
		glfwTerminate();
		return 1;
	}

	try
	{
		auto window_description = glfw_get_window_descripton(pWindow);

		std::vector<const char*> instance_extensions = {};
		{
			uint32_t glfwExtensionCount = 0;
			const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
			instance_extensions.insert(instance_extensions.begin(), glfwExtensions, glfwExtensions + glfwExtensionCount);
		}

		auto instance = sn::voxeng::vk::Instance::Builder()
			.withApiVersion(VK_API_VERSION_1_3)
			.withApplicationName("snvoxeng test")
			.withApplicationVersion(VK_MAKE_API_VERSION(0, 0, 1, 0))
			.addExtensions(instance_extensions)
#ifndef NDEBUG
			.addValidationLayers({ "VK_LAYER_KHRONOS_validation" })
			.addExtensions({ "VK_EXT_debug_utils" })
			.withDebugMessengerEnabled(true)
			.withDebugStream(std::cout)
#endif
			.sbuild();

		auto surface_khr = sn::voxeng::vk::SurfaceKHR::Builder()
			.withInstance(instance)
			.withWindowDescription(window_description)
			.sbuild();

		sn::voxeng::vk::PhysicalDeviceRegistry physical_device_registry(instance);

		sn::voxeng::vk::PhysicalDevice gpu = physical_device_registry.first();
		{
			using namespace sn::voxeng::vk::fPhysicalDeviseSelectors;

			std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
			fExtensions_user_data_t fExtensions_user_data{
				.ppExtensionNames = deviceExtensions.data(),
				.extensionCount = deviceExtensions.size(),
			};
			fQueueSupport_user_data_t fQueueSupport_user_data{
				.requiredFlagsOr = VkQueueFlagBits::VK_QUEUE_COMPUTE_BIT | VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT,
				.requiredFlagsAnd = VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT,
			};
			fSurfaceSupport_user_data_t fSurfaceSupport_user_data{
				.surface = surface_khr.vkHandle(),
			};

			const auto gpus = physical_device_registry
				.pick(fExtensions, &fExtensions_user_data)
				.pick(fQueueSupport, &fQueueSupport_user_data)
				.pick(fSurfaceSupport, &fSurfaceSupport_user_data)
				;

			fDeviceType_user_data_t fDeviceType_user_data{
				.deviceType = VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
			};

			const auto best_gpus = gpus
				.pick(fDeviceType, &fDeviceType_user_data)
				;
			if (best_gpus) gpu = best_gpus.first();
			else gpu = gpus.first();
		}
		std::cout << "GPU: " << gpu.getProperties().deviceName << "\n";

		uint32_t graphics_family_index = gpu.findQueueFamily({ .flags = VK_QUEUE_GRAPHICS_BIT, .surface = surface_khr.vkHandle() });
		snassert(graphics_family_index != sn::voxeng::vk::PhysicalDevice::nmatch,
			"Failed to find requested GPU's graphics queue family",
			"See GPU pick impl");

		uint32_t transfer_family_index = gpu.findQueueFamily({ .flags = VK_QUEUE_TRANSFER_BIT, .preferDedicated = true });
		snassert(transfer_family_index != sn::voxeng::vk::PhysicalDevice::nmatch,
			"Failed to find requested GPU's transfer queue family",
			"See GPU pick impl");

		uint32_t compute_family_index = gpu.findQueueFamily({ .flags = VK_QUEUE_COMPUTE_BIT,  .preferDedicated = true });
		snassert(compute_family_index != sn::voxeng::vk::PhysicalDevice::nmatch,
			"Failed to find requested GPU's compute queue family",
			"See GPU pick impl");

		size_t graphics_queue_index = 0u;
		size_t compute_queue_index = 0u;
		size_t transfer_queue_index = 0u;

		auto device = sn::voxeng::vk::Device::Builder()
			.withPhysicalDevice(gpu)
			.addQueueRequests({ .queueFamilyIndex = graphics_family_index, .queuePriority = 1.0f, .pOutQueueIndex = &graphics_queue_index })
			.addQueueRequests({ .queueFamilyIndex = compute_family_index, .queuePriority = 0.8f, .pOutQueueIndex = &compute_queue_index })
			.addQueueRequests({ .queueFamilyIndex = transfer_family_index, .queuePriority = 0.5f, .pOutQueueIndex = &transfer_queue_index })
			.addExtensions(VK_KHR_SWAPCHAIN_EXTENSION_NAME)
			.withPhysicalDevice13Features({ .dynamicRendering = VK_TRUE })
			.sbuild();

		auto graphics_queue = device.getQueue(graphics_queue_index);
		auto compute_queue = device.getQueue(compute_queue_index);
		auto transfer_queue = device.getQueue(transfer_queue_index);

		std::cout << "Graphics Family Index: " << graphics_family_index << std::endl;
		std::cout << "Compute Family Index: " << compute_family_index << std::endl;
		std::cout << "Transfer Family Index: " << transfer_family_index << std::endl;

		std::cout << "Graphics Queue Index: " << graphics_queue.getContainerIdx() << std::endl;
		std::cout << "Compute Queue Index: " << compute_queue.getContainerIdx() << std::endl;
		std::cout << "Transfer Queue Index: " << transfer_queue.getContainerIdx() << std::endl;

		std::cout << "Graphics Queue 0x" << std::hex << graphics_queue.vkHandle() << std::dec << "\n";
		std::cout << "Compute Queue 0x" << std::hex << compute_queue.vkHandle() << std::dec << "\n";
		std::cout << "Transfer Queue 0x" << std::hex << transfer_queue.vkHandle() << std::dec << "\n";

		auto swapchain_khr = sn::voxeng::vk::SwapchainKHR::Builder()
			.withDevice(device)
			.withSurfaceKHR(surface_khr)
			.addQueueFamilyIndices(graphics_family_index)
			.withImageUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT)
			.sbuild();

		std::cout << "VkSwapchainKHR: 0x" << std::hex << swapchain_khr.vkHandle() << std::dec << "\n";
		std::cout << "Swapchain image count: " << swapchain_khr.getImages().size() << "\n";
		for (const auto& image : swapchain_khr.getImages())
			std::cout << "- 0x" << std::hex << image.vkHandle() << std::dec << "\n";
		std::cout << "Swapchain image view count: " << swapchain_khr.getImageViews().size() << "\n";
		for (const auto& image_view : swapchain_khr.getImageViews())
			std::cout << "- 0x" << std::hex << image_view.vkHandle() << std::dec << "\n";

		auto storage_image = sn::voxeng::vk::Image::Builder()
			.withDevice(device)
			.withImageType(VK_IMAGE_TYPE_2D)
			.withFormat(VK_FORMAT_R8G8B8A8_UNORM)
			.withExtent({ window_description.width, window_description.height, 1u })
			.withMipLevels(1)
			.withArrayLayers(1)
			.withSamples(VK_SAMPLE_COUNT_1_BIT)
			.withTiling(VK_IMAGE_TILING_OPTIMAL)
			.withUsage(VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
			.withSharingMode(VK_SHARING_MODE_EXCLUSIVE)
			.withInitialLayout(VK_IMAGE_LAYOUT_UNDEFINED)
			.sbuild();
		std::cout << "Storage Image 0x " << std::hex << storage_image.vkHandle() << std::dec << "\n";

		auto storage_image_mem_req = storage_image.getMemoryRequirements();
		auto storage_image_mem_type = gpu.findMemoryType(
			storage_image_mem_req.memoryTypeBits,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
		snassert(storage_image_mem_type != sn::voxeng::vk::PhysicalDevice::nmatch,
			"Failed to find requested GPU's memory", "");

		auto storage_image_memory = sn::voxeng::vk::DeviceMemory::Builder()
			.withDevice(device)
			.withAllocationSize(storage_image_mem_req.size)
			.withMemoryTypeIndex(storage_image_mem_type)
			.sbuild();

		storage_image_memory.bindImage(storage_image, 0);

		auto storage_image_view = sn::voxeng::vk::ImageView::Builder()
			.withImage(storage_image)
			.withViewType(VK_IMAGE_VIEW_TYPE_2D)
			.withSubresourceRange({
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
				})
			.sbuild();
		std::cout << "Storage Image View 0x " << std::hex << storage_image_view.vkHandle() << std::dec << "\n";

		auto compiler_settings = sn::voxeng::ShaderCompiler::getSettings();
		compiler_settings.apiVersion = instance.getApiVersion();
		sn::voxeng::ShaderCompiler::setSettings(compiler_settings);
		
		auto compute_shader_spv = sn::voxeng::ShaderCompiler::loadFromFile(".res/shaders/test.comp");
		std::cout << "Shader compiled (" << compute_shader_spv.getSize() << " bytes)\n";

		auto compute_shader = sn::voxeng::vk::ShaderModule::Builder()
			.withDevice(device)
			.withCode(compute_shader_spv)
			.sbuild();

		auto descriptor_set_layout = sn::voxeng::vk::DescriptorSetLayout::Builder()
			.withDevice(device)
			.addBindings(VkDescriptorSetLayoutBinding{
				.binding = 0,
				.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
				.descriptorCount = 1u,
				.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
				.pImmutableSamplers = nullptr,
				})
			.sbuild();

		auto pipeline_layout = sn::voxeng::vk::PipelineLayout::Builder()
			.withDevice(device)
			.addSetLayouts(descriptor_set_layout.vkHandle())
			.sbuild();

		auto compute_pipeline = sn::voxeng::vk::ComputePipeline::Builder()
			.withDevice(device)
			.withLayout(pipeline_layout)
			.withStage(VkPipelineShaderStageCreateInfo{
				.stage = VK_SHADER_STAGE_COMPUTE_BIT,
				.module = compute_shader.vkHandle(),
				.pName = "main",
				})
			.sbuild();
		std::cout << "Compute Pipeline 0x " << std::hex << compute_pipeline.vkHandle() << std::dec << "\n";

		auto descriptor_pool = sn::voxeng::vk::DescriptorPool::Builder()
			.withDevice(device)
			.withMaxSets(1u)
			.addPoolSizes(VkDescriptorPoolSize{
				.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
				.descriptorCount = 1u
				})
			.sbuild();

		auto descripor_sets = sn::voxeng::vk::DescriptorSetsContainer::Builder()
			.withDescriptorPool(descriptor_pool)
			.addSetLayouts(descriptor_set_layout.vkHandle())
			.sbuild();

		auto descriptor_set = descripor_sets.get(0u);
		std::cout << "Descripor Set 0x " << std::hex << descriptor_set.vkHandle() << std::dec << "\n";

		descriptor_set.updateStorageImage(0u, storage_image_view.vkHandle(), VK_IMAGE_LAYOUT_GENERAL);

		// ==========================================================
		constexpr size_t MAX_FRAMES_IN_FLIGHT = 2u;
		const size_t swapchain_image_count = swapchain_khr.getImages().size();
		size_t currentFrame = 0;

		// --- 1. Compute Command Pool & Buffer ---
		auto compute_command_pool = sn::voxeng::vk::CommandPool::Builder()
			.withDevice(device)
			.withQueueFamilyIndex(compute_family_index)
			.withFlags(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)
			.sbuild();
		auto compute_cmdbufs_continer = compute_command_pool.allocateCommandBuffers(MAX_FRAMES_IN_FLIGHT * 1u);

		// --- 2. Graphics Command Pool & Buffer ---
		auto graphics_command_pool = sn::voxeng::vk::CommandPool::Builder()
			.withDevice(device)
			.withQueueFamilyIndex(graphics_family_index)
			.withFlags(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)
			.sbuild();
		auto graphics_cmdbufs_continer = graphics_command_pool.allocateCommandBuffers(MAX_FRAMES_IN_FLIGHT * 1u);

		std::vector<sn::voxeng::vk::CommandBuffer> compute_cmdbufs;
		std::vector<sn::voxeng::vk::CommandBuffer> graphics_cmdbufs;
		compute_cmdbufs.reserve(MAX_FRAMES_IN_FLIGHT * 1u);
		graphics_cmdbufs.reserve(MAX_FRAMES_IN_FLIGHT * 1u);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			compute_cmdbufs.push_back(compute_cmdbufs_continer.get(i * 1u));
			graphics_cmdbufs.push_back(graphics_cmdbufs_continer.get(i * 1u));
		}

		// --- Sync objects ---
		std::vector<sn::voxeng::vk::Semaphore> imageAvailableSemaphores;
		std::vector<sn::voxeng::vk::Semaphore> computeFinishedSemaphores;
		std::vector<sn::voxeng::vk::Fence> inFlightFences;
		imageAvailableSemaphores.reserve(MAX_FRAMES_IN_FLIGHT * 1u);
		computeFinishedSemaphores.reserve(MAX_FRAMES_IN_FLIGHT * 1u);
		inFlightFences.reserve(MAX_FRAMES_IN_FLIGHT * 1u);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			imageAvailableSemaphores.push_back(sn::voxeng::vk::Semaphore::Builder().withDevice(device).sbuild());
			computeFinishedSemaphores.push_back(sn::voxeng::vk::Semaphore::Builder().withDevice(device).sbuild());
			inFlightFences.push_back(sn::voxeng::vk::Fence::Builder()
				.withDevice(device)
				.withFlags(VK_FENCE_CREATE_SIGNALED_BIT)
				.sbuild());
		}

		std::vector<sn::voxeng::vk::Semaphore> renderFinishedSemaphores;
		renderFinishedSemaphores.reserve(swapchain_image_count * 1u);
		for (size_t i = 0; i < swapchain_image_count; ++i)
		{
			renderFinishedSemaphores.push_back(sn::voxeng::vk::Semaphore::Builder().withDevice(device).sbuild());
		}

		// Main cycle
		while (!glfwWindowShouldClose(pWindow))
		{
			auto& imageAvailableSemaphore = imageAvailableSemaphores[currentFrame];
			auto& computeFinishedSemaphore = computeFinishedSemaphores[currentFrame];
			auto& inFlightFence = inFlightFences[currentFrame];

			auto& compute_cmdbuf = compute_cmdbufs[currentFrame];
			auto& graphics_cmdbuf = graphics_cmdbufs[currentFrame];

			currentFrame = (currentFrame + 1u) % MAX_FRAMES_IN_FLIGHT;

			glfwPollEvents();

			// Wait for prev. frame
			inFlightFence.wait();
			inFlightFence.reset();

			// Acquire next image
			uint32_t imageIndex = 0;
			VkResult acquireRes = swapchain_khr.acquireNextImageKHR(
				UINT64_MAX,
				imageAvailableSemaphore,
				VK_NULL_HANDLE,
				&imageIndex
			);
			if (acquireRes == VK_ERROR_OUT_OF_DATE_KHR)
			{
				// TODO: Recreate swapchain
				continue;
			}
			auto& renderFinishedSemaphore = renderFinishedSemaphores[imageIndex];

			VkImage currentSwapchainImage = swapchain_khr.getImages()[imageIndex].vkHandle();

			// =========================================================================
			// 1. COMPUTE COMMAND BUFFER (Выполняется на compute_queue / compute_family)
			// =========================================================================
			compute_cmdbuf.reset();
			compute_cmdbuf.begin({ .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT });

			// --- STEP A: storage_image -> GENERAL ---
			{
				const VkImageMemoryBarrier image_memory_barriers[]{ {
						.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
						.srcAccessMask = 0,
						.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT,
						.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
						.newLayout = VK_IMAGE_LAYOUT_GENERAL,
						.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
						.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
						.image = storage_image.vkHandle(),
						.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
					}
				};
				compute_cmdbuf.cmdPipelineBarrier(
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					0, {}, {}, image_memory_barriers
				);
			}

			// --- STEP B: Dispatch ---
			{
				compute_cmdbuf.cmdBindPipeline(
					VK_PIPELINE_BIND_POINT_COMPUTE,
					compute_pipeline.vkHandle()
				);
				const VkDescriptorSet descriptor_sets[] = { descriptor_set.vkHandle() };
				compute_cmdbuf.cmdBindDescriptorSets(
					VK_PIPELINE_BIND_POINT_COMPUTE,
					compute_pipeline.getLayout(),
					0u, descriptor_sets, {}
				);

				uint32_t group_count_x = (storage_image.getExtent().width + 15u) / 16u;
				uint32_t group_count_y = (storage_image.getExtent().height + 15u) / 16u;
				compute_cmdbuf.cmdDispatch(group_count_x, group_count_y, 1u);
			}

			// --- STEP C1: RELEASE Ownership (Compute -> Graphics) ---
			{
				const VkImageMemoryBarrier release_barriers[]{ {
						.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
						.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT,
						.dstAccessMask = 0, // Игнорируется при RELEASE
						.oldLayout = VK_IMAGE_LAYOUT_GENERAL,
						.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
						.srcQueueFamilyIndex = compute_family_index,  // Отдаем
						.dstQueueFamilyIndex = graphics_family_index, // Графической очереди
						.image = storage_image.vkHandle(),
						.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
					}
				};
				compute_cmdbuf.cmdPipelineBarrier(
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
					0, {}, {}, release_barriers
				);
			}
			compute_cmdbuf.end();

			// =========================================================================
			// 2. GRAPHICS COMMAND BUFFER (Выполняется на graphics_queue / graphics_family)
			// =========================================================================
			graphics_cmdbuf.reset();
			graphics_cmdbuf.begin({ .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT });

			// --- STEP C2: ACQUIRE Ownership & Prep Swapchain ---
			{
				const VkImageMemoryBarrier image_memory_barriers[]{ {
						// ACQUIRE barrier for Storage Image
						.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
						.srcAccessMask = 0, // Игнорируется при ACQUIRE
						.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
						.oldLayout = VK_IMAGE_LAYOUT_GENERAL,
						.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
						.srcQueueFamilyIndex = compute_family_index,
						.dstQueueFamilyIndex = graphics_family_index,
						.image = storage_image.vkHandle(),
						.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
					}, {
						// Swapchain Image barrier
						.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
						.srcAccessMask = 0,
						.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
						.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
						.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
						.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
						.image = currentSwapchainImage,
						.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
					}
				};
				graphics_cmdbuf.cmdPipelineBarrier(
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
					VK_PIPELINE_STAGE_TRANSFER_BIT,
					0, {}, {}, image_memory_barriers
				);
			}

			// --- STEP D: Copy Storage Image -> Swapchain Image ---
			{
				const VkImageCopy copy_regions[]{ {
						.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
						.srcOffset = { 0, 0, 0 },
						.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
						.dstOffset = { 0, 0, 0 },
						.extent = { window_description.width, window_description.height, 1u }
					}
				};
				graphics_cmdbuf.cmdCopyImage(
					storage_image.vkHandle(),
					VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					currentSwapchainImage,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					copy_regions
				);
			}

			// --- STEP E: Swapchain Image -> PRESENT_SRC_KHR ---
			{
				const VkImageMemoryBarrier present_barriers[]{ {
						.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
						.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
						.dstAccessMask = 0,
						.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
						.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
						.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
						.image = currentSwapchainImage,
						.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
					}
				};
				graphics_cmdbuf.cmdPipelineBarrier(
					VK_PIPELINE_STAGE_TRANSFER_BIT,
					VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
					0, {}, {}, present_barriers
				);
			}
			graphics_cmdbuf.end();

			// =========================================================================
			// 3. SUBMITS & PRESENT
			// =========================================================================

			// --- Submit 1: Compute Queue ---
			{
				const VkCommandBuffer c_buffers[] = { compute_cmdbuf.vkHandle() };
				const VkSemaphore c_signals[] = { computeFinishedSemaphore.vkHandle() };

				VkSubmitInfo compute_submit_info{
					.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
					.commandBufferCount = std::size(c_buffers),
					.pCommandBuffers = c_buffers,
					.signalSemaphoreCount = std::size(c_signals),
					.pSignalSemaphores = c_signals,
				};

				if (compute_queue.submit(compute_submit_info, VK_NULL_HANDLE) != VK_SUCCESS)
				{
					std::cerr << "Failed compute submit()\n";
					break;
				}
			}

			// --- Submit 2: Graphics Queue ---
			{
				const VkSemaphore g_waits[] = {
					imageAvailableSemaphore.vkHandle(),
					computeFinishedSemaphore.vkHandle()
				};
				const VkPipelineStageFlags g_wait_stages[] = {
					VK_PIPELINE_STAGE_TRANSFER_BIT,
					VK_PIPELINE_STAGE_TRANSFER_BIT
				};
				const VkCommandBuffer g_buffers[] = { graphics_cmdbuf.vkHandle() };
				const VkSemaphore g_signals[] = { renderFinishedSemaphore.vkHandle() };

				VkSubmitInfo graphics_submit_info{
					.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
					.waitSemaphoreCount = std::size(g_waits),
					.pWaitSemaphores = g_waits,
					.pWaitDstStageMask = g_wait_stages,
					.commandBufferCount = std::size(g_buffers),
					.pCommandBuffers = g_buffers,
					.signalSemaphoreCount = std::size(g_signals),
					.pSignalSemaphores = g_signals,
				};

				if (graphics_queue.submit(graphics_submit_info, inFlightFence) != VK_SUCCESS)
				{
					std::cerr << "Failed graphics submit()\n";
					break;
				}
			}

			// --- STEP G: Present ---
			{
				const VkSemaphore wait_semaphores[] = { renderFinishedSemaphore.vkHandle() };
				const VkSwapchainKHR swapchains[] = { swapchain_khr.vkHandle() };
				VkPresentInfoKHR presentInfo{
					.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
					.waitSemaphoreCount = std::size(wait_semaphores),
					.pWaitSemaphores = wait_semaphores,
					.swapchainCount = std::size(swapchains),
					.pSwapchains = swapchains,
					.pImageIndices = &imageIndex
				};
				if (graphics_queue.presentKHR(presentInfo) != VK_SUCCESS)
				{
					std::cerr << "Failed to presentKHR()\n";
					break;
				}
			}
		}

		device.waitIdle();

		std::cout << "[main()]: OK\n";
	}
	catch (const std::exception& e)
	{
		std::cerr << "[main()]: Error: " << e.what() << std::endl;
		glfwTerminate();
		return 1;
	}

	glfwTerminate();

	return 0;
}