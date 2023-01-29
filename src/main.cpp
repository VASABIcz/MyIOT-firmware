


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


void setup() {
    Serial.begin(115200);
    WiFi.softAP("to jsem ja", nullptr);
    delay(1000);

    router->createCapability<bool>("switch", "/switch", "switch", "bool", &power);

    httpSetup(router);
}

void loop() {
    httpHandle();
    delay(10);
}