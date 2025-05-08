#pragma once
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>
#include "SerialMessage.hpp"
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
void handleGetRA_DEC_HDMS(AsyncWebServerRequest *request);	   // GET http://localhost:3000/get_RA_DEC_HDMS
void handleGetRA_DEC_Float(AsyncWebServerRequest *request);	   // GET http://localhost:3000/get_RA_DEC_Float
void handleGetConfig(AsyncWebServerRequest *request);		   // GET http://localhost:3000/get_config
void handleGetStatus(AsyncWebServerRequest *request);		   // GET http://localhost:3000/get_status
void handleGetEfuseMac(AsyncWebServerRequest *request);		   // GET http://localhost:3000/get_EfuseMac
void handleGetTiltFusion(AsyncWebServerRequest *request);	   // GET http://localhost:3000/get_TiltFusion
void handleGetGPS(AsyncWebServerRequest *request);			   // GET http://localhost:3000/get_gps
void handleGetChipDiagnostics(AsyncWebServerRequest *request); // GET http://localhost:3000/get_ChipDiagnostics
void handleGetSystemStatus(AsyncWebServerRequest *request);	   // GET http://localhost:3000/get_SystemStatus
// HTTP_POST
void handleSetStatus(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);	   // POST http://localhost:3000/set_status
void handleSetConfig(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);	   // POST http://localhost:3000/set_config
void handleSetRA_DEC_Float(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total); // POST http://localhost:3000/set_RA_DEC_Float
void handleSetRA_DEC_HDMS(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);  // POST http://localhost:3000/set_RA_DEC_HDMS
void handleSetGPS(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);		   // POST http://localhost:3000/set_gps
void handleSetTime(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);		   // POST http://localhost:3000/set_time
// void handleCalcMagneticDeclination(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total); // POST http://localhost:3000/calc_MagneticDeclination

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
	if (!SPIFFS.begin(true))
	{
		Serial0_Println("An Error has occurred while mounting SPIFFS");
		return;
	}
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

	// Get RA and DEC in HDMS format
	server.on("/get_RA_DEC_HDMS", HTTP_GET, handleGetRA_DEC_HDMS);
	// Get RA and DEC in float format
	server.on("/get_RA_DEC_Float", HTTP_GET, handleGetRA_DEC_Float);
	// Get Config
	server.on("/get_config", HTTP_GET, handleGetConfig);
	// Get Status
	server.on("/get_status", HTTP_GET, handleGetStatus);
	// Get EfuseMac
	server.on("/get_EfuseMac", HTTP_GET, handleGetEfuseMac);
	// Get TiltFusion MPU6050
	server.on("/get_TiltFusion", HTTP_GET, handleGetTiltFusion);
	// Get GPS
	server.on("/get_gps", HTTP_GET, handleGetGPS);
	// Get Chip Diagnostics
	server.on("/get_ChipDiagnostics", HTTP_GET, handleGetChipDiagnostics);
	// Get System Status
	server.on("/get_SystemStatus", HTTP_GET, handleGetSystemStatus);
	// Register POST API
	server.on("/set_status", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, handleSetStatus);
	server.on("/set_config", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, handleSetConfig);
	server.on("/set_RA_DEC_Float", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, handleSetRA_DEC_Float);
	server.on("/set_RA_DEC_HDMS", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, handleSetRA_DEC_HDMS);
	server.on("/set_gps", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, handleSetGPS);
	// server.on("/calc_MagneticDeclination", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, handleCalcMagneticDeclination);

	// POST API
	// server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
	// 					 {

	// 	Serial0_Printf(">> Body received for: %s\n", request->url().c_str());
	// 	Serial0_Printf(">> Content-Type: %s\n", request->contentType().c_str());
	// 	Serial0_Printf(">> Method: %s\n", request->method() == HTTP_POST ? "POST" : "OTHER");

	//     if(request->method()==HTTP_POST && request->contentType()=="application/json"){
	//         if(request->url()=="/set_status"){
	//             handleSetStatus(request,data,len,index,total);
	//         }else if(request->url()=="/set_config"){
	//             handleSetConfig(request,data,len,index,total);
	// 		}else if(request->url()=="/set_RA_DEC_Float"){
	//             handleSetRA_DEC_Float(request,data,len,index,total);
	// 		}else if(request->url()=="/set_RA_DEC_HDMS"){
	//             handleSetRA_DEC_HDMS(request,data,len,index,total);
	// 		}else if(request->url()=="/set_gps"){
	//             handleSetGPS(request,data,len,index,total);
	// 		// }else if (request->url()=="/calc_MagneticDeclination"){
	// 		// 	handleCalcMagneticDeclination(request,data,len,index,total);
	// 		}else{
	//             request->send(404,"text/plain","Unknown POST endpoint");
	//         }
	//     }
	// 	else{
	// 		request->send(415,"text/plain","Unsupported content type");
	// 	} });
	Serial0_Println("register Web Server Event Finished!");
}

// HTTP_GET
void handleGetRA_DEC_HDMS(AsyncWebServerRequest *request) // GET http://localhost:3000/get_RA_DEC_HDMS
{
	File RA_DEC_File = SPIFFS.open("/RA_DEC_Float.json", "r");
	JsonDocument RA_DEC_Json;
	DeserializationError RA_DEC_Fileerror = deserializeJson(RA_DEC_Json, RA_DEC_File);
	if (RA_DEC_Fileerror)
	{
		request->send(400, "text/plain", "Invalid JSON on SPIFFS");
		return;
	}
	RA_DEC_File.close();
	// make resp json object
	double ra = RA_DEC_Json["ra"];
	double dec = RA_DEC_Json["dec"];
	int ra_h, ra_m, dec_d, dec_m;
	double ra_s, dec_s;
	degrees_to_hms(ra, ra_h, ra_m, ra_s);
	degrees_to_dms(dec, dec_d, dec_m, dec_s);
	JsonDocument respJson;
	respJson["ra_h"] = ra_h;
	respJson["ra_m"] = ra_m;
	respJson["ra_s"] = ra_s;
	respJson["dec_d"] = dec_d;
	respJson["dec_m"] = dec_m;
	respJson["dec_s"] = dec_s;
	String response;
	serializeJson(respJson, response);
	request->send(200, "application/json", response);
}
void handleGetRA_DEC_Float(AsyncWebServerRequest *request) // GET http://localhost:3000/get_RA_DEC_Float
{
	File RA_DEC_File = SPIFFS.open("/RA_DEC_Float.json", "r");
	JsonDocument RA_DEC_Json;
	DeserializationError RA_DEC_Fileerror = deserializeJson(RA_DEC_Json, RA_DEC_File);
	if (RA_DEC_Fileerror)
	{
		request->send(400, "text/plain", "Invalid JSON on SPIFFS");
		return;
	}
	RA_DEC_File.close();
	// make resp json object
	JsonDocument respJson;
	respJson["ra"] = RA_DEC_Json["ra"];
	respJson["dec"] = RA_DEC_Json["dec"];

	String response;
	serializeJson(respJson, response);
	request->send(200, "application/json", response);
}
void handleGetConfig(AsyncWebServerRequest *request) // GET http://localhost:3000/get_config
{
	File configFile = SPIFFS.open("/Config.json", "r");
	JsonDocument configJson;
	DeserializationError configFileerror = deserializeJson(configJson, configFile);
	if (configFileerror)
	{
		request->send(400, "text/plain", "Invalid JSON on SPIFFS");
		return;
	}
	configFile.close();

	// make resp json object
	JsonDocument respJson;
	respJson["ssid"] = configJson["ssid"];
	respJson["pwd"] = configJson["pwd"];
	respJson["ratio"] = configJson["ratio"];
	String response;
	serializeJson(respJson, response);
	request->send(200, "application/json", response);
}
void handleGetStatus(AsyncWebServerRequest *request) // GET http://localhost:3000/get_status
{
	// make resp json object
	File statusFile = SPIFFS.open("/Status.json", "r");
	JsonDocument statusJson;
	DeserializationError statusFileerror = deserializeJson(statusJson, statusFile);
	if (statusFileerror)
	{
		request->send(400, "text/plain", "Invalid JSON on SPIFFS");
		return;
	}
	statusFile.close();
	// Construct the response
	JsonDocument respJson;
	respJson["t"] = millis() / 1000;
	JsonObject innerObjectS = respJson["s"].to<JsonObject>();
	innerObjectS["d"] = statusJson["d"];
	innerObjectS["t"] = statusJson["t"];
	innerObjectS["s"] = statusJson["s"];
	String response;
	serializeJson(respJson, response);
	request->send(200, "application/json", response);
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
void handleGetTiltFusion(AsyncWebServerRequest *request) // GET http://localhost:3000/get_TiltFusion
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
	}else // This should not happen.....
	{
		request->send(503, "text/plain", "No MPU data");
	}
}
void handleGetGPS(AsyncWebServerRequest *request) // GET http://localhost:3000/get_gps
{
	File gpsFile = SPIFFS.open("/GPS.json", "r");
	JsonDocument gpsJson;
	DeserializationError configFileerror = deserializeJson(gpsJson, gpsFile);
	if (configFileerror)
	{
		request->send(400, "text/plain", "Invalid JSON on SPIFFS");
		return;
	}
	gpsFile.close();

	// make resp json object
	JsonDocument respJson;
	respJson["lon_d"] = gpsJson["lon_d"];
	respJson["lon_m"] = gpsJson["lon_m"];
	respJson["lon_s"] = gpsJson["lon_s"];
	respJson["lat_d"] = gpsJson["lat_d"];
	respJson["lat_m"] = gpsJson["lat_m"];
	respJson["lat_s"] = gpsJson["lat_s"];
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
void handleGetSystemStatus(AsyncWebServerRequest *request) // GET http://localhost:3000/get_SystemStatus
{
	JsonDocument respJson;

	// System Uptime
	respJson["uptime_ms"] = millis();

	// Usage of heap
	respJson["heap"]["total_bytes"] = ESP.getHeapSize();
	respJson["heap"]["free_bytes"] = ESP.getFreeHeap();
	respJson["heap"]["min_free_bytes"] = esp_get_minimum_free_heap_size();

	//  RTOS Tick and Number of task
	respJson["rtos"]["tick_rate_hz"] = configTICK_RATE_HZ;
	respJson["rtos"]["task_count"] = uxTaskGetNumberOfTasks();

	// Stack Status of Each Task
	/*
	!Warning Builtin Precompiled ESP-IDF NOT Support!
	Enable these flags under build_flags in platformio.ini
		-DconfigGENERATE_RUN_TIME_STATS=1
		-DconfigUSE_STATS_FORMATTING_FUNCTIONS=1
		-DconfigUSE_TRACE_FACILITY=1
	*/
	// UBaseType_t taskCount = uxTaskGetNumberOfTasks();
	// TaskStatus_t *status_array = (TaskStatus_t *)pvPortMalloc(taskCount * sizeof(TaskStatus_t));
	// if (status_array)
	// {
	// 	taskCount = uxTaskGetSystemState(status_array, taskCount, NULL);
	// 	JsonArray taskArr = respJson["tasks"].to<JsonArray>();

	// 	for (UBaseType_t i = 0; i < taskCount; i++)
	// 	{
	// 		JsonObject taskObj = taskArr.add<JsonObject>();
	// 		taskObj["name"] = status_array[i].pcTaskName;
	// 		taskObj["stack_remaining_bytes"] = status_array[i].usStackHighWaterMark * sizeof(StackType_t);
	// 		taskObj["priority"] = status_array[i].uxCurrentPriority;
	// 	}
	// 	vPortFree(status_array);
	// }

	// CPU Time status
	/*
	!Warning Builtin Precompiled ESP-IDF NOT Support!
	Enable these flags under build_flags in platformio.ini
		-DconfigGENERATE_RUN_TIME_STATS=1
		-DconfigUSE_STATS_FORMATTING_FUNCTIONS=1
		-DconfigUSE_TRACE_FACILITY=1
	*/

	// #if (configGENERATE_RUN_TIME_STATS == 1)
	// 	char *stats_buffer = (char *)pvPortMalloc(1024);
	// 	if (stats_buffer)
	// 	{
	// 		vTaskGetRunTimeStats(stats_buffer);
	// 		respJson["rtos"]["runtime_stats"] = stats_buffer;
	// 		vPortFree(stats_buffer);
	// 	}
	// 	else
	// 	{
	// 		respJson["rtos"]["runtime_stats"] = "malloc failed";
	// 	}
	// #else
	// 	respJson["rtos"]["runtime_stats"] = "disabled";
	// #endif

	String response;
	serializeJson(respJson, response);
	request->send(200, "application/json", response);
}
// HTTP_POST
void handleSetStatus(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) // POST http://localhost:3000/set_status
{
	// check req json validation
	JsonDocument reqJson;
	DeserializationError reqJsonerror = deserializeJson(reqJson, data);
	if (reqJsonerror)
	{
		request->send(400, "text/plain", "Invalid JSON");
		return;
	}
	int d = reqJson["d"];
	int t = reqJson["t"];
	float s = reqJson["s"];

	// Write status to SPIFFS
	File statusFile = SPIFFS.open("/Status.json", "w");
	serializeJson(reqJson, statusFile);
	statusFile.close();

	// Calculate pulse_count for stepper motor!!!
	uint32_t pulse_count = Pulse_DEC(123.456); // just one fake number to test

	// For Example if you want to call task_MOVE_RA and task_Move_DEC, you can do things like these.
	// cmd from deserializeJson
	/*
	MoveCommand raCmd = {
		.action = ACTION_CONTINUOUS,
		.dir = DIR_WORK,
		.pulse_count = 0,	// under ACTION_CONTINUOUS, the pulse_count is meaningless
		.delay_us = 7355608 // just one fake number to test
	};
	JsonDocument respJson;
	if (!Stepper_RA_TrySendCommand(&raCmd))
	{
		respJson["status"] = "RA move started";
	}
	else
	{
		respJson["status"] = "RA motor is already running";
	}
	String response;
	serializeJson(respJson, response);
	request->send(200, "application/json", response);

	MoveCommand decCmd = {
		.action = ACTION_POSITION,
		.dir = DIR_WORK,
		.pulse_count = pulse_count, // Just like use the real pulse_count from Pulse_DEC();
		.delay_us = 7355608			// under ACTION_POSITION, the delay_us is meaningless
	};
	JsonDocument respJson;
	if (!Stepper_RA_TrySendCommand(&decCmd))
	{
		respJson["status"] = "DEC move started";
	}
	else
	{
		respJson["status"] = "DEC motor is already running";
	}
	String response;
	serializeJson(respJson, response);
	request->send(200, "application/json", response);
	*/

	// For Example if you want to stop stepper, you can do things like these.
	// cmd from deserializeJson
	/*
		if cmd.action==ACTION_STOP
			Stepper_RA_Stop();
			Stepper_RA_Stop();
	*/

	// set coordinate content to 0.0 and 0.0
	File coordinateFile = SPIFFS.open("/Coordinate.json", "w");
	JsonDocument coordinateJson;
	coordinateJson["azimuth"] = 0.0;
	coordinateJson["altitude"] = 0.0;
	serializeJson(coordinateJson, coordinateFile);
	coordinateFile.close();
	// make resp json object
	JsonDocument respJson;
	respJson["status"] = "OK";
	String response;
	serializeJson(respJson, response);
	request->send(200, "application/json", response);
}
void handleSetConfig(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) // POST http://localhost:3000/set_config
{
	// check req json validation
	JsonDocument reqJson;
	DeserializationError reqJsonerror = deserializeJson(reqJson, data);
	if (reqJsonerror)
	{
		request->send(400, "text/plain", "Invalid JSON");
		return;
	}

	String ap_ssid = reqJson["ssid"];
	String ap_pwd = reqJson["pwd"];
	int ratio = reqJson["ratio"];
	//! Write your logic here
	// Write Config to SPIFFS
	File configFile = SPIFFS.open("/Config.json", "w");
	serializeJson(reqJson, configFile);
	configFile.close();
	//! Warning: never setting ssid as empty string
	WiFi_AP_Reboot(ap_ssid, ap_pwd);
	// make resp json object
	JsonDocument respJson;
	respJson["status"] = "OK";
	String response;
	serializeJson(respJson, response);
	request->send(200, "application/json", response);
}
void handleSetRA_DEC_Float(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) // POST http://localhost:3000/set_RA_DEC_Float
{
	JsonDocument reqJson;
	DeserializationError reqJsonerror = deserializeJson(reqJson, data);
	if (reqJsonerror)
	{
		request->send(400, "text/plain", "Invalid JSON");
		return;
	}

	double ra = reqJson["ra"];
	double dec = reqJson["dec"];
	// Write RA and DEC to SPIFFS
	File RA_DEC_File = SPIFFS.open("/RA_DEC_Float.json", "w");
	serializeJson(reqJson, RA_DEC_File);
	RA_DEC_File.close();
	// make resp json object
	JsonDocument respJson;
	respJson["status"] = "OK";
	String response;
	serializeJson(respJson, response);
	request->send(200, "application/json", response);
}
void handleSetRA_DEC_HDMS(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
// POST http://localhost:3000/set_RA_DEC_HDMS
{
	JsonDocument reqJson;
	DeserializationError reqJsonerror = deserializeJson(reqJson, data);
	if (reqJsonerror)
	{
		request->send(400, "text/plain", "Invalid JSON");
		return;
	}
	int ra_h, ra_m, dec_d, dec_m;
	double ra_s, dec_s;
	ra_h = reqJson["ra_h"];
	ra_m = reqJson["ra_m"];
	ra_s = reqJson["ra_s"];
	dec_d = reqJson["dec_d"];
	dec_m = reqJson["dec_m"];
	dec_s = reqJson["dec_s"];

	double ra = hms_to_hours(ra_h, ra_m, ra_s);
	double dec = dms_to_degrees(dec_d, dec_m, dec_s);

	// Write RA and DEC to SPIFFS
	File RA_DEC_File = SPIFFS.open("/RA_DEC_Float.json", "w");
	serializeJson(reqJson, RA_DEC_File);
	RA_DEC_File.close();
	// make resp json object
	JsonDocument respJson;
	respJson["status"] = "OK";
	String response;
	serializeJson(respJson, response);
	request->send(200, "application/json", response);
}
void handleSetGPS(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) // POST http://localhost:3000/set_gps
{
	JsonDocument reqJson;
	DeserializationError reqJsonerror = deserializeJson(reqJson, data);
	if (reqJsonerror)
	{
		request->send(400, "text/plain", "Invalid JSON");
		return;
	}

	int lon_d, lon_m, lat_d, lat_m;
	double lon_s, lat_s;

	lon_d = reqJson["lon_d"];
	lon_m = reqJson["lon_m"];
	lon_s = reqJson["lon_s"];
	lat_d = reqJson["lat_d"];
	lat_m = reqJson["lat_m"];
	lat_s = reqJson["lat_s"];
	// Write RA and DEC to SPIFFS
	File RA_DEC_File = SPIFFS.open("/GPS.json", "w");
	serializeJson(reqJson, RA_DEC_File);
	RA_DEC_File.close();
	// make resp json object
	JsonDocument respJson;
	respJson["status"] = "OK";
	String response;
	serializeJson(respJson, response);
	request->send(200, "application/json", response);
}

// void handleCalcMagneticDeclination(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) // POST http://localhost:3000/calc_MagneticDeclination
// {
// 	JsonDocument reqJson;
// 	DeserializationError reqJsonerror = deserializeJson(reqJson, data);
// 	if (reqJsonerror)
// 	{
// 		request->send(400, "text/plain", "Invalid JSON");
// 		return;
// 	}

// 	double lat, lon, alt, year;

// 	lat = reqJson["lat"];
// 	lon = reqJson["lon"];
// 	alt = reqJson["alt"];
// 	year = reqJson["year"];

// 	if (load_cof("/WMMHR.COF") != 0)
// 	{
// 		request->send(500, "text/plain", "Failed to load WMMHR.COF");
// 		return;
// 	}
// 	Serial0_Println("Cal Declination");
// 	double decl = calc_declination(lat, lon, alt, year);

// 	// make resp json object
// 	JsonDocument respJson;
// 	respJson["decl"] = decl;
// 	Serial0_Printf("Magnetic declination (deg): %f\n", decl);
// 	String response;
// 	serializeJson(respJson, response);
// 	request->send(200, "application/json", response);
// }
