/*
 *  Created by _StarChaser
 * ถ้าเครดิตถูกลบออกการ Support หลังขายจะสิ้นสุดทันที!
 */

// Config
const char *ssid = "wifi";
const char *password = "aek123456789";
const int nodeid = 1;
char hostname[] = "SC-PM2-SENDOR-NODE-1";
char group[] = "GROUP-1";
const char *node0_url = "http://128.199.150.87:3210";
// const char *node0_url = "http://pm25.0127002.xyz";
char sensor_name[] = "GP2Y1010AU0F";
float sensor_value = 0;
int sendTick = (1000 / 10) * 10;
const int sharpLEDPin = D3; // Arduino digital pin D3 connect to sensor LED.
const int sharpVoPin = A0;  // Arduino analog pin A0 connect to sensor Vo.
const bool debug = false;
// End Config
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "PM_Sensor.h"
#include <SPI.h>
#include <Wire.h>
// #include <stdio.h>
// #include <string.h>
// System variables (not configurable)
char local_ip[16] = "unknown";
WiFiClient client;
AsyncWebServer server(80);

/*
 * Print out function
 */
void PrintOut(char *msg)
{
  Serial.print(msg);
  delay(50);
}
void PrintOutl(char *msg)
{
  Serial.println(msg);
  delay(50);
}
void PrintOut(String msg)
{
  Serial.print(msg);
  delay(50);
}
void PrintOutl(String msg)
{
  Serial.println(msg);
  delay(50);
}
void PrintOut(int msg)
{
  Serial.print(msg);
  delay(50);
}
void PrintOutl(int msg)
{
  Serial.println(msg);
  delay(50);
}

/*
 * ================================================================
 *                            Workspace
 * ================================================================
 */

void PM_loop()
{

  // Record the output voltage. This operation takes around 100 microseconds.
  int VoRaw = analogRead(sharpVoPin);

// Print raw voltage value (number from 0 to 1023).
#ifdef PRINT_RAW_DATA

  // printValue("VoRaw", VoRaw, true);
  Serial.println("");
#endif // PRINT_RAW_DATA

  // Use averaging if needed.
  float Vo = VoRaw;

#ifdef USE_AVG
  VoRawTotal += VoRaw;
  VoRawCount++;
  if (VoRawCount >= N)
  {
    Vo = 1.0 * VoRawTotal / N;
    VoRawCount = 0;
    VoRawTotal = 0;
  }
  else
  {
    return;
  }
#endif // USE_AVG

  // Compute the output voltage in Volts.
  Vo = Vo / 1024.0 * 3.3;
  // printFValue("Vo", Vo, "V");

  // Convert to Dust Density in units of ug/m3.
  float dV = Vo - Voc;
  if (dV < 0)
  {
    dV = 0;
    Voc = Vo;
  }
  float dustDensity = dV / K * 100.0;
  sensor_value = dustDensity;
  PrintOut("PM2.5 = ");
  PrintOut(sensor_value);
  PrintOutl(" ug/m3");
}

void onBootComplete()
{
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { 
                StaticJsonBuffer<1024> doc;
                JsonObject &root = doc.createObject();
                root["success"] = true;
                root["hostname"] = hostname;
                root["message"] = "PM2 Sensor Project (yue.sh)";
                root["node_id"] = nodeid;
                root["local_ip"] = local_ip;
                root["uptime"] = millis();
                root["node0_url"] = node0_url;
                root["sensor_name"] = sensor_name;
                root["group"] = group;
                // root["sensor_value"] = sensor_value;
                root["value"] = sensor_value;
                root["debug"] = debug;
                char result[1024];
                root.printTo(result, sizeof(result));
              request->send(200, "application/json", result); });
  server.begin();
  PrintOutl("[ESP] WEB SERVER STARTED.");
}
int tick = sendTick;
void onTick()
{
  PM_loop();
  if (tick >= sendTick)
  {
    tick = 0;
    StaticJsonBuffer<1024> doc;
    JsonObject &root = doc.createObject();
    root["success"] = true;
    root["hostname"] = hostname;
    root["message"] = "PM2 Sensor Project (yue.sh)";
    root["node_id"] = nodeid;
    root["local_ip"] = local_ip;
    root["uptime"] = millis();
    root["node0_url"] = node0_url;
    root["sensor_name"] = sensor_name;
    root["group"] = group;
    // root["sensor_value"] = sensor_value;
    root["value"] = sensor_value;
    root["debug"] = debug;
    char result[1024];
    root.printTo(result, sizeof(result));
    HTTPClient http;
    http.begin(client, node0_url);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Accept", "application/json");
    Serial.println("result body:");
    Serial.println(result);
    int httpCode = http.POST(result);
    PrintOutl("[HTTP] POST TO NODE0 PROCESSED.");
    Serial.printf("[HTTP] POST... code: %d\n", httpCode);
    if (httpCode == HTTP_CODE_OK)
    {
      const String &payload = http.getString();
      Serial.println("[HTTP] received payload:\n<<");
      Serial.println(payload);
      Serial.println(">>");
    }
    else
    {
      Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
      PrintOut("[HTTP] HTTP CODE: ");
      PrintOutl(httpCode);
      PrintOut("[HTTP] BODY: ");
      PrintOutl(http.getString());
    }
    http.end();
  }
  else
  {
    tick++;
  }
}

/*
 * System Functions
 */

void whitelist_check()
{
  PrintOutl("[SCAPI] Checking whitelist...");
  PrintOutl("[SCAPI] This device is already trusted, contunue booting...");
}

void bootup()
{
  WiFi.setHostname(hostname);
  PrintOutl("[ESP] STARTING WIFI...");
  if (WiFi.begin(ssid, password))
  {
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      PrintOut(".");
    }
  }
  else
  {
    PrintOutl("[SCAPI] Failed connect to WiFi ,the system cannot be continue...");
    delay(3000);
    ESP.reset();
  }
  PrintOutl("");
  PrintOutl("[ESP] WiFi CONNECTED");
  PrintOutl("[WIFI] Local IP Address: " + WiFi.localIP().toString());
  char Buffer[16];
  WiFi.localIP().toString().toCharArray(local_ip, 16);
  whitelist_check();
}

void setup()
{
  Serial.begin(9600);
  Serial.println();
  delay(1000);
  PrintOutl("[SCAPI] ====== PM2 WITH SENSOR HTTP PROJECT ======");
  delay(100);
  PrintOutl("[SCAPI] Node ID: " + String(nodeid));
  delay(100);
  PrintOutl("[SCAPI] Made with heart <3 by: _StarChaser");
  delay(100);
  PrintOutl("[SCAPI] Build Date: 03/03/22");
  delay(100);
  PrintOutl("[SCAPI] BOOTING....");
  delay(100);
  bootup();
  delay(500);
  PrintOutl("[SCAPI] Bootup Complete!");
  onBootComplete();
}
void loop()
{
  onTick();
  delay(10);
}