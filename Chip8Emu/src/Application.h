#pragma once

#include "Interpreter.h"

class Application {
public:
	Application();
	void run();
	void drawPixels(Interpreter& emu);
	uint16_t getPressedKeys();
private:
	int m_fps = 60;
	std::array<std::array<uint8_t, 32>, 64> m_lastFrame;
};