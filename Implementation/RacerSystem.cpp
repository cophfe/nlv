#include "RacerSystem.h"

RacerSystem::RacerSystem()
{
	manualOutput = std::vector<float>(OUTPUT_COUNT);

	raceSpline = Spline({ 0, 50 }
	, true, true);
	raceSpline.AddCurve({ 38.0574837f, 42.7155609f });
	raceSpline.AddCurve({ 13.6769075f, 5.45094156f });
	raceSpline.AddCurve({ 51.5361748f, -29.4350834f });
	raceSpline.AddCurve({ 0, -50 });
	raceSpline.AddCurve({ -93.1615448f, -0.495540112f });
	raceSpline.AddCurve({ -83.4489594f, 20.5153618f });
	raceSpline.AddCurve({ -27.3538151f, -4.85629320f });

	BuildTrack();

	carTexture.Load("car.png", TextureFormat::RGBA);
}

void RacerSystem::SetDefaultDataPack(std::minstd_rand& random)
{
	defaultDataPack.position = raceArray[0];
	glm::vec2 dir =glm::normalize(raceArray[1] - raceArray[0]);
	defaultDataPack.rotation = glm::atan(dir.y, dir.x);
	defaultDataPack.stoppedTime = 0;
	defaultDataPack.velocity = glm::vec2(0);
}

void RacerSystem::StepOrganism(DataPack* data, const float* networkOutputs, float& fitness, bool& continueStepping)
{
	RacerDataPack& info = *(RacerDataPack*)data;

	//Outputs are:
	//acceleration force, rotational acceleration force

	float accelerate = networkOutputs[0] > 0.5f ? 1.0f : 0.0f;
	float turn = glm::sign(networkOutputs[1] - 0.5f) * 2 * (glm::abs(networkOutputs[1] - 0.5f) - DEAD_ZONE/2)/(0.5f - DEAD_ZONE/2);
	
	float c = glm::cos(info.rotation);
	float s = glm::sin(info.rotation);
	glm::vec2 forward = { c, s };
	glm::vec2 right = { s, -c};

	glm::vec2 force = forward * accelerate * ACCELERATION;

	info.rotation -= TIME_STEP * turn * TURN_SPEED * glm::clamp(glm::dot(info.velocity, forward) / MAX_SPEED, 0.0f, 1.0f);
	info.position += TIME_STEP * info.velocity;

	if (accelerate == 0)
	{
		//this parameter might make it hard for the ai to tell what is going on so i will remove it
		//info.currentDrag = glm::max(info.currentDrag + TIME_STEP * DRAG, DRAG_MAX);
		//apply drag
		info.velocity -= info.velocity * DRAG * TIME_STEP;
	}

	//apply drift
	info.velocity -= right * glm::dot(right, info.velocity) * DRIFT_CANCEL * TIME_STEP;
	//apply force
	info.velocity += force * TIME_STEP;

	//if you stop moving for too long, you lose
	if (glm::length2(info.velocity) < 1.0f)
	{
		info.stoppedTime += TIME_STEP;
		if (info.stoppedTime > MAX_STOP_TIME)
			continueStepping = false;
	}
	else
		info.stoppedTime = 0;

	//if you collide with a wall, you lose
	//(just construct a box out of raycasts cuz lazy)
	auto rightOffset = right * (CAR_DIMENSIONS.y / 2.0f);
	auto forwardOffset = forward * (CAR_DIMENSIONS.x / 2.0f);
	if (RaycastWalls(info.position - rightOffset - forwardOffset, forward, CAR_DIMENSIONS.x, nullptr) //bottom left to top left
		|| RaycastWalls(info.position + rightOffset - forwardOffset, forward, CAR_DIMENSIONS.x, nullptr) //bottom right to top right
		|| RaycastWalls(info.position - rightOffset - forwardOffset, right, CAR_DIMENSIONS.y, nullptr) //bottom left to bottom right
		|| RaycastWalls(info.position - rightOffset + forwardOffset, right, CAR_DIMENSIONS.y, nullptr)) //top left to top right
	{
		continueStepping = false;
	}

	//fitness is distance along curve
	const int sampleCount = 150;
	int index = 0;
	float sqDist = glm::length2(info.position - raceArray[0]);
	
	for (size_t i = 1; i < raceArray.size(); i++)
	{
		float sqDist2 = glm::length2(info.position - raceArray[i]);
		if (sqDist2 < sqDist)
		{
			sqDist = sqDist2;
			index = i;
		}
	}
	float t = (float)index / raceArray.size();

	//happens on loop
	float progress = glm::mod(fitness, 1.0f);
	if (t < 0.2f && progress > 0.8f)
		fitness = glm::ceil(fitness) + t;
	//remove loop exploit for ez fitness
	else if (t > 0.8f && progress < 0.2f)
		fitness = glm::floor(fitness) - 1 + t;
	else
		fitness = glm::floor(fitness) + t;
}

void RacerSystem::SetNetworkInputs(DataPack* data, float* networkInputArray)
{
	RacerDataPack& info = *(RacerDataPack*)data;

	//Inputs are:
	// raycast values for forward, left-forward, right-forward, left, right
	// current speed forward + speed sideward

	float angle = glm::radians(info.rotation);
	float c = glm::cos(info.rotation);
	float s = glm::sin(info.rotation);
	float c2 = glm::cos(info.rotation - glm::pi<float>()/4.0f);
	float s2 = glm::sin(info.rotation - glm::pi<float>()/4.0f);

	glm::vec2 forward = { c, s };
	glm::vec2 right = { s, -c };
	glm::vec2 left = -right;
	glm::vec2 rightForward = { c2, s2 };
	glm::vec2 leftForward = { -s2, c2 };

	glm::vec2 pos = info.position + forward * CAR_DIMENSIONS.x / 3.0f;
	RaycastWalls(pos, forward, RAYCAST_DISTANCE, networkInputArray);
	RaycastWalls(pos, leftForward, RAYCAST_DISTANCE, networkInputArray + 1);
	RaycastWalls(pos, rightForward, RAYCAST_DISTANCE, networkInputArray + 2);
	RaycastWalls(pos, left, RAYCAST_DISTANCE, networkInputArray + 3);
	RaycastWalls(pos, right, RAYCAST_DISTANCE, networkInputArray + 4);
	networkInputArray[5] = glm::dot(forward, info.velocity);
	networkInputArray[6] = glm::dot(right, info.velocity);
}

void RacerSystem::StartOrganismPreview(bool manual, Renderer& renderer)
{
	editing = false;
	heldControlPointIndex = -1;
	holdingPoint = false;
	dragging = false;
	if (manual)
	{
		manualOutput[0] = 0;
		manualOutput[1] = 0.5f;
	}
}

void RacerSystem::OnStartEndGeneration(bool start)
{
	editing = false;
	canEdit = !start;
}

void RacerSystem::ResetManualOutput()
{
	
}

void RacerSystem::OnKeyPressed(Renderer& renderer, int keycode, int action, bool manual)
{
	if (manual)
	{
		switch (keycode)
		{
		case GLFW_KEY_UP:
			if (action == GLFW_RELEASE)
				manualOutput[0] = 0;
			else
				manualOutput[0] = 1.0f;
			break;
		case GLFW_KEY_LEFT:
			if (action == GLFW_RELEASE)
				manualOutput[1] = 0.5f;
			else
				manualOutput[1] = 0;
			break;
		case GLFW_KEY_RIGHT:
			if (action == GLFW_RELEASE)
				manualOutput[1] = 0.5f;
			else
				manualOutput[1] = 1;
			break;
		}
	}
}

void RacerSystem::OnMousePressed(Renderer& renderer, int button, int action, bool manual)
{
	if (!editing)
	{
		lastMousePos = renderer.GetMouseScreenPosition();
		dragging = button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS;
		return;
	}
	else
		dragging = false;

	if (button == GLFW_MOUSE_BUTTON_1)
	{
		holdingPoint = action == GLFW_PRESS;
		if (holdingPoint)
		{
			auto mP = renderer.GetMousePosition();
			heldControlPointIndex = raceSpline.GetClosestControlPointIndex(mP);
			if (glm::length2(raceSpline.GetControlPoints()[heldControlPointIndex] - mP) > 3.5f * 3.5f)
			{
				glm::vec2 point;
				float dist = raceSpline.GetMinimumDistanceToPoint(mP, &point);
				if (dist < 3.5f)
				{
					heldControlPointIndex = raceSpline.InsertCurveFromPosition(mP);
				}
				else
					heldControlPointIndex = -1;
			}
		}
	}
	if (button == GLFW_MOUSE_BUTTON_2 && action == GLFW_PRESS && raceSpline.GetCurveCount() > 3)
	{
		auto mP = renderer.GetMousePosition();

		heldControlPointIndex = raceSpline.GetClosestControlPointIndex(mP);
		if (glm::length2(raceSpline.GetControlPoints()[heldControlPointIndex] - mP) < 3.5f * 3.5f)
		{
			raceSpline.RemoveCurve(heldControlPointIndex);
			BuildTrack();
			heldControlPointIndex = -1;
		}
	}
}

void RacerSystem::OnMouseScrolled(Renderer& renderer, float amount, bool manual)
{
	if (editing)
		return;

	renderer.SetCameraSize(glm::clamp(renderer.GetCamera().size - amount * 5.0f, 1.0f, 150.0f));
}

void RacerSystem::DrawGame(DataPack* data, Renderer& renderer)
{
	renderer.SetLineWidth(2);

	//draw race spline
	for (size_t i = 0; i < raceArray.size() - 1; i++)
		if (i % 2)
			renderer.DrawLine(raceArray[i], raceArray[i + 1], glm::vec3(0.5f));
	for (size_t i = 0; i < leftArray.size() - 1; i++)
		renderer.DrawLine(leftArray[i], leftArray[i + 1]);
	for (size_t i = 0; i < rightArray.size() - 1; i++)
		renderer.DrawLine(rightArray[i], rightArray[i + 1]);

	RacerDataPack& info = *(RacerDataPack*)data;
	
	if (showDebug)
	{
		//draw closest point on track
		glm::vec2 point;
		raceSpline.GetMinimumDistanceToPoint(info.position, &point);
		renderer.DrawBox(point, 1, 1, 0, { 1, 0, 1 });
		renderer.DrawLine(point, info.position, { 1, 0, 1 });

		//draw rays
		float angle = glm::radians(info.rotation);
		float c = glm::cos(info.rotation);
		float s = glm::sin(info.rotation);
		float c2 = glm::cos(info.rotation - glm::pi<float>() / 4.0f);
		float s2 = glm::sin(info.rotation - glm::pi<float>() / 4.0f);

		glm::vec2 forward = { c, s };
		glm::vec2 right = { s, -c };
		glm::vec2 left = -right;
		glm::vec2 rightForward = { c2, s2 };
		glm::vec2 leftForward = { -s2, c2 };

		float dist[5];
		glm::vec2 pos = info.position + forward * CAR_DIMENSIONS.x / 3.0f;
		RaycastWalls(pos, forward, RAYCAST_DISTANCE, dist);
		RaycastWalls(pos, leftForward, RAYCAST_DISTANCE, dist + 1);
		RaycastWalls(pos, rightForward, RAYCAST_DISTANCE, dist + 2);
		RaycastWalls(pos, left, RAYCAST_DISTANCE, dist + 3);
		RaycastWalls(pos, right, RAYCAST_DISTANCE, dist + 4);

		renderer.DrawLine(pos, pos + RAYCAST_DISTANCE * forward, { 1,0,0 });
		renderer.DrawLine(pos, pos + RAYCAST_DISTANCE * leftForward, { 1,0,0 });
		renderer.DrawLine(pos, pos + RAYCAST_DISTANCE * rightForward, { 1,0,0 });
		renderer.DrawLine(pos, pos + RAYCAST_DISTANCE * left, { 1,0,0 });
		renderer.DrawLine(pos, pos + RAYCAST_DISTANCE * right, { 1,0,0 });

		renderer.DrawLine(pos, pos + dist[0] * forward, {0,1,0});
		renderer.DrawLine(pos, pos + dist[1] * leftForward, {0,1,0});
		renderer.DrawLine(pos, pos + dist[2] * rightForward, {0,1,0});
		renderer.DrawLine(pos, pos + dist[3] * left, {0,1,0});
		renderer.DrawLine(pos, pos + dist[4] * right, {0,1,0});
	}
	
	//draw race car
	renderer.DrawSprite(&carTexture, info.position, CAR_DIMENSIONS.x * 1.2f, CAR_DIMENSIONS.y * 1.2f, glm::degrees(info.rotation));

	ImGui::Begin("Racer Editor");
	{
		ImGui::BeginDisabled(!canEdit);
		if (ImGui::Checkbox("Editing", &editing) && editing)
		{
			heldControlPointIndex = -1;
		}
		if (ImGui::SliderFloat("Radius", &radius, 5.0f, 15.0f, "%0.2f"))
		{
			BuildTrack();
		}
		ImGui::Checkbox("Show Debug", &showDebug);
		ImGui::Checkbox("Follow Car", &followCar);
		
		ImGui::EndDisabled();
	}
	ImGui::End();

	if (editing)
	{
		auto& controlPoints = raceSpline.GetControlPoints();
		for (int i = 0; i < controlPoints.size(); i++)
		{
			if (raceSpline.IsIntermediate(i))
			{
				renderer.DrawLine(controlPoints[i], controlPoints[Spline::GetAttachedControlPointIndex(i)], glm::vec3(0, 0, 1));
				renderer.DrawBox(controlPoints[i], 2, 2, 0, glm::vec3(0, 0, 1));
			}
			else
			{
				renderer.DrawBox(controlPoints[i], 2, 2, 0, glm::vec3(1, 1, 1));
			}
		}

		auto mP = renderer.GetMousePosition();
		int closest = raceSpline.GetClosestControlPointIndex(mP);

		if (holdingPoint && heldControlPointIndex != -1)
		{
			renderer.DrawBox(controlPoints[heldControlPointIndex], 3, 3, 0, glm::vec3(0, 1, 0));
			raceSpline.SetControlPoint(heldControlPointIndex, mP);
			BuildTrack();
		}
		else if (glm::length2(controlPoints[closest] - mP) < 3.5f * 3.5f)
			renderer.DrawBox(controlPoints[closest], 3, 3, 0, glm::vec3(0, 1, 1));
		else
		{
			glm::vec2 point;
			float dist = raceSpline.GetMinimumDistanceToPoint(mP, &point);
			if (dist < 3.5f)
				renderer.DrawBox(point, 1, 1, 0, glm::vec3(0, 1, 1));
		}
	}
	else
	{
		if (dragging)
		{
			followCar = false;

			auto mP = renderer.GetMouseScreenPosition();
			auto delta = mP - lastMousePos;
			renderer.SetCameraPosition(renderer.GetCamera().position + glm::vec2{delta.x, -delta.y} * 0.001f * renderer.GetCamera().size);
			renderer.SetCameraRotation(0);

			lastMousePos = mP;
		}
		else if (followCar)
		{
			renderer.SetCameraPosition({ -info.position.x, -info.position.y });
			renderer.SetCameraRotation(90 + glm::degrees(-info.rotation));
		}
	}

}

GameSystem::DataPack* RacerSystem::GetDefaultDataPack()
{
	return &defaultDataPack;
}

GameSystem::DataPack* RacerSystem::NewDataPack() const
{
    return new RacerDataPack();
}

void RacerSystem::CopyDataPack(DataPack* dest, DataPack* src) const
{
	auto destt = (RacerDataPack*)dest;
	*destt = *(RacerDataPack*)src;
}

bool RacerSystem::RaycastLine(std::vector<glm::vec2>& line, glm::vec2 origin, glm::vec2 dir, float raycastDistance, float* hitDistance)
{
	//just line-line (cool version against the function that defines the walls was interesting but too slow)

	auto a = line[0];
	float dist = raycastDistance;
	bool success = false;
	glm::vec2 perp = { -dir.y, dir.x };

	int size = line.size();
	for (size_t i = 1; i < size; i++)
	{
		auto b = line[i];

		auto v1 = origin - a;
		auto v2 = b - a;

		float d = glm::dot(v2, perp);
		if (glm::abs(d) > 0.00001f)
		{
			float t1 = (v2.x * v1.y - v2.y * v1.x) / d;
			float t2 = glm::dot(v1, perp) / d;

			if (dist >= t1 && t1 > 0.0f && (t2 >= 0.0f && t2 <= 1.0f))
			{
				dist = t1;
				success = true;
			}
		}
		
		a = b;
	}

	if (hitDistance)
		*hitDistance = dist;

	return success;
}

bool RacerSystem::RaycastWalls(glm::vec2 origin, glm::vec2 dir, float raycastDistance, float* hitDistance)
{
	if (hitDistance)
	{
		if (RaycastLine(leftArray, origin, dir, raycastDistance, hitDistance))
		{
			float dist;

			if (RaycastLine(rightArray, origin, dir, raycastDistance, &dist)
				&& dist < *hitDistance)
			{
				*hitDistance = dist;
			}
			return true;
		}
		else
			return RaycastLine(rightArray, origin, dir, raycastDistance, hitDistance);
		
	}
	
	//else
	return RaycastLine(leftArray, origin, dir, raycastDistance, nullptr)
		|| RaycastLine(rightArray, origin, dir, raycastDistance, nullptr);
}

void RacerSystem::BuildTrack()
{
	raceArray.clear();
	leftArray.clear();
	rightArray.clear();
	const float speed = 2.0f;

	//just so I don't have to make an extra function
	std::vector<glm::vec2>* lines[2] = { &leftArray, &rightArray };

	//build a track with a constant distance between points
	//interestingly, using the spline's gradient to do this doesn't actually work
	// (since the walls are not following the bezier curve function exactly, they are extruded from it by a radius)
	//luckily I have a solution! I already calculated the wall's gradient for the mathmatical raycast (which I'm not using cuz it's slow)
	//so I can just use that 

	int curveCount = raceSpline.GetCurveCount();
	const auto& ctrlPoints = raceSpline.GetControlPoints();

	for (int i = 0; i < 2; i++)
	{
		auto& line = *lines[i];
		int sign = 2 * i - 1;

		for (float globalT = 0; globalT < 1;)
		{
			int index = (int)(globalT * curveCount);
			float t = globalT * curveCount - index;
			index *= 3;
			auto p1 = ctrlPoints[index];
			auto p2 = ctrlPoints[index + 1];
			auto p3 = ctrlPoints[index + 2];
			auto p4 = ctrlPoints[index + 3];

			glm::vec2 a = 3.0f * (p4 - p1 - 3.0f * p3 + 3.0f * p2);
			glm::vec2 b = 2.0f * (3.0f * p3 - 6.0f * p2 + 3.0f * p1);
			glm::vec2 c = 3.0f * p2 - 3.0f * p1;

			glm::vec2 m;

			//get gradient (x and y are only slightly different)
			float term1 = (a.x * b.y - a.y * b.x) * t * t;
			float xTerm = (t * (a.x * t + b.x) + c.x) * term1;
			float yTerm = (t * (a.y * t + b.y) + c.y) * term1;
			double term2 = 2.0f * (a.x * c.y - a.y * c.x) * t + b.x * c.y - b.y * c.x;

			glm::vec<2, double> sq = (t * (a * t + b) + c);
			double len = sq.x * sq.x + sq.y * sq.y;
			double bottomTerm = glm::inversesqrt(len) / len;

			glm::vec2 curveGradient = raceSpline.EvaluateBezierCurveGradient(p1, p2, p3, p4, t);
			glm::vec2 curvePoint = raceSpline.EvaluateBezierCurve(p1, p2, p3, p4, t);

			m.x = curveGradient.x - sign * radius * ((xTerm + term2) * bottomTerm);
			m.y = curveGradient.y + sign * radius * ((yTerm + term2) * bottomTerm);

			line.push_back(curvePoint + sign * radius * glm::normalize(glm::vec2{ -curveGradient.y, curveGradient.x }));
			globalT += speed / (glm::clamp(glm::length(m), 50.0f, 1000000.0f)*curveCount);
		}
	}
	
	// also do regular bezier gradient stuff for race spline
	for (float i = 0; i < 1;)
	{
		glm::vec2 p, m;
		raceSpline.GetPointAndGradientOnSpline(i, p, m);

		raceArray.push_back(p);
		i += speed / (glm::clamp(glm::length(m), 10.0f, 1000000.0f) * curveCount);
	}

	//loop
	raceArray.push_back(raceArray[0]);
	leftArray.push_back(leftArray[0]);
	rightArray.push_back(rightArray[0]);

	defaultDataPack.position = raceArray[0];
	glm::vec2 dir = glm::normalize(raceArray[1] - raceArray[0]);
	defaultDataPack.rotation = glm::atan(dir.y, dir.x);
}

#pragma region Unused
/*
glm::vec2 RacerSystem::GetRadiusCurveValue(glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec2 d, float t, int sign)
{
	glm::vec2 curveGradient = raceSpline.EvaluateBezierCurveGradient(a, b, c, d, t);
	glm::vec2 curveValue = raceSpline.EvaluateBezierCurve(a, b, c, d, t);;
	//return curveValue - sign * radius * curveGradient.y * glm::inversesqrt(curveGradient.x * curveGradient.x + curveGradient.y * curveGradient.y);
	curveGradient = sign * radius * glm::normalize(curveGradient);
	return curveValue + glm::vec2{ -curveGradient.y, curveGradient.x };
}

bool RacerSystem::RaycastAgainstCurve(glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec2 d, float& t, float& dist, int sign, int sampleCount)
{
	//do basic intersection test as a rough collision test

	bool collides = false;
	glm::vec2 lastSamplePoint = GetRadiusCurveValue(a, b, c, d, 0, sign);

	for (size_t i = 1; i <= sampleCount; i++)
	{
		auto samplePoint = GetRadiusCurveValue(a, b, c, d, (float)i / sampleCount, sign);

		if (glm::sign(lastSamplePoint.x) != glm::sign(samplePoint.x) && samplePoint.y > 0 && samplePoint.y < dist)
		{
			dist = samplePoint.y;

			float zeroPosition = -samplePoint.x / (lastSamplePoint.x - samplePoint.x);

			//interpolate to make value more accurate
			//this sadly makes it accurate enough that there is no point in doing all the math I did for increasing accuracy
			t = (i - zeroPosition) / sampleCount;
			collides = true;
		}
		lastSamplePoint = samplePoint;
	}

	return collides;
}

glm::vec2 RacerSystem::TransformPoint(glm::vec2 point, glm::vec2 translation, float sin, float cos)
{
	point -= translation;
	return { point.x * cos - point.y * sin, point.x * sin + point.y * cos };
}

bool RacerSystem::RaycastAgainstWall(glm::vec2 point, float cos, float sin, glm::vec2& hitPoint, float raycastDistance, int wallSign)
{
	//this is SIGNIFICANTLY slower than just doing a lot of line-line intersection tests, and only changes the accuracy from 0.001 to 0.0000001
	//and you could definitely increase the accuracy even more by just doing more line-line intersection tests

	auto& controlPoints = raceSpline.GetControlPoints();
	int curveCount = raceSpline.GetCurveCount();
	int sampleCountPerCurve = glm::max(10, 150 / curveCount);

	int curveIndex = -1;
	float t = 0;
	float dist = 999999999.0f;
	glm::vec2 p1;
	glm::vec2 p2;
	glm::vec2 p3;
	glm::vec2 p4;

	//get rough intersection test
	glm::vec2 curveA = TransformPoint(controlPoints[0], point, sin, cos);
	for (int i = 0; i < curveCount; i++)
	{
		int cIndex = 3 * i;
		glm::vec2 curveB = TransformPoint(controlPoints[cIndex + 1], point, sin, cos);
		glm::vec2 curveC = TransformPoint(controlPoints[cIndex + 2], point, sin, cos);
		glm::vec2 curveD = TransformPoint(controlPoints[cIndex + 3], point, sin, cos);

		float curveT;
		float curveDist = dist;
		if (RaycastAgainstCurve(curveA, curveB, curveC, curveD, curveT, curveDist, wallSign, sampleCountPerCurve) && curveDist < dist)
		{
			dist = curveDist;
			t = curveT;
			curveIndex = cIndex;

			p1 = curveA;
			p2 = curveB;
			p3 = curveC;
			p4 = curveD;
		}

		curveA = curveD;
	}

	if (curveIndex == -1)
		return false;

	//now use qwik maths to get more accurate point
	{
		glm::vec2 a = 3.0f * (p4 - p1 - 3.0f * p3 + 3.0f * p2);
		glm::vec2 b = 2.0f * (3.0f * p3 - 6.0f * p2 + 3.0f * p1);
		glm::vec2 c = 3.0f * p2 - 3.0f * p1;
		const int iterations = 2;
		double gradient = 1;
		//now that curve has been selected get its derivitive
		//using newtons method approximate the closest root, getting a very slightly more accurate t value then with just samples
		//yeah this was more for fun then for practicality okay? story of this entire library

		for (size_t i = 0; i < iterations && glm::abs(gradient) > 0.01f; i++)
		{
			double top = (t * (a.x * t + b.x) + c.x) * ((a.x * b.y - a.y * b.x) * t * t + 2.0f * (a.x * c.y - a.y * c.x) * t + b.x * c.y - b.y * c.x);
			glm::vec<2, double> sq = (t * (a * t + b) + c);
			double bottom = sq.x * sq.x + sq.y * sq.y;
			//float bottomTerm = glm::pow(bottom, 3.0 / 2.0);
			double deriveTerm = top * glm::inversesqrt(bottom) / bottom;
			glm::vec2 curveGradient = raceSpline.EvaluateBezierCurveGradient(p1, p2, p3, p4, t);
			gradient = curveGradient.x + wallSign * radius * deriveTerm;

			float curveValue = raceSpline.EvaluateBezierCurve(p1.x, p2.x, p3.x, p4.x, t);
			curveValue = curveValue - wallSign * radius * curveGradient.y * glm::inversesqrt(curveGradient.x * curveGradient.x + curveGradient.y * curveGradient.y);
			t = (gradient * t - curveValue) / gradient;
			//t -= gradient * 0.00001f;

		}
	}

	//now get point with t value
	auto finalHitPoint = GetRadiusCurveValue(controlPoints[curveIndex], controlPoints[curveIndex + 1],
		controlPoints[curveIndex + 2], controlPoints[curveIndex + 3], t, wallSign);

	if (glm::length2(finalHitPoint - point) > raycastDistance * raycastDistance)
		return false;
	else
	{
		hitPoint = finalHitPoint;
		return true;
	}
}

bool RacerSystem::CoolRaycastAgainstWalls(glm::vec2 point, glm::vec2 dir, float raycastDistance, float* hitDistance)
{
	//this is hella unoptimised

	//cannot just solve for t when x = 0 on a transformed bezier curve 
	// because the actual function the walls follow is curve + a vector perpendicular to the curve's gradient of length = radius
	//it turns out that function is quintic and therefore there is no guarantee they can be solved for exactly, like you could solve the bezier curve
	//but thats okay because we can estimate it using newton's method
	//but wait, newtons method requires we have a t value that is kind of close to the right t value, 
	// otherwise it will either find a different local minima or diverge and be useless
	// soo we need a rough idea of the right value anyway
	// solutions are:
	//		1) its line-line intersection time
	//		2) do gradient descent (doesn't diverge as easily) on on a bunch of evenly distributed points, then do newtons method on those points, then compare and find the best one
	//ok fine i will just do god damn line line intersections

	//first transform bezier curve points
	float angle = -glm::atan(dir.y, dir.x) + glm::half_pi<float>();
	float co = glm::cos(angle);
	float si = glm::sin(angle);

	glm::vec2 hitPoint;
	if (RaycastAgainstWall(point, co, si, hitPoint, raycastDistance, 1))
	{
		float dist = glm::length2(hitPoint - point);
		if (RaycastAgainstWall(point, co, si, hitPoint, raycastDistance, -1) && hitDistance)
		{
			float dist2 = glm::length2(hitPoint - point);
			*hitDistance = glm::sqrt(dist > dist2 ? dist2 : dist);
		}
		else if (hitDistance)
		{
			*hitDistance = glm::sqrt(dist);
		}
		return true;
	}
	else if (RaycastAgainstWall(point, co, si, hitPoint, raycastDistance, -1))
	{
		if (hitDistance)
			*hitDistance = glm::length(hitPoint - point);
		return true;
	}
	else if (hitDistance)
		*hitDistance = raycastDistance;

	return false;
}
*/
#pragma endregion