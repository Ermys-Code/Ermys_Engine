#pragma once

#include "Core/Globals.h"

struct FrameContext
{
	ID3D12Resource* backBuffer = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE rtvCpuHandle;

	float width;
	float height;

	uint64_t frameIndex = 0;

	ComPtr<ID3D12CommandAllocator> commandAllocator;
};