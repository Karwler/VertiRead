#pragma once

#include "engine.h"

class World {
public:
	static kptr<Engine> engine;
	static vector<string> args;

	static AudioSys* audioSys();
	static InputSys* inputSys();
	static WindowSys* winSys();
	static Scene* scene();
	static Library* library();
	static Program* program();

	static void PlaySound(const string& sound);
};
