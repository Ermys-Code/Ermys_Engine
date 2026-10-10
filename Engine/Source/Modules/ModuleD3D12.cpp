#include "Core/Globals.h"
#include "Modules/ModuleD3D12.h"

#include "D3D12/SwapChain.h"
#include "D3D12/FrameSync.h"

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
	m_swapChain = std::make_unique<SwapChain>(m_hWnd, m_device.Get(), m_commandQueue.Get(), m_factory.Get());
	m_frameSync = std::make_unique<FrameSync>(m_device.Get());

	return true;
}

bool ModuleD3D12::cleanUp()
{
	m_swapChain->CleanUp();
	m_frameSync->CleanUp();

	m_commandList.Reset();
	m_commandQueue.Reset();
	m_adapter.Reset();
	m_factory.Reset();
	m_device.Reset();

	m_swapChain.reset();
	m_frameSync.reset();

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
