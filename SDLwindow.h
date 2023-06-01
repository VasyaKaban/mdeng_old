#pragma once

#include <SDL2/SDL.h>
#include <map>
#include <string>
#include <memory>
#include <functional>
#include <optional>
#include <type_traits>
#include <concepts>
#include <tuple>
#include "utils/ResultDef.hpp"
#include "VulkanContext.h"

class SDLwindow
{
public:
	struct Callbacks
	{
		enum class EventRepresentation : uint8_t
		{
			Quit = 0,
			MouseMotion = 1,
			MouseWheel = 2,
			WindowEvent = 3,
			KeyboardKey = 4,
			MouseButton = 5,
		};

		struct MouseCoord
		{
			Sint32 x;
			Sint32 y;

			auto operator-=(const MouseCoord &mc) -> MouseCoord &
			{
				x -= mc.x;
				y -= mc.y;
				return *this;
			}
		};

		struct MouseWheel
		{
			MouseCoord coord;
			Sint32 delta_x;
			Sint32 delta_y;
		};

		std::function<auto() -> void> quit_callback;//Quit
        std::function<auto(MouseCoord) -> void> mousemotion_callback;//Mousemotion
        std::function<auto(MouseWheel) -> void> mousewheel_callback;//MouseWheelEvent
		std::function<auto(SDL_WindowEventID) -> void> window_event_callback;//WindowEvent
		std::function<auto(SDL_KeyCode, bool) -> void> keyboard_key_callback;//KeyboardKey
		std::function<auto(Uint8, MouseCoord, bool) -> void> mouse_button_callback;//MouseButton

        ~Callbacks() = default;
	};

	struct Result
    {
        enum class error_code : uint8_t
        {
            //common
            Success,

			//inner window
			InnerWindowError,

			//Objects
			WindowNotCreated,
			VkSurfaceConnectedToAnotherWin


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
	SDL_Window *win;

	std::weak_ptr<vk::SurfaceKHR> connected_surface;

	Callbacks callbacks;
	std::vector<SDL_Event> quit_events_batch;
	std::vector<SDL_Event> window_events_batch;
	std::vector<SDL_Event> keyboard_events_batch;
	std::vector<SDL_Event> mouse_events_batch;
public:
	SDLwindow();
	SDLwindow(const SDLwindow &sdl_win) = delete;
	SDLwindow(SDLwindow &&sdl_win) noexcept;
	~SDLwindow();


	auto init(const std::string &title, int width, int height, bool is_full,
			  int quit_batches_count = 1, int window_batches_count = 16,
			  int mouse_batches_count = 16, int keyboard_batches_count = 16) -> hrs::ResultDef<SDLwindow::Result>;

	auto get_extensions(std::vector<const char *> &extensions) -> hrs::ResultDef<SDLwindow::Result>;
	auto resize(int width, int height) -> void;
	auto get_drawable_size(int &width, int &height) -> Result;

	auto handle_all_events() -> void;//SDL_PoolEvent
	auto update_event_queue() -> void;//SDL_PumpEvents

	auto set_quit_event_batches_count(int count) -> void;
	auto handle_quit_event() -> void;//SDL_PeepEvents

	auto set_window_events_batches_count(int count) -> void;
	auto handle_window_events() -> void;

	auto set_keyboard_events_batches_count(int count) -> void;
	auto handle_keyboard_events() -> void;

	auto set_mouse_events_batches_count(int count) -> void;
	auto handle_mouse_events() -> void;

	auto vulkan_surface_handshake(vk::Instance &instance, std::shared_ptr<vk::SurfaceKHR> &surface) -> hrs::ResultDef<SDLwindow::Result>;

	operator bool() const;

	template<std::convertible_to<decltype(Callbacks::quit_callback)> F>
	auto set_quit_callback(F &&f) -> void;

	template<std::convertible_to<decltype(Callbacks::mousemotion_callback)> F>
	auto set_mousemotion_callback(F &&f) -> void;

	template<std::convertible_to<decltype(Callbacks::mousewheel_callback)> F>
	auto set_mousewheel_callback(F &&f) -> void;

	template<std::convertible_to<decltype(Callbacks::window_event_callback)> F>
	auto set_window_event_callback(F &&f) -> void;

	template<std::convertible_to<decltype(Callbacks::keyboard_key_callback)> F>
	auto set_keyboard_key_callback(F &&f) -> void;

	template<std::convertible_to<decltype(Callbacks::mouse_button_callback)> F>
	auto set_mouse_button_callback(F &&f) -> void;

};

constexpr auto SDLwindow::Result::operator=(const SDLwindow::Result::error_code err) -> SDLwindow::Result &
{
    code = err;
    return *this;
}

constexpr auto operator==(const SDLwindow::Result &res, const SDLwindow::Result::error_code &err_code) -> bool
{
    return res.code == err_code;
}

constexpr auto SDLwindow::Result::message() const -> std::string_view
{
    std::string_view res;
    switch(code)
    {
        case Result::error_code::Success:
            res = "SDLwindow successful operation";
            break;
        case Result::error_code::InnerWindowError:
            res = "Underlaying window subsystem error";
            break;
        case Result::error_code::WindowNotCreated:
            res = "Window isn't created yet";
            break;
        case Result::error_code::VkSurfaceConnectedToAnotherWin:
            res = "Vulkan surface is connected to another window object";
            break;
    }

    return res;
}

constexpr auto SDLwindow::Result::to_view() const -> std::string_view
{
    std::string_view res;
    switch(code)
    {
        case Result::error_code::Success:
            res = "Success";
            break;
        case Result::error_code::InnerWindowError:
            res = "InnerWindowError";
            break;
        case Result::error_code::WindowNotCreated:
            res = "WindowNotCreated";
            break;
        case Result::error_code::VkSurfaceConnectedToAnotherWin:
            res = "VkSurfaceConnectedToAnotherWin";
            break;
    }

    return res;
}

template<std::convertible_to<decltype(SDLwindow::Callbacks::quit_callback)> F>
auto SDLwindow::set_quit_callback(F &&f) -> void
{
	callbacks.quit_callback = std::forward<F>(f);
}

template<std::convertible_to<decltype(SDLwindow::Callbacks::mousemotion_callback)> F>
auto SDLwindow::set_mousemotion_callback(F &&f) -> void
{
	callbacks.mousemotion_callback = std::forward<F>(f);
}

template<std::convertible_to<decltype(SDLwindow::Callbacks::mousewheel_callback)> F>
auto SDLwindow::set_mousewheel_callback(F &&f) -> void
{
	callbacks.mousewheel_callback = std::forward<F>(f);
}

template<std::convertible_to<decltype(SDLwindow::Callbacks::window_event_callback)> F>
auto SDLwindow::set_window_event_callback(F &&f) -> void
{
	callbacks.window_event_callback = std::forward<F>(f);
}

template<std::convertible_to<decltype(SDLwindow::Callbacks::keyboard_key_callback)> F>
auto SDLwindow::set_keyboard_key_callback(F &&f) -> void
{
	callbacks.keyboard_key_callback = std::forward<F>(f);
}

template<std::convertible_to<decltype(SDLwindow::Callbacks::mouse_button_callback)> F>
auto SDLwindow::set_mouse_button_callback(F &&f) -> void
{
	callbacks.mouse_button_callback = std::forward<F>(f);
}
