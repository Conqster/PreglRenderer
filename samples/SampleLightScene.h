#pragma once

#include "Core/GraphicsProgramInterface.h"

#include "glm/glm.hpp"
#include "Renderer/Shader.h"
#include "Renderer/GPUResources.h"
#include "Renderer/Lighting.h"

#include <memory>

#include "Renderer/GPUVertexData.h"
#include "Core/EditorCamera.h"

#include "Core/ShaderHotReloadTracker.h"

class RenderableMesh;
class BaseMaterial;
class ImVec2;
class SampleLightingProgram : public GraphicsProgramInterface
{
public:
	SampleLightingProgram() {}
	SampleLightingProgram(AppWindow* display_manager) : GraphicsProgramInterface(display_manager){}

	virtual void OnInitialise(AppWindow* display_window) override;
	virtual void OnUpdate(float delta_time) override;
	virtual void OnLateUpdate(float delta_time) override;
	virtual void OnDestroy() override;
	virtual void OnUI() override;

	~SampleLightingProgram() {
		printf("Sample Gfx Program Closed!!!!!!\n");
	}


private: 
	glm::vec4 mClearColour = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);

	Shader mShader;
	Shader mShadowShader;
	Shader mTexScrceenShader;
	Util::ShaderHotReloadTracker mShaderHotReloadTracker;
	
	glm::mat4 mLightProjViewMat = glm::mat4(1.0f);
	GPUResource::ShadowMap mShadowMapFBO;
	GPUResource::UniformBuffer mCameraUBO;
	GPUResource::UniformBuffer mLightUBO;

	Lighting::Directional mDirLight;
	Lighting::Point mPointLight;
	Lighting::Spot mSpotLight;

	//scene object
	std::shared_ptr<RenderableMesh> mSphereMesh = nullptr;
	std::shared_ptr<RenderableMesh> mQuadMesh = nullptr;
	std::shared_ptr<RenderableMesh> mCubeMesh = nullptr;
	std::shared_ptr<RenderableMesh> mConeMesh = nullptr;

	std::shared_ptr<BaseMaterial> mSphereMaterial = nullptr;
	std::shared_ptr<BaseMaterial> mQuadMaterial = nullptr;
	std::shared_ptr<BaseMaterial> mCubeMaterial = nullptr;

	std::vector<glm::mat4> mSphereTransform;
	std::vector<glm::mat4> m_PlaneTransform;
	std::vector<glm::mat4> m_CubeTransform;
	std::vector<glm::mat4> m_ConeTransform;


	void CreateObjects();
	void UpdateCameraUBO(EditorCamera& cam, float aspect_ratio);
	void UpdateShaders();


	void BeginFrame();
	void ShadowPass();

	void MaterialShaderHelper(Shader& shader, const BaseMaterial& mat);


	void MainSceneUIEditor();
	//Helper 
	EditorCamera mMaterialPreviewCam;
	GPUResource::Framebuffer mMaterialPreviewFBO;
	std::weak_ptr<BaseMaterial> mPtrPreviewMaterial;
	float mPreviewAmbientRatio = 0.4f;
	Shader mPreviewShader;
	glm::mat4 mPreviewTransform = glm::mat4(1.0f);
	//void MaterialEditorUIWindow();
	//bool Ext_QuatEditorPanel(glm::mat4& transform, ImVec2 top_left, ImVec2 size);

	//Utility 
	std::shared_ptr<GPUResource::Texture> mErrorTex = nullptr;
	std::shared_ptr<GPUResource::Texture> mNoiseTex = nullptr;
	std::shared_ptr<GPUResource::Texture> mNoiseTex2 = nullptr;
};