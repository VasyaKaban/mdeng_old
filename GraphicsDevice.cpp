#include <chrono>
#include <cstring>
//#include "VulkanInclude.h"
#include "GraphicsDevice.h"
#include <limits>

#include <iostream>

using
    std::vector,
    std::optional,
    std::span,
    std::move,
    std::vector,
    std::string_view,
    std::string,
    std::pair,
    std::map,
    std::remove_reference_t,
    std::array;

auto GraphicsDevice::create_swapchain(uint32_t width, uint32_t height, vk::PresentModeKHR mode) -> GraphicsDevice::Result
{
    //if(draw_surface.expired())
     //   return Result::error_code::SurfaceNotConnected;
        //return WarningLevel::FatalError("Drawing surface isn't connected!");

    //if(!device)
     //    return Result::error_code::DeviceNotCreated;
        //return WarningLevel::FatalError("Device isn't creted yet!");

    auto surface_formats = parent_ph_dev.getSurfaceFormatsKHR(*draw_surface.lock().get());
    if(surface_formats.result != vk::Result::eSuccess)
        return surface_formats.result;
        //return WarningLevel::FatalError(vk::to_string(surface_formats.result));

    auto surface_capabilities = parent_ph_dev.getSurfaceCapabilitiesKHR(*draw_surface.lock().get());
    if(surface_capabilities.result != vk::Result::eSuccess)
        return surface_capabilities.result;
       // return WarningLevel::FatalError(vk::to_string(surface_capabilities.result));

    auto surface_present_modes = parent_ph_dev.getSurfacePresentModesKHR(*draw_surface.lock().get());
    if(surface_present_modes.result != vk::Result::eSuccess)
        return surface_present_modes.result;
        //return WarningLevel::FatalError(vk::to_string(surface_present_modes.result));

    //EXPLICITLY DROP!
    /*if(surface_formats.value.empty())
        return WarningLevel::FatalError("Surface doesn't support any formats!");

    if(surface_present_modes.value.empty())
        return WarningLevel::FatalError("Surface doesn't support any presentation modes!");
    */


    constexpr uint32_t SWAPCHAIN_IMAGES_COUNT_EXTENT = 2;
    uint32_t swapchain_min_image_count = 0;
    if(surface_capabilities.value.maxImageCount == 0)
        swapchain_min_image_count = surface_capabilities.value.minImageCount + SWAPCHAIN_IMAGES_COUNT_EXTENT;
    else if(surface_capabilities.value.minImageCount + SWAPCHAIN_IMAGES_COUNT_EXTENT > surface_capabilities.value.maxImageCount)
        swapchain_min_image_count = surface_capabilities.value.maxImageCount;
    else
        swapchain_min_image_count = surface_capabilities.value.minImageCount + SWAPCHAIN_IMAGES_COUNT_EXTENT;

    optional<vk::Format> swapchain_format;
    for(auto &fmt : surface_formats.value)
    {
        if(fmt.colorSpace == vk::ColorSpaceKHR::eVkColorspaceSrgbNonlinear)
        {
            if(fmt.format >= vk::Format::eR8G8B8A8Unorm && fmt.format <= vk::Format::eB8G8R8A8Srgb)
            {
               swapchain_format = fmt.format;
                break;
            }
        }
    }

    if(!swapchain_format)
        return Result::error_code::SurfaceNoSupportedFormats;
        //return WarningLevel::FatalError("No compatible formats for swapchain was founded!");


    vk::Extent2D swapchain_extent = surface_capabilities.value.currentExtent;

    if(swapchain_extent.width == 0 || swapchain_extent.width == 0xFFFFFFFF)
    {
        swapchain_extent.width = width;
        if(swapchain_extent.width > surface_capabilities.value.maxImageExtent.width)
            swapchain_extent.width = surface_capabilities.value.maxImageExtent.width;
    }

    if(swapchain_extent.height == 0 || swapchain_extent.height == 0xFFFFFFFF)
    {
        swapchain_extent.height = height;
        if(swapchain_extent.height > surface_capabilities.value.maxImageExtent.height)
            swapchain_extent.height = surface_capabilities.value.maxImageExtent.height;
    }

    vk::SharingMode swapchain_share_mode;
    vector<uint32_t> swapchain_queue_family;
    if(graphics_queue.value().first == presentation_queue.value().first)
    {
        swapchain_share_mode = vk::SharingMode::eExclusive;
        swapchain_queue_family.push_back(graphics_queue.value().first);
    }
    else
    {
        swapchain_share_mode = vk::SharingMode::eConcurrent;
        swapchain_queue_family.reserve(2);
        swapchain_queue_family.push_back(graphics_queue.value().first);
        swapchain_queue_family.push_back(presentation_queue.value().first);
    }

    bool is_present_mode_found = false;
    for(auto &present_mode : surface_present_modes.value)
        if(present_mode == mode)
        {
            is_present_mode_found = true;
            break;
        }

    //WarningLevel out_res;
    if(!is_present_mode_found)
        return Result::error_code::PresentModeNotSupported;
        //mode = surface_present_modes.value.front();
        //out_res = WarningLevel::Info(string("Passed mode doesn't supported by surface! ") + vk::to_string(mode) + string(" will be used instead!"));

    vk::SwapchainCreateInfoKHR swapchain_info;
    swapchain_info
        .setFlags({})
        .setSurface(*draw_surface.lock().get())
        .setMinImageCount(swapchain_min_image_count)
        .setImageFormat(swapchain_format.value())
        .setImageColorSpace(vk::ColorSpaceKHR::eVkColorspaceSrgbNonlinear)
        .setImageExtent(swapchain_extent)
        .setImageArrayLayers(1)
        .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
        .setImageSharingMode(swapchain_share_mode)
        .setQueueFamilyIndices(swapchain_queue_family)
        .setPreTransform(surface_capabilities.value.currentTransform)
        .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
        .setPresentMode(mode)
        .setClipped(VK_TRUE);

    auto swapchain_tmp = device.createSwapchainKHR(swapchain_info);
    if(swapchain_tmp.result != vk::Result::eSuccess)
        return swapchain_tmp.result;
        //return out_res.concate(WarningLevel::FatalError(vk::to_string(swapchain_tmp.result)));

    auto swapchain_images_tmp = device.getSwapchainImagesKHR(swapchain_tmp.value);
    if(swapchain_images_tmp.result != vk::Result::eSuccess)
        return swapchain_images_tmp.result;
        //return out_res.concate(WarningLevel::FatalError(vk::to_string(swapchain_images_tmp.result)));

    swapchain_squad.swapchain = move(swapchain_tmp.value);
    swapchain_squad.swapchain_images = move(swapchain_images_tmp.value);
    swapchain_squad.image_color_space = vk::ColorSpaceKHR::eVkColorspaceSrgbNonlinear;
    swapchain_squad.image_extent = swapchain_extent;
    swapchain_squad.image_format = swapchain_format.value();

    return Result::error_code::Success;

    //return out_res.is_contain_msg_otherwise_return(WarningLevel::Ok("Swapchain is successfully created!"));
}

auto GraphicsDevice::create_renderpass() -> GraphicsDevice::Result
{
    //if(!swapchain_squad.swapchain)
    //    return WarningLevel::FatalError("Swapchain isn't created yet!");

    vk::AttachmentDescription swapchain_color_attachment_desc;
    swapchain_color_attachment_desc
        .setFlags({})
        .setFormat(swapchain_squad.image_format)
        .setSamples(vk::SampleCountFlagBits::e1)//just no AA
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
        .setStoreOp(vk::AttachmentStoreOp::eDontCare)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);//change this when creating offscreen render!

    vk::AttachmentReference color_attachment_ref;
    color_attachment_ref
        .setAttachment(0)//use 0 index of attachment(swapchain_color_attachment_desc) from parent renderpass
        .setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    vk::SubpassDescription subpass_desc;
    subpass_desc
        .setFlags({})
        .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
        .setInputAttachments({})
        .setColorAttachments(color_attachment_ref);
        //.setResolveAttachments({})
        //.setPDepthStencilAttachment(nullptr)//use late, when creating depth image!
        //.setPreserveAttachments({});

    #warning CHANGE THESE DEPENDENCIES FOR ABILITY TO USE SINGLE DEPTH BUFFER!!!! OR USE MULTIPLE DEPTH BUFFERS PER FRAME!!!!!!
    vector<vk::SubpassDependency> deps
    {
        vk::SubpassDependency()
            .setSrcSubpass(VK_SUBPASS_EXTERNAL)
            .setDstSubpass(0)
            .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .setSrcAccessMask({})
            .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead)
            .setDependencyFlags(vk::DependencyFlagBits::eByRegion),

        vk::SubpassDependency()
            .setSrcSubpass(0)
            .setDstSubpass(VK_SUBPASS_EXTERNAL)
            .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead)
            .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .setDstAccessMask({})
            .setDependencyFlags(vk::DependencyFlagBits::eByRegion),
    };

    vk::RenderPassCreateInfo renderpass_info;
    renderpass_info
        .setFlags({})//compatibility with transform only
        .setAttachments(swapchain_color_attachment_desc)
        .setSubpasses(subpass_desc)
        .setDependencies(deps);//see in future, because external - is implicit subpass

    auto renderpass_tmp = device.createRenderPass(renderpass_info);
    if(renderpass_tmp.result != vk::Result::eSuccess)
        return renderpass_tmp.result;
        //return WarningLevel::FatalError(vk::to_string(renderpass_tmp.result));

    surface_renderpass = move(renderpass_tmp.value);

    return Result::error_code::Success;
    //return WarningLevel::Ok("Renderpass is successfully created!");
}

auto GraphicsDevice::create_swapchain_framebuffers() -> GraphicsDevice::Result
{
    /*if(!swapchain_squad.swapchain)
        return WarningLevel::FatalError("Swapchain isn't created yet!");

    if(!surface_renderpass)
        return WarningLevel::FatalError("Renderpass isn't created yet!");
    */

    vector<vk::ImageView> swapchain_images_views_tmp(swapchain_squad.swapchain_images.size(), vk::ImageView());

    vk::ImageViewCreateInfo image_view_info;
    image_view_info
        .setFlags({})
        .setViewType(vk::ImageViewType::e2D)
        .setFormat(swapchain_squad.image_format)
        .setComponents(vk::ComponentMapping()
            .setR(vk::ComponentSwizzle::eIdentity)
            .setG(vk::ComponentSwizzle::eIdentity)
            .setB(vk::ComponentSwizzle::eIdentity)
            .setA(vk::ComponentSwizzle::eIdentity))
        .setSubresourceRange(vk::ImageSubresourceRange()
            .setAspectMask(vk::ImageAspectFlagBits::eColor)
            .setBaseMipLevel(0)
            .setLevelCount(1)
            .setBaseArrayLayer(0)
            .setLayerCount(1));

    for(size_t i = 0; i < swapchain_squad.swapchain_images.size(); i++)
    {
        image_view_info.setImage(swapchain_squad.swapchain_images[i]);
        auto created_img_view = device.createImageView(image_view_info);
        if(created_img_view.result != vk::Result::eSuccess)
        {
            for(size_t j = 0; j < i; j++)
                device.destroy(swapchain_images_views_tmp[j]);

            return created_img_view.result;
            //return WarningLevel::FatalError(vk::to_string(created_img_view.result));
        }

        swapchain_images_views_tmp[i]  = created_img_view.value;
    }

    vk::FramebufferCreateInfo swapchain_fb_info;
    swapchain_fb_info
        .setFlags({})
        .setRenderPass(surface_renderpass)
        .setWidth(swapchain_squad.image_extent.width)
        .setHeight(swapchain_squad.image_extent.height)
        .setLayers(1);

    vector<vk::Framebuffer> swapchain_framebuffers_tmp(swapchain_images_views_tmp.size(), vk::Framebuffer());

    for(size_t i = 0; i < swapchain_images_views_tmp.size(); i++)
    {
        swapchain_fb_info.setAttachments(swapchain_images_views_tmp[i]);
        auto swapchain_img_view_fb = device.createFramebuffer(swapchain_fb_info);
        if(swapchain_img_view_fb.result != vk::Result::eSuccess)
        {
            for(size_t j = 0; j < i; j++)
                device.destroy(swapchain_framebuffers_tmp[j]);

            for(auto &img_view : swapchain_images_views_tmp)
                device.destroy(img_view);

            return swapchain_img_view_fb.result;
            //return WarningLevel::FatalError(vk::to_string(swapchain_img_view_fb.result));
        }

        swapchain_framebuffers_tmp[i] = move(swapchain_img_view_fb.value);
    }

    swapchain_squad.swapchain_framebuffers = move(swapchain_framebuffers_tmp);
    swapchain_squad.swapchain_images_views = move(swapchain_images_views_tmp);

    return Result::error_code::Success;
    //return WarningLevel::Ok("Swapchain framebuffers are successfully created!");
}

auto GraphicsDevice::load_shaders(const std::vector<LoadedShaderProps> &loaded) -> hrs::ResultDef<GraphicsDevice::Result>
{
    //if(!device)
    //    return WarningLevel::FatalError("Device isn't created yet!");

    map<decltype(loaded.begin()), vk::ShaderModule> shader_refs;
    string missed_shaders;
    for(auto &sh : shaders)
    {
        auto it = std::find_if(loaded.begin(), loaded.end(), [&](const LoadedShaderProps &pr)
        {
            return (pr.name == sh.first) && (pr.code.size() != 0);
        });

        if(it == loaded.end())
        {
            missed_shaders += sh.first;
            missed_shaders += " ";
        }
        else
            shader_refs.insert(pair<decltype(shader_refs)::key_type, vk::ShaderModule>(it, {}));
    }

    if(missed_shaders.size() != 0)
    {
        missed_shaders.pop_back();
        return {Result::error_code::NoDesiredShaders, string("This shaders have been missed: ") + missed_shaders};
        //return WarningLevel::FatalError(string("This shaders have been missed: ") + missed_shaders);
    }

    vk::ShaderModuleCreateInfo shader_info;
    shader_info
        .setFlags({});

    for(auto &sh_ref : shader_refs)
    {
        shader_info.setCode(sh_ref.first->code);
        auto shader_tmp = device.createShaderModule(shader_info);
        if(shader_tmp.result != vk::Result::eSuccess)
        {
            for(auto &del_sh : shader_refs)
                device.destroy(del_sh.second);

            return {shader_tmp.result};
            //return WarningLevel::FatalError(vk::to_string(shader_tmp.result));
        }

        sh_ref.second = move(shader_tmp.value);
    }

    for(auto &sh_ref : shader_refs)
    {
        shaders[string(sh_ref.first->name)] = sh_ref.second;
    }

    return {Result::error_code::Success};
    //return WarningLevel::Ok("Shaders are creaated successfully!");
}

auto GraphicsDevice::create_pipeline() -> GraphicsDevice::Result
{
    //if(!surface_renderpass)
    //    return WarningLevel::FatalError("Renderpass isn't created yet!");

    using namespace std::string_literals;

    struct necessary_shaders_info
    {
        string_view name;
        vk::ShaderStageFlagBits stage;
    };

    constexpr array<necessary_shaders_info, 2> necessary_shaders
    {
        necessary_shaders_info{"vertex_shader_test.spv", vk::ShaderStageFlagBits::eVertex},
        necessary_shaders_info{"fragment_shader_test.spv", vk::ShaderStageFlagBits::eFragment}
    };

#ifndef NDEBUG
    for(auto &n_sh : necessary_shaders)
    {
        auto it = shaders.find(n_sh.name);
        assert(it != shaders.end());
    }
#endif

    string missed_shaders;

    vector<vk::PipelineShaderStageCreateInfo> shaders_info;
    shaders_info.reserve(necessary_shaders.size());

    vk::PipelineShaderStageCreateInfo sh_stage_info;
    sh_stage_info
        .setFlags({})
        .setPName("main");
        //.setPSpecializationInfo(nullptr);//set specialization in future!

    for(auto &sh : necessary_shaders)
    {
        auto it = shaders.find(sh.name);
        /*if(it == shaders.end())
        {
            missed_shaders += sh.name;
            missed_shaders += ' ';
        }
        else if(!it->second)
        {
            missed_shaders += sh.name;
            missed_shaders += ' ';
        }*/
        //else
        //{
        sh_stage_info
            .setStage(sh.stage)
            .setModule(it->second);

        shaders_info.push_back(sh_stage_info);
        //}
    }

    //EXPLICITLY SKIPPED!!!
    /*if(!missed_shaders.empty())
    {
        missed_shaders.pop_back();
        return WarningLevel::FatalError(string("These shaders haven't been loaded: ") + missed_shaders);
    }*/

    vk::PipelineVertexInputStateCreateInfo vertex_input_state_info;
    vertex_input_state_info
        .setFlags({})
        .setVertexBindingDescriptions({})
        .setVertexAttributeDescriptions({});//add bindings for vertex buffers in future!

    vk::PipelineInputAssemblyStateCreateInfo input_asm_state_info;
    input_asm_state_info
        .setFlags({})
        .setTopology(vk::PrimitiveTopology::eTriangleList)
        .setPrimitiveRestartEnable(VK_FALSE);

    vk::Viewport viewport;
    viewport
        .setX(0.0f)
        .setY(0.0f)
        .setWidth(swapchain_squad.image_extent.width)
        .setHeight(swapchain_squad.image_extent.height)
        .setMinDepth(0.0f)
		.setMaxDepth(1.0f);

    vk::Rect2D scissors;
    scissors
        .setOffset({0, 0})
        .setExtent(swapchain_squad.image_extent);

    vk::PipelineViewportStateCreateInfo viewport_create_info;
    viewport_create_info
        .setFlags({})
        .setViewports(viewport)
        .setScissors(scissors);

    //see depth clamping and bias in future!
    //change culling!
    //see line width in features!(if need)
    vk::PipelineRasterizationStateCreateInfo rasterization_state_info;
    rasterization_state_info
        .setFlags({})
        .setDepthClampEnable(VK_FALSE)
        .setRasterizerDiscardEnable(VK_FALSE)
        .setPolygonMode(vk::PolygonMode::eFill)
        .setCullMode(vk::CullModeFlagBits::eNone)
        .setFrontFace(vk::FrontFace::eClockwise)
        .setDepthBiasEnable(VK_FALSE)
        .setDepthBiasConstantFactor(0.0f)
        .setDepthBiasClamp(0.0f)
        .setDepthBiasSlopeFactor(0.0f)
        .setLineWidth(1.0f);

    vk::PipelineMultisampleStateCreateInfo multisample_state_info;
    multisample_state_info
        .setFlags({})
        .setRasterizationSamples(vk::SampleCountFlagBits::e1)
        .setSampleShadingEnable(VK_FALSE)
        .setMinSampleShading(0.0f)
        .setPSampleMask(nullptr)
        .setAlphaToCoverageEnable(VK_FALSE)
        .setAlphaToOneEnable(VK_FALSE);

    vk::PipelineColorBlendAttachmentState color_blend_attach_state_info;
    color_blend_attach_state_info
        .setBlendEnable(VK_TRUE)
        .setSrcColorBlendFactor(vk::BlendFactor::eOne)
        .setDstColorBlendFactor(vk::BlendFactor::eZero)
        .setColorBlendOp(vk::BlendOp::eAdd)
        .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
        .setDstAlphaBlendFactor(vk::BlendFactor::eZero)
        .setAlphaBlendOp(vk::BlendOp::eAdd)
        .setColorWriteMask
        (
            vk::ColorComponentFlagBits::eA |
            vk::ColorComponentFlagBits::eR |
            vk::ColorComponentFlagBits::eG |
            vk::ColorComponentFlagBits::eB
        );

    vk::PipelineColorBlendStateCreateInfo color_blend_state_info;
    color_blend_state_info
        .setFlags({})
        .setLogicOpEnable(VK_FALSE)
        .setLogicOp(vk::LogicOp::eCopy)
        .setAttachments(color_blend_attach_state_info)
        .setBlendConstants({0.0f, 0.0f, 0.0f, 0.0f});//just unused in our env.

	vk::PushConstantRange push_constant;
	push_constant
			.setStageFlags(vk::ShaderStageFlagBits::eVertex)
			.setSize(sizeof(twv::Mat<float, 4, 4>))
			.setOffset(0);

    vk::PipelineLayoutCreateInfo ppl_layout_info;
    ppl_layout_info
        .setFlags({})
        .setSetLayouts({})//use it in future for uniforms!
		.setPushConstantRanges(push_constant);//use it in future for MVP ant other!

    auto ppl_layout_tmp = device.createPipelineLayout(ppl_layout_info);
    if(ppl_layout_tmp.result != vk::Result::eSuccess)
        return ppl_layout_tmp.result;
        //return WarningLevel::FatalError(vk::to_string(ppl_layout_tmp.result));

    vk::GraphicsPipelineCreateInfo graphics_ppl_info;
    graphics_ppl_info
        .setFlags({})
        .setStages(shaders_info)
        .setPVertexInputState(&vertex_input_state_info)
        .setPInputAssemblyState(&input_asm_state_info)
        .setPTessellationState(nullptr)
        .setPViewportState(&viewport_create_info)
        .setPRasterizationState(&rasterization_state_info)
        .setPMultisampleState(&multisample_state_info)
        .setPDepthStencilState(nullptr)//use depth in future!
        .setPColorBlendState(&color_blend_state_info)
        .setPDynamicState(nullptr)//see in future when using swapchain recreation!
        .setLayout(ppl_layout_tmp.value)
        .setRenderPass(surface_renderpass)
        .setSubpass(0);

    auto graphics_ppl_tmp = device.createGraphicsPipelines({}, graphics_ppl_info);
    if(graphics_ppl_tmp.result != vk::Result::eSuccess)
    {
        device.destroy(ppl_layout_tmp.value);
        return graphics_ppl_tmp.result;
        //return WarningLevel::FatalError(vk::to_string(graphics_ppl_tmp.result));
    }

    pipeline_squad.ppl_layout = move(ppl_layout_tmp.value);
    pipeline_squad.ppl = move(graphics_ppl_tmp.value[0]);
    return Result::error_code::Success;
    //return WarningLevel::Ok("Graphics pipeline is created successfully!");
}

auto GraphicsDevice::create_frames_property() -> GraphicsDevice::Result
{
    //if(!device)
    //    return WarningLevel::FatalError("Device isn't created yet!");

    vk::CommandPoolCreateInfo comm_pool_info;
    comm_pool_info
        .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
        .setQueueFamilyIndex(graphics_queue.value().first);

    auto comm_pool_tmp = device.createCommandPool(comm_pool_info);
    if(comm_pool_tmp.result != vk::Result::eSuccess)
        return comm_pool_tmp.result;
        //return WarningLevel::FatalError(vk::to_string(comm_pool_tmp.result));

    vk::CommandBufferAllocateInfo comm_buf_info;
    comm_buf_info
        .setCommandPool(comm_pool_tmp.value)
        .setLevel(vk::CommandBufferLevel::ePrimary)
        .setCommandBufferCount(FREE_FRAMES);

    auto comm_bufs_tmp = device.allocateCommandBuffers(comm_buf_info);
    if(comm_bufs_tmp.result != vk::Result::eSuccess)
    {
        device.destroyCommandPool(comm_pool_tmp.value);
        return comm_bufs_tmp.result;
        //return WarningLevel::FatalError(vk::to_string(comm_bufs_tmp.result));
    }

    frames_comm_pool = move(comm_pool_tmp.value);
    for(size_t i = 0; i < FREE_FRAMES; i++)
        frames_sync[i].buf = move(comm_bufs_tmp.value[i]);

    auto cleanup_prev = [this](size_t i)
    {
        for(size_t j = 0; j < i; j++)
        {
            device.destroy(frames_sync[j].cpu_graphics_submit_fence);
            device.destroy(frames_sync[j].gpu_acquire_image_sem);
        }

        for(size_t j = 0; j < FREE_FRAMES; j++)
            device.free(frames_comm_pool, frames_sync[j].buf);

        device.destroy(frames_comm_pool);
    };

    vk::FenceCreateInfo fence_create_info;
    fence_create_info.setFlags(vk::FenceCreateFlagBits::eSignaled);
    vk::SemaphoreCreateInfo sem_create_info;
    for(size_t i = 0; i < FREE_FRAMES; i++)
    {
        auto fence_tmp = device.createFence(fence_create_info);
        if(fence_tmp.result != vk::Result::eSuccess)
        {
            cleanup_prev(i);
            return fence_tmp.result;
            //return WarningLevel::FatalError(vk::to_string(fence_tmp.result));
        }

        auto acquire_sem_tmp = device.createSemaphore(sem_create_info);
        if(acquire_sem_tmp.result != vk::Result::eSuccess)
        {
            cleanup_prev(i);
            device.destroy(fence_tmp.value);
            return acquire_sem_tmp.result;
            //return WarningLevel::FatalError(vk::to_string(acquire_sem_tmp.result));
        }

        auto submit_sem_tmp = device.createSemaphore(sem_create_info);
        if(submit_sem_tmp.result != vk::Result::eSuccess)
        {
            cleanup_prev(i);
            device.destroy(fence_tmp.value);
            device.destroy(acquire_sem_tmp.value);
            return submit_sem_tmp.result;
            //return WarningLevel::FatalError(vk::to_string(submit_sem_tmp.result));
        }

        frames_sync[i].cpu_graphics_submit_fence = move(fence_tmp.value);
        frames_sync[i].gpu_acquire_image_sem = move(acquire_sem_tmp.value);
        frames_sync[i].gpu_graphics_submit_sem = move(submit_sem_tmp.value);
    }

    return Result::error_code::Success;
    //return WarningLevel::Ok("Frames property is created successfully!");
}

GraphicsDevice::GraphicsDevice()
{
    #warning TBA!
}

GraphicsDevice::GraphicsDevice(GraphicsDevice &&gd) noexcept
{
    #warning TBA!
}

GraphicsDevice::~GraphicsDevice()
{
    if(device)
    {
        //see, what we can do with res?!
        auto res = device.waitIdle();
        if(frames_comm_pool)
        {
            for(auto &frame : frames_sync)
            {
                device.destroy(frame.cpu_graphics_submit_fence);
                device.destroy(frame.gpu_graphics_submit_sem);
                device.destroy(frame.gpu_acquire_image_sem);
                device.free(frames_comm_pool, frame.buf);
            }
        }

        device.destroy(frames_comm_pool);

        device.destroy(pipeline_squad.ppl);
        device.destroy(pipeline_squad.ppl_layout);
        for(auto &sh : shaders)
            device.destroy(sh.second);

        for(auto &fb: swapchain_squad.swapchain_framebuffers)
            device.destroy(fb);

        device.destroy(surface_renderpass);

        for(auto &img_v: swapchain_squad.swapchain_images_views)
            device.destroy(img_v);

        device.destroy(swapchain_squad.swapchain);

        device.destroy();
        //no cleanup!!!
    }
    parent_ph_dev = vk::PhysicalDevice();
    draw_surface.reset();
    #warning TBA!
}

auto GraphicsDevice::init(vk::PhysicalDevice &ph_dev) -> hrs::ResultDef<GraphicsDevice::Result>
{
    if(draw_surface.expired())
        return {Result::error_code::SurfaceNotConnected};
        //return WarningLevel::FatalError("Surface isn't connected yet!");
    if(!ph_dev)
        return {Result::error_code::PhysicalDeviceExpired};
        //return WarningLevel::FatalError("PhysicalDevice isn't created yet!");

    uint32_t queue_families_count = 0;
    auto queue_props = ph_dev.getQueueFamilyProperties();

    optional<uint32_t> graphics_presentation_queue_opt;
    optional<uint32_t>  graphics_queue_opt;
    optional<uint32_t>  presentation_queue_opt;
    vk::Result res;
    for(size_t i = 0; i < queue_props.size(); i++)
    {
        if(queue_props[i].queueFlags & vk::QueueFlagBits::eGraphics)
        {
            auto surface_supported = ph_dev.getSurfaceSupportKHR(i, *draw_surface.lock().get());
            if(surface_supported.result != vk::Result::eSuccess)
                return {surface_supported.result};
                //return WarningLevel::FatalError(vk::to_string(surface_supported.result));

            if(surface_supported.value)
            {
                graphics_presentation_queue_opt = i;
                break;
            }
        }
    }

    if(!graphics_presentation_queue_opt)//we don't have queue that supports both presentation and rendering
    {
        for(size_t i = 0; i < queue_props.size(); i++)
        {
            if(queue_props[i].queueFlags & vk::QueueFlagBits::eGraphics)
            {
                graphics_queue_opt = i;
                break;
            }
        }

        if(!graphics_queue_opt)
            return {Result::error_code::NoGraphicsQueue};
            //return WarningLevel::FatalError("No one queue is supporting VK_QUEUE_GRAPHICS_BIT!");

        for(size_t i = 0; i < queue_props.size(); i++)
        {

            auto surface_supported = ph_dev.getSurfaceSupportKHR(i, *draw_surface.lock().get());
            if(surface_supported.result != vk::Result::eSuccess)
                return {surface_supported.result};
                //return WarningLevel::FatalError(vk::to_string(res));

            if(surface_supported.value)
            {
                presentation_queue_opt = i;
                break;
            }
        }

        if(!presentation_queue_opt)
            return {Result::error_code::NoPresentationQueue};
            //return WarningLevel::FatalError("No one queue is supporting presentation!");
    }

    vector<vk::DeviceQueueCreateInfo> queue_infos;
    float queue_priority[] = {1.0f};

    if(graphics_presentation_queue_opt)
    {
        vk::DeviceQueueCreateInfo graphics_presentation_queue_info;
        graphics_presentation_queue_info
            .setFlags({})
            .setQueueFamilyIndex(graphics_presentation_queue_opt.value())
            .setQueueCount(1)
            .setQueuePriorities(queue_priority);

        queue_infos.push_back(graphics_presentation_queue_info);
    }
    else
    {
        queue_infos.reserve(2);
        vk::DeviceQueueCreateInfo graphics_queue_info;
        graphics_queue_info
            .setFlags({})
            .setQueueFamilyIndex(graphics_queue_opt.value())
            .setQueueCount(1)
            .setQueuePriorities(queue_priority);

        vk::DeviceQueueCreateInfo presentation_queue_info;
        graphics_queue_info
            .setFlags({})
            .setQueueFamilyIndex(presentation_queue_opt.value())
            .setQueueCount(1)
            .setQueuePriorities(queue_priority);

        queue_infos.push_back(graphics_queue_info);
        queue_infos.push_back(presentation_queue_info);
    }

    string missed_exts;

    vector<const char *> extensions
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    auto ext_props = ph_dev.enumerateDeviceExtensionProperties();
    if(ext_props.result != vk::Result::eSuccess)
        return {ext_props.result};
        //return WarningLevel::FatalError(vk::to_string(ext_props.result));

	for(auto &ext : extensions)
	{
		auto res = std::find_if(ext_props.value.begin(), ext_props.value.end(), [&](const vk::ExtensionProperties &ext_prop)
			{
				if(string_view(ext) == ext_prop.extensionName)
					return true;

                return false;
			});

		if(res == ext_props.value.end())
		{
			missed_exts += ext;
			missed_exts.push_back(' ');
		}
	}

	if(!missed_exts.empty())
	{
		missed_exts.pop_back();
        return {Result::error_code::ExtensionNotSupported, string("These extensions are not supported: ") + missed_exts};
		//return WarningLevel::FatalError(string("These extensions are not supported: " + missed_exts));
	}


	vk::PhysicalDeviceFeatures enabled_features;//Use for future!

    vk::DeviceCreateInfo device_info;
    device_info
        .setFlags({})
        .setQueueCreateInfos(queue_infos)
        .setPEnabledLayerNames({})
        .setPEnabledExtensionNames(extensions)
        .setPEnabledFeatures(&enabled_features);

    auto created_device = ph_dev.createDevice(device_info);
    if(created_device.result != vk::Result::eSuccess)
        return {created_device.result};
        //return WarningLevel::FatalError(vk::to_string(created_device.result));

    device = move(created_device.value);

    parent_ph_dev = ph_dev;
    vk::Queue recv_queue;
    if(graphics_presentation_queue_opt)
    {
        graphics_queue_opt = graphics_presentation_queue_opt;
        presentation_queue_opt = graphics_presentation_queue_opt;
    }

    recv_queue = device.getQueue(graphics_queue_opt.value(), 0);
    graphics_queue = {graphics_queue_opt.value(), recv_queue};

    recv_queue = device.getQueue(presentation_queue_opt.value(), 0);
    presentation_queue = {presentation_queue_opt.value(), recv_queue};

    return {Result::error_code::Success};
    //return WarningLevel::Ok("Graphics device was successfully inited!");
}

auto GraphicsDevice::is_inited() -> bool
{
    return device;
}

auto GraphicsDevice::connect_surface(std::weak_ptr<vk::SurfaceKHR> sur) -> GraphicsDevice::Result
{
    if(!draw_surface.expired())
        return Result::error_code::SurfaceAlreadyConnected;
        //return WarningLevel::FatalError("Drawing surface is already connected!");

    if(sur.expired())
        return Result::error_code::SurfaceNotExist;
        //return WarningLevel::FatalError("Passed surface is expired!");

    draw_surface = sur;
    return Result::error_code::Success;
    //return WarningLevel::Ok("Surface is connected successfully");
}

auto GraphicsDevice::is_surface_connected() -> bool
{
    return !draw_surface.expired();
}

auto GraphicsDevice::QueryShaders() -> std::vector<std::string_view>
{
    vector<string_view> shaders_names;
    shaders_names.reserve(shaders.size());
    for(auto &sh : shaders)
        shaders_names.push_back(sh.first);
    return shaders_names;
}

#warning no cleanup here!
auto GraphicsDevice::CreateWorkEnv(const DrawableAreaParams &params, const std::vector<LoadedShaderProps> &loaded) -> hrs::ResultDef<GraphicsDevice::Result>
{
    auto res = create_swapchain(params.width, params.height, params.mode);
    if(res.code != Result::error_code::Success)
        return res;

    res = create_renderpass();
    if(res.code != Result::error_code::Success)
        return res;

    res = create_swapchain_framebuffers();
    if(res.code != Result::error_code::Success)
        return res;

    auto res_def = load_shaders(loaded);
    if(res_def.error.code != Result::error_code::Success)
        return res;

    res = create_pipeline();
    if(res.code != Result::error_code::Success)
        return res;

    res = create_frames_property();
    if(res.code != Result::error_code::Success)
        return res;

    is_env_created = true;

    return {Result::error_code::Success};
    //return WarningLevel::Ok("Environment created!");
}

auto GraphicsDevice::RecreateDrawableArea(const DrawableAreaParams &params) -> GraphicsDevice::Result
{

}

auto GraphicsDevice::Draw(const twv::glsl::Mat4x4 &model) -> GraphicsDevice::Result
{
    if(!is_env_created)
        return Result::error_code::EnvironmentNotCreated;

    /*if(!frames_comm_pool)
        return WarningLevel::FatalError("Frames property isn't allocated yet!");

    if(!pipeline_squad.ppl)
        return WarningLevel::FatalError("Pipeline isn't allocated yet!");
    */

	auto blind_res = ExplicitBlindDraw(model);
    if(blind_res != vk::Result::eSuccess)
        return blind_res;
        //return WarningLevel::FatalError(vk::to_string(blind_res));

    return Result::error_code::Success;
    //return WarningLevel::Ok();
}

auto GraphicsDevice::ExplicitBlindDraw(const twv::glsl::Mat4x4 &model) -> vk::Result
{
    //add timeout check!
    auto res = device.waitForFences(frames_sync[target_frame_ind].cpu_graphics_submit_fence, VK_FALSE, std::numeric_limits<uint64_t>::max());
    if(res != vk::Result::eSuccess)
        return res;

    res = device.resetFences(frames_sync[target_frame_ind].cpu_graphics_submit_fence);
    if(res != vk::Result::eSuccess)
        return res;

    auto acquired_img_ind = device.acquireNextImageKHR(swapchain_squad.swapchain, std::numeric_limits<uint64_t>::max(), frames_sync[target_frame_ind].gpu_acquire_image_sem, {});
    if(acquired_img_ind.result != vk::Result::eSuccess)
        return res;

    vk::CommandBufferBeginInfo comm_buf_begin_info;
    comm_buf_begin_info
        .setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);//see in future!


    vk::RenderPassBeginInfo renderpass_begin_info;
    renderpass_begin_info
        .setRenderPass(surface_renderpass)
        .setFramebuffer(swapchain_squad.swapchain_framebuffers[acquired_img_ind.value])
        .setRenderArea
        (
            vk::Rect2D()
                .setOffset({0, 0})
                .setExtent(swapchain_squad.image_extent)
        )
        .setClearValues
        (
            vk::ClearValue()
                .setColor(vk::ClearValue().color = {0.0f, 0.0f, 0.0f, 1.0f})//add depth!
        );

    res = frames_sync[target_frame_ind].buf.begin(comm_buf_begin_info);
    if(res != vk::Result::eSuccess)
        return res;

    frames_sync[target_frame_ind].buf.beginRenderPass(renderpass_begin_info, vk::SubpassContents::eInline);
    frames_sync[target_frame_ind].buf.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_squad.ppl);
	frames_sync[target_frame_ind].buf.pushConstants(pipeline_squad.ppl_layout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(twv::Mat<float, 4, 4>), &model[0][0]);
	frames_sync[target_frame_ind].buf.draw(3, 1, 0, 0);
    frames_sync[target_frame_ind].buf.endRenderPass();

    res = frames_sync[target_frame_ind].buf.end();
    if(res != vk::Result::eSuccess)
        return res;

    vk::PipelineStageFlags stages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

    vk::SubmitInfo graphics_submit_info;
    graphics_submit_info
        .setCommandBuffers(frames_sync[target_frame_ind].buf)
        .setWaitDstStageMask(stages)
        .setWaitSemaphores(frames_sync[target_frame_ind].gpu_acquire_image_sem)
        .setSignalSemaphores(frames_sync[target_frame_ind].gpu_graphics_submit_sem);

    res = graphics_queue.value().second.submit(graphics_submit_info, frames_sync[target_frame_ind].cpu_graphics_submit_fence);
    if(res != vk::Result::eSuccess)
        return res;

    /*res = frames_sync[target_frame_ind].buf.reset();
    if(res != vk::Result::eSuccess)
        return WarningLevel::FatalError(vk::to_string(res));*/

    vk::PresentInfoKHR present_info;
    present_info
        .setWaitSemaphores(frames_sync[target_frame_ind].gpu_graphics_submit_sem)
        .setSwapchains(swapchain_squad.swapchain)
        .setImageIndices(acquired_img_ind.value);


    res = presentation_queue.value().second.presentKHR(present_info);
    if(res != vk::Result::eSuccess)
        return res;

    target_frame_ind++;
    target_frame_ind = target_frame_ind % FREE_FRAMES;

    return vk::Result::eSuccess;
}

auto GraphicsDevice::IsEnvCreated() -> bool
{
   return is_env_created;
}
