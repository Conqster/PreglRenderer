#pragma once

#include "Display/DisplayManager.h"
#include "Camera.h"
#include <string>


struct ApplicationSpecification
{
	std::string name = "Test Application";
	bool launchFullScreen = false;
	int launchWindowSize[2] = { 2133, 1199 };
};


//int main();

#define USE_IMGUI 1



namespace InputSystem {
	class EventHandler;
}
class Application
{
private:
	DisplayManager mDisplayManager;


	class GraphicsProgramInterface* mGfxProgram = nullptr;
	class DebugGizmosRenderer* mDebugGizmos = nullptr;

	Camera mMainCamera;

	//Pointer to global static event handler
	InputSystem::EventHandler* mPtrInputEventHandle = nullptr;

public:
	Application() = delete;
	Application(const ApplicationSpecification& app_spec);

	void AddGfxProgram(GraphicsProgramInterface* Gfx) { mGfxProgram = Gfx; }
	DisplayManager& GetDisplay() { return mDisplayManager; }
	Camera& GetCamera() { return mMainCamera; }

	virtual ~Application();

	//main loop
	void Run();
private:
	bool bFailLaunch = false;
	double mFrameDeltaTime = 0.0f;
	double mLastFrameTime = 0.0f;

	void UpdateMainCamera(float dt);
};