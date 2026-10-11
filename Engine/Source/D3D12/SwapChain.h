#pragma once

#include "Rendering/FrameContext.h"

#include <dxgi1_6.h>

class SwapChain
{
public:
	

public:
	SwapChain(HWND hWmd, ID3D12Device4* device, ID3D12CommandQueue* commandQueue, IDXGIFactory6* factory);
	~SwapChain();

	void CleanUp();

	FrameContext GetFrameContext();
	
	FrameContext StartNewFrame();
	void PresentCurrentFrame();
	void FinishCurrentFrame(ComPtr<ID3D12CommandQueue> commandQueue);

	void CreateBarrierForCurrentFrame(ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);

private:
	D3D12_CPU_DESCRIPTOR_HANDLE GetRtvCpuHandle(UINT frameIndex);

	ComPtr<IDXGISwapChain3>	           m_swapChain;
	ComPtr<ID3D12DescriptorHeap>	   m_rtvDescriptorHeap;
	ComPtr<ID3D12Resource>	           m_renderTargets[FRAMES_IN_FLIGHT];

	HWND m_hWnd;

	uint64_t m_currentFrameIndex = 0;
	uint64_t m_frameNumber = 0;
	
	
};

