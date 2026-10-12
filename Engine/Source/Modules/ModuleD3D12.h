#pragma once

#include "Core/Globals.h"
#include "Modules/IModule.h"

#include "D3D12/SwapChain.h"
#include "D3D12/CommandQueue.h"

#include <dxgi1_6.h>

class ModuleD3D12 : public IModule
{
public:
	ModuleD3D12(HWND hWnd);
	~ModuleD3D12();

	bool init()		  override;
	bool cleanUp()	  override;

	ID3D12Device4* GetDevice() { return m_device.Get(); }
	
	SwapChain*    GetSwapChain()    { return m_swapChain.get(); }
	CommandQueue* GetCommandQueue()	{ return m_commandQueue.get(); }

private:
	bool InitDX();

	ComPtr<IDXGIAdapter4>               m_adapter;
	ComPtr<IDXGIFactory6>               m_factory;
	ComPtr<ID3D12Device4>               m_device;
	ComPtr<ID3D12CommandAllocator>      m_commandAllocators[FRAMES_IN_FLIGHT];
	ComPtr<ID3D12GraphicsCommandList4>  m_commandList;

	HWND m_hWnd;

	std::unique_ptr<SwapChain>    m_swapChain;
	std::unique_ptr<CommandQueue> m_commandQueue;
};

