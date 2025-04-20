#pragma once

#include "AppWindow.h"
#include "NonCopyable.h"

class GraphicsProgramInterface : public NonCopyable
{
public: 
	GraphicsProgramInterface() {}
	GraphicsProgramInterface(AppWindow* display_window) {
		mDisplayManager = display_window;
	}
	virtual void OnInitialise(AppWindow* display_window) = 0;
	virtual void OnUpdate(float delta_time) = 0;
	//concluting frame
	virtual void OnLateUpdate(float delta_time) = 0;
	virtual void OnDestroy() = 0;

	virtual void OnUI() = 0;

	void SetCamera(class EditorCamera* cam) {
		mCamera = cam;
	}
protected:
	AppWindow* mDisplayManager = nullptr;
	EditorCamera* mCamera = nullptr;
};
