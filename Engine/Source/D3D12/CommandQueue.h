#pragma once

#include "Core/Globals.h"

class CommandQueue
{
public:
	CommandQueue(ID3D12Device4* device);
	virtual ~CommandQueue();
	void CleanUp();

	ID3D12CommandQueue* GetCommandQueue() { return m_commandQueue.Get(); }

	void ExecuteCommandList(ID3D12CommandList* commandList);

	uint64_t Signal();
	void     WaitForFenceValue(uint64_t fenceValue);
	void     Flush();

	uint64_t ExecuteCommandListAndSignal(ID3D12CommandList* commandList);

	uint64_t GetCompletedFenceValue();


private:
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<ID3D12Fence>        m_fence;
	
	HANDLE m_fenceEvent;

	uint64_t m_nextFenceValue = 1;
};

