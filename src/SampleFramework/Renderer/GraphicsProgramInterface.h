#pragma once

#include "Display/DisplayManager.h"
#include "NonCopyable.h"

class GraphicsProgramInterface : public NonCopyable
{
public: 
	GraphicsProgramInterface() {}
	GraphicsProgramInterface(DisplayManager* display_window) {
		mDisplayManager = display_window;
	}
	virtual void OnInitialise(DisplayManager* display_window) = 0;
	virtual void OnUpdate(float delta_time) = 0;
	virtual void OnDestroy() = 0;

	virtual void OnUI() = 0;

	void SetCamera(class Camera* cam) {
		mCamera = cam;
	}
protected:
	DisplayManager* mDisplayManager = nullptr;
	Camera* mCamera = nullptr;
};