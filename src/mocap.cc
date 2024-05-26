#include "lib.h"
#include "trackers.h"
#include "capture.h"
#include "server.h"

int WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmdLine, int nShow) {
	// Initialize OpenVR
	if (TrackerSystem::init() > 0)
		return 1;

	// Start up web server
	WebServer* server = new WebServer{HTTP};

	return 0;
}