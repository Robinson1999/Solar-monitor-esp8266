

//Thingsboard
#include <PubSubClient.h>
#include <ArduinoJson.hpp>
#include <ArduinoJson.h>
#include <ThingsBoard.h>

#define TOKEN "xbTZzfiFSwOW55TjM5LO"
char thingsboardServer[] = "http://thingsboard.labparvum.com";

WiFiClient wifiClient;

ThingsBoard tb(wifiClient);

int status = WL_IDLE_STATUS;

void tbCheckConnection()
{
    if (!tb.connected())
        tbConnect();
}

void tbConnect()
{
    // Loop until we're reconnected
    while (!tb.connected()) {
        status = WiFi.status();
        if (status != WL_CONNECTED) {
            wifiConnect();
        }
        Serial.print("Connecting to ThingsBoard node ...");
        if (tb.connect(thingsboardServer, TOKEN)) {
            Serial.println("[DONE]");
        }
        else {
            Serial.print("[FAILED]");
            Serial.println(" : retrying in 5 seconds]");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

void sendDataThingsboard(float data, String datatype)
{

    tb.sendTelemetryFloat("temperature", data);
    

}