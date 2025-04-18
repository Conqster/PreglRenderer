#include "SampleLightScene.h"

#include "Core/Util.h"
#include "Renderer/GPUVertexData.h"

#include "Loader/Loader.h"
#include "Loader/ModelLoader.h"

#include "Renderer/Material.h"
#include <GLM/glm/gtc/matrix_transform.hpp>

#include "Core/EditorCamera.h"

#include <libs/imgui/imgui.h>
#include "glm/gtc/quaternion.hpp"

#include "Core/Log.h"

#include "Renderer/DebugGizmosRenderer.h"

#include "Core/UI_Window_Panel_Editors.h"

#include <random>

#include "Core/HeapMemAllocationTracking.h"

void SampleLightingProgram::OnInitialise(AppWindow* display_window)
{
	display_window->ChangeWindowTitle("Lighting Program Model");
	mDisplayManager = display_window;
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glDepthFunc(GL_LEQUAL);

	if (!mErrorTex)
		mErrorTex = std::make_shared<GPUResource::Texture>("assets/textures/Blank.png", false);

	PGL_ASSERT_WARN(mErrorTex, "Failed to create Error Texture");

	CreateObjects();
}

void SampleLightingProgram::OnUpdate(float delta_time)
{
	BeginFrame(); 

	ShadowPass();

	mShader.Bind();
	mShadowMapFBO.Read(0);
	mShader.SetUniform1i("uShadowMap", 0);
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//plane 
	if (mQuadMaterial)
	{

		MaterialShaderHelper(mShader, *mQuadMaterial);
		//Draw planes
		for (const auto& model : m_PlaneTransform)
		{
			mShader.SetUniformMat4("uModel", model);
			mQuadMesh->Draw();
		}
	}



	//sphere
	if (mSphereMaterial)
	{
		MaterialShaderHelper(mShader, *mSphereMaterial);
		//Draw sphere
		for (const auto& model : m_SphereTransform)
		{
			mShader.SetUniformMat4("uModel", model);
			mSphereMesh->Draw();
		}
	}



	//cube
	if (mCubeMaterial)
	{
		MaterialShaderHelper(mShader, *mCubeMaterial);
		//Draw sphere
		for (const auto& model : m_CubeTransform)
		{
			mShader.SetUniformMat4("uModel", model);
			mCubeMesh->Draw();
		}
	}


	if (mConeMesh)
	{
		if (!mCubeMaterial)
			MaterialShaderHelper(mShader, *mCubeMaterial);
		//Draw sphere
		for (const auto& model : m_ConeTransform)
		{
			mShader.SetUniformMat4("uModel", model);
			mConeMesh->Draw();
		}
	}


	glDisable(GL_BLEND);

	//Draw a sample line connecting spheres
	for (int i = 1; i < m_SphereTransform.size(); i++)
	{
		DebugGizmosRenderer::Instance().DrawLine(m_SphereTransform[i - 1][3], m_SphereTransform[i][3], glm::vec3(0.0f, 0.0f, 1.0f));
	}

	//ROTATE ALL CONEs
	static glm::mat4 origin = glm::mat4(1.0f);
	//origin = glm::rotate(glm::mat4(1.0f), glm::radians(30.0f * delta_time), glm::vec3(0.0f, 1.0f, 0.0f));
	for (auto& model : m_ConeTransform)
	{
		model = glm::translate(origin, glm::vec3(model[3])) * glm::rotate(model, glm::radians(15.0f * delta_time), glm::vec3(1.0f, 1.0f, 1.0f));
		////fake retrive origin offset 
		//glm::vec3 translate = model[3];
		//model = glm::translate(origin, translate);
	}
}

void SampleLightingProgram::OnDestroy()
{
	mShader.Clear();
	mCameraUBO.Delete();
	mLightUBO.Delete();


	//scene object
	if (mSphereMesh)
	{
		mSphereMesh->Clear();
		mSphereMesh.reset();
	}
	if(mQuadMesh)
	{
		mQuadMesh->Clear();
		mQuadMesh.reset();
	}
	if(mCubeMesh)
	{
		mCubeMesh->Clear();
		mCubeMesh.reset();
	}
	if(mConeMesh)
	{
		mConeMesh->Clear();
		mConeMesh.reset();
	}

	mSphereMaterial = nullptr;
	mQuadMaterial = nullptr;
	mCubeMaterial = nullptr;


	mShadowMapFBO.Destroy();
	mMaterialPreviewFBO.Delete();
	mErrorTex.reset();
}

void SampleLightingProgram::OnUI()
{

	static UI::Windows::MaterialList mMaterialList{
		mSphereMaterial,
		mQuadMaterial,
		mCubeMaterial,
	};
	UI::Windows::MaterialsEditor(mMaterialList);

	UI::Windows::SingleTextureEditor(*mQuadMaterial->diffuseMap);

	/*static bool test = */ HELPER_REGISTER_UIFLAG("Lets Go", p_open_flag);

	if (p_open_flag)
	{
		if (ImGui::Begin("Lets Go", &p_open_flag))
		{
			ImGui::Text("bsjbhsa os oshigp jwpiasdfhipasiphgrisfpj ipgji pj");
			ImGui::Text("bsjbhsa os oshigp jwpiasdfhipasiphgrisfpj ipgji pj");
			ImGui::Text("bsjbhsa os oshigp jwpiasdfhipasiphgrisfpj ipgji pj");
			ImGui::Text("bsjbhsa os oshigp jwpiasdfhipasiphgrisfpj ipgji pj");
		}
		ImGui::End();

	}

	if (mNoiseTex)
		UI::Windows::SingleTextureEditor(*mNoiseTex);

	if (mNoiseTex2)
		UI::Windows::SingleTextureEditor(*mNoiseTex2);

}




void SampleLightingProgram::CreateObjects()
{
	mSphereMesh = std::make_shared<RenderableMesh>(Util::CreateSphere());
	mQuadMesh = std::make_shared<RenderableMesh>(Loader::LoadMesh("assets/meshes/quad.rmesh"));
	mCubeMesh = std::make_shared<RenderableMesh>(Loader::LoadMesh("assets/meshes/cube.rmesh"));
	//for now load bunny instead of cone
	ModelLoader model_loader;
	mConeMesh = std::make_shared<RenderableMesh>(model_loader.LoadAsSingleMesh("assets/models/blendershapes/blender_cone.fbx"));


	bool success = mShader.Create("lighting_shader", "assets/shaders/lighting/ModelLighting.vert", "assets/shaders/lighting/ModelLighting.frag");


	mQuadMaterial = std::make_shared<BaseMaterial>();
	mQuadMaterial->name = "Quad_Mat";
	mSphereMaterial = std::make_shared<BaseMaterial>();
	mSphereMaterial->name = "Sphere_Mat";
	mCubeMaterial = std::make_shared<BaseMaterial>();
	mCubeMaterial->name = "Cube_Mat_test";


	mQuadMaterial->ambient = glm::vec3(0.0468f, 0.3710993f, 0.07421f);
	mQuadMaterial->diffuse = glm::vec3(0.0f, 0.96875f, 0.1406f);
	mQuadMaterial->specular = glm::vec3(0.35156f, 0.67578f, 0.37109f);

	mSphereMaterial->ambient = glm::vec3(0.3710993f, 0.07421f, 0.0468f);
	mSphereMaterial->diffuse = glm::vec3(0.96875f, 0.f, 0.1406f);
	mSphereMaterial->specular = glm::vec3(0.67578f, 0.37109f, 0.35156f);

	mCubeMaterial->ambient = glm::vec3(0.109375f, 0.35546f, 0.578125f);
	mCubeMaterial->diffuse = glm::vec3(0.1406f, 0.f, 0.96875f);
	mCubeMaterial->specular = glm::vec3(0.4726f, 0.8984f, 0.9921f);

	mQuadMaterial->diffuseMap = std::make_shared<GPUResource::Texture>("assets/textures/floor_brick/patterned_brick_floor_diff.jpg", true);
	mQuadMaterial->normalMap = std::make_shared<GPUResource::Texture>("assets/textures/floor_brick/patterned_brick_floor_nor.jpg", true);
	mQuadMaterial->specularMap = std::make_shared<GPUResource::Texture>("assets/textures/floor_brick/patterned_brick_floor_ao.jpg", true);


	////////////////////////////////////////
	// UNIFORM BUFFERs
	////////////////////////////////////////
	//------------------Camera Matrix Data UBO-----------------------------/
	long long int buf_size = sizeof(glm::vec3);//for view pos
	buf_size += sizeof(float);// camera far
	buf_size += 2 * sizeof(glm::mat4);// +sizeof(glm::vec2);   //to store view, projection
	mCameraUBO.Generate(buf_size);
	mCameraUBO.BindBufferRndIdx(0, buf_size, 0);

	buf_size = Lighting::Directional::GetGPUShaderSize();
	//printf("memory size of Direction Light: %d bytes\n", int(sizeof(Lighting::Directional)));
	//printf("memory size of GPU usage: %d bytes\n", int(buf_size));
	//only directional light at the moment
	mLightUBO.Generate(buf_size);
	mLightUBO.BindBufferRndIdx(1, buf_size, 0);

	mDirLight.direction = glm::vec3(-1.0f, 1.0f, -0.2f);


	//Scene object transformations
	glm::mat4 mat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f)) *
		glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) *
		glm::scale(glm::mat4(1.0f), glm::vec3(50.0f));
	m_PlaneTransform.push_back(mat);

	//Spheres
	mat = glm::translate(glm::mat4(1.0f), glm::vec3(7.0f, 1.0f, 0.0f));
	int count_per_axis = 2;
	glm::vec2 offset(1.5f);
	for (int i = 0; i < count_per_axis; i++)
	{
		float x = (float)i * offset.x;
		for (int j = 0; j < count_per_axis; j++)
		{
			m_SphereTransform.push_back(glm::translate(mat, glm::vec3(x, 0.0f, (float)j * offset.y)));
		}
	}

	//Cubes
	mat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	for (int i = 0; i < count_per_axis; i++)
	{
		float x = (float)i * offset.x;
		for (int j = 0; j < count_per_axis; j++)
		{
			m_CubeTransform.push_back(glm::translate(mat, glm::vec3(x, 0.0f, (float)j * offset.y)));
		}
	}

	//Cones
	mat = glm::translate(glm::mat4(1.0f), glm::vec3(-10.0f, 1.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	offset = glm::vec2(2.0f);
	for (int i = 0; i < count_per_axis; i++)
	{
		float x = (float)i * offset.x;
		for (int j = 0; j < count_per_axis; j++)
		{
			m_ConeTransform.push_back(glm::translate(mat, glm::vec3(x, (float)j * -offset.y, 0.0f)));
		}
	}


	mShadowShader.Create("shadow_depth", "assets/shaders/shadowMapping/ShadowDepth.vert", "assets/shaders/shadowMapping/ShadowDepth.frag");
	mShadowMapFBO.Generate(4096);


	//Preview Window
	mMaterialPreviewFBO.Generate(512, 512);
	mPtrPreviewMaterial = mSphereMaterial;
	mMaterialPreviewCam.SetPosition(glm::vec3(0.0f, 0.0f, -1.9f));
	//mPreviewTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 1.9f));
	mPreviewShader.Create("preview_shader", "assets/shaders/lighting/PreviewMaterial.vert", "assets/shaders/lighting/PreviewMaterial.frag");


	std::uniform_real_distribution<float> random_floats(0.0f, 1.0f);
	std::default_random_engine generator;
	//Noise generation per pixel data 
	std::vector<glm::vec3> noise_vector;
	for (uint16_t i = 0; i < 16; i++)
	{
		float x = Util::Random::Float(0.0f, 1.0f) * 2.0f - 1.0f;
		float y = Util::Random::Float(0.0f, 1.0f) * 2.0f - 1.0f;
		float z = Util::Random::Float(0.0f, 1.0f);

		x = random_floats(generator) * 2.0f - 1.0f;
		y = random_floats(generator) * 2.0f - 1.0f;

		glm::vec3 noise(x, y, 0.0f);
		noise_vector.push_back(noise);
	}

	GPUResource::TextureParameter tex{
		GPUResource::IMGFormat::RGBA,
		//GPUResource::IMGFormat::RGBA16F,
		GPUResource::TextureType::UTILITIES,
		GPUResource::TexWrapMode::REPEAT,
		GPUResource::TexFilterMode::NEAREST,
		GPUResource::DataType::FLOAT,
		false, GPUResource::IMGFormat::RGB,
	};
	DEBUG_LOG("GPU resources texture paramerter struct size: ", sizeof(GPUResource::TextureParameter));
	DEBUG_LOG("GPU resources image format enum size: ", sizeof(GPUResource::IMGFormat));
	DEBUG_LOG("GPU resources texture type enum size: ", sizeof(GPUResource::TextureType));
	DEBUG_LOG("GPU resources texture wrap mode enum size: ", sizeof(GPUResource::TexWrapMode));
	DEBUG_LOG("GPU resources data type enum size: ", sizeof(GPUResource::DataType));

	mNoiseTex = std::make_shared<GPUResource::Texture>(4, 4, &noise_vector[0], tex);

	//Util::Random::SetFloatDistRange(0.0f, 1.0f);
	//Util::Random::SetFloatDistRange(-1.0f, 1.0f);
	//overriden whole
	for (uint16_t i = 0; i < 16; i++)
	{
		//float x = Util::Random::Float(0.0f, 1.0f) * 2.0f - 1.0f;
		//float y = Util::Random::Float(0.0f, 1.0f) * 2.0f - 1.0f;
		//float z = Util::Random::Float(0.0f, 1.0f);

		float x = Util::Random::GetFloatRndDist() * 2.0f - 1.0f;
		float y = Util::Random::GetFloatRndDist() * 2.0f - 1.0f;

		glm::vec3 noise(x, y, 0.0f);
		noise_vector[i] = noise;
	}

	mNoiseTex2= std::make_shared<GPUResource::Texture>(4, 4, &noise_vector[0], tex);
}

void SampleLightingProgram::UpdateCameraUBO(EditorCamera& cam, float aspect_ratio)
{
	//SCOPE_MEM_ALLOC_PROFILE("UpdateCameraUBO");
	//------------------Camera Matrix Data UBO-----------------------------/
	unsigned int offset_ptr = 0;
	mCameraUBO.SetSubDataByID(&(cam.GetPosition()[0]), sizeof(glm::vec3), offset_ptr);
	offset_ptr += sizeof(glm::vec3);
	mCameraUBO.SetSubDataByID(&cam.mFar, sizeof(float), offset_ptr);
	offset_ptr += sizeof(float);
	mCameraUBO.SetSubDataByID(&(cam.ProjMat(aspect_ratio)[0][0]), sizeof(glm::mat4), offset_ptr);
	offset_ptr += sizeof(glm::mat4);
	mCameraUBO.SetSubDataByID(&(cam.ViewMat()[0][0]), sizeof(glm::mat4), offset_ptr);
	DEBUG_LOG("Int size: ", sizeof(int), " bytes.");
	DEBUG_LOG("uInt8 size: ", sizeof(uint8_t), " bytes.");
	DEBUG_LOG("uInt16 size: ", sizeof(uint16_t), " bytes.");
	float offset = 5.0f;
	glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 1.0f, 0.0f) + (mDirLight.direction * offset), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)); //world up 0, 1, 0
	glm::mat4 proj = glm::ortho(-30.f, 30.f, -30.f, 30.f, 0.1f, 100.0f);
	mLightProjViewMat = proj * view;



	mShader.Bind();
	mShader.SetUniformMat4("uLightSpaceMat", mLightProjViewMat);
	//------------------Lighting Data UBO-----------------------------/
}

void SampleLightingProgram::UpdateShaders()
{
	mShader.Bind();

	//mShader.SetUniform1i("u_UseSkybox", 0);


	//------Render type Blinn-Phong/Phong---------
	mShader.SetUniform1i("uPhongRendering", 0);

	//---------directional Light-------------------
	//vec3s
	//direction 
	mShader.SetUniformVec3("uDirectionalLight.direction", mDirLight.direction);
	//diffuse
	mShader.SetUniformVec3("uDirectionalLight.diffuse", mDirLight.base.diffuse);
	//ambient
	mShader.SetUniformVec3("uDirectionalLight.ambient", mDirLight.base.ambient);
	//specular
	mShader.SetUniformVec3("uDirectionalLight.specular", mDirLight.base.specular);
	mShader.SetUniform1i("uDirectionalLight.enable", mDirLight.base.enable);
}

void SampleLightingProgram::BeginFrame()
{
	glClearColor(mClearColour.r, mClearColour.g, mClearColour.b, mClearColour.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	UpdateCameraUBO(*mCamera, mDisplayManager->GetAspectRatio()); 

	UpdateShaders();
}

void SampleLightingProgram::ShadowPass()
{
	if (!mDisplayManager)
		return;
	mShadowMapFBO.Write();
	glCullFace(GL_FRONT);
	mShadowShader.Bind();
	mShadowShader.SetUniformMat4("uLightSpaceMat", mLightProjViewMat);

	glClear(GL_DEPTH_BUFFER_BIT);
	//Draw avaliable mesh geometry

	//sphere
	if (mSphereMaterial)
	{
		//Draw sphere
		for (const auto& model : m_SphereTransform)
		{
			mShadowShader.SetUniformMat4("uModel", model);
			mSphereMesh->Draw();
		}
	}
	//cube
	if (mCubeMaterial)
	{
		//Draw sphere
		for (const auto& model : m_CubeTransform)
		{
			mShadowShader.SetUniformMat4("uModel", model);
			mCubeMesh->Draw();
		}
	}
	if (mConeMesh)
	{
		//Draw sphere
		for (const auto& model : m_ConeTransform)
		{
			mShadowShader.SetUniformMat4("uModel", model);
			mConeMesh->Draw();
		}
	}

	//swicth back to default frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glCullFace(GL_BACK);
	glViewport(0, 0, mDisplayManager->GetWidth(), mDisplayManager->GetHeight());
}

void SampleLightingProgram::MaterialShaderHelper(Shader& shader, const BaseMaterial& mat)
{
	//Material (u_Material)
//diffuse; vec3s
	shader.SetUniformVec3("uMaterial.diffuse", mat.diffuse);
	//ambient;
	shader.SetUniformVec3("uMaterial.ambient", mat.ambient);
	//specular;
	shader.SetUniformVec3("uMaterial.specular", mat.specular);
	//shinness; float
	shader.SetUniform1f("uMaterial.shinness", mat.shinness);
}

void SampleLightingProgram::MainSceneUIEditor()
{
	//ImGui::ShowDemoWindow();
	if (ImGui::Begin("Sample Gfx Program"))
	{
		ImGui::SeparatorText("Window info");
		//ImGui::Text("Win Title: %s", mDisplayManager.);
		ImGui::Text("Window Width: %d", mDisplayManager->GetWidth());
		ImGui::Text("Window Height: %d", mDisplayManager->GetHeight());

		//CAMERA
		if (mCamera)
		{
			ImGui::SeparatorText("Camera info");
			auto cam_pos = mCamera->GetPosition();
			ImGui::Text("Position x: %f, y: %f, z: %f", cam_pos.x, cam_pos.y, cam_pos.z);

			ImGui::Text("Pitch: %f", mCamera->GetPitch());
			ImGui::Text("Yaw: %f", mCamera->GetYaw());


			if (ImGui::TreeNode("Camera Properties"))
			{
				ImGui::SliderFloat("Move Speed", &mCamera->mMoveSpeed, 5.0f, 250.0f);
				ImGui::SliderFloat("Rot Speed", &mCamera->mRotSpeed, 0.0f, 10.0f, "%.1f");

				ImGui::SliderFloat("FOV", &mCamera->mFOV, 0.0f, 179.0f, "%.1f");
				ImGui::DragFloat("Near", &mCamera->mNear, 0.1f, 0.1f, 50.0f, "%.1f");
				ImGui::DragFloat("Far", &mCamera->mFar, 0.1f, 0.0f, 500.0f, "%.1f");

				ImGui::TreePop();
			}
		}


		//////////////////////////////////////
		// Directional Light
		//////////////////////////////////////
		ImGui::Spacing();
		ImGui::SeparatorText("Directional Light");
		ImGui::Checkbox("Enable", &mDirLight.base.enable);
		ImGui::DragFloat3("Light Direction", &mDirLight.direction[0], 0.1f, -1.0f, 1.0f);
		ImGui::ColorEdit3("Diffuse colour", &mDirLight.base.diffuse[0]);
		ImGui::ColorEdit3("Ambient colour", &mDirLight.base.ambient[0]);
		ImGui::ColorEdit3("Specular colour", &mDirLight.base.specular[0]);

		ImGui::End();
	}
}
//
//void SampleLightingProgram::MaterialEditorUIWindow()
//{
//	static bool update_preview_fbo = true;
//	static int mesh_id = 0;
//	static Lighting::Directional preview_lighting = Lighting::Directional(glm::vec3(-1.0f, 1.0f, -1.0f), glm::vec3(1.0f), glm::vec3(1.0), glm::vec3(1.0f));
//
//
//	if (update_preview_fbo)
//	{
//		mMaterialPreviewFBO.Bind();
//		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
//		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//		//Update camera 
//		float aspect_ratio = mMaterialPreviewFBO.GetHeight() / mMaterialPreviewFBO.GetWidth();
//		UpdateCameraUBO(mMaterialPreviewCam, aspect_ratio);
//		//tranform matrix later
//		mPreviewShader.Bind();
//		//with the hope that other parameter are updated like 
//		//the lighting in  
//		mPreviewShader.SetUniformMat4("uModel", mPreviewTransform);
//		//m_Shader.SetUniform1i("u_OnlyBaseColour", !b_MaterialPreviewLighting);
//		mPreviewShader.SetUniform1i("uPhongRendering", !mPtrPreviewMaterial.lock()->blinn_phong);
//		//---------directional Light-------------------
//		mPreviewShader.SetUniformVec3("uDirectionalLight.direction", preview_lighting.direction);
//		//diffuse
//		mPreviewShader.SetUniformVec3("uDirectionalLight.diffuse", preview_lighting.base.diffuse);
//		//ambient
//		mPreviewShader.SetUniformVec3("uDirectionalLight.ambient", preview_lighting.base.ambient);
//		//specular
//		mPreviewShader.SetUniformVec3("uDirectionalLight.specular", preview_lighting.base.specular);
//		mPreviewShader.SetUniform1i("uDirectionalLight.enable", preview_lighting.base.enable);
//		mPreviewShader.SetUniform1f("uAmbientRatio", mPreviewAmbientRatio);
//		if (mPtrPreviewMaterial.lock()->diffuseMap)
//			mPtrPreviewMaterial.lock()->diffuseMap->Activate();
//		else
//			mErrorTex->Activate();
//		mPreviewShader.SetUniform1i("uDirectionalLight.enable", mDirLight.base.enable);
//		MaterialShaderHelper(mPreviewShader, *mPtrPreviewMaterial.lock());
//
//		auto preview_mesh = mSphereMesh;
//		if (mesh_id == 1)
//			preview_mesh = mQuadMesh;
//		else if (mesh_id == 2)
//			preview_mesh = mCubeMesh;
//
//		preview_mesh->Draw();
//
//		mMaterialPreviewFBO.UnBind();
//		update_preview_fbo = false;
//		glViewport(0, 0, mDisplayManager->GetWidth(), mDisplayManager->GetHeight());
//	}
//
//	if (ImGui::Begin("Materials Editor"))
//	{
//		ImVec2 ui_win_size = ImGui::GetWindowSize();
//		ImVec2 preview_panel_size = ImVec2(ui_win_size.x * 0.5f, ui_win_size.x * 0.5f);
//		preview_panel_size.y *= (mMaterialPreviewFBO.GetHeight() / mMaterialPreviewFBO.GetWidth()); //invert
//
//		ImVec2 top_left = ImGui::GetCursorPos();
//		ImGui::Image((ImTextureID)(intptr_t)mMaterialPreviewFBO.GetRenderTextureGPU_ID(),
//					preview_panel_size, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
//		update_preview_fbo |= Ext_QuatEditorPanel(mPreviewTransform, top_left, preview_panel_size);
//
//
//		ImGui::SameLine();
//		ImGui::BeginChild("##mesh_type", ImVec2(preview_panel_size.x * 0.75f, preview_panel_size.y * 0.5f));
//		const char* mesh_type[] = { "Sphere", "Quad", "Cube" };
//		update_preview_fbo |= ImGui::Combo("Preview Mesh", &mesh_id, mesh_type, 3);
//		ImGui::EndChild();
//
//		//material properties
//		auto& mat = mPtrPreviewMaterial.lock();
//		ImGui::Text(mat->name);
//		//update_preview_fbo |= ImGui::Checkbox("Preview Lighting or Base colour", &b_MaterialPreviewLighting);
//		update_preview_fbo |= ImGui::Checkbox("Blinn-Phong", &mat->blinn_phong);
//		update_preview_fbo |= ImGui::ColorEdit3("Diffuse", &mat->diffuse[0]);
//		update_preview_fbo |= ImGui::ColorEdit3("Ambient", &mat->ambient[0]);
//		update_preview_fbo |= ImGui::ColorEdit3("Specular", &mat->specular[0]);
//		update_preview_fbo |= ImGui::SliderFloat("Shinness", &mat->shinness, 8.0f, 256.0f, "%.0f");
//		update_preview_fbo |= ImGui::SliderFloat("Ambinent Ratio", &mPreviewAmbientRatio, 0.0f, 1.0f, "%.1f");
//		//diffuse map
//		int tex_id = (mat->diffuseMap) ? mat->diffuseMap->GetID() : mErrorTex->GetID();
//		ImGui::Image((ImTextureID)(intptr_t)tex_id, ImVec2(100, 100));
//		ImGui::SameLine(); ImGui::Text("Diffuse Map");
//		//normal map
//		tex_id = (mat->normalMap) ? mat->normalMap->GetID() : mErrorTex->GetID();
//		ImGui::Image((ImTextureID)(intptr_t)tex_id, ImVec2(100, 100));
//		ImGui::SameLine(); ImGui::Text("Normal Map");
//		//specular map
//		tex_id = (mat->specularMap) ? mat->specularMap->GetID() : mErrorTex->GetID();
//		ImGui::Image((ImTextureID)(intptr_t)tex_id, ImVec2(100, 100));
//		ImGui::SameLine(); ImGui::Text("Specular Map");
//
//		//list all materials
//		if (ImGui::Button("Sphere Material"))
//		{
//			mPtrPreviewMaterial = mSphereMaterial;
//			update_preview_fbo |= true;
//		}
//		if (ImGui::Button("Quad Material"))
//		{
//			mPtrPreviewMaterial = mQuadMaterial;
//			update_preview_fbo |= true;
//		}
//		if (ImGui::Button("Cube Material"))
//		{
//			mPtrPreviewMaterial = mCubeMaterial;
//			update_preview_fbo |= true;
//		}
//
//
//		if (ImGui::TreeNode("Preview DirectionalLight"))
//		{
//			update_preview_fbo |= ImGui::Checkbox("Enable", &preview_lighting.base.enable);
//			update_preview_fbo |= ImGui::DragFloat3("Light Direction", &preview_lighting.direction[0], 0.1f, -1.0f, 1.0f);
//			update_preview_fbo |= ImGui::ColorEdit3("Diffuse colour", &preview_lighting.base.diffuse[0]);
//			update_preview_fbo |= ImGui::ColorEdit3("Ambient colour", &preview_lighting.base.ambient[0]);
//			update_preview_fbo |= ImGui::ColorEdit3("Specular colour", &preview_lighting.base.specular[0]);
//			ImGui::TreePop();
//		}
//
//		if (ImGui::TreeNode("Preview Camera"))
//		{
//			static bool update_cam = false;
//
//			update_cam |= ImGui::SliderFloat("FOV", &mMaterialPreviewCam.mFOV, 0.0f, 179.0f, "%.1f");
//			update_cam |= ImGui::DragFloat("Near", &mMaterialPreviewCam.mNear, 0.1f, 0.1f, 50.0f, "%.1f");
//			update_cam |= ImGui::DragFloat("Far", &mMaterialPreviewCam.mFar, 0.1f, 0.0f, 500.0f, "%.1f");
//
//			glm::vec3 pos = mMaterialPreviewCam.GetPosition();
//			if (update_cam |= ImGui::DragFloat3("Position", &pos[0], 0.1f))
//				mMaterialPreviewCam.SetPosition(pos);
//
//			update_cam |= ImGui::SliderFloat("Yaw", &mMaterialPreviewCam.mYaw, 0.0f, 360.0f);
//			update_cam |= ImGui::SliderFloat("Pitch", &mMaterialPreviewCam.mPitch, 0.0f, 360.0f);
//			
//			//quick hack
//			if (update_cam)
//				mMaterialPreviewCam.SetPosition(pos);
//
//			update_preview_fbo |= update_cam;
//
//			ImGui::TreePop();
//		}
//
//		ImGui::End();
//	}
//}
//
//bool SampleLightingProgram::Ext_QuatEditorPanel(glm::mat4& transform, ImVec2 top_left, ImVec2 size)
//{
//	glm::quat quat = glm::quat_cast(transform);
//
//
//	ImVec2 ui_win_size = ImGui::GetWindowSize();
//	//ImVec2 drag_panel_size = ImVec2(200, 200);
//	ImVec2 drag_panel_size = ImVec2(ui_win_size.x * 0.5f, ui_win_size.x * 0.5f);
//	static float drag_senstitivity = 0.001f;
//	static int drag_speed = 5;
//	static bool invert_x = false;
//	static bool invert_y = false;
//
//
//	//need location
//	ImGui::SetCursorPos(top_left);
//	ImGui::InvisibleButton("##quat_editor", size);
//
//	ImDrawList* draw_list = ImGui::GetWindowDrawList();
//	ImVec2 min = ImGui::GetItemRectMin();
//	ImVec2 max = ImGui::GetItemRectMax();
//	draw_list->AddRect(min, max, IM_COL32(255, 255, 255, 255));
//
//
//	//mouse drag
//	if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0))
//	{
//		draw_list->AddRect(min, max, IM_COL32(0, 0, 255, 255));
//		ImVec2 dt_drag = ImGui::GetMouseDragDelta();
//
//		//dt drag -> rot angle
//		float dt_x = dt_drag.x * drag_senstitivity * drag_speed;// *(invert_x) ? -1.0f : 1.0f;
//		float dt_y = dt_drag.y * drag_senstitivity * drag_speed;
//
//		dt_x *= (invert_x) ? -1.0f : 1.0f;
//		dt_y *= (invert_y) ? 1.0f : -1.0f;
//
//		glm::quat rot_x = glm::angleAxis(dt_y, glm::vec3(1.0f, 0.0f, 0.0f));
//		glm::quat rot_y = glm::angleAxis(dt_x, glm::vec3(0.0f, 1.0f, 0.0f));
//
//		quat = glm::normalize(rot_y * rot_x * quat);
//
//		transform = glm::mat4_cast(quat);
//
//		ImGui::ResetMouseDragDelta();
//		return true;
//	}
//
//	return false;
//}
