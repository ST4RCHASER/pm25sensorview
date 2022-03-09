/*
 *  Created by _StarChaser
 * ถ้าเครดิตถูกลบออกการ Support หลังขายจะสิ้นสุดทันที!
 */

// Config
const char *ssid = "wifi";
const char *password = "aek123456789";
const int nodeid = 1;
char hostname[] = "SC-PM2-SENDOR-NODE-1";
const char node0_addr[16] = "192.168.173.37";
const bool debug = false;

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <WebSocketsClient.h>
#include <stdio.h>
#include <string.h>
// System variables (not configurable)
#define input_sensor V5 
char local_ip[16] = "unknown";
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
AsyncWebSocketClient *latestWSsClient;
WebSocketsClient webSocketC;
WiFiClient client;

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

/*
 * WebSocket Events
 */
// SERVER
void onWSsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  if (type == WS_EVT_CONNECT)
  {
    latestWSsClient = client;
    client->text("Welcome to PM2-SENSOR-NODE-" + String(nodeid) + " all infomation will send after this message.");
    PrintOut("[WSs] New client connected: ");
    // PrintOutl(client->id());
  }
  if (type == WS_EVT_DATA)
  {
    PrintOut("[WSs] Message from client: ");
    PrintOut(client->id());
    PrintOut(" : ");
    PrintOutl((char *)data);
    StaticJsonBuffer<200> respJsonBuffer;
    JsonObject &root = respJsonBuffer.parseObject((char *)data);
    if (!root.success())
    {
      PrintOutl("[WSs] Failed to parse JSON");
      return;
    }
    if (root["type"] == "update")
    {
      PrintOutl("[WSs] Received command update data from client: " + String(client->id()));
      String command_data = root["data"];
      PrintOutl("[WSs] Command data: " + command_data);
      // send_command_data(command_data);
    }
  }
  else if (type == WS_EVT_DISCONNECT)
  {
    PrintOutl("[WSs] Client disconnected");
  }
  else if (type == WS_EVT_ERROR)
  {
    PrintOutl("[WSs] Error");
  }
  else if (type == WS_EVT_PONG)
  {
    PrintOutl("[WSs] PONG");
  }
}

// CLIENT
void onWScEvent(WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case WStype_DISCONNECTED:
    PrintOutl("[WSc] Disconnected!\n");
    break;
  case WStype_CONNECTED:
  {
    PrintOut("[WSc] Connected to :");
    PrintOut(node0_addr);
    PrintOutl((char *)payload);

    // send message to server when Connected
    // PrintOutl("[WSc] SENT: Connected");
    // StaticJsonBuffer<200> jsonBuffer;
    // JsonObject &root = jsonBuffer.createObject();
    // root["type"] = "connected";
    // root["nodeid"] = nodeid;
    // root["ip"] = local_ip;
    // root["hostname"] = hostname;
    // // Convert to text and sent it
    // char buffer[200];
    // root.printTo(buffer, sizeof(buffer));
    // webSocketC.sendTXT(buffer);
  }
  break;
  case WStype_TEXT:
    PrintOut("[WSc] RESPONSE: ");
    PrintOutl((char *)payload);
    break;
  case WStype_BIN:
    PrintOutl("[WSc] get binary length:");
    PrintOut(length);
    hexdump(payload, length);
    break;
  case WStype_PING:
    // pong will be send automatically
    PrintOutl("[WSc] get ping\n");
    break;
  case WStype_PONG:
    // answer to a ping we send
    PrintOutl("[WSc] get pong\n");
    break;
  }
}

void onBootComplete()
{
  if (nodeid == 0)
  {
    PrintOutl("[ESP] STARTING WEB SERVER AND WEBSOCKET SERVER....");
    ws.onEvent(onWSsEvent);
    server.addHandler(&ws);
  }
  else
  {
    PrintOutl("[ESP] STARTING WEB SERVER AND WEBSOCKET CLIENT....");
    webSocketC.onEvent(onWScEvent);
    webSocketC.begin(node0_addr, 80, "/ws");
  }
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    StaticJsonBuffer<1024> doc;
    JsonObject &root = doc.createObject();
    root["success"] = true;
    root["hostname"] = hostname;
    root["message"] = "PM2 Sensor Project (yue.sh)";
    root["node_id"] = nodeid;
    root["local_ip"] = local_ip;
    root["node0_addr"] = node0_addr;
    root["debug"] = debug;
    char result[1024];
    root.printTo(result, sizeof(result));
    request->send(200, "application/json", result); });
  server.begin();
  PrintOutl("[ESP] WEB SERVER AND WEBSOCKET SERVER STARTED.");
}
int tick = 0;
void onTick()
{
  if (nodeid != 0)
  {
    if (tick == 20)
    {
      tick--;
      webSocketC.loop();
      char sensor_name[] = "";
      char sensor_value[] = "0";
      StaticJsonBuffer<1024> doc;
      JsonObject &root = doc.createObject();
      root["node_id"] = nodeid;
      root["local_ip"] = local_ip;
      root["message"] = "Sensor updated";
      root["command"] = "update";
      root["hostname"] = hostname;
      JsonObject &sensor = root.createNestedObject("sensor");
      sensor["name"] = sensor_name;
      sensor["value"] = sensor_value;
      char result_json[1024];
      root.printTo(result_json, sizeof(result_json));
      webSocketC.sendTXT(result_json);
    }else {
      tick++;
    }
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
  PrintOutl("[SCAPI] ====== PM2 WITH SENSOR SOCKET PROJECT ======");
  delay(100);
  PrintOutl("[SCAPI] Node ID: " + String(nodeid));
  delay(100);
  PrintOutl("[SCAPI] Made with heart <3 by: _StarChaser");
  delay(100);
  PrintOutl("[SCAPI] Build Date: 27/02/22");
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