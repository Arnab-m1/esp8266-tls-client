/**
 * PlatformIO compatible version for ESP8266 Core 3.x
 * This version loads DER certificates from LittleFS
 * 
 * For ESP8266 Core 3.x, the certificate loading API changed.
 * This version reads DER files from LittleFS and converts them for use.
 * 
 * NOTE: For ESP8266 Core 3.x, it's recommended to use PEM format certificates
 * instead (see platformio_simple.cpp). This DER version requires more memory.
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

// Helper function to read file from LittleFS into a String
String readFileFromLittleFS(const char* filename) {
  File file = LittleFS.open(filename, "r");
  if (!file) {
    Serial.print("Failed to open ");
    Serial.println(filename);
    return String("");
  }

  String content = "";
  while (file.available()) {
    content += (char)file.read();
  }
  file.close();
  return content;
}

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
  
  // Initialize LittleFS
  if (!LittleFS.begin()) {
    Serial.println("Failed to mount LittleFS file system");
    return;
  }
  
  Serial.print("Heap: ");
  Serial.println(ESP.getFreeHeap());
  
  // Setup WiFi first
  setup_wifi();
  delay(1000);
  
  // Configure buffer sizes
  espClient.setBufferSizes(1024, 1024);
  
  // For ESP8266 Core 3.x, we need to use PEM format certificates
  // If you have DER files, convert them to PEM first:
  // openssl x509 -inform DER -in ca.der -out ca.pem
  // openssl x509 -inform DER -in cert.der -out cert.pem
  // openssl rsa -inform DER -in private.der -out private.pem
  
  // Option 1: Read PEM certificates from LittleFS
  // Place your PEM certificates in LittleFS: /ca.pem, /cert.pem, /private.pem
  String caCert = readFileFromLittleFS("/ca.pem");
  String clientCert = readFileFromLittleFS("/cert.pem");
  String privateKey = readFileFromLittleFS("/private.pem");
  
  if (caCert.length() > 0 && clientCert.length() > 0 && privateKey.length() > 0) {
    espClient.setCACert(caCert.c_str());
    espClient.setCertificate(clientCert.c_str());
    espClient.setPrivateKey(privateKey.c_str());
    Serial.println("Certificates loaded from LittleFS (PEM format)");
  } else {
    Serial.println("ERROR: Failed to load certificates from LittleFS");
    Serial.println("Please ensure PEM format certificates are uploaded to LittleFS:");
    Serial.println("  - /ca.pem");
    Serial.println("  - /cert.pem");
    Serial.println("  - /private.pem");
    Serial.println("");
    Serial.println("To convert DER to PEM:");
    Serial.println("  openssl x509 -inform DER -in ca.der -out ca.pem");
    Serial.println("  openssl x509 -inform DER -in cert.der -out cert.pem");
    Serial.println("  openssl rsa -inform DER -in private.der -out private.pem");
  }
  
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

