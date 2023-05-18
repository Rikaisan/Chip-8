#include "Application.h"
#include <raylib/raylib.h>

Application::Application() {}

void Application::run() {
	InitWindow(640, 640, "Chip-8 Emulator");

	SetTargetFPS(m_fps);

	Interpreter emulator(m_fps);
	emulator.load("programs/Brix [Andreas Gustafsson, 1990].ch8");

	while (!WindowShouldClose()) {
		emulator.tick(getPressedKeys());
		BeginDrawing();

			DrawFPS(10, 10);

			drawPixels(emulator);

			ClearBackground(BLACK);

			if (IsKeyPressed(KEY_T)) {
				emulator.tick(0);
			}

		EndDrawing();
	}
}

void Application::drawPixels(Interpreter& emu) {
	// Plates
	int pixelSize = 10;

	Color barsColor = Color{ 0x0c, 0x0c, 0x0c, 0xFF };
	Color lastFrameColor = Color{ 0xBB, 0xBB, 0xBB, 0xFF };
	auto& pixels = emu.getPixels();

	for (int x = 0; x < 64; ++x) {
		for (int y = 0; y < 32; ++y) {
			if (m_lastFrame.at(x).at(y) == 1)
				DrawRectangle(x * pixelSize, (y + 16) * pixelSize, pixelSize, pixelSize, lastFrameColor);
		}

		for (int y = 0; y < 16; ++y) {
			DrawRectangle(x * pixelSize, y * pixelSize, pixelSize, pixelSize, barsColor);
		}

		for (int y = 0; y < 32; ++y) {
			m_lastFrame.at(x).at(y) = pixels.at(x).at(y);

			if (pixels.at(x).at(y) == 1)
				DrawRectangle(x * pixelSize, (y + 16) * pixelSize, pixelSize, pixelSize, Color{ 0xFF, 0xFF, 0xFF, 0xFF });
		}

		for (int y = 48; y < 64; ++y) {
			DrawRectangle(x * pixelSize, y * pixelSize, pixelSize, pixelSize, barsColor);
		}
	}
}

uint16_t Application::getPressedKeys() {
	uint16_t keys = 0;
	if (IsKeyDown(KEY_ONE))
		keys |= 1 << 0x01; // 1
	if (IsKeyDown(KEY_TWO))
		keys |= 1 << 0x02; // 2
	if (IsKeyDown(KEY_THREE))
		keys |= 1 << 0x03; // 3
	if (IsKeyDown(KEY_FOUR))
		keys |= 1 << 0x0C; // C
	if (IsKeyDown(KEY_Q))
		keys |= 1 << 0x04; // 4
	if (IsKeyDown(KEY_W))
		keys |= 1 << 0x05; // 5
	if (IsKeyDown(KEY_E))
		keys |= 1 << 0x06; // 6
	if (IsKeyDown(KEY_R))
		keys |= 1 << 0x0D; // D
	if (IsKeyDown(KEY_A))
		keys |= 1 << 0x07; // 7
	if (IsKeyDown(KEY_S))
		keys |= 1 << 0x08; // 8
	if (IsKeyDown(KEY_D))
		keys |= 1 << 0x09; // 9
	if (IsKeyDown(KEY_F))
		keys |= 1 << 0x0E; // E
	if (IsKeyDown(KEY_Z))
		keys |= 1 << 0x0A; // A
	if (IsKeyDown(KEY_X))
		keys |= 1 << 0x00; // 0
	if (IsKeyDown(KEY_C))
		keys |= 1 << 0x0B; // B
	if (IsKeyDown(KEY_V))
		keys |= 1 << 0x0F; // F
	return keys;
}
