#pragma once

#include <GLM/glm/glm.hpp>


class EditorCamera
{
public:
	EditorCamera() = default;
	EditorCamera(glm::vec3 pos, float yaw, float pitch, float move_speed, float rot_speed);

	glm::mat4 ViewMat();
	glm::mat4 ProjMat(float aspect_ratio);

	void Translate(glm::vec3 dir, float dt);
	void Rotate(float dt_x, float dt_y);

	glm::vec3 GetPosition() { return mPosition; }
	void SetPosition(glm::vec3 pos) {
		mPosition = pos;
	}
	glm::vec3 GetForward() { return mForward; }
	glm::vec3 GetRight() { return mRight; }
	glm::vec3 GetUp() { return mUp; }

	float GetYaw() { return mYaw; }
	float GetPitch() { return mPitch; }


private:
	void Update();

	glm::vec3 mPosition = glm::vec3(0.0f);
	glm::vec3 mWorldUp = glm::vec3(0.0f, 1.0f, 0.0f);

	glm::vec3 mForward = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 mUp = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 mRight = glm::vec3(1.0f, 0.0f, 0.0f);


public:
	float mMoveSpeed = 100.0f;
	float mRotSpeed = 5.0f;

	float mFOV = 60.0f;
	float mNear = 0.1f;
	float mFar = 150.0f;

	float mYaw = 90.0f;
	float mPitch = 0.0f;
};