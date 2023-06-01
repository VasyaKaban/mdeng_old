#include "Player.h"
#include "../math/Math.hpp"

Player::Player()
{
	forward = RunState::None;
	aside = RunState::None;
	yaw = 0.0f;
	pitch = 0.0f;
}

Player::Player(const Player &pl)
{
	forward = pl.forward;
	aside = pl.aside;
	pov = pl.pov;
	yaw = pl.yaw;
	pitch = pl.pitch;
}

auto Player::GetPOV() -> POV &
{
	return pov;
}

auto Player::GetForwardRunState() -> Player::RunState
{
	return forward;
}

auto Player::GetAsideRunState() -> Player::RunState
{
	return aside;
}

auto Player::SetForwardRunState(Player::RunState state) -> void
{
	forward = state;
}

auto Player::SetAsideRunState(Player::RunState state) -> void
{
	aside = state;
}

auto Player::GetYaw() -> float &
{
	return yaw;
}

auto Player::GetPitch() -> float &
{
	return pitch;
}

auto Player::SetupYawPitch() -> void
{
	auto yaw_tmp = twv::RotateMatrix(twv::glsl::Vec3{0.0f, 1.0f, 0.0f}, yaw);
	auto pitch_tmp = twv::RotateMatrix(static_cast<twv::glsl::Vec3>(yaw_tmp[0]), pitch);
	pov.GetViewRotate() = yaw_tmp * pitch_tmp;
}

auto Player::GetForwardDir() -> twv::glsl::Vec4 &
{
	return pov.GetViewRotate()[2];
}

auto Player::GetAsideDir() -> twv::glsl::Vec4 &
{
	return pov.GetViewRotate()[0];
}

auto Player::GetUpDir() -> twv::glsl::Vec4 &
{
	return pov.GetViewRotate()[1];
}

auto Player::TranslateForward(float factor) -> void
{
	twv::glsl::Vec3 forward_move = pov.GetViewRotate()[2];
	forward_move *= factor;
	pov.GetViewTranslate() += forward_move;
}

auto Player::TranslateAside(float factor) -> void
{
	twv::glsl::Vec3 forward_move = pov.GetViewRotate()[0];
	forward_move *= factor;
	pov.GetViewTranslate() += forward_move;
}

auto Player::TranslateUp(float factor) -> void
{
	twv::glsl::Vec3 forward_move = pov.GetViewRotate()[1];
	forward_move *= factor;
	pov.GetViewTranslate() += forward_move;
}
