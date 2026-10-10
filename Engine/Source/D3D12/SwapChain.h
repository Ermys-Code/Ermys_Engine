#pragma once

#include <dxgi1_6.h>

class SwapChain
{
public:
	struct FrameContext
	{
		ID3D12Resource* backBuffer = nullptr;
		D3D12_CPU_DESCRIPTOR_HANDLE rtvCpuHandle;

		uint64_t frameIndex = 0;
		
		ComPtr<ID3D12CommandAllocator> commandAllocator;
	};

public:
	SwapChain(HWND hWmd, ComPtr<ID3D12Device4> device, ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<IDXGIFactory6> factory);
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

	uint64_t m_currentFrameIndex = 0;
	uint64_t m_frameNumber = 0;
	
	
};

