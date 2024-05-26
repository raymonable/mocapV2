#include "lib.h"
#include <openvr.h>

using namespace vr;

struct CFrame {
	float m[12];
};
const double CFRAME_MULTIPLIER = 3;

struct TrackedDevice {
	void init();
	CFrame get(std::vector<TrackedDevicePose_t> poses);
	TrackedDevice(TrackedDeviceIndex_t i) : idx(i) {
		init();
	};
	TrackedDevice(TrackedDeviceIndex_t i, std::string a) : idx(i) {
		init();

		alias = (char*)malloc(a.length() + 1);
		memcpy(alias, a.c_str(), a.length() + 1);
	}
	char* name = nullptr;
	char* alias = nullptr;
	TrackedDeviceIndex_t idx;
};

std::map<TrackedDeviceIndex_t, std::string> deviceAliases;
std::vector<TrackedDevice*> devices;
IVRSystem* iVR;

void TrackedDevice::init() {
	this->name = (char*)malloc(256);
	iVR->GetStringTrackedDeviceProperty(this->idx, ETrackedDeviceProperty::Prop_ControllerType_String, this->name, 256);

	if (std::string(name).length() <= 0)
	{
		this->name = nullptr;
		return;
	}

	if (deviceAliases.find(this->idx) != deviceAliases.end()) {
		this->alias = (char*)deviceAliases[this->idx].c_str();
	}
	else
		this->alias = nullptr;
};
CFrame TrackedDevice::get(std::vector<TrackedDevicePose_t> poses) {
	CFrame cf;
	HmdMatrix34_t matrix = poses[this->idx].mDeviceToAbsoluteTracking;

	for (size_t idx = 0; 3 > idx; idx++)
		cf.m[idx] = matrix.m[idx][3] * CFRAME_MULTIPLIER;
	for (size_t idx = 0; 3 > idx; idx++)
		for (size_t g_idx = 0; 3 > g_idx; g_idx++)
			cf.m[(idx * 3) + g_idx + 3] = matrix.m[idx][g_idx];
	
	return cf;
};

// TrackerSystem

namespace TrackerSystem {
	void assignAliasIfExists(ETrackedControllerRole deviceType, std::string alias) {
		TrackedDeviceIndex_t idx = iVR->GetTrackedDeviceIndexForControllerRole(deviceType);
		if (idx != k_unTrackedDeviceIndexInvalid)
			deviceAliases[idx] = alias;
	}
	void setupTrackers() {
		for (TrackedDevice* device : devices)
			delete device;
		devices.clear();
		deviceAliases.clear();

		// Assign Left and Right hands (if they can be found, at least)
		assignAliasIfExists(ETrackedControllerRole::TrackedControllerRole_LeftHand, "left_hand");
		assignAliasIfExists(ETrackedControllerRole::TrackedControllerRole_RightHand, "right_hand");

		if (iVR->IsTrackedDeviceConnected(k_unTrackedDeviceIndex_Hmd))
			devices.push_back(new TrackedDevice{ k_unTrackedDeviceIndex_Hmd, "head"});

		// Assign all other trackers
		for (TrackedDeviceIndex_t idx = 1; idx < k_unMaxTrackedDeviceCount; idx++) {
			if (iVR->IsTrackedDeviceConnected(idx)) {
				TrackedDevice* device = new TrackedDevice(idx);
				if (!device->name)
					delete device;
				else
					devices.push_back(device);
			}
		}
	};
	int init() {
		HmdError error;
		iVR = VR_Init(&error, VRApplication_Background);

		if (error != VRInitError_None) {
			std::cout << "An error occurred with OpenVR." << std::endl;
			switch (error) {
			case VRInitError_Init_NoServerForBackgroundApp:
				MessageBoxA(NULL, "Please ensure SteamVR is running and try again.", "Error", MB_OK);
				break;
			default:
				MessageBoxA(NULL, ((std::string)"An error occurred when trying to initialize SteamVR SDK\n" + std::to_string(error)).c_str(), "Error", MB_OK);
			}
			return 1;
		}
		setupTrackers();

		return 0;
	};
	std::vector<TrackedDevice*> getTrackers() {
		return devices;
	}
};