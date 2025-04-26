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

#include "Core/HeapMemAllocationTracking.h"
#include "Core/Profiler.h"


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

	//to default framebuffer
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	DrawScene(mShader);
	glDisable(GL_BLEND);


	//Draw to normal render target frame buffer
	mRenderTarget.Bind();
	glClearColor(mClearColour.r, mClearColour.g, mClearColour.b, mClearColour.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	DrawScene(mShader);
	glDisable(GL_BLEND);
	mRenderTarget.UnBind();

	//Draw to multirender target frame buffer
	mMultiRenderTarget.Bind();
	glClearColor(mClearColour.r, mClearColour.g, mClearColour.b, mClearColour.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	DrawScene(mTestMultiRenderShader);
	mMultiRenderTarget.UnBind();


	//Draw a sample line connecting spheres
	for (int i = 1; i < mSphereTransform.size(); i++)
	{
		DebugGizmosRenderer::Instance().DrawLine(mSphereTransform[i - 1][3], mSphereTransform[i][3], glm::vec3(0.0f, 0.0f, 1.0f));
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





	////Test render target
	//mRenderTarget.Bind();
	//glClearColor(mClearColour.r, mClearColour.g, mClearColour.b, mClearColour.a);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//mShader.Bind();
	//mShadowMapFBO.Read(0);
	//mShader.SetUniform1i("uShadowMap", 0);
	//glEnable(GL_BLEND);
	//glEnable(GL_DEPTH_TEST);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	////plane 
	//if (mQuadMaterial)
	//{

	//	MaterialShaderHelper(mShader, *mQuadMaterial);
	//	//Draw planes
	//	for (const auto& model : m_PlaneTransform)
	//	{
	//		mShader.SetUniformMat4("uModel", model);
	//		mQuadMesh->Draw();
	//	}
	//}



	//////sphere
	////if (mSphereMaterial)
	////{
	////	MaterialShaderHelper(mShader, *mSphereMaterial);
	////	//Draw sphere
	////	for (const auto& model : mSphereTransform)
	////	{
	////		mShader.SetUniformMat4("uModel", model);
	////		mSphereMesh->Draw();
	////	}
	////}



	////cube
	//if (mCubeMaterial)
	//{
	//	MaterialShaderHelper(mShader, *mCubeMaterial);
	//	//Draw sphere
	//	for (const auto& model : m_CubeTransform)
	//	{
	//		mShader.SetUniformMat4("uModel", model);
	//		mCubeMesh->Draw();
	//	}
	//}


	//if (mConeMesh)
	//{
	//	if (!mCubeMaterial)
	//		MaterialShaderHelper(mShader, *mCubeMaterial);
	//	//Draw sphere
	//	for (const auto& model : m_ConeTransform)
	//	{
	//		mShader.SetUniformMat4("uModel", model);
	//		mConeMesh->Draw();
	//	}
	//}


	////Draw sphere
	//MaterialShaderHelper(mShader, *mSphereMaterial);
	//for (const auto& model : mSphereTransform)
	//{
	//	mShader.SetUniformMat4("uModel", model);
	//	mSphereMesh->Draw();
	//}



	////sample origin 
	//sample_origin_tran = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 4.0f, 0.0f));
	////draw plane quad
	//MaterialShaderHelper(mShader, *mCubeMaterial);
	//mShader.SetUniformMat4("uModel", glm::rotate(sample_origin_tran, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(2.0f)));
	//mQuadMesh->Draw();
	//mShader.SetUniformMat4("uModel", glm::rotate(sample_origin_tran, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(2.0f)));
	//mQuadMesh->DrawOutline();
	//mShader.SetUniformMat4("uModel", glm::rotate(sample_origin_tran, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(2.0f)));
	//mQuadMesh->DrawOutline();
	////sample points
	//for (const auto& s : mSamplingKernelPoints)
	//{
	//	mShader.SetUniformMat4("uModel", glm::translate(sample_origin_tran, s) * glm::scale(glm::mat4(1.0f), glm::vec3(0.2f)));
	//	mSphereMesh->Draw();
	//}
	////sphere
	//MaterialShaderHelper(mShader, *mSphereMaterial);
	//mShader.SetUniformMat4("uModel", glm::scale(sample_origin_tran, glm::vec3(2.0f)));
	//mSphereMesh->Draw();

	////Screen texture 
	//mTexScrceenShader.Bind();
	//mNoiseTex->Activate(0);
	//mQuadMesh->Draw();
	//mNoiseTex->Disactivate();

	//mRenderTarget.UnBind();
	//glDisable(GL_BLEND);

}

void SampleLightingProgram::OnLateUpdate(float delta_time)
{
	mShaderHotReloadTracker.Update();
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

	//UI::Windows::SingleTextureEditor(*mQuadMaterial->diffuseMap);
	HELPER_SINGLE_TEXTURE_EDITOR_UIFLAG((*mQuadMaterial->diffuseMap), dm_open_flag, false);

	DirectionLightUIEditor();

	/*static bool test = */ HELPER_REGISTER_UIFLAG("Lets Go", p_open_flag, false);

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
	{
		HELPER_SINGLE_TEXTURE_EDITOR_UIFLAG((*mNoiseTex), nt_open_flag, false);
		//UI::Windows::SingleTextureEditor(*mNoiseTex);
	}

	if (mNoiseTex2)
	{
		HELPER_SINGLE_TEXTURE_EDITOR_UIFLAG((*mNoiseTex2), nt_open_flag, false);
		//UI::Windows::SingleTextureEditor(*mNoiseTex2);
	}



	UI::Windows::RenderTargetViewport(mRenderTarget);
	UI::Windows::MultiRenderTargetViewport(mMultiRenderTarget);
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
	
	mShaderHotReloadTracker.AddShader(&mShader);

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
		glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) *
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
			mSphereTransform.push_back(glm::translate(mat, glm::vec3(x, 0.0f, (float)j * offset.y)));
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

	mTexScrceenShader.Create("tex screen shader", "assets/shaders/TextureToScreen.vert", "assets/shaders/TextureToScreen.frag");
	mShaderHotReloadTracker.AddShader(&mTexScrceenShader);

	//Noise generation per pixel data 
	std::vector<glm::vec3> noise_vector;
	auto randomf = Util::Random::RNG<float>(Util::Random::rnd_b4mt, -1.0f, 1.0f);
	for (uint16_t i = 0; i < 16; i++)
	{
		glm::vec3 noise(randomf(), randomf(), 0.0f);
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


	//black & white texure 
	//requirement 
	// single channel red 
	// utility tyoe
	// float data type
	//
	tex.format = GPUResource::IMGFormat::RED;
	randomf.SetDistribution(0.0f, 1.0f);
	//16x16
	//1920 * 1080
	//noise_vector.clear();
	//noise_vector.resize(256);
	noise_vector.reserve(300);
	DEBUG_LOG("Vector Size", sizeof(noise_vector), ", count: ", noise_vector.size());

	const int noise_width = 100;
	const int noise_height = 101;
	const int channel_count = 1;
	const int noise_pixels = noise_width * noise_height * channel_count;
	float* noise_buf = new float[noise_pixels];
	for (uint16_t i = 0; i+1 < noise_pixels; i += 2)
	{
		noise_buf[i] = randomf();
		noise_buf[i+1] = randomf();
	}
	if (noise_pixels % 2 != 0)
		noise_buf[noise_pixels - 1] = randomf();

	mNoiseTex2= std::make_shared<GPUResource::Texture>(noise_width, noise_height, noise_buf, tex);


	randomf.SetDistribution(0.0f, 1.0f);
	mSamplingKernelPoints.reserve(64);
	for (size_t i = 0; i < 64; i++)
	{
		glm::vec3 v = glm::vec3(
						randomf() * 2.0f - 1.0f,
						randomf(), // * 2.0f - 1.0f,
						randomf() * 2.0f - 1.0f);


		//normalise to anchor to the hemisphere surface
		v = glm::normalize(v);
		//v *= randomf();

		float scale = static_cast<float>(i) / 64.0f;
		scale = glm::mix(0.1f, 1.0f, scale * scale);
		v *= scale;

		DEBUG_LOG("Vector Sample: ", v);

		mSamplingKernelPoints.emplace_back(v);
	}


	PGL_ASSERT_CRITICAL(mDisplayManager, "No Display Window ro retrive screen dimension from");
	mRenderTarget.Generate(mDisplayManager->GetWidth(), mDisplayManager->GetHeight());
	//Register Resize
	REGISTER_RESIZE_CALLBACK_HELPER((*mDisplayManager), &GPUResource::Framebuffer::ResizeBuffer2, &mRenderTarget);
	DEBUG_LOG_INFO("Framebuffer/renderTarget size: ", sizeof(GPUResource::Texture));

	GPUResource::TextureParameter render_target_para[3] = 
	{
		{GPUResource::IMGFormat::RGBA16F, GPUResource::TextureType::RENDER, GPUResource::TexWrapMode::CLAMP, GPUResource::TexFilterMode::LINEAR, GPUResource::DataType::FLOAT, false, GPUResource::IMGFormat::RGBA},
		{GPUResource::IMGFormat::RGB16F, GPUResource::TextureType::RENDER, GPUResource::TexWrapMode::CLAMP, GPUResource::TexFilterMode::LINEAR, GPUResource::DataType::FLOAT, false, GPUResource::IMGFormat::RGB},
		{GPUResource::IMGFormat::RGB16F, GPUResource::TextureType::RENDER, GPUResource::TexWrapMode::CLAMP, GPUResource::TexFilterMode::LINEAR, GPUResource::DataType::FLOAT, false, GPUResource::IMGFormat::RGB},
	};
	mMultiRenderTarget.Generate(mDisplayManager->GetWidth(), mDisplayManager->GetHeight(), 3, render_target_para);
	mTestMultiRenderShader.Create("lighting_multi_render_shader", "assets/shaders/lighting/ModelLighting.vert", "assets/shaders/lighting/TestMultiRender.frag");
	mShaderHotReloadTracker.AddShader(&mTestMultiRenderShader);
	REGISTER_RESIZE_CALLBACK_HELPER((*mDisplayManager), &GPUResource::MultiRenderTarget::ResizeBuffer, &mMultiRenderTarget);
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


	//------------------Lighting Data UBO-----------------------------/
}

void SampleLightingProgram::UpdateShaders(Shader& shader)
{
	shader.Bind();

	//mShader.SetUniform1i("u_UseSkybox", 0);

	float offset = 5.0f;
	glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 1.0f, 0.0f) + (mDirLight.direction * offset), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)); //world up 0, 1, 0
	glm::mat4 proj = glm::ortho(-30.f, 30.f, -30.f, 30.f, 0.1f, 100.0f);
	mLightProjViewMat = proj * view;


	shader.SetUniformMat4("uLightSpaceMat", mLightProjViewMat);
	//------Render type Blinn-Phong/Phong---------
	shader.SetUniform1i("uPhongRendering", 0);

	//---------directional Light-------------------
	//vec3s
	//direction 
	shader.SetUniformVec3("uDirectionalLight.direction", mDirLight.direction);
	//diffuse
	shader.SetUniformVec3("uDirectionalLight.diffuse", mDirLight.base.diffuse);
	//ambient
	shader.SetUniformVec3("uDirectionalLight.ambient", mDirLight.base.ambient);
	//specular
	shader.SetUniformVec3("uDirectionalLight.specular", mDirLight.base.specular);
	shader.SetUniform1i("uDirectionalLight.enable", mDirLight.base.enable);
}

void SampleLightingProgram::BeginFrame()
{
	glClearColor(mClearColour.r, mClearColour.g, mClearColour.b, mClearColour.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	UpdateCameraUBO(*mCamera, mDisplayManager->GetAspectRatio()); 

	UpdateShaders(mShader);
	UpdateShaders(mTestMultiRenderShader);
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
		for (const auto& model : mSphereTransform)
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

void SampleLightingProgram::DrawScene(Shader& shader)
{
	shader.Bind();
	mShadowMapFBO.Read(0);
	shader.SetUniform1i("uShadowMap", 0);
	//glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//plane 
	if (mQuadMaterial)
	{

		MaterialShaderHelper(shader, *mQuadMaterial);
		//Draw planes
		for (const auto& model : m_PlaneTransform)
		{
			shader.SetUniformMat4("uModel", model);
			mQuadMesh->Draw();
		}
	}



	////sphere
	//if (mSphereMaterial)
	//{
	//	MaterialShaderHelper(shader, *mSphereMaterial);
	//	//Draw sphere
	//	for (const auto& model : mSphereTransform)
	//	{
	//		shader.SetUniformMat4("uModel", model);
	//		mSphereMesh->Draw();
	//	}
	//}



	//cube
	if (mCubeMaterial)
	{
		MaterialShaderHelper(shader, *mCubeMaterial);
		//Draw sphere
		for (const auto& model : m_CubeTransform)
		{
			shader.SetUniformMat4("uModel", model);
			mCubeMesh->Draw();
		}
	}


	if (mConeMesh)
	{
		if (!mCubeMaterial)
			MaterialShaderHelper(shader, *mCubeMaterial);
		//Draw sphere
		for (const auto& model : m_ConeTransform)
		{
			shader.SetUniformMat4("uModel", model);
			mConeMesh->Draw();
		}
	}


	//Draw sphere
	MaterialShaderHelper(shader, *mSphereMaterial);
	for (const auto& model : mSphereTransform)
	{
		shader.SetUniformMat4("uModel", model);
		mSphereMesh->Draw();
	}



	//sample origin 
	glm::mat4 sample_origin_tran = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 4.0f, 0.0f));
	//draw plane quad
	MaterialShaderHelper(shader, *mCubeMaterial);
	shader.SetUniformMat4("uModel", glm::rotate(sample_origin_tran, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(2.0f)));
	mQuadMesh->Draw();
	shader.SetUniformMat4("uModel", glm::rotate(sample_origin_tran, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(2.0f)));
	mQuadMesh->DrawOutline();
	shader.SetUniformMat4("uModel", glm::rotate(sample_origin_tran, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(2.0f)));
	mQuadMesh->DrawOutline();
	//sample points
	for (const auto& s : mSamplingKernelPoints)
	{
		shader.SetUniformMat4("uModel", glm::translate(sample_origin_tran, s) * glm::scale(glm::mat4(1.0f), glm::vec3(0.2f)));
		mSphereMesh->Draw();
	}
	//sphere
	MaterialShaderHelper(shader, *mSphereMaterial);
	shader.SetUniformMat4("uModel", glm::scale(sample_origin_tran, glm::vec3(2.0f)));
	mSphereMesh->Draw();

	//Screen texture 
	//mTexScrceenShader.Bind();
	//mNoiseTex->Activate(0);
	//mQuadMesh->Draw();
	//mNoiseTex->Disactivate();

	//quick visualise noise as sphere
	//if (mSphereMesh)
	//{
	//	glm::mat4 world_trans = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 5.0f, 0.0f)) *
	//							glm::scale(glm::mat4(1.0f), glm::vec3(0.2f));

	//	//define center
	//	if (mCubeMesh)
	//	{
	//		shader.SetUniformMat4("uModel", world_trans);
	//		mCubeMesh->DrawOutline();

	//		//borders
	//		//top left
	//		shader.SetUniformMat4("uModel", glm::translate(world_trans, 3.0f * glm::vec3(1.0f, 0.0f, 0.0f)));
	//		mCubeMesh->DrawOutline();
	//		//bottom left
	//		shader.SetUniformMat4("uModel", glm::translate(world_trans, 3.0f * glm::vec3(1.0f, -1.0f, 0.0f)));
	//		mCubeMesh->DrawOutline();
	//		//top right
	//		shader.SetUniformMat4("uModel", glm::translate(world_trans,3.0f * glm::vec3(-1.0f, 0.0f, 0.0f)));
	//		mCubeMesh->DrawOutline();
	//		//bottom right
	//		shader.SetUniformMat4("uModel", glm::translate(world_trans, 3.0f * glm::vec3(-1.0f, -1.0f, 0.0f)));
	//		mCubeMesh->DrawOutline();
	//		//mid top 
	//		shader.SetUniformMat4("uModel", glm::translate(world_trans, 3.0f * glm::vec3(0.0f, 1.0f, 0.0f)));
	//		mCubeMesh->DrawOutline();
	//		//mid bottom 
	//		shader.SetUniformMat4("uModel", glm::translate(world_trans, 3.0f * glm::vec3(0.0f, -1.0f, 0.0f)));
	//		mCubeMesh->DrawOutline();
	//		shader.SetUniformMat4("uModel", glm::translate(world_trans, 3.0f * glm::vec3(1.0f, 1.0f, 0.0f)));
	//		mCubeMesh->DrawOutline();
	//		shader.SetUniformMat4("uModel", glm::translate(world_trans, 3.0f * glm::vec3(-1.0f, 1.0f, 0.0f)));
	//		mCubeMesh->DrawOutline();
	//	}
	//	MaterialShaderHelper(shader, *mSphereMaterial);
	//	for (const auto& n : mNoisePosData)
	//	{
	//		shader.SetUniformMat4("uModel", glm::translate(world_trans, n * 3.0f));
	//		mSphereMesh->Draw();
	//	}
	//}

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

	shader.SetUniform1f("uOpacity", mat.opacity);
}

void SampleLightingProgram::DirectionLightUIEditor()
{
	//ImGui::ShowDemoWindow();
	HELPER_REGISTER_UIFLAG("Direction Light UI", p_open, true);
	if (p_open)
	{
		if (ImGui::Begin("Direction Light UI", &p_open))
		{
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

		}
		ImGui::End();
	}
}
