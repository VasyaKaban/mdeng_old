#include "Engine.h"
#include <iostream>
#include <type_traits>

#include "math/Math.hpp"

using
	std::vector,
	std::string_view,
	std::cout,
	std::endl;


auto main(int argc, char **argv) -> int
{
	Engine engine;
	auto res = engine.init(argc, argv);
	if(res.code != Engine::Result::error_code::Success)
		return EXIT_FAILURE;

	res = engine.run();
	if(res.code != Engine::Result::error_code::Success)
		return EXIT_FAILURE;


	return EXIT_SUCCESS;
}
