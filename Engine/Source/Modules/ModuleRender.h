#pragma once

#include "Modules/IModule.h"
#include "D3D12/SwapChain.h"

//class IRenderPass;

class ModuleRender : public IModule
{
public:
	ModuleRender();
	~ModuleRender();

	bool init()		  override;
	void preRender()  override;
	void render()	  override;
	void postRender() override;
	bool cleanUp()	  override;

private:
	SwapChain::FrameContext m_currentFrameContext;

	SwapChain*						  m_swapChain;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	ComPtr<ID3D12CommandQueue>		  m_commandQueue;

	//std::vector<std::unique_ptr<IRenderPass>> m_renderPasses;
};

