#include "Interpreter.h"
#include "Logger.h"
#include "fstream"
#include <random>

Interpreter::Interpreter(int display_fps) {
	m_instructionsPerFrame = m_instructionsPerSecond / display_fps;
	srand(time(NULL));

	reset();
}

void Interpreter::load(const char* filePath) {
	std::ifstream file(filePath, std::ios_base::binary);

	int idx = 0x200;
	char byte;
	while (file.get(byte) && idx < 4096) {
		m_memory[idx] = byte;
		++idx;
	}
}

void Interpreter::tick(uint16_t pressedKeys) {
	m_pressedKeys = pressedKeys;
	for (int i = 0; i < m_instructionsPerFrame; ++i) {
		uint8_t* instruction = fetch();
		decode(instruction);
		if (m_timer > 0)
			--m_timer;
		if (m_soundTimer > 0)
			--m_soundTimer;
	}
}

const std::array<std::array<uint8_t, 32>, 64>& Interpreter::getPixels()
{
	return m_display;
}

uint8_t* Interpreter::fetch() {
	uint8_t* nextInstruction = &m_memory[m_pc];
	m_pc += 2;
	return nextInstruction;
}

void Interpreter::decode(uint8_t* instruction) {
	uint8_t w = *instruction & 0xF0;
	uint8_t x = *instruction & 0x0F;
	uint8_t y = (*(instruction + 1) & 0xF0) >> 4;
	uint8_t n = *(instruction + 1) & 0x0F;
	uint8_t nn = *(instruction + 1) & 0xFF;
	uint16_t nnn = (*instruction & 0x0F) << 8 | *(instruction + 1);
	uint16_t full = (*instruction & 0xFF) << 8 | *(instruction + 1);

#if 0
	LOG("W: " << ASBIN(w));
	LOG("X: " << ASBIN(x));
	LOG("Y: " << ASBIN(y));
	LOG("N: " << ASBIN(n));
	LOG("NN: " << ASBIN(nn));
	LOG("NNN: " << ASBIN(nnn));
#endif

	switch (w) {
	case 0x00:
		if (nnn == 0x0E0)
			i_clearScreen();
		else if (nnn == 0x0EE)
			i_endSubroutine();
		else if (nnn == 0x000)
			LOG("Trying to execute 0x0000, something's wrong (?)");
		break;
	case 0x10:
		i_jump(nnn);
		break;
	case 0x20:
		i_callSubroutine(nnn);
		break;
	case 0x30: // Skip Eqs
		i_skipRegVal(x, nn, false);
		break;
	case 0x40:
		i_skipRegVal(x, nn, true);
		break;
	case 0x50:
		i_skipRegReg(x, y, false);
		break;
	case 0x90:
		i_skipRegReg(x, y, true);
		break; // End Skip Eqs
	case 0x60:
		i_setRegister(x, nn);
		break;
	case 0x70:
		i_addValue(x, nn);
		break;
	case 0x80:
		i_doMath(x, y, n);
		break;
	case 0xA0:
		i_setIndex(nnn);
		break;
	case 0xB0:
		i_offsetJump(nnn);
		break;
	case 0xC0:
		i_getRandom(x, nn);
		break;
	case 0xD0:
		i_display(x, y, n);
		break;
	case 0xE0:
		if (nn == 0x9E)
			i_skipKey(x, false);
		else if (nn == 0xA1)
			i_skipKey(x, true);
		break;
	case 0xF0:
		if (nn == 0x07)
			m_registers[x] = m_timer;
		else if (nn == 0x15)
			m_timer = m_registers[x];
		else if (nn == 0x18)
			m_soundTimer = m_registers[x];
		else if (nn == 0x1E)
			i_addToIndex(x);
		else if (nn == 0x0A)
			i_getKey(x);
		else if (nn == 0x29)
			m_index = (m_registers[x] & 0x0F) * 5 + m_fontOffset;
		else if (nn == 0x33)
			i_decimalConversion(x);
		else if (nn == 0x55)
			i_storeRegisters(x);
		else if (nn == 0x65)
			i_loadRegisters(x);
		break;
	default:
		LOG("Trying to execute an unknown instruction: ");
		break;
	}
	LogStatus(full);
}

void Interpreter::i_clearScreen() {
	for (int x = 0; x < 64; ++x) {
		for (int y = 0; y < 32; ++y) {
			m_display.at(x).at(y) = 0;
		}
	}
}

void Interpreter::i_jump(uint16_t address) {
	m_pc = address;
}

void Interpreter::i_offsetJump(uint16_t address) {
	uint8_t newAddress = 0;
	uint8_t reg = m_useNewOffsetJump ? (address & 0x0F00) >> 8 : 0;
	uint8_t offset = m_registers[reg];
	newAddress = address + offset;
	i_jump(newAddress);
}

void Interpreter::i_callSubroutine(uint16_t address) {
	m_stack[m_stackIndex++] = m_pc;
	m_pc = address;
}

void Interpreter::i_endSubroutine() {
	m_pc = m_stack[--m_stackIndex];
	m_stack[m_stackIndex] = 0;
}

void Interpreter::i_skipRegVal(uint8_t reg, uint8_t value, bool notEq) {
	uint8_t regValue = m_registers[reg];

	bool skip = false;

	if (regValue == value) {
		if (!notEq)
			skip = true;
	}
	else {
		if (notEq)
			skip = true;
	}

	if (skip) {
		m_pc += 2;
	}
}

void Interpreter::i_skipRegReg(uint8_t xreg, uint8_t yreg, bool notEq) {
	uint8_t regValue = m_registers[yreg];
	i_skipRegVal(xreg, regValue, notEq);
}

void Interpreter::i_skipKey(uint8_t reg, bool invert) {
	uint8_t key = m_registers[reg];
	bool isPressed = ((m_pressedKeys >> key) & 0x01) == 1;
	if (invert)
		isPressed = !isPressed;
	if (isPressed)
		m_pc += 2;
}

void Interpreter::i_doMath(uint8_t xreg, uint8_t yreg, uint8_t operation) {
	uint8_t a = m_registers[xreg];
	uint8_t b = m_registers[yreg];
	uint8_t c = 0;
	uint8_t d = 0;

	switch (operation) {
	case 0x00:
		m_registers[xreg] = b;
		break;
	case 0x01:
		m_registers[xreg] = a | b;
		break;
	case 0x02:
		m_registers[xreg] = a & b;
		break;
	case 0x03:
		m_registers[xreg] = a ^ b;
		break;
	case 0x04:
		m_registers[0x0F] = 0;
		d = a;
		c = a + b;
		m_registers[xreg] = c;
		if (c < d)
			m_registers[0x0F] = 1;
		break;
	case 0x05:
		m_registers[0x0F] = a > b ? 1 : 0;
		m_registers[xreg] = a - b;
		break;
	case 0x07:
		m_registers[0x0F] = a > b ? 0 : 1;
		m_registers[xreg] = b - a;
		break;
	case 0x06:
		c = m_useNewShift ? a : b;
		m_registers[0x0F] = (c & 0x01) == 1 ? 1 : 0;
		c = c >> 1;
		m_registers[xreg] = c;
		break;
	case 0x0E:
		c = m_useNewShift ? a : b;
		m_registers[0x0F] = (c & 0x80) == 0x80 ? 1 : 0;
		c = c << 1;
		m_registers[xreg] = c;
		break;
	}
}

void Interpreter::i_setRegister(uint8_t reg, uint8_t value) {
	m_registers[reg] = value;
}

void Interpreter::i_addValue(uint8_t reg, uint8_t value) {
	m_registers[reg] += value;

}

void Interpreter::i_setIndex(uint16_t value) {
	m_index = value;
}

void Interpreter::i_addToIndex(uint8_t reg) {
	m_index += m_registers[reg];
	if (m_useNewIndexAddition && m_index > 0x0FFF)
			m_registers[0x0F] = 1;
}

void Interpreter::i_getRandom(uint8_t reg, uint8_t value) {
	uint8_t num = rand() % 255;
	m_registers[reg] = num & value;
}

void Interpreter::i_getKey(uint8_t reg) {
	if (m_pressedKeys == 0) {
		m_pc -= 2;
		return;
	}

	uint8_t pressedKey = 0;
	for (int i = 0; i < 16; ++i) {
		pressedKey = (m_pressedKeys >> i) & 0x01;
		if (pressedKey == 1) {
			pressedKey = i;
			break;
		}
	}
	m_registers[reg] = pressedKey;
}

void Interpreter::i_decimalConversion(uint8_t reg) {
	uint8_t num = m_registers[reg];
	m_memory[m_index] = num / 100;
	m_memory[m_index + 1] = (num / 10) % 10;
	m_memory[m_index + 2] = num % 10;
}

void Interpreter::i_storeRegisters(uint8_t reg) {
	for (int i = 0; i <= reg; ++i) {
		m_memory[m_index + i] = m_registers[i];
	}
	if (!m_useNewStoringAndLoading)
		m_index = m_index + reg + 1;
}

void Interpreter::i_loadRegisters(uint8_t reg) {
	for (int i = 0; i <= reg; ++i) {
		m_registers[i] = m_memory[m_index + i];
	}
	if (!m_useNewStoringAndLoading)
		m_index = m_index + reg + 1;
}

void Interpreter::i_display(uint8_t xreg, uint8_t yreg, uint8_t height) {
	uint8_t x = m_registers[xreg] % 64;
	uint8_t y = m_registers[yreg] % 32;
	uint8_t* spritePtr = &m_memory[m_index];

	m_registers[0x0F] = 0;

	for (int j = 0; j < height; ++j) {
		if (y + j >= 32) break;
		uint8_t sprite = *spritePtr;

		for (int i = 0; i < 8; ++i) {
			if (x + i >= 64) break;

			bool isPixelOn = m_display.at(x + i).at(y + j) == 1;
			if (sprite & (128 >> i)) { // 1000 0000
				if (isPixelOn) {
					m_display.at(x + i).at(y + j) = 0;
					m_registers[0x0F] = 1;
				}
				else {
					m_display.at(x + i).at(y + j) = 1;
				}
			}
		}

		++spritePtr;
	}
}

void Interpreter::LogStatus(uint16_t current) {
	LOG("------------------------------------------------------------");
	LOGSL("Current Instruction: ");
	LOGHEX(current);
	LOGSL("PC: ");
	LOGHEX(m_pc);
	LOGSL("INDEX: ");
	LOGHEX(m_index);

	LOGSL("Registers: 0:[");
	LOGHEXSL(m_registers[0x00]);
	for (int i = 1; i < 16; ++i) {
		LOGSL("] ");
		LOGHEXSL(i);
		LOGSL(":[");
		LOGHEXSL(m_registers[i]);
	}
	LOG("]");

	LOGSL("Stack(" << static_cast<int>(m_stackIndex) << "): [");
	LOGHEXSL(m_stack[0]);
	for (int i = 1; i < 48; ++i) {
		LOGSL(", ");
		LOGHEXSL(m_stack[i]);
	}
	LOG("]");
}

void Interpreter::reset() {
	// Memory
	for (int i = 0; i < 4096; ++i) {
		m_memory[i] = 0;
	}
	// Stack
	for (int i = 0; i < 48; ++i) {
		m_stack[i] = 0;
	}
	// Registers
	for (int i = 0; i < 16; ++i) {
		m_registers[i] = 0;
	}

	i_clearScreen();

	// Copy font to memory
	for (int i = 0; i < 80; ++i) {
		m_memory[i + m_fontOffset] = m_font[i];
	}
}
