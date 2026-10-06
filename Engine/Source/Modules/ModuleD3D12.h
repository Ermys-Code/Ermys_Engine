#pragma once

#include "Core/Globals.h"
#include "Modules/Module.h"

#include <dxgi1_6.h>

class ModuleD3D12 : public Module
{
public:
	ModuleD3D12(HWND hWnd);
	~ModuleD3D12();

	bool init() override;
	void update()		 override;
	void preRender()	 override;
	void render()		 override;
	void postRender()	 override;
	bool cleanUp()		 override;

private:
	void InitDX();

	ComPtr<IDXGIAdapter4>              m_adapter;
	ComPtr<IDXGIFactory6>              m_factory;
	ComPtr<ID3D12Device4>              m_device;
	ComPtr<ID3D12CommandAllocator>     m_commandAllocator;
	ComPtr<ID3D12GraphicsCommandList>  m_commandList;
	ComPtr<ID3D12CommandQueue>         m_commandQueue;
	ComPtr<ID3D12Fence1>			   m_commandQueueFence;
	ComPtr<IDXGISwapChain3>	           m_swapChain;
	ComPtr<ID3D12DescriptorHeap>	   m_rtvDescriptorHeap;
	ComPtr<ID3D12Resource>	           m_renderTargets[FRAMES_IN_FLIGHT];

	HWND m_hWnd;

	uint64_t m_frameIndex = 0;
	uint64_t m_frameNumber = 0;
	uint64_t m_fenceValues[FRAMES_IN_FLIGHT] = {};
	uint64_t m_lastCompletedFenceValue = 0;

};

