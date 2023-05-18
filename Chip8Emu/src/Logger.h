#pragma once

#ifdef NDEBUG
#define LOG(x)
#define LOGSL(x)
#define LOGHEX(x)
#define LOGBIN(x)
#define ASBIN(x)
#else
#include <iostream>
#include <bitset>
#define LOG(x) do { \
	std::cout << x << std::endl;\
} while(0)
#define LOGSL(x) do { \
	std::cout << x;\
} while(0)
#define LOGHEX(x) do { \
	std::ios::fmtflags f(std::cout.flags()); \
	std::cout << std::hex << static_cast<int>(x) << std::endl;\
	std::cout.flags(f); \
} while(0)
#define LOGHEXSL(x) do { \
	std::ios::fmtflags f(std::cout.flags()); \
	std::cout << std::hex << static_cast<int>(x);\
	std::cout.flags(f); \
} while(0)
#define LOGBIN(x) do { \
	std::bitset<8 * sizeof(x)> bits(x); \
	std::cout << bits << std::endl;\
} while(0)
#define ASBIN(x) std::bitset<8 * sizeof(x)>(x)
#endif