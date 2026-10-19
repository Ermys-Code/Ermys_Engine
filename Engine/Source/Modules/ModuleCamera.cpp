#include "Core/Globals.h"
#include "ModuleCamera.h"

#include "Core/Application.h"
#include "Modules/ModuleD3D12.h"
#include "D3D12/SwapChain.h"

ModuleCamera::ModuleCamera()
{
}

ModuleCamera::~ModuleCamera()
{
}

bool ModuleCamera::init()
{
    UpdateViewMatrix();
    UpdateProjectionMatrix();
    UpdateViewProjectionMatrix();

    return true;
}

void ModuleCamera::update()
{
    if (m_isDirty)
    {
        Resize();
        UpdateViewMatrix();
        UpdateProjectionMatrix();
        UpdateViewProjectionMatrix();

        m_isDirty = false;
    }
}

bool ModuleCamera::cleanUp()
{
    return true;
}

void ModuleCamera::SetCameraSettings(CameraSettings& settings)
{
    m_cameraSettings = settings;

    if (m_viewportHeight > 0.0f)
    {

    }
}

void ModuleCamera::Resize()
{
    unsigned int width, height;
    app->GetModuleD3D12()->GetSwapChain()->GetWindowSize(width, height);
    m_cameraSettings.aspectRatio = float(width) / float(height);
}

void ModuleCamera::UpdateViewMatrix()
{
    m_viewMatrix = Matrix::CreateLookAt(Vector3(0.0f, 5.0f, 10.0f), Vector3::Zero, Vector3::Up);
}

void ModuleCamera::UpdateProjectionMatrix()
{
    unsigned int width, height;
    app->GetModuleD3D12()->GetSwapChain()->GetWindowSize(width, height);
    m_projectionMatrix = Matrix::CreatePerspectiveFieldOfView(m_cameraSettings.fovYRadiants, m_cameraSettings.aspectRatio, m_cameraSettings.nearPlane, m_cameraSettings.farPlane);
}

void ModuleCamera::UpdateViewProjectionMatrix()
{
    m_viewProjectionMatrix = m_viewMatrix * m_projectionMatrix;
}
