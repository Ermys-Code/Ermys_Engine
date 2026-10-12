#include "Core/Globals.h"
#include "Rendering/CommandContext.h"

CommandContext::CommandContext(ID3D12Device4* device)
{
	//Create Command Allocator
	device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_allocator));
	
	//Create Command List
	device->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&m_commandList));

}

CommandContext::~CommandContext()
{
}

void CommandContext::CleanUp()
{
	m_allocator.Reset();
	m_commandList.Reset();
}

void CommandContext::Begin()
{
	m_allocator->Reset();
	m_commandList->Reset(m_allocator.Get(), nullptr);
}

void CommandContext::Close()
{
	m_commandList->Close();
}