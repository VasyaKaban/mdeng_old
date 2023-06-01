#pragma once

#include "VulkanContext.h"
#include <memory>
#include <optional>
#include <map>
#include "VulkanDeviceDriver.h"
#include "utils/expected.hpp"
#include "math/Mat.hpp"

class GraphicsDevice : public VulkanDeviceDriver
{
public:
    struct LoadedShaderProps
    {
        std::string_view name;
        std::span<uint32_t> code;
    };

    struct DrawableAreaParams
    {
        uint32_t width;
        uint32_t height;
        vk::PresentModeKHR mode;
    };

    struct Result
    {
        enum class error_code : uint8_t
        {
            //common
            Success,

			//Vulkan
			InnerVulkanError,

            #warning ADD IN FUTURE!!!
            //Device objects
            DeviceNotCreated,
            SurfaceNotConnected,
            PresentModeNotSupported,
            SurfaceNoSupportedFormats,
            NoDesiredShaders,
            PhysicalDeviceExpired,
            NoGraphicsQueue,
            NoPresentationQueue,
            ExtensionNotSupported,
            SurfaceAlreadyConnected,
            SurfaceNotExist,
            EnvironmentNotCreated
            //SwapchainNotCreated,
            //RenderPassNotCreated,
            //PipelineNotCreated,
            //FramePropertyNotCreated,


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

private:
    vk::Device device;
    std::weak_ptr<vk::SurfaceKHR> draw_surface;
    vk::PhysicalDevice parent_ph_dev;

    std::optional<std::pair<uint32_t, vk::Queue>> graphics_queue;
    std::optional<std::pair<uint32_t, vk::Queue>> presentation_queue;

    struct SwapchainDesc
    {
        vk::SwapchainKHR swapchain;
        std::vector<vk::Image> swapchain_images;
        vk::Format image_format;
        vk::Extent2D image_extent;
        vk::ColorSpaceKHR image_color_space;
        std::vector<vk::Framebuffer> swapchain_framebuffers;
        std::vector<vk::ImageView> swapchain_images_views;

    } swapchain_squad;

    vk::RenderPass surface_renderpass;

    std::map<std::string_view, vk::ShaderModule> shaders
    {
        {"vertex_shader_test.spv", {}},
        {"fragment_shader_test.spv", {}},
    };

    struct PipelineDesc
    {
        vk::PipelineLayout ppl_layout;
        vk::Pipeline ppl;
    } pipeline_squad;

    constexpr static uint32_t FREE_FRAMES = 1;

    vk::CommandPool frames_comm_pool;

    struct AcquireFrameSync
    {
        vk::Fence cpu_graphics_submit_fence;
        vk::Semaphore gpu_acquire_image_sem;
        vk::Semaphore gpu_graphics_submit_sem;
        vk::CommandBuffer buf;
    };

    std::array<AcquireFrameSync, FREE_FRAMES> frames_sync;

    uint32_t target_frame_ind = 0;

    bool is_env_created = false;

private:
    auto create_swapchain(uint32_t width, uint32_t height, vk::PresentModeKHR mode = vk::PresentModeKHR::eFifo) -> Result;
    auto create_renderpass() -> Result;
    auto create_swapchain_framebuffers() -> Result;
    auto load_shaders(const std::vector<LoadedShaderProps> &loaded) -> hrs::ResultDef<GraphicsDevice::Result>;
    auto create_pipeline() -> Result;
    auto create_frames_property() -> Result;
public:
    GraphicsDevice();
    GraphicsDevice(const GraphicsDevice &gd) = delete;
    GraphicsDevice(GraphicsDevice &&gd) noexcept;
    ~GraphicsDevice();

    //VulkanDeviceDriverExt
    auto init(vk::PhysicalDevice &ph_dev) -> hrs::ResultDef<Result>;
    auto is_inited() -> bool;

    //VulkanDeviceDriverSurfaceConnectionExt
    auto connect_surface(std::weak_ptr<vk::SurfaceKHR> sur) -> Result;
    auto is_surface_connected() -> bool;

    //
    auto QueryShaders() -> std::vector<std::string_view>;
    auto CreateWorkEnv(const DrawableAreaParams &params, const std::vector<LoadedShaderProps> &loaded) -> hrs::ResultDef<Result>;
    auto RecreateDrawableArea(const DrawableAreaParams &params) -> Result;
	auto Draw(const twv::glsl::Mat4x4 &model) -> Result;
	auto ExplicitBlindDraw(const twv::glsl::Mat4x4 &model) -> vk::Result;
    auto IsEnvCreated() -> bool;
};

constexpr auto GraphicsDevice::Result::message() const -> std::string_view
{
	std::string_view res;
    switch(code)
    {
        case Result::error_code::Success:
            res = "Graphics device successfull operation";
            break;
        case Result::error_code::InnerVulkanError:
            res = vk::to_string(vulkan_res);
            break;
        case Result::error_code::DeviceNotCreated:
            res = "Device isn't created yet";
            break;
        case Result::error_code::SurfaceNotConnected:
            res = "Surface isn't connected yet";
            break;
        case Result::error_code::PresentModeNotSupported:
            res = "Present mode not supported";
            break;
        case Result::error_code::SurfaceNoSupportedFormats:
            res = "Surface doesn't support desired formats";
            break;
        case Result::error_code::NoDesiredShaders:
            res = "Some shaders haven't been loaded";
            break;
        case Result::error_code::PhysicalDeviceExpired:
            res = "Physical device isn't created yet";
            break;
        case Result::error_code::NoGraphicsQueue:
            res = "Device doesn't support any graphics queues";
            break;
        case Result::error_code::NoPresentationQueue:
            res = "Device doesn't support any presentation queues";
            break;
        case Result::error_code::ExtensionNotSupported:
            res = "Extension not supoorted";
            break;
        case Result::error_code::SurfaceAlreadyConnected:
            res = "Drawing surface already connected";
            break;
        case Result::error_code::SurfaceNotExist:
            res = "Surface not exist";
            break;
        case Result::error_code::EnvironmentNotCreated:
            res = "Work environment isn't created yet";
            break;
    }

    return res;
}
constexpr auto GraphicsDevice::Result::to_view() const -> std::string_view
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
        case Result::error_code::DeviceNotCreated:
            res = "DeviceNotCreated";
            break;
        case Result::error_code::SurfaceNotConnected:
            res = "SurfaceNotConnected";
            break;
        case Result::error_code::PresentModeNotSupported:
            res = "PresentModeNotSupported";
            break;
        case Result::error_code::SurfaceNoSupportedFormats:
            res = "SurfaceNoSupportedFormats";
            break;
        case Result::error_code::NoDesiredShaders:
            res = "NoDesiredShaders";
            break;
        case Result::error_code::PhysicalDeviceExpired:
            res = "PhysicalDeviceExpired";
            break;
        case Result::error_code::ExtensionNotSupported:
            res = "ExtensionNotSupported";
            break;
        case Result::error_code::SurfaceAlreadyConnected:
            res = "SurfaceAlreadyConnected";
            break;
        case Result::error_code::SurfaceNotExist:
            res = "SurfaceNotExist";
            break;
        case Result::error_code::EnvironmentNotCreated:
            res = "EnvironmentNotCreated";
            break;
    }

    return res;
}

constexpr auto GraphicsDevice::Result::operator=(const Result::error_code err) -> Result &
{
	code = err;
    return *this;
}

constexpr auto GraphicsDevice::Result::operator=(const vk::Result res) -> Result &
{
	code = GraphicsDevice::Result::error_code::InnerVulkanError;
	vulkan_res = res;
	return *this;
}

constexpr auto operator==(const GraphicsDevice::Result &res, const GraphicsDevice::Result::error_code &err_code) -> bool
{
	return res.code == err_code;
}
