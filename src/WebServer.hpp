#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>

static AsyncWebServer server(80);

unsigned long ota_progress_millis = 0;

void onOTAStart() {
  // Log when OTA has started
  Serial.println("OTA update started!");
  // <Add your own code here>
}

void onOTAProgress(size_t current, size_t final) {
  // Log every 1 second
  if (millis() - ota_progress_millis > 1000) {
    ota_progress_millis = millis();
    Serial.printf("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
  }
}

void onOTAEnd(bool success) {
  // Log when OTA has finished
  if (success) {
    Serial.println("OTA update finished successfully!");
  } else {
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
// void AsyncWebServerStart()
// {
//     Serial.println("AsyncWebServerStart");
//     if (!SPIFFS.begin(true))
//     {
//         Serial.println("An Error has occurred while mounting SPIFFS");
//         return;
//     }

//     server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
//               { request->send(SPIFFS, "/index.html", "text/html"); });
//     // Route to load style.css file
//     server.on("/css/app.2cf79ad6.css", HTTP_GET, [](AsyncWebServerRequest *request)
//               { request->send(SPIFFS, "/css/app.2cf79ad6.css", "text/css"); });
//     server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request)
//               { request->send(SPIFFS, "/favicon.ico", "image/x-icon"); });
//     server.on("/js/chunk-vendors.js", HTTP_GET, [](AsyncWebServerRequest *request)
//               { request->send(SPIFFS, "/js/chunk-vendors.js", "application/javascript"); });
//     server.on("/js/app.998f6777.js", HTTP_GET, [](AsyncWebServerRequest *request)
//               { request->send(SPIFFS, "/js/app.998f6777.js", "application/javascript"); });

//     // Server api
//     server.on("/buzz", HTTP_GET, [](AsyncWebServerRequest *request)
//               { request->send(200, "application/json", "{\"message\":\"Buzz play Little Star\"}"); 
//                 vTaskResume(xHandleBuzzPlayLittleStar);
//               });

//     server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
//                          {
//         if(request->method()==HTTP_POST && request->contentType()=="application/json"){
//             if(request->url()=="/switch/led"){
//                 handleSwitchLed(request,data);  
//             }
//             else{
//                 request->send(500);
//             }
//         } });

//     // Start server
//     server.begin();
// }
