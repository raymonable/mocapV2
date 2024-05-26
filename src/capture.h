#include "lib.h"

double freq = 0;
int64_t counterStart = 0;

double getCurrentTimeInMilliseconds() {
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	return double(li.QuadPart - counterStart) / freq;
}

class CaptureSystem {
public:
	CaptureSystem() {
		this->captureThread = std::thread(&CaptureSystem::startCapture, this, this);
	}
	~CaptureSystem() {
		// just so it doesn't shit itself
		this->captureThread.join();
	}
	void startCapture(CaptureSystem* system) {
		
		// Initialize timer
		LARGE_INTEGER li;
		if (!QueryPerformanceFrequency(&li))
			MessageBoxA(NULL, "An error occurred. Please try again.", "An error occurred.", MB_ICONERROR | MB_OK);
		freq = double(li.QuadPart) / 1000.0;
		QueryPerformanceCounter(&li);
		counterStart = li.QuadPart;

		double  previousTick = 0;
		double  baseTick = getCurrentTimeInMilliseconds();

		std::vector<TrackedDevice*> trackers = TrackerSystem::getTrackers();
		std::vector<TrackedDevicePose_t> poses;

		poses.resize(k_unMaxTrackedDeviceCount);

		while (system->isCapturing) {
			double  currentTick = getCurrentTimeInMilliseconds();
			if ((currentTick - previousTick) > ((double)1000 / (double)60)) {
				double  currentFrame = (currentTick - baseTick);
				frames[currentFrame] = {};
				
				iVR->GetDeviceToAbsoluteTrackingPose(ETrackingUniverseOrigin::TrackingUniverseStanding, 0, poses.data(), k_unMaxTrackedDeviceCount);
				for (auto tracker : trackers)
					frames[currentFrame][tracker] = tracker->get(poses);

				previousTick = currentTick;
			}
		}

		poses.clear();
	}
	void end() {
		isCapturing = false;
	};
	bool isCapturing = true;
	std::map<double, std::map<TrackedDevice*, CFrame>> frames;
private:
	std::thread captureThread;
};