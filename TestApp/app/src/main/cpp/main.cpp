#include "AndroidOut.hpp"

#include "Hiena/Hiena.hpp"
#include "MainActivity.hpp"

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
	return hiena::Initialize(vm, {.MainClass = "com/testapp/example/MainActivity"});
}

#include <game-activity/native_app_glue/android_native_app_glue.h>
#include <game-activity/GameActivity.h>

extern "C" {

void android_main(struct android_app *pApp)
{
	aout << "Welcome to android_main" << std::endl;

	int events;
	android_poll_source *pSource;
	do
	{
		auto C = hiena::FindClass("com/testapp/example/MainActivity",{});
		auto C2 = hiena::FindClass("java/lang/String",{});
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