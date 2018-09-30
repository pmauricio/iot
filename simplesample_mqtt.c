// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>

#include <stdio.h>
#include <stdint.h>

#include "AzureIoTHub.h"
#include "iot_configs.h"
#include "sample.h"

/*String containing Hostname, Device Id & Device Key in the format:             */
/*  "HostName=<host_name>;DeviceId=<device_id>;SharedAccessKey=<device_key>"    */
static const char* connectionString = IOT_CONFIG_CONNECTION_STRING;

// Define the Model
BEGIN_NAMESPACE(WeatherStation);

DECLARE_MODEL(ContosoAnemometer,
WITH_DATA(float, ATemperature),
WITH_DATA(float, Temperature),
 WITH_DATA(ascii_char_ptr, Location),
WITH_DATA(ascii_char_ptr, type),
WITH_DATA(ascii_char_ptr, DeviceId),
WITH_DATA(int, WindSpeed),
WITH_DATA(float, Humidity),
WITH_DATA(bool, TemperatureAlert),
WITH_DATA(float, Latitude),
WITH_DATA(float, Longitude),
WITH_ACTION(TurnFanOn),
WITH_ACTION(TurnFanOff),
WITH_ACTION(SetAirResistance, int, Position)
);

END_NAMESPACE(WeatherStation);

static char propText[1024];
#define LED_BUILTIN 2
#define BUTTON_BUILTIN 0 

#define HIGH 0x1
#define LOW  0x0
EXECUTE_COMMAND_RESULT TurnFanOn(ContosoAnemometer* device)
{
    (void)device;
    (void)printf("Turning fan on.\r\n");
    return EXECUTE_COMMAND_SUCCESS;
}

EXECUTE_COMMAND_RESULT TurnFanOff(ContosoAnemometer* device)
{
    (void)device;
    (void)printf("Turning fan off.\r\n");
    return EXECUTE_COMMAND_SUCCESS;
}

EXECUTE_COMMAND_RESULT SetAirResistance(ContosoAnemometer* device, int Position)
{
    (void)device;
    (void)printf("Setting Air Resistance Position to %d.\r\n", Position);
    return EXECUTE_COMMAND_SUCCESS;
}

void sendCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void* userContextCallback)
{
    unsigned int messageTrackingId = (unsigned int)(uintptr_t)userContextCallback;

    (void)printf("Message Id: %u Received.\r\n", messageTrackingId);

    (void)printf("Result Call Back Called! Result is: %s \r\n", ENUM_TO_STRING(IOTHUB_CLIENT_CONFIRMATION_RESULT, result));
    blinkLed();
}

void blinkLed(){
  

    digitalWrite(LED_BUILTIN, LOW);
  delay(1000); // Esperar un segundo
  // Apagar el LED
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000); // Esperar un segundo
  }
static void sendMessage(IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle, const unsigned char* buffer, size_t size, ContosoAnemometer *myWeather)
{
   (void)printf("Sending");
    static unsigned int messageTrackingId;
    IOTHUB_MESSAGE_HANDLE messageHandle = IoTHubMessage_CreateFromByteArray(buffer, size);
    if (messageHandle == NULL)
    {
        printf("unable to create a new IoTHubMessage\r\n");
    }
    else
    {
        MAP_HANDLE propMap = IoTHubMessage_Properties(messageHandle);
        (void)sprintf_s(propText, sizeof(propText), myWeather->Temperature > 28 ? "true" : "false");
        if (Map_AddOrUpdate(propMap, "temperatureAlert", propText) != MAP_OK)
        {
            (void)printf("ERROR: Map_AddOrUpdate Failed!\r\n");
        }

        if (IoTHubClient_LL_SendEventAsync(iotHubClientHandle, messageHandle, sendCallback, (void*)(uintptr_t)messageTrackingId) != IOTHUB_CLIENT_OK)
        {
            printf("failed to hand over the message to IoTHubClient");
        }
        else
        {
            printf("IoTHubClient accepted the message for delivery\r\n");
        }
        IoTHubMessage_Destroy(messageHandle);
    }
    messageTrackingId++;
}

/*this function "links" IoTHub to the serialization library*/
static IOTHUBMESSAGE_DISPOSITION_RESULT IoTHubMessage(IOTHUB_MESSAGE_HANDLE message, void* userContextCallback)
{
    IOTHUBMESSAGE_DISPOSITION_RESULT result;
    const unsigned char* buffer;
    size_t size;
    if (IoTHubMessage_GetByteArray(message, &buffer, &size) != IOTHUB_MESSAGE_OK)
    {
        printf("unable to IoTHubMessage_GetByteArray\r\n");
        result = IOTHUBMESSAGE_ABANDONED;
    }
    else
    {
        /*buffer is not zero terminated*/
        char* temp = malloc(size + 1);
        if (temp == NULL)
        {
            printf("failed to malloc\r\n");
            result = IOTHUBMESSAGE_ABANDONED;
        }
        else
        {
            (void)memcpy(temp, buffer, size);
            temp[size] = '\0';
            EXECUTE_COMMAND_RESULT executeCommandResult = EXECUTE_COMMAND(userContextCallback, temp);
            result =
                (executeCommandResult == EXECUTE_COMMAND_ERROR) ? IOTHUBMESSAGE_ABANDONED :
                (executeCommandResult == EXECUTE_COMMAND_SUCCESS) ? IOTHUBMESSAGE_ACCEPTED :
                IOTHUBMESSAGE_REJECTED;
            free(temp);
        }
    }
    return result;
}
 
void simplesample_mqtt_run(int temperature)
{     
    if (platform_init() != 0)
    {
        (void)printf("Failed to initialize platform.\r\n");
    }
    else
    {
        if (serializer_init(NULL) != SERIALIZER_OK)
        {
            (void)printf("Failed on serializer_init\r\n");
        }
        else
        {
            IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString(connectionString, MQTT_Protocol);
            srand((unsigned int)time(NULL));
            int avgWindSpeed = 10;
            float minTemperature = temperature;
            float minHumidity = 60.0;

            if (iotHubClientHandle == NULL)
            {
                (void)printf("Failed on IoTHubClient_LL_Create\r\n");
            }
            else
            {
#ifdef SET_TRUSTED_CERT_IN_SAMPLES
                // For mbed add the certificate information
                if (IoTHubClient_LL_SetOption(iotHubClientHandle, "TrustedCerts", certificates) != IOTHUB_CLIENT_OK)
                {
                    (void)printf("failure to set option \"TrustedCerts\"\r\n");
                }
#endif // SET_TRUSTED_CERT_IN_SAMPLES


                ContosoAnemometer* myWeather = CREATE_MODEL_INSTANCE(WeatherStation, ContosoAnemometer);
                if (myWeather == NULL)
                {
                    (void)printf("Failed on CREATE_MODEL_INSTANCE\r\n");
                }
                else
                {
                    if (IoTHubClient_LL_SetMessageCallback(iotHubClientHandle, IoTHubMessage, myWeather) != IOTHUB_CLIENT_OK)
                    {
                        printf("unable to IoTHubClient_SetMessageCallback\r\n");
                    }
                    else
                    {
                        while (1)
                        {
                        int tempRead = analogRead(0);
      
                         float tempVolt = (float) tempRead / 1024.0 * 3.3; 
   
                       float tempCelsius = (tempVolt * 100.0) + 20.0;
 
                        bool temAlert =  tempCelsius > 24;
                        myWeather->Temperature = tempCelsius;
   myWeather->ATemperature = tempCelsius;
                        myWeather->TemperatureAlert = (tempCelsius > 20.0);
                        myWeather->DeviceId = "Termometro VPN";
                        myWeather->WindSpeed = analogRead(0) ;
                        myWeather-> Humidity =digitalRead(1);
                        myWeather-> Longitude= 10.0;
                             myWeather-> Location= "Field";
                        myWeather-> Latitude= 10.0;
                          myWeather-> type= "termometro";
                        
                        {
                            unsigned char* destination;
                            size_t destinationSize;
                            if (SERIALIZE(&destination, &destinationSize,    myWeather-> Location,  myWeather->ATemperature , myWeather->DeviceId, myWeather->WindSpeed, myWeather->Temperature, myWeather->Humidity, myWeather->Latitude,myWeather->Longitude,    myWeather-> type) != CODEFIRST_OK)
                            {
                                (void)printf("Failed to serialize\r\n");
                            }
                            else
                            {
                                   //     (void)printf("A verga!!!!!!!!!!!!!!!!!"+read(0)+"\r\n");
                                sendMessage(iotHubClientHandle, destination, destinationSize, myWeather);
                                    (void)printf("A verga  1 !!!!!!!!!!!!!!!!!\r\n");
                                free(destination);
                                    (void)printf("A verga 2 !!!!!!!!!!!!!!!!!\r\n");
                                IoTHubClient_LL_DoWork(iotHubClientHandle);
                            }
                        }
                           ThreadAPI_Sleep(5000);
                        }
                        /* wait for commands */
                        while (1)
                        {
                            IoTHubClient_LL_DoWork(iotHubClientHandle);
                            ThreadAPI_Sleep(100);
                        }
                    }

                    DESTROY_MODEL_INSTANCE(myWeather);
                }
                IoTHubClient_LL_Destroy(iotHubClientHandle);
            }
            serializer_deinit();
        }
        platform_deinit();
    }
}


void sample_run(int temp)
{
  
   (void)printf("run........");
    simplesample_mqtt_run(temp);
     (void)printf("run........");
}
