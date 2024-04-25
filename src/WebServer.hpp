#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>
#include "SerialMessage.hpp"

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
	DeserializationError error = deserializeJson(reqJson, data);
	if (error)
	{
		request->send(400, "text/plain", "Invalid JSON");
		return;
	}
	// make resp json object
	JsonDocument respJson;
	respJson["t"] = 999;
	JsonObject innerObjectS = respJson["s"].to<JsonObject>();
	innerObjectS["d"] = 2;
	innerObjectS["t"] = 444;
	innerObjectS["s"] = 12.3;
	String response;
	serializeJson(respJson, response);
	request->send(200, "application/json", response);
}
void handleGetConfig(AsyncWebServerRequest *request, uint8_t *data) // POST http://localhost:3000/config
{
	// check req json validation
	JsonDocument reqJson;
	DeserializationError error = deserializeJson(reqJson, data);
	if (error)
	{
		request->send(400, "text/plain", "Invalid JSON");
		return;
	}
	// make resp json object
	JsonDocument respJson;
	respJson["ssid"] = "SSID";
	respJson["pwd"] = "PWD";
	respJson["ratio"] = 123;
	String response;
	serializeJson(respJson, response);
	request->send(200, "application/json", response);
}
void handleSetStatus(AsyncWebServerRequest *request, uint8_t *data) // POST http://localhost:3000/set_status
{
	// check req json validation
	JsonDocument reqJson;
	DeserializationError error = deserializeJson(reqJson, data);
	if (error)
	{
		request->send(400, "text/plain", "Invalid JSON");
		return;
	}
	int d = reqJson["d"];
	int t = reqJson["t"];
	float s = reqJson["s"];
	//! Write your logic here
	// TODO: set stepper motor work method
	digitalWrite(Pin_Stepper_Motor_Dir, Stepper_Motor_Initialize_Dir);
	ledcSetup(Stepper_Motor_Channel, Stepper_Motor_Freq, Stepper_Motor_resolution);
	ledcWrite(Stepper_Motor_Channel, Stepper_Motor_dutyCycle);
	ledcAttachPin(Pin_Stepper_Motor_Step, Stepper_Motor_Channel);

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
	DeserializationError error = deserializeJson(reqJson, data);
	if (error)
	{
		request->send(400, "text/plain", "Invalid JSON");
		return;
	}
	String ssid = reqJson["ssid"];
	String pwd = reqJson["pwd"];
	int ratio = reqJson["ratio"];
	//! Write your logic here

	// make resp json object
	JsonDocument respJson;
	respJson["status"] = "OK";
	String response;
	serializeJson(respJson, response);
	request->send(200, "application/json", response);
}