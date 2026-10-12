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

private:
	std::unique_ptr<FrameContext> m_frameContexts[FRAMES_IN_FLIGHT];
	FrameContext*				  m_currentFrameContext = nullptr;
	RenderContext*				  m_currentRenderContext = nullptr;

	CommandQueue* m_commandQueue;
	SwapChain*	  m_swapChain;


	std::vector<std::unique_ptr<IRenderPass>> m_renderPasses;
};

