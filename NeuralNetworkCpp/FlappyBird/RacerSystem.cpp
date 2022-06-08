#include "RacerSystem.h"

RacerSystem::RacerSystem()
{
	manualOutput = std::vector<float>(OUTPUT_COUNT);

	//eh just make a circle
	raceSpline = Spline({ 0, 50 }
	, true, true);
	raceSpline.AddCurve({ 50, 0 });
	raceSpline.AddCurve({ 0, -50 });
	raceSpline.AddCurve({ -50, 0 });

	BuildTrack();

	carTexture.Load("car.png");
}

void RacerSystem::SetDefaultDataPack(std::minstd_rand& random)
{
	defaultDataPack.position = raceArray[0];
	glm::vec2 dir =glm::normalize(raceArray[1] - raceArray[0]);
	defaultDataPack.rotation = glm::atan(dir.y, dir.x);
	defaultDataPack.rotationalVelocity = 0;
	defaultDataPack.stoppedTime = 0;
	defaultDataPack.velocity = glm::vec2(0);
}

void RacerSystem::StepOrganism(DataPack* data, const float* networkOutputs, float& fitness, bool& continueStepping)
{
	//Outputs are:
	//acceleration force, rotational acceleration force
}

void RacerSystem::SetNetworkInputs(DataPack* data, float* networkInputArray)
{
	//Inputs are:
	// raycast values for forward, left-forward, right-forward, right, left
	// current speed + rotational speed
}

void RacerSystem::StartOrganismPreview(bool manual, Renderer& renderer)
{
	editing = false;
	heldControlPointIndex = -1;
	holdingPoint = false;
	dragging = false;
}

void RacerSystem::OnStartEndGeneration(bool start)
{
	editing = false;
	canEdit = !start;
}

void RacerSystem::ResetManualOutput()
{
	for (int i = 0; i < OUTPUT_COUNT; i++)
	{
		manualOutput[i] = 0;
	}
}

void RacerSystem::OnKeyPressed(Renderer& renderer, int keycode, int action, bool manual)
{

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
	//draw race spline
	for (size_t i = 0; i < raceArray.size() - 1; i++)
	{
		if (i % 2)
			renderer.DrawLine(raceArray[i], raceArray[i + 1], glm::vec3(0.5f));
		renderer.DrawLine(leftArray[i], leftArray[i + 1]);
		renderer.DrawLine(rightArray[i], rightArray[i + 1]);
	}

	//draw race car
	RacerDataPack& info = *(RacerDataPack*)data;
	renderer.DrawSprite(&carTexture, info.position, 8, 4, glm::degrees(info.rotation));
	
	ImGui::Begin("Racer Editor");
	{
		ImGui::BeginDisabled(!canEdit);
		if (ImGui::Checkbox("Editing", &editing) && editing)
		{
			heldControlPointIndex = -1;
		}
		if (ImGui::SliderFloat("Radius", &radius, 0.1f, 10.0f, "%0.2f"))
		{
			BuildTrack();
		}
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
			auto mP = renderer.GetMouseScreenPosition();
			auto delta = mP - lastMousePos;
			renderer.SetCameraPosition(renderer.GetCamera().position + glm::vec2{delta.x, -delta.y} * 0.001f * renderer.GetCamera().size);

			lastMousePos = mP;
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

bool RacerSystem::Raycast(glm::vec2 point, glm::vec2 dir, float raycastDistance, float* hitDistance)
{
	//just a bunch o line line intersection tests with the same line between point and point + raycastDistance * dir

	//actually wait, could do the same thing as I did here https://www.desmos.com/calculator/6ta8zrfya9
	//is still a cubic, i think
	//wait no, its a quartic
	// STILL DOABLE, THERE IS A QUARTIC FORMULA
	//although it might not be..
	//but
	for (size_t i = 0; i < leftArray.size(); i++)
	{

	}
	for (size_t i = 0; i < rightArray.size(); i++)
	{

	}

	return false;
}

void RacerSystem::BuildTrack()
{
	raceArray.clear();
	leftArray.clear();
	rightArray.clear();
	for (float i = 0; i < 1;)
	{
		glm::vec2 p, m;
		raceSpline.GetPointAndGradientOnSpline(i, p, m);

		raceArray.push_back(p);

		glm::vec2 radiusAddition = radius * glm::normalize(glm::vec2{ -m.y, m.x });
		leftArray.push_back(p + radiusAddition);
		rightArray.push_back(p - radiusAddition);

		i += 0.1f / glm::length(m);
	}
	raceArray.push_back(raceArray[0]);
	leftArray.push_back(leftArray[0]);
	rightArray.push_back(rightArray[0]);

	defaultDataPack.position = raceArray[0];
	glm::vec2 dir = glm::normalize(raceArray[1] - raceArray[0]);
	defaultDataPack.rotation = glm::atan(dir.y, dir.x);
}
