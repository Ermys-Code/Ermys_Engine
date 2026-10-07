#pragma once

#include "Core/Globals.h"
#include "Modules/IModule.h"

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

private:
	void InitDX();

	ComPtr<IDXGIAdapter4>              m_adapter;
	ComPtr<IDXGIFactory6>              m_factory;
	ComPtr<ID3D12Device4>              m_device;
	ComPtr<ID3D12CommandAllocator>     m_commandAllocators[FRAMES_IN_FLIGHT];
	ComPtr<ID3D12GraphicsCommandList>  m_commandList;
	ComPtr<ID3D12CommandQueue>         m_commandQueue;
	ComPtr<ID3D12Fence1>			   m_currentFrameFence;
	ComPtr<IDXGISwapChain3>	           m_swapChain;
	ComPtr<ID3D12DescriptorHeap>	   m_rtvDescriptorHeap;
	ComPtr<ID3D12Resource>	           m_renderTargets[FRAMES_IN_FLIGHT];

	HWND m_hWnd;
	HANDLE m_fenceEvent;

	uint64_t m_currentFrameIndex = 0;
	uint64_t m_frameNumber = 0;
	uint64_t m_fenceValues[FRAMES_IN_FLIGHT] = {};
	uint64_t m_lastCompletedFenceValue = 0;
};

