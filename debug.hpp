#pragma once
#include "constants.hpp"
#ifndef DEBUG
#define DEBUG_PRINT(a)
#else
#define DEBUG_PRINT(a) Serial.print(a)
#endif