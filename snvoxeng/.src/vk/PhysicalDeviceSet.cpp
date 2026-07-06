#include <snvoxeng/snvoxeng/vk/PhysicalDeviceSet.hpp>

using namespace sn::voxeng::vk::_impl;

PhysicalDeviceSet::PhysicalDeviceSet() = default;
PhysicalDeviceSet::~PhysicalDeviceSet() noexcept = default;

PhysicalDeviceSet::operator bool() const noexcept { return count() > 0u; }
