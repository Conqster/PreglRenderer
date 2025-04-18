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
	#include <libs/imgui/imgui.h>
	#include <libs/imgui/imgui_impl_glfw.h>
	#include <libs/imgui/imgui_impl_opengl3.h>

	#include "Core/UI_Window_Panel_Editors.h"
#endif // USE_IMGUI

#include "Log.h"

#include "HeapMemAllocationTracking.h"

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

		//UI functions
		UI::Initialise();
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
	DEBUG_LOG("Closing Application");
	if (mGfxProgram)
	{
		mGfxProgram->OnDestroy();
		delete mGfxProgram;
		mGfxProgram = nullptr;
	}


	//Shut down UI (ImGui)
#if USE_IMGUI
	//UI function
	UI::ShutDown();

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

	//stack based text to prevent write to heap string
	char win_title_char_buf[128];
	int write_offset = 0;

	//main loop {delta time >> camera 
	while (mDisplayManager.ProgramWindowActive())
	{
		SCOPE_MEM_ALLOC_PROFILE("MainLoop");
		static float loop_time = 0.0f;
		//SCOPE_TIME("Apploop");
		SCOPE_TIME("Main Loop", &loop_time);

		double curr_frame_time = glfwGetTime();
		mFrameDeltaTime = curr_frame_time - mLastFrameTime;
		mLastFrameTime = curr_frame_time;

		////Gfx Program Interface
		if (mGfxProgram)
			mGfxProgram->OnUpdate(mFrameDeltaTime);


		auto& mem_alloc_tracker = Util::Memory::GetMemAllocTracker();

		//Program name | VSync: true | 23.4 ms | 23 FPS
		//writing in the char buffer 
		// reset write offset per frame
		write_offset = 0;
		//update write offset to determine stack memory location  
		//write based on offset (win_title_update + write_offset)
		// writable size/writable left based on used (sizeof(win_title_update) - write_offset)
		// writing fmt/ info 
		// ...... 
		write_offset += snprintf(win_title_char_buf + write_offset, sizeof(win_title_char_buf) - write_offset, "%s | VSync: %s | ",
						mDisplayManager.GetName().c_str(), mDisplayManager.GetVSync() ? "true" : "false");
		
		write_offset += snprintf(win_title_char_buf + write_offset, sizeof(win_title_char_buf) - write_offset, "%.2fms | %.2fFPS",
						mFrameDeltaTime, (1.0f / mFrameDeltaTime));

		

		//28
		DEBUG_LOG("Titile Prinf Update size ", sizeof(win_title_char_buf), ".size(): ", write_offset);


		if (Input::GetKeyUp(IKeyCode::V))
			mDisplayManager.SetVSync(!mDisplayManager.GetVSync());

		mDisplayManager.ChangeWindowTitle(win_title_char_buf);

		UpdateMainCamera(mFrameDeltaTime);


		DebugGizmosRenderer::Instance().SendBatchesToGPU(mMainCamera, mDisplayManager.GetAspectRatio());
		//UI STUFFS(IMGUI)
#if USE_IMGUI
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		//ui logic
		ApplicationUI();
		//OPEN_BLOCK_MEM_TRACKING_PROFILE(block_program);
		if (mGfxProgram)
			mGfxProgram->OnUI();
		//CLOSE_BLOCK_MEM_TRACKING_PROFILE(block_program);

		//present all ui stuffs
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif // USE_IMGUI


		//Resolve frame
		if (mPtrInputEventHandle)
			mPtrInputEventHandle->FlushFrameInputs();
		mDisplayManager.FlushAndSwapBuffer();
		//printf("===============================================END OF FRAME=====================================================\n");
		//DEBUG_LOG("Current Heap Memory Allocation Count: ", mem_alloc_tracker.CurrentAllocation(), ".");
		//std::cout << "Current Heap Memory Allocation Size: " << mem_alloc_tracker.CurrentUsage() << " bytes.\n";
		//std::cout << "Program Overall Allocation Count: " << mem_alloc_tracker.allocatedCount << ".\n";
		//std::cout << "Program Overall Allocation Size: " << mem_alloc_tracker.allocatedSize << " bytes.\n";
		//std::cout << "Program Overall Freed Count: " << mem_alloc_tracker.freeCount << ".\n";
		//std::cout << "Program Overall Freed Size: " << mem_alloc_tracker.freeSize << " bytes.\n";
		//printf("===============================================END OF INFO=====================================================\n");
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

		if (ImGui::TreeNode("Available Extranal UI"))
		{
			for (auto& ui : UI::GetRegisteredUIFlags())
			{
				ImGui::PushID(&ui);
				ImGui::Checkbox(ui.name.c_str(), ui.open);
				ImGui::PopID();
			}
			ImGui::TreePop();
		}
	}
	ImGui::End();
}
