#include <Arduino.h>
#include "Adafruit_Si7021.h"
#include "SPI.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>


BLECharacteristic *pCharacteristic;

//Boolean that says if the device is connected or not
bool deviceConnected= false;

// float to store temperature reading
float Temp= 0;

//Define the UUID for service and characteristics
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"


//Define constante for Deep sleep mode

#define uS_TO_S_CONVERSION 1000000
#define DEEP_SLEEP_TIME 10 // in sec

void device_sleep(int seconds){
   
    delay(500);
    esp_sleep_enable_timer_wakeup(seconds * uS_TO_S_CONVERSION);
    esp_deep_sleep_start();
}



class serverCallbacks: public BLEServerCallbacks{
    void onConnect(BLEServer* pServer){
        deviceConnected=true;
    };

    void onDisconnect(BLEServer* pServer){
        deviceConnected=false;
    };


};


Adafruit_Si7021 sensor = Adafruit_Si7021();


void setup (){
    Serial.begin(115200);

    sensor.begin();

    // Create the BLE Device
    BLEDevice::init("ESP32");

    // Create the BLE Server
    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new serverCallbacks());

    // Create the BLE Service
    BLEService *pService = pServer->createService(SERVICE_UUID);

    // Create the BLE Characteristic
    pCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID,
                        BLECharacteristic::PROPERTY_NOTIFY
    );

    //BLE2902 needed to notify
    pCharacteristic->addDescriptor(new BLE2902());

    //Start the service
    pService-> start();

    //start advertising 
    pServer->getAdvertising()->start();
    Serial.println("Waiting for a client connection to notify...");



}
void loop (){

    delay(5000);

    if (deviceConnected){
        Temp=sensor.readTemperature();

        // Conversion of Temp
        char txString[8];
        dtostrf(Temp,2,2,txString);

        //Setting the value to the characteristic
        pCharacteristic->setValue(txString);

        //Notify the connected client
        pCharacteristic->notify();

        Serial.println("Sent value: "+String(txString));

        Serial.println("Going to deep sleep");
        device_sleep(DEEP_SLEEP_TIME);
    };


    if (!deviceConnected){
        
        Serial.println("No connection detected, going back to sleep");
        device_sleep(DEEP_SLEEP_TIME);
    };

   
}







