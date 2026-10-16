#pragma once

#include "Modules/IModule.h"
#include "Rendering/FrameContext.h"
#include "Rendering/RenderContext.h"

class IRenderPass;
class SwapChain;
class CommandQueue;

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
	
	CommandQueue* GetCommandQueue() { return m_commandQueue; }

private:
	std::unique_ptr<FrameContext> m_frameContexts[FRAMES_IN_FLIGHT];
	
	FrameContext* m_currentFrameContext   = nullptr;
	RenderContext* m_currentRenderContext = nullptr;

	CommandQueue* m_commandQueue = nullptr;
	SwapChain* m_swapChain       = nullptr;


	std::vector<std::unique_ptr<IRenderPass>> m_renderPasses;
};

