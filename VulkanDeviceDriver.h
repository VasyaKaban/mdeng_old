#pragma once

#include <memory>
#include "VulkanInclude.h"
#include "utils/ResultDef.hpp"

class VulkanDeviceDriver
{
public:
	VulkanDeviceDriver() = default;
	virtual ~VulkanDeviceDriver() = default;

	//virtual auto init(vk::PhysicalDevice &ph_dev) -> WarningLevel = 0;

    //virtual auto is_inited() -> bool = 0;
};

template<typename T_DEVICE_DRIVER>
concept VulkanDeviceDriverExt =
    std::derived_from<T_DEVICE_DRIVER, VulkanDeviceDriver> &&
    requires(T_DEVICE_DRIVER &&dev, vk::PhysicalDevice &ph_dev)
{
    {hrs::ResultType<typename T_DEVICE_DRIVER::Result>};
    {dev.init(ph_dev)} -> std::convertible_to<hrs::ResultDef<typename T_DEVICE_DRIVER::Result>>;
    {dev.is_inited()} -> std::same_as<bool>;
};


template<typename T_DEVICE_DRIVER>
concept VulkanDeviceDriverSurfaceConnectionExt =
    std::derived_from<T_DEVICE_DRIVER, VulkanDeviceDriver> &&
    requires(T_DEVICE_DRIVER &&dev, std::weak_ptr<vk::SurfaceKHR> sur)
    {
        {dev.connect_surface(sur)} -> std::convertible_to<hrs::ResultDef<typename T_DEVICE_DRIVER::Result>>;
        {dev.is_surface_connected()} -> std::same_as<bool>;
    };
