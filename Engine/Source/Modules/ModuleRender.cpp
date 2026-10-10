#include "Core/Globals.h"
#include "Modules/ModuleRender.h"

#include "Core/Application.h"

#include "Modules/ModuleD3D12.h"

#include "D3D12/SwapChain.h"

//#include "Passes/IRenderPass.h"

ModuleRender::ModuleRender()
{
}

ModuleRender::~ModuleRender()
{
}

bool ModuleRender::init()
{
	m_swapChain    = app->GetModuleD3D12()->GetSwapChain();
	m_commandList  = app->GetModuleD3D12()->GetCommandList();
	m_commandQueue = app->GetModuleD3D12()->GetCommandQueue();

	return true;
}

void ModuleRender::preRender()
{
	m_currentFrameContext = m_swapChain->StartNewFrame();

	m_currentFrameContext.commandAllocator->Reset();
}

void ModuleRender::render()
{

	m_commandList->Reset(m_currentFrameContext.commandAllocator.Get(), nullptr);

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



	//for (auto& pass : m_renderPasses)
	//{
	//	pass->prepare();
	//	pass->render();
	//}
}

void ModuleRender::postRender()
{
	m_swapChain->FinishCurrentFrame(m_commandQueue);
}

bool ModuleRender::cleanUp()
{
	return true;
}
