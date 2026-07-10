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
	cstrs::cstr buf(512);
	at = buf.fill("[", at);
	if ((val & VK_QUEUE_GRAPHICS_BIT) != 0) at = buf.fill("\"GRAPHICS_BIT\", ", at);
	if ((val & VK_QUEUE_COMPUTE_BIT) != 0) at = buf.fill("\"COMPUTE_BIT\", ", at);
	if ((val & VK_QUEUE_TRANSFER_BIT) != 0) at = buf.fill("\"TRANSFER_BIT\", ", at);
	if ((val & VK_QUEUE_SPARSE_BINDING_BIT) != 0) at = buf.fill("\"SPARSE_BINDING_BIT\", ", at);
	if ((val & VK_QUEUE_PROTECTED_BIT) != 0) at = buf.fill("\"PROTECTED_BIT\", ", at);
	if ((val & VK_QUEUE_VIDEO_DECODE_BIT_KHR) != 0) at = buf.fill("\"VIDEO_DECODE_BIT_KHR\", ", at);
	if ((val & VK_QUEUE_VIDEO_ENCODE_BIT_KHR) != 0) at = buf.fill("\"VIDEO_ENCODE_BIT_KHR\", ", at);
	if ((val & VK_QUEUE_OPTICAL_FLOW_BIT_NV) != 0) at = buf.fill("\"OPTICAL_FLOW_BIT_NV\", ", at);
	if ((val & VK_QUEUE_DATA_GRAPH_BIT_ARM) != 0) at = buf.fill("\"DATA_GRAPH_BIT_ARM\", ", at);
	buf.truncate(buf.find_last_of(cstrs::cset("\"[")) + 2u);
	buf.back() = ']';
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
			.addValidationLayers({ "VK_LAYER_KHRONOS_validation" })
			.addExtensions({ "VK_EXT_debug_utils" })
			.addExtensions(instance_extensions)
			.withDebugMessengerEnabled(true)
			.withDebugStream(std::cout)
			.sbuild();

		auto surface_khr = sn::voxeng::vk::SurfaceKHR::Builder()
			.withInstance(instance)
			.withWindowDescription(glfw_get_window_descripton(pWindow))
			.sbuild();

		sn::voxeng::vk::PhysicalDeviceRegistry physical_device_registry(instance);

		sn::voxeng::vk::PhysicalDevice gpu = physical_device_registry.first();
		{
			using namespace sn::voxeng::vk::fPhysicalDeviseSelectors;

			std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
			fExtensions_user_data_t fExtensions_user_data {
				.ppExtensionNames = deviceExtensions.data(),
				.extensionCount = deviceExtensions.size(),
			};
			fQueueSupport_user_data_t fQueueSupport_user_data {
				.requiredFlagsOr = VkQueueFlagBits::VK_QUEUE_COMPUTE_BIT | VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT,
				.requiredFlagsAnd = VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT,
			};
			fSurfaceSupport_user_data_t fSurfaceSupport_user_data {
				.surface = surface_khr.vkHandle(),
			};

			const auto gpus = physical_device_registry
				.pick(fExtensions, &fExtensions_user_data)
				.pick(fQueueSupport, &fQueueSupport_user_data)
				.pick(fSurfaceSupport, &fSurfaceSupport_user_data)
				;

			fDeviceType_user_data_t fDeviceType_user_data {
				.deviceType = VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
			};

			const auto best_gpus = gpus
				.pick(fDeviceType, &fDeviceType_user_data)
				;
			if (best_gpus) gpu = best_gpus.first();
			else gpu = gpus.first();
		}
		std::cout << "GPU: " << gpu.getProperties().deviceName << "\n";

		uint32_t graphics_family;
		uint32_t transfer_family;
		uint32_t compute_family;
		try
		{
			graphics_family = gpu.findQueueFamily({ .flags = VK_QUEUE_GRAPHICS_BIT, .surface = surface_khr.vkHandle() }).value();
			transfer_family = gpu.findQueueFamily({ .flags = VK_QUEUE_TRANSFER_BIT, .preferDedicated = true }).value();
			compute_family = gpu.findQueueFamily({ .flags = VK_QUEUE_COMPUTE_BIT,  .preferDedicated = true }).value();
		}
		catch (...)
		{
			throw std::runtime_error("Failed to find requested GPU's queue family.");
		}
		std::cout 
			<< "GPU's graphics_family: " << graphics_family 
			<< " " << gpu.getQueueFamilyProperties()[graphics_family] << "\n";
		std::cout 
			<< "GPU's transfer_family: " << transfer_family
			<< " " << gpu.getQueueFamilyProperties()[transfer_family] << "\n";
		std::cout 
			<< "GPU's compute_family: " << compute_family
			<< " " << gpu.getQueueFamilyProperties()[compute_family] << "\n";

		auto device = sn::voxeng::vk::Device::Builder()
			.withPhysicalDevice(gpu)
			.withQueueRequests({
				{ .name = "graphics", .familyIndex = graphics_family, .priority = 1.0f },
				{ .name = "compute",  .familyIndex = compute_family,  .priority = 0.8f },
				{ .name = "transfer", .familyIndex = transfer_family, .priority = 0.5f },
				})
			.withExtensions({ VK_KHR_SWAPCHAIN_EXTENSION_NAME })
			.withPhysicalDevice13Features({ .dynamicRendering = VK_TRUE })
			.sbuild();

		VkQueue mainGraphics = device.getQueueInfo("graphics")->handle;
		VkQueue asyncCompute = device.getQueueInfo("compute")->handle;
		VkQueue asyncTransfer = device.getQueueInfo("transfer")->handle;

		std::cout << "mainGraphics VkQueue 0x" << std::hex << mainGraphics << std::dec << "\n";
		std::cout << "asyncCompute VkQueue 0x" << std::hex << asyncCompute << std::dec << "\n";
		std::cout << "asyncTransfer VkQueue 0x" << std::hex << asyncTransfer << std::dec << "\n";

		auto swapchain_khr = sn::voxeng::vk::SwapchainKHR::Builder()
			.withDevice(device)
			.withSurfaceKHR(surface_khr)
			.addQueueFamilyIndices(device.getQueueInfo("graphics")->index)
			.sbuild();

		std::cout << "VkSwapchainKHR: 0x" << std::hex << swapchain_khr.vkHandle() << std::dec << "\n";
		std::cout << "Swapchain image count: " << swapchain_khr.getImages().size() << "\n";
		for (const auto& image : swapchain_khr.getImages())
			std::cout << "- 0x" << std::hex << image.vkHandle() << std::dec << "\n";
		std::cout << "Swapchain image view count: " << swapchain_khr.getImageViews().size() << "\n";
		for (const auto& image_view : swapchain_khr.getImageViews())
			std::cout << "- 0x" << std::hex << image_view.vkHandle() << std::dec << "\n";

		auto command_pool = sn::voxeng::vk::CommandPool::Builder()
			.withDevice(device)
			.withQueueFamilyIndex(device.getQueueInfo("compute")->index)
			.withFlags(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)
			.sbuild();
		std::cout << "VkCommandPool (Compute) 0x" << std::hex << command_pool.vkHandle() << std::dec << "\n";

		auto command_buffers = command_pool.allocateCommandBuffers(1u);
		std::cout << "Command buffers (Compute) count: " << command_buffers.count() << "\n";
		for (size_t i = 0; i < command_buffers.count(); ++i)
		{
			auto buf = command_buffers.get(i);
			std::cout << "Command buffer " << buf.getContainerIdx() << " (Compute): 0x" << std::hex << buf.vkHandle() << std::dec << "\n";
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