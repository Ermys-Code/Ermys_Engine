#pragma once

#include "Core/Globals.h"
#include "Modules/IModule.h"

#include "Settings/CameraSettings.h"

class ModuleCamera : public IModule
{
public:
	bool init()    override;
	void update()  override;
	bool cleanUp() override;

	void SetCameraSettings(CameraSettings& settings);
	void SetViewportSize(float width, float height);

	const Matrix& GetViewMatrix() const;
	const Matrix& GetProjectionMatrix() const;
	const Matrix& GetViewProjectionMatrix() const;

	const CameraSettings& GetCameraSettings() const { return m_cameraSettings; }

	const BoundingFrustum& GetWorldFrustum() const;

private:
	void UpdateViewMatrix();
	void UpdateProjectionMatrix();
	void UpdateViewProjectionMatrix();

	CameraSettings m_cameraSettings;

	float m_viewportWidth = 1280.0f;
	float m_viewportHeight = 720.0f;

	Matrix m_viewMatrix{};
	Matrix m_projectionMatrix{};
	Matrix m_viewProjectionMatrix{};

	BoundingFrustum m_viewFrustum{};
	BoundingFrustum m_worldFrustum{};
};

