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
#endif

#ifdef SNBCG_DEFAULT_VALUES
// --- v Defaults v ---
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
	uint32_t, uint32_t,
	vkCreateInfo., queueFamilyIndex, QueueFamilyIndex,
	SNBCG_POLICY_RETURN_COPY, SNBCG_POLICY_STORE_COPY
)
#undef SNBCG_THIS
#endif

#ifdef SNBCG_OPTIONAL
#define SNBCG_THIS SNBCG_OPTIONAL
// --- v Optional Fields v ---
SNBCG_THIS(const void*, const void*, vkCreateInfo., pNext, Next, SNBCG_POLICY_RETURN_COPY, SNBCG_POLICY_STORE_COPY)
SNBCG_THIS(VkCommandPoolCreateFlags, VkCommandPoolCreateFlags, vkCreateInfo., flags, Flags, SNBCG_POLICY_RETURN_COPY, SNBCG_POLICY_STORE_COPY)
SNBCG_THIS(const VkAllocationCallbacks*, const VkAllocationCallbacks*, , vkPAllocator, Allocator, SNBCG_POLICY_RETURN_COPY, SNBCG_POLICY_STORE_COPY)
#undef SNBCG_THIS
#endif

// --- V -------------- V ---
// === V  ARRAY FIELDS  V ===
// --- V -------------- V ---

#ifdef SNBCG_REQUIRED_ADDITIVE
#define SNBCG_THIS SNBCG_REQUIRED_ADDITIVE
// --- v Required Fields v ---
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
