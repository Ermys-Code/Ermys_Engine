#pragma once

#include "Rendering/FrameContext.h"
#include "Rendering/RenderContext.h"

#include <dxgi1_6.h>

class SwapChain
{
public:
	

public:
	SwapChain(HWND hWmd, ID3D12Device4* device, ID3D12CommandQueue* commandQueue, IDXGIFactory6* factory);
	~SwapChain();

	void CleanUp();

	uint64_t	   GetCurrentFrameIndex();
	RenderContext* GetCurrentRenderContext();

	void PresentCurrentFrame();

	void CreateBarrierForCurrentFrame(ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);

	void GetWindowSize(unsigned& width, unsigned& height);
	void Resize();

private:
	D3D12_CPU_DESCRIPTOR_HANDLE GetRtvCpuHandle(UINT frameIndex);

	ComPtr<IDXGISwapChain3>	           m_swapChain;
	ComPtr<ID3D12DescriptorHeap>	   m_rtvDescriptorHeap;
	ComPtr<ID3D12Resource>	           m_renderTargets[FRAMES_IN_FLIGHT];

	HWND m_hWnd;

	uint64_t m_currentFrameIndex = 0;
	uint64_t m_frameNumber = 0;
	
	D3D12_VIEWPORT m_viewport;
	D3D12_RECT     m_scissorRect;

	RenderContext m_renderContext;

	unsigned int m_windowWidth{ 0 };
	unsigned int m_windowHeight{ 0 };
};

