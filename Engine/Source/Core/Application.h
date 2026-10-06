#pragma once

#include "Globals.h"

#include <array>
#include <vector>
#include <chrono>

class Module;

class Application
{
public:
	Application(int argc, wchar_t** argv, void* hWnd);
	~Application();

	bool init();
	void update();
	bool cleanUp();

	float    getFPS() const { return 1000.0f * float(MAX_FPS_TICKS) / m_tickSum; }
	float    getAvgElapsedMs() const { return m_tickSum / float(MAX_FPS_TICKS); }
	uint64_t getElapsedMilis() const { return m_elapsedMilis; }

	bool isPaused() const { return m_paused; }
	bool setPaused(bool p) { m_paused = p; return m_paused; }

private:
	enum { MAX_FPS_TICKS = 30 };
	typedef std::array<uint64_t, MAX_FPS_TICKS> TickList;

	std::vector<Module*> m_modules;

	uint64_t  m_lastMilis = 0;
	TickList  m_tickList;
	uint64_t  m_tickIndex;
	uint64_t  m_tickSum = 0;
	uint64_t  m_elapsedMilis = 0;
	bool      m_paused = false;
};

extern Application* app;