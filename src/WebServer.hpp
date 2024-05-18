#pragma once
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>
#include "SerialMessage.hpp"
#include "WiFiApSta.hpp"
#include "CelestialPositioning.hpp"
#include "GPSInfo.hpp"
#include "CelestialStepper.hpp"
static AsyncWebServer server(80);
unsigned long ota_progress_millis = 0;

void onOTAStart();
void onOTAProgress(size_t current, size_t final);
void onOTAEnd(bool success);

void WebServerEvent();
// Server API events
void handleGetStatus(AsyncWebServerRequest *request, uint8_t *data);	// POST http://localhost:3000/status
void handleGetConfig(AsyncWebServerRequest *request, uint8_t *data);	// POST http://localhost:3000/config
void handleSetStatus(AsyncWebServerRequest *request, uint8_t *data);	// POST http://localhost:3000/set_status
void handleSetConfig(AsyncWebServerRequest *request, uint8_t *data);	// POST http://localhost:3000/set_config
void handleMoveRelative(AsyncWebServerRequest *request, uint8_t *data); // POST http://localhost:3000/move_relative
void handleMoveAbsolute(AsyncWebServerRequest *request, uint8_t *data); // POST http://localhost:3000/move_absolute

void onOTAStart()
{
	// Log when OTA has started
	xQueueSend(queueHandle_Serial0, &"OTA update started!", (TickType_t)0);

	// <Add your own code here>
}

void onOTAProgress(size_t current, size_t final)
{
	// Log every 1 second
	if (millis() - ota_progress_millis > 1000)
	{
		char message[Max_Message_Length];
		ota_progress_millis = millis();
		snprintf(message, Max_Message_Length, "OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
		xQueueSend(queueHandle_Serial0, &message, (TickType_t)0);
	}
}

void onOTAEnd(bool success)
{
	// Log when OTA has finished
	if (success)
	{
		xQueueSend(queueHandle_Serial0, &"OTA update finished successfully!", (TickType_t)0);
	}
	else
	{
		xQueueSend(queueHandle_Serial0, &"There was an error during OTA update!", (TickType_t)0);
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
	xQueueSend(queueHandle_Serial0, &"register Web Server Event", (TickType_t)0);
	if (!SPIFFS.begin(true))
	{
		xQueueSend(queueHandle_Serial0, &"An Error has occurred while mounting SPIFFS", (TickType_t)0);
		return;
	}
	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
			  { request->send(SPIFFS, "/index.html", "text/html"); });
	// Route to load style.css file
	//! you must set the right name
	server.on("/main.98185c89.css", HTTP_GET, [](AsyncWebServerRequest *request)
			  { request->send(SPIFFS, "/main.98185c89.css", "text/css"); });
	server.on("/main.b11ab86c.js", HTTP_GET, [](AsyncWebServerRequest *request)
			  { request->send(SPIFFS, "/main.b11ab86c.js", "application/javascript"); });
	server.on("/main.c051389f.js", HTTP_GET, [](AsyncWebServerRequest *request)
			  { request->send(SPIFFS, "/main.c051389f.js", "application/javascript"); });
	server.on("/asset-manifest.json", HTTP_GET, [](AsyncWebServerRequest *request)
			  { request->send(SPIFFS, "/asset-manifest.json", "application/json"); });
	// Get Stepper Coordinate(azimuth and altitude) from SPIFFS
	server.on("/get_coordinate", HTTP_GET, [](AsyncWebServerRequest *request)
			  { File coordinateFile = SPIFFS.open("/Coordinate.json", "r");
	JsonDocument coordinateJson;
	DeserializationError coordinateFileerror = deserializeJson(coordinateJson, coordinateFile);
	if (coordinateFileerror)
	{
		request->send(400, "text/plain", "Invalid JSON on SPIFFS");
		return;
	}
	coordinateFile.close();

	// make resp json object
	JsonDocument respJson;
	respJson["azimuth"] = coordinateJson["azimuth"];
	respJson["altitude"] = coordinateJson["altitude"];
	String response;
	serializeJson(respJson, response);
	request->send(200, "application/json", response); });

	// Server api
	// server.on("/buzz", HTTP_GET, [](AsyncWebServerRequest *request)
	//           { request->send(200, "application/json", "{\"message\":\"Buzz play Little Star\"}");
	//             vTaskResume(xHandleBuzzPlayLittleStar);
	//           });

	server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
						 {
        if(request->method()==HTTP_POST && request->contentType()=="application/json"){
            if(request->url()=="/status"){
                handleGetStatus(request,data);
            }else if(request->url()=="/config"){
                handleGetConfig(request,data);
            }else if(request->url()=="/set_status"){
                handleSetStatus(request,data);
            }else if(request->url()=="/set_config"){
                handleSetStatus(request,data);
            }else if(request->url()=="/move_relative"){
				handleMoveRelative(request,data);
			}else if(request->url()=="/move_absolute"){
				handleMoveAbsolute(request,data);
			}
            else{
                request->send(500);
            }
        } });
}
void handleGetStatus(AsyncWebServerRequest *request, uint8_t *data) // POST http://localhost:3000/status
{
	// check req json validation
	JsonDocument reqJson;
	DeserializationError reqJsonError = deserializeJson(reqJson, data);
	if (reqJsonError)
	{
		request->send(400, "text/plain", "Invalid JSON");
		return;
	}
	// make resp json object

	File statusFile = SPIFFS.open("/status.json", "r");
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
void handleGetConfig(AsyncWebServerRequest *request, uint8_t *data) // POST http://localhost:3000/config
{
	// check req json validation
	JsonDocument reqJson;
	DeserializationError reqJsonerror = deserializeJson(reqJson, data);
	if (reqJsonerror)
	{
		request->send(400, "text/plain", "Invalid JSON");
		return;
	}
	File configFile = SPIFFS.open("/config.json", "r");
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
	respJson["ssid"] = configJson["SSID"];
	respJson["pwd"] = configJson["pwd"];
	respJson["ratio"] = configJson["ratio"];
	String response;
	serializeJson(respJson, response);
	request->send(200, "application/json", response);
}
void handleSetStatus(AsyncWebServerRequest *request, uint8_t *data) // POST http://localhost:3000/set_status
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
	File statusFile = SPIFFS.open("/status.json", "w");
	serializeJson(reqJson, statusFile);
	statusFile.close();

	//! Write your logic here
	// TODO: set stepper motor work method
	/* Calculate azimuth and altitude here
	 if (xSemaphoreTake(semphr_gps_info_Mutex, portMAX_DELAY) == pdTRUE) {
		gpsinfo.相关信息
		// void CalculatePosition(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second,
		//                float longitude, float latitude, // 示例经度和纬度：121.93, 30.9
		//                float raHours, float decDegrees, // 示例北极星的赤经和赤纬：2.9667, 89.25
		//                float &azimuth, float &altitude);// 方位角和高度角
		CalculatePosition(相关参数，最后两个参数是方位角和高度角)
		xSemaphoreGive(semphr_gps_info_Mutex);

		// 生成和发送响应
		// 例如：sprintf(response, "Latitude: %f, Longitude: %f", lat, lon);
		// sendResponse(response);
	}

	*/

	// set coordinate content to 0.0 and 0.0
	File coordinateFile = SPIFFS.open("/Coordinate.json", "w");
	JsonDocument coordinateJson;
	coordinateJson["azimuth"] = 0.0;
	coordinateJson["altitude"] = 0.0;
	serializeJson(coordinateJson, coordinateFile);
	coordinateFile.close();

	// * I use "d" as direction and "s" as frequency
	// Attach the PWM OUTPUT
	ledcAttachPin(Pin_Stepper_Equator_Step, Stepper_Equator_Channel);
	switch (d)
	{
	case Stepper_Equator_Status_Clockwise:
		digitalWrite(Pin_Stepper_Equator_Dir, Stepper_Equator_Initialize_Dir);
		ledcSetup(Stepper_Equator_Channel, s, Stepper_Equator_resolution);
		ledcWrite(Stepper_Equator_Channel, Stepper_Equator_dutyCycle);
		break;
	case Stepper_Equator_Status_CounterClockwise:
		digitalWrite(Pin_Stepper_Equator_Dir, Stepper_Equator_Work_Dir);
		ledcSetup(Stepper_Equator_Channel, s, Stepper_Equator_resolution);
		ledcWrite(Stepper_Equator_Channel, Stepper_Equator_dutyCycle);
		break;
	case Stepper_Equator_Status_Stop:
		ledcDetachPin(Pin_Stepper_Equator_Step);
		ledcWrite(Stepper_Equator_Channel, 0); // set duty cycle to zero as stop PWM output
		break;
	default:
		//! What happen?
		break;
	}

	// make resp json object
	JsonDocument respJson;
	respJson["status"] = "OK";
	String response;
	serializeJson(respJson, response);
	request->send(200, "application/json", response);
}
void handleSetConfig(AsyncWebServerRequest *request, uint8_t *data) // POST http://localhost:3000/set_config
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
	File configFile = SPIFFS.open("/config.json", "w");
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

void handleMoveRelative(AsyncWebServerRequest *request, uint8_t *data) // POST http://localhost:3000/move_relative
{

	// make sure Celestial Stepper is IDLE
	if ((xTaskHandle_Move_Horizontal != NULL && eTaskGetState(xTaskHandle_Move_Horizontal) != eDeleted) ||
		(xTaskHandle_Move_Vertical != NULL && eTaskGetState(xTaskHandle_Move_Vertical) != eDeleted))
	{
		// make resp json object
		JsonDocument notIDLEJson;
		notIDLEJson["status"] = "Celestial Stepper is Working!";
		String response;
		serializeJson(notIDLEJson, response);
		request->send(200, "application/json", response);
	}

	JsonDocument reqJson;
	DeserializationError reqJsonerror = deserializeJson(reqJson, data);
	if (reqJsonerror)
	{
		request->send(400, "text/plain", "Invalid JSON");
		return;
	}
	float req_azimuth = reqJson["azimuth"];
	float req_altitude = reqJson["altitude"];

	File coordinateFileR = SPIFFS.open("/Coordinate.json", "r");
	JsonDocument coordinateRJson;
	DeserializationError coordinateFileRerror = deserializeJson(coordinateRJson, coordinateFileR);
	if (coordinateFileRerror)
	{
		request->send(400, "text/plain", "Invalid JSON on SPIFFS");
		return;
	}
	coordinateFileR.close();

	// float az = coordinateRJson["azimuth"].as<float>();
	// float azimuth = az + req_azimuth;

	// float al = coordinateRJson["altitude"].as<float>();
	// float altitude = al + req_altitude;
	// Write new coordinate to SPIFFS
	File coordinateFileW = SPIFFS.open("/Coordinate.json", "w");
	JsonDocument coordinateWJson;
	coordinateWJson["azimuth"] = req_azimuth;
	coordinateWJson["altitude"] = req_altitude;
	serializeJson(coordinateWJson, coordinateFileW);
	coordinateFileW.close();

	// Call CelestialStepper
	ledcDetachPin(Pin_Stepper_Equator_Step);
	int64_t pluse_hor = Pluse_Horizontal(req_azimuth);
	int64_t pluse_vec = Pluse_Vertical(req_altitude);

	xTaskCreate(task_Move_Horizontal, "Move Horizontal", configMINIMAL_STACK_SIZE + 8192, (void *)pluse_hor, configMAX_PRIORITIES - 3, &xTaskHandle_Move_Horizontal);
	xTaskCreate(task_Move_Vertical, "Move Vertical", configMINIMAL_STACK_SIZE + 8192, (void *)pluse_vec, configMAX_PRIORITIES - 3, &xTaskHandle_Move_Vertical);

	// make resp json object
	JsonDocument respJson;
	respJson["status"] = "OK";
	String response;
	serializeJson(respJson, response);
	request->send(200, "application/json", response);
}
void handleMoveAbsolute(AsyncWebServerRequest *request, uint8_t *data) // POST http://localhost:3000/move_absolute
{
	// make sure Celestial Stepper is IDLE
	if ((xTaskHandle_Move_Horizontal != NULL && eTaskGetState(xTaskHandle_Move_Horizontal) != eDeleted) ||
		(xTaskHandle_Move_Vertical != NULL && eTaskGetState(xTaskHandle_Move_Vertical) != eDeleted))
	{
		// make resp json object
		JsonDocument notIDLEJson;
		notIDLEJson["status"] = "Celestial Stepper is Working!";
		String response;
		serializeJson(notIDLEJson, response);
		request->send(200, "application/json", response);
	}

	JsonDocument reqJson;
	DeserializationError reqJsonerror = deserializeJson(reqJson, data);
	if (reqJsonerror)
	{
		request->send(400, "text/plain", "Invalid JSON");
		return;
	}
	float req_azimuth = reqJson["azimuth"];
	float req_altitude = reqJson["altitude"];

	File coordinateFileR = SPIFFS.open("/Coordinate.json", "r");
	JsonDocument coordinateRJson;
	DeserializationError coordinateFileRerror = deserializeJson(coordinateRJson, coordinateFileR);
	if (coordinateFileRerror)
	{
		request->send(400, "text/plain", "Invalid JSON on SPIFFS");
		return;
	}
	coordinateFileR.close();

	float az = coordinateRJson["azimuth"].as<float>();
	float azimuth = req_azimuth - az;

	float al = coordinateRJson["altitude"].as<float>();
	float altitude = req_altitude - al;
	// Write new coordinate to SPIFFS
	File coordinateFileW = SPIFFS.open("/Coordinate.json", "w");
	JsonDocument coordinateWJson;
	coordinateWJson["azimuth"] = azimuth;
	coordinateWJson["altitude"] = altitude;
	serializeJson(coordinateWJson, coordinateFileW);
	coordinateFileW.close();

	// Call CelestialStepper
	ledcDetachPin(Pin_Stepper_Equator_Step);
	int64_t pluse_hor = Pluse_Horizontal(azimuth);
	int64_t pluse_vec = Pluse_Vertical(altitude);

	xTaskCreate(task_Move_Horizontal, "Move Horizontal", configMINIMAL_STACK_SIZE + 8192, (void *)pluse_hor, configMAX_PRIORITIES - 3, &xTaskHandle_Move_Horizontal);
	xTaskCreate(task_Move_Vertical, "Move Vertical", configMINIMAL_STACK_SIZE + 8192, (void *)pluse_vec, configMAX_PRIORITIES - 3, &xTaskHandle_Move_Vertical);

	// make resp json object
	JsonDocument respJson;
	respJson["status"] = "OK";
	String response;
	serializeJson(respJson, response);
	request->send(200, "application/json", response);
}
