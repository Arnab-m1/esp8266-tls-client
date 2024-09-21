# ESP8266 MQTT Client with SSL/TLS

This project demonstrates how to connect an ESP8266 to an MQTT broker (AWS or other) using SSL/TLS encryption. The project uses NodeMCU or any ESP8266-based board to connect securely to an MQTT broker, send messages, and subscribe to topics. Certificates are required to authenticate and secure the connection.

## Features

- Secure connection to MQTT broker using SSL/TLS
- WiFi connectivity with ESP8266
- Periodic publishing of messages to an MQTT topic
- Subscription to an MQTT topic

## Requirements

- ESP8266-based board (e.g., NodeMCU, Wemos D1 Mini)
- [Arduino IDE](https://www.arduino.cc/en/software)
- ESP8266 core for Arduino IDE
- MQTT Broker (e.g., AWS IoT)
- SSL/TLS certificates (CA, client certificate, and private key)

## How It Works

1. **WiFi Connection**: The ESP8266 connects to the specified WiFi network.
2. **MQTT Connection**: It connects to the MQTT broker using SSL/TLS on port `8883`.
3. **Message Publishing**: The ESP8266 publishes a message periodically to the `outTopic` and subscribes to the `inTopic` to receive messages.
4. **SSL/TLS Authentication**: The connection is secured using certificates (client certificate, private key, and CA certificate).
5. **LED Blink**: The built-in LED blinks to indicate the ESP8266 is functioning.

## How to Use

1. **Install Dependencies**:
    - Download and install the [Arduino IDE](https://www.arduino.cc/en/software).
    - Install the ESP8266 core in the Arduino IDE by going to `File` -> `Preferences` -> `Additional Boards Manager URLs` and adding:
      ```
      http://arduino.esp8266.com/stable/package_esp8266com_index.json
      ```
    - Install the required libraries from the Arduino Library Manager:
      - `PubSubClient`
      - `NTPClient`
      - `ESP8266WiFi`
      - `FS` (SPIFFS)

2. **Convert Certificates to DER Format**:
    MQTT over SSL/TLS requires that the certificates are in `.der` format. If your certificates are in `.pem` or `.crt`, you need to convert them to `.der`. Use OpenSSL to do this:

    ```bash
    openssl x509 -in cert.pem -outform DER -out cert.der
    openssl rsa -in private.pem -outform DER -out private.der
    openssl x509 -in ca.pem -outform DER -out ca.der
    ```

3. **Upload Certificates to ESP8266 (SPIFFS)**:
    - In Arduino IDE, go to `Tools` -> `ESP8266 Sketch Data Upload` to upload the `cert.der`, `private.der`, and `ca.der` files to the SPIFFS file system.
    - Create the `data` folder in the project directory, place the `.der` files inside, and run the SPIFFS upload tool.

4. **Change WiFi and MQTT Broker Details**:
    Modify the following values in the code:

    ```cpp
    const char* ssid = "your_ssid";
    const char* password = "your_password";
    const char* endpoint = "your_mqtt_broker_url"; // e.g., AWS IoT endpoint
    ```

5. **Flashing the Code**:
    Upload the code to your ESP8266 using the Arduino IDE. Ensure that the board and port settings are correctly configured in the `Tools` menu.

## Code Explanation

- **WiFi Setup**: The `setup_wifi()` function handles the connection to the WiFi network.
- **MQTT Setup**: The `PubSubClient` library is used to connect to the MQTT broker and handle message publishing/subscribing.
- **Certificate Loading**: The `SPIFFS` file system is used to store and load the SSL/TLS certificates required for secure communication.
- **Time Syncing**: The `NTPClient` library is used to get the current time, which is needed for certificate validation.

## Example Commands for Converting and Uploading Certificates

Convert `.pem` certificates to `.der` format using OpenSSL:

```bash
openssl x509 -in cert.pem -outform DER -out cert.der
openssl rsa -in private.pem -outform DER -out private.der
openssl x509 -in ca.pem -outform DER -out ca.der
```

## Flashing Certificates to SPIFFS

To securely connect your ESP8266 to the MQTT broker using SSL/TLS, you need to upload the certificates (`cert.der`, `private.der`, and `ca.der`) to the ESP8266's SPIFFS.

### Steps to Upload Certificates to SPIFFS:

1. **Create a `data` Folder:**
   - In your project directory, create a folder named `data`.

2. **Place Certificate Files:**
   - Copy the `cert.der`, `private.der`, and `ca.der` files into the `data` folder.

3. **Upload Files to SPIFFS:**
   - In the Arduino IDE, go to `Tools` -> `ESP8266 Sketch Data Upload`. This will flash the files in the `data` folder to the ESP8266's SPIFFS.

4. **Install the SPIFFS Upload Tool:**
   - If you don't have the **ESP8266 SPIFFS Upload Tool** installed, you can follow [this guide](https://github.com/esp8266/arduino-esp8266fs-plugin) to download and install it in your Arduino IDE.

Once the certificates are uploaded to SPIFFS, the ESP8266 will be able to load them for secure MQTT communication.

## Author

- [Arnab Mallick](mailto:arnabmallick2000@gmail.com)
