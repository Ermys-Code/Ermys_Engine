#include "Core/Globals.h"
#include "D3D12/SwapChain.h"

#include "Core/Application.h"
#include "Modules/ModuleD3D12.h"

SwapChain::SwapChain(HWND hWnd, ComPtr<ID3D12Device4> device, ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<IDXGIFactory6> factory)
{
	//Create CurrentFrameFence Fence
	device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_currentFrameFence));

	//Get Window Size
	RECT rect = {};
	GetClientRect(hWnd, &rect);
	unsigned width = unsigned(rect.right - rect.left);
	unsigned height = unsigned(rect.bottom - rect.top);

	//Create Swap Chain
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = width;
	swapChainDesc.Height = height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = FALSE;
	swapChainDesc.SampleDesc = { 1, 0 };
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = FRAMES_IN_FLIGHT;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapChainDesc.Flags = 0;
	ComPtr<IDXGISwapChain1> swapChain1;
	factory->CreateSwapChainForHwnd(commandQueue.Get(), hWnd, &swapChainDesc, nullptr, nullptr, &swapChain1);
	swapChain1.As(&m_swapChain);

	//Create Descriptor Heap
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = FRAMES_IN_FLIGHT;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvDescriptorHeap));
	
	//Create Render Target Views
	UINT rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
	{
		m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i]));
		device->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHandle);

		rtvHandle.ptr += rtvDescriptorSize;
	}

	//Create Fence Event
	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
}

SwapChain::~SwapChain()
{
	m_currentFrameFence.Reset();
	m_swapChain.Reset();
	m_rtvDescriptorHeap.Reset();
	m_renderTargets->Reset();
}

void SwapChain::CleanUp()
{
	CloseHandle(m_fenceEvent);
}

SwapChain::FrameContext SwapChain::GetFrameContext()
{
	m_currentFrameIndex = m_swapChain->GetCurrentBackBufferIndex();

	FrameContext frameContext = {};
	frameContext.rtv		  = m_renderTargets[m_currentFrameIndex].Get();
	frameContext.rtvCpuHandle = GetRtvCpuHandle(m_currentFrameIndex);
	frameContext.frameIndex   = m_currentFrameIndex;

	return frameContext;
}

SwapChain::FrameContext SwapChain::StartNewFrame()
{
	if (m_frameNumber != 0)
	{
		HRESULT hr = m_currentFrameFence->SetEventOnCompletion(m_fenceValues[m_currentFrameIndex], m_fenceEvent);
		DWORD waitResult = WaitForSingleObject(m_fenceEvent, INFINITE);
	}

	return GetFrameContext();
}

void SwapChain::PresentCurrentFrame()
{
	m_swapChain->Present(0, 0);
}

void SwapChain::FinishCurrentFrame(ComPtr<ID3D12CommandQueue> commandQueue)
{
	uint64_t fenceValue = m_fenceValues[m_currentFrameIndex];

	commandQueue->Signal(m_currentFrameFence.Get(), ++fenceValue);
	m_fenceValues[m_currentFrameIndex] = fenceValue;

	m_frameNumber++;
}

void SwapChain::CreateBarrierForCurrentFrame(ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES		after)
{
	D3D12_RESOURCE_BARRIER barrierToRenderTarget = {};
	barrierToRenderTarget.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrierToRenderTarget.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrierToRenderTarget.Transition.pResource = m_renderTargets[m_currentFrameIndex].Get();
	barrierToRenderTarget.Transition.StateBefore = before;
	barrierToRenderTarget.Transition.StateAfter = after;
	barrierToRenderTarget.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	commandList->ResourceBarrier(1, &barrierToRenderTarget);
}

D3D12_CPU_DESCRIPTOR_HANDLE SwapChain::GetRtvCpuHandle(UINT frameIndex)
{
	UINT rtvDescriptorSize = app->GetModuleD3D12()->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE currentRtvHandle = m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	currentRtvHandle.ptr += m_currentFrameIndex * rtvDescriptorSize;

	return currentRtvHandle;
}
