#pragma once
#include "Core/Log.h"
#include "Core/Application.h"
#include "SampleGfxProgram.h"

#include "SampleLightScene.h"
#include "Core/HeapMemAllocationTracking.h"

int main()
{
	//SCOPE_MEM_ALLOC_PROFILE("Program");
	//OPEN_BLOCK_MEM_TRACKING_PROFILE(Program);
	DEBUG_LOG("Launch PreglRenderer Application Program !!!!!!!!");

	auto app = Application({ "PreglRenderer App", false, {1920, 1080} });
	
	//Add Gfx Program to run 
	//app.AddGfxProgram(new SampleGfxProgram());
	//auto gfx = new SampleGfxProgram()
	// gfx.SetCamera(app.MainCam);
	//app.AddGfxProgram(gfx);
	//

	//auto gfx = new SampleGfxProgram();
	auto gfx = new SampleLightingProgram();
	gfx->OnInitialise(&app.GetWindow());
	gfx->SetCamera(&app.GetCamera());
	app.AddGfxProgram(gfx);

	app.Run();

	//CLOSE_BLOCK_MEM_TRACKING_PROFILE(Program);
	return 0;
}



