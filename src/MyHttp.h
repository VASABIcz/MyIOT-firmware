//
// Created by vasabi on 29.1.23.
//


#ifndef EMBEDED_MYHTTP_H
#define EMBEDED_MYHTTP_H

#include <WebServer.h>
#include "Router.h"
#include "Arduino.h"

WebServer server(80);

void httpSetup(Router* router) {
    server.on("/", [](){
        server.send(200, "text/plain", "UwU");
    });

    for (const auto& pair : router->capatibilites) {
        auto cap = pair.second;

        server.on(cap->route.c_str(), HTTP_POST, [cap](){
            auto body = server.arg("plain");
            auto r = Router::getInstance();
            r->capabilitySet(cap->route, (void*)body.c_str());
            auto val = r->capabilityGet(cap->route);
            auto str = base64_encode((unsigned char*)val.data, val.size);

            server.send(200, "text/plain", str.c_str());
        });
        server.on(cap->route.c_str(), HTTP_GET, [cap](){
            Serial.println("received get request");
            auto r = Router::getInstance();
            Serial.println("got router instance");
            Serial.println((unsigned long int)cap);
            Serial.println((unsigned long int)r);
            auto val = r->capabilityGet(cap->route);
            Serial.println("marshaled data");
            auto str = base64_encode((unsigned char*)val.data, val.size);
            Serial.println("b64 encoded data");

            server.send(200, "text/plain", str.c_str());
        });
    }
    server.begin();
}

void httpHandle() {
    server.handleClient();
}


#endif //EMBEDED_MYHTTP_H
