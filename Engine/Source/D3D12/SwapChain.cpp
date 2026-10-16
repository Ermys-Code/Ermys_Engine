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

	//Create RTV Descriptor Heap
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = FRAMES_IN_FLIGHT;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvDescriptorHeap));
	m_rtvDescriptorHeap->SetName(L"Render Target Descriptor Heap");

	//Create DSV Descriptor Heap
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = FRAMES_IN_FLIGHT;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvDescriptorHeap));
	m_dsvDescriptorHeap->SetName(L"Depth/Stencil Descriptor Heap");

	//Create Render Target Views
	UINT rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
	{
		m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i]));
		device->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHandle);
		m_renderTargets[i]->SetName(L"Render Target Texture " + i);

		rtvHandle.ptr += rtvDescriptorSize;
	}

	//Create Depth Stencil Views
	CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, m_windowWidth, m_windowHeight, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = DXGI_FORMAT_D32_FLOAT;
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.DepthStencil.Stencil = 0;
	UINT dsvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
	{
		device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue, IID_PPV_ARGS(&m_depthStencils[i]));
		device->CreateDepthStencilView(m_depthStencils[i].Get(), nullptr, dsvHandle);
		m_depthStencils[i]->SetName(L"Depth/Stencil Texture " + i);

		dsvHandle.ptr += dsvDescriptorSize;
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
	m_renderContext.frameIndex       = GetCurrentFrameIndex();
	m_renderContext.rendeRargetView  = m_renderTargets[m_currentFrameIndex].Get();
	m_renderContext.depthStencilView = m_depthStencils[m_currentFrameIndex].Get();
	m_renderContext.rtvCpuHandle	 = GetRtvCpuHandle(m_currentFrameIndex);
	m_renderContext.dsvCpuHandle     = GetDsvCpuHandle(m_currentFrameIndex);
	m_renderContext.scissorRect      = m_scissorRect;
	m_renderContext.viewport	     = m_viewport;
	m_renderContext.width		     = m_windowWidth;
	m_renderContext.height		     = m_windowHeight;

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
			m_depthStencils[n].Reset();
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

		//Create Depth Stencil Views
		CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, m_windowWidth, m_windowHeight, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
		D3D12_CLEAR_VALUE clearValue = {};
		clearValue.Format = DXGI_FORMAT_D32_FLOAT;
		clearValue.DepthStencil.Depth = 1.0f;
		clearValue.DepthStencil.Stencil = 0;
		UINT dsvDescriptorSize = app->GetModuleD3D12()->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
		{
			app->GetModuleD3D12()->GetDevice()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue, IID_PPV_ARGS(&m_depthStencils[i]));
			app->GetModuleD3D12()->GetDevice()->CreateDepthStencilView(m_depthStencils[i].Get(), nullptr, dsvHandle);
			m_depthStencils[i]->SetName(L"Depth/Stencil Texture " + i);

			dsvHandle.ptr += dsvDescriptorSize;
		}
	}
}

D3D12_CPU_DESCRIPTOR_HANDLE SwapChain::GetRtvCpuHandle(UINT frameIndex)
{
	UINT rtvDescriptorSize = app->GetModuleD3D12()->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE currentRtvHandle = m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	currentRtvHandle.ptr += frameIndex * rtvDescriptorSize;

	return currentRtvHandle;
}

D3D12_CPU_DESCRIPTOR_HANDLE SwapChain::GetDsvCpuHandle(UINT frameIndex)
{
	UINT dsvDescriptorSize = app->GetModuleD3D12()->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	D3D12_CPU_DESCRIPTOR_HANDLE currentDsvHandle = m_dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	currentDsvHandle.ptr += frameIndex * dsvDescriptorSize;

	return currentDsvHandle;
}
