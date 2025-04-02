#include "Camera.h"

#include <GLM/glm/gtc/matrix_transform.hpp>

Camera::Camera(glm::vec3 pos, float yaw, float pitch, float move_speed, float rot_speed)
	: mPosition(pos), mYaw(yaw), mPitch(pitch), mMoveSpeed(move_speed), mRotSpeed(rot_speed)
{
	Update();
}

glm::mat4 Camera::ViewMat()
{
	Update();
	return glm::lookAt(mPosition, mPosition + mForward, mUp);
}

glm::mat4 Camera::ProjMat(float aspect_ratio)
{
	return glm::perspective(glm::radians(mFOV), aspect_ratio, mNear, mFar);
}

void Camera::Translate(glm::vec3 dir, float dt)
{
	mPosition += dir * mMoveSpeed * dt;// *0.01f;
}

void Camera::Rotate(float dt_x, float dt_y)
{
	mYaw += dt_x * mRotSpeed * 0.01f;
	mPitch += dt_y * mRotSpeed * 0.01f;

	if (mYaw > 360.0f)
		mYaw = mYaw - 360;
	if (mYaw < 0)
		mYaw = 360 - mYaw;


	if (mPitch > 89.0f)
		mPitch = 89.0f;
	if (mPitch < -89.0f)
		mPitch = -89.0f;

	Update();
}

void Camera::Update()
{
	mForward.x = cos(glm::radians(mYaw)) * cos(glm::radians(mPitch));
	mForward.y = sin(glm::radians(mPitch));
	mForward.z = sin(glm::radians(mYaw)) * cos(glm::radians(mPitch));
	mForward = glm::normalize(mForward);

	mRight = glm::normalize(glm::cross(mForward, mWorldUp));
	mUp = glm::normalize(glm::cross(mRight, mForward));
}
