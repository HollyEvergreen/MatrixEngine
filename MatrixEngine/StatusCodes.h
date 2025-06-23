#pragma once
#include <cstdint>

enum StatusCodes : int8_t{
	// Error Codes -128 -> -1
	Fail = -128,
	// Success Codes 0 -> 127
	Success 
};