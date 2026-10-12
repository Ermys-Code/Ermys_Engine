#include "Core/Globals.h"
#include "Modules/ModuleD3D12.h"

#include "D3D12/SwapChain.h"
#include "D3D12/CommandQueue.h"

ModuleD3D12::ModuleD3D12(HWND hWnd) : m_hWnd(hWnd)
{
}

ModuleD3D12::~ModuleD3D12()
{
}

bool ModuleD3D12::init()
{
	if (!InitDX()) return false;
	if (!m_device || !m_factory || !m_adapter) return false;

	m_commandQueue = std::make_unique<CommandQueue>(m_device.Get());
	if (!m_commandQueue) return false;

	m_swapChain = std::make_unique<SwapChain>(m_hWnd, m_device.Get(), m_commandQueue->GetCommandQueue(), m_factory.Get());
	if (!m_swapChain) return false;

	return true;
}

bool ModuleD3D12::cleanUp()
{
	if(m_swapChain) m_swapChain->CleanUp();
	if(m_commandQueue) m_commandQueue->CleanUp();

	m_swapChain.reset();
	m_commandQueue.reset();

	m_commandList.Reset();
	m_adapter.Reset();
	m_factory.Reset();
	m_device.Reset();


	return true;
}

bool ModuleD3D12::InitDX()
{
	bool ok = true;

#if defined(_DEBUG)
	//Enable Debug Layer
	ComPtr<ID3D12Debug> debugInterface;
	ok = ok && SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
	if (ok)
	{
		debugInterface->EnableDebugLayer();
	}

	//Create Debug Factory
	ok = ok && SUCCEEDED(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&m_factory)));
#else
	//Create Factory
	ok = ok && SUCCEEDED(CreateDXGIFactory2(0, IID_PPV_ARGS(&m_factory)));
#endif 
	
	//Create Adapter
	ok = ok && SUCCEEDED(m_factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&m_adapter)));

	//Create Device
	ok = ok && SUCCEEDED(D3D12CreateDevice(m_adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_device)));

#if defined(_DEBUG)
	//Enable BreackOnSeverity
	ComPtr<ID3D12InfoQueue> infoQueue;
	ok = ok && SUCCEEDED(m_device.As(&infoQueue));
	infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
	infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
	infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
#endif

	return ok;
}
