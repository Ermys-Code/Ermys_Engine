#include "Core/Globals.h"
#include "Core/Application.h"

#include "Modules/ModuleInput.h"
#include "Modules/ModuleD3D12.h"
#include "Modules/ModuleRender.h"

Application::Application(int argc, wchar_t** argv, void* hWnd)
{
    m_modules.push_back(new ModuleInput((HWND)hWnd));
    m_modules.push_back(m_moduleD3D12 = new ModuleD3D12((HWND)hWnd));
    m_modules.push_back(new ModuleRender());
}

Application::~Application()
{
    cleanUp();

    for (auto it = m_modules.rbegin(); it != m_modules.rend(); ++it)
    {
        delete* it;
    }
}

bool Application::init()
{
    bool ret = true;

    for (auto it = m_modules.begin(); it != m_modules.end() && ret; ++it)
	{
		ret = (*it)->init();
	}

    m_lastMilis = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    return ret;
}

void Application::update()
{
    using namespace std::chrono_literals;

    uint64_t currentMilis = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    m_elapsedMilis = currentMilis - m_lastMilis;
    m_lastMilis = currentMilis;
    m_tickSum -= m_tickList[m_tickIndex];
    m_tickSum += m_elapsedMilis;
    m_tickList[m_tickIndex] = m_elapsedMilis;
    m_tickIndex = (m_tickIndex + 1) % MAX_FPS_TICKS;

    if (!app->m_paused)
    {
        for (auto it = m_modules.begin(); it != m_modules.end(); ++it)
        {
            (*it)->update();
        }

        for (auto it = m_modules.begin(); it != m_modules.end(); ++it)
        {
            (*it)->preRender();
        }

        for (auto it = m_modules.begin(); it != m_modules.end(); ++it)
        {
            (*it)->render();
        }

        for (auto it = m_modules.begin(); it != m_modules.end(); ++it)
        {
            (*it)->postRender();
        }
    }
}

bool Application::cleanUp()
{
    bool ret = true;

    for (auto it = m_modules.rbegin(); it != m_modules.rend() && ret; ++it)
    {
        ret = (*it)->cleanUp();
    }

    return ret;
}
