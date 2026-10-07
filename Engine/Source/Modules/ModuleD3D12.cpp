#include "Core/Globals.h"
#include "Modules/ModuleD3D12.h"

ModuleD3D12::ModuleD3D12(HWND hWnd)
{
	m_hWnd = hWnd;
	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	InitDX();
}

ModuleD3D12::~ModuleD3D12()
{
}

bool ModuleD3D12::init()
{
	return true;
}

void ModuleD3D12::preRender()
{
	UINT currentFrameIndex = m_swapChain->GetCurrentBackBufferIndex();
	
	if (m_frameNumber != 0)
	{
		HRESULT hr          = m_currentFrameFence->SetEventOnCompletion(m_fenceValues[currentFrameIndex], m_fenceEvent);
		DWORD waitResult    = WaitForSingleObject(m_fenceEvent, INFINITE);
	}

	m_commandAllocators[currentFrameIndex]->Reset();
}

void ModuleD3D12::render()
{
	UINT currentFrameIndex = m_swapChain->GetCurrentBackBufferIndex();
	
	m_commandList->Reset(m_commandAllocators[currentFrameIndex].Get(), nullptr);

	D3D12_RESOURCE_BARRIER barrierToRenderTarget = {};
	barrierToRenderTarget.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrierToRenderTarget.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrierToRenderTarget.Transition.pResource = m_renderTargets[currentFrameIndex].Get();
	barrierToRenderTarget.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrierToRenderTarget.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrierToRenderTarget.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_commandList->ResourceBarrier(1, &barrierToRenderTarget);
	

	UINT rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE currentRtvHandle = m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	currentRtvHandle.ptr += currentFrameIndex * rtvDescriptorSize;
	float clearColor1[] = { 0.1f, 0.0f, 0.0f, 1.0f };
	float clearColor2[] = { 0.0f, 0.1f, 0.0f, 1.0f };
	float clearColor3[] = { 0.0f, 0.0f, 0.1f, 1.0f };
	float clearColor4[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	switch (currentFrameIndex)
	{
		case 0:
			m_commandList->ClearRenderTargetView(currentRtvHandle, clearColor1, 0, nullptr);
			break;
		case 1:
			m_commandList->ClearRenderTargetView(currentRtvHandle, clearColor2, 0, nullptr);
			break;
		case 2:
			m_commandList->ClearRenderTargetView(currentRtvHandle, clearColor3, 0, nullptr);
			break;
		default:
			m_commandList->ClearRenderTargetView(currentRtvHandle, clearColor4, 0, nullptr);
			break;
	}

	D3D12_RESOURCE_BARRIER barrierToPresent = {};
	barrierToPresent.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrierToPresent.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrierToPresent.Transition.pResource = m_renderTargets[currentFrameIndex].Get();
	barrierToPresent.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrierToPresent.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	barrierToPresent.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_commandList->ResourceBarrier(1, &barrierToPresent);

	m_commandList->Close();

	ID3D12CommandList* commandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

	m_swapChain->Present(0, 0);
}	

void ModuleD3D12::postRender()
{
	UINT currentFrameIndex = m_swapChain->GetCurrentBackBufferIndex();
	uint64_t fenceValue = m_fenceValues[currentFrameIndex];
	
	m_commandQueue->Signal(m_currentFrameFence.Get(), ++fenceValue);
	m_fenceValues[currentFrameIndex] = fenceValue;

	m_frameNumber++;
}

bool ModuleD3D12::cleanUp()
{
	CloseHandle(m_fenceEvent);

	m_commandList.Reset();

	m_commandQueue.Reset();

	m_device.Reset();

	m_factory.Reset();

	return true;
}

void ModuleD3D12::InitDX()
{
#if defined(_DEBUG)
	//Enable Debug Layer
	ComPtr<ID3D12Debug> debugInterface;
	D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface));
	debugInterface->EnableDebugLayer();
	
	//Create Debug Factory
	CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&m_factory));
#else
	//Create Factory
	CreateDXGIFactory2(0, IID_PPV_ARGS(&m_factory));
#endif 

	//Create Adapter
	m_factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&m_adapter));
	
	//Create Device
	D3D12CreateDevice(m_adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_device));

#if defined(_DEBUG)
	//Enable BreackOnSeverity
	ComPtr<ID3D12InfoQueue> infoQueue;
	m_device.As(&infoQueue);
	infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
	infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
	infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
#endif

	//Create Command Allocator
	for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
	{
		m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[i]));
	}
	
	//Create Command List
	m_device->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&m_commandList));

	//Create Command Queue
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.NodeMask = 0;
	m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));

	//Create Command Queue Fence
	m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_currentFrameFence));

	//Get Window Size
	RECT rect = {};
	GetClientRect(m_hWnd, &rect);
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
	m_factory->CreateSwapChainForHwnd(m_commandQueue.Get(), m_hWnd, &swapChainDesc, nullptr, nullptr, &swapChain1);
	swapChain1.As(&m_swapChain);

	//Create Descriptor Heap
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = FRAMES_IN_FLIGHT;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvDescriptorHeap));

	//Create Render Target Views
	UINT rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
	{
		m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i]));
		m_device->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHandle);

		rtvHandle.ptr += rtvDescriptorSize;
	}
}
