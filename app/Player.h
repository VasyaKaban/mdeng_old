#pragma once

#include "POV.h"

class Player
{
public:

	enum class RunState : uint8_t
	{
		None = 0,
		Positive = 1,
		Negative = 2
	};


	Player();
	~Player() = default;
	Player(const Player &pl);
	Player(Player &&pl) noexcept = default;

	auto GetPOV() -> POV &;

	auto GetForwardRunState() -> RunState;
	auto GetAsideRunState() -> RunState;
	auto SetForwardRunState(RunState state) -> void;
	auto SetAsideRunState(RunState state) -> void;

	auto GetYaw() -> float &;
	auto GetPitch() -> float &;
	auto SetupYawPitch() -> void;

	auto GetForwardDir() -> twv::glsl::Vec4 &;
	auto GetAsideDir() -> twv::glsl::Vec4 &;
	auto GetUpDir() -> twv::glsl::Vec4 &;

	auto TranslateForward(float factor) -> void;
	auto TranslateAside(float factor) -> void;
	auto TranslateUp(float factor) -> void;

private:
	RunState forward;
	RunState aside;

	float yaw;
	float pitch;

	POV pov;
};
