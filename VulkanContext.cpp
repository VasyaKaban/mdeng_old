#include <string>
#include "VulkanContext.h"


using
	std::string,
	std::move,
	std::vector,
	std::string_view,
	std::optional,
	std::weak_ptr;

VulkanContext::VulkanContext()
{
}

VulkanContext::VulkanContext(VulkanContext &&ctx) noexcept
{
	instance = ctx.instance;
	ctx.instance = VK_NULL_HANDLE;
	physical_devices = move(ctx.physical_devices);
	device_drivers = move(ctx.device_drivers);
}

VulkanContext::~VulkanContext()
{
	device_drivers.clear();

	if(surface)
	{
		instance.destroy(*surface.get());
		surface.reset();
	}

	instance.destroy();
}

auto VulkanContext::init(const vector<const char *> &layers, const vector<const char *> &extensions,
						const string &application_name, uint32_t application_version,
						const string &engine_name, uint32_t engine_version) -> hrs::ResultDef<VulkanContext::Result>
{
	/*if(instance)
		return WarningLevel::Warning("Context is already inited!");
	*/

	VkResult plain_res;
	auto ptr = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceVersion");
	uint32_t instance_version = VK_API_VERSION_1_0;
	if(ptr)
	{
		auto api_ver = vk::enumerateInstanceVersion();
		if(api_ver.result != vk::Result::eSuccess)
			return {api_ver.result};

		instance_version = api_ver.value;
	}

	auto instance_exts = vk::enumerateInstanceExtensionProperties(nullptr);
	if(instance_exts.result != vk::Result::eSuccess)
		return {instance_exts.result};

	auto instance_lays = vk::enumerateInstanceLayerProperties();
	if(instance_lays.result != vk::Result::eSuccess)
		return {instance_lays.result};

	string missed_exts_lays;

	for(auto &ext : extensions)
	{
		auto res = std::find_if(instance_exts.value.begin(), instance_exts.value.end(), [&](const vk::ExtensionProperties &ext_prop)
			{
				if(ext_prop.extensionName == string_view(ext))
					return true;
				else
					return false;
			});

		if(res == instance_exts.value.end())
		{
			missed_exts_lays += ext;
			missed_exts_lays.push_back('\n');
		}
	}

	for(auto &lay : layers)
	{
		auto res = std::find_if(instance_lays.value.begin(), instance_lays.value.end(), [&](const vk::LayerProperties &lay_prop)
			{
				if(lay_prop.layerName == string_view(lay))
					return true;
				else
					return false;
			});
		if(res == instance_lays.value.end())
		{
			missed_exts_lays += lay;
			missed_exts_lays.push_back('\n');
		}
	}

	if(!missed_exts_lays.empty())
	{
		missed_exts_lays.pop_back();
		return {Result::error_code::ExtensionOrLayerNotSupported, string("These extensions and layers are not supported: " + missed_exts_lays)};
	}

	vk::ApplicationInfo app_info;
	app_info
		.setPApplicationName(application_name.c_str())
		.setApplicationVersion(application_version)
		.setPEngineName(engine_name.c_str())
		.setEngineVersion(engine_version)
		.setApiVersion(instance_version);

	vk::InstanceCreateInfo instance_info;
	instance_info
		.setPApplicationInfo(&app_info)
		.setPEnabledLayerNames(layers)
		.setPEnabledExtensionNames(extensions);

	auto tmp_instance = vk::createInstance(instance_info);
	if(tmp_instance.result != vk::Result::eSuccess)
		return {tmp_instance.result};

	auto devs = tmp_instance.value.enumeratePhysicalDevices();
	if(devs.result != vk::Result::eSuccess)
	{
		tmp_instance.value.destroy();
		return {devs.result};
	}

	instance = move(tmp_instance.value);
	physical_devices = move(devs.value);

	return {Result::error_code::Success};
}

auto VulkanContext::GetPhysicalDevicesCount() -> size_t
{
	return physical_devices.size();

}

auto VulkanContext::GetPhysicalDevice(size_t ind) -> optional<vk::PhysicalDevice>
{
	if(physical_devices.size() > ind)
		return physical_devices[ind];

	return {};
}

auto VulkanContext::GetPhysicalDevices() -> vector<vk::PhysicalDevice>
{
	return physical_devices;
}

auto VulkanContext::DropDeviceDriver(hrs::SpectatorPtr<VulkanDeviceDriver> dev) -> void
{
	if(dev.is_empty())
		return;

	size_t ind = 0;
	for(auto &it : device_drivers)
	{
		if(it.get()->data == dev.get())
		{
			device_drivers.erase(device_drivers.begin() + ind);
			break;
		}
		ind++;
	}
}

/*auto VulkanContext::InitDeviceDriver(weak_ptr<VulkanDeviceDriver> dev) -> WarningLevel
{
	if(!instance)
		return WarningLevel::FatalError("Instance isn't created yet!");

	auto locked_dev = dev.lock();

	if(locked_dev.get() == nullptr)
		return WarningLevel::FatalError("Device is expired!");

	bool found = false;
	for(auto &it : device_drivers)
	{
		if(it.device.get() == locked_dev.get())
		{
			found = true;
			auto res = locked_dev.get()->init(physical_devices[it.ph_dev_ind]);

			return res.is_contain_msg_or_lvls_otherwise_return(hrs::append(WarningLevel::levels::FatalError), WarningLevel::Ok("Device is inited successfully!"));
		}
	}

	if(!found)
		return WarningLevel::FatalError("Device isn't a part of this context!");

	return WarningLevel::Ok("Device is inited successfully!");
}*/
