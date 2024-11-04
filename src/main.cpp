#include <Arduino.h>
#include "Adafruit_Si7021.h"
#include "SPI.h"
#include "WiFi.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "wifiConfig.h"
#include <fstream>
#include <iostream>
#include <string>
#include "time.h"

//Define constante for Deep sleep mode
#define uS_TO_S_CONVERSION 1000000
#define DEEP_SLEEP_TIME 600 // in sec

void device_deep_sleep(int seconds){
    delay(500);
    esp_sleep_enable_timer_wakeup(seconds * uS_TO_S_CONVERSION);
    esp_deep_sleep_start();
}

Adafruit_Si7021 sensor = Adafruit_Si7021();

int serialNumber = 100;

time_t getUnixTimestamp() {
    struct tm timeinfo;
    int retryCount = 0;
    while (!getLocalTime(&timeinfo)) {
        retryCount++;
        if (retryCount > 5) {
            Serial.println("Failed to obtain time after multiple retries");
            return 0;
        }
        delay(1000); // Wait for 1 second before retrying
    }
    time_t unixTime = mktime(&timeinfo);
    return unixTime;
}


void sendJson(float temperature, float humidity) {
    HTTPClient http;

    String apiEndpoint = String(apiEndpointBase) + "/api/measurement";
    http.begin(apiEndpoint.c_str()); //Specify request destination
    http.addHeader("Content-Type", "application/json");

    // Prepare the HTTP POST request data
    // Create a JSON document
    JsonDocument doc;
    doc["temperature"] = temperature;
    doc["humidity"] = humidity/100;
    doc["sensor_id"] = serialNumber;

    // TODO: debug getUnixTimestamp, getUniTimestamp keeps failing even with retries and return 0
    // time_t timestamp =getUnixTimestamp();

    // If the timestamp is not 0, then it is added to the json object
    // if (timestamp != 0) {
    //     Serial.println("timestamp: " + String(timestamp));
    //     doc["timestamp"] = timestamp;
    // }
    
    // Serialize the json object
    String json;
    serializeJson(doc, json);
    Serial.print(json);

    // Post the json object to the api endpoint
    int httpResponseCode = http.POST(json);

    // Error handling
    if (httpResponseCode > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
    } else {
        Serial.print("Error on sending POST: ");
        Serial.println(httpResponseCode);
    }

    http.end();  //Free resources
}


void setup (){

    Serial.begin(115200);

    // Initialize the Si7021 sensor
    if (!sensor.begin()) {
    Serial.println("Could not find a valid Si7021 sensor, check wiring!");
    while (1);
    }

    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    Serial.println("Connecting to WiFi..");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi..");
    }

    Serial.println("Connected to the WiFi network");
    
}

void loop (){

    float temperature = sensor.readTemperature(); // Read temperature from Si7021
    float humidity = sensor.readHumidity(); // Read Humidity from Si7021

    //printing for debugging only
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");

    // Check WiFi connection status
    if(WiFi.status() == WL_CONNECTED){

        sendJson(temperature, humidity);

    } else {
        Serial.println("WiFi Disconnected. Attempting reconnection...");
        WiFi.disconnect();
        WiFi.reconnect();
    }

    //put the device in deep sleep mode for 10min
    device_deep_sleep(DEEP_SLEEP_TIME);

}







