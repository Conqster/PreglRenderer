#pragma once

#include "Display/DisplayManager.h"

#include "Shader.h"
#include "GPUVertexData.h"

#include <vector>
#include <array>



//need Window resize listener 

//struct Light
//{
//	glm::vec3 direction;
//	glm::vec3 colour;
//};

class Texture;
class Camera;
class Renderer
{
public:
	Renderer() = default;
	~Renderer() = default;

	Renderer(DisplayManager& display_window);
	void Initialise(DisplayManager& display_window);

	//This is just test renderable entity functionality 
	//Helpers
	void SubmitSpherePrimitive(const RenderableEntity entity) {
		AddFrameRenderableEntity({&mSpherePrimitive, entity.transform, entity.solidRender, entity.canCastShadow, entity.colour, entity.plainTexture});
	}
	void SubmitCubePrimitive(const RenderableEntity entity) {
		AddFrameRenderableEntity({&mCubePrimitive, entity.transform, entity.solidRender, entity.canCastShadow, entity.colour, entity.plainTexture});
	}
	void SubmitQuadPrimitive(const RenderableEntity entity) {
		AddFrameRenderableEntity({&mQuadPrimitive, entity.transform, entity.solidRender, entity.canCastShadow, entity.colour, entity.plainTexture});
	}

	glm::vec3& GetDirectionalLightPos() { return mLightWorldPos; }
	void SetDirectionalLightPos(const glm::vec3& pos) { mLightWorldPos = pos; }

	void BeginFrame(Camera* p_camera);
	void ShadowPass();
	void DrawPass();
	void EndFrame();


	void Destroy();
private:
	DisplayManager mDisplay;
	Camera* mCamera = nullptr; 

	glm::vec4 mClearColour = glm::vec4(0.0f, 0.0f, 0.5f, 1.0f);

	ShadowMap mShadowMapFBO;
	Shader mShadowShader;
	glm::vec3 mLightWorldPos = glm::vec3(10.0f, 40.0f, 0.0f);
	glm::mat4 mLightProjViewMat = glm::mat4(1.0f);


	Shader mWorldGridShader;


	void DrawObject(Shader& shader, bool only_depth = false);


	Shader mMeshShader;
	RenderableMesh mSpherePrimitive;
	RenderableMesh mCubePrimitive;
	RenderableMesh mQuadPrimitive;
	RenderableMesh mTrianglePrimitive;


	unsigned int mMaxFrameEntity = 500;
	std::array<RenderableEntity, 500> mFrameRenderableEntities;
	unsigned int mFrameEntitiesCount = 0;
	void AddFrameRenderableEntity(const RenderableEntity entity);


	Texture* mBrickTexture = nullptr;
	Texture* mCheckersTexture = nullptr;
	Texture* mPlainTexture = nullptr;
};