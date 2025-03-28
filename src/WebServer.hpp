#pragma once
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>
#include "SerialMessage.hpp"
#include "WiFiApSta.hpp"
#include "CelestialPositioning.hpp"
// #include "GPSInfo.hpp"
#include "CelestialStepper.hpp"
static AsyncWebServer server(80);
unsigned long ota_progress_millis = 0;

void onOTAStart();
void onOTAProgress(size_t current, size_t final);
void onOTAEnd(bool success);

void WebServerEvent();
// Server API events
void handleSetStatus(AsyncWebServerRequest *request, uint8_t *data);	   // POST http://localhost:3000/set_status
void handleSetConfig(AsyncWebServerRequest *request, uint8_t *data);	   // POST http://localhost:3000/set_config
void handleSetRA_DEC_Float(AsyncWebServerRequest *request, uint8_t *data); // POST http://localhost:3000/set_RA_DEC_Float
void handleSetRA_DEC_HDMS(AsyncWebServerRequest *request, uint8_t *data);  // POST http://localhost:3000/set_RA_DEC_HDMS

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
	server.on("/main.equatorial.css", HTTP_GET, [](AsyncWebServerRequest *request)
			  { request->send(SPIFFS, "/main.equatorial.css", "text/css"); });
	server.on("/main.equatorial.js", HTTP_GET, [](AsyncWebServerRequest *request)
			  { request->send(SPIFFS, "/main.equatorial.js", "application/javascript"); });
	server.on("/asset-manifest.json", HTTP_GET, [](AsyncWebServerRequest *request)
			  { request->send(SPIFFS, "/asset-manifest.json", "application/json"); });

	// Get RA and DEC in HDMS format
	server.on("/get_RA_DEC_HDMS", HTTP_GET, [](AsyncWebServerRequest *request)
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
				double ra=RA_DEC_Json["ra"];
				double dec=RA_DEC_Json["dec"];
				int ra_h,ra_m,dec_d,dec_m;
				double ra_s,dec_s;
				degrees_to_hms(ra, ra_h, ra_m, ra_s);
				degrees_to_dms(dec, dec_d, dec_m, dec_s);
		JsonDocument respJson;
		respJson["ra_h"] =ra_h;
		respJson["ra_m"] =ra_m; 
		respJson["ra_s"] =ra_s; 
		respJson["dec_d"] = dec_d;
		respJson["dec_m"] = dec_m;
		respJson["dec_s"] = dec_s;
		String response;
		serializeJson(respJson, response);
		request->send(200, "application/json", response); });
	// Get RA and DEC in float format
	server.on("/get_RA_DEC_Float", HTTP_GET, [](AsyncWebServerRequest *request)
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
			respJson["ra"] =RA_DEC_Json["ra"];
			respJson["dec"] = RA_DEC_Json["dec"];

			String response;
			serializeJson(respJson, response);
			request->send(200, "application/json", response); });
	// Get Config
	server.on("/get_config", HTTP_GET, [](AsyncWebServerRequest *request)
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
				respJson["ssid"] = configJson["SSID"];
				respJson["pwd"] = configJson["pwd"];
				respJson["ratio"] = configJson["ratio"];
				String response;
				serializeJson(respJson, response);
				request->send(200, "application/json", response); });
	// Get Status
	server.on("/get_status", HTTP_GET, [](AsyncWebServerRequest *request)
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
				request->send(200, "application/json", response); });
	// Get EfuseMac
	server.on("/get_EfuseMac", HTTP_GET, [](AsyncWebServerRequest *request)
			  { 
				uint64_t chipId =ESP.getEfuseMac();
				char chipIdStr[17]; 
    			sprintf(chipIdStr, "%012llX", chipId); 
				JsonDocument respJson; 
				respJson["EfuseMac"]=chipIdStr; 
				String response;
				serializeJson(respJson, response);
				request->send(200, "application/json", response); });
	// POST API
	server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
						 {
        if(request->method()==HTTP_POST && request->contentType()=="application/json"){
            if(request->url()=="/set_status"){
                handleSetStatus(request,data);
            }else if(request->url()=="/set_config"){
                handleSetStatus(request,data);
			}else if(request->url()=="/set_RA_DEC_Float"){
                handleSetRA_DEC_Float(request,data);
			}else if(request->url()=="/set_RA_DEC_HDMS"){
                handleSetRA_DEC_HDMS(request,data);
			}
            else{
                request->send(500);
            }
        } });
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
	File statusFile = SPIFFS.open("/Status.json", "w");
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

void handleSetRA_DEC_Float(AsyncWebServerRequest *request, uint8_t *data) // POST http://localhost:3000/set_RA_DEC_Float
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
void handleSetRA_DEC_HDMS(AsyncWebServerRequest *request, uint8_t *data)
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