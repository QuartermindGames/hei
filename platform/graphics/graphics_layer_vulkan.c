/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org>
*/

#if defined( PL_SUPPORT_VULKAN )

#include <vulkan/vulkan.h>

#include "graphics_private.h"

#define PRINT_VAR_VERSION( VERSION ) GfxLog( PL_TOSTRING( VERSION ) ": %d.%d.%d\n", VK_VERSION_MAJOR( VERSION ), VK_VERSION_MINOR( VERSION ), VK_VERSION_PATCH( VERSION ) )
#define PRINT_VAR_INTEGER( INTEGER ) GfxLog( PL_TOSTRING( INTEGER ) ": %d\n", INTEGER )

static VkInstance vk_instance = NULL;
static VkPhysicalDevice vk_physicalDevice = VK_NULL_HANDLE;

static VkExtensionProperties *vk_extensionProperties = NULL;
static unsigned int vk_numExtensions = 0;

static VkInstance VK_CreateInstance( void ) {
	static const VkApplicationInfo appInfo = {
			.apiVersion = VK_API_VERSION_1_2,
			.applicationVersion = VK_MAKE_VERSION( PL_VERSION_MAJOR, PL_VERSION_MINOR, PL_VERSION_PATCH ),
			.engineVersion = VK_MAKE_VERSION( PL_VERSION_MAJOR, PL_VERSION_MINOR, PL_VERSION_PATCH ),
			.pApplicationName = "libplatform",
			.pEngineName = "libplatform",
			.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO, /* why? ... */
			.pNext = NULL,
	};
	static const VkInstanceCreateInfo createInfo = {
			.pNext = NULL,
			.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.enabledExtensionCount = 0,
			.enabledLayerCount = 0,
			.flags = 0,
			.pApplicationInfo = &appInfo,
			.ppEnabledExtensionNames = NULL,
			.ppEnabledLayerNames = NULL,
	};

	GfxLog( "Creating Vulkan instance...\n" );

	VkInstance instance;
	if( vkCreateInstance( &createInfo, NULL, &instance ) != VK_SUCCESS ) {
		ReportError( PL_RESULT_GRAPHICSINIT, "failed to create Vulkan instance" );
		return NULL;
	}

	GfxLog( "Vulkan instance created!\n" );

	return instance;
}

/**
 * Iterate through and find the best possible device.
 */
static VkPhysicalDevice VK_SelectPhysicalDevice( void ) {
	GfxLog( "Fetching physical devices...\n" );

	unsigned int numDevices;
	vkEnumeratePhysicalDevices( vk_instance, &numDevices, NULL );
	if ( numDevices == 0 ) {
		ReportError( PL_RESULT_GRAPHICSINIT, "no supported physical devices available" );
		return NULL;
	}

	GfxLog( "Found %d graphics devices, selecting appropriate device...\n", numDevices );

	VkPhysicalDevice *physicalDevices = pl_malloc( sizeof( VkPhysicalDevice ) * numDevices );
	vkEnumeratePhysicalDevices( vk_instance, &numDevices, physicalDevices );

	VkPhysicalDeviceProperties properties;
	for ( unsigned int i = 0; i < numDevices; ++i ) {
		if ( i == 0 ) {
			vk_physicalDevice = physicalDevices[ i ];
			vkGetPhysicalDeviceProperties( vk_physicalDevice, &properties );
			continue;
		}

		/* compare this device and the device we'd like to use */
		VkPhysicalDeviceProperties newProps;
		vkGetPhysicalDeviceProperties( physicalDevices[ i ], &newProps );
		if ( newProps.limits.maxImageDimension2D > properties.limits.maxImageDimension2D ) {
			vk_physicalDevice = physicalDevices[ i ];
			properties = newProps;
		}
	}

	GfxLog( "Selected device, \"%s\"\n", properties.deviceName );
	PRINT_VAR_VERSION( properties.apiVersion );
	PRINT_VAR_VERSION( properties.driverVersion );
	PRINT_VAR_INTEGER( properties.deviceID );
	PRINT_VAR_INTEGER( properties.vendorID );

	pl_free( physicalDevices );

	return vk_physicalDevice;
}

void plInitVulkan( void ) {
	FunctionStart();

	/* attempt to create the vulkan instance */
	vk_instance = VK_CreateInstance();
	if ( vk_instance == NULL ) {
		return;
	}

	/* fetch a physical device */
	vk_physicalDevice = VK_SelectPhysicalDevice();
	if ( vk_physicalDevice == NULL ) {
		return;
	}

	/* query vulkan extensions */
	vkEnumerateInstanceExtensionProperties( NULL, &vk_numExtensions, NULL );
	vk_extensionProperties = pl_malloc( sizeof( VkExtensionProperties ) * vk_numExtensions );
	vkEnumerateInstanceExtensionProperties( NULL, &vk_numExtensions, vk_extensionProperties );
	GfxLog( "  extensions:\n" );
	for( unsigned int i = 0; i < vk_numExtensions; ++i ) {
		GfxLog( "    %s\n", vk_extensionProperties[ i ].extensionName );
	}
}

void plShutdownVulkan( void ) {
	pl_free( vk_extensionProperties );
	vk_numExtensions = 0;

	if ( vk_instance != NULL ) {
		vkDestroyInstance( vk_instance, NULL );
		vk_instance = NULL;
	}
}

#else

void plInitVulkan( void ) {}
void plShutdownVulkan( void ) {}

#endif /* PL_SUPPORT_VULKAN */
