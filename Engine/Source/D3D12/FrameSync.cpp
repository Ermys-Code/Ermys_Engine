#include "Core/Globals.h"
#include "D3D12/FrameSync.h"

FrameSync::FrameSync(ID3D12Device4* device)
{
	//Create CurrentFrameFence Fence
	device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_currentFrameFence));

	//Create Fence Event
	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
}

FrameSync::~FrameSync()
{
	m_currentFrameFence.Reset();
}

void FrameSync::CleanUp()
{
	CloseHandle(m_fenceEvent);
}

void FrameSync::WaitForFence(UINT frameIndex)
{
	HRESULT hr = m_currentFrameFence->SetEventOnCompletion(m_fenceValues[frameIndex], m_fenceEvent);
	DWORD waitResult = WaitForSingleObject(m_fenceEvent, INFINITE);
}

void FrameSync::SignalFence(UINT frameIndex, ComPtr<ID3D12CommandQueue> commandQueue)
{
	uint64_t fenceValue = m_fenceValues[frameIndex];

	commandQueue->Signal(m_currentFrameFence.Get(), ++fenceValue);
	m_fenceValues[frameIndex] = fenceValue;
}


