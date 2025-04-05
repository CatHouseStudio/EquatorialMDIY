#pragma once
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>
#include "SerialMessage.hpp"
#include "WiFiApSta.hpp"
#include "CelestialPositioning.hpp"
// #include "GPSInfo.hpp"
#include "CelestialStepper.hpp"
#include "TiltFusionMPU6050.hpp"
static AsyncWebServer server(80);
unsigned long ota_progress_millis = 0;

void onOTAStart();
void onOTAProgress(size_t current, size_t final);
void onOTAEnd(bool success);

void WebServerEvent();
// Server API events
// HTTP_GET
void handleGetRA_DEC_HDMS(AsyncWebServerRequest *request);	// GET http://localhost:3000/get_RA_DEC_HDMS
void handleGetRA_DEC_Float(AsyncWebServerRequest *request); // GET http://localhost:3000/get_RA_DEC_Float
void handleGetConfig(AsyncWebServerRequest *request);		// GET http://localhost:3000/get_config
void handleGetStatus(AsyncWebServerRequest *request);		// GET http://localhost:3000/get_status
void handleGetEfuseMac(AsyncWebServerRequest *request);		// GET http://localhost:3000/get_EfuseMac
void handleGetTiltFusion(AsyncWebServerRequest *request);	// GET http://localhost:3000/get_TiltFusion
void handleGetGPS(AsyncWebServerRequest *request);			// GET http://localhost:3000/get_gps

// HTTP_POST
void handleSetStatus(AsyncWebServerRequest *request, uint8_t *data);	   // POST http://localhost:3000/set_status
void handleSetConfig(AsyncWebServerRequest *request, uint8_t *data);	   // POST http://localhost:3000/set_config
void handleSetRA_DEC_Float(AsyncWebServerRequest *request, uint8_t *data); // POST http://localhost:3000/set_RA_DEC_Float
void handleSetRA_DEC_HDMS(AsyncWebServerRequest *request, uint8_t *data);  // POST http://localhost:3000/set_RA_DEC_HDMS
void handleSetGPS(AsyncWebServerRequest *request, uint8_t *data);		   // POST http://localhost:3000/set_gps
void handleSetTime(AsyncWebServerRequest *request, uint8_t *data);		   // POST http://localhost:3000/set_time

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
	// POST API
	server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
						 {
        if(request->method()==HTTP_POST && request->contentType()=="application/json"){
            if(request->url()=="/set_status"){
                handleSetStatus(request,data);
            }else if(request->url()=="/set_config"){
                handleSetConfig(request,data);
			}else if(request->url()=="/set_RA_DEC_Float"){
                handleSetRA_DEC_Float(request,data);
			}else if(request->url()=="/set_RA_DEC_HDMS"){
                handleSetRA_DEC_HDMS(request,data);
			}else if(request->url()=="/set_gps"){
                handleSetGPS(request,data);
			}
            else{
                request->send(404,"text/plain","Unknown POST endpoint");
            }
        }
		else{
			request->send(415,"text/plain","Unsupported content type");
		} });
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
	respJson["ssid"] = configJson["SSID"];
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
	float r, p, z;
	safeGetAngles(r, p, z);
	JsonDocument respJson;
	respJson["roll"] = r;
	respJson["pitch"] = r;
	respJson["ztilt"] = z;
	String response;
	serializeJson(respJson, response);
	request->send(200, "application/json", response);
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
// HTTP_POST
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
void handleSetGPS(AsyncWebServerRequest *request, uint8_t *data) // POST http://localhost:3000/set_gps
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