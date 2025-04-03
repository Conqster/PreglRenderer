#include "SampleGfxProgram.h"

#include "Renderer/Texture.h"
#include "Core/EditorCamera.h"

#include <GLM/glm/gtc/matrix_transform.hpp>
#include "Core/Util.h"
#include "Loader/Loader.h"

#include "Core/InputSystem.h"

#include "Loader/ModelLoader.h"

#include "libs/imgui/imgui.h"


using namespace InputSystem;


void SampleGfxProgram::OnInitialise(DisplayManager* display_window)
{
	mDisplayManager = display_window;

	bool success = mMeshShader.Create("experiment", "assets/shaders/test.vert", "assets/shaders/test.frag");
	success &= mShadowShader.Create("shadow_depth", "assets/shaders/shadowDepth.vert", "assets/shaders/shadowDepth.frag");
	success &= mWorldGridShader.Create("world_grid", "assets/shaders/worldGrid/worldGrid.vert", "assets/shaders/worldGrid/worldGrid.frag", "assets/shaders/worldGrid/worldGrid.geo");

	if(!mCamera)
		mCamera = new EditorCamera(glm::vec3(0.41f, 5.29f, -10.61f), 90.0f, -0.3f, 17.5f, 4.0f);

	if (success)
		printf("Successfully create a shaders!!!!!!\n");
	else
	{
		printf("Failed create a shaders!!!!!!\n");
		exit(-1);
	}


	GLint max_vertices;
	glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES, &max_vertices);
	printf("maximum vertices: %d\n", int(max_vertices));

	//mBrickTexture = new Texture("assets/textures/floor_brick/patterned_brick_floor_diff.jpg", true);
	mBrickTexture = new Texture("assets/textures/floor_brick/patterned_brick_floor_diff.jpg", true);
	mCheckersTexture = new Texture("assets/textures/test_checkers.jpg", true);
	mPlainTexture = new Texture("assets/textures/plain64.png", true);



	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	mCubePrimitive = Loader::LoadMesh("assets/meshes/cube.rmesh");
	mQuadPrimitive = Loader::LoadMesh("assets/meshes/quad.rmesh");
	mSpherePrimitive = Util::CreateSphere();
	mTrianglePrimitive = Loader::LoadMesh("assets/meshes/triangle.rmesh");

	mShadowMapFBO.Generate();


	CreateEntities();
}

void SampleGfxProgram::OnUpdate(float delta_time)
{
	BeginFrame();
	ShadowPass();
	DrawObject(mMeshShader);
	InputCheck(delta_time);
}

void SampleGfxProgram::OnDestroy()
{
	mDisplayManager = nullptr;
	mMeshShader.Clear();

	if (mBrickTexture)
	{
		mBrickTexture->Clear();
		delete mBrickTexture;
	}
	//mBrickTexture = nullptr;

	if (mCheckersTexture)
	{
		mCheckersTexture->Clear();
		delete mCheckersTexture;
	}
	mCheckersTexture = nullptr;
}

void SampleGfxProgram::OnUI()
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


		//test model translatioon
		if (ptr_model_transform)
		{
			glm::vec3 prev_translate = (*ptr_model_transform)[3];
			//ImGui::DragFloat4("Translate Model2", ptr_model_transform[0][3], 0.1f);
			if (ImGui::DragFloat3("Translate Model", &prev_translate[0], 0.1f))
			{
				glm::vec3 new_translate = prev_translate - (glm::vec3)(*ptr_model_transform)[3];
				*ptr_model_transform = glm::translate(*ptr_model_transform, new_translate); 
			}
		}

		ImGui::End();
	}
	
}

void SampleGfxProgram::DrawObject(Shader& shader, bool only_depth)
{
	shader.Bind();
	if (mEntities.size() <= 0)
		return;

	if (!only_depth)
	{
		mShadowMapFBO.Read(1);
		shader.SetUniform1i("uShadowMap", 1);

		if (mCheckersTexture)
			mCheckersTexture->Activate();

		glEnable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	for (unsigned int i = 0; i < mEntities.size(); i++)
	{
		if (only_depth && !mEntities[i].canCastShadow)
			continue;

		RenderableEntity entt = mEntities[i];
		shader.SetUniformMat4("uModel", entt.transform);

		bool used_plain_texture = false;
		if (!only_depth)
		{
			if (entt.plainTexture && mPlainTexture)
			{
				mPlainTexture->Activate();
				used_plain_texture = true;
			}
			shader.SetUniformVec4("uColour", entt.colour);
		}


		if (entt.solidRender)
			entt.renderableMesh->Draw();
		else
			entt.renderableMesh->DrawOutline();

		//reset texture
		if (used_plain_texture && mCheckersTexture)
			mCheckersTexture->Activate();
	}

	if (!only_depth)
		glDisable(GL_BLEND);

}

void SampleGfxProgram::BeginFrame()
{
	if (!mDisplayManager)
		return;

	glClearColor(mClearColour.r, mClearColour.g, mClearColour.b, mClearColour.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//shadow properties 
	//calculate light project view matrix 
	glm::mat4 view = glm::lookAt(mLightWorldPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 proj = glm::ortho(-30.f, 30.f, -30.f, 30.f, 0.1f, 100.0f);
	mLightProjViewMat = proj * view;


	mMeshShader.Bind();
	//trs
	//test_transform = glm::translate(test_transform, delta_pos * 0.1f);
	mMeshShader.SetUniformMat4("uproj", mCamera->ProjMat(mDisplayManager->GetAspectRatio()));
	mMeshShader.SetUniformMat4("uview", mCamera->ViewMat());
	mMeshShader.SetUniformMat4("uLightSpaceMat", mLightProjViewMat);

	mMeshShader.SetUniformVec3("uviewPos", mCamera->GetPosition());
	mMeshShader.SetUniformVec3("ulight.direction", glm::normalize(mLightWorldPos)); // <- since world location is the origin (0,0,0)
	mMeshShader.SetUniformVec3("ulight.colour", glm::vec3(0.7f));


	//debug light location
	glm::mat4 debug_light_location = glm::translate(glm::mat4(1.0f), mLightWorldPos) *
		glm::scale(glm::mat4(1.0f), glm::vec3(0.2f));
	mMeshShader.SetUniformMat4("uModel", debug_light_location);
	mCubePrimitive.Draw();


	//debug sphere 
	mMeshShader.SetUniformMat4("uModel", glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 10.0f, 0.0f)) *
		glm::scale(glm::mat4(1.0f), glm::vec3(20.0f)));
}

void SampleGfxProgram::ShadowPass()
{
	if (!mDisplayManager)
		return;

	glCullFace(GL_FRONT);
	mShadowShader.Bind();
	mShadowShader.SetUniformMat4("uLightSpaceMat", mLightProjViewMat);

	mShadowMapFBO.Write();
	glClear(GL_DEPTH_BUFFER_BIT);
	//Draw avaliable mesh geometry
	DrawObject(mShadowShader, true);

	//swicth back to default frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glCullFace(GL_BACK);
	glViewport(0, 0, mDisplayManager->GetWidth(), mDisplayManager->GetHeight());
}

void SampleGfxProgram::InputCheck(float delta_time)
{
	//directional light location 
	float light_move_speed = 20.0f;
	auto light_pos = &mLightWorldPos;
	if (!light_pos)
		return;
	if (!Input::GetKey(IKeyCode::RightShift) && Input::GetKey(IKeyCode::LeftArrow))
		*light_pos += glm::vec3(1.0f, 0.0f, 0.0f) * delta_time * light_move_speed;
	if (!Input::GetKey(IKeyCode::RightShift) && Input::GetKey(IKeyCode::RightArrow))
		*light_pos += glm::vec3(-1.0f, 0.0f, 0.0f) * delta_time * light_move_speed;
	if (!Input::GetKey(IKeyCode::RightShift) && Input::GetKey(IKeyCode::P))
		*light_pos += glm::vec3(0.0f, 1.0f, 0.0f) * delta_time * light_move_speed;
	if (!Input::GetKey(IKeyCode::RightShift) && Input::GetKey(IKeyCode::O))
		*light_pos += glm::vec3(0.0f, -1.0f, 0.0f) * delta_time * light_move_speed;
	if (!Input::GetKey(IKeyCode::RightShift) && Input::GetKey(IKeyCode::DownArrow))
		*light_pos += glm::vec3(0.0f, 0.0f, -1.0f) * delta_time * light_move_speed;
	if (!Input::GetKey(IKeyCode::RightShift) && Input::GetKey(IKeyCode::UpArrow))
		*light_pos += glm::vec3(0.0f, 0.0f, 1.0f) * delta_time * light_move_speed;

	//mLightWorldPos = light_pos;
}

void SampleGfxProgram::CreateEntities()
{
	glm::mat4 plane_transform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)) *
		glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) *
		glm::scale(glm::mat4(1.0f), glm::vec3(20.0f));
	//glm::scale(glm::mat4(1.0f), glm::vec3(100.0f));

	//create a plane enit
	RenderableEntity plane{
		&mQuadPrimitive,
		plane_transform,
		true, //entity.solidRender, 
		true, //entity.canCastShadow, 
		glm::vec4(1.0f), //entity.colour, 
		false, //entity.plainTexture
	};
	mEntities.push_back(plane);


	glm::vec3 mOrbitOrigin = glm::vec3(0.0f, 10.0f, 0.0f);
	glm::mat4 origin = glm::translate(glm::mat4(1.0f), mOrbitOrigin);// *
					   //glm::scale(glm::mat4(1.0f), glm::vec3(2.5f));


	for (unsigned int i = 0; i < 70; i++)
	{
		RenderableEntity enitt{
			&mSpherePrimitive,
			glm::translate(origin,Util::Random::PointInSphere(10.0f)),
			true, //bool(rand() % 2), //entity.solidRender, 
			true, //entity.canCastShadow, 
			glm::vec4(1.0f), //entity.colour, 
			false, //entity.plainTexture
		};
		mEntities.push_back(enitt);
	}

	//RenderableMesh stanford_bunny_mesh = ModelLoader::LoadAsSingleMesh()
	ModelLoader model_loader;
	RenderableMesh stanford_bunny_mesh = model_loader.LoadAsSingleMesh("assets/models/stanford-bunny.obj");
	RenderableEntity bunny{
		new RenderableMesh(stanford_bunny_mesh),
		//&stanford_bunny_mesh, bad as the renderable mesh goes out of scope it gets deleted
		glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(3.0f)),
		true,
		true,
		glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
		false//true
	};
	mEntities.push_back(bunny);
	//set model transform ptr
	ptr_model_transform = &mEntities.back().transform;
}

