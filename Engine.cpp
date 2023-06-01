#include "Engine.h"
#include "GraphicsDevice.h"
#include <fstream>
#include <charconv>
#include <optional>
#include <array>
#include "math/Math.hpp"

using
    std::vector,
    std::string_view,
    std::filesystem::path,
    std::filesystem::status,
    std::filesystem::file_type,
    std::filesystem::file_size,
    std::filesystem::create_directories,
    std::filesystem::exists,
    std::ifstream,
    std::ofstream,
    std::string,
    std::istringstream,
    std::getline,
    std::from_chars,
    std::optional,
    std::array,
    std::pair,
    std::error_code,
    std::endl,
    std::move;

auto Engine::init_logger() -> Engine::Result
{
    auto res = logger.init(settings.log_output_path.value);
    logger.log(res);
    if(res.code != Logger::Result::error_code::Success)
        return Engine::Result::error_code::LoggerInitError;

    return Engine::Result::error_code::Success;
}

auto Engine::init_settings() -> Engine::Result
{
	Settings readed_settings;
	auto res = readed_settings.read_settings("settings.conf");
	logger.log(res);
	if(res.error.code == Settings::Result::error_code::OutputOpenError ||
		res.error.code == Settings::Result::error_code::WriteError ||
		res.error.code == Settings::Result::error_code::InputOpenError ||
		res.error.code == Settings::Result::error_code::ReadError)
		return Engine::Result::error_code::SettingsInitError;

	settings = readed_settings;

	return Engine::Result::error_code::Success;
}

auto Engine::init_window() -> Engine::Result
{
    auto res = window.init("Engine", settings.window_width.value, settings.window_height.value, settings.window_is_fullscreen.value);
    logger.log(res);
    if(res.error.code != SDLwindow::Result::error_code::Success)
        return Engine::Result::error_code::WindowInitError;

	window.set_quit_callback([&]()
	{
		is_run = false;
	});

	window.set_keyboard_key_callback([&](SDL_KeyCode key, bool is_pressed)
	{
		std::stringstream strstream;
		strstream<<"Pressed: "<<key<<" is pressed: "<<std::boolalpha<<is_pressed<<std::endl;
		logger.log(strstream.str());

		switch(key)
		{
			case SDLK_a:
				if(is_pressed)
					main_player.SetAsideRunState(Player::RunState::Negative);
				else
					main_player.SetAsideRunState(Player::RunState::None);
				break;
			case SDLK_d:
				if(is_pressed)
					main_player.SetAsideRunState(Player::RunState::Positive);
				else
					main_player.SetAsideRunState(Player::RunState::None);
				break;
			case SDLK_w:
				if(is_pressed)
					main_player.SetForwardRunState(Player::RunState::Positive);
				else
					main_player.SetForwardRunState(Player::RunState::None);
				break;
			case SDLK_s:
				if(is_pressed)
					main_player.SetForwardRunState(Player::RunState::Negative);
				else
					main_player.SetForwardRunState(Player::RunState::None);
				break;
			default:
				break;
		}
	});

	window.set_mousemotion_callback([&](SDLwindow::Callbacks::MouseCoord coord)
	{
		static SDLwindow::Callbacks::MouseCoord prev{.x = 0, .y = 0};
		auto delta = coord;
		delta -= prev;

		const float delta_rot = 0.8f;

		if(delta.x < 0)
		{
			main_player.GetYaw() += delta_rot;
		}
		else if(delta.x > 0)
		{
			main_player.GetYaw() -= delta_rot;
		}

		if(delta.y < 0)
		{
			main_player.GetPitch() += delta_rot;
		}
		else if(delta.y > 0)
		{
			main_player.GetPitch() -= delta_rot;
		}

		main_player.SetupYawPitch();

		prev = coord;
	});

    return Engine::Result::error_code::Success;
}

auto Engine::init_draw_context() -> Engine::Result
{
    vector<const char *> extensions;
    auto res = window.get_extensions(extensions);
    logger.log(res);
    if(res.error.code != SDLwindow::Result::error_code::Success)
        return Engine::Result::error_code::DrawContextInitError;

    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    vector<const char *> layers = {"VK_LAYER_KHRONOS_validation"};

    auto ctx_res = drawing_context.init(layers, extensions, "melting_dawn", VK_MAKE_VERSION(0, 1, 0), "harbourside", VK_MAKE_VERSION(1, 0, 0));
    logger.log(ctx_res);
    if(ctx_res.error.code != VulkanContext::Result::error_code::Success)
        return Engine::Result::error_code::DrawContextInitError;

    return Result::error_code::Success;
}

auto Engine::init_resource_manager() -> Engine::Result
{
    ResourceManager::ResourceManagerFS start_search_root(settings.shaders_path.value);
    auto res = resource_manager.init(start_search_root);
    logger.log(res);
    if(res.code != ResourceManager::Result::error_code::Success)
        return Engine::Result::error_code::ResourceManagerInitError;

    return Engine::Result::error_code::Success;
}

auto Engine::init_inter_module_connection() -> Engine::Result
{
    auto res = drawing_context.WindowContextHandshake(window);
    if(res.first != VulkanContext::Result::error_code::SurfaceHandshakeResult)
    {
        logger.log(res.first);
        return Engine::Result::error_code::IMCInitError;
    }
    else if(res.second.error.code != SDLwindow::Result::error_code::Success)
    {
        logger.log(res.second);
        return Engine::Result::error_code::IMCInitError;
    }

    return Engine::Result::error_code::Success;
}

auto Engine::init_devices() -> Engine::Result
{
    auto ph_devs = drawing_context.GetPhysicalDevices();
    if(ph_devs.empty())
    {
        logger.log(Result(Result::error_code::NoAvailableDevices));
        return Result::error_code::DevicesInitError;
    }

    std::stringstream strs;
    strs<<"Available devices:\n";
    for(auto &ph_d : ph_devs)
	{
        auto props = ph_d.getProperties();
        strs<<"Device name: "<<props.deviceName<<" | DeviceId: "<<props.deviceID<<" | VendorID: "<<props.vendorID<<" | Type: ";
		strs<<vk::to_string(props.deviceType);
		/*switch(props.deviceType)
        {
            case vk::PhysicalDeviceType::eCpu:
                strs<<"CPU";
                break;
            case vk::PhysicalDeviceType::eDiscreteGpu:
                strs<<"Discrete GPU";
                break;
            case vk::PhysicalDeviceType::eIntegratedGpu:
                strs<<"Integrated GPU";
                break;
            case vk::PhysicalDeviceType::eOther:
                strs<<"Other";
                break;
            case vk::PhysicalDeviceType::eVirtualGpu:
                strs<<"Virtual GPU";
                break;
		}*/
        strs<<" | UUID: ";
        for(auto &it : props.pipelineCacheUUID)
            strs<<std::hex<<+it;
        strs<<"\n";
    }

    logger.log(hrs::ResultDef<Result>(Result::error_code::AvailableDevicesList, strs.str()));
    return Engine::Result::error_code::Success;
}

auto Engine::init_graphics_device() -> Engine::Result
{
    //just select fisrt GPU
    optional<vk::PhysicalDevice> choosed_device{};

    auto ph_devs = drawing_context.GetPhysicalDevices();
    string choosed_dev_name;
    size_t i = 0;
    for(auto &ph_d : ph_devs)
    {
        auto props = ph_d.getProperties();
        if(i == 0)
            choosed_dev_name = string(props.deviceName);
        //if(!choosed_device_index && (info.type == vk::PhysicalDeviceType::eCpu))
        if(!choosed_device && (props.deviceType == vk::PhysicalDeviceType::eIntegratedGpu || props.deviceType == vk::PhysicalDeviceType::eDiscreteGpu))
        {
            choosed_dev_name = string(props.deviceName);
            choosed_device = ph_d;
            break;
        }
        i++;
    }

    using namespace std::string_literals;


    if(!choosed_device)
    {
        choosed_device = ph_devs[0];
        logger.log(hrs::ResultDef<Result>(Result(Result::error_code::ChoosedDeviceInstead), "No any integrated or discrete GPUs in current system are available! This device will be used instead: "s + choosed_dev_name));
    }

    auto create_res_exp = drawing_context.AllocateDevice<GraphicsDevice>(choosed_device.value());
    if(!create_res_exp.has_value())
    {
        logger.log(create_res_exp.error());
        return Engine::Result::error_code::GraphicsDeviceInitError;
    }

    auto weak_ptr_dev = create_res_exp.value();

    auto bind_res = drawing_context.BindSurface<GraphicsDevice>(weak_ptr_dev);
    if(bind_res.first.code != VulkanContext::Result::error_code::SurfaceConnectionToDevice)
    {
        logger.log(bind_res.first);
        return Engine::Result::error_code::GraphicsDeviceInitError;
    }
    else
    {
        logger.log(bind_res.second);
        if(bind_res.second.code != GraphicsDevice::Result::error_code::Success)
            return Engine::Result::error_code::GraphicsDeviceInitError;
    }

    auto init_res = dynamic_cast<GraphicsDevice *>(weak_ptr_dev.get())->init(choosed_device.value());
    if(init_res.error.code != GraphicsDevice::Result::error_code::Success)
    {
        logger.log(init_res);
        return Engine::Result::error_code::GraphicsDeviceInitError;
    }

    GraphicsDeviceInfo target
    {
        .raw_device = weak_ptr_dev,
        .graphics_device = dynamic_cast<GraphicsDevice *>(weak_ptr_dev.get()),
        .ph_dev = choosed_device.value()
    };

    //explicitly changed!
	target_graphics_device = move(target);

    return Result::error_code::Success;
}

auto Engine::create_graphics_device_env() -> Engine::Result
{
	if(target_graphics_device.graphics_device->IsEnvCreated())
		return Result::error_code::Success;

    auto recv_shaders = target_graphics_device.graphics_device->QueryShaders();

    auto res = resource_manager.LoadShaders(recv_shaders);
    if(res.error.code != ResourceManager::Result::error_code::Success)
    {
        logger.log(res);
        return Engine::Result::error_code::GraphicsDeviceEnvironmentCreationError;
    }

    vector<GraphicsDevice::LoadedShaderProps> shaders;
    shaders.reserve(recv_shaders.size());
    for(size_t i = 0; i < recv_shaders.size(); i++)
        shaders.push_back(GraphicsDevice::LoadedShaderProps{.name = recv_shaders[i], .code = {}});

    res = resource_manager.GetShaders(shaders);
    if(res.error.code != ResourceManager::Result::error_code::Success)
    {
        logger.log(res);
        return Engine::Result::error_code::GraphicsDeviceEnvironmentCreationError;
    }

	auto win_res = window.get_drawable_size(settings.window_width.value, settings.window_height.value);
    if(win_res.code != SDLwindow::Result::error_code::Success)
    {
        logger.log(win_res);
        return Engine::Result::error_code::GraphicsDeviceEnvironmentCreationError;
    }

    auto env_res = target_graphics_device.graphics_device->CreateWorkEnv(GraphicsDevice::DrawableAreaParams
        {
			.width = static_cast<uint32_t>(settings.window_width.value),
			.height = static_cast<uint32_t>(settings.window_height.value),
            .mode = vk::PresentModeKHR::eImmediate
        }, shaders
    );

    if(env_res.error.code != GraphicsDevice::Result::error_code::Success)
    {
        logger.log(env_res);
        return Engine::Result::error_code::GraphicsDeviceEnvironmentCreationError;
    }

    return Engine::Result::error_code::Success;
}

/*auto Engine::switch_graphics_device(size_t ind) -> WarningLevel
{
    auto exp = drawing_context.FindOrAllocDeviceDriver<GraphicsDevice>(ind);
    if(!exp.has_value())
        return exp.error();

    auto weak_ptr_dev = exp.value();
    auto graphics_ptr = dynamic_cast<GraphicsDevice *>(weak_ptr_dev.lock().get());
    if(graphics_ptr->is_surface_connected())
    {
        if(graphics_ptr->is_inited())
        {
            GraphicsDeviceInfo target
            {
                .raw_device = weak_ptr_dev,
                .graphics_device = graphics_ptr,
                .ph_dev_ind = ind
            };

            target_graphics_device = move(target);
        }
        else
        {
            auto res = drawing_context.InitDeviceDriver(weak_ptr_dev);
            GraphicsDeviceInfo target
            {
                .raw_device = weak_ptr_dev,
                .graphics_device = graphics_ptr,
                .ph_dev_ind = ind
            };

            target_graphics_device = move(target);

            return res;
        }
    }
    else
    {
        auto res = drawing_context.BindSurface<GraphicsDevice>(weak_ptr_dev);
        if(res.is_fatal_error())
            return res;

        res = drawing_context.InitDeviceDriver(weak_ptr_dev);
        if(res.is_fatal_error())
            return res;

        GraphicsDeviceInfo target
        {
            .raw_device = weak_ptr_dev,
            .graphics_device = graphics_ptr,
            .ph_dev_ind = ind
        };

        //explicitly changed!
        target_graphics_device = move(target);
    }
}*/

Engine::Engine()
{
    is_initizalized = false;
    is_run = false;
    is_settings_changed = false;
}

Engine::~Engine()
{
	auto res = settings.write_settings("./settings.conf");
	logger.log(res);
    if(is_initizalized)
        logger.log("Engine terminated successfully");
	is_run = false;
	is_initizalized = false;
}

#define INIT_MODULE(name) \
            { \
                auto res = name(); \
                logger.log(res); \
                if(res != Result::error_code::Success) \
                    return res; \
            } \

auto Engine::init(int argc, char **argv) -> Engine::Result
{
    INIT_MODULE(init_logger)
	INIT_MODULE(init_settings)
    INIT_MODULE(init_window)
    INIT_MODULE(init_draw_context)
    INIT_MODULE(init_resource_manager)
    INIT_MODULE(init_inter_module_connection)
    INIT_MODULE(init_devices)
    INIT_MODULE(init_graphics_device)
    INIT_MODULE(create_graphics_device_env)

    is_initizalized = true;

    logger.log("Engine is inited successfully!");
    return Engine::Result::error_code::Success;
}

#undef INIT_MODULE

auto Engine::run() -> Engine::Result
{
    if(!is_initizalized)
        return Result::error_code::EngineNotInited;

    logger.log("Engine is running!");

    is_run = true;

	main_player.GetPOV().GetProjection() = twv::Perspective(0.1f, 100.0f, 90.0f, static_cast<float>(settings.window_width.value) / settings.window_height.value);

    GraphicsDevice::Result res;

	//main_player.TranslateForward(2);
	//main_player.GetPOV().GetViewTranslate()[2] += 2.0f;
	//main_player.GetPOV().GetView()[3][2] += 2.0f;

	main_player.TranslateForward(-2);
	//main_player.GetYaw() = -45.0f;
	main_player.SetupYawPitch();
	twv::Print(main_player.GetPOV().GetViewRotate());
	twv::Print(main_player.GetPOV().GetCommonMatrix());

	auto on_events_end = [&]()
	{
		const float delta_add = 0.001f;

		switch(main_player.GetForwardRunState())
		{
			case Player::RunState::None:
				break;
			case Player::RunState::Positive:
				main_player.TranslateForward(delta_add);
				//main_player.GetPOV().GetView()[3] += main_player.GetPOV().GetView()[2] * delta_add;
				break;
			case Player::RunState::Negative:
				main_player.TranslateForward(-delta_add);
				//main_player.GetPOV().GetView()[3] -= main_player.GetPOV().GetView()[2] * delta_add;
				break;
		}

		switch(main_player.GetAsideRunState())
		{
			case Player::RunState::None:
				break;
			case Player::RunState::Positive:
				main_player.TranslateAside(delta_add);
				//main_player.GetPOV().GetView()[3] += main_player.GetPOV().GetView()[0] * delta_add;
				break;
			case Player::RunState::Negative:
				main_player.TranslateAside(-delta_add);
				//main_player.GetPOV().GetView()[3] -= main_player.GetPOV().GetView()[0] * delta_add;
				break;
		}
	};


    while(is_run)
    {
        window.handle_all_events();

		on_events_end();


		//rotate_matrix = twv::RotateMatrix(twv::glsl::Vec3{sinf(start_rot), cosf(start_rot), sqrtf(powf(sinf(start_rot), 2) + powf(cosf(start_rot), 2))}, start_rot);
		//rotate_matrix = twv::RotateMatrix(twv::glsl::Vec3{0.0f, 1.0f, 0.0f}, start_rot);

		//twv::Print(main_player.GetPOV().GetProjection());
		//twv::Print(main_player.GetPOV().GetView());
		res = target_graphics_device.graphics_device->Draw(main_player.GetPOV().GetCommonMatrix());
		//twv::Print(main_player.GetForwardDir());
		//twv::Print(main_player.GetPOV().GetCommonMatrix());
		//twv::Print(rotate_matrix * model_matrix * proj_matrix);
        if(res.code != GraphicsDevice::Result::error_code::Success)
        {
            logger.log(res);
            return Engine::Result::error_code::RuntimeError;
        }
    }

    logger.log("Engine running is stoped!");
    return Engine::Result::error_code::Success;
}

auto Engine::get_settings() -> Settings
{
    return settings;
}
