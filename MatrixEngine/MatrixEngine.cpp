#include "MatrixEngine.h"
#include "pch.h"


class MatrixEngine
{
public:
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	const char* Title;
	int width;
	int height;
	SDL_WindowFlags flags;
	StatusCode currStatus;
	VkInstance instance;
	VkSurfaceKHR surface;
	VkDevice device;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkQueue graphicsQueue;
private:
	PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)SDL_Vulkan_GetVkGetInstanceProcAddr();

public:
	MatrixEngine(const char* Title, int width, int height, SDL_WindowFlags flags) {
		if (InitializeMatrixEngine(Title, width, height, flags) >= 0)
			currStatus = Running;
		this->Title = Title;
		this->width = width;
		this->height = height;
		this->flags = flags;

	}

	~MatrixEngine()
	{
		QuitMatrixEngine();
	}

private:
	StatusCode InitializeMatrixEngine(const char* Title, int width, int height, SDL_WindowFlags flags) {
		SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);

#pragma region LoadVulkan
		if (!SDL_Vulkan_LoadLibrary(NULL))
			std::cout << "Vulkan Load Failure " << SDL_GetError() << std::endl;
		else
			std::cout << "Vulkan Loaded Successfully" << std::endl;
#pragma endregion

#pragma region InstanceExtensions
		Uint32 InstanceExtensionCount;
		auto ExtNames = SDL_Vulkan_GetInstanceExtensions(&InstanceExtensionCount);
		if (!ExtNames) {
			std::cout << "Vulkan Instance Extension Load Failure " << SDL_GetError() << std::endl;
			return Fail;
		}
		else {
			std::cout << "Vulkan Instance Extensions Loaded Successfully " << InstanceExtensionCount << '\n';
			for (size_t i = 0; i < InstanceExtensionCount; i++) {
				std::cout << '\t' << i + 1 << " " << ExtNames[i] << '\n';
			}
			std::cout << std::flush;
		}
#pragma endregion

#pragma region app_info
		VkApplicationInfo app_info = {};
		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pNext = NULL;
		app_info.pApplicationName = Title;
		app_info.applicationVersion = 1;
		app_info.pEngineName = "Matrix";
		app_info.engineVersion = 0;
		app_info.apiVersion = VK_API_VERSION_1_0;
#pragma endregion

#pragma region inst_info
		VkInstanceCreateInfo inst_info = {};
		inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		inst_info.pNext = NULL;
		inst_info.flags = 0;
		inst_info.pApplicationInfo = &app_info;
		inst_info.enabledExtensionCount = InstanceExtensionCount;
		inst_info.ppEnabledExtensionNames = ExtNames;
		inst_info.enabledLayerCount = 0;
		inst_info.ppEnabledLayerNames = NULL;
#pragma endregion

#pragma region CreateInstance
		VkResult res;

		res = vkCreateInstance(&inst_info, NULL, &instance);

		if (res == VK_ERROR_INCOMPATIBLE_DRIVER) {
			std::cout << "cannot find a compatible Vulkan ICD\n";
			exit(Fail);
		}
		else if (res) {
			std::cout << "unknown error\n";
			exit(unknownFail);
		}
#pragma endregion

#pragma region Createwindow
		window = SDL_CreateWindow(Title, width, height, flags);

		if (!window) {
			return Fail;
		}
#pragma endregion

#pragma region CreateSurface
		if (!SDL_Vulkan_CreateSurface(window, instance, NULL, &surface)) {
			std::cout << "Surface Creation Failed\n\t" << SDL_GetError() << std::endl;
			return Fail;
		}
#pragma endregion

#pragma region vulkan_setup

		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		VkPhysicalDevice* devices = MatrixAlloc<VkPhysicalDevice>(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices);

		QueueFamilyIndices indices = QueueFamilyIndices();
		for (size_t idx = 0; idx < deviceCount; idx++) {
			const auto& device = devices[idx];
			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(device, &deviceProperties);
			VkPhysicalDeviceFeatures features;
			vkGetPhysicalDeviceFeatures(device, &features);

			uint32_t queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

			VkQueueFamilyProperties* queueFamilies = MatrixAlloc<VkQueueFamilyProperties>(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies);
			int i = 0;
			for (size_t idx = 0; idx < queueFamilyCount; idx++) {
				const auto& queueFamily = queueFamilies[idx];
				if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
					indices.append(idx);
				}
			}

			bool suitability = deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && features.geometryShader && indices.valid;
			std::cout << deviceProperties.deviceName << "\nis Suitable:\t\t" << suitability;
			if (suitability) {
				physicalDevice = device;
				break;
			}
		}
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = indices.first();
		queueCreateInfo.queueCount = 1;
		float queuePriority = 1.0;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		VkPhysicalDeviceFeatures deviceFeatures{};
		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = &queueCreateInfo;
		createInfo.queueCreateInfoCount = 1;

		createInfo.pEnabledFeatures = &deviceFeatures;
		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
			throw std::runtime_error("failed to create logical device!");
		}
		vkGetDeviceQueue(device, indices.first(), 0, &graphicsQueue);


		// TODO: Setup The Swapchain
		// TODO: Setup The Command Buffers
		// TODO: Setup The Render Passes
#pragma endregion


		return Success;
	}

	void QuitMatrixEngine() {
		vkDestroyDevice(device, nullptr);
		vkDestroySurfaceKHR(instance, surface, NULL);
		vkDestroyInstance(instance, NULL);
		SDL_Vulkan_UnloadLibrary();
		SDL_Quit();
	}


};

