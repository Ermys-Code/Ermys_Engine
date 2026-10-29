#pragma once

#include "Core/Globals.h"
#include "Modules/IModule.h"

#include "Settings/CameraSettings.h"

class ModuleCamera : public IModule
{
public:
	ModuleCamera();
	~ModuleCamera();

	bool init()    override;
	void update()  override;
	bool cleanUp() override;

	void SetCameraSettings(CameraSettings& settings);
	const CameraSettings& GetCameraSettings() const { return m_cameraSettings; }
	
	const Matrix& GetViewMatrix() const			  { return m_viewMatrix; }
	const Matrix& GetProjectionMatrix() const     { return m_projectionMatrix; }
	const Matrix& GetViewProjectionMatrix() const { return m_viewProjectionMatrix; }

	void SetDirty() { m_isDirty = true; }
	void Resize();

	void Up();
	void Down();
	void Forward();
	void Backward();
	void Left();
	void Right();

private:
	void UpdateViewMatrix();
	void UpdateProjectionMatrix();
	void UpdateViewProjectionMatrix();

	CameraSettings m_cameraSettings{};

	float m_viewportWidth = 1280.0f;
	float m_viewportHeight = 720.0f;

	Matrix m_viewMatrix{};
	Matrix m_projectionMatrix{};
	Matrix m_viewProjectionMatrix{};

	bool m_isDirty = false;

	Vector3 m_position = Vector3(0.0f, 5.0f, 10.0f);

};

