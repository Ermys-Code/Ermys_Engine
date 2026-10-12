#pragma once

#include "Core/Globals.h"

class CommandContext
{
public:
	CommandContext(ID3D12Device4* device);
	~CommandContext();
	void CleanUp();

	void Begin();
	void Close();



	ID3D12GraphicsCommandList4* GetCommandList() { return m_commandList.Get(); }

private:
	ComPtr<ID3D12CommandAllocator>     m_allocator;
	ComPtr<ID3D12GraphicsCommandList4> m_commandList;
};