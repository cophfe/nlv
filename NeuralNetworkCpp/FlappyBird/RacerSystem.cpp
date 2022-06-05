#include "RacerSystem.h"

void RacerSystem::SetDefaultDataPack(std::minstd_rand& random)
{
}

void RacerSystem::StepOrganism(DataPack* data, const float* networkOutputs, float& fitness, bool& continueStepping)
{
}

void RacerSystem::SetNetworkInputs(DataPack* data, float* networkInputArray)
{
}

void RacerSystem::ResetManualOutput()
{
}

void RacerSystem::OnKeyPressed(GLFWwindow* window, int keycode, int action)
{
}

void RacerSystem::DrawGame(DataPack* data, Renderer& renderer)
{
}

GameSystem::DataPack* RacerSystem::GetDefaultDataPack()
{
    return nullptr;
}

GameSystem::DataPack* RacerSystem::NewDataPack() const
{
    return nullptr;
}

void RacerSystem::CopyDataPack(DataPack* dest, DataPack* src) const
{
}
