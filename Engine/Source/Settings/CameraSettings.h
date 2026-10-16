#pragma once

#include "Core/Globals.h"

struct CameraSettings
{
	Vector3 position = { 0.0f, 0.0f, -10.0f};
	Vector3 forward  = { 0.0f, 0.0f, 1.0f };
	Vector3 up       = { 0.0f, 1.0f, 0.0f };

	float fovYRadiants = DirectX::XMConvertToRadians(60.0f);
	float aspectRatio  = 16.0f / 9.0f;
	float nearPlane    = 0.1f;
	float farPlane	   = 1000.0f;
};