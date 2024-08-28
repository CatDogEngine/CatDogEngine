#include "CameraComponent.h"

namespace engine
{

void CameraComponent::BuildViewMatrix(const cd::Transform& transform)
{
	cd::Vec3f lookAt = GetLookAt(transform).Normalize();
	cd::Vec3f up = GetUp(transform).Normalize();
	cd::Vec3f eye = transform.GetTranslation();
	m_viewMatrix = cd::Matrix4x4::LookAt<cd::Handedness::Left>(eye, eye + lookAt, up);
	m_projectionMatrix = cd::Matrix4x4::Perspective(m_fov, m_aspect, m_nearPlane, m_farPlane, cd::NDCDepth::MinusOneToOne == m_ndcDepth);
}

void CameraComponent::BuildViewMatrix(const cd::Vec3f& eye, const cd::Vec3f& lookAt, const cd::Vec3f& up)
{
	m_viewMatrix = cd::Matrix4x4::LookAt<cd::Handedness::Left>(eye, eye + lookAt, up);
}

void CameraComponent::BuildProjectMatrix()
{
	m_projectionMatrix = cd::Matrix4x4::Perspective(m_fov, m_aspect, m_nearPlane, m_farPlane, cd::NDCDepth::MinusOneToOne == m_ndcDepth);
}

std::vector<float> CameraComponent::getProjectionMatrix(double fx, double fy, float width, float height)
{
	const double znear = 0.2;
	const double zfar = 200.0;
	std::vector<float> projectionMatrix = {
		static_cast<float>(2 * fx / width), 0.0f, 0.0f, 0.0f,
		0.0f, static_cast<float>(-2 * fy / height), 0.0f, 0.0f,
		0.0f, 0.0f, static_cast<float>(zfar / (zfar - znear)), 1.0f,
		0.0f, 0.0f, static_cast<float>(-zfar * znear / (zfar - znear)), 0.0f
	};
	return projectionMatrix;
}

std::vector<float> CameraComponent::getViewMatrix(const float R[3][3], const float t[3])
{
	std::vector<float> camToWorld(16);

	camToWorld[0] = R[0][0]; camToWorld[1] = R[0][1]; camToWorld[2] = R[0][2]; camToWorld[3] = 0;
	camToWorld[4] = R[1][0]; camToWorld[5] = R[1][1]; camToWorld[6] = R[1][2]; camToWorld[7] = 0;
	camToWorld[8] = R[2][0]; camToWorld[9] = R[2][1]; camToWorld[10] = R[2][2]; camToWorld[11] = 0;
	camToWorld[12] = -t[0] * R[0][0] - t[1] * R[1][0] - t[2] * R[2][0];
	camToWorld[13] = -t[0] * R[0][1] - t[1] * R[1][1] - t[2] * R[2][1];
	camToWorld[14] = -t[0] * R[0][2] - t[1] * R[1][2] - t[2] * R[2][2];
	camToWorld[15] = 1;

	return camToWorld;
}

cd::Ray CameraComponent::EmitRay(float screenX, float screenY, float width, float height) const
{
	cd::Matrix4x4 vpInverse = m_projectionMatrix * m_viewMatrix;
	vpInverse = vpInverse.Inverse();

	float x = cd::Math::GetValueInNewRange(screenX / width, 0.0f, 1.0f, -1.0f, 1.0f);
	float y = cd::Math::GetValueInNewRange(screenY / height, 0.0f, 1.0f, -1.0f, 1.0f);

	cd::Vec4f near = vpInverse * cd::Vec4f(x, -y, 0.0f, 1.0f);
	near /= near.w();

	cd::Vec4f far = vpInverse * cd::Vec4f(x, -y, 1.0f, 1.0f);
	far /= far.w();

	cd::Vec4f direction = (far - near).Normalize();
	return cd::Ray(cd::Vec3f(near.x(), near.y(), near.z()),
		cd::Vec3f(direction.x(), direction.y(), direction.z()));
}

void CameraComponent::SetLookAt(const cd::Vec3f& lookAt, cd::Transform& transform)
{
	cd::Vec3f rotAxis = GetLookAt(transform).Cross(lookAt);
	float rotAngle = std::acos(GetLookAt(transform).Dot(lookAt));
	transform.SetRotation(transform.GetRotation() * cd::Quaternion::FromAxisAngle(rotAxis, rotAngle));
}

void CameraComponent::SetUp(const cd::Vec3f& up, cd::Transform& transform)
{
	cd::Vec3f rotAxis = GetUp(transform).Cross(up);
	float rotAngle = std::acos(GetUp(transform).Dot(up));
	transform.SetRotation(transform.GetRotation() * cd::Quaternion::FromAxisAngle(rotAxis, rotAngle));
}

void CameraComponent::SetCross(const cd::Vec3f& cross, cd::Transform& transform)
{
	cd::Vec3f rotAxis = GetCross(transform).Cross(cross);
	float rotAngle = std::acos(GetUp(transform).Dot(cross));
	transform.SetRotation(transform.GetRotation() * cd::Quaternion::FromAxisAngle(rotAxis, rotAngle));
}

void CameraComponent::FrameAll(const cd::AABB& aabb, cd::Transform& transform)
{
	if (aabb.IsEmpty())
	{
		return;
	}

	cd::Point lookAt = aabb.Center();
	cd::Direction lookDirection(0.0f, 0.0f, 1.0f);
	lookDirection.Normalize();
	SetLookAt(lookDirection, transform);

	cd::Point lookFrom = lookAt - lookDirection * aabb.Size().Length();
	transform.SetTranslation(cd::MoveTemp(lookFrom));
}

}