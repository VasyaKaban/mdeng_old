#include "POV.h"

using
	std::move;

POV::POV()
{
	projection = twv::glsl::Mat4x4::identity();
	view_rotate = twv::glsl::Mat4x4::identity();
}

POV::POV(const POV &pov)
{
	projection = pov.projection;
	view_rotate = pov.view_rotate;
	view_translate = pov.view_translate;
}

auto POV::GetProjection() -> twv::glsl::Mat4x4 &
{
	return projection;
}

auto POV::GetViewRotate() -> twv::glsl::Mat4x4 &
{
	return view_rotate;
}

auto POV::GetViewTranslate() -> twv::glsl::Vec3 &
{
	return view_translate;
}

auto POV::GetCommonMatrix() -> twv::glsl::Mat4x4
{
	twv::glsl::Mat4x4 translate = twv::glsl::Mat4x4::identity();
	translate[3] = view_translate;
	translate[3][0] = -translate[3][0];
	translate[3][1] = -translate[3][1];
	translate[3][2] = -translate[3][2];

	return translate * view_rotate.transpose() * projection;
}

auto POV::operator=(const POV &pov) -> POV &
{
	projection = pov.projection;
	view_rotate = pov.view_rotate;
	view_translate = pov.view_translate;
	return *this;
}
