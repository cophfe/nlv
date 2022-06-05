#include "SnakeSystem.h"

SnakeSystem::SnakeSystem()
{
	manualOutput = std::vector<float>(OUTPUT_NODES);
}

void SnakeSystem::SetDefaultDataPack(std::minstd_rand& random)
{
	//something here causes a memory leak??
	//its the body vector, it is not being destructed

	defaultDataPack.body.clear();
	defaultDataPack.movementDirection = Direction::DOWN;
	defaultDataPack.body.emplace_back(GRID_SIZE / 2, GRID_SIZE / 2);
	defaultDataPack.body.push_back(defaultDataPack.body[0] + Coord(0, 1));
	defaultDataPack.body.push_back(defaultDataPack.body[0] + Coord(0, 2));
	defaultDataPack.stepsLeft = STARTING_STEPS;
	defaultDataPack.amountToAdd = 0;
	defaultDataPack.random = random;

	bool appleIntersecting = false;
	//loop apple placement until it isn't on body
	do {
		appleIntersecting = false;
		//not the same every time
		defaultDataPack.appleCoord = Coord{ dist(random), dist(random) };
		for (Coord pos : defaultDataPack.body)
			appleIntersecting |= pos == defaultDataPack.appleCoord;
	} while (appleIntersecting);
}

void SnakeSystem::StepOrganism(DataPack* data, const float* networkOutputs, float& fitness, bool& continueStepping)
{
	SnakeDataPack& info = *(SnakeDataPack*)data;

	//read output actions
	char maxIndex = 0;
	if (networkOutputs[1] > networkOutputs[0])
		maxIndex = 1;
	if (networkOutputs[2] > networkOutputs[maxIndex])
		maxIndex = 2;

	char moveDir = (char)info.movementDirection;
	switch (maxIndex)
	{
	case 0: //Turn left
		moveDir--;
		break;
	case 2: //Turn right
		moveDir++;
		break;
		//case 1 is move forward
	}
	//loop direction so it doesn't go out of bounds
	moveDir %= (char)Direction::COUNT;
	if (moveDir < 0)
		moveDir += (char)Direction::COUNT;
	info.movementDirection = (Direction)moveDir;

	//now movement direction is set up correctly, step the game
	switch (StepSystem(info))
	{
	case SystemState::NOTHING:
		break;
	case SystemState::GAME_OVER:
		continueStepping = false;
		return;
	case SystemState::APPLE_GET:
		info.stepsLeft = std::min(info.stepsLeft + APPLE_GAIN, MAX_REMAINING_STEPS);
		break;
	}
	fitness = info.body.size();

	info.stepsLeft--;
	if (info.stepsLeft <= 0)
	{
		continueStepping = false;
	}
}

void SnakeSystem::SetNetworkInputs(DataPack* data, float* networkInputArray)
{
	SnakeDataPack& info = *(SnakeDataPack*)data;

	Coord head = info.body[0];
	auto left = AddDirectionToCoord(head, GetLocalDirection(info.movementDirection, Direction::LEFT));
	auto right = AddDirectionToCoord(head, GetLocalDirection(info.movementDirection, Direction::RIGHT));
	auto forward = AddDirectionToCoord(head, info.movementDirection);

	//if wall is in any of the directions
	bool isOnLeft = left.x < 0 || left.x >= GRID_SIZE || left.y < 0 || left.y >= GRID_SIZE;
	bool isOnRight = right.x < 0 || right.x >= GRID_SIZE || right.y < 0 || right.y >= GRID_SIZE;
	bool isInFront = forward.x < 0 || forward.x >= GRID_SIZE || forward.y < 0 || forward.y >= GRID_SIZE;
	//if body is in any of the directions
	for (Coord bodyPart : info.body)
	{
		isOnLeft |= left == bodyPart;
		isOnRight |= right == bodyPart;
		isInFront |= forward == bodyPart;
	}

	//if there is a bodypart or wall on left side of head
	networkInputArray[0] = (float)isOnLeft;
	//if there is a bodypart or wall on right side of head
	networkInputArray[1] = (float)isOnRight;
	//if there is a bodypart or wall in front of head
	networkInputArray[2] = (float)isInFront;

	//angle to apple from current facing direction
	Coord direction = forward - head;
	glm::vec2 appleDir = { info.appleCoord.x - head.x, info.appleCoord.y - head.y };
	float invSq = 1.0f / sqrtf(appleDir.x * appleDir.x + appleDir.y * appleDir.y);
	appleDir.x *= invSq;
	appleDir.y *= invSq;
	//input 3 is the dot product of the direction to the apple and the perpendicular direction of the head
	networkInputArray[3] = direction.y * appleDir.x - direction.x * appleDir.y;
}

void SnakeSystem::ResetManualOutput()
{
	manualOutput[0] = 0;
	manualOutput[1] = 1;
	manualOutput[2] = 0;
}

void SnakeSystem::OnKeyPressed(GLFWwindow* window, int keycode, int action)
{
	if (action == GLFW_PRESS)
	{
		if (keycode == GLFW_KEY_LEFT)
		{
			manualOutput[0] = 1;
			manualOutput[1] = 0;
		}
		else if (keycode == GLFW_KEY_RIGHT)
		{
			manualOutput[2] = 1;
			manualOutput[1] = 0;
		}
	}
}

void SnakeSystem::DrawGame(DataPack* data, Renderer& renderer)
{
	SnakeDataPack& info = *(SnakeDataPack*)data;

	float size = 95.0f;
	float cellSize = (float)size / (GRID_SIZE);
	glm::vec2 bottomLeft = glm::vec2(-(size / 2.0f) + cellSize / 2, -(size / 2.0f) + cellSize / 2);
	//draw border
	renderer.DrawBox(glm::vec2(0,0), size + cellSize * 2, size + cellSize * 2, 0, glm::vec3(0.3f, 0.3f, 0.3f));
	renderer.DrawBox(glm::vec2(0,0), size, size, 0, glm::vec3(0,0,0));

	//draw apple
	renderer.DrawBox(glm::vec2(cellSize * info.appleCoord.x, cellSize * info.appleCoord.y) + bottomLeft,
		cellSize, cellSize, 0, glm::vec3(1,0,0));

	glm::vec3 gray = glm::vec3(0.5f, 0.5f, 0.5f);
	renderer.DrawBox(glm::vec2(cellSize * GRID_SIZE, cellSize * (GRID_SIZE/2)) + bottomLeft,
		cellSize, cellSize, 0, gray);
	renderer.DrawBox(glm::vec2(cellSize * -1, cellSize * (GRID_SIZE / 2)) + bottomLeft,
		cellSize, cellSize, 0, gray);
	renderer.DrawBox(glm::vec2(cellSize * (GRID_SIZE/2), cellSize * GRID_SIZE) + bottomLeft,
		cellSize, cellSize, 0, gray);
	renderer.DrawBox(glm::vec2(cellSize * (GRID_SIZE/2), cellSize * -1 ) + bottomLeft,
		cellSize, cellSize, 0, gray);

	for (auto bodyPart : info.body)
	{
		renderer.DrawBox(glm::vec2(cellSize * bodyPart.x, cellSize * bodyPart.y) + bottomLeft,
			cellSize, cellSize, 0, glm::vec3(0.3f, 0.8f, 0.3f));
	}
	if (info.body.size() > 0)
	{
		renderer.DrawBox(glm::vec2(cellSize * info.body[0].x, cellSize * info.body[0].y) + bottomLeft,
			cellSize, cellSize, 0, glm::vec3(0.25f, 0.7f, 0.25f));

		glm::vec2 dir = { 0,0 };
		switch (info.movementDirection)
		{
		case Direction::UP:
			dir.y++;
			break;
		case Direction::DOWN:
			dir.y--;
			break;
		case Direction::LEFT:
			dir.x--;
			break;
		case Direction::RIGHT:
			dir.x++;
			break;
		}

		float mouthSize = cellSize / 6;
		glm::vec2 mouthAddition = { dir.x * (cellSize / 2 - mouthSize / 2), dir.y * (cellSize / 2 - mouthSize / 2) };
		renderer.DrawBox(glm::vec2(cellSize * info.body[0].x, cellSize * info.body[0].y) + mouthAddition + bottomLeft,
			std::max(mouthSize, cellSize * glm::abs(dir.y)), std::max(mouthSize, cellSize * glm::abs(dir.x)), 0, glm::vec3(0.0f, 0.3f, 0.0f));

		float eyeSize = cellSize / 3;
		glm::vec2 eyeAddition = { dir.y * (cellSize/2 - eyeSize/2), dir.x * (cellSize/2 - eyeSize/2) };
		renderer.DrawBox(glm::vec2(cellSize * info.body[0].x + eyeAddition.x, cellSize * info.body[0].y + eyeAddition.y) + bottomLeft,
			eyeSize, eyeSize, 0, glm::vec3(0.9f, 0.9f, 0.9f));
		renderer.DrawBox(glm::vec2(cellSize * info.body[0].x - eyeAddition.x, cellSize * info.body[0].y - eyeAddition.y) + bottomLeft,
			eyeSize, eyeSize, 0, glm::vec3(0.9f, 0.9f, 0.9f));
		
		float pupilSize= cellSize / 3.3f;
		glm::vec2 pupilAddition = { dir.y * (cellSize/2 - pupilSize /2), dir.x * (cellSize/2 - pupilSize /2) };
		renderer.DrawBox(glm::vec2(cellSize * info.body[0].x + eyeAddition.x, cellSize * info.body[0].y + eyeAddition.y) + bottomLeft,
			pupilSize, pupilSize, 0, glm::vec3(0,0,0));
		renderer.DrawBox(glm::vec2(cellSize * info.body[0].x - eyeAddition.x, cellSize * info.body[0].y - eyeAddition.y) + bottomLeft,
			pupilSize, pupilSize, 0, glm::vec3(0,0,0));
	}
}

GameSystem::DataPack* SnakeSystem::GetDefaultDataPack()
{
    return &defaultDataPack;
}

GameSystem::DataPack* SnakeSystem::NewDataPack() const
{
    return new SnakeDataPack();
}

void SnakeSystem::CopyDataPack(DataPack* dest, DataPack* src) const
{
	auto destt = (SnakeDataPack*)dest;
	*destt = *(SnakeDataPack*)src;
}

SnakeSystem::SystemState SnakeSystem::StepSystem(SnakeDataPack& system)
{
	//move snake in the movement direction
	Coord newCoord = AddDirectionToCoord(system.body[0], system.movementDirection);

	//if head intersects with body pos, game over
	for (Coord pos : system.body)
		if (pos == newCoord)
			return SystemState::GAME_OVER;

	//if head outside of bounds, game over
	if (newCoord.x >= GRID_SIZE || newCoord.x < 0 || newCoord.y >= GRID_SIZE || newCoord.y < 0)
		return SystemState::GAME_OVER;


	//if no die add head to body
	system.body.insert(system.body.begin(), newCoord);
	//if head intersects with apple place apple in new place
	if (newCoord.x == system.appleCoord.x && newCoord.y == system.appleCoord.y)
	{
		system.amountToAdd += SIZE_GAIN;

		//place apple in new place
		bool appleIntersecting = false;
		//loop apple placement until it isn't on body
		do {
			appleIntersecting = false;
			system.appleCoord = Coord(dist(system.random), dist(system.random));
			for (Coord pos : system.body)
				appleIntersecting |= pos == system.appleCoord;
		} while (appleIntersecting);
		return SystemState::APPLE_GET;
	}

	if (system.amountToAdd <= 0)
		system.body.pop_back();
	else
		system.amountToAdd--;
	return SystemState::NOTHING;
}

SnakeSystem::Direction SnakeSystem::GetLocalDirection(Direction direction, Direction relativeDirection)
{
	char dir = (char)direction;
	switch (relativeDirection)
	{
	case Direction::LEFT:
		dir++;
		break;
	case Direction::RIGHT:
		dir--;
		break;
	case Direction::DOWN:
		dir += 2;
		break;
	}
	dir %= (char)Direction::COUNT;
	if (dir < 0)
		dir += (char)Direction::COUNT;

	return (Direction)dir;
}

SnakeSystem::Coord SnakeSystem::AddDirectionToCoord(Coord coord, Direction direction)
{
	switch (direction)
	{
	case Direction::LEFT:
		coord.x--;
		break;
	case Direction::UP:
		coord.y++;
		break;
	case Direction::RIGHT:
		coord.x++;
		break;
	case Direction::DOWN:
		coord.y--;
		break;
	}
	return coord;
}