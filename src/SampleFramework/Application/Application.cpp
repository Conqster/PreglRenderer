#include "Application.h"
#include "Input/InputSystem.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

//remove this later
#include <GLM/glm/gtc/matrix_transform.hpp>

#include "Utils/Util.h"

#include "../Profiler.h"
#include "Renderer/DebugGizmosRenderer.h"
#include "Renderer/GraphicsProgramInterface.h"


//using imgui
#if USE_IMGUI
	#include "libs/imgui/imgui.h"
	#include "libs/imgui/imgui_impl_glfw.h"
	#include "libs/imgui/imgui_impl_opengl3.h"
#endif // USE_IMGUI


using namespace InputSystem;

Application::Application(const ApplicationSpecification& app_spec)
{

	printf("memory size of DisplayManager: %d bytes\n", int(sizeof(DisplayManager)));
	//printf("memory size of Renderer: %d bytes\n", int(sizeof(Renderer)));
	//printf("memory size of SampleGfxProgram: %d bytes\n", int(sizeof(SampleGfxProgram)));
	printf("memory size of DebugGizmosRenderer: %d bytes\n", int(sizeof(DebugGizmosRenderer)));
	printf("memory size of Camera: %d bytes\n", int(sizeof(Camera)));
	printf("memory size of EventHandler: %d bytes\n", int(sizeof(EventHandler)));



	if (!mDisplayManager.Init(app_spec.name.c_str(), app_spec.launchWindowSize[0], app_spec.launchWindowSize[1], app_spec.launchFullScreen))
		bFailLaunch = true;


	if (!bFailLaunch)
	{
		mMainCamera = Camera(glm::vec3(0.41f, 5.29f, -10.61f), 90.0f, -0.3f, 17.5f, 4.0f);

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

		mDebugGizmos = new DebugGizmosRenderer();
		if (!mDebugGizmos->Initialise())
		{
			delete mDebugGizmos;
			mDebugGizmos = nullptr;
			printf("[APP -- {%s}], Failed to initialise debug gizmos renderer\n", app_spec.name.c_str());
		}
		mDebugGizmos->SetLineWidth(3.0f);
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


	if(mDebugGizmos)
		delete mDebugGizmos;
	mDebugGizmos = nullptr;
	printf("Terminating Program!!!!\n");
	mDisplayManager.Close();
	mPtrInputEventHandle = nullptr;
}

void Application::Run()
{
	if (bFailLaunch)
		return;

	const char* gl_renderer_version = (const char*)(glGetString(GL_RENDERER));
	printf("OpenGL ver: %s\n", gl_renderer_version);


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
		if (mGfxProgram)
			mGfxProgram->OnUI();

		//present all ui stuffs
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif // USE_IMGUI



		//Resolve frame
		mDebugGizmos->SendBatchesToGPU(mMainCamera, mDisplayManager.GetAspectRatio());
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
