#pragma once

#include "Core/Globals.h"
#include "Modules/IModule.h"

#include "D3D12/SwapChain.h"

#include <dxgi1_6.h>

class ModuleD3D12 : public IModule
{
public:
	ModuleD3D12(HWND hWnd);
	~ModuleD3D12();

	bool init()		  override;
	void preRender()  override;
	void render()	  override;
	void postRender() override;
	bool cleanUp()	  override;

	ID3D12Device4* GetDevice() { return m_device.Get(); }

private:
	void InitDX();

	ComPtr<IDXGIAdapter4>              m_adapter;
	ComPtr<IDXGIFactory6>              m_factory;
	ComPtr<ID3D12Device4>              m_device;
	ComPtr<ID3D12CommandAllocator>     m_commandAllocators[FRAMES_IN_FLIGHT];
	ComPtr<ID3D12GraphicsCommandList>  m_commandList;
	ComPtr<ID3D12CommandQueue>         m_commandQueue;

	HWND m_hWnd;

	SwapChain*				m_swapChain;
	SwapChain::FrameContext m_currentFrameContext;
};

