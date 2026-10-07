#include "Core/Globals.h"
#include "Modules/ModuleRender.h"

//#include "Passes/IRenderPass.h"

ModuleRender::ModuleRender()
{
}

ModuleRender::~ModuleRender()
{
}

bool ModuleRender::init()
{
	return true;
}

void ModuleRender::preRender()
{
}

void ModuleRender::render()
{
	//for (auto& pass : m_renderPasses)
	//{
	//	pass->prepare();
	//	pass->render();
	//}
}

void ModuleRender::postRender()
{
}

bool ModuleRender::cleanUp()
{
	return true;
}
