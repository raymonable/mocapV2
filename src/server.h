#pragma once

#include <httplib.h>
#include "rapidjson/prettywriter.h"

using namespace rapidjson;
using namespace httplib;

#define DECIMAL_PRECISION 100000000
const int frameDataLength = (1024 * 1024) / 2;
std::vector<std::string> lastCaptureData;

enum WebServerType {
	HTTP, HTTPS
};
class WebServer {
public:
	WebServer(WebServerType t) : type(t) {
		switch (type) {
		case HTTP: {
			server = new Server();
			this->init<Server*>(std::any_cast<Server*>(this->server));
			break;
		}
		case HTTPS:
			// TODO...
			break;
		};
	}
	template <typename T>
	void init(T www) {
		www->Get("/capture/begin", [&](const Request& req, Response& res) {
			this->capture = new CaptureSystem{};
			res.set_content("OK", "text/plain");
		});
		www->Get("/capture/fetch", [&](const Request& req, Response& res) {
			if (!req.has_param("page"))
				res.set_content("Invalid page", "text/plain");
			int page = std::stoi(req.get_param_value("page")) - 1;
			if (lastCaptureData.size() <= page)
				return res.set_content("Invalid page", "text/plain");
			res.set_content(lastCaptureData[page].c_str(), "text/plain");
		});
		www->Get("/capture/end", [&](const Request& req, Response& res) {
			if (!this->capture)
				return res.set_content("{\"error\":\"No capture\"}", "application/json");
			this->capture->end();

			StringBuffer response;
			Writer<StringBuffer> writer(response);

			writer.StartObject();
			writer.String("length");
			writer.Double(this->capture->frames.rbegin()->first);
			writer.String("frames");
			writer.StartArray();

			for (auto const& [frameIdx, frameData] : this->capture->frames)
			{
				writer.StartObject();
				writer.String("frame");
				writer.Double(frameIdx);
				writer.String("data");
				writer.StartArray();
				for (auto const& [device, cframe] : frameData) {
					writer.StartObject();
					writer.String("device");
					writer.Int(device->idx);
					writer.String("cframe");
					writer.StartArray();
					for (size_t idx = 0; 12 > idx; idx++)
						writer.Double((double)((int)(cframe.m[idx] * DECIMAL_PRECISION)) / DECIMAL_PRECISION);
					writer.EndArray();
					writer.EndObject();
				}
				writer.EndArray();
				writer.EndObject();
			}

			writer.EndArray();
			writer.EndObject();

			std::string captureData = response.GetString();
			lastCaptureData = std::vector<std::string>{};
			lastCaptureData.resize(std::ceil((double)captureData.size() / (double)frameDataLength));
			for (size_t i = 0; lastCaptureData.size() > i; i += 1) {
				char* data = (char*)malloc(frameDataLength + 1);
				size_t len = frameDataLength;
				if (len + (i * frameDataLength) > captureData.size())
					len = captureData.size() - (i * frameDataLength);
				memset(data, '\0', len + 1);
				memcpy(data, captureData.c_str() + (i * frameDataLength), len);

				lastCaptureData[i] = std::string(data);
			};

			{
				StringBuffer response;
				PrettyWriter<StringBuffer> writer(response);
				writer.StartObject();
				writer.String("count");
				writer.Int(lastCaptureData.size());
				writer.EndObject();
				res.set_content(response.GetString(), "application/json");
			}

			delete this->capture;
			this->capture = nullptr;
		});
		www->Get("/trackers", [&](const Request& req, Response& res) {
			StringBuffer response;
			PrettyWriter<StringBuffer> writer(response);

			writer.StartObject();
			writer.String("trackers");
			writer.StartArray();

			std::vector<TrackedDevice*> trackers = TrackerSystem::getTrackers();
			for (auto tracker : trackers) {
				writer.StartObject();
				writer.String("name");
				writer.String(tracker->name);

				if (tracker->alias != nullptr) {
					writer.String("alias");
					writer.String(tracker->alias);
				}

				writer.String("index");
				writer.Int(tracker->idx);
				
				writer.EndObject();
			};

			writer.EndArray();
			writer.EndObject();

			res.set_content(response.GetString(), "application/json");
		});
		www->Get("/trackers/live", [&](const Request& req, Response& res) {
			StringBuffer response;
			PrettyWriter<StringBuffer> writer(response);

			std::vector<TrackedDevicePose_t> poses;
			poses.resize(k_unMaxTrackedDeviceCount);
			iVR->GetDeviceToAbsoluteTrackingPose(ETrackingUniverseOrigin::TrackingUniverseStanding, 0, poses.data(), k_unMaxTrackedDeviceCount);

			writer.StartObject();
			writer.String("trackers");
			writer.StartArray();

			std::vector<TrackedDevice*> trackers = TrackerSystem::getTrackers();
			for (auto tracker : trackers) {
				writer.StartObject();
				writer.String("name");
				writer.String(tracker->name);

				if (tracker->alias != nullptr) {
					writer.String("alias");
					writer.String(tracker->alias);
				}

				writer.String("cframe");
				writer.StartArray();

				CFrame trackerCFrame = tracker->get(poses);
				for (size_t idx = 0; 12 > idx; idx++)
					writer.Double(trackerCFrame.m[idx]);

				writer.EndArray();
				writer.EndObject();
			}

			writer.EndArray();
			writer.EndObject();

			poses.clear();

			res.set_content(response.GetString(), "application/json");
		});
		www->Get("/trackers/refresh", [&](const Request& req, Response& res) {
			TrackerSystem::setupTrackers();
			res.set_content("OK", "text/plain");
		});
		www->listen("0.0.0.0", this->type == WebServerType::HTTP ? 80 : 443, 0);
	};
private:
	WebServerType type;
	std::any server;
	CaptureSystem* capture;
};
