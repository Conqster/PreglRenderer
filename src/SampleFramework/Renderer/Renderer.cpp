#include "Renderer.h"

#include "Texture.h"
#include "Camera.h"

//remove this later
#include <GLM/glm/gtc/matrix_transform.hpp>
#include "Utils/Util.h"
#include "Utils/Loader.h"

#include "Input/InputSystem.h"

Renderer::Renderer(DisplayManager& display_window)
{
	Initialise(display_window);
}

void Renderer::Initialise(DisplayManager& display_window)
{
	mDisplay = display_window;

	bool success = mMeshShader.Create("experiment", "assets/shaders/test.vert", "assets/shaders/test.frag");
	success &= mShadowShader.Create("shadow_depth", "assets/shaders/shadowDepth.vert", "assets/shaders/shadowDepth.frag");
	success &= mWorldGridShader.Create("world_grid", "assets/shaders/worldGrid/worldGrid.vert", "assets/shaders/worldGrid/worldGrid.frag", "assets/shaders/worldGrid/worldGrid.geo");

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

}

void Renderer::BeginFrame(Camera* p_camera)
{
	glClearColor(mClearColour.r, mClearColour.g, mClearColour.b, mClearColour.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mCamera = p_camera;

	if (!p_camera)
	{
		EndFrame();
		return;
	}

	//shadow properties 
	//calculate light project view matrix 
	glm::mat4 view = glm::lookAt(mLightWorldPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 proj = glm::ortho(-30.f, 30.f, -30.f, 30.f, 0.1f, 100.0f);
	mLightProjViewMat = proj * view;


	mMeshShader.Bind();
	//trs
	//test_transform = glm::translate(test_transform, delta_pos * 0.1f);
	mMeshShader.SetUniformMat4("uproj", mCamera->ProjMat(mDisplay.GetAspectRatio()));
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

void Renderer::EndFrame()
{
	//mDisplay.FlushAndSwapBuffer();
	mCamera = nullptr;
	mFrameEntitiesCount = 0;
	//memset(&mFrameRenderableEntities, {}, sizeof(mFrameRenderableEntities));
}

void Renderer::Destroy()
{
	Camera* mCamera = nullptr;

	mMeshShader.Clear();

	mBrickTexture->Clear();
	delete mBrickTexture;
	mBrickTexture = nullptr;

	mCheckersTexture->Clear();
	delete mCheckersTexture;
	mCheckersTexture = nullptr;
}

void Renderer::ShadowPass()
{
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
	glViewport(0, 0, mDisplay.GetWidth(), mDisplay.GetHeight());
}

void Renderer::DrawPass()
{
	DrawObject(mMeshShader);
}

void Renderer::DrawObject(Shader& shader, bool only_depth)
{
	shader.Bind();
	if (mFrameEntitiesCount <= 0)
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

	for (unsigned int i = 0; i < mFrameEntitiesCount; i++)
	{
		if (only_depth && !mFrameRenderableEntities[i].canCastShadow)
			continue;

		RenderableEntity entt = mFrameRenderableEntities[i];
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

	if(!only_depth)
		glDisable(GL_BLEND);
}

void Renderer::AddFrameRenderableEntity(const RenderableEntity entity)
{
	if (mFrameEntitiesCount < mMaxFrameEntity)
	{
		mFrameRenderableEntities[mFrameEntitiesCount++] = entity;
	}
}

