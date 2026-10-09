#include "Core/Globals.h"
#include "Modules/ModuleD3D12.h"

#include "D3D12/SwapChain.h"

ModuleD3D12::ModuleD3D12(HWND hWnd)
{
	m_hWnd = hWnd;
	InitDX();
}

ModuleD3D12::~ModuleD3D12()
{
}

bool ModuleD3D12::init()
{
	m_swapChain = new SwapChain(m_hWnd, m_device, m_commandQueue, m_factory);

	return true;
}

void ModuleD3D12::preRender()
{
	m_currentFrameContext = m_swapChain->StartNewFrame();

	m_commandAllocators[m_currentFrameContext.frameIndex]->Reset();
}

void ModuleD3D12::render()
{
	m_commandList->Reset(m_commandAllocators[m_currentFrameContext.frameIndex].Get(), nullptr);

	m_swapChain->CreateBarrierForCurrentFrame(m_commandList, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	float clearColor1[] = { 0.1f, 0.0f, 0.0f, 1.0f };
	float clearColor2[] = { 0.0f, 0.1f, 0.0f, 1.0f };
	float clearColor3[] = { 0.0f, 0.0f, 0.1f, 1.0f };
	float clearColor4[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	switch (m_currentFrameContext.frameIndex)
	{
		case 0:
			m_commandList->ClearRenderTargetView(m_currentFrameContext.rtvCpuHandle, clearColor1, 0, nullptr);
			break;
		case 1:
			m_commandList->ClearRenderTargetView(m_currentFrameContext.rtvCpuHandle, clearColor2, 0, nullptr);
			break;
		case 2:
			m_commandList->ClearRenderTargetView(m_currentFrameContext.rtvCpuHandle, clearColor3, 0, nullptr);
			break;
		default:
			m_commandList->ClearRenderTargetView(m_currentFrameContext.rtvCpuHandle, clearColor4, 0, nullptr);
			break;
	}

	m_swapChain->CreateBarrierForCurrentFrame(m_commandList, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	m_commandList->Close();

	ID3D12CommandList* commandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

	m_swapChain->PresentCurrentFrame();
}	

void ModuleD3D12::postRender()
{
	m_swapChain->FinishCurrentFrame(m_commandQueue);
}

bool ModuleD3D12::cleanUp()
{
	m_swapChain->CleanUp();

	m_commandList.Reset();

	m_commandQueue.Reset();

	m_adapter.Reset();

	m_factory.Reset();

	m_device.Reset();

	delete m_swapChain;

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
}
