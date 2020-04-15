// ************************************* //
// * Arduino Project RFLink-esp        * //
// * https://github.com/couin3/RFLink  * //
// * 2018..2020 Stormteam - Marc RIVES * //
// * More details in RFLink.ino file   * //
// ************************************* //

#include <Arduino.h>
#include "RFLink.h"
#if (defined(ESP32) || defined(ESP8266))

#include "4_Display.h"
#include "6_WiFi_MQTT.h"
#include "9_AutoConnect.h"

#ifdef ESP32
#include <WiFi.h>
#elif ESP8266
#include <ESP8266WiFi.h>
#endif

#ifdef MQTT_ENABLED

// MQTT_KEEPALIVE : keepAlive interval in Seconds
#define MQTT_KEEPALIVE 60

// MQTT_SOCKET_TIMEOUT: socket timeout interval in Seconds
#define MQTT_SOCKET_TIMEOUT 60

#include <PubSubClient.h>

// Update these with values suitable for your network.

WiFiClient WIFIClient;
PubSubClient MQTTClient; // MQTTClient(WIFIClient);

void setup_WIFI()
{
  WiFi.persistent(false);
  WiFi.setAutoReconnect(true);
#ifdef ESP8266
  WiFi.setSleepMode(WIFI_MODEM_SLEEP);
  WiFi.setOutputPower(ac_WIFI_PWR);
#endif // ESP8266
  WiFi.mode(WIFI_STA);

  // Comment out for Dynamic IP
  WiFi.config(ac_WIFI_IP, ac_WIFI_GATEWAYS, ac_WIFI_SUBNET);

  // We start by connecting to a WiFi network
  Serial.print(F("\nConnecting to "));
  Serial.print(ac_WIFI_SSID);
  WiFi.begin(ac_WIFI_SSID, ac_WIFI_PSWD);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.print(F("\nWiFi connected\t"));
  Serial.print(F("IP address: "));
  Serial.print(WiFi.localIP());
  Serial.print(F("\tRSSI "));
  Serial.println(WiFi.RSSI());
}

void setup_MQTT()
{
  MQTTClient.setClient(WIFIClient);
  MQTTClient.setServer(ac_MQTT_SERVER.c_str(), ac_MQTT_PORT.toInt());
  // MQTTClient.setCallback(callback);
}

/*
  void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print(F("Message arrived ["));
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.write(payload[i]);
  }
  Serial.write('\n');
  Serial.println();
  }
*/

void reconnect()
{
  // Loop until we're reconnected
  // delay(1);
  while (!MQTTClient.connected())
  {
    Serial.print(F("Attempting MQTT connection..."));
    // Attempt to connect
    if (MQTTClient.connect(ac_MQTT_ID.c_str(), ac_MQTT_USER.c_str(), ac_MQTT_PSWD.c_str()))
    {
      Serial.println(F("Connected"));
      // Once connected, resubscribe
      // MQTTClient.subscribe(ac_MQTT_TOPIC_IN.c_str());
    }
    else
    {
      Serial.print(F("\nFailed, rc="));
      Serial.print(MQTTClient.state());
      Serial.println(F("\tTry again in 5 seconds"));
      // Wait 5 seconds before retrying
      for (byte i = 0; i < 10; i++)
        delay(500); // delay(5000) may cause hang
    }
  }
}

void publishMsg()
{
  if (!MQTTClient.connected())
  {
    reconnect();
  }
  MQTTClient.publish(ac_MQTT_TOPIC_OUT.c_str(), pbuffer, ac_MQTT_RETAINED);
}

void checkMQTTloop()
{
  static unsigned long lastCheck = millis();

  if (millis() > lastCheck + MQTT_LOOP_MS)
  {
    if (!MQTTClient.connected())
    {
      reconnect();
    }
    // Serial.print(F("Calling MQTT loop()..."));
    MQTTClient.loop();
    // Serial.println(F("Done"));
    lastCheck = millis();
  }
}

#else // MQTT_ENABLED

void setup_WIFI_OFF()
{
  WiFi.persistent(false);
  WiFi.setAutoReconnect(false);
  WiFi.setSleepMode(WIFI_MODEM_SLEEP);
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
}

#endif // MQTT_ENABLED
#endif // (defined(ESP32) || defined(ESP8266))
