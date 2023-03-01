#include <ESPmDNS.h>
#include <WiFi.h>
#include <Wire.h>
#include <Arduino.h>

#include <WebServer.h>
#include "Router.h"
#include "connection.h"

#include "base64.h"
#include "MyHttp.h"
#include "serialization.h"
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
    }

    MDNS.setInstanceName(deviceName.c_str());

    router->registerConnection(new SyncHttpConnection());
    router->registerConnection(new TcpConnection());

    router->enableConnection(Tcp);
    router->enableConnection(Http);

    MDNS.addService("iotHttp", "tcp", 80);
    MDNS.addServiceTxt("iotHttp", "tcp", "identifier", identifier.c_str());

    MDNS.addService("iotTcp", "tcp", 420);
    MDNS.addServiceTxt("iotTcp", "tcp", "identifier", identifier.c_str());


    pinMode(26, OUTPUT);
    *power.data = false;
    // capabilities
}

void loop() {
    router->handle();
    delay(10);
    digitalWrite(26, *power.data);
}