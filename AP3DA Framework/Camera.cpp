#include "Camera.h"

Camera::Camera(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 at, DirectX::XMFLOAT3 up, FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth)
	: m_eye(position), m_at(at), m_up(up), m_windowWidth(windowWidth), m_windowHeight(windowHeight), m_nearDepth(nearDepth), m_farDepth(farDepth)
{
	Update();
}

Camera::~Camera()
{
}

void Camera::Update()
{
    // Initialize the view matrix

	DirectX::XMFLOAT4 eye = DirectX::XMFLOAT4(m_eye.x, m_eye.y, m_eye.z, 1.0f);
	DirectX::XMFLOAT4 at = DirectX::XMFLOAT4(m_at.x, m_at.y, m_at.z, 1.0f);
	DirectX::XMFLOAT4 up = DirectX::XMFLOAT4(m_up.x, m_up.y, m_up.z, 0.0f);

	DirectX::XMVECTOR EyeVector = DirectX::XMLoadFloat4(&eye);
	DirectX::XMVECTOR AtVector = DirectX::XMLoadFloat4(&at);
	DirectX::XMVECTOR UpVector = DirectX::XMLoadFloat4(&up);

	DirectX::XMStoreFloat4x4(&m_view, DirectX::XMMatrixLookAtLH(EyeVector, AtVector, UpVector));

    // Initialize the projection matrix
	DirectX::XMStoreFloat4x4(&m_projection, DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV2, m_windowWidth / m_windowHeight, m_nearDepth, m_farDepth));
}

void Camera::Reshape(FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth)
{
	m_windowWidth = windowWidth;
	m_windowHeight = windowHeight;
	m_nearDepth = nearDepth;
	m_farDepth = farDepth;
}

DirectX::XMFLOAT4X4 Camera::GetViewProjection() const 
{ 
	DirectX::XMMATRIX view = DirectX::XMLoadFloat4x4(&m_view);
	DirectX::XMMATRIX projection = DirectX::XMLoadFloat4x4(&m_projection);

	DirectX::XMFLOAT4X4 viewProj;

	DirectX::XMStoreFloat4x4(&viewProj, view * projection);

	return viewProj;
}