#include "FS.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <DHT.h>

// Update these with values suitable for your network.
const char* ssid = "your_ssid";
const char* password = "your_password";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "time.ntp.org");
const char* endpoint = "aws.mqtt.com"; // MQTT broker IP

// DHT Sensor Setup
#define DHTPIN 2          // DHT11 data pin
#define DHTTYPE DHT11     // DHT 11
DHT dht(DHTPIN, DHTTYPE);

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

void setup_wifi() {
  delay(10);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  timeClient.begin();
  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP-75C1F7")) { // Unique client ID
      Serial.println("connected");
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  dht.begin(); // Initialize DHT sensor

  // Initialize secure connection
  espClient.setBufferSizes(1024, 1024);
  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }

  // Load certificate file
  File cert = SPIFFS.open("/cert.der", "r");
  if (cert) {
    espClient.loadCertificate(cert);
    Serial.println("Certificate loaded");
  } else {
    Serial.println("Failed to open cert file");
  }

  // Load private key file
  File private_key = SPIFFS.open("/private.der", "r");
  if (private_key) {
    espClient.loadPrivateKey(private_key);
    Serial.println("Private key loaded");
  } else {
    Serial.println("Failed to open private key file");
  }

  // Load CA file
  File ca = SPIFFS.open("/ca.der", "r");
  if (ca) {
    espClient.loadCACert(ca);
    Serial.println("CA loaded");
  } else {
    Serial.println("Failed to open CA file");
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;

    // Read DHT11 sensor data
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    // Check if any reads failed
    if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    // Create JSON message
    snprintf(msg, sizeof(msg), "{\"temperature\": %.1f, \"humidity\": %.1f}", t, h);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("sensor/data", msg);
  }
}
