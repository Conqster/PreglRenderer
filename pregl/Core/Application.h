#pragma once

#include "AppWindow.h"
#include "EditorCamera.h"
#include <string>

#include "NonCopyable.h"


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
class Application : NonCopyable
{
private:
	AppWindow mDisplayManager;
	class GraphicsProgramInterface* mGfxProgram = nullptr;
	EditorCamera mMainCamera;
	//Pointer to global static event handler
	InputSystem::EventHandler* mPtrInputEventHandle = nullptr;
public:
	Application(const ApplicationSpecification& app_spec);

	void AddGfxProgram(GraphicsProgramInterface* Gfx) { mGfxProgram = Gfx; }
	AppWindow& GetWindow() { return mDisplayManager; }
	EditorCamera& GetCamera() { return mMainCamera; }

	virtual ~Application();

	//main loop
	void Run();
private:
	bool bFailLaunch = false;
	double mFrameDeltaTime = 0.0f;
	double mLastFrameTime = 0.0f;

	void UpdateMainCamera(float dt);


	void ApplicationUI();
};