#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>

using namespace DirectX;

class Camera
{
protected:
	XMFLOAT3 m_eye; 
	XMFLOAT3 m_at;
	XMFLOAT3 m_up;

	FLOAT m_windowWidth;
	FLOAT m_windowHeight;
	FLOAT m_nearDepth;
	FLOAT m_farDepth;

	XMFLOAT4X4 m_view;
	XMFLOAT4X4 m_projection;

public:
	Camera(XMFLOAT3 position, XMFLOAT3 at, XMFLOAT3 up, FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth);
	~Camera();

	void Update();

	XMFLOAT4X4 GetView() const { return m_view; }
	XMFLOAT4X4 GetProjection() const { return m_projection; }

	XMFLOAT4X4 GetViewProjection() const;

	XMFLOAT3 GetPosition() const { return m_eye; }
	XMFLOAT3 GetLookAt() const { return m_at; }
	XMFLOAT3 GetUp() const { return m_up; }

	void SetPosition(XMFLOAT3 position) { m_eye = position; }
	void SetLookAt(XMFLOAT3 lookAt) { m_at = lookAt; }
	void SetUp(XMFLOAT3 up) { m_up = up; }

	void Reshape(FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth);

	virtual void updateLogic(float dt) = 0; // this is to be overriden
};

