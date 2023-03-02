# MyIOT-firmware



### lib/examples/example.cpp
```c++
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



// create router
Router* router = Router::getInstance();

// declare device capabilities
Serializable<bool> power(std::vector<ClassDescription>{Bool});
Serializable<int> speed(std::vector<ClassDescription>{Int});


// device name
std::string deviceName = "test device";
// device unique identifier
std::string identifier = "123123";

void setup() {
    // create serial communication
    Serial.begin(115200);

    // create wifi ap
    WiFi.softAP("to jsem ja", nullptr);


    // register capabilities to router
    router->createCapability<bool>("switch", "/switch", "switch", "bool", &power);
    router->createCapability<int>("speed", "/speed", "speed", "int", &speed);

    // register connection backends
    router->registerConnection(new SyncHttpConnection());
    router->registerConnection(new TcpConnection());

    // start connection backends
    router->enableConnection(Tcp);
    router->enableConnection(Http);


    // !IMPORTANT
    // make our backends discoverable
    // without this application cant discover our device
    if(!MDNS.begin(deviceName.c_str())) {
        Serial.println("Error starting mDNS");
    }

    // set device name
    MDNS.setInstanceName(deviceName.c_str());

    // register http service
    MDNS.addService("iotHttp", "tcp", 80);
    MDNS.addServiceTxt("iotHttp", "tcp", "identifier", identifier.c_str());

    // register tcp service
    MDNS.addService("iotTcp", "tcp", 420);
    MDNS.addServiceTxt("iotTcp", "tcp", "identifier", identifier.c_str());


    // set pin mode to output
    pinMode(26, OUTPUT);
}

void loop() {
    // !IMPORTANT
    // needed so connections can process clients
    router->handle();
    delay(10);

    // set pin to state of our capability
    digitalWrite(26, *power.data);
}
```
