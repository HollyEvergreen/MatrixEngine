#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include <iostream>
#include <memory>
#include <vector>
template<typename T>
T* MatrixAlloc(size_t count) {
	return (T*)malloc(count * sizeof(T));
}

enum StatusCode {
    //Error sign bit set
    Fail = -128,
	unknownFail,
    //Success sign bit not set
    Success = 0,
	Running,
};

template<typename T>
struct optional {
	bool HasVal = false;

	T operator =(const T& other) {
		option = std::move(other);
	}

	optional(T val) {
		option = val;
		HasVal = true;
	}

	void reset() {
		HasVal = false;
	}

	T unwrap() {
		if (!HasVal) {
			throw std::exception("panic optional has no valid value");
		}
		return option;
	}

private:
	T option;

};

struct QueueFamilyIndices {
	std::vector<optional<uint32_t>> indices = std::vector<optional<uint32_t>>(4);
	bool valid = false;
	void append(uint32_t t) {
		indices.emplace_back(t);
		valid |= true;
	}
	uint32_t first() {
		if (valid) {
			return indices[0].unwrap();
		}
	}
};
class MatrixEngine;