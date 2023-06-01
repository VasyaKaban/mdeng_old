#pragma once

#include "../math/Mat.hpp"

class POV
{
public:
	POV();
	~POV() = default;
	POV(const POV &pov);
	POV(POV &&pov) noexcept = default;

	auto GetProjection() -> twv::glsl::Mat4x4 &;
	auto GetViewRotate() -> twv::glsl::Mat4x4 &;
	auto GetViewTranslate() -> twv::glsl::Vec3 &;
	auto GetCommonMatrix() -> twv::glsl::Mat4x4;

	auto operator=(const POV &pov) -> POV &;
	auto operator=(POV &&pov) noexcept -> POV & = default;

private:
	twv::glsl::Mat4x4 projection;
	twv::glsl::Mat4x4 view_rotate;
	twv::glsl::Vec3 view_translate;
};
