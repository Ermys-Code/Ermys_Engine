#pragma once

#include "Modules/IModule.h"

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
	//std::vector<std::unique_ptr<IRenderPass>> m_renderPasses;
};

