cmake_minimum_required(VERSION 3.5)

project(mdeng LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(Sources
    main.cpp
    VulkanContext.cpp
    VulkanContext.h
    utils/expected.hpp
    Engine.h
    Engine.cpp
    SDLwindow.h
    SDLwindow.cpp
    VulkanDeviceDriver.h
    ResourceManager.h
    ResourceManager.cpp
    Logger.h
    Logger.cpp
    VulkanInclude.h
    GraphicsDevice.h
    GraphicsDevice.cpp
    Settings.h
    Settings.cpp
    utils/ResultDef.hpp
    utils/ControlBlock.hpp
	math/Vec.hpp
	math/Mat.hpp
	math/Math.hpp
	app/POV.cpp
	app/POV.h
	app/Player.h
	app/Player.cpp
)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "../../out/debug_bin/")

add_executable(${PROJECT_NAME} ${Sources})


find_package(Vulkan REQUIRED)
find_package(SDL2 REQUIRED)

set(Libs ${SDL2_LIBRARIES} ${VULKAN_LIBRARIES})

target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan SDL2::SDL2)
