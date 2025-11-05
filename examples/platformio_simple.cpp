/**
 * Simplified PlatformIO version for ESP8266 Core 3.x
 * Uses PEM format certificates (easier to work with in PlatformIO)
 * 
 * This version uses setCACert(), setCertificate(), and setPrivateKey()
 * with PROGMEM strings containing PEM-encoded certificates.
 * 
 * To use this:
 * 1. Convert your certificates to PEM format (if needed)
 * 2. Replace the certificate strings below with your actual certificates
 * 3. Make sure your certificates are in PEM format (not DER)
 */

#include "LittleFS.h"
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// Update these with values suitable for your network.
const char* ssid = "your_ssid";
const char* password = "your_password";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "time.ntp.org");
const char* endpoint = "aws.mqtt.com"; // MQTT broker IP

// CA Certificate in PEM format
const char* ca_cert = R"(
-----BEGIN CERTIFICATE-----
YOUR_CA_CERTIFICATE_HERE
-----END CERTIFICATE-----
)";

// Client Certificate in PEM format
const char* client_cert = R"(
-----BEGIN CERTIFICATE-----
YOUR_CLIENT_CERTIFICATE_HERE
-----END CERTIFICATE-----
)";

// Private Key in PEM format
const char* private_key = R"(
-----BEGIN PRIVATE KEY-----
YOUR_PRIVATE_KEY_HERE
-----END PRIVATE KEY-----
)";

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

WiFiClientSecure espClient;
PubSubClient client(endpoint, 8883, callback, espClient);

long lastMsg = 0;
char msg[50];
int value = 0;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  timeClient.begin();
  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }
  
  // Set time for certificate validation
  espClient.setX509Time(timeClient.getEpochTime());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    if (client.connect("ESP-75C1F7")) {
      Serial.println("connected");
      client.publish("outTopic", "hello world");
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      
      char buf[256];
      espClient.getLastSSLError(buf, 256);
      Serial.print("WiFiClientSecure SSL error: ");
      Serial.println(buf);
      
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  
  pinMode(LED_BUILTIN, OUTPUT);
  
  // Configure buffer sizes
  espClient.setBufferSizes(1024, 1024);
  
  // Setup WiFi first
  setup_wifi();
  delay(1000);
  
  Serial.print("Heap: ");
  Serial.println(ESP.getFreeHeap());
  
  // Load certificates using PEM format (ESP8266 Core 3.x method)
  espClient.setCACert(ca_cert);
  espClient.setCertificate(client_cert);
  espClient.setPrivateKey(private_key);
  
  Serial.println("Certificates loaded");
  Serial.print("Heap: ");
  Serial.println(ESP.getFreeHeap());
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf(msg, 75, "{\"message\": \"hello world #%ld\"}", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("outTopic", msg);
    Serial.print("Heap: ");
    Serial.println(ESP.getFreeHeap());
  }
  
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
}

