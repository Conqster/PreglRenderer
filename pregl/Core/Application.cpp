#include "Application.h"
#include "InputSystem.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

//remove this later
#include <GLM/glm/gtc/matrix_transform.hpp>

#include "Util.h"

#include "Profiler.h"
#include "Renderer/DebugGizmosRenderer.h"
#include "GraphicsProgramInterface.h"


//using imgui
#if USE_IMGUI
	#include "libs/imgui/imgui.h"
	#include "libs/imgui/imgui_impl_glfw.h"
	#include "libs/imgui/imgui_impl_opengl3.h"
#endif // USE_IMGUI

#include "Log.h"

using namespace InputSystem;

Application::Application(const ApplicationSpecification& app_spec)
{

	if (!mDisplayManager.Init(app_spec.name.c_str(), app_spec.launchWindowSize[0], app_spec.launchWindowSize[1], app_spec.launchFullScreen))
		bFailLaunch = true;


	if (!bFailLaunch)
	{
		mMainCamera = EditorCamera(glm::vec3(0.41f, 5.29f, -10.61f), 90.0f, -0.3f, 17.5f, 4.0f);

		mPtrInputEventHandle = &InputSystem::EventHandler::Instance();
		mPtrInputEventHandle->CreateCallbacks(mDisplayManager.GetWindow());

		//ImGui
#if USE_IMGUI
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

		//ImGui Renderer
		ImGui_ImplGlfw_InitForOpenGL(mDisplayManager.GetWindow(), true);
		ImGui_ImplOpenGL3_Init("#version 400");
#endif // USE_IMGUI


		if (!DebugGizmosRenderer::Instance().Initialise())
		{
			DEBUG_LOG_WARNING("[APP -- {", app_spec.name, "}], Failed to initialise debug gizmos renderer");
		}
		DebugGizmosRenderer::Instance().SetLineWidth(3.0f);
	}
}

Application::~Application()
{
	if (mGfxProgram)
	{
		mGfxProgram->OnDestroy();
		delete mGfxProgram;
		mGfxProgram = nullptr;
	}


	//Shut down UI (ImGui)
#if USE_IMGUI
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
#endif // USE_IMGUI

	DebugGizmosRenderer::Instance().ClearData();
	DEBUG_LOG_STATUS("Terminating Program!!!!");
	mDisplayManager.Close();
	mPtrInputEventHandle = nullptr;
}

void Application::Run()
{
	if (bFailLaunch)
		return;

	//main loop {delta time >> camera 
	while (mDisplayManager.ProgramWindowActive())
	{
		static float loop_time = 0.0f;
		//SCOPE_TIME("Apploop");
		SCOPE_TIME("Main Loop", &loop_time);


		double curr_frame_time = glfwGetTime();
		mFrameDeltaTime = curr_frame_time - mLastFrameTime;
		mLastFrameTime = curr_frame_time;

		//Gfx Program Interface
		if(mGfxProgram)
			mGfxProgram->OnUpdate(mFrameDeltaTime);


		//Program name | VSync: true | 23.4 ms | 23 FPS
		std::string vsync = mDisplayManager.GetVSync() ? "true | " : "false | ";
		std::string title_update = mDisplayManager.GetName();
		title_update += " | VSync: " +
			vsync +
			std::to_string(mFrameDeltaTime) + "ms | " +
			std::to_string(1 / mFrameDeltaTime) + "FPS ";


		if (Input::GetKeyUp(IKeyCode::V))
			mDisplayManager.SetVSync(!mDisplayManager.GetVSync());

		mDisplayManager.ChangeWindowTitle(title_update.c_str());

		UpdateMainCamera(mFrameDeltaTime);


		//UI STUFFS(IMGUI)
#if USE_IMGUI
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		//ui logic
		ApplicationUI();
		if (mGfxProgram)
			mGfxProgram->OnUI();

		//present all ui stuffs
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif // USE_IMGUI



		//Resolve frame
		DebugGizmosRenderer::Instance().SendBatchesToGPU(mMainCamera, mDisplayManager.GetAspectRatio());
		if (mPtrInputEventHandle)
			mPtrInputEventHandle->FlushFrameInputs();
		mDisplayManager.FlushAndSwapBuffer();
	}

}

void Application::UpdateMainCamera(float dt)
{
	if (mDisplayManager.GetLockCursor())
		mMainCamera.Rotate(Input::GetMouseAxisFloat(IAxis::Vertical), Input::GetMouseAxisFloat(IAxis::Horizontal));


	//inputs
	if (Input::GetKey(IKeyCode::A))
		mMainCamera.Translate(-mMainCamera.GetRight(), dt);
	if (Input::GetKey(IKeyCode::D))
		mMainCamera.Translate(mMainCamera.GetRight(), dt);
	if (Input::GetKey(IKeyCode::W))
		mMainCamera.Translate(mMainCamera.GetForward(), dt);
	if (Input::GetKey(IKeyCode::S))
		mMainCamera.Translate(-mMainCamera.GetForward(), dt);
	if (Input::GetKey(IKeyCode::Q))
		mMainCamera.Translate(-mMainCamera.GetUp(), dt);
	if (Input::GetKey(IKeyCode::E))
		mMainCamera.Translate(mMainCamera.GetUp(), dt);



	if (Input::GetMousePressed(IKeyCode::MouseRightButton))
	{
		mDisplayManager.ToggleLockCursor();
	}
}

void Application::ApplicationUI()
{
	//ImGui::ShowDemoWindow();
	if (ImGui::Begin("Sample Gfx Program"))
	{
		ImGui::SeparatorText("PreglRenderer App");
		//ImGui::Text("Win Title: %s", mDisplayManager.);
		ImGui::Text("Window Width: %d", mDisplayManager.GetWidth());
		ImGui::Text("Window Height: %d", mDisplayManager.GetHeight());

		static bool helper_vsync_toggle = mDisplayManager.GetVSync();
		if(ImGui::Checkbox("VSync", &helper_vsync_toggle))
			mDisplayManager.SetVSync(helper_vsync_toggle);


		//CAMERA
		ImGui::SeparatorText("Camera info");
		auto cam_pos = mMainCamera.GetPosition();
		ImGui::Text("Position x: %f, y: %f, z: %f", cam_pos.x, cam_pos.y, cam_pos.z);

		ImGui::Text("Pitch: %f", mMainCamera.GetPitch());
		ImGui::Text("Yaw: %f", mMainCamera.GetYaw());


		if (ImGui::TreeNode("Camera Properties"))
		{
			ImGui::SliderFloat("Move Speed", &mMainCamera.mMoveSpeed, 5.0f, 250.0f);
			ImGui::SliderFloat("Rot Speed", &mMainCamera.mRotSpeed, 0.0f, 10.0f, "%.1f");

			ImGui::SliderFloat("FOV", &mMainCamera.mFOV, 0.0f, 179.0f, "%.1f");
			ImGui::DragFloat("Near", &mMainCamera.mNear, 0.1f, 0.1f, 50.0f, "%.1f");
			ImGui::DragFloat("Far", &mMainCamera.mFar, 0.1f, 0.0f, 500.0f, "%.1f");

			ImGui::TreePop();
		}
		

		ImGui::End();
	}
}
