#pragma once

#include <tuple>
#include "GraphicsDevice.h"
#include "Logger.h"
#include "SDLwindow.h"
#include "VulkanContext.h"
#include "ResourceManager.h"
#include "Settings.h"
#include <variant>
#include "app/Player.h"

class Engine
{
public:
	struct GraphicsDeviceInfo
	{
		hrs::SpectatorPtr<VulkanDeviceDriver> raw_device;
		GraphicsDevice *graphics_device = nullptr;
		vk::PhysicalDevice ph_dev;
	};

	struct Result
    {
        enum class error_code : uint8_t
        {
            //common
            Success,

			//Work
			EngineNotInited,
			NoAvailableDevices,


			//Defs
			AvailableDevicesList,
			ChoosedDeviceInstead,

			//Init
			SettingsInitError,
			LoggerInitError,
			WindowInitError,
			DrawContextInitError,
			ResourceManagerInitError,
			IMCInitError,
			DevicesInitError,
			GraphicsDeviceInitError,
			GraphicsDeviceEnvironmentCreationError,

			//Run
			RuntimeError

			// I/O
            //IOOpenError,
            //WriteError,
            //ReadError,
        } code;

        constexpr Result(error_code err = error_code::Success) : code(err)
        {}

        constexpr auto message() const -> std::string_view;
        constexpr auto to_view() const -> std::string_view;

        constexpr auto operator=(const Result::error_code err) -> Result &;

        constexpr friend auto operator==(const Result &res, const Result::error_code &err_code) -> bool;
    };

    static_assert(hrs::ResultType<Result>);

private:
	Logger logger;
	SDLwindow window;
	VulkanContext drawing_context;
	ResourceManager resource_manager;
 	Settings settings;
	GraphicsDeviceInfo target_graphics_device;


	bool is_run;
	bool is_initizalized;
	bool is_settings_changed;


	Player main_player;

private:
	auto init_settings() -> Engine::Result;
	auto init_logger() -> Engine::Result;
	auto init_window() -> Engine::Result;
	auto init_draw_context() -> Engine::Result;
	auto init_resource_manager() -> Engine::Result;
	auto init_inter_module_connection() -> Engine::Result;
	auto init_devices() -> Engine::Result;
	auto init_graphics_device() -> Engine::Result;
	auto create_graphics_device_env() -> Engine::Result;
	//auto switch_graphics_device(size_t ind) -> WarningLevel;
public:
	Engine();
	~Engine();
	Engine(const Engine &app) = delete;
	Engine(Engine &&app) noexcept = delete;

	auto init(int argc, char **argv) -> Result;
	auto run() -> Result;
	auto get_settings() -> Settings;

	auto is_inited() -> bool;
};

constexpr auto Engine::Result::operator=(const Engine::Result::error_code err) -> Engine::Result &
{
    code = err;
    return *this;
}

constexpr auto operator==(const Engine::Result &res, const Engine::Result::error_code &err_code) -> bool
{
    return res.code == err_code;
}

constexpr auto Engine::Result::message() const -> std::string_view
{
    std::string_view res;
    switch(code)
    {
        case Result::error_code::Success:
            res = "Successful engine operation";
            break;
        case Result::error_code::EngineNotInited:
            res = "Engine isn't initialized yet";
            break;
        case Result::error_code::NoAvailableDevices:
            res = "No available devices found";
            break;
		case Result::error_code::AvailableDevicesList:
            res = "Found available devices list";
            break;
		case Result::error_code::ChoosedDeviceInstead:
            res = "Another device choosed instead desired";
            break;
		case Result::error_code::SettingsInitError:
            res = "Settings initialization error";
            break;
		case Result::error_code::LoggerInitError:
            res = "Logger initialization error";
            break;
		case Result::error_code::WindowInitError:
            res = "Window initialization error";
            break;
		case Result::error_code::DrawContextInitError:
            res = "Drawing context initialization error";
            break;
		case Result::error_code::IMCInitError:
            res = "IMC initialization error";
            break;
		case Result::error_code::ResourceManagerInitError:
            res = "Resource manager initialization error";
            break;
		case Result::error_code::DevicesInitError:
            res = "Devices initialization error";
            break;
		case Result::error_code::GraphicsDeviceInitError:
            res = "Graphics device initialization error";
            break;
		case Result::error_code::GraphicsDeviceEnvironmentCreationError:
            res = "Graphics device environment creation error";
            break;
		case Result::error_code::RuntimeError:
            res = "Runtime error";
            break;
    }

    return res;
}

constexpr auto Engine::Result::to_view() const -> std::string_view
{
    std::string_view res;
    switch(code)
    {
        case Result::error_code::Success:
            res = "Success";
            break;
        case Result::error_code::EngineNotInited:
            res = "EngineNotInited";
            break;
        case Result::error_code::NoAvailableDevices:
            res = "NoAvailableDevices";
            break;
		case Result::error_code::AvailableDevicesList:
            res = "AvailableDevicesList";
            break;
		case Result::error_code::ChoosedDeviceInstead:
            res = "ChoosedDeviceInstead";
            break;
		case Result::error_code::SettingsInitError:
            res = "SettingsInitError";
            break;
		case Result::error_code::LoggerInitError:
            res = "LoggerInitError";
            break;
		case Result::error_code::WindowInitError:
            res = "WindowInitError";
            break;
		case Result::error_code::DrawContextInitError:
            res = "DrawContextInitError";
            break;
		case Result::error_code::IMCInitError:
            res = "IMCInitError";
            break;
		case Result::error_code::ResourceManagerInitError:
            res = "ResourceManagerInitError";
            break;
		case Result::error_code::DevicesInitError:
            res = "DevicesInitError";
            break;
		case Result::error_code::GraphicsDeviceInitError:
            res = "GraphicsDeviceInitError";
            break;
		case Result::error_code::GraphicsDeviceEnvironmentCreationError:
            res = "GraphicsDeviceEnvironmentCreationError";
            break;
		case Result::error_code::RuntimeError:
            res = "RuntimeError";
            break;
    }

    return res;
}
