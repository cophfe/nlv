#include "BalancerSystem.h"

void BalancerSystem::SetDefaultDataPack(std::minstd_rand& random)
{
}

void BalancerSystem::StepOrganism(DataPack* data, const float* networkOutputs, float& fitness, bool& continueStepping)
{
}

void BalancerSystem::SetNetworkInputs(DataPack* data, float* networkInputArray)
{
}

void BalancerSystem::ResetManualOutput()
{
}

void BalancerSystem::OnKeyPressed(GLFWwindow* window, int keycode, int action)
{
}

void BalancerSystem::DrawGame(DataPack* data, Renderer& renderer)
{
}

DataPack* BalancerSystem::GetDefaultDataPack()
{
    return nullptr;
}

DataPack* BalancerSystem::NewDataPack() const
{
    return nullptr;
}

void BalancerSystem::CopyDataPack(DataPack* dest, DataPack* src) const
{
}

int BalancerSystem::GetInputCount() const
{
    return 0;
}

int BalancerSystem::GetOutputCount() const
{
    return 0;
}

int BalancerSystem::GetDefaultHiddenNodes() const
{
    return 0;
}
