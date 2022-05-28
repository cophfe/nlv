#include "FlappyBirdSystem.h"

FlappyBirdSystem::FlappyBirdSystem()
{
	manualOutput = std::vector<float>(1);
	manualOutput[0] = 0;
	bird.Load("bird.png");
}

FlappyBirdSystem::~FlappyBirdSystem()
{
}

void FlappyBirdSystem::SetDefaultSystem(std::minstd_rand& random)
{
	std::uniform_real_distribution<float> dist(0.0f, 1.0f);
	defaultDataPack.yVelocity = 0;
	defaultDataPack.yPos = SCREEN_HALF_HEIGHT;
	//SCREEN_WIDTH is on the absolute rightmost end of the screen, 0 is where the bird is
	defaultDataPack.barXPos = BAR_DISTANCE;
	defaultDataPack.barHeight = dist(random) * (BAR_MAX_HEIGHT - BAR_MIN_HEIGHT) + BAR_MIN_HEIGHT;
	defaultDataPack.spaceHeight = dist(random) * (SPACE_MAX_HEIGHT - SPACE_MIN_HEIGHT) + SPACE_MIN_HEIGHT;
	defaultDataPack.random = random;

}

void FlappyBirdSystem::StepOrganism(DataPack* data, const float* networkOutput, float& fitness, bool& continueStepping)
{
	//fitness is basically distance
	fitness += TIME_STEP * MOVEMENT_SPEED;

	std::uniform_real_distribution<float> dist(0.0f, 1.0f);
	FlappyBirdDataPack& dP = *(FlappyBirdDataPack*)data;
	
	dP.yPos += TIME_STEP * dP.yVelocity;
	if (*networkOutput)
		dP.yVelocity = JUMP_FORCE;
	dP.yVelocity += TIME_STEP * GRAVITY;
	
	//bar moves towards bird
	//theres only ever one bar
	dP.barXPos -= TIME_STEP * MOVEMENT_SPEED;
	
	if (dP.barXPos + BAR_HALF_WIDTH + BIRD_RADIUS < 0)
	{
		//go to next bar
		dP.barHeight = dist(dP.random) * (BAR_MAX_HEIGHT - BAR_MIN_HEIGHT) + BAR_MIN_HEIGHT;
		dP.spaceHeight = dist(dP.random) * (SPACE_MAX_HEIGHT - SPACE_MIN_HEIGHT) + SPACE_MIN_HEIGHT;
		dP.barXPos = BAR_DISTANCE + BAR_HALF_WIDTH;
	}

	//if at the bottom or top of the screen
	if (dP.yPos + BIRD_RADIUS >= SCREEN_HALF_HEIGHT * 2 || dP.yPos - BIRD_RADIUS <= 0)
	{
		continueStepping = false;
		return;
	}
	//if collide with bar
	float yDelta = dP.barHeight - dP.yPos;
	float xDelta = dP.barXPos - BAR_HALF_WIDTH;
	if ((xDelta * xDelta + yDelta * yDelta < BIRD_RADIUS * BIRD_RADIUS)
		|| (xDelta < 0 && (dP.yPos < dP.barHeight || dP.yPos > dP.barHeight + dP.spaceHeight)))
	{
		continueStepping = false;
		return;
	}
}

void FlappyBirdSystem::SetNetworkInputs(DataPack* data, float* networkInputArray)
{
	FlappyBirdDataPack& dP = *(FlappyBirdDataPack*)data;

	networkInputArray[0] = dP.barXPos;
	networkInputArray[1] = dP.barHeight;
	networkInputArray[2] = dP.spaceHeight;
	networkInputArray[3] = dP.yPos;
	networkInputArray[4] = dP.yVelocity;
}

void FlappyBirdSystem::ResetManualOutput()
{
	manualOutput[0] = 0;
}

void FlappyBirdSystem::OnKeyPressed(GLFWwindow* window, int keycode, int action)
{
	if (action == GLFW_PRESS && keycode == GLFW_KEY_SPACE)
		manualOutput[0] = 1;
}

void FlappyBirdSystem::DrawGame(DataPack* data, Renderer& renderer)
{
	//renderer.DrawSprite(&bird, )
}
