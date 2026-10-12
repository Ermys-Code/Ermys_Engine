#pragma once

#include "Core/Globals.h"
#include "Rendering/CommandContext.h"

struct FrameContext
{
	FrameContext(ID3D12Device4* device)
	{
		commandContext = std::make_unique<CommandContext>(device);
	}

	~FrameContext()
	{
		commandContext.reset();
	}

	std::unique_ptr<CommandContext> commandContext = nullptr;
	
	uint64_t fenceValue = 0;
};