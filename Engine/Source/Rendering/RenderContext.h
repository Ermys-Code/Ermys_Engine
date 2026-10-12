#pragma once

#include "Core/Globals.h"

struct RenderContext
{
	ID3D12Resource* backBuffer = nullptr;

	D3D12_CPU_DESCRIPTOR_HANDLE rtvCpuHandle{};

	uint64_t frameIndex;

	D3D12_VIEWPORT viewport{};
	D3D12_RECT     scissorRect{};
};