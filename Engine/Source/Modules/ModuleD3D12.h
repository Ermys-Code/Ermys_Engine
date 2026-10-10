#pragma once

#include "Core/Globals.h"
#include "Modules/IModule.h"

#include "D3D12/SwapChain.h"

#include <dxgi1_6.h>

class FrameSync;

class ModuleD3D12 : public IModule
{
public:
	ModuleD3D12(HWND hWnd);
	~ModuleD3D12();

	bool init()		  override;
	bool cleanUp()	  override;

	ID3D12Device4* GetDevice()									 { return m_device.Get(); }
	ID3D12CommandAllocator* GetCommandAllocator(UINT frameIndex) { return m_commandAllocators[frameIndex].Get(); }
	ID3D12GraphicsCommandList* GetCommandList()					 { return m_commandList.Get(); }
	ID3D12CommandQueue* GetCommandQueue()						 { return m_commandQueue.Get(); }

	FrameSync* GetFrameSync()  { return m_frameSync.get(); }
	SwapChain* GetSwapChain()  { return m_swapChain.get(); }


private:
	void InitDX();

	ComPtr<IDXGIAdapter4>              m_adapter;
	ComPtr<IDXGIFactory6>              m_factory;
	ComPtr<ID3D12Device4>              m_device;
	ComPtr<ID3D12CommandAllocator>     m_commandAllocators[FRAMES_IN_FLIGHT];
	ComPtr<ID3D12GraphicsCommandList>  m_commandList;
	ComPtr<ID3D12CommandQueue>         m_commandQueue;

	HWND m_hWnd;

	std::unique_ptr<SwapChain> m_swapChain;
	std::unique_ptr<FrameSync> m_frameSync;
};

