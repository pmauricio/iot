// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// Please use an Arduino IDE 1.6.8 or greater

// You must set the device id, device key, IoT Hub name and IotHub suffix in
// iot_configs.h
#include "iot_configs.h"

#include <AzureIoTHub.h>
#if defined(IOT_CONFIG_MQTT)
    #include <AzureIoTProtocol_MQTT.h>
#elif defined(IOT_CONFIG_HTTP)
    #include <AzureIoTProtocol_HTTP.h>
#endif

#include "sample.h"
#include "esp8266/sample_init.h"

#define LED_BUILTIN 2
#define BUTTON_BUILTIN 0 

static char ssid[] = IOT_CONFIG_WIFI_SSID;
static char pass[] = IOT_CONFIG_WIFI_PASSWORD;
//const int analog_ip = A0;

void setup() {
    sample_init(ssid, pass);
    pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BUTTON_BUILTIN, INPUT);
}


// Azure IoT samples contain their own loops, so only run them once
static bool done = false;
void loop() {
    if (true)
    {
      Serial.print(".");
        // Run the sample
        // You must set the device id, device key, IoT Hub name and IotHub suffix in
        // iot_configs.h
        sample_run(analogRead(0));
        done = true;
          Serial.print("<>!!!!!!!!!!!!!!!!!!!!!!!!!!!");
            Serial.print("<>");
      delay(7000);
    }
    else
    {
       Serial.print("<>");
      delay(2000);
    }
}
