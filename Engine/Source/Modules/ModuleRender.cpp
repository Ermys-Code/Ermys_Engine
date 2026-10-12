#include "Core/Globals.h"
#include "Modules/ModuleRender.h"

#include "Core/Application.h"

#include "Modules/ModuleD3D12.h"

#include "D3D12/SwapChain.h"
#include "D3D12/CommandQueue.h"

#include "Passes/IRenderPass.h"
#include "Passes/TrianglePass.h"

ModuleRender::ModuleRender()
{
}

ModuleRender::~ModuleRender()
{
}

bool ModuleRender::init()
{
	for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
	{
		m_frameContexts[i] = std::make_unique<FrameContext>(app->GetModuleD3D12()->GetDevice());
	}

	m_swapChain    = app->GetModuleD3D12()->GetSwapChain();
	m_commandQueue = app->GetModuleD3D12()->GetCommandQueue();

	m_renderPasses.push_back(std::make_unique<TrianglePass>());
	
	return true;
}

void ModuleRender::preRender()
{
	m_currentRenderContext = m_swapChain->GetCurrentRenderContext();

	m_currentFrameContext = m_frameContexts[m_currentRenderContext->frameIndex].get();

	m_commandQueue->WaitForFenceValue(m_currentFrameContext->fenceValue);

	m_currentFrameContext->commandContext->Begin();
}

void ModuleRender::render()
{
	ID3D12GraphicsCommandList4* commandList = m_currentFrameContext->commandContext->GetCommandList();
	
	m_swapChain->CreateBarrierForCurrentFrame(commandList, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	commandList->ClearRenderTargetView(m_currentRenderContext->rtvCpuHandle, clearColor, 0, nullptr);
	
	for (auto& pass : m_renderPasses)
	{
		pass->prepare(m_currentRenderContext, m_currentFrameContext);
		pass->render(commandList);
	}

	m_swapChain->CreateBarrierForCurrentFrame(commandList, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

}

void ModuleRender::postRender()
{
	m_currentFrameContext->commandContext->Close();

	m_commandQueue->ExecuteCommandList(m_currentFrameContext->commandContext->GetCommandList());

	m_swapChain->PresentCurrentFrame();

	m_currentFrameContext->fenceValue = m_commandQueue->Signal();
}

bool ModuleRender::cleanUp()
{
	return true;
}
