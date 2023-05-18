#pragma once

#include <stdint.h>
#include <array>

class Interpreter {
public:
	Interpreter(int display_fps);
	void load(const char* filePath);
	void tick(uint16_t pressedKeys);
	const std::array<std::array<uint8_t, 32>, 64>& getPixels();
private:
	uint8_t* fetch();
	void decode(uint8_t* instruction);

	void i_clearScreen();
	void i_jump(uint16_t address);
	void i_offsetJump(uint16_t address);
	void i_callSubroutine(uint16_t address);
	void i_endSubroutine();
	void i_skipRegVal(uint8_t reg, uint8_t value, bool notEq);
	void i_skipRegReg(uint8_t xreg, uint8_t yreg, bool notEq);
	void i_skipKey(uint8_t reg, bool invert);
	void i_doMath(uint8_t xreg, uint8_t yreg, uint8_t operation);
	void i_setRegister(uint8_t reg, uint8_t value);
	void i_addValue(uint8_t reg, uint8_t value);
	void i_setIndex(uint16_t value);
	void i_addToIndex(uint8_t reg);
	void i_getRandom(uint8_t reg, uint8_t value);
	void i_getKey(uint8_t reg);
	void i_decimalConversion(uint8_t reg);
	void i_storeRegisters(uint8_t reg);
	void i_loadRegisters(uint8_t reg);
	void i_display(uint8_t xreg, uint8_t yreg, uint8_t height);

	// Debug
	void LogStatus(uint16_t current);
	void reset();


	int16_t m_instructionsPerSecond = 720;
	int m_instructionsPerFrame = 1;

	uint16_t m_pc = 0x200;
	uint8_t m_memory[4096]; // store games after address 0x200 (512)
	uint16_t m_stack[48];
	uint8_t m_stackIndex = 0;
	uint16_t m_index = 0;
	uint8_t m_timer = 255;
	uint8_t m_soundTimer = 255;
	uint8_t m_registers[16] = { 0 };

	// Graphics
	uint16_t m_fontOffset = 100;
	uint8_t m_font[80] = {
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	};
	std::array<std::array<uint8_t, 32>, 64> m_display;

	// State
	uint16_t m_pressedKeys = 0;

	// Compatibility
	bool m_useNewShift = true; // if true, it won't move Y to X before shifting [Chip-48 true]
	bool m_useNewOffsetJump = false; // if true, it will use BXNN instead of BNNN [No info]
	bool m_useNewIndexAddition = false; // if true, it will set VF to 1 if index > 0x0FFF [Commodore Amiga true]-[Spacefight 2091! true, Animal Race false]
	bool m_useNewStoringAndLoading = true; // if true, index won't be altered while saving/loading [SCHIP false]
};
