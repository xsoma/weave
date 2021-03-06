#pragma once
#include "DLL_MAIN.h"

typedef uint8_t*(*FindPatternIDA_T)(HMODULE szModule, const char* szSignature);
typedef void(*pattern_to_byte_t)(const char*, int**, size_t*);
typedef uint8_t*(*FindPatternIDACore_t)(pattern_to_byte_t, decltype(free), HMODULE, const char*);

class CUtils
{
public:
	FindPatternIDACore_t FindPatternIDACore;
	FindPatternIDA_T FindPatternIDA;
};
