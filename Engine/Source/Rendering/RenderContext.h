#pragma once

#include "Core/Globals.h"

struct RenderContext
{
	ID3D12Resource* rendeRargetView  = nullptr;
	ID3D12Resource* depthStencilView = nullptr;

	D3D12_CPU_DESCRIPTOR_HANDLE rtvCpuHandle{};
	D3D12_CPU_DESCRIPTOR_HANDLE dsvCpuHandle{};

	uint64_t frameIndex;

	D3D12_VIEWPORT viewport{};
	D3D12_RECT     scissorRect{};

	unsigned width;
	unsigned height;
};