#pragma once
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>
#include "SerialMessage.hpp"
#include "WiFiApSta.hpp"
#include "CelestialPositioning.hpp"

static AsyncWebServer server(80);
unsigned long ota_progress_millis = 0;

void onOTAStart();
void onOTAProgress(size_t current, size_t final);
void onOTAEnd(bool success);

void WebServerEvent();
// Server API events
void handleGetStatus(AsyncWebServerRequest *request, uint8_t *data); // POST http://localhost:3000/status
void handleGetConfig(AsyncWebServerRequest *request, uint8_t *data); // POST http://localhost:3000/config
void handleSetStatus(AsyncWebServerRequest *request, uint8_t *data); // POST http://localhost:3000/set_status
void handleSetConfig(AsyncWebServerRequest *request, uint8_t *data); // POST http://localhost:3000/set_config

void onOTAStart()
{
	// Log when OTA has started
	Serial.println("OTA update started!");
	// <Add your own code here>
}

void onOTAProgress(size_t current, size_t final)
{
	// Log every 1 second
	if (millis() - ota_progress_millis > 1000)
	{
		ota_progress_millis = millis();
		Serial.printf("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
	}
}

void onOTAEnd(bool success)
{
	// Log when OTA has finished
	if (success)
	{
		Serial.println("OTA update finished successfully!");
	}
	else
	{
		Serial.println("There was an error during OTA update!");
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
	server.on("/main.98185c89.css", HTTP_GET, [](AsyncWebServerRequest *request)
			  { request->send(SPIFFS, "/main.98185c89.css", "text/css"); });
	server.on("/main.b11ab86c.js", HTTP_GET, [](AsyncWebServerRequest *request)
			  { request->send(SPIFFS, "/main.b11ab86c.js", "application/javascript"); });
	server.on("/main.c051389f.js", HTTP_GET, [](AsyncWebServerRequest *request)
			  { request->send(SPIFFS, "/main.c051389f.js", "application/javascript"); });
	server.on("/asset-manifest.json", HTTP_GET, [](AsyncWebServerRequest *request)
			  { request->send(SPIFFS, "/asset-manifest.json", "application/json"); });

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

	// * I use "d" as direction and "s" as frequency

	switch (d)
	{
	case Stepper_Status_Clockwise:
		digitalWrite(Pin_Stepper_Motor_Dir, Stepper_Motor_Initialize_Dir);
		ledcSetup(Stepper_Motor_Channel, s, Stepper_Motor_resolution);
		ledcWrite(Stepper_Motor_Channel, Stepper_Motor_dutyCycle);
		break;
	case Stepper_Status_CounterClockwise:
		digitalWrite(Pin_Stepper_Motor_Dir, Stepper_Motor_Work_Dir);
		ledcSetup(Stepper_Motor_Channel, s, Stepper_Motor_resolution);
		ledcWrite(Stepper_Motor_Channel, Stepper_Motor_dutyCycle);
		break;
	case Stepper_Status_Stop:
		ledcWrite(Stepper_Motor_Channel, 0); // set duty cycle to zero as stop PWM output
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