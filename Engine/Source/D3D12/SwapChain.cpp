#include "Core/Globals.h"
#include "D3D12/SwapChain.h"

#include "Core/Application.h"
#include "Modules/ModuleD3D12.h"
#include "Rendering/FrameContext.h"
#include "Rendering/RenderContext.h"

SwapChain::SwapChain(HWND hWnd, ID3D12Device4* device, ID3D12CommandQueue* commandQueue, IDXGIFactory6* factory)
{
	m_hWnd = hWnd;

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
	factory->CreateSwapChainForHwnd(commandQueue, hWnd, &swapChainDesc, nullptr, nullptr, &swapChain1);
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
	
	m_renderContext = RenderContext{};
}

SwapChain::~SwapChain()
{
	
	m_swapChain.Reset();
	m_rtvDescriptorHeap.Reset();
	m_renderTargets->Reset();
}

void SwapChain::CleanUp()
{
}

uint64_t SwapChain::GetCurrentFrameIndex()
{
	m_currentFrameIndex = m_swapChain->GetCurrentBackBufferIndex();
	return m_currentFrameIndex;
}

RenderContext* SwapChain::GetCurrentRenderContext()
{
	m_renderContext.frameIndex   = GetCurrentFrameIndex();
	m_renderContext.backBuffer   = m_renderTargets[m_currentFrameIndex].Get();
	m_renderContext.rtvCpuHandle = GetRtvCpuHandle(m_currentFrameIndex);
	m_renderContext.scissorRect  = m_scissorRect;
	m_renderContext.viewport	  = m_viewport;

	return &m_renderContext;
}



void SwapChain::PresentCurrentFrame()
{
	m_swapChain->Present(0, 0);
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

void SwapChain::GetWindowSize(unsigned& width, unsigned& height)
{
	RECT rect = {};
	GetClientRect(m_hWnd, &rect);

	width = unsigned(rect.right - rect.left);
	height = unsigned(rect.bottom - rect.top);
}

void SwapChain::Resize()
{
	unsigned width, height;
	GetWindowSize(width, height);

	if (width != m_windowWidth || height != m_windowHeight) {

		m_windowWidth = width;
		m_windowHeight = height;

		m_viewport = D3D12_VIEWPORT{ 0.0, 0.0, float(m_windowWidth), float(m_windowHeight) , 0.0, 1.0 };
		m_scissorRect = D3D12_RECT{ 0, 0, long(m_windowWidth), long(m_windowHeight) };

		app->GetModuleD3D12()->GetCommandQueue()->Flush();

		for (UINT n = 0; n < FRAMES_IN_FLIGHT; n++)
		{
			m_renderTargets[n].Reset();
		}

		// Resize the swap chain
		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};

		m_swapChain->GetDesc(&swapChainDesc);

		m_swapChain->ResizeBuffers(FRAMES_IN_FLIGHT, width, height, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags);

		//Create Render Target Views
		UINT rtvDescriptorSize = app->GetModuleD3D12()->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
		{
			m_swapChain->GetBuffer(i, IID_PPV_ARGS(m_renderTargets[i].ReleaseAndGetAddressOf()));
			app->GetModuleD3D12()->GetDevice()->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHandle);

			rtvHandle.ptr += rtvDescriptorSize;
		}
	}
}

D3D12_CPU_DESCRIPTOR_HANDLE SwapChain::GetRtvCpuHandle(UINT frameIndex)
{
	UINT rtvDescriptorSize = app->GetModuleD3D12()->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE currentRtvHandle = m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	currentRtvHandle.ptr += m_currentFrameIndex * rtvDescriptorSize;

	return currentRtvHandle;
}
