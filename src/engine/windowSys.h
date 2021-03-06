#pragma once

#include "utils/utils.h"

// handles window events and contains video settings
class WindowSys {
private:
	static constexpr char title[] = "VertiRead";
	static constexpr ivec2 windowMinSize = { 500, 300 };
	static constexpr uint32 eventCheckTimeout = 50;
	static constexpr float ticksPerSec = 1000.f;

	FileSys* fileSys;
	DrawSys* drawSys;
	InputSys* inputSys;
	Program* program;
	Scene* scene;
	Settings* sets;
	SDL_Window* window = nullptr;
	float dSec = 0.f;		// delta seconds, aka the time between each iteration of the above mentioned loop
	bool run = true;		// whether the loop in which the program runs should continue

public:
	int start();
	void close();

	float getDSec() const;
	uint32 windowID() const;
	ivec2 displayResolution() const;
	void setWindowPos(ivec2 pos);
	void moveCursor(ivec2 mov);
	void toggleOpacity();
	void setFullscreen(bool on);
	void setResolution(ivec2 res);
	void setRenderer(const string& name);
	void resetSettings();

	FileSys* getFileSys();
	DrawSys* getDrawSys();
	InputSys* getInputSys();
	Program* getProgram();
	Scene* getScene();
	Settings* getSets();

private:
	void init();
	void exec();

	void createWindow();
	void destroyWindow();
	void handleEvent(const SDL_Event& event);	// pass events to their specific handlers
	void eventWindow(const SDL_WindowEvent& winEvent);
};

inline void WindowSys::close() {
	run = false;
}

inline float WindowSys::getDSec() const {
	return dSec;
}

inline FileSys* WindowSys::getFileSys() {
	return fileSys;
}

inline DrawSys* WindowSys::getDrawSys() {
	return drawSys;
}

inline InputSys* WindowSys::getInputSys() {
	return inputSys;
}

inline Program* WindowSys::getProgram() {
	return program;
}

inline Scene* WindowSys::getScene() {
	return scene;
}

inline Settings* WindowSys::getSets() {
	return sets;
}

inline uint32 WindowSys::windowID() const {
	return SDL_GetWindowID(window);
}

inline ivec2 WindowSys::displayResolution() const {
	SDL_DisplayMode mode;
	return !SDL_GetDesktopDisplayMode(window ? SDL_GetWindowDisplayIndex(window) : 0, &mode) ? ivec2(mode.w, mode.h) : ivec2(INT_MAX);
}

inline void WindowSys::setWindowPos(ivec2 pos) {
	SDL_SetWindowPosition(window, pos.x, pos.y);
}
