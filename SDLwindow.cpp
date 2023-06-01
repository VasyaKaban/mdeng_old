#include "SDLwindow.h"
#include <SDL2/SDL_vulkan.h>

using
	std::string;

SDLwindow::SDLwindow()
{
	win = nullptr;
}

SDLwindow::SDLwindow(SDLwindow &&sdl_win) noexcept
{
	win = sdl_win.win;
}

SDLwindow::~SDLwindow()
{
	SDL_DestroyWindow(win);
	SDL_Quit();
}

auto SDLwindow::init(const string &title, int width, int height, bool is_full,
					 int quit_batches_count, int window_batches_count,
					 int mouse_batches_count, int keyboard_batches_count) -> hrs::ResultDef<SDLwindow::Result>
{
	auto res = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
	if(res != 0)
		return {Result::error_code::InnerWindowError, SDL_GetError()};

	win = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
						   width, height, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | (is_full ? SDL_WINDOW_FULLSCREEN : 0));

	if(win == nullptr)
		return {Result::error_code::InnerWindowError, SDL_GetError()};

	set_quit_event_batches_count(quit_batches_count);
	set_window_events_batches_count(window_batches_count);
	set_mouse_events_batches_count(mouse_batches_count);
	set_keyboard_events_batches_count(keyboard_batches_count);

	return {Result::error_code::Success};
}

auto SDLwindow::get_extensions(std::vector<const char *> &extensions) -> hrs::ResultDef<SDLwindow::Result>
{
	unsigned int ext_cnt = 0;
	if(SDL_Vulkan_GetInstanceExtensions(win, &ext_cnt, nullptr) != SDL_TRUE)
		return {Result::error_code::InnerWindowError, SDL_GetError()};

	extensions.resize(ext_cnt);
	if(SDL_Vulkan_GetInstanceExtensions(win, &ext_cnt, extensions.data()) != SDL_TRUE)
		return {Result::error_code::InnerWindowError, SDL_GetError()};

	return {Result::error_code::Success};

}

auto SDLwindow::resize(int width, int height) -> void
{
	SDL_SetWindowSize(win, width, height);
}

auto SDLwindow::get_drawable_size(int &width, int &height) -> SDLwindow::Result
{
	if(win == nullptr)
		return Result::error_code::WindowNotCreated;

	SDL_Vulkan_GetDrawableSize(win, &width, &height);

	return Result::error_code::Success;
}

auto SDLwindow::handle_all_events() -> void
{
	SDL_Event ev;
	while(SDL_PollEvent(&ev))
	{
		switch(ev.type)
		{
			case SDL_EventType::SDL_QUIT:
				if(callbacks.quit_callback)
					callbacks.quit_callback();
				break;
			case SDL_EventType::SDL_MOUSEMOTION:
				if(callbacks.mousemotion_callback)
				{
					Callbacks::MouseCoord coord
					{
						.x = ev.motion.x,
						.y = ev.motion.y
					};
					callbacks.mousemotion_callback(coord);
				}
				break;
			case SDL_EventType::SDL_MOUSEWHEEL:
				if(callbacks.mousewheel_callback)
				{
					Callbacks::MouseWheel wheel_data
					{
						.coord
							{
								.x = ev.wheel.mouseX,
								.y = ev.wheel.mouseY
							},
						.delta_x = ev.wheel.x,
						.delta_y = ev.wheel.y
					};
					callbacks.mousewheel_callback(wheel_data);
				}
				break;
			case SDL_EventType::SDL_WINDOWEVENT:
				if(callbacks.window_event_callback)
					callbacks.window_event_callback(static_cast<SDL_WindowEventID>(ev.window.event));
				break;
			case SDL_EventType::SDL_KEYDOWN:
				if(callbacks.keyboard_key_callback)
					callbacks.keyboard_key_callback(static_cast<SDL_KeyCode>(ev.key.keysym.sym), true);
				break;
			case SDL_EventType::SDL_KEYUP:
				if(callbacks.keyboard_key_callback)
					callbacks.keyboard_key_callback(static_cast<SDL_KeyCode>(ev.key.keysym.sym), false);
				break;
			case SDL_EventType::SDL_MOUSEBUTTONDOWN:
				if(callbacks.mouse_button_callback)
				{
					Callbacks::MouseCoord coord
					{
						.x = ev.button.x,
						.y = ev.button.y
					};

					callbacks.mouse_button_callback(ev.button.button, coord, true);
				}
				break;
			case SDL_EventType::SDL_MOUSEBUTTONUP:
				if(callbacks.mouse_button_callback)
				{
					Callbacks::MouseCoord coord
					{
						.x = ev.button.x,
						.y = ev.button.y
					};

					callbacks.mouse_button_callback(ev.button.button, coord, false);
				}
				break;
		}
	}
}

auto SDLwindow::update_event_queue() -> void
{
	SDL_PumpEvents();
}

auto SDLwindow::set_quit_event_batches_count(int count) -> void
{
	if(count <= 0)
		return;

	quit_events_batch.resize(count);
}

auto SDLwindow::handle_quit_event() -> void
{
	if(quit_events_batch.empty())
		return;

	if(!callbacks.quit_callback)
		return;


	auto events_count = SDL_PeepEvents(quit_events_batch.data(), quit_events_batch.size(), SDL_GETEVENT, SDL_EventType::SDL_QUIT, SDL_EventType::SDL_QUIT);
	for(auto i = 0; i < events_count; i++)
			callbacks.quit_callback();
}

auto SDLwindow::set_window_events_batches_count(int count) -> void
{
	if(count <= 0)
		return;

	window_events_batch.resize(count);
}

auto SDLwindow::handle_window_events() -> void
{
	if(window_events_batch.empty())
		return;

	if(!callbacks.window_event_callback)
		return;


	auto events_count = SDL_PeepEvents(window_events_batch.data(), window_events_batch.size(), SDL_GETEVENT, SDL_EventType::SDL_WINDOWEVENT, SDL_EventType::SDL_WINDOWEVENT);
	for(auto i = 0; i < events_count; i++)
			callbacks.window_event_callback(static_cast<SDL_WindowEventID>(window_events_batch[i].window.event));
}

auto SDLwindow::set_keyboard_events_batches_count(int count) -> void
{
	if(count <= 0)
		return;

	keyboard_events_batch.resize(count);
}

auto SDLwindow::handle_keyboard_events() -> void
{
	if(keyboard_events_batch.empty())
		return;

	if(!callbacks.keyboard_key_callback)
		return;

	auto events_count = SDL_PeepEvents(keyboard_events_batch.data(), keyboard_events_batch.size(), SDL_GETEVENT, SDL_EventType::SDL_KEYDOWN, SDL_EventType::SDL_KEYUP);
	for(auto i = 0; i < events_count; i++)
	{
		bool is_pressed = false;
		if(keyboard_events_batch[i].key.state == SDL_PRESSED)
			is_pressed = true;
		callbacks.keyboard_key_callback(static_cast<SDL_KeyCode>(keyboard_events_batch[i].key.keysym.sym), is_pressed);
	}
}

auto SDLwindow::set_mouse_events_batches_count(int count) -> void
{
	if(count <= 0)
		return;

	mouse_events_batch.resize(count);
}

auto SDLwindow::handle_mouse_events() -> void
{
	if(mouse_events_batch.empty())
		return;

	if(!callbacks.mouse_button_callback && !callbacks.mousemotion_callback && !callbacks.mousewheel_callback)
		return;

	auto events_count = SDL_PeepEvents(mouse_events_batch.data(), mouse_events_batch.size(), SDL_GETEVENT, SDL_EventType::SDL_MOUSEMOTION, SDL_EventType::SDL_MOUSEWHEEL);
	for(auto i = 0; i < events_count; i++)
	{
		switch(mouse_events_batch[i].type)
		{
			case SDL_EventType::SDL_MOUSEMOTION:
				if(callbacks.mousemotion_callback)
				{
					Callbacks::MouseCoord coord
					{
						.x = mouse_events_batch[i].motion.x,
						.y = mouse_events_batch[i].motion.y
					};

					callbacks.mousemotion_callback(coord);
				}
				break;
			case SDL_EventType::SDL_MOUSEWHEEL:
				if(callbacks.mousewheel_callback)
				{
					Callbacks::MouseWheel wheel_data
					{
						.coord =
						{
							.x = mouse_events_batch[i].wheel.mouseX,
							.y = mouse_events_batch[i].wheel.mouseY
						},
						.delta_x = mouse_events_batch[i].wheel.x,
						.delta_y = mouse_events_batch[i].wheel.y
					};

					callbacks.mousewheel_callback(wheel_data);
				}
				break;
			case SDL_EventType::SDL_MOUSEBUTTONDOWN:
				if(callbacks.mouse_button_callback)
				{
					Callbacks::MouseCoord coord
					{
						.x = mouse_events_batch[i].button.x,
						.y = mouse_events_batch[i].button.y
					};

					callbacks.mouse_button_callback(mouse_events_batch[i].button.button, coord, true);
				}
				break;
			case SDL_EventType::SDL_MOUSEBUTTONUP:
				if(callbacks.mouse_button_callback)
				{
					Callbacks::MouseCoord coord
					{
						.x = mouse_events_batch[i].button.x,
						.y = mouse_events_batch[i].button.y
					};

					callbacks.mouse_button_callback(mouse_events_batch[i].button.button, coord, false);
				}
				break;
		}
	}
}

auto SDLwindow::vulkan_surface_handshake(vk::Instance &instance, std::shared_ptr<vk::SurfaceKHR> &surface) -> hrs::ResultDef<SDLwindow::Result>
{
	if(win == nullptr)
		return {Result::error_code::WindowNotCreated};

	if(!connected_surface.expired())
		return {Result::error_code::VkSurfaceConnectedToAnotherWin};

	VkSurfaceKHR tmp_surface;
	auto res = SDL_Vulkan_CreateSurface(win, instance, &tmp_surface);
	if(res != SDL_TRUE)
		return {Result::error_code::InnerWindowError, SDL_GetError()};

	surface.reset(new vk::SurfaceKHR);
	(*surface) = tmp_surface;
	connected_surface = surface;

	return {Result::error_code::Success};
}

SDLwindow::operator bool() const
{
	return (win != nullptr);
}
