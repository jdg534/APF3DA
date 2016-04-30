#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>

// using namespace DirectX;

class Camera
{
protected:
	DirectX::XMFLOAT3 m_eye;
	DirectX::XMFLOAT3 m_at;
	DirectX::XMFLOAT3 m_up;

	FLOAT m_windowWidth;
	FLOAT m_windowHeight;
	FLOAT m_nearDepth;
	FLOAT m_farDepth;

	DirectX::XMFLOAT4X4 m_view;
	DirectX::XMFLOAT4X4 m_projection;

public:
	Camera(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 at, DirectX::XMFLOAT3 up, FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth);
	~Camera();

	void Update();

	DirectX::XMFLOAT4X4 GetView() const { return m_view; }
	DirectX::XMFLOAT4X4 GetProjection() const { return m_projection; }

	DirectX::XMFLOAT4X4 GetViewProjection() const;

	DirectX::XMFLOAT3 GetPosition() const { return m_eye; }
	DirectX::XMFLOAT3 GetLookAt() const { return m_at; }
	DirectX::XMFLOAT3 GetUp() const { return m_up; }

	void SetPosition(DirectX::XMFLOAT3 position) { m_eye = position; }
	void SetLookAt(DirectX::XMFLOAT3 lookAt) { m_at = lookAt; }
	void SetUp(DirectX::XMFLOAT3 up) { m_up = up; }

	void Reshape(FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth);

	virtual void updateLogic(float dt) {} // this is to be overriden
};

