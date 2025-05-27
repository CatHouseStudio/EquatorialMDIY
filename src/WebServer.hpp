#pragma once
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>
#include "SerialMessage.hpp"
#include "SpiffsService.hpp"
#include "WiFiApSta.hpp"
#include "CelestialPositioning.hpp"
// #include "GPSInfo.hpp"
#include "CelestialStepper.hpp"
#include "I2CWorker.hpp"
// #include "MagneticDeclination.hpp"
static AsyncWebServer server(80);
unsigned long ota_progress_millis = 0;

class LoggingMiddleware : public AsyncMiddleware
{
public:
	void run(AsyncWebServerRequest *request, ArMiddlewareNext next) override
	{
		// 获取基本信息
		String method = request->methodToString();
		String url = request->url();
		String clientIP = request->client()->remoteIP().toString();
		uint8_t version = request->version();
		unsigned long start = millis();
		// Log tig time
		unsigned long ms = millis();
		unsigned long sec = ms / 1000;
		unsigned long min = sec / 60;
		unsigned long hour = min / 60;
		Serial0_Printf("[%02lu:%02lu:%02lu.%03lu]  [REQ] %s %s from %s HTTP/1.%d\n", hour % 24, min % 60, sec % 60, ms % 1000, method.c_str(), url.c_str(), clientIP.c_str(), version);

		// 执行下一个中间件或最终 handler
		next();

		// handler 之后，打印响应耗时和状态码
		unsigned long duration = millis() - start;
		ms = millis();
		sec = ms / 1000;
		min = sec / 60;
		hour = min / 60;
		if (request->getResponse() != nullptr)
		{
			int code = request->getResponse()->code();
			Serial0_Printf("[%02lu:%02lu:%02lu.%03lu]  [RES] %s -> %d in %lums\n", hour % 24, min % 60, sec % 60, ms % 1000, url.c_str(), code, duration);
		}
		else
		{
			Serial0_Printf("[%02lu:%02lu:%02lu.%03lu]  [RES] %s -> unknown response in %lums\n", hour % 24, min % 60, sec % 60, ms % 1000, url.c_str(), duration);
		}
	}
};

void onOTAStart();
void onOTAProgress(size_t current, size_t final);
void onOTAEnd(bool success);

void WebServerEvent();
// Server API events
// HTTP_GET
void handleGetStatus(AsyncWebServerRequest *request);		// GET http://localhost:3000/api/get_status
void handleGetAPConfig(AsyncWebServerRequest *request);		// GET http://localhost:3000/api/get_ap_config
void handleGetRatioConfig(AsyncWebServerRequest *request);	// GET http://localhost:3000/api/get_ratio_config
void handleGetTiltFusion(AsyncWebServerRequest *request);	// GET http://localhost:3000/api/get_TiltFusion
void handleGetSystemStatus(AsyncWebServerRequest *request); // GET http://localhost:3000/api/get_SystemStatus
void handleStartTracking(AsyncWebServerRequest *request);	// GET http://localhost:3000/api/start_tracking
void handleStopMoving(AsyncWebServerRequest *request);		// GET http://localhost:3000/api/stop_moving
void handleGetMotorStatus(AsyncWebServerRequest *request);	// GET http://localhost:3000/api/get_motot_status

//!! API document Need fix!!
void handleGetChipDiagnostics(AsyncWebServerRequest *request); // GET http://localhost:3000/get_ChipDiagnostics
void handleGetEfuseMac(AsyncWebServerRequest *request);		   // GET http://localhost:3000/get_EfuseMac
// HTTP_POST
void handleSetAPConfig(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);			  // POST http://localhost:3000/api/set_ap_config
void handleSetRatioConfig(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);		  // POST http://localhost:3000/api/set_ratio_config
void handlePluseToTarget(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);		  // POST http://localhost:3000/api/pluse_to_target
void handleSetCurrentMotorPostion(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total); // POST http://localhost:3000/api/set_current_motor_position

void onOTAStart()
{
	// Log when OTA has started
	Serial0_Println("OTA update started!");
	// <Add your own code here>
}

void onOTAProgress(size_t current, size_t final)
{
	// Log every 1 second
	if (millis() - ota_progress_millis > 1000)
	{
		Serial0_Printf("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
	}
}

void onOTAEnd(bool success)
{
	// Log when OTA has finished
	if (success)
	{
		Serial0_Println("OTA update finished successfully!");
	}
	else
	{
		Serial0_Println("There was an error during OTA update!");
	}
	// <Add your own code here>
}

/*
	You can use the way to enable spa
	Click the PIO icon at the left side bar. The project tasks should open.
	Select env:esp32doit-devkit-v1 (it may be slightly different depending on the board you’re using).
	Expand the Platform menu.
	Select Build Filesystem Image.
	Finally, click Upload Filesystem Image.
*/
void WebServerEvent()
{
	Serial0_Println("registering Web Server Event");
	server.addMiddleware(new LoggingMiddleware());
	// Route to load static resource
	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
			  { request->send(SPIFFS, "/index.html", "text/html"); });
	// Route to load style.css file
	//! you must set the right name
	server.on("/main.equatorial.css", HTTP_GET, [](AsyncWebServerRequest *request)
			  { request->send(SPIFFS, "/main.equatorial.css", "text/css"); });
	server.on("/main.equatorial.js", HTTP_GET, [](AsyncWebServerRequest *request)
			  { request->send(SPIFFS, "/main.equatorial.js", "application/javascript"); });
	server.on("/asset-manifest.json", HTTP_GET, [](AsyncWebServerRequest *request)
			  { request->send(SPIFFS, "/asset-manifest.json", "application/json"); });
	// Get Status
	server.on("/api/get_status", HTTP_GET, handleGetStatus);
	// Get AP Config
	server.on("/api/get_ap_config", HTTP_GET, handleGetAPConfig);
	// Get Ratio Config
	server.on("/api/get_ratio_config", HTTP_GET, handleGetRatioConfig);
	// Get TiltFusion MPU6050
	server.on("/api/get_TiltFusion", HTTP_GET, handleGetTiltFusion);
	// Get System Status
	server.on("/get_SystemStatus", HTTP_GET, handleGetSystemStatus);
	// Start Tracking
	server.on("/api/start_tracking", HTTP_GET, handleStartTracking);
	// Stop Moving
	server.on("/api/stop_moving", HTTP_GET, handleStopMoving);
	// Get Motor Status
	server.on("/api/get_motor_status", HTTP_GET, handleGetMotorStatus);

	// Get EfuseMac
	server.on("/get_EfuseMac", HTTP_GET, handleGetEfuseMac);
	// Get Chip Diagnostics
	server.on("/get_ChipDiagnostics", HTTP_GET, handleGetChipDiagnostics);

	// Register POST API
	server.on("/api/set_ap_config", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, handleSetAPConfig);
	server.on("/api/set_ratio_config", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, handleSetRatioConfig);
	server.on("/api/pluse_to_target", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, handlePluseToTarget);
	server.on("/api/set_current_motor_position", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, handleSetCurrentMotorPostion);

	Serial0_Println("register Web Server Event Finished!");
}

// HTTP_GET
void handleGetStatus(AsyncWebServerRequest *request) // GET http://localhost:3000/api/get_status
{
	request->send(500, "application/json", "API not Implement!!!");
}
void handleGetAPConfig(AsyncWebServerRequest *request) // GET http://localhost:3000/get_config
{
	JsonDocument configJson;
	bool ok = ReadJsonFromFile(fs_path_config, configJson);
	if (ok)
	{
		// make resp json object
		JsonDocument respJson;
		respJson["ssid"] = configJson["ssid"];
		respJson["pwd"] = configJson["pwd"];
		String response;
		serializeJson(respJson, response);
		request->send(200, "application/json", response);
	}
	else
	{
		request->send(400, "text/plain", "Invalid JSON on SPIFFS");
		return;
	}
}
void handleGetRatioConfig(AsyncWebServerRequest *request) // GET http://localhost:3000/api/get_ratio_config
{
	request->send(500, "application/json", "API Deprecated!!!");
}
void handleGetTiltFusion(AsyncWebServerRequest *request) // GET http://localhost:3000/api/get_TiltFusion
{
	MPUResult mpuResult;
	if (GetLatestMPU(mpuResult))
	{

		JsonDocument respJson;
		respJson["roll"] = mpuResult.roll;
		respJson["pitch"] = mpuResult.pitch;
		respJson["ztilt"] = mpuResult.ztilt;
		String response;
		serializeJson(respJson, response);
		request->send(200, "application/json", response);
	}
	else // This should not happen.....
	{
		request->send(503, "text/plain", "No MPU data");
	}
}
void handleGetSystemStatus(AsyncWebServerRequest *request) // GET http://localhost:3000/api/get_SystemStatus
{
	request->send(500, "application/json", "API not Implement!!!");
}
void handleStartTracking(AsyncWebServerRequest *request) // GET http://localhost:3000/api/start_tracking
{
	request->send(500, "application/json", "API not Implement!!!");
}
void handleStopMoving(AsyncWebServerRequest *request) // GET http://localhost:3000/api/stop_moving
{
	Stepper_RA_Stop();
	Stepper_DEC_Stop();
	JsonDocument respJson;
	respJson["status"] = "stopped";
	String response;
	serializeJson(respJson, response);
	request->send(200, "application/json", response);
}
void handleGetMotorStatus(AsyncWebServerRequest *request) // GET http://localhost:3000/api/get_motot_status
{
	request->send(500, "application/json", "API not Implement!!!");
}

void handleGetEfuseMac(AsyncWebServerRequest *request) // GET http://localhost:3000/get_EfuseMac
{
	uint64_t chipId = ESP.getEfuseMac();
	char chipIdStr[17];
	sprintf(chipIdStr, "%012llX", chipId);
	JsonDocument respJson;
	respJson["EfuseMac"] = chipIdStr;
	String response;
	serializeJson(respJson, response);
	request->send(200, "application/json", response);
}
void handleGetChipDiagnostics(AsyncWebServerRequest *request) // GET http://localhost:3000/get_ChipDiagnostics
{
	esp_chip_info_t chip_info;
	esp_chip_info(&chip_info);

	const char *model = "Unknown";
	switch (chip_info.model)
	{
	case CHIP_ESP32:
		model = "ESP32";
		break;
	case CHIP_ESP32S2:
		model = "ESP32-S2";
		break;
	case CHIP_ESP32S3:
		model = "ESP32-S3";
		break;
	case CHIP_ESP32C3:
		model = "ESP32-C3";
		break;
	case CHIP_ESP32H2:
		model = "ESP32-H2";
		break;
	default:
		model = "Unknown";
		break;
	}
	JsonDocument respJson;
	respJson["chip"]["model"] = model;
	respJson["chip"]["cores"] = chip_info.cores;
	respJson["chip"]["revision"] = chip_info.revision;
	respJson["chip"]["psram"] = psramFound();
	respJson["chip"]["features"]["wifi"] = (bool)(chip_info.features & CHIP_FEATURE_WIFI_BGN);
	respJson["chip"]["features"]["bt"] = (bool)(chip_info.features & CHIP_FEATURE_BT);
	respJson["chip"]["features"]["ble"] = (bool)(chip_info.features & CHIP_FEATURE_BLE);
	respJson["clock"]["cpu_mhz"] = getCpuFrequencyMhz();
	respJson["flash"]["size_mb"] = spi_flash_get_chip_size() / (1024 * 1024);
	String response;
	serializeJson(respJson, response);
	request->send(200, "application/json", response);
}
// HTTP_POST
void handleSetAPConfig(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) // POST http://localhost:3000/api/set_ap_config
{
	// check req json validation
	JsonDocument reqJson;
	DeserializationError reqJsonerror = deserializeJson(reqJson, data);
	if (reqJsonerror)
	{
		request->send(400, "text/plain", "Invalid JSON");
		return;
	}
	else
	{
		String ap_ssid = reqJson["ssid"];
		String ap_pwd = reqJson["pwd"];
		JsonDocument respJson;
		String response;
		if (ap_ssid.equals("")) // system error, ssid should not be empty
		{
			request->send(400, "text/plain", "ap_ssid should not be empty!");
		}
		else
		{
			//! Warning: never setting ssid as empty string
			bool ok = WriteJsonToFile(fs_path_config, reqJson);
			// make resp json object
			respJson["status"] = "OK";
			serializeJson(respJson, response);
			request->send(200, "application/json", response);
			WiFi_AP_Reboot(ap_ssid, ap_pwd);
		}
	}
}
void handleSetRatioConfig(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) // POST http://localhost:3000/api/set_ratio_config
{
	request->send(500, "application/json", "API Deprecated!!!");
}
void handlePluseToTarget(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) // POST http://localhost:3000/api/pluse_to_target
{
	request->send(500, "application/json", "API not Implement!!!");
}
void handleSetCurrentMotorPostion(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) // POST http://localhost:3000/api/set_current_motor_position
{
	request->send(500, "application/json", "API not Implement!!!");
}