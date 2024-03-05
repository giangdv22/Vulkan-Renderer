#pragma once

/*********** C/C++ HEADER FILES ***********/
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>
#include <assert.h>
#include <set>
#include <optional>
#include <chrono>
#include <thread>
#include <map>
#include <unordered_map>

// Header files for Singleton
#include <memory>
#include <mutex>

/*********** VULKAN HEADER FILES ***********/
#include <vulkan/vulkan.hpp>

/*********** GLM HEADER FILES ***********/
#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace Giang
{
	template<typename T>
	using Scope = std::unique_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Ref = std::shared_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}
}