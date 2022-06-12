#include "BalancerSystem.h"

BalancerSystem::BalancerSystem()
{
	manualOutput = std::vector<float>(GetOutputCount());
	minecart.Load("minecart.png");
}

void BalancerSystem::SetDefaultDataPack(std::minstd_rand& random)
{
	defaultDataPack.cartVelocity = 0;
	defaultDataPack.cartAcceleration = 0;
	defaultDataPack.poleVelocity = 0;
	defaultDataPack.poleAcceleration = 0;
	defaultDataPack.pole2Velocity = 0;
	defaultDataPack.pole2Acceleration = 0;
	defaultDataPack.poleAngle = 0;
	defaultDataPack.pole2Angle = 0;
	defaultDataPack.cartPosition = 0;
}

void BalancerSystem::StepOrganism(DataPack* data, const float* networkOutputs, float& fitness, bool& continueStepping)
{
	BalancerDataPack& info = *(BalancerDataPack*)data;
	float networkOutput = *networkOutputs;
	//fitness += TIME_STEP; //fitness == time
	//fitness += (POLE_FAILURE_ANGLE - glm::abs( system.poleAngle); //just to remove the spice, the less wobbly the sticks the better
	fitness += glm::abs(info.poleAngle) + glm::abs(info.pole2Angle); //just to add more spice, the wobblier the sticks the better
	//fitness += glm::abs(system.pole2Angle - system.poleAngle); //the further apart the sticks, the more points
	float force = glm::sign(networkOutput - 0.5f) * MOVEMENT_SPEED;

	info.cartPosition += TIME_STEP * info.cartVelocity;
	info.cartVelocity += TIME_STEP * info.cartAcceleration;
	info.poleAngle += TIME_STEP * info.poleVelocity;
	info.pole2Angle += TIME_STEP * info.pole2Velocity;
	info.poleVelocity += TIME_STEP * info.poleAcceleration;
	info.pole2Velocity += TIME_STEP * info.pole2Acceleration;

	float iMass = 1.0f / (SPACE_MAX_HEIGHT + POLE_MAX_HEIGHT);
	float cosAngle = glm::cos(info.poleAngle);
	float sinAngle = glm::sin(info.poleAngle);

	info.cartAcceleration = (force + POLE_MAX_HEIGHT * SPACE_MIN_HEIGHT * (info.poleVelocity * info.poleVelocity * sinAngle - info.poleAcceleration * cosAngle)) * iMass;

	info.poleAcceleration = GRAVITY * sinAngle + cosAngle *
		(-force - POLE_MAX_HEIGHT * SPACE_MIN_HEIGHT * info.poleVelocity * info.poleVelocity * sinAngle * iMass);
	info.poleAcceleration /= SPACE_MIN_HEIGHT * (4.0f / 3.0f - POLE_MAX_HEIGHT * cosAngle * cosAngle * iMass);
	info.pole2Acceleration = GRAVITY * sinAngle + cosAngle *
		(-force - BAR_MIN_HEIGHT * POLE_2_LENGTH * info.pole2Velocity * info.pole2Velocity * sinAngle * iMass);
	info.pole2Acceleration /= POLE_2_LENGTH * (4.0f / 3.0f - BAR_MIN_HEIGHT * cosAngle * cosAngle * iMass);

	if (glm::abs(info.poleAngle) >= POLE_FAILURE_ANGLE || glm::abs(info.pole2Angle) >= POLE_FAILURE_ANGLE
		|| glm::abs(info.cartPosition) >= TRACK_LIMIT)
	{
		continueStepping = false;
	}
}

void BalancerSystem::SetNetworkInputs(DataPack* data, float* networkInputArray)
{
	BalancerDataPack& info = *(BalancerDataPack*)data;

	networkInputArray[0] = info.cartPosition;
	networkInputArray[1] = info.poleAngle;
	networkInputArray[2] = info.pole2Angle;
	networkInputArray[3] = info.cartVelocity;
	networkInputArray[4] = info.poleVelocity;
	networkInputArray[5] = info.pole2Velocity;
}

void BalancerSystem::ResetManualOutput()
{
}

void BalancerSystem::StartOrganismPreview(bool manual, Renderer& renderer)
{
	manualOutput[0] = 0;
}

void BalancerSystem::OnKeyPressed(Renderer& renderer, int keycode, int action, bool manual)
{
	if (!manual)
		return;

	if (keycode == GLFW_KEY_RIGHT)
	{
		if (action == GLFW_PRESS)
			manualOutput[0] = 1;
		else
			manualOutput[0] = 0;
	}
}

void BalancerSystem::DrawGame(DataPack* data, Renderer& renderer)
{
	renderer.SetLineWidth(3);

	BalancerDataPack& info = *(BalancerDataPack*)data;
	float size = 95.0f;
	float borderSize = std::min(size / 100.0f, 1.0f);
	float floorSize = 20.0f;
	renderer.DrawBox(glm::vec2(-size/2 + borderSize/2,0), borderSize, size, 0, glm::vec3(0.2f));
	renderer.DrawBox(glm::vec2(size/2 - borderSize/2,0), borderSize, size, 0, glm::vec3(0.2f));
	renderer.DrawBox(glm::vec2(0, size / 2 - borderSize / 2), size, borderSize, 0, glm::vec3(0.2f));
	renderer.DrawBox(glm::vec2(0, -size / 2 + borderSize / 2), size, borderSize, 0, glm::vec3(0.2f));
	size = size - borderSize * 2;
	float halfSize = size / 2.0f;

	float unit = (float)size / (TRACK_LIMIT * 2.0f + 2);
	short cartSizeY = unit * 0.5f;
	//floor
	renderer.DrawBox(glm::vec2(0, -halfSize + floorSize/2), size, floorSize, 0, glm::vec3(0.4f, 0.8f, 0.4f));
	//cart
	glm::vec2 cartStart = glm::vec2(unit * info.cartPosition, floorSize + cartSizeY/2 - halfSize);
	renderer.DrawSprite(&minecart, cartStart, cartSizeY * 2, cartSizeY * 2 * 0.8f, 0);

	glm::vec2 start = glm::vec2(cartStart.x , cartStart.y + cartSizeY / 2);
	glm::vec2 offset = (SPACE_MIN_HEIGHT * 2.0f * unit) * glm::vec2(glm::sin(info.poleAngle), glm::cos(info.poleAngle));
	renderer.DrawLine(start, start + offset, glm::vec3(0, 0, 1));
	glm::vec2 offset2 = (POLE_2_LENGTH * 2.0f * unit) * glm::vec2(glm::sin(info.pole2Angle), glm::cos(info.pole2Angle));
	renderer.DrawLine(start, start + offset2, glm::vec3(0,1.0f,0));

	//cart size y is also half of cartsize x
	float limit = halfSize - cartSizeY;
	glm::vec3 red(1.0f, 0, 0);
	renderer.DrawLine(glm::vec2(limit, floorSize - halfSize), glm::vec2(limit, halfSize), red);
	renderer.DrawLine(glm::vec2(-limit, floorSize - halfSize), glm::vec2(-limit, halfSize), red);

	static const float failCos = glm::cos(POLE_FAILURE_ANGLE);
	static const float failSin = glm::sin(POLE_FAILURE_ANGLE);
	glm::vec2 failOffset = (SPACE_MIN_HEIGHT * 2.0f * unit) * glm::vec2(failSin, failCos);
	renderer.DrawLine(start, start + failOffset, red);
	renderer.DrawLine(start, start + glm::vec2(-failOffset.x, failOffset.y), red);
}

GameSystem::DataPack* BalancerSystem::GetDefaultDataPack()
{
    return &defaultDataPack;
}

GameSystem::DataPack* BalancerSystem::NewDataPack() const
{
    return new BalancerDataPack();
}

void BalancerSystem::CopyDataPack(DataPack* dest, DataPack* src) const
{
	auto destt = (BalancerDataPack*)dest;
	*destt = *(BalancerDataPack*)src;
}