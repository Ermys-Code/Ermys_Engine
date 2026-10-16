#include "Core/Globals.h"
#include "ModuleCamera.h"

bool ModuleCamera::init()
{
    //UpdateViewMatrix();
    //UpdateProjectionMatrix();
    //UpdateViewProjectionMatrix();

    return false;
}

void ModuleCamera::update()
{
}

bool ModuleCamera::cleanUp()
{
    return false;
}

void ModuleCamera::SetCameraSettings(CameraSettings& settings)
{
    m_cameraSettings = settings;

    if (m_viewportHeight > 0.0f)
    {

    }
}


