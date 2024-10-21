#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Creds.h>
#include <ArduinoJson.h>
const int sensor_pin = 32;

const char* _serverName = "https://andrew-pinner.asuscomm.com/smart/api/saveSensor";
const char* _authServer = "https://andrewp.online/RCON/api/login";
String _token = "";

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
}

void GetAuth() {
  String token;

  if (WiFi.status() == WL_CONNECTED)
  {
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient https;
    https.useHTTP10(true);

    if (https.begin(client, _authServer))
    {
      String request;
      JsonDocument doc;
      doc["Username"] = Username;
      doc["Password"] = Password;

      serializeJson(doc, request);

      https.addHeader("Content-Type", "application/json");
      int httpCode = https.POST(request);

      JsonDocument response;
      deserializeJson(response, https.getStream());
      token = response["Token"].as<String>();

      https.end();
    }
    else
    {
      Serial.println("Could not connect to server");
    }
  }

  _token = token;
}

void UpdateSensor() {
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient https;
  https.useHTTP10(true);

  if (_token.isEmpty())
  {
    GetAuth();
  }
  
  if (https.begin(client, _serverName))
  {
    JsonDocument doc;
    doc["Location"] = "Cilantro";
    doc["SensorValue"] = analogRead(sensor_pin);
    doc["SensorType"] = "Moisture_Sensor";

    String request;
    serializeJson(doc, request);
    Serial.println(request);

    https.addHeader("Content-Type", "application/json");
    https.addHeader("Authorization", "Bearer " + _token);
    int httpCode = https.POST(request);
    https.end();

    if (httpCode == 401)
    {
      _token = "";
      UpdateSensor();
    }
    
  }
  else
  {
    Serial.println("Could not connect to server");
  }
}

void loop() {
  UpdateSensor();
  delay(6000000);
}
