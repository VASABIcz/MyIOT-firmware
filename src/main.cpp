


#include <ESPmDNS.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include "Router.h"
#include "base64.h"
#include "MyHttp.h"
#include "MyTcp.h"

Router* router = Router::getInstance();
Serializable<bool> power(std::vector<ClassDescription>{Bool});
std::string deviceName = "test device";
std::string identifier = "123123";


void setup() {
    Serial.begin(115200);
    WiFi.softAP("to jsem ja", nullptr);
    delay(1000);

    // 20 + 6 + 7 + 6 + 4
    router->createCapability<bool>("switch", "/switch", "switch", "bool", &power);

    if(!MDNS.begin(deviceName.c_str())) {
        Serial.println("Error starting mDNS");
        return;
    }

    MDNS.setInstanceName(deviceName.c_str());

    MDNS.addService("iotHttp", "tcp", 80);
    MDNS.addServiceTxt("iotHttp", "tcp", "identifier", identifier.c_str());

    httpSetup(router);


    pinMode(26, OUTPUT);
    *power.data = false;
}

void loop() {
    httpHandle();
    delay(10);
    digitalWrite(26, *power.data);
}