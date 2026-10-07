#pragma once

#include "Core/Globals.h"

class IModule
{
public:

	IModule() {}

	virtual ~IModule() {}

	virtual bool init()
	{
		return true;
	}

	virtual void update() {}

	virtual void preRender() {}

	virtual void postRender() {}

	virtual void render() {}

	virtual bool cleanUp()
	{
		return true;
	}
};

