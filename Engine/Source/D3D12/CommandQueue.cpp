#include "Core/Globals.h"
#include "CommandQueue.h"

CommandQueue::CommandQueue(ID3D12Device4* device)
{
	//Create Command Queue
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.NodeMask = 0;
	device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));
	
	//Create Fence
	device->CreateFence(m_nextFenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
	m_fence->SetName(L"CommandQueue Fence");

	//Create Fence Event
	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
}

CommandQueue::~CommandQueue()
{
	Flush();

	m_fence.Reset();
	m_commandQueue.Reset();

	CloseHandle(m_fenceEvent);
}

void CommandQueue::CleanUp()
{

}

void CommandQueue::ExecuteCommandList(ID3D12CommandList* commandList)
{
	ID3D12CommandList* commandLists[] = { commandList };
	m_commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);
}

uint64_t CommandQueue::Signal()
{
	uint64_t fenceToSignal = m_nextFenceValue++;
	m_commandQueue->Signal(m_fence.Get(), fenceToSignal);

	return fenceToSignal;
}

void CommandQueue::WaitForFenceValue(uint64_t fenceValue)
{
	m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent);
	WaitForSingleObject(m_fenceEvent, INFINITE);
}

void CommandQueue::Flush()
{
	WaitForFenceValue(Signal());
}

uint64_t CommandQueue::GetCompletedFenceValue()
{
	return 0;
}
