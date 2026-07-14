/*
* ======= MACRO SYNTAX =======
* > For single fields:
*   [1] field store type                       [2] arg type
*   [3] subdata name (optional, ends with '.') [4] name (snake_case) [5] Name (PascalCase)
*   [6] return policy
*   [7] store policy
*
* > For array fields:
*   [1] field store type                       [2] arg type          [3]+args type
*   [4] subdata name (optional, ends with '.') [5] name (snake_case) [6] Name (PascalCase)
*   [7] return policy
*   [8] store policy
*   [9]+store action
*
* ======= POLICIES & ACTIONS =======
* > Policies:
*    * SNBCG_POLICY_RETURN_CREF -> const   store_t&
* 	 * SNBCG_POLICY_RETURN_COPY ->         store_t
* 	 * SNBCG_POLICY_RETURN_UNPTR-> const (*store_t)&
* 	 * SNBCG_POLICY_RETURN_PTR  -> const (&store_t)
* 	 * SNBCG_POLICY_RETURN_VIEW -> std::string_view
*
*    * SNBCG_POLICY_STORE_COPY  ->           arg
*    * SNBCG_POLICY_STORE_MOVE  -> std::move(arg)
*    * SNBCG_POLICY_STORE_ADDR  ->          &arg
*    * SNBCG_POLICY_STORE_STATIC_CAST
*    * SNBCG_POLICY_STORE_REINTERPRET_CAST
*    * SNBCG_POLICY_STORE_CONST_CAST
*    * SNBCG_POLICY_STORE_DYNAMIC_CAST
*
* > Actions (SNBCG_ACTION_...) describe how additional data will affect an array field:
*    * APPEND_EMPLACE          ->           emplace_back(arg)  | .insert(multifield.end(), args.begin(), args.end())
* 	 * APPEND_PUSH             ->              push_back(arg)  | .insert(multifield.end(), args.begin(), args.end())
* 	 * APPEND_EMPLACE_MOVE     -> emplace_back(std::move(arg)) | std::move(args.begin(), args.end(), std::back_inserter(multifield))
* 	 * APPEND_PUSH_MOVE        ->    push_back(std::move(arg)) | std::move(args.begin(), args.end(), std::back_inserter(multifield))
*/

// --- V -------------- V ---
// === V      MISC      V ===
// --- V -------------- V ---

#ifdef SNBCG_HEADER_INCLUDE
// --- v Includes v ---
#include <snvoxeng/snvoxeng/vk/Device.hpp>
#include <snvoxeng/snvoxeng/vk/SurfaceKHR.hpp>
#endif

/*
VkStructureType                  sType;
const void*                      pNext;
VkSwapchainCreateFlagsKHR        flags;

VkSurfaceKHR                     surface;

uint32_t                         minImageCount;

VkFormat                         imageFormat;
VkColorSpaceKHR                  imageColorSpace;

VkExtent2D                       imageExtent;

uint32_t                         imageArrayLayers;
VkImageUsageFlags                imageUsage;
VkSharingMode                    imageSharingMode;

uint32_t                         queueFamilyIndexCount;
const uint32_t*                  pQueueFamilyIndices;

VkSurfaceTransformFlagBitsKHR    preTransform;

VkCompositeAlphaFlagBitsKHR      compositeAlpha;
VkPresentModeKHR                 presentMode;
VkBool32                         clipped;
VkSwapchainKHR                   oldSwapchain;
*/

#ifdef SNBCG_DEFAULT_VALUES
// --- v Defaults v ---
static constexpr auto imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
static constexpr auto imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
static constexpr auto imageArrayLayers = 1u;
static constexpr auto imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
static constexpr auto imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
static constexpr auto compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
static constexpr auto presentMode = VK_PRESENT_MODE_FIFO_KHR;
static constexpr auto clipped = VK_FALSE;
static constexpr auto oldSwapchain = VK_NULL_HANDLE;

static constexpr auto pNext = nullptr;
static constexpr auto flags = 0u;
static constexpr auto vkPAllocator = nullptr;
#endif

// --- V -------------- V ---
// === V SINGLE  FIELDS V ===
// --- V -------------- V ---

#ifdef SNBCG_REQUIRED
#define SNBCG_THIS SNBCG_REQUIRED
// --- v Required Fields v ---
SNBCG_THIS(
	const Device*, const Device&,
	, pDevice, Device,
	SNBCG_POLICY_RETURN_UNPTR, SNBCG_POLICY_STORE_ADDR
)
SNBCG_THIS(
	const SurfaceKHR*, const SurfaceKHR&,
	, pSurfaceKHR, SurfaceKHR,
	SNBCG_POLICY_RETURN_UNPTR, SNBCG_POLICY_STORE_ADDR
)
#undef SNBCG_THIS
#endif

#ifdef SNBCG_OPTIONAL
#define SNBCG_THIS SNBCG_OPTIONAL
// --- v Optional Fields v ---
SNBCG_THIS(
	VkFormat, VkFormat,
	vkCreateInfo., imageFormat, ImageFormat,
	SNBCG_POLICY_RETURN_COPY, SNBCG_POLICY_STORE_COPY
)
SNBCG_THIS(
	VkColorSpaceKHR, VkColorSpaceKHR,
	vkCreateInfo., imageColorSpace, ImageColorSpace,
	SNBCG_POLICY_RETURN_COPY, SNBCG_POLICY_STORE_COPY
)
SNBCG_THIS(
	uint32_t, uint32_t,
	vkCreateInfo., imageArrayLayers, ImageArrayLayers,
	SNBCG_POLICY_RETURN_COPY, SNBCG_POLICY_STORE_COPY
)
SNBCG_THIS(
	VkImageUsageFlags, VkImageUsageFlags,
	vkCreateInfo., imageUsage, ImageUsage,
	SNBCG_POLICY_RETURN_COPY, SNBCG_POLICY_STORE_COPY
)
SNBCG_THIS(
	VkSharingMode, VkSharingMode,
	vkCreateInfo., imageSharingMode, ImageSharingMode,
	SNBCG_POLICY_RETURN_COPY, SNBCG_POLICY_STORE_COPY
)
SNBCG_THIS(
	VkCompositeAlphaFlagBitsKHR, VkCompositeAlphaFlagBitsKHR,
	vkCreateInfo., compositeAlpha, CompositeAlpha,
	SNBCG_POLICY_RETURN_COPY, SNBCG_POLICY_STORE_COPY
)
SNBCG_THIS(
	VkPresentModeKHR, VkPresentModeKHR,
	vkCreateInfo., presentMode, PresentMode,
	SNBCG_POLICY_RETURN_COPY, SNBCG_POLICY_STORE_COPY
)
SNBCG_THIS(
	VkBool32, VkBool32,
	vkCreateInfo., clipped, Clipped,
	SNBCG_POLICY_RETURN_COPY, SNBCG_POLICY_STORE_COPY
)
SNBCG_THIS(
	VkSwapchainKHR, VkSwapchainKHR,
	vkCreateInfo., oldSwapchain, OldSwapchain,
	SNBCG_POLICY_RETURN_COPY, SNBCG_POLICY_STORE_COPY
)

SNBCG_THIS(const void*, const void*, vkCreateInfo., pNext, Next, SNBCG_POLICY_RETURN_COPY, SNBCG_POLICY_STORE_COPY)
SNBCG_THIS(VkSwapchainCreateFlagsKHR, VkSwapchainCreateFlagsKHR, vkCreateInfo., flags, Flags, SNBCG_POLICY_RETURN_COPY, SNBCG_POLICY_STORE_COPY)
SNBCG_THIS(const VkAllocationCallbacks*, const VkAllocationCallbacks*, , vkPAllocator, Allocator, SNBCG_POLICY_RETURN_COPY, SNBCG_POLICY_STORE_COPY)
#undef SNBCG_THIS
#endif

// --- V -------------- V ---
// === V  ARRAY FIELDS  V ===
// --- V -------------- V ---

#ifdef SNBCG_REQUIRED_ADDITIVE
#define SNBCG_THIS SNBCG_REQUIRED_ADDITIVE
// --- v Required Fields v ---
SNBCG_THIS(
	std::vector<uint32_t>, uint32_t, std::span<uint32_t>,
	, queueFamilyIndices, QueueFamilyIndices,
	SNBCG_POLICY_RETURN_SPAN, SNBCG_POLICY_STORE_SPAN_COPY, SNBCG_ACTION_APPEND_EMPLACE
)
#undef SNBCG_THIS
#endif

#ifdef SNBCG_OPTIONAL_ADDITIVE
#define SNBCG_THIS SNBCG_OPTIONAL_ADDITIVE
// --- v Optional Fields v ---
#undef SNBCG_THIS
#endif

// --- V -------------- V ---
// === V      MISC      V ===
// --- V -------------- V ---

#ifdef SNBCG_HEADER_INCLUDE
#undef SNBCG_HEADER_INCLUDE
#endif
#ifdef SNBCG_DEFAULT_VALUES
#undef SNBCG_DEFAULT_VALUES
#endif
#ifdef SNBCG_REQUIRED
#undef SNBCG_REQUIRED
#endif
#ifdef SNBCG_OPTIONAL
#undef SNBCG_OPTIONAL
#endif
#ifdef SNBCG_REQUIRED_ADDITIVE
#undef SNBCG_REQUIRED_ADDITIVE
#endif
#ifdef SNBCG_OPTIONAL_ADDITIVE
#undef SNBCG_OPTIONAL_ADDITIVE
#endif
