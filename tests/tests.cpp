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

		VkDeviceSize image_size = storage_image.getExtent().width * storage_image.getExtent().height * 4u;
		auto staging_buffer = sn::voxeng::vk::Buffer::Builder()
			.withDevice(device)
			.withSize(image_size)
			.withUsage(VK_BUFFER_USAGE_TRANSFER_DST_BIT)
			.withSharingMode(VK_SHARING_MODE_EXCLUSIVE)
			.addQueueFamilyIndices(transfer_family_index)
			.sbuild();

		auto staging_buffer_mem_req = staging_buffer.getMemoryRequirements();
		auto staging_buffer_mem_type = gpu.findMemoryType(
			staging_buffer_mem_req.memoryTypeBits,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);
		snassert(staging_buffer_mem_type != sn::voxeng::vk::PhysicalDevice::nmatch,
			"Failed to find requested GPU's memory", "");

		auto staging_buffer_memory = sn::voxeng::vk::DeviceMemory::Builder()
			.withDevice(device)
			.withAllocationSize(staging_buffer_mem_req.size)
			.withMemoryTypeIndex(staging_buffer_mem_type)
			.sbuild();
		staging_buffer_memory.bindBuffer(staging_buffer, 0);

		auto command_pool = sn::voxeng::vk::CommandPool::Builder()
			.withDevice(device)
			.withQueueFamilyIndex(compute_family_index)
			.withFlags(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)
			.sbuild();
		auto command_buffers = command_pool.allocateCommandBuffers(1u);
		auto storage_image_cmdbuf = command_buffers.get(0);
		storage_image_cmdbuf.begin({ .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT });

		// STEP 1: Convert the image to GENERAL for shader recording
		{
			VkImageMemoryBarrier begin_barrier{
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				.srcAccessMask = 0,
				.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT,
				.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.newLayout = VK_IMAGE_LAYOUT_GENERAL,
				.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.image = storage_image.vkHandle(),
				.subresourceRange = {
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.baseMipLevel = 0,
					.levelCount = 1u,
					.baseArrayLayer = 0,
					.layerCount = 1u
				}
			};
			storage_image_cmdbuf.cmdPipelineBarrier(
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				0,
				{},
				{},
				{ &begin_barrier, 1u }
			);
		}

		// STEP 2: Bind the pipeline and descriptors, and FIRE
		{
			storage_image_cmdbuf.cmdBindPipeline(VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipeline.vkHandle());
			storage_image_cmdbuf.cmdBindDescriptorSets(
				VK_PIPELINE_BIND_POINT_COMPUTE,
				compute_pipeline.getLayout(),
				0u,
				std::vector<VkDescriptorSet>{ descriptor_set.vkHandle() },
				{}
			);

			uint32_t group_count_x = (storage_image.getExtent().width + 15u) / 16u;
			uint32_t group_count_y = (storage_image.getExtent().height + 15u) / 16u;
			uint32_t group_count_z = 1u;

			storage_image_cmdbuf.cmdDispatch(group_count_x, group_count_y, group_count_z);
		}

		// STEP 3: Wait for the Compute operation to complete and transition to TRANSFER_SRC
		{
			VkImageMemoryBarrier copy_barrier{
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT,        // We're waiting for Dispatch to finish writing.
				.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,       // Preparing for reading and copying
				.oldLayout = VK_IMAGE_LAYOUT_GENERAL,
				.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.image = storage_image.vkHandle(),
				.subresourceRange = {
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.baseMipLevel = 0,
					.levelCount = 1u,
					.baseArrayLayer = 0,
					.layerCount = 1u
				}
			};
			storage_image_cmdbuf.cmdPipelineBarrier(
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,               // Src - Compute
				VK_PIPELINE_STAGE_TRANSFER_BIT,                     // Dst - Transfer
				0,
				{},
				{},
				{ &copy_barrier, 1u }
			);
		}

		// STEP 4: Copy the already FILLED image to the Staging Buffer
		{
			VkBufferImageCopy region{
				.bufferOffset = 0,
				.bufferRowLength = 0,
				.bufferImageHeight = 0,
				.imageSubresource = {
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.mipLevel = 0,
					.baseArrayLayer = 0,
					.layerCount = 1
				},
				.imageOffset = { 0, 0, 0 },
				.imageExtent = { window_description.width, window_description.height, 1u }
			};
			storage_image_cmdbuf.cmdCopyImageToBuffer(
				storage_image.vkHandle(),
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				staging_buffer.vkHandle(),
				{ &region, 1u }
			);
		}
		storage_image_cmdbuf.end();

		// STEP 5: Submit & Wait Result
		{
			auto cmdbuf_handle = storage_image_cmdbuf.vkHandle();
			VkSubmitInfo submit_info{
				.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
				.pNext = nullptr,
				.waitSemaphoreCount = 0,
				.pWaitSemaphores = nullptr,
				.pWaitDstStageMask = nullptr,
				.commandBufferCount = 1u,
				.pCommandBuffers = &cmdbuf_handle,
				.signalSemaphoreCount = 0,
				.pSignalSemaphores = nullptr
			};

			compute_queue.submit(submit_info, VK_NULL_HANDLE);
			compute_queue.waitIdle();
		}

		// STEP 6: Get & Save Result
		{
			void* data{ nullptr };
			staging_buffer_memory.map(0, image_size, 0, &data);

			if (data)
			{
				std::filesystem::path output_path(".out/render_result.tga");
				std::filesystem::create_directories(output_path.parent_path());

				std::ofstream file(output_path, std::ios::binary);
				if (!file.is_open())
					throw std::runtime_error("Failed to open or create output file at: " + output_path.string());

				uint8_t header[18] = { 0 };
				header[2] = 2; // Uncompressed true-color image
				header[12] = storage_image.getExtent().width & 0xFF;
				header[13] = (storage_image.getExtent().width >> 8) & 0xFF;
				header[14] = storage_image.getExtent().height & 0xFF;
				header[15] = (storage_image.getExtent().height >> 8) & 0xFF;
				header[16] = 32; // 32 bpp (RGBA)

				file.write(reinterpret_cast<char*>(header), sizeof(header));

				// Vulkan uses RGBA, while TGA expects BGRA.
				// Solved via shader color conversion
				file.write(reinterpret_cast<char*>(data), image_size);
				file.close();

				staging_buffer_memory.unmap();

				std::cout << "Image saved to render_result.tga" << std::endl;
			}
		}

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