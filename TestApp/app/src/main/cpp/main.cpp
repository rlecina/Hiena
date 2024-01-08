#include "AndroidOut.hpp"

#include <game-activity/GameActivity.cpp>

#include "Hiena/Hiena.hpp"

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
	return hiena::Initialize(vm, {.MainClass = "com/testapp/example/MainActivity"});
}

extern "C" {

#include <game-activity/native_app_glue/android_native_app_glue.c>

void android_main(struct android_app *pApp)
{
	aout << "Welcome to android_main" << std::endl;

	int events;
	android_poll_source *pSource;
	do
	{
		// Process all pending events before running game logic.
		if (ALooper_pollAll(0, nullptr, &events, (void **) &pSource) >= 0)
		{
			if (pSource)
			{
				pSource->process(pApp, pSource);
			}
		}
	} while (!pApp->destroyRequested);
}
}