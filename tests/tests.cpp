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
			.build();

		auto surface_khr = sn::voxeng::vk::SurfaceKHR::Builder()
			.withInstance(instance)
			.withWindowDescription(window_description)
			.build();

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
			.build();

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

		auto compiler_settings = sn::voxeng::ShaderCompiler::getSettings();
		compiler_settings.apiVersion = instance.getApiVersion();
		sn::voxeng::ShaderCompiler::setSettings(compiler_settings);
		
		auto compute_shader_spv = sn::voxeng::ShaderCompiler::loadFromFile(".res/shaders/test.comp");
		std::cout << "Shader compiled (" << compute_shader_spv.getSize() << " bytes)\n";

		auto compute_shader = sn::voxeng::vk::ShaderModule::Builder()
			.withDevice(device)
			.withCode(compute_shader_spv)
			.build();

		auto descriptor_set_layout = sn::voxeng::vk::DescriptorSetLayout::Builder()
			.withDevice(device)
			.addBindings(VkDescriptorSetLayoutBinding{
				.binding = 0,
				.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
				.descriptorCount = 1u,
				.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
				.pImmutableSamplers = nullptr,
				})
			.build();

		static constexpr float PI = 3.14159265f;
		static constexpr float D2R = PI / 180.f;
		static constexpr float R2D = 180.f / PI;
		struct PushConstants
		{
			sn::voxeng::math::vec4 cameraPos;  // xyz position, w fovy in rads
			sn::voxeng::math::vec4 cameraRot;  // xyz rotation Euler zyx in rads
			sn::voxeng::math::vec2 resolution; // px
			float time;                        // seconds
			uint32_t frameCount;
		} push_constants{
			.cameraPos{ 0.f, 0.f, 0.f, 060.f * D2R },
			.cameraRot{ 0.f * D2R, 0.f * D2R, 0.f * D2R },
			.resolution{ 0u, 0u },
			.time{ 0.f },
			.frameCount{ 0u },
		};

		auto pipeline_layout = sn::voxeng::vk::PipelineLayout::Builder()
			.withDevice(device)
			.addSetLayouts(descriptor_set_layout.vkHandle())
			.addPushConstantRanges(VkPushConstantRange{
					.stageFlags{ VK_SHADER_STAGE_COMPUTE_BIT },
					.offset{ 0 },
					.size{ sizeof(PushConstants) },
				})
			.build();

		auto compute_pipeline = sn::voxeng::vk::ComputePipeline::Builder()
			.withDevice(device)
			.withLayout(pipeline_layout)
			.withStage(VkPipelineShaderStageCreateInfo{
				.stage = VK_SHADER_STAGE_COMPUTE_BIT,
				.module = compute_shader.vkHandle(),
				.pName = "main",
				})
			.build();
		std::cout << "Compute Pipeline 0x " << std::hex << compute_pipeline.vkHandle() << std::dec << "\n";

		sn::voxeng::Renderer renderer(
			device, surface_khr,
			graphics_family_index, compute_family_index,
			graphics_queue_index, compute_queue_index);
		push_constants.resolution = {
			static_cast<float>(renderer.getCanvasImage().getExtent().width),
			static_cast<float>(renderer.getCanvasImage().getExtent().height),
		};

		auto descriptor_pool = sn::voxeng::vk::DescriptorPool::Builder()
			.withDevice(device)
			.withMaxSets(1u)
			.addPoolSizes(VkDescriptorPoolSize{
				.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
				.descriptorCount = 1u
				})
			.build();

		auto descripor_sets = sn::voxeng::vk::DescriptorSetsContainer::Builder()
			.withDescriptorPool(descriptor_pool)
			.addSetLayouts(descriptor_set_layout.vkHandle())
			.build();

		auto descriptor_set = descripor_sets.get(0u);
		descriptor_set.updateStorageImage(0u, renderer.getCanvasImageView().vkHandle(), VK_IMAGE_LAYOUT_GENERAL);

		// Main cycle
		bool is_swapchain_recreate_needeed{ false };
		while (!glfwWindowShouldClose(pWindow))
		{
			glfwPollEvents();
			if (is_swapchain_recreate_needeed)
			{
				if (renderer.recreateSwapchainKHR())
				{
					is_swapchain_recreate_needeed = false;
					descriptor_set.updateStorageImage(0u, renderer.getCanvasImageView().vkHandle(), VK_IMAGE_LAYOUT_GENERAL);
					push_constants.resolution = {
						static_cast<float>(renderer.getCanvasImage().getExtent().width),
						static_cast<float>(renderer.getCanvasImage().getExtent().height),
					};
				}
				else continue;
			}

			auto frame_context = renderer.beginFrame();
			if (!frame_context.has_value())
			{
				is_swapchain_recreate_needeed = true;
				continue;
			}

			{
				const VkImageMemoryBarrier acquire_barriers[]{ {
						.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
						.srcAccessMask = 0,
						.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT,
						.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
						.newLayout = VK_IMAGE_LAYOUT_GENERAL,
						.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
						.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
						.image = renderer.getCanvasImage().vkHandle(),
						.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
					}
				};
				frame_context->computeCmd.cmdPipelineBarrier(
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					0, {}, {}, acquire_barriers
				);
			}
			{
				frame_context->computeCmd.cmdBindPipeline(
					VK_PIPELINE_BIND_POINT_COMPUTE,
					compute_pipeline.vkHandle()
				);
				const VkDescriptorSet descriptor_sets[] = { descriptor_set.vkHandle() };
				frame_context->computeCmd.cmdBindDescriptorSets(
					VK_PIPELINE_BIND_POINT_COMPUTE,
					compute_pipeline.getLayout(),
					0u, descriptor_sets, {}
				);

				frame_context->computeCmd.cmdPushConstants(
					compute_pipeline.getLayout(), VK_SHADER_STAGE_COMPUTE_BIT,
					0, sizeof(PushConstants),
					&push_constants
				);
			
				uint32_t group_count_x = (renderer.getCanvasImage().getExtent().width + 15u) / 16u;
				uint32_t group_count_y = (renderer.getCanvasImage().getExtent().height + 15u) / 16u;
				frame_context->computeCmd.cmdDispatch(group_count_x, group_count_y, 1u);
			}
			{
				const VkImageMemoryBarrier release_barriers[]{ {
						.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
						.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT,
						.dstAccessMask = 0,
						.oldLayout = VK_IMAGE_LAYOUT_GENERAL,
						.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
						.srcQueueFamilyIndex = compute_family_index,
						.dstQueueFamilyIndex = graphics_family_index,
						.image = renderer.getCanvasImage().vkHandle(),
						.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
					}
				};

				frame_context->computeCmd.cmdPipelineBarrier(
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
					0, {}, {}, release_barriers
				);
			}

			if (!renderer.endFrame(frame_context->imageIndex))
			{
				is_swapchain_recreate_needeed = true;
				continue;
			}

			push_constants.frameCount++;
			push_constants.time += 1.f / 60.f;

			float r = 8.f;
			push_constants.cameraPos.x = r * std::sin(push_constants.time);
			push_constants.cameraPos.y = 1.5f;
			push_constants.cameraPos.z = r * -std::cos(push_constants.time);

			// VV
			push_constants.cameraRot.x = 10.f * D2R;
			// <<
			push_constants.cameraRot.y = -push_constants.time;
			// (_>
			push_constants.cameraRot.z = 0.f;
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