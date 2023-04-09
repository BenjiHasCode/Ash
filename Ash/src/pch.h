#pragma once

// Utility
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>

// Data structures
#include <string>
#include <sstream>
#include <vector>
#include <array>
#include <unordered_map>
#include <unordered_set>

// Logging
#include "core/log.h"

// Platform specific
#ifdef DUSTY_PLATFORM_WINDOWS
	#include <Windows.h>
#endif // DUSTY_PLATFORM_WINDOWS