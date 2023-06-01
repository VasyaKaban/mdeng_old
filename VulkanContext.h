#pragma once

//#include "VulkanInclude.h"
#include <vector>
#include <memory>
#include <concepts>
#include <span>
#include <optional>
#include "utils/expected.hpp"
#include "VulkanDeviceDriver.h"
#include "utils/ResultDef.hpp"
#include "utils/ControlBlock.hpp"

class VulkanContext;

class VulkanContext
{
public:
	struct Result
    {
        enum class error_code : uint8_t
        {
            //common
            Success,

			//Vulkan
			InnerVulkanError,

			//Instance
			ExtensionOrLayerNotSupported,
			InstanceNotCreated,

			//Devices
			PhysicalDeviceNotExist,
			SurfaceNotCreated,
			DeviceDriverNotExist,

			//WindowSurfaceOuter
			//It's can either success or error
			SurfaceHandshakeResult,
			SurfaceConnectionToDevice

        } code;

		vk::Result vulkan_res;

		constexpr Result(error_code err) : code(err)
        {}

        constexpr Result(vk::Result res = vk::Result::eSuccess) : code(error_code::InnerVulkanError), vulkan_res(res)
        {}

        constexpr auto message() const -> std::string_view;
        constexpr auto to_view() const -> std::string_view;

        constexpr auto operator=(const Result::error_code err) -> Result &;
		constexpr auto operator=(const vk::Result res) -> Result &;

        constexpr friend auto operator==(const Result &res, const Result::error_code &err_code) -> bool;
    };

    static_assert(hrs::ResultType<Result>);

	struct vulkan_device_driver_deleter
	{
		auto operator()(hrs::ControlBlock<VulkanDeviceDriver> *ptr) const -> void
		{
			if(ptr != nullptr)
			{
				delete ptr->data;
				ptr->data = nullptr;

				if(ptr->spectator_count == 1)
					delete ptr;
			}
		}
	};
private:
	vk::Instance instance;
	std::shared_ptr<vk::SurfaceKHR> surface;
	std::vector<vk::PhysicalDevice> physical_devices;
	std::vector<std::unique_ptr<hrs::ControlBlock<VulkanDeviceDriver>, vulkan_device_driver_deleter>> device_drivers;
public:
	VulkanContext();
	VulkanContext(const VulkanContext &ctx) = delete;
	VulkanContext(VulkanContext &&ctx) noexcept;
	~VulkanContext();
	auto init(const std::vector<const char *> &layers, const std::vector<const char *> &extensions,
			  const std::string &application_name = "application_placeholder_name", uint32_t application_version = 1,
			  const std::string &engine_name = "engine_placeholder_name", uint32_t engine_version = 1) -> hrs::ResultDef<Result>;

	auto operator=(const VulkanContext &ctx) -> VulkanContext & = delete;

	auto GetPhysicalDevicesCount() -> size_t;
	auto GetPhysicalDevice(size_t ind) -> std::optional<vk::PhysicalDevice>;
	auto GetPhysicalDevices() -> std::vector<vk::PhysicalDevice>;
	auto DropDeviceDriver(hrs::SpectatorPtr<VulkanDeviceDriver> dev) -> void;

	//auto InitDeviceDriver(hrs::SpectatorPtr<VulkanDeviceDriver> dev) -> Result;

	template<typename WIN_T>
	requires
		requires(WIN_T &&w, vk::Instance &ins, std::shared_ptr<vk::SurfaceKHR> &shared_surface)
		{
			{hrs::ResultType<typename WIN_T::Result>};
			{w.vulkan_surface_handshake(ins, shared_surface)} -> std::convertible_to<hrs::ResultDef<typename WIN_T::Result>>;
		}
	auto WindowContextHandshake(WIN_T &win); //-> std::pair<Result, hrs::ResultDef<typename WIN_T::Result>>;


	template<typename T_DEVICE_DRIVER>
	requires
		std::derived_from<T_DEVICE_DRIVER, VulkanDeviceDriver>
	auto AllocateDevice(const vk::PhysicalDevice &ph_dev) -> hrs::expected<hrs::SpectatorPtr<VulkanDeviceDriver>, Result>;

	template<VulkanDeviceDriverSurfaceConnectionExt T_DEVICE_DRIVER>
	auto BindSurface(hrs::SpectatorPtr<VulkanDeviceDriver> dev); //-> Result;

};

constexpr auto VulkanContext::Result::message() const -> std::string_view
{
	std::string_view res;
    switch(code)
    {
        case Result::error_code::Success:
            res = "Vulkan context successfull operation";
            break;
        case Result::error_code::InnerVulkanError:
            res = vk::to_string(vulkan_res);
            break;
        case Result::error_code::ExtensionOrLayerNotSupported:
            res = "Extension or layer is not supported by implementation";
            break;
		case Result::error_code::InstanceNotCreated:
            res = "Instance isn't created yet";
            break;
		case Result::error_code::PhysicalDeviceNotExist:
            res = "Physical device isn't exist";
            break;
		case Result::error_code::SurfaceNotCreated:
            res = "Surface isn't created";
            break;
		case Result::error_code::DeviceDriverNotExist:
            res = "Device driver ins't exist";
            break;
		case Result::error_code::SurfaceHandshakeResult:
            res = "Surface handshaking with window result";
            break;
		case Result::error_code::SurfaceConnectionToDevice:
            res = "Surface connection to device result";
            break;
    }

    return res;
}
constexpr auto VulkanContext::Result::to_view() const -> std::string_view
{
	std::string_view res;
    switch(code)
    {
        case Result::error_code::Success:
            res = "Success";
            break;
        case Result::error_code::InnerVulkanError:
            res = vk::to_string(vulkan_res);
            break;
        case Result::error_code::ExtensionOrLayerNotSupported:
            res = "ExtensionOrLayerNotSupported";
            break;
		case Result::error_code::InstanceNotCreated:
            res = "InstanceNotCreated";
            break;
		case Result::error_code::PhysicalDeviceNotExist:
            res = "PhysicalDeviceNotExist";
            break;
		case Result::error_code::SurfaceNotCreated:
            res = "SurfaceNotCreated";
            break;
		case Result::error_code::DeviceDriverNotExist:
            res = "DeviceDriverNotExist";
            break;
		case Result::error_code::SurfaceHandshakeResult:
            res = "SurfaceHandshakeResult";
            break;
		case Result::error_code::SurfaceConnectionToDevice:
            res = "SurfaceConnectionToDevice";
            break;
    }

    return res;
}

constexpr auto VulkanContext::Result::operator=(const Result::error_code err) -> Result &
{
	code = err;
    return *this;
}

constexpr auto VulkanContext::Result::operator=(const vk::Result res) -> Result &
{
	code = VulkanContext::Result::error_code::InnerVulkanError;
	vulkan_res = res;
	return *this;
}

constexpr auto operator==(const VulkanContext::Result &res, const VulkanContext::Result::error_code &err_code) -> bool
{
	return res.code == err_code;
}

template<typename WIN_T>
requires
	requires(WIN_T &&w, vk::Instance &ins, std::shared_ptr<vk::SurfaceKHR> &shared_surface)
	{
		{hrs::ResultType<typename WIN_T::Result>};
		{w.vulkan_surface_handshake(ins, shared_surface)} -> std::convertible_to<hrs::ResultDef<typename WIN_T::Result>>;
	}
auto VulkanContext::WindowContextHandshake(WIN_T &win) //-> std::pair<VulkanContext::Result, hrs::ResultDef<typename WIN_T::Result>>
{
	using OUT_WIN_SURFACE_CONN_T = decltype(win.vulkan_surface_handshake(instance, surface));
	std::pair<Result, OUT_WIN_SURFACE_CONN_T> out_result;

	if(!instance)
	{
		out_result.first = Result::error_code::InstanceNotCreated;
		return out_result;
	}
		//return WarningLevel::FatalError("Instance isn't created yet!");

	//if(surface)
		//return WarningLevel::FatalError("Surface is already connected to another window!");

	auto res = win.vulkan_surface_handshake(instance, surface);

	out_result.first = Result::error_code::SurfaceHandshakeResult;
	out_result.second = res;
	return out_result;

	/*if(!surface)
	{
		return WarningLevel::FatalError("Surface handshake isn't happened, cause WIN_T issues!");
	}

	return WarningLevel::Ok();*/
}

template<typename T_DEVICE_DRIVER>
requires
	std::derived_from<T_DEVICE_DRIVER, VulkanDeviceDriver>
auto VulkanContext::AllocateDevice(const vk::PhysicalDevice &ph_dev) -> hrs::expected<hrs::SpectatorPtr<VulkanDeviceDriver>, VulkanContext::Result>
{
	if(!instance)
		return {Result::error_code::InstanceNotCreated};

	auto it = std::find(physical_devices.begin(), physical_devices.end(), ph_dev);

	if(it == physical_devices.end())
		return {Result::error_code::PhysicalDeviceNotExist};

	decltype(device_drivers)::value_type t_dev(new hrs::ControlBlock<VulkanDeviceDriver>(new T_DEVICE_DRIVER()));

	hrs::SpectatorPtr ret_dev(t_dev.get());

	device_drivers.push_back
	(
		move(t_dev)
	);

	return ret_dev;
}

template<VulkanDeviceDriverSurfaceConnectionExt T_DEVICE_DRIVER>
auto VulkanContext::BindSurface(hrs::SpectatorPtr<VulkanDeviceDriver> dev) //-> WarningLevel
{
	using OUT_DEVICE_SURFACE_CONN_T = decltype(T_DEVICE_DRIVER().connect_surface(surface));
	std::pair<Result, OUT_DEVICE_SURFACE_CONN_T> out_result;

	if(!instance)
	{
		out_result.first = Result::error_code::InstanceNotCreated;
		return out_result;
		//return WarningLevel::FatalError("Instance isn't created yet!");
	}

	if(!surface)
	{
		out_result.first = Result::error_code::SurfaceNotCreated;
		return out_result;
		//return WarningLevel::FatalError("Surface isn't connected to window!");
	}

	//auto locked_dev = dev.lock();
	if(dev.is_empty())
	{
		out_result.first = Result::error_code::DeviceDriverNotExist;
		return out_result;
	}

	//if(locked_dev.get() == nullptr)
	//	return WarningLevel::FatalError("Device is expired!");

	bool found = false;
	for(auto &it : device_drivers)
	{
		if(it.get() != nullptr)
		{
			if(it.get()->data == dev.get())
			{
				found = true;
				break;
			}
		}
	}

	if(!found)
	{
		out_result.first = Result::error_code::DeviceDriverNotExist;
		return out_result;
	}


	//auto casted_dev = std::dynamic_pointer_cast<T_DEVICE_DRIVER>(dev.lock());
	//if(!casted_dev)
	//	return WarningLevel::FatalError("Dynamic dispatch on device is failed!");

	auto casted_dev = dynamic_cast<T_DEVICE_DRIVER *>(dev.get());
	auto res = casted_dev->connect_surface(surface);

	out_result.first = Result::error_code::SurfaceConnectionToDevice;
	out_result.second = res;
	return out_result;

	//return res.is_contain_msg_or_lvls_otherwise_return(hrs::append(WarningLevel::levels::FatalError), WarningLevel::Ok("Surface was successfully binded to device!"));
}
