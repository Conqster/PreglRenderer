#pragma once

#include "Core/GraphicsProgramInterface.h"

#include "Core/EditorCamera.h"
#include "GLM/glm/glm.hpp"
#include "Renderer/Shader.h"
#include "Renderer/GPUVertexData.h"

struct Light
{
	glm::vec3 direction;
	glm::vec3 colour;
};

class Texture;
class SampleGfxProgram : public GraphicsProgramInterface
{
public:
	SampleGfxProgram() {}
	SampleGfxProgram(AppWindow* display_window) : GraphicsProgramInterface(display_window){}

	virtual void OnInitialise(AppWindow* display_window) override;
	virtual void OnUpdate(float delta_time) override;
	virtual void OnDestroy() override;
	virtual void OnUI() override;

	~SampleGfxProgram() {
		printf("Sample Gfx Program Closed!!!!!!\n");
	}

private:

	glm::vec4 mClearColour = glm::vec4(0.0f, 0.0f, 0.5f, 1.0f);

	ShadowMap mShadowMapFBO;
	Shader mShadowShader;
	glm::vec3 mLightWorldPos = glm::vec3(10.0f, 40.0f, 0.0f);
	glm::mat4 mLightProjViewMat = glm::mat4(1.0f);


	Shader mWorldGridShader;


	void DrawObject(Shader& shader, bool only_depth = false);

	void BeginFrame();
	void ShadowPass();

	void InputCheck(float delta_time);

	Shader mMeshShader;
	RenderableMesh mSpherePrimitive;
	RenderableMesh mCubePrimitive;
	RenderableMesh mQuadPrimitive;
	RenderableMesh mTrianglePrimitive;

	glm::mat4* ptr_model_transform = nullptr;

	std::vector<RenderableEntity> mEntities;
	void CreateEntities();

	Texture* mBrickTexture = nullptr;
	Texture* mCheckersTexture = nullptr;
	Texture* mPlainTexture = nullptr;
};