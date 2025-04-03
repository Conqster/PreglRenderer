#pragma once
#include "Core/Application.h"
#include "SampleGfxProgram.h"

int main()
{
	printf("Launch PreglRenderer Application Program !!!!!!!!\n");

	printf("Size of Application: %d bytes\n", int(sizeof(Application)));
	auto app = Application({ "PreglRenderer App", false, {1920, 1080} });
	
	//Add Gfx Program to run 
	//app.AddGfxProgram(new SampleGfxProgram());
	//auto gfx = new SampleGfxProgram()
	// gfx.SetCamera(app.MainCam);
	//app.AddGfxProgram(gfx);
	//

	auto gfx = new SampleGfxProgram();
	gfx->OnInitialise(&app.GetDisplay());
	gfx->SetCamera(&app.GetCamera());
	app.AddGfxProgram(gfx);

	app.Run();

	return 0;
}



