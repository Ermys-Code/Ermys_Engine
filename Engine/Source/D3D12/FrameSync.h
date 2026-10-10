#pragma once
#include "Core/Globals.h"

class FrameSync
{
public:
	FrameSync(ComPtr<ID3D12Device4> device);
	~FrameSync();

	void CleanUp();

	void WaitForFence(UINT frameIndex);
	void SignalFence(UINT frameIndex, ComPtr<ID3D12CommandQueue> commandQueue);

private:
	ComPtr<ID3D12Fence1> m_currentFrameFence;

	HANDLE m_fenceEvent;

	uint64_t m_fenceValues[FRAMES_IN_FLIGHT] = {};
};

