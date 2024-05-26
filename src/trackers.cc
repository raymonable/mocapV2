#include "trackers.h"

std::map<TrackedDeviceIndex_t, std::string> deviceAliases;
std::vector<TrackedDevice*> devices;
IVRSystem* iVR;

void TrackedDevice::init() {
	this->name = (char*)malloc(256);
	iVR->GetStringTrackedDeviceProperty(this->idx, ETrackedDeviceProperty::Prop_ControllerType_String, this->name, 256);

	if (deviceAliases.find(this->idx) != deviceAliases.end()) {
		this->alias = (char*)deviceAliases[this->idx].c_str();
	}
	else
		this->alias = nullptr;
};

// TrackerSystem

namespace TrackerSystem {
	void assignAliasIfExists(ETrackedControllerRole deviceType, std::string alias) {
		TrackedDeviceIndex_t idx = iVR->GetTrackedDeviceIndexForControllerRole(deviceType);
		if (idx != k_unTrackedDeviceIndexInvalid)
			deviceAliases[idx] = alias;
	}
};

void TrackerSystem::setupTrackers() {
	for (TrackedDevice* device : devices)
		delete device;
	devices.clear();
	deviceAliases.clear();

	// Assign Left and Right hands (if they can be found, at least)
	assignAliasIfExists(ETrackedControllerRole::TrackedControllerRole_LeftHand, "left_hand");
	assignAliasIfExists(ETrackedControllerRole::TrackedControllerRole_RightHand, "right_hand");

	if (iVR->IsTrackedDeviceConnected(k_unTrackedDeviceIndex_Hmd))
		devices.push_back(new TrackedDevice{ k_unTrackedDeviceIndex_Hmd });

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
int TrackerSystem::init() {
	HmdError error;
	iVR = VR_Init(&error, VRApplication_Background);

	if (error != VRInitError_None) {
		std::cout << "An error occurred with OpenVR." << std::endl;
		switch (error) {
		case VRInitError_Init_NoServerForBackgroundApp:
			MessageBoxA(NULL, "Error", "Please ensure SteamVR is running and try again.", MB_OK);
			break;
		default:
			MessageBoxA(NULL, "Error", ((std::string)"An error occurred when trying to initialize SteamVR SDK\n" + std::to_string(error)).c_str(), MB_OK);
		}
		return 1;
	}

	return 0;
};
std::vector<TrackedDevice*> TrackerSystem::getTrackers() {
	return devices;
}