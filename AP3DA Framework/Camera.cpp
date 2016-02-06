#include "Camera.h"

Camera::Camera(XMFLOAT3 position, XMFLOAT3 at, XMFLOAT3 up, FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth)
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

	XMFLOAT4 eye = XMFLOAT4(m_eye.x, m_eye.y, m_eye.z, 1.0f);
	XMFLOAT4 at = XMFLOAT4(m_at.x, m_at.y, m_at.z, 1.0f);
	XMFLOAT4 up = XMFLOAT4(m_up.x, m_up.y, m_up.z, 0.0f);

	XMVECTOR EyeVector = XMLoadFloat4(&eye);
	XMVECTOR AtVector = XMLoadFloat4(&at);
	XMVECTOR UpVector = XMLoadFloat4(&up);

	XMStoreFloat4x4(&m_view, DirectX::XMMatrixLookAtLH(EyeVector, AtVector, UpVector));

    // Initialize the projection matrix
	XMStoreFloat4x4(&m_projection, XMMatrixPerspectiveFovLH(XM_PIDIV2, m_windowWidth / m_windowHeight, m_nearDepth, m_farDepth));
}

void Camera::Reshape(FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth)
{
	m_windowWidth = windowWidth;
	m_windowHeight = windowHeight;
	m_nearDepth = nearDepth;
	m_farDepth = farDepth;
}

XMFLOAT4X4 Camera::GetViewProjection() const 
{ 
	XMMATRIX view = XMLoadFloat4x4(&m_view);
	XMMATRIX projection = XMLoadFloat4x4(&m_projection);

	XMFLOAT4X4 viewProj;

	XMStoreFloat4x4(&viewProj, view * projection);

	return viewProj;
}