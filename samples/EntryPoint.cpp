#pragma once
#include "Core/Log.h"
#include "Core/Application.h"
#include "SampleGfxProgram.h"

#include "SampleLightScene.h"


int main()
{
	DEBUG_LOG("Launch PreglRenderer Application Program !!!!!!!!");


	auto app = Application({ "PreglRenderer App", false, {1920, 1080} });
	
	//Add Gfx Program to run 
	//app.AddGfxProgram(new SampleGfxProgram());
	//auto gfx = new SampleGfxProgram()
	// gfx.SetCamera(app.MainCam);
	//app.AddGfxProgram(gfx);
	//

	//glm::vec3 mVector(2.0f, 1.0f, 0.0f);
	//DEBUG_LOG_WARNING("This is crazy; " , mVector);
	//DEBUG_LOG_ERROR("fbnfj", "esgr");
	//DEBUG_LOG_ERROR("fbnfj", "esgr", "dsvk");
	//DEBUG_LOG_STATUS("This is my new log");
	//DEBUG_LOG_INFO(mVector, ", ", mVector, " = ", mVector + mVector);
	//Debug::Log<Debug::SEVERITY::ERROR>("Lets goooooo");
	//Debug::Log("Lets goooooo");

	//auto gfx = new SampleGfxProgram();
	auto gfx = new SampleLightingProgram();
	gfx->OnInitialise(&app.GetWindow());
	gfx->SetCamera(&app.GetCamera());
	app.AddGfxProgram(gfx);

	app.Run();

	return 0;
}



